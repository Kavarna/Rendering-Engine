#include "gtest/gtest.h"
#include "Jnrlib.h"

namespace
{
    TEST(CompileTimeTests, CountParameters)
    {
        EXPECT_EQ(Jnrlib::CountParameters<>::value, 0);

        EXPECT_EQ(Jnrlib::CountParameters<float>::value, 1);
        EXPECT_EQ(Jnrlib::CountParameters<std::string>::value, 1);
        EXPECT_EQ(Jnrlib::CountParameters<const char*>::value, 1);

        EXPECT_EQ((Jnrlib::CountParameters<const char*, std::string>::value), 2);

        EXPECT_EQ((Jnrlib::CountParameters<const char*, std::string, float>::value), 3);
        EXPECT_EQ((Jnrlib::CountParameters<const char*, std::string, float, int, unsigned int>::value), 5);
        EXPECT_EQ((Jnrlib::CountParameters<const char*, std::string, float, unsigned int, double>::value), 5);
        
        struct Struct1
        { };
        EXPECT_EQ((Jnrlib::CountParameters<Struct1>::value), 1);

        struct Struct2
        {
            float a;
        };
        EXPECT_EQ((Jnrlib::CountParameters<Struct2>::value), 1);
        union Union1
        {

        };
        EXPECT_EQ((Jnrlib::CountParameters<Union1>::value), 1);
        union Union2
        {
            float a;
        };
        EXPECT_EQ((Jnrlib::CountParameters<Union2>::value), 1);
    }

    TEST(CompileTimeTests, SingletoneUnicity)
    {
        class Singletone : public Jnrlib::ISingletone<Singletone>
        {
            MAKE_SINGLETONE_CAPABLE(Singletone);
        private:
            Singletone() = default;
            ~Singletone() = default;
        public:
            float value;
        };

        auto sg1 = Singletone::Get();
        sg1->value = 3.0f;

        auto sg2 = Singletone::Get();
        EXPECT_EQ(sg1, sg2);
        EXPECT_EQ(sg1->value, sg2->value);
    }
}