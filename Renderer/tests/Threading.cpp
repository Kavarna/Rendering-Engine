#ifdef BUILD_TESTS

#include "gtest/gtest.h"
#include "Jnrlib.h"

#include "glog/logging.h"

using namespace Jnrlib;

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
        constexpr const unsigned int numbersCount = 100;
        int numbers[numbersCount] = {};

        for (uint32_t j = 0; j < numbersCount; ++j)
        {
            LOG(INFO) << "Adding task for index = " << j;
            threadPool->ExecuteDeffered([j, &numbers]()
            {
                numbers[j] = 1;
            });
        }

        threadPool->WaitForAll();
        LOG(INFO) << "Stopped waiting";

        // Check the result
        for (uint32_t j = 0; j < numbersCount; ++j)
        {
            EXPECT_EQ(numbers[j], 1);
            if (::testing::Test::HasFailure())
            {
                LOG(INFO) << "Assertion failed for index = " << j;
            }
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
            threadPool->ExecuteDeffered([&numbers]()
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
            threadPool->ExecuteDeffered([&numbersMutex, &numbers]()
        {
            std::unique_lock<std::mutex> lock(numbersMutex);
            numbers.insert(0);
            std::this_thread::sleep_for(std::chrono::nanoseconds(10));
        });

        threadPool->Wait(task, ThreadPool::WaitPolicy::EXECUTE_THEN_EXIT);

        EXPECT_NE(numbers.find(0), numbers.end());
        threadPool->WaitForAll();
    }

    /*TEST_P(Threading, ParralelForCorrectness)
    {
        EXPECT_NO_THROW(auto threadPool = ThreadPool::Get());

        auto threadPool = ThreadPool::Get();
        EXPECT_NE(threadPool, nullptr);

        const unsigned int cols = threadPool->GetNumberOfThreads() * 100;
        const unsigned int rows = 10;
        std::mutex numbersMutex;
        std::set<std::pair<uint32_t, uint32_t>> numbers;
        std::shared_ptr<struct Task> forTask =
            threadPool->ParralelForDeffered2D([&numbersMutex, &numbers](uint32_t i, uint32_t j)
        {
            std::unique_lock<std::mutex> lock(numbersMutex);
            numbers.insert({i,j});
            std::this_thread::sleep_for(std::chrono::nanoseconds(10));
        }, rows, cols, cols / threadPool->GetNumberOfThreads());



    }*/

    INSTANTIATE_TEST_SUITE_P(ThreadingTests, Threading, testing::Range(0, 1000));

}

#endif // BUILD_TESTS