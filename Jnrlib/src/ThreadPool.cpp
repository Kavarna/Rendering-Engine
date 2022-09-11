#include "ThreadPool.h"
#include "glog/logging.h"
#include "Exceptions.h"

#undef max

namespace Jnrlib
{
    struct Task
    {
        static uint64_t TaskID;

        Task() : taskID(TaskID++)
        {
            VLOG(3) << "Task with ID = " << taskID << " was created";
        };

        ~Task()
        {

            VLOG(3) << "Task with ID = " << taskID << " was destroyed";
        }

        uint64_t taskID;

        std::shared_ptr<Task> nextTask = nullptr;

        virtual void Work() = 0;
        virtual bool IsCompleted() const = 0;
        virtual void Complete() = 0;
    };

    struct SimpleTask : public Task
    {
        bool completed = false;

        SimpleTask(std::function<void()> work) :
            Task(), work(work)
        {
        }

        std::function<void()> work;

        void Work()
        {
            VLOG(3) << "Task with ID = " << taskID << " is being executed now";
            work();
            Complete();
            VLOG(3) << "Task with ID = " << taskID << " is done being executed";
        }

        void Complete() override
        {
            completed = true;
        }

        bool IsCompleted() const override
        {
            return completed;
        }
    };


    struct ChunkTask2D : public Task
    {
        struct ChunkWork
        {
            std::mutex mu;
            uint32_t leftWork;
            uint64_t parentTaskID;
        };

        std::shared_ptr<ChunkWork> chunkWork;

        uint32_t row;
        uint32_t startCol, chunkSize;
        std::function<void(uint32_t, uint32_t)> func;

        ChunkTask2D(std::function<void(uint32_t, uint32_t)> func, uint32_t row, uint32_t startCol, uint32_t chunkSize, uint32_t totalWork) :
            Task(), func(func), row(row), startCol(startCol), chunkSize(chunkSize)
        {
            chunkWork = std::make_shared<ChunkWork>();
            chunkWork->leftWork = totalWork;
            chunkWork->parentTaskID = taskID;
        }

        ChunkTask2D(std::function<void(uint32_t, uint32_t)> func, uint32_t row, uint32_t startCol, uint32_t chunkSize, ChunkTask2D* const task2D) :
            Task(), func(func), row(row), startCol(startCol), chunkSize(chunkSize), chunkWork(task2D->chunkWork)
        {
        }

        void Work() override
        {
            for (uint32_t i = startCol; i < chunkSize; ++i)
            {
                func(row, i);
            }
            Complete();
        }

        void Complete() override
        {
            std::unique_lock<std::mutex> lock(chunkWork->mu);
            chunkWork->leftWork -= 1;
        }

        bool IsCompleted() const override
        {
            std::unique_lock<std::mutex> lock(chunkWork->mu);
            return chunkWork->leftWork == 0;
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
    std::shared_ptr<Task> currentTask = std::make_shared<SimpleTask>(func);
    {
        std::unique_lock<std::mutex> lock(mWorkListMutex);
        if (mWorkList != nullptr)
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
    VLOG(2) << "Task " << currentTask->taskID << " was inserted into the work list";
    mWorkersCV.notify_all();
    return currentTask;
}

std::shared_ptr<struct Task> Jnrlib::ThreadPool::ParralelForDeffered2D(std::function<void(uint32_t, uint32_t)> func, uint32_t nRows, uint32_t nCols, uint32_t chunkSize)
{
    LOG_IF(FATAL, nCols % chunkSize != 0) << "Using parralel for an undivisible number of columns";

    // TODO: Unwrap the function to multiple tasks that would have the same ID


    return std::shared_ptr<struct Task>();
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

uint32_t Jnrlib::ThreadPool::GetNumberOfThreads() const
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

void ThreadPool::WaitForAll()
{
    // Make sure that there are no tasks to be run
    std::unique_lock<std::mutex> lock(mWorkListMutex);
    if (mWorkList == nullptr)
        return;
    mWorkersCV.wait(lock, [this]
    {
        return mWorkList == nullptr;
    });
    
    // Make sure there are no active tasks 
    uint64_t oldValue = mActiveTasksCount;
    
    while (oldValue != 0)
    {
        mActiveTasksCount.wait(oldValue, std::memory_order::relaxed);
        oldValue = mActiveTasksCount;
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
            
            {
                std::unique_lock<std::mutex> lock(mActiveTasksMutex);
                mActiveTasks.insert(myTask->taskID);
                mActiveTasksCount++;
                VLOG(3) << "Thread " << index << " increased active tasks to " << mActiveTasksCount;
                VLOG(2) << "Adding to active tasks task with id = " << myTask->taskID;
            }

            lock.unlock();

            myTask->Work();

            {
                VLOG(2) << "Removing to active tasks task with id = " << myTask->taskID;
                std::unique_lock<std::mutex> lock(mActiveTasksMutex);
                mActiveTasks.erase(myTask->taskID);
                mActiveTasksCount--;
                VLOG(3) << "Thread " << index << " decreased active tasks to " << mActiveTasksCount;
                mActiveTasksCount.notify_all();
            }

            if (myTask->IsCompleted())
            {
                std::unique_lock<std::mutex> lock(mCompletedTasksMutex);
                mCompletedTasks.insert(myTask->taskID);
                mWorkersCV.notify_all();
            }

            lock.lock();
        }
    }
}

void ThreadPool::WaitForTaskToFinish(std::shared_ptr<struct Task> task)
{
    // Just wait for other thread to finish this
    std::unique_lock<std::mutex> lock(mCompletedTasksMutex);
    if (mCompletedTasks.find(task->taskID) != mCompletedTasks.end())
        return;
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

        if (mWorkList == nullptr)
        {
            if (IsTaskActive(task))
            {
                mWorkersCV.wait(lock, [&]
                {
                    return mCompletedTasks.find(task->taskID) != mCompletedTasks.end();
                });
                break;
            }
            else
            {
                if (IsTaskFinished(task))
                    break;
                // Task not finished, not active and there's no tasks remaining => there is no such task
                throw Jnrlib::Exceptions::TaskNotFound(task->taskID);
            }
        }

        std::shared_ptr<Task> myTask = mWorkList;
        mWorkList = mWorkList->nextTask;
        
        {
            std::unique_lock<std::mutex> lock(mActiveTasksMutex);
            mActiveTasks.insert(myTask->taskID);
        }

        lock.unlock();

        myTask->Work();

        if (myTask->IsCompleted())
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
    {
        VLOG(4) << "Task" << task->taskID << " is finished, returning";
        return;
    }
    if (IsTaskActive(task))
    {
        VLOG(4) << "Task" << task->taskID << " active, waiting for it";
        WaitForTaskToFinish(task);
        VLOG(4) << "Task" << task->taskID << " finished, returning";
        return;
    }

    // Search for the task and then execute it
    VLOG(4) << "Task" << task->taskID << ". Locking work list mutex";
    std::unique_lock<std::mutex> lock(mWorkListMutex);
    if (mWorkList == nullptr)
    {
        if (IsTaskFinished(task))
        {
            VLOG(4) << "Task" << task->taskID << " is finished, returning";
            return;
        }
        if (IsTaskActive(task))
        {
            VLOG(4) << "Task" << task->taskID << " active, waiting for it";
            WaitForTaskToFinish(task);
            VLOG(4) << "Task" << task->taskID << " finished, returning";
        }
        VLOG(4) << "Task" << task->taskID << " not found, throwing an exception";
        throw Jnrlib::Exceptions::TaskNotFound(task->taskID);
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

    do
    {
        while (currentTask != nullptr && currentTask->taskID != task->taskID)
        {
            previousTask = currentTask;
            currentTask = currentTask->nextTask;
        }

        if (currentTask == nullptr)
        {
            // If we didn't find the task, something must be wrong :-(
            // But we give it one more try and wait for all active workers to finish their tasks
            uint64_t oldValue = mActiveTasksCount;
            while (oldValue != 0)
            {
                mActiveTasksCount.wait(oldValue, std::memory_order::relaxed);
                oldValue = mActiveTasksCount;
            }

            // And then check if the task is actually finished
            if (IsTaskFinished(task))
            {
                return;
            }
            // And if that doesn't work, we throw an error
            throw Jnrlib::Exceptions::TaskNotFound(task->taskID);
        }
        
    } while (currentTask == nullptr);
    
    if (currentTask->taskID == task->taskID)
    {
        VLOG(4) << "Task" << task->taskID << " was found, deleting it and executing it";
        previousTask->nextTask = currentTask->nextTask;
        lock.unlock();
        VLOG(4) << "Task" << task->taskID << " was not found, throwing an exception";
        task->Work();
        task->Complete();
        return;
    }

    // This should never happen
    throw Jnrlib::Exceptions::ImpossibleToGetHere("Task ID = " + std::to_string(task->taskID) + ". Could not be found");
}
