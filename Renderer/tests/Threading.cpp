#include "gtest/gtest.h"
#include "Jnrlib.h"

#include "glog/logging.h"

namespace
{
    class Threading : public testing::TestWithParam<int>
    { };

    TEST(Threading, AllThreadsRun)
    {
        EXPECT_NO_THROW(auto threadPool = ThreadPool::Get());

        auto threadPool = ThreadPool::Get();
        EXPECT_NE(threadPool, nullptr);
    }

    TEST_P(Threading, ThreadingEventualCompleteness)
    {
        EXPECT_NO_THROW(auto threadPool = ThreadPool::Get());

        auto threadPool = ThreadPool::Get();
        EXPECT_NE(threadPool, nullptr);

        // Test
        const unsigned int numbersCount = 100;
        std::mutex numbersMutex;
        std::set<uint32_t> numbers;

        for (uint32_t j = 0; j < numbersCount; ++j)
        {
            threadPool->ExecuteDeffered([j, &numbersMutex, &numbers]()
            {
                std::unique_lock<std::mutex> lock(numbersMutex);
                numbers.insert(j);
            });
        }

        threadPool->WaitForAll();

        // Check the result
        EXPECT_EQ(numbers.size(), numbersCount);
        std::unordered_set<uint32_t> encounteredNumbers;
        for (uint32_t number : numbers)
        {
            EXPECT_EQ(encounteredNumbers.find(number), encounteredNumbers.end());
            encounteredNumbers.insert(number);
        }

    }

    TEST_P(Threading, ThreadingImmediateCompletenessPolicyExitASAP)
    {
        EXPECT_NO_THROW(auto threadPool = ThreadPool::Get());

        auto threadPool = ThreadPool::Get();
        EXPECT_NE(threadPool, nullptr);

        const unsigned int numbersCount = 1000;
        std::mutex numbersMutex;
        std::set<uint32_t> numbers;

        std::shared_ptr<struct Task> first_task;

        for (uint32_t j = 0; j < numbersCount; ++j)
        {
            auto task = threadPool->ExecuteDeffered([j, &numbersMutex, &numbers]()
            {
                std::unique_lock<std::mutex> lock(numbersMutex);
                numbers.insert(j);
                std::this_thread::sleep_for(std::chrono::nanoseconds(j * 10));
            });
            if (j == 0)
            {
                first_task = task;
            }
        }

        threadPool->Wait(first_task, ThreadPool::WaitPolicy::EXIT_ASAP);

        EXPECT_NE(numbers.find(0), numbers.end());
        threadPool->CancelRemainingTasks();
        threadPool->WaitForAll();
    }

    TEST_P(Threading, ThreadingImmediateCompletenessPolicySearchAndExecute)
    {
        EXPECT_NO_THROW(auto threadPool = ThreadPool::Get());

        auto threadPool = ThreadPool::Get();
        EXPECT_NE(threadPool, nullptr);

        const unsigned int numbersCount = 1000;
        std::mutex numbersMutex;
        std::set<uint32_t> numbers;

        std::shared_ptr<struct Task> first_task;

        for (uint32_t j = 0; j < numbersCount; ++j)
        {
            auto task = threadPool->ExecuteDeffered([j, &numbersMutex, &numbers]()
            {
                std::unique_lock<std::mutex> lock(numbersMutex);
                numbers.insert(j);
                std::this_thread::sleep_for(std::chrono::nanoseconds(j * 10));
            });
            if (j == 0)
            {
                first_task = task;
            }
        }

        threadPool->Wait(first_task, ThreadPool::WaitPolicy::SEARCH_AND_EXECUTE_THEN_EXIT);

        EXPECT_NE(numbers.find(0), numbers.end());
        threadPool->CancelRemainingTasks();
        threadPool->WaitForAll();
    }

    TEST_P(Threading, ThreadingImmediateCompletenessPolicyExecuteThenExit)
    {
        EXPECT_NO_THROW(auto threadPool = ThreadPool::Get());

        auto threadPool = ThreadPool::Get();
        EXPECT_NE(threadPool, nullptr);

        const unsigned int numbersCount = 1000;
        std::mutex numbersMutex;
        std::set<uint32_t> numbers;

        std::shared_ptr<struct Task> first_task;

        for (uint32_t j = 0; j < numbersCount; ++j)
        {
            auto task = threadPool->ExecuteDeffered([j, &numbersMutex, &numbers]()
            {
                std::unique_lock<std::mutex> lock(numbersMutex);
                numbers.insert(j);
                std::this_thread::sleep_for(std::chrono::nanoseconds(j * 10));
            });
            if (j == 0)
            {
                first_task = task;
            }
        }

        threadPool->Wait(first_task, ThreadPool::WaitPolicy::EXECUTE_THEN_EXIT);

        EXPECT_NE(numbers.find(0), numbers.end());
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
            std::this_thread::sleep_for(std::chrono::nanoseconds(100));
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

        const unsigned int numbersCount = 1000;
        std::mutex numbersMutex;
        std::set<uint32_t> numbers;

        std::shared_ptr<struct Task> task =
            threadPool->ExecuteDeffered([&numbersMutex, &numbers]()
        {
            std::unique_lock<std::mutex> lock(numbersMutex);
            numbers.insert(0);
            std::this_thread::sleep_for(std::chrono::nanoseconds(100));
        });

        threadPool->Wait(task, ThreadPool::WaitPolicy::EXIT_ASAP);

        EXPECT_NE(numbers.find(0), numbers.end());
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
            threadPool->ExecuteDeffered([&numbersMutex, &numbers]()
        {
            std::unique_lock<std::mutex> lock(numbersMutex);
            numbers.insert(0);
            std::this_thread::sleep_for(std::chrono::nanoseconds(100));
        });

        threadPool->Wait(task, ThreadPool::WaitPolicy::EXECUTE_THEN_EXIT);

        EXPECT_NE(numbers.find(0), numbers.end());
        threadPool->WaitForAll();
    }

    INSTANTIATE_TEST_SUITE_P(ThreadingTests, Threading, testing::Range(0, 100));
}