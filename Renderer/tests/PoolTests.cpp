#ifdef BUILD_TESTS

#include "gtest/gtest.h"
#include "Pool.h"

#include "glog/logging.h"

namespace
{
    TEST(PoolTests, CheckRotation)
    {
        Jnrlib::Pool<int> pool(5);
        for (uint32_t i = 0; i < 5; ++i)
        {
            int& current = pool.GetCurrent();
            EXPECT_EQ(current, 0);
            current = i;
            pool.Advance();
        }

        for (uint32_t i = 0; i < 5; ++i)
        {
            int& current = pool.GetCurrent();
            EXPECT_EQ(current, i);
            pool.Advance();
        }
    }

    TEST(PoolTests, CreateWithArguments)
    {
        Jnrlib::Pool<int> pool(5, 3);
        for (uint32_t i = 0; i < 5; ++i)
        {
            int& current = pool.GetCurrent();
            EXPECT_EQ(current, 3);
            pool.Advance();
        }
    }

    int numberOfObjectsCreatedWithoutArguments = 0;
    int numberOfObjectsCreatedWithArguments = 0;
    int totalNumberOfObjects = 0;
    class AB
    {
    public:

        AB()
        {
            numberOfObjectsCreatedWithoutArguments++;
            totalNumberOfObjects++;
        }

        AB(const AB& ab)
        {
            numberOfObjectsCreatedWithArguments++;
            totalNumberOfObjects++;
        }

        ~AB()
        {
            totalNumberOfObjects--;
        }
    };

    TEST(PoolTests, CreateObjects)
    {
        {
            Jnrlib::Pool<AB> pool(100);
            EXPECT_EQ(numberOfObjectsCreatedWithoutArguments, 100);
        }
        {
            AB ab;
            Jnrlib::Pool<AB> pool(100, ab);
            EXPECT_EQ(numberOfObjectsCreatedWithArguments, 100);
        }
        EXPECT_EQ(totalNumberOfObjects, 0);
    }
}

#endif