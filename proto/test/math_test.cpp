#include <gtest/gtest.h>
#include <typeinfo>
#include "../Math.h"

template<typename T>
class MathTest : public testing::Test
{
public:
    MathTest()
    {
    }

    virtual ~MathTest()
    {
    }

protected:
    // 初期化
    virtual void SetUp()
    {}

    // 後始末
    virtual void TearDown()
    {}

};

TYPED_TEST_SUITE_P(MathTest);

TYPED_TEST_P(MathTest, IsNearZeroTest)
{
    ASSERT_TRUE(Math::IsNearZero<TypeParam>(0.000001));
}

TYPED_TEST_P(MathTest, IsNearZeroTest_2)
{
    TypeParam exp = 0.0009;

    if (typeid(TypeParam).name() == "float") {
        ASSERT_TRUE(Math::IsNearZero<TypeParam>(exp));
    }
    else if (typeid(TypeParam).name() == "double") {
        ASSERT_FALSE(Math::IsNearZero<TypeParam>(exp));
    }
}

TYPED_TEST_P(MathTest, IsNearZeroTest_3)
{
    TypeParam exp = 0.0011;
    ASSERT_FALSE(Math::IsNearZero<TypeParam>(exp));
}

TYPED_TEST_P(MathTest, IsNearEqualTest)
{
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(1.0, 1.000001));
}

TYPED_TEST_P(MathTest, IsNearEqualTest_2)
{
    ASSERT_FALSE(Math::IsNearEqual<TypeParam>(1.0, 1.001));
}

// -------------------------------------
// テスト登録
// -------------------------------------
REGISTER_TYPED_TEST_SUITE_P(MathTest,
    IsNearZeroTest,
    IsNearZeroTest_2,
    IsNearZeroTest_3,
    IsNearEqualTest,
    IsNearEqualTest_2
    );

typedef testing::Types<float, double> MathTestCase;
INSTANTIATE_TYPED_TEST_SUITE_P(Typed, MathTest, MathTestCase);
