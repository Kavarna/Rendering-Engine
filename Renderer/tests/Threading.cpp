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

    INSTANTIATE_TEST_CASE_P(ThreadingTests, Threading, testing::Range(0, 100));
}