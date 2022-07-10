#pragma once


#include "Singletone.h"
#include <thread>
#include <unordered_set>
#include <functional>


class ThreadPool : public Jnrlib::ISingletone<ThreadPool>
{
    MAKE_SINGLETONE_CAPABLE(ThreadPool);
private:
    ThreadPool(uint32_t nthreads = std::thread::hardware_concurrency() - 1);
    ~ThreadPool();

public:
    enum class WaitPolicy
    {
        EXECUTE_THEN_EXIT,
        EXIT_ASAP,
        SEARCH_AND_EXECUTE_THEN_EXIT,
    };

public:
    std::shared_ptr<struct Task> ExecuteDeffered(std::function<void()> func);
    void Wait(std::shared_ptr<struct Task> task, WaitPolicy wp = WaitPolicy::EXECUTE_THEN_EXIT);
    void WaitForAll();
    void CancelRemainingTasks();

    bool IsTaskFinished(std::shared_ptr<struct Task> task);
    bool IsTaskActive(std::shared_ptr<struct Task> task);

private:
    void Init(uint32_t nthreads);
    void WorkerThread(uint32_t index);

private:
    void WaitForTaskToFinish(std::shared_ptr<struct Task> task);
    void ExecuteTasksUntilTaskCompleted(std::shared_ptr<struct Task> task);
    void ExecuteSpecificTask(std::shared_ptr<struct Task> task);

private:
    std::vector<std::thread> mThreads;

    std::condition_variable mWorkersCV;
    std::atomic_bool mShouldClose = false;

    // If there will be lots of tasks, adding a look-up table would be very useful
    std::mutex mWorkListMutex;
    std::shared_ptr<struct Task> mWorkList = nullptr;

    std::mutex mCompletedTasksMutex;
    std::unordered_set<uint64_t> mCompletedTasks;

    std::mutex mActiveTasksMutex;
    std::unordered_set<uint64_t> mActiveTasks;
};

