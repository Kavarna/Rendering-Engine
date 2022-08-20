#include "ThreadPool.h"
#include "glog/logging.h"
#include "Exceptions.h"

#undef max

#define THROW_TASK_NOT_FOUND {\
throw Jnrlib::Exceptions::TaskNotFound(task->taskID);\
}\

namespace Jnrlib
{
    struct Task
    {
        static uint64_t TaskID;

        Task() : taskID(TaskID++)
        { };

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
            work();
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

std::shared_ptr<struct Task> Jnrlib::ThreadPool::ParralelForDeffered2D(std::function<void(uint32_t, uint32_t)> func, uint32_t nRows, uint32_t nCols, uint32_t chunkSize)
{
    LOG_IF(FATAL, nCols % chunkSize != 0) << "Using parralel for an undivisible number of columns";



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

            myTask->Work();
            myTask->Complete();

            if (myTask->IsCompleted())
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

        myTask->Work();
        myTask->Complete();

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
        throw Jnrlib::Exceptions::TaskNotFound(task->taskID);
    }

    std::shared_ptr<Task> previousTask = mWorkList;
    std::shared_ptr<Task> currentTask = previousTask->nextTask;

    if (previousTask->taskID == task->taskID)
    {
        // Delete the first element from the list, execute the work and then return
        mWorkList = mWorkList->nextTask;
        lock.unlock();
        previousTask->Work();
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
        task->Work();
        return;
    }

    THROW_TASK_NOT_FOUND;
}
