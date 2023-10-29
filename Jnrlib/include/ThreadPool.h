#pragma once


#include "Singletone.h"
#include <thread>
#include <unordered_set>
#include <functional>
#include <condition_variable>
#include <atomic>


namespace Jnrlib
{
    class ThreadPool : public Jnrlib::ISingletone<ThreadPool>
    {
        MAKE_SINGLETONE_CAPABLE(ThreadPool);
    private:
        ThreadPool(uint32_t nthreads = std::thread::hardware_concurrency() - 1);
        ~ThreadPool() final;

    public:
        enum class WaitPolicy
        {
            EXECUTE_THEN_EXIT,
            EXIT_ASAP,
            SEARCH_AND_EXECUTE_THEN_EXIT,
        };

    public:
        std::shared_ptr<struct Task> ExecuteDeffered(std::function<void()> func);
        std::vector<std::shared_ptr<struct Task>> ExecuteBatchDeffered(std::vector<std::function<void()>> const& funcs);

        void ExecuteParallelForImmediate(std::function<void(uint32_t)> const& func, uint32_t size, uint32_t batchSize, WaitPolicy wp = WaitPolicy::EXECUTE_THEN_EXIT);
        void Wait(std::shared_ptr<struct Task> task, WaitPolicy wp = WaitPolicy::EXECUTE_THEN_EXIT);
        void WaitForAll(WaitPolicy wp = WaitPolicy::EXECUTE_THEN_EXIT);
        void CancelRemainingTasks();

        bool IsTaskCompleted(std::shared_ptr<struct Task> task);

        uint32_t GetCurrentThreadId() const;
        uint32_t GetNumberOfThreads() const;


    private:
        void Init(uint32_t nthreads);
        void WorkerThread(uint32_t index);

    private:
        void WaitForTaskToFinish(std::shared_ptr<struct Task> task);
        void ExecuteTasksUntilTaskCompleted(std::shared_ptr<struct Task> task);
        void ExecuteSpecificTask(std::shared_ptr<struct Task> task);

        void WaitForAllToFinish();
        void WaitForAllExecutingTasks();

    private:
        std::vector<std::thread> mThreads;

        std::condition_variable mWorkersCV;
        std::atomic<bool> mShouldClose = false;

        // If there will be lots of tasks, adding a look-up table would be very useful
        std::mutex mWorkListMutex;
        std::shared_ptr<struct Task> mWorkList = nullptr;
        
        /* TODO: Keep track of the last task */
        // std::shared_ptr<struct Task> mLastTask = nullptr;

        std::atomic<uint64_t> mActiveTasksCount = 0;
    };


}
