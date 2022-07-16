#include "ThreadPool.h"
#include "glog/logging.h"
#include "Exceptions.h"

#undef max

#define THROW_TASK_NOT_FOUND {\
std::string error = "Could not find task with ID = ";\
error += std::to_string(task->taskID);\
throw Jnrlib::Exceptions::TaskNotFound(error);\
}\


struct Task
{
    static uint64_t TaskID;

    Task(std::function<void()> func) :
        work(func), taskID(TaskID++)
    { };

    std::function<void()> work;
    uint64_t taskID;

    std::shared_ptr<Task> nextTask = nullptr;
    
    bool completed = false;
};

uint64_t Task::TaskID = 0;

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
    std::shared_ptr<Task> currentTask = std::make_shared<Task>(func);
    {
        std::unique_lock<std::mutex> lock(mWorkListMutex);
        if (mWorkList != nullptr) [[likely]]
        {
            std::shared_ptr<Task> lastTask = mWorkList;
            while (true)
            {
                if (lastTask->nextTask == nullptr)
                    break;
                lastTask = lastTask->nextTask;
            }
            lastTask->nextTask = currentTask;
        }
        else
        {
            mWorkList = currentTask;
        }
    }
    mWorkersCV.notify_all();
    return currentTask;
}

bool ThreadPool::IsTaskFinished(std::shared_ptr<struct Task> task)
{
    std::unique_lock<std::mutex> lock(mCompletedTasksMutex);
    return mCompletedTasks.find(task->taskID) != mCompletedTasks.end();
}

bool ThreadPool::IsTaskActive(std::shared_ptr<struct Task> task)
{
    std::unique_lock<std::mutex> lock(mActiveTasksMutex);
    return mActiveTasks.find(task->taskID) != mActiveTasks.end();
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

void ThreadPool::WaitForAll()
{
    std::unique_lock<std::mutex> lock(mWorkListMutex);
    if (mWorkList == nullptr && mActiveTasks.size() == 0)
        return;
    mWorkersCV.wait(lock, [this]
    {
        return mWorkList == nullptr && mActiveTasks.size() == 0;
    });
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
            VLOG(2) << "Thread " << index << " starts waiting for work";
            mWorkersCV.wait(lock, [this] { return mWorkList != nullptr; });
            VLOG(2) << "Thread " << index << " stopped waiting for work" ;
        }
        
        if (mShouldClose)
        {
            VLOG(2) << "Thread " << index << " is shutting down";
            break;
        }
        else
        {
            if (mWorkList == nullptr)
                continue;

            std::shared_ptr<Task> myTask = mWorkList;
            mWorkList = mWorkList->nextTask;
            
            {
                std::unique_lock<std::mutex> lock(mActiveTasksMutex);
                mActiveTasks.insert(myTask->taskID);
            }

            lock.unlock();

            myTask->work();
            myTask->completed = true;

            {
                std::unique_lock<std::mutex> lock(mCompletedTasksMutex);
                mCompletedTasks.insert(myTask->taskID);
                mWorkersCV.notify_all();
            }
            {
                std::unique_lock<std::mutex> lock(mActiveTasksMutex);
                mActiveTasks.erase(myTask->taskID);
                mWorkersCV.notify_all();
            }

            lock.lock();
        }
    }
}

void ThreadPool::WaitForTaskToFinish(std::shared_ptr<struct Task> task)
{
    if (IsTaskFinished(task))
        return;
    // Just wait for other thread to finish this
    std::unique_lock<std::mutex> lock(mCompletedTasksMutex);
    mWorkersCV.wait(lock, [&]
    {
        return mCompletedTasks.find(task->taskID) != mCompletedTasks.end();
    });
}

void ThreadPool::ExecuteTasksUntilTaskCompleted(std::shared_ptr<struct Task> task)
{
    // Execute tasks in the queue until the searched task is executed
    std::unique_lock<std::mutex> lock(mWorkListMutex);
    while (true)
    {
        if (IsTaskFinished(task))
            break;
        if (IsTaskActive(task))
        {
            mWorkersCV.wait(lock, [&]
            {
                return IsTaskFinished(task);
            });
            break;
        }

        if (mWorkList == nullptr) [[unlikely]]
        {
            if (IsTaskActive(task))
            {
                mWorkersCV.wait(lock, [&]
                {
                    return IsTaskFinished(task);
                });
                break;
            }
            else
            {
                if (IsTaskFinished(task))
                    break;
                // Task not finished, not active and there's no tasks remaining => there is no such task
                THROW_TASK_NOT_FOUND;
            }
        }

        std::shared_ptr<Task> myTask = mWorkList;
        mWorkList = mWorkList->nextTask;
        
        {
            std::unique_lock<std::mutex> lock(mActiveTasksMutex);
            mActiveTasks.insert(myTask->taskID);
        }

        lock.unlock();

        myTask->work();
        myTask->completed = true;

        {
            std::unique_lock<std::mutex> lock(mCompletedTasksMutex);
            mCompletedTasks.insert(myTask->taskID);
        }
        {
            std::unique_lock<std::mutex> lock(mActiveTasksMutex);
            mActiveTasks.erase(myTask->taskID);
        }

        lock.lock();
    }
}

void ThreadPool::ExecuteSpecificTask(std::shared_ptr<struct Task> task)
{
    if (IsTaskFinished(task))
        return;
    if (IsTaskActive(task))
    {
        WaitForTaskToFinish(task);
        return;
    }

    // Search for the task and then execute it
    std::unique_lock<std::mutex> lock(mWorkListMutex);
    if (mWorkList == nullptr) [[unlikely]]
    {
        if (IsTaskFinished(task))
           return;
        if (IsTaskActive(task))
        {
            WaitForTaskToFinish(task);
            return;
        }
        std::string error = "Could not find task with ID = ";
        error += std::to_string(task->taskID);
        throw Jnrlib::Exceptions::TaskNotFound(error);
    }

    std::shared_ptr<Task> previousTask = mWorkList;
    std::shared_ptr<Task> currentTask = previousTask->nextTask;

    if (previousTask->taskID == task->taskID)
    {
        // Delete the first element from the list, execute the work and then return
        mWorkList = mWorkList->nextTask;
        lock.unlock();
        previousTask->work();
        return;
    }

    while (currentTask != nullptr && currentTask->taskID != task->taskID)
    {
        previousTask = currentTask;
        currentTask = currentTask->nextTask;
    }

    if (currentTask == nullptr)
        THROW_TASK_NOT_FOUND;
    
    if (currentTask->taskID == task->taskID)
    {
        previousTask->nextTask = currentTask->nextTask;
        lock.unlock();
        task->work();
        return;
    }

    THROW_TASK_NOT_FOUND;
}
