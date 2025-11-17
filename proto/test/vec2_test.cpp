#include <gtest/gtest.h>
#include "../vector.h"

template<typename T>
class Vec2Test : public testing::Test
{
public:
    Vec2Test()
    {
    }

    virtual ~Vec2Test()
    {
    }

protected:
    // èâä˙âª
    virtual void SetUp()
    {
    }

    // å„énññ
    virtual void TearDown()
    {
    }
};

TYPED_TEST_SUITE_P(Vec2Test);

TYPED_TEST_P(Vec2Test, ConstructorTest)
{
    Vec2<TypeParam> vec;

    ASSERT_EQ(vec.x_, 0);
    ASSERT_EQ(vec.y_, 0);
    ASSERT_EQ(vec.data[0], 0);
    ASSERT_EQ(vec.data[1], 0);
}

TYPED_TEST_P(Vec2Test, ConstructorTest_2)
{
    Vec2<TypeParam> vec(1, 2);

    ASSERT_EQ(vec.x_, 1);
    ASSERT_EQ(vec.y_, 2);
    ASSERT_EQ(vec.data[0], 1);
    ASSERT_EQ(vec.data[1], 2);
}

TYPED_TEST_P(Vec2Test, SetTest)
{
    Vec2<TypeParam> vec;
    vec.Set(3, 4);

    ASSERT_EQ(vec.x_, 3);
    ASSERT_EQ(vec.y_, 4);
    ASSERT_EQ(vec.data[0], 3);
    ASSERT_EQ(vec.data[1], 4);
}

TYPED_TEST_P(Vec2Test, ZeroTest)
{
    Vec2<TypeParam> vec = Vec2<TypeParam>::ZERO;

    ASSERT_EQ(vec.x_, 0);
    ASSERT_EQ(vec.y_, 0);
    ASSERT_EQ(vec.data[0], 0);
    ASSERT_EQ(vec.data[1], 0);
}

TYPED_TEST_P(Vec2Test, UnitXTest)
{
    Vec2<TypeParam> vec = Vec2<TypeParam>::UNIT_X;

    ASSERT_EQ(vec.x_, 1);
    ASSERT_EQ(vec.y_, 0);
    ASSERT_EQ(vec.data[0], 1);
    ASSERT_EQ(vec.data[1], 0);
}

TYPED_TEST_P(Vec2Test, UnitYTest)
{
    Vec2<TypeParam> vec = Vec2<TypeParam>::UNIT_Y;

    ASSERT_EQ(vec.x_, 0);
    ASSERT_EQ(vec.y_, 1);
    ASSERT_EQ(vec.data[0], 0);
    ASSERT_EQ(vec.data[1], 1);
}

// -------------------------------------
// ÉeÉXÉgìoò^
// -------------------------------------
REGISTER_TYPED_TEST_SUITE_P(
    Vec2Test,
    ConstructorTest,
    ConstructorTest_2,
    SetTest,
    ZeroTest,
    UnitXTest,
    UnitYTest
    );

typedef testing::Types<float, double> Vec2TestSuite;
INSTANTIATE_TYPED_TEST_SUITE_P(Typed, Vec2Test, Vec2TestSuite);
