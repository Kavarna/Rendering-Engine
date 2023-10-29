#include "ThreadPool.h"
#include "glog/logging.h"
#include "Exceptions.h"

#undef max

#define WAIT_ALL_ACTIVE_THREADS \
{ \
while (true) \
{\
    if (mActiveTasksCount.load() == 0) \
        break; \
}\
}

namespace Jnrlib
{
    struct Task
    {
        static uint64_t TaskID;

        Task(std::function<void()> work) : taskID(TaskID++), work(work)
        {
            VLOG(2) << "Task with ID = " << taskID << " was created";
        };

        inline ~Task()
        {
            VLOG(2) << "Task with ID = " << taskID << " was destroyed";
        }

        uint64_t taskID;

        std::function<void()> work;
        std::shared_ptr<Task> nextTask = nullptr;

        bool completed = false;

        void Work()
        {
            VLOG(3) << "Task with ID = " << taskID << " is being executed now";
            work();
            Complete();
            VLOG(3) << "Task with ID = " << taskID << " is done being executed";
        }
        bool IsCompleted() const
        {
            return completed;
        };
        void Complete()
        {
            completed = true;
        }
    };

    uint64_t Task::TaskID = 0;
}

using namespace Jnrlib;

ThreadPool::ThreadPool(uint32_t numThreads)
{
    Init(numThreads);
}

ThreadPool::~ThreadPool()
{
    mShouldClose = true;
    for (auto& thread : mThreads)
    {
        thread.join();
    }
}

std::shared_ptr<Task> ThreadPool::ExecuteDeffered(std::function<void()> func)
{
    auto currentTask = std::make_shared<Task>(func);
    {
        std::unique_lock lock(mWorkListMutex);
        if (mWorkList != nullptr)
        {
            currentTask->nextTask = mWorkList;
            mWorkList = currentTask;
        }
        else
        {
            mWorkList = currentTask;
        }
    }
    VLOG(3) << "Task " << currentTask->taskID << " was inserted into the work list";
    mWorkersCV.notify_all();
    return currentTask;
}

std::vector<std::shared_ptr<struct Task>> ThreadPool::ExecuteBatchDeffered(std::vector<std::function<void()>> const& funcs)
{
    std::vector<std::shared_ptr<struct Task>> tasks;
    tasks.reserve(funcs.size());

    std::shared_ptr<struct Task> taskList;
    for (auto const& func : funcs)
    {
        auto currentTask = std::make_shared<Task>(func);
        if (taskList == nullptr) [[unlikely]]
        {
            taskList = currentTask;
        }
        else
        {
            currentTask->nextTask = taskList;
            taskList = currentTask;
        }

        tasks.push_back(currentTask);
        VLOG(3) << "Task " << currentTask->taskID << " was inserted into a local work list";
    }

    {
        std::unique_lock lock(mWorkListMutex);
        if (mWorkList != nullptr)
        {
            CHECK(tasks[0]->nextTask == nullptr) << "The first task in the vector is not the first task inserted in the local work list";
            tasks[0]->nextTask = mWorkList;
            mWorkList = taskList;
        }
        else
        {
            mWorkList = taskList;
        }
        VLOG(3) << "Local task list was inserted in the global work list";
    }

    mWorkersCV.notify_all();
    return tasks;
}

void ThreadPool::ExecuteParallelForImmediate(std::function<void(uint32_t)> const& func, uint32_t size, uint32_t batchSize, WaitPolicy wp)
{
    if (size < batchSize)
    {
        for (uint32_t i = 0; i < size; ++i)
        {
            func(i);
        }
        return;
    }

    uint32_t remainingTasks = size % batchSize;
    uint32_t fullTasks = size / batchSize;

    std::vector<std::function<void()>> tasks;
    tasks.reserve(fullTasks + remainingTasks);

    for (uint32_t taskBatch = 0; taskBatch < fullTasks; ++taskBatch)
    {
        auto task = [batchSize, func, taskBatch]()
        {
            for (uint32_t i = 0; i < batchSize; ++i)
            {
                func(taskBatch * batchSize + i);
            }
        };
        tasks.emplace_back(task);
    }

    std::vector<std::shared_ptr<struct Jnrlib::Task>> tasksToWait;
    if (wp == WaitPolicy::EXECUTE_THEN_EXIT)
    {
        tasksToWait = ExecuteBatchDeffered(tasks);
        for (uint32_t i = 0; i < remainingTasks; ++i)
        {
            func(fullTasks * batchSize + i);
        }
    }
    else
    {
        auto task = [&]()
        {
            for (uint32_t i = 0; i < remainingTasks; ++i)
            {
                func(fullTasks * batchSize + i);
            }
        };
        tasks.emplace_back(task);
        tasksToWait = ExecuteBatchDeffered(tasks);
    }

    for (const auto& task : tasksToWait)
    {
        Wait(task, wp);
    }
}

bool ThreadPool::IsTaskCompleted(std::shared_ptr<struct Task> task)
{
    return task->completed;
}


uint32_t ThreadPool::GetCurrentThreadId() const
{
    auto currentThreadId = std::this_thread::get_id();
    for (uint32_t i = 0; i < mThreads.size(); ++i)
    {
        if (currentThreadId == mThreads[i].get_id())
        {
            return i;
        }
    }
    return -1;
}

uint32_t ThreadPool::GetNumberOfThreads() const
{
    return (uint32_t)mThreads.size();
}

void ThreadPool::Wait(std::shared_ptr<struct Task> task, WaitPolicy wp)
{
    if (wp == WaitPolicy::EXIT_ASAP)
    {
        WaitForTaskToFinish(task);
    }
    else if (wp == WaitPolicy::EXECUTE_THEN_EXIT)
    {
        ExecuteTasksUntilTaskCompleted(task);
    }
    else if (wp == WaitPolicy::SEARCH_AND_EXECUTE_THEN_EXIT)
    {
        ExecuteSpecificTask(task);
    }
}

void ThreadPool::WaitForAll(WaitPolicy wp)
{
    switch (wp)
    {
        case ThreadPool::WaitPolicy::EXECUTE_THEN_EXIT:
            WaitForAllExecutingTasks();
            break;
        case ThreadPool::WaitPolicy::EXIT_ASAP:
            WaitForAllToFinish();
            break;
        default:
            CHECK(false) << "Invalid policy provided to WaitForAll()";
    }
}

void ThreadPool::CancelRemainingTasks()
{
    std::unique_lock<std::mutex> lock(mWorkListMutex);
    
    // Make sure we delete everything
    while (mWorkList)
    {
        mWorkList = mWorkList->nextTask;
    }
}

void ThreadPool::Init(uint32_t nthreads)
{
    auto numThreads = std::max(1u, nthreads);
    mThreads.reserve(numThreads);
    
    for (uint32_t i = 0; i < nthreads; ++i)
    {
        mThreads.emplace_back(&ThreadPool::WorkerThread, this, i);
    }
}

void ThreadPool::WorkerThread(uint32_t index)
{
    std::unique_lock<std::mutex> lock(mWorkListMutex);
    while (true)
    {
        if (!mShouldClose && mWorkList == nullptr)
        {
            // No work to do, just wait
            VLOG(4) << "Thread " << index << " starts waiting for work";
            mWorkersCV.wait(lock, [this] { return mWorkList != nullptr; });
            VLOG(4) << "Thread " << index << " stopped waiting for work" ;
        }
        
        if (mShouldClose)
        {
            VLOG(4) << "Thread " << index << " is shutting down";
            break;
        }
        else
        {
            if (mWorkList == nullptr)
                continue;

            std::shared_ptr<Task> myTask = mWorkList;
            mWorkList = mWorkList->nextTask;
            myTask->nextTask = nullptr;
            
            {
                mActiveTasksCount++;
                VLOG(3) << "Thread " << index << " increased active tasks to " << mActiveTasksCount;
                VLOG(2) << "Adding to active tasks task with id = " << myTask->taskID;
            }

            lock.unlock();

            myTask->Work();

            {
                VLOG(2) << "Removing to active tasks task with id = " << myTask->taskID;
                mActiveTasksCount--;
                VLOG(3) << "Thread " << index << " decreased active tasks to " << mActiveTasksCount;
                mWorkersCV.notify_all();
            }

            lock.lock();
        }
    }
}

void ThreadPool::WaitForTaskToFinish(std::shared_ptr<struct Task> task)
{
    // Just wait for other thread to finish this
    std::unique_lock<std::mutex> lock(mWorkListMutex);
    if (task->IsCompleted())
        return;
    mWorkersCV.wait(lock, [&]
    {
        return task->IsCompleted();
    });
}

void ThreadPool::ExecuteTasksUntilTaskCompleted(std::shared_ptr<struct Task> task)
{
    // Execute tasks in the queue until the searched task is executed
    std::unique_lock<std::mutex> lock(mWorkListMutex);
    while (true)
    {
        // Is it complete? then return
        if (task->IsCompleted())
            break;
        if (mWorkList == nullptr)
        {
            // No work left? Let's check whether the task is completed
            if (task->IsCompleted())
            {
                break;
            }

            // Not completed? Let's wait for active tasks
            bool found = false;

            /* While waiting for the task to be finished, unlock the mutex, so other threads can still add tasks
             * and if they do, it means that this task might depend on one of other threads, so give up waiting and 
             * get back to doing work
             */
            bool shouldContinue = false;
            auto numActiveTasksLast = mActiveTasksCount.load();
            lock.unlock();
            while (true)
            {
                if (task->IsCompleted())
                {
                    found = true;
                    break;
                }
                auto numActiveTasksNow = mActiveTasksCount.load();
                if (mActiveTasksCount.load() == 0)
                    break;

                if (numActiveTasksNow >= numActiveTasksLast)
                {
                    shouldContinue = true;
                }
                numActiveTasksLast = numActiveTasksNow;
            }
            lock.lock();

            if (shouldContinue)
            {
                /* If there are new tasks added to the work list, we should just give up on waiting this one
                 * and continue executing taks untill it's finished
                 */
                continue;
            }

            if (!found && !task->IsCompleted())
            {
                throw Exceptions::TaskNotFound(task->taskID);
            }
            else
            {
                break;
            }
        }

        // Get the first task in work list and then execute it
        std::shared_ptr<Task> myTask = mWorkList;
        mWorkList = mWorkList->nextTask;
        myTask->nextTask = nullptr;

        lock.unlock();

        myTask->Work();

        lock.lock();
    }
}

void ThreadPool::ExecuteSpecificTask(std::shared_ptr<struct Task> task)
{
    if (task->IsCompleted())
    {
        VLOG(4) << "Task" << task->taskID << " is finished, returning";
        return;
    }

    // Search for the task and then execute it
    VLOG(4) << "Task" << task->taskID << ". Locking work list mutex";
    std::unique_lock<std::mutex> lock(mWorkListMutex);
    if (mWorkList == nullptr)
    {
        // No work left, check if the task is completed
        if (IsTaskCompleted(task))
        {
            VLOG(4) << "Task" << task->taskID << " is finished, returning";
            return;
        }
        
        // If it's not completed, then we should wait for all active threads
        WAIT_ALL_ACTIVE_THREADS;

        // Now it MUST be either complete or invalid
        if (task->IsCompleted())
        {
            VLOG(4) << "Task" << task->taskID << " is finished, returning";
            return;
        }
        else
        {
            VLOG(4) << "Task" << task->taskID << " not found, throwing an exception";
            throw Exceptions::TaskNotFound(task->taskID);
        }
    }

    VLOG(4) << "Task" << task->taskID << ". Start searching for it.";
    std::shared_ptr<Task> previousTask = mWorkList;
    std::shared_ptr<Task> currentTask = previousTask->nextTask;

    if (previousTask->taskID == task->taskID)
    {
        VLOG(4) << "Task" << task->taskID << " was found as first task, deleting it and executing it";
        // Delete the first element from the list, execute the work and then return
        mWorkList = mWorkList->nextTask;
        lock.unlock();
        previousTask->Work();
        mWorkersCV.notify_all();
        return;
    }

    while (currentTask != nullptr && currentTask->taskID != task->taskID)
    {
        previousTask = currentTask;
        currentTask = currentTask->nextTask;
    }

    if (currentTask == nullptr)
    {
        // If we didn't find the task, something must be wrong :-(
        // But we give it one more try and wait for all active workers to finish their tasks
        WAIT_ALL_ACTIVE_THREADS;

        // And then check if the task is actually finished
        if (task->IsCompleted())
        {
            return;
        }
        // And if that doesn't work, we throw an error
        throw Exceptions::TaskNotFound(task->taskID);
    }
    
    if (currentTask->taskID == task->taskID)
    {
        VLOG(4) << "Task" << task->taskID << " was found, deleting it and executing it";
        previousTask->nextTask = currentTask->nextTask;
        lock.unlock();
        task->Work();
        task.reset();
        return;
    }

    // This should never happen
    throw Exceptions::ImpossibleToGetHere("Task ID = " + std::to_string(task->taskID) + ". Could not be found");
}

void ThreadPool::WaitForAllToFinish()
{
    // Make sure that there are no tasks to be run
    {
        std::unique_lock<std::mutex> lock(mWorkListMutex);
        if (mWorkList != nullptr)
        {
            mWorkersCV.wait(lock, [this]
            {
                return mWorkList == nullptr;
            });
        }
    }

    WAIT_ALL_ACTIVE_THREADS;
}

void ThreadPool::WaitForAllExecutingTasks()
{
    // Execute tasks in the queue until the searched task is executed
    std::unique_lock<std::mutex> lock(mWorkListMutex);
    while (true)
    {
        // No tasks left in the worklist, wait for all threads to finish working
        if (mWorkList == nullptr)
        {
            // Not completed? Let's wait for active tasks
            bool found = false;
            while (true)
            {
                if (mActiveTasksCount.load() == 0)
                    break;
            }

            break;
        }

        // Get the first task in work list and then execute it
        std::shared_ptr<Task> myTask = mWorkList;
        mWorkList = mWorkList->nextTask;
        myTask->nextTask = nullptr;

        lock.unlock();

        myTask->Work();

        lock.lock();
    }
}
