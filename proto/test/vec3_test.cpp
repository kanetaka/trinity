#include <gtest/gtest.h>
#include "../vector.h"

template<typename T>
class Vec3Test : public testing::Test
{
public:
    Vec3Test()
    {
    }

    virtual ~Vec3Test()
    {
    }

protected:
    virtual void SetUp()
    {}

    virtual void TearDown()
    {}

};

TYPED_TEST_SUITE_P(Vec3Test);

TYPED_TEST_P(Vec3Test, ConstructorTest)
{
    Vec3<TypeParam> act;

    ASSERT_EQ(0.0, act.x_);
    ASSERT_EQ(0.0, act.y_);
    ASSERT_EQ(0.0, act.z_);
    ASSERT_EQ(0.0, act.r_);
    ASSERT_EQ(0.0, act.g_);
    ASSERT_EQ(0.0, act.b_);
    ASSERT_EQ(0.0, act.data_[0]);
    ASSERT_EQ(0.0, act.data_[1]);
    ASSERT_EQ(0.0, act.data_[2]);
}

TYPED_TEST_P(Vec3Test, ConstructorTest_2)
{
    Vec3<TypeParam> act(1.0, 2.0, 3.0);

    ASSERT_EQ(1.0, act.x_);
    ASSERT_EQ(2.0, act.y_);
    ASSERT_EQ(3.0, act.z_);
    ASSERT_EQ(1.0, act.r_);
    ASSERT_EQ(2.0, act.g_);
    ASSERT_EQ(3.0, act.b_);
    ASSERT_EQ(1.0, act.data_[0]);
    ASSERT_EQ(2.0, act.data_[1]);
    ASSERT_EQ(3.0, act.data_[2]);
}

TYPED_TEST_P(Vec3Test, SetTest)
{
    Vec3<TypeParam> act;
    act.Set(3.0, 4.0, 5.0);

    ASSERT_EQ(3.0, act.x_);
    ASSERT_EQ(4.0, act.y_);
    ASSERT_EQ(5.0, act.z_);
    ASSERT_EQ(3.0, act.r_);
    ASSERT_EQ(4.0, act.g_);
    ASSERT_EQ(5.0, act.b_);
    ASSERT_EQ(3.0, act.data_[0]);
    ASSERT_EQ(4.0, act.data_[1]);
    ASSERT_EQ(5.0, act.data_[2]);
}

TYPED_TEST_P(Vec3Test, OpAssignmentTest)
{
    Vec3<TypeParam> act;
    Vec3<TypeParam> v(1.0, 2.0, 3.0);
    act = v;

    ASSERT_EQ(1.0, act.x_);
    ASSERT_EQ(2.0, act.y_);
    ASSERT_EQ(3.0, act.z_);
}

TYPED_TEST_P(Vec3Test, OpAddAssignmentTest)
{
    Vec3<TypeParam> act(1.0, 2.0, 3.0);
    Vec3<TypeParam> v(4.0, 5.0, 6.0);
    act += v;

    ASSERT_EQ(5.0, act.x_);
    ASSERT_EQ(7.0, act.y_);
    ASSERT_EQ(9.0, act.z_);
}

TYPED_TEST_P(Vec3Test, OpSubAssignmentTest)
{
    Vec3<TypeParam> act(1.0, 2.0, 3.0);
    Vec3<TypeParam> v(4.0, 6.0, 8.0);
    act -= v;

    ASSERT_EQ(-3.0, act.x_);
    ASSERT_EQ(-4.0, act.y_);
    ASSERT_EQ(-5.0, act.z_);
}

TYPED_TEST_P(Vec3Test, OpMulAssignmentTest)
{
    Vec3<TypeParam> act(1.0, 2.0, 3.0);
    Vec3<TypeParam> v(4.0, 6.0, 8.0);
    act *= v;

    ASSERT_EQ(4.0, act.x_);
    ASSERT_EQ(12.0, act.y_);
    ASSERT_EQ(24.0, act.z_);
}

TYPED_TEST_P(Vec3Test, OpMulAssignmentTest_2)
{
    Vec3<TypeParam> act(1.0, 2.0, 3.0);
    act *= 4.0;

    ASSERT_EQ(4.0, act.x_);
    ASSERT_EQ(8.0, act.y_);
    ASSERT_EQ(12.0, act.z_);
}

TYPED_TEST_P(Vec3Test, OpDivAssignmentTest)
{
    Vec3<TypeParam> act(1.0, 2.0, 3.0);
    act /= 4.0;

    ASSERT_EQ(1.0/4.0, act.x_);
    ASSERT_EQ(2.0/4.0, act.y_);
    ASSERT_EQ(3.0/4.0, act.z_);
}

TYPED_TEST_P(Vec3Test, OpPlusTest)
{
    Vec3<TypeParam> v(1.0, 2.0, 3.0);
    auto act = +v;

    ASSERT_EQ(1.0, act.x_);
    ASSERT_EQ(2.0, act.y_);
    ASSERT_EQ(3.0, act.z_);
}

TYPED_TEST_P(Vec3Test, OpMinusTest)
{
    Vec3<TypeParam> v(1.0, 2.0, 3.0);
    auto act = -v;

    ASSERT_EQ(-1.0, act.x_);
    ASSERT_EQ(-2.0, act.y_);
    ASSERT_EQ(-3.0, act.z_);
}

TYPED_TEST_P(Vec3Test, OpAddTest)
{
    Vec3<TypeParam> a(1.0, 2.0, 3.0);
    Vec3<TypeParam> b(-3.0, 2.0, 5.0);
    auto act =  a + b;

    ASSERT_EQ(-2.0, act.x_);
    ASSERT_EQ( 4.0, act.y_);
    ASSERT_EQ( 8.0, act.z_);
}

TYPED_TEST_P(Vec3Test, OpSubTest)
{
    Vec3<TypeParam> a(1.0, 2.0, 3.0);
    Vec3<TypeParam> b(-3.0, 2.0, 5.0);
    auto act =  a - b;

    ASSERT_EQ(4.0 , act.x_);
    ASSERT_EQ(0.0 , act.y_);
    ASSERT_EQ(-2.0, act.z_);
}

TYPED_TEST_P(Vec3Test, OpMulTest)
{
    Vec3<TypeParam> a(1.0, 2.0, 3.0);
    Vec3<TypeParam> b(-3.0, 2.0, 5.0);
    auto act =  a * b;

    ASSERT_EQ(-3.0, act.x_);
    ASSERT_EQ(4.0 , act.y_);
    ASSERT_EQ(15.0, act.z_);
}

TYPED_TEST_P(Vec3Test, OpMulTest_2)
{
    Vec3<TypeParam> a(1.0, 2.0, 3.0);
    auto act =  a * 4;

    ASSERT_EQ(4.0 , act.x_);
    ASSERT_EQ(8.0 , act.y_);
    ASSERT_EQ(12.0, act.z_);
}

TYPED_TEST_P(Vec3Test, OpMulTest_3)
{
    Vec3<TypeParam> a(1.0, 2.0, 3.0);
    auto act =  4 * a;

    ASSERT_EQ(4.0 , act.x_);
    ASSERT_EQ(8.0 , act.y_);
    ASSERT_EQ(12.0, act.z_);
}

TYPED_TEST_P(Vec3Test, OpDivTest)
{
    Vec3<TypeParam> a(1.0, 2.0, 3.0);
    auto act =  a / 4.0;

    ASSERT_EQ(1.0/4.0, act.x_);
    ASSERT_EQ(2.0/4.0, act.y_);
    ASSERT_EQ(3.0/4.0, act.z_);
}

TYPED_TEST_P(Vec3Test, ZeroTest)
{
    Vec3<TypeParam> act = Vec3<TypeParam>::ZERO;

    ASSERT_EQ(0.0, act.x_);
    ASSERT_EQ(0.0, act.y_);
    ASSERT_EQ(0.0, act.z_);
    ASSERT_EQ(0.0, act.data_[0]);
    ASSERT_EQ(0.0, act.data_[1]);
    ASSERT_EQ(0.0, act.data_[2]);
}

TYPED_TEST_P(Vec3Test, UnitXTest)
{
    Vec3<TypeParam> act = Vec3<TypeParam>::UNIT_X;

    ASSERT_EQ(1.0, act.x_);
    ASSERT_EQ(0.0, act.y_);
    ASSERT_EQ(0.0, act.z_);
    ASSERT_EQ(1.0, act.data_[0]);
    ASSERT_EQ(0.0, act.data_[1]);
    ASSERT_EQ(0.0, act.data_[2]);
}

TYPED_TEST_P(Vec3Test, UnitYTest)
{
    Vec3<TypeParam> act = Vec3<TypeParam>::UNIT_Y;

    ASSERT_EQ(0.0, act.x_);
    ASSERT_EQ(1.0, act.y_);
    ASSERT_EQ(0.0, act.z_);
    ASSERT_EQ(0.0, act.data_[0]);
    ASSERT_EQ(1.0, act.data_[1]);
    ASSERT_EQ(0.0, act.data_[2]);
}

TYPED_TEST_P(Vec3Test, UnitZTest)
{
    Vec3<TypeParam> act = Vec3<TypeParam>::UNIT_Z;

    ASSERT_EQ(0.0, act.x_);
    ASSERT_EQ(0.0, act.y_);
    ASSERT_EQ(1.0, act.z_);
    ASSERT_EQ(0.0, act.data_[0]);
    ASSERT_EQ(0.0, act.data_[1]);
    ASSERT_EQ(1.0, act.data_[2]);
}

// -------------------------------------
// Register Test Cases
// -------------------------------------
REGISTER_TYPED_TEST_SUITE_P(
    Vec3Test,
    ConstructorTest,
    ConstructorTest_2,
    SetTest,
    OpAssignmentTest,
    OpAddAssignmentTest,
    OpSubAssignmentTest,
    OpMulAssignmentTest,
    OpMulAssignmentTest_2,
    OpDivAssignmentTest,
    OpPlusTest,
    OpMinusTest,
    OpAddTest,
    OpSubTest,
    OpMulTest,
    OpMulTest_2,
    OpMulTest_3,
    OpDivTest,
    ZeroTest,
    UnitXTest,
    UnitYTest,
    UnitZTest
    );

typedef testing::Types<float, double> Vec3TestSuite;
INSTANTIATE_TYPED_TEST_SUITE_P(Typed, Vec3Test, Vec3TestSuite);
