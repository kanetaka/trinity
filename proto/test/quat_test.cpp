#include <gtest/gtest.h>
#include "../math.h"
#include "../quaternion.h"

template<typename T>
class QuatTest : public testing::Test
{
public:
    QuatTest()
    {
    }

    virtual ~QuatTest()
    {
    }

protected:
    virtual void SetUp()
    {}

    virtual void TearDown()
    {}

};

TYPED_TEST_SUITE_P(QuatTest);

TYPED_TEST_P(QuatTest, ConstructorTest)
{
    Quat<TypeParam> q;

    ASSERT_EQ(q.x_, 0);
    ASSERT_EQ(q.y_, 0);
    ASSERT_EQ(q.z_, 0);
    ASSERT_EQ(q.w_, 1);
    ASSERT_EQ(q.data_[0], 0);
    ASSERT_EQ(q.data_[1], 0);
    ASSERT_EQ(q.data_[2], 0);
    ASSERT_EQ(q.data_[3], 1);
}

TYPED_TEST_P(QuatTest, ConstructorTest_2)
{
    Quat<TypeParam> q(1, 2, 3, 4);

    ASSERT_EQ(q.x_, 1);
    ASSERT_EQ(q.y_, 2);
    ASSERT_EQ(q.z_, 3);
    ASSERT_EQ(q.w_, 4);
    ASSERT_EQ(q.data_[0], 1);
    ASSERT_EQ(q.data_[1], 2);
    ASSERT_EQ(q.data_[2], 3);
    ASSERT_EQ(q.data_[3], 4);
}

TYPED_TEST_P(QuatTest, ConstructorTest_3)
{
    Quat<TypeParam> q(Vec3<TypeParam>(1.0, 2.0, 3.0), Math::PI<TypeParam>);

    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(q.x_, 1.0));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(q.y_, 2.0));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(q.z_, 3.0));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(q.w_, 0.0));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(q.data_[0], 1.0));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(q.data_[1], 2.0));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(q.data_[2], 3.0));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(q.data_[3], 0.0));
}

TYPED_TEST_P(QuatTest, SetTest)
{
    Quat<TypeParam> q;
    q.Set(1, 2, 3, 4);

    ASSERT_EQ(q.x_, 1);
    ASSERT_EQ(q.y_, 2);
    ASSERT_EQ(q.z_, 3);
    ASSERT_EQ(q.w_, 4);
    ASSERT_EQ(q.data_[0], 1);
    ASSERT_EQ(q.data_[1], 2);
    ASSERT_EQ(q.data_[2], 3);
    ASSERT_EQ(q.data_[3], 4);
}
// -------------------------------------
// Register Test Cases
// -------------------------------------
REGISTER_TYPED_TEST_SUITE_P(
    QuatTest,
    ConstructorTest,
    ConstructorTest_2,
    ConstructorTest_3,
    SetTest
    );

typedef testing::Types<float, double> QuatTestSuite;
INSTANTIATE_TYPED_TEST_SUITE_P(Typed, QuatTest, QuatTestSuite);
