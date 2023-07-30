#ifdef BUILD_TESTS

#include "gtest/gtest.h"
#include "Jnrlib.h"

namespace
{
    TEST(Math, RemapRangeBasic)
    {
        CHECK(true);
        struct Interval
        {
            uint32_t start;
            uint32_t end;
        };
        Interval from{.start = 0,.end = 1024};
        Interval to{.start = 0,.end = 256};
        uint32_t value = 256; /* This value is a "quarter" of the first interval, hence we expect the value to be a quarter of the new interval */

        uint32_t valueRemapped = Jnrlib::RemapValueFromIntervalToInterval(value, from.start, from.end, to.start, to.end);

        EXPECT_EQ(valueRemapped, value / 4);

        uint32_t valueRemappedRemapped = Jnrlib::RemapValueFromIntervalToInterval(valueRemapped, to.start, to.end, from.start, from.end);
        EXPECT_EQ(valueRemappedRemapped, value);
    }

    TEST(Math, RemapRangeIntervalStartNegative)
    {
        struct Interval
        {
            int32_t start;
            int32_t end;
        };
        Interval from{.start = -256,.end = 256};
        Interval to{.start = 0,.end = 512};
        int32_t value = 0;

        int32_t valueRemapped = Jnrlib::RemapValueFromIntervalToInterval(value, from.start, from.end, to.start, to.end);

        EXPECT_EQ(valueRemapped, to.end / 2);

        int32_t valueRemappedRemapped = Jnrlib::RemapValueFromIntervalToInterval(valueRemapped, to.start, to.end, from.start, from.end);
        EXPECT_EQ(valueRemappedRemapped, value);
    }
}

#endif