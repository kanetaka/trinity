#include <gtest/gtest.h>
#include <typeinfo>
#include "../Random.h"

template<typename T>
class RandomTest : public testing::Test
{
public:
    RandomTest()
    {
    }

    virtual ~RandomTest()
    {
    }

protected:
    // 初期化
    virtual void SetUp()
    {
    }

    // 後始末
    virtual void TearDown()
    {
    }
};

TYPED_TEST_SUITE_P(RandomTest);

TYPED_TEST_P(RandomTest, GetValueTest)
{
    Random::Init();
    TypeParam min = static_cast<TypeParam>(0.0);
    TypeParam max = static_cast<TypeParam>(1.0);

    for (int i = 0; i < 10000; ++i) {
        auto val = Random::GetValue<TypeParam>();
        ASSERT_TRUE((val >= min) && (val <= max));
    }
}

TYPED_TEST_P(RandomTest, GetValueRangeTest)
{
    Random::Init();
    auto min = static_cast<TypeParam>(5.0);
    auto max = static_cast<TypeParam>(100.0);

    for (int i = 0; i < 10000; ++i) {
        auto val = Random::GetValueRange(min, max);
        ASSERT_TRUE((val >= min) && (val <= max));
    }
}


// -------------------------------------
// テスト登録
// -------------------------------------
REGISTER_TYPED_TEST_SUITE_P(RandomTest,
    GetValueTest,
    GetValueRangeTest
);

typedef testing::Types<int, float, double> RandomTestCase;
INSTANTIATE_TYPED_TEST_SUITE_P(Typed, RandomTest, RandomTestCase);
