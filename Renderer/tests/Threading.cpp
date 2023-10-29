#ifdef BUILD_TESTS

#include "gtest/gtest.h"
#include "Jnrlib.h"

#include "glog/logging.h"

using namespace Jnrlib;

namespace
{
    using namespace std::chrono;
    class Threading : public testing::TestWithParam<int>
    { };

    void Func1(std::string& result, std::mutex& mu)
    {
        std::this_thread::sleep_for(1ns);

        std::lock_guard g(mu);
        result += "Task1";
    }

    void Func2(std::string& result, std::mutex& mu, std::shared_ptr<struct Task> task1)
    {
        auto threadPool = ThreadPool::Get();
        threadPool->Wait(task1, ThreadPool::WaitPolicy::EXIT_ASAP);

        std::lock_guard g(mu);
        result += "Task2";
    }

    void Func3(std::string& result, std::mutex& mu, std::shared_ptr<struct Task> task1)
    {
        auto threadPool = ThreadPool::Get();
        threadPool->Wait(task1, ThreadPool::WaitPolicy::EXIT_ASAP);

        std::lock_guard g(mu);
        result += "Task3";
    }

    void Func4(std::string& result, std::mutex& mu, std::shared_ptr<struct Task> task2, std::shared_ptr<struct Task> task3)
    {
        auto threadPool = ThreadPool::Get();
        threadPool->Wait(task2, ThreadPool::WaitPolicy::EXIT_ASAP);
        threadPool->Wait(task3, ThreadPool::WaitPolicy::EXIT_ASAP);

        std::lock_guard g(mu);
        result += "Task4";
    }

    TEST_P(Threading, ThreadingCorectnessWithDependencies)
    {
        EXPECT_NO_THROW(auto threadPool = ThreadPool::Get());

        auto threadPool = ThreadPool::Get();
        EXPECT_NE(threadPool, nullptr);

        std::mutex mu;
        std::string result = "";

        auto task1 = threadPool->ExecuteDeffered(std::bind(Func1, std::ref(result), std::ref(mu)));
        auto task2 = threadPool->ExecuteDeffered(std::bind(Func2, std::ref(result), std::ref(mu), task1));
        auto task3 = threadPool->ExecuteDeffered(std::bind(Func3, std::ref(result), std::ref(mu), task1));
        auto task4 = threadPool->ExecuteDeffered(std::bind(Func4, std::ref(result), std::ref(mu), task2, task3));

        threadPool->WaitForAll(ThreadPool::WaitPolicy::EXIT_ASAP);

        EXPECT_TRUE(result == "Task1Task2Task3Task4" || result == "Task1Task3Task2Task4");
    }

    TEST(Threading, AllThreadsRun)
    {
        EXPECT_NO_THROW(auto threadPool = ThreadPool::Get());

        auto threadPool = ThreadPool::Get();
        EXPECT_NE(threadPool, nullptr);
    }

    TEST_P(Threading, ThreadingEventualCompletenessWait)
    {
        EXPECT_NO_THROW(auto threadPool = ThreadPool::Get());

        auto threadPool = ThreadPool::Get();
        EXPECT_NE(threadPool, nullptr);

        // Test
        constexpr const unsigned int numbersCount = 100;
        int numbers[numbersCount] = {};

        for (uint32_t j = 0; j < numbersCount; ++j)
        {
            threadPool->ExecuteDeffered([j, &numbers]()
            {
                numbers[j] = 1;
            });
        }

        threadPool->WaitForAll(ThreadPool::WaitPolicy::EXIT_ASAP);

        // Check the result
        for (uint32_t j = 0; j < numbersCount; ++j)
        {
            EXPECT_EQ(numbers[j], 1);
        }

    }

    TEST_P(Threading, ThreadingEventualCompletenessWaitExecute)
    {
        EXPECT_NO_THROW(auto threadPool = ThreadPool::Get());

        auto threadPool = ThreadPool::Get();
        EXPECT_NE(threadPool, nullptr);

        // Test
        constexpr const unsigned int numbersCount = 100;
        int numbers[numbersCount] = {};

        for (uint32_t j = 0; j < numbersCount; ++j)
        {
            threadPool->ExecuteDeffered([j, &numbers]()
            {
                numbers[j] = 1;
            });
        }

        threadPool->WaitForAll(ThreadPool::WaitPolicy::EXECUTE_THEN_EXIT);

        // Check the result
        for (uint32_t j = 0; j < numbersCount; ++j)
        {
            EXPECT_EQ(numbers[j], 1);
        }

    }

    TEST_P(Threading, ThreadingImmediateCompletenessPolicyExitASAP)
    {
        EXPECT_NO_THROW(auto threadPool = ThreadPool::Get());

        auto threadPool = ThreadPool::Get();
        EXPECT_NE(threadPool, nullptr);

        constexpr const unsigned int numbersCount = 1000;
        int numbers[numbersCount];

        std::shared_ptr<struct Task> first_task;

        for (uint32_t j = 0; j < numbersCount; ++j)
        {
            auto task = threadPool->ExecuteDeffered([j, &numbers]()
            {
                numbers[j] = 1;
                std::this_thread::sleep_for(std::chrono::nanoseconds(j * 10));
            });
            if (j == 0)
            {
                first_task = task;
            }
        }

        threadPool->Wait(first_task, ThreadPool::WaitPolicy::EXIT_ASAP);

        EXPECT_EQ(numbers[0], 1);
        threadPool->CancelRemainingTasks();
        threadPool->WaitForAll();
    }

    TEST_P(Threading, ThreadingImmediateCompletenessPolicySearchAndExecute)
    {
        EXPECT_NO_THROW(auto threadPool = ThreadPool::Get());

        auto threadPool = ThreadPool::Get();
        EXPECT_NE(threadPool, nullptr);

        constexpr const unsigned int numbersCount = 1000;
        int numbers[numbersCount];

        std::shared_ptr<struct Task> first_task;

        for (uint32_t j = 0; j < numbersCount; ++j)
        {
            auto task = threadPool->ExecuteDeffered([j, &numbers]()
            {
                numbers[j] = 1;
                std::this_thread::sleep_for(std::chrono::nanoseconds(j * 10));
            });
            if (j == 0)
            {
                first_task = task;
            }
        }

        threadPool->Wait(first_task, ThreadPool::WaitPolicy::SEARCH_AND_EXECUTE_THEN_EXIT);

        EXPECT_EQ(numbers[0], 1);
        threadPool->CancelRemainingTasks();
        threadPool->WaitForAll();
    }

    TEST_P(Threading, ThreadingImmediateCompletenessPolicyExecuteThenExit)
    {
        EXPECT_NO_THROW(auto threadPool = ThreadPool::Get());

        auto threadPool = ThreadPool::Get();
        EXPECT_NE(threadPool, nullptr);

        constexpr const unsigned int numbersCount = 1000;
        int numbers[numbersCount];
        std::shared_ptr<struct Task> first_task;

        for (uint32_t j = 0; j < numbersCount; ++j)
        {
            auto task = threadPool->ExecuteDeffered([j, &numbers]()
            {
                numbers[j] = 1;
                std::this_thread::sleep_for(std::chrono::nanoseconds(j * 10));
            });
            if (j == 0)
            {
                first_task = task;
            }
        }

        threadPool->Wait(first_task, ThreadPool::WaitPolicy::EXECUTE_THEN_EXIT);

        EXPECT_EQ(numbers[0], 1);
        threadPool->CancelRemainingTasks();
        threadPool->WaitForAll();
    }

    TEST_P(Threading, ThreadingCompletenessWithOneElementPolicySearchAndExecute)
    {
        EXPECT_NO_THROW(auto threadPool = ThreadPool::Get());

        auto threadPool = ThreadPool::Get();
        EXPECT_NE(threadPool, nullptr);

        const unsigned int numbersCount = 1000;
        std::mutex numbersMutex;
        std::set<uint32_t> numbers;

        std::shared_ptr<struct Task> task =
            threadPool->ExecuteDeffered([&numbersMutex, &numbers]()
        {
            std::unique_lock<std::mutex> lock(numbersMutex);
            numbers.insert(0);
            std::this_thread::sleep_for(std::chrono::nanoseconds(10));
        });

        threadPool->Wait(task, ThreadPool::WaitPolicy::SEARCH_AND_EXECUTE_THEN_EXIT);

        EXPECT_NE(numbers.find(0), numbers.end());
        threadPool->WaitForAll();
    }

    TEST_P(Threading, ThreadingCompletenessWithOneElementPolicyExitASAP)
    {
        EXPECT_NO_THROW(auto threadPool = ThreadPool::Get());

        auto threadPool = ThreadPool::Get();
        EXPECT_NE(threadPool, nullptr);

        constexpr unsigned int numbersCount = 1000;
        uint32_t numbers[numbersCount];

        std::shared_ptr<struct Task> task =
            threadPool->ExecuteDeffered(
                [&numbers]()
                {
                    numbers[0] = 1;
                    std::this_thread::sleep_for(std::chrono::nanoseconds(10));
                });

        threadPool->Wait(task, ThreadPool::WaitPolicy::EXIT_ASAP);

        EXPECT_EQ(numbers[0], 1);
        threadPool->WaitForAll();
    }

    TEST_P(Threading, ThreadingCompletenessWithOneElementPolicyExecuteThenExit)
    {
        EXPECT_NO_THROW(auto threadPool = ThreadPool::Get());

        auto threadPool = ThreadPool::Get();
        EXPECT_NE(threadPool, nullptr);

        const unsigned int numbersCount = 1000;
        std::mutex numbersMutex;
        std::set<uint32_t> numbers;

        std::shared_ptr<struct Task> task =
            threadPool->ExecuteDeffered(
                [&numbersMutex, &numbers]()
                {
                    std::unique_lock<std::mutex> lock(numbersMutex);
                    numbers.insert(0);
                    std::this_thread::sleep_for(std::chrono::nanoseconds(10));
                });

        threadPool->Wait(task, ThreadPool::WaitPolicy::EXECUTE_THEN_EXIT);

        EXPECT_NE(numbers.find(0), numbers.end());
        threadPool->WaitForAll();
    }

    TEST_P(Threading, ThreadingForCompletenessExact)
    {
        EXPECT_NO_THROW(auto threadPool = ThreadPool::Get());

        auto threadPool = ThreadPool::Get();
        EXPECT_NE(threadPool, nullptr);

        const unsigned int batchSize = 64;
        const unsigned int numbersCount = 64 * batchSize;
        std::mutex numbersMutex;
        std::set<uint32_t> numbers;

        threadPool->ExecuteParallelForImmediate(
            [&numbersMutex, &numbers](uint32_t index)
            {
                std::unique_lock<std::mutex> lock(numbersMutex);
                numbers.insert(index);
                std::this_thread::sleep_for(std::chrono::nanoseconds(10));
            }, numbersCount, batchSize);

        EXPECT_EQ(numbers.size(), numbersCount);
        threadPool->WaitForAll();
    }

    TEST_P(Threading, ThreadingForCompletenessRemainder)
    {
        EXPECT_NO_THROW(auto threadPool = ThreadPool::Get());

        auto threadPool = ThreadPool::Get();
        EXPECT_NE(threadPool, nullptr);

        const unsigned int batchSize = 64;
        const unsigned int numbersCount = 64 * batchSize + 32;
        std::mutex numbersMutex;
        std::set<uint32_t> numbers;

        threadPool->ExecuteParallelForImmediate(
            [&numbersMutex, &numbers](uint32_t index)
            {
                std::unique_lock<std::mutex> lock(numbersMutex);
                numbers.insert(index);
                std::this_thread::sleep_for(std::chrono::nanoseconds(10));
            }, numbersCount, batchSize);

        EXPECT_EQ(numbers.size(), numbersCount);
        threadPool->WaitForAll();
    }

    INSTANTIATE_TEST_SUITE_P(ThreadingTests, Threading, testing::Range(0, 100));

}

#endif // BUILD_TESTS