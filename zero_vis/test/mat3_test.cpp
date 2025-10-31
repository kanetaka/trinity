#include <gtest/gtest.h>
#include "../matrix.h"

template<typename T>
class Mat3Test : public testing::Test
{
public:
    Mat3Test()
    {
    }

    virtual ~Mat3Test()
    {
    }

protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TYPED_TEST_SUITE_P(Mat3Test);

TYPED_TEST_P(Mat3Test, ConstructorTest)
{
    Mat3<TypeParam> mat;

    ASSERT_EQ(mat.m00_, 1); ASSERT_EQ(mat.m01_, 0); ASSERT_EQ(mat.m02_, 0);
    ASSERT_EQ(mat.m10_, 0); ASSERT_EQ(mat.m11_, 1); ASSERT_EQ(mat.m12_, 0);
    ASSERT_EQ(mat.m20_, 0); ASSERT_EQ(mat.m21_, 0); ASSERT_EQ(mat.m22_, 1);
    ASSERT_EQ(mat.data_[0], 1); ASSERT_EQ(mat.data_[1], 0); ASSERT_EQ(mat.data_[2], 0);
    ASSERT_EQ(mat.data_[3], 0); ASSERT_EQ(mat.data_[4], 1); ASSERT_EQ(mat.data_[5], 0);
    ASSERT_EQ(mat.data_[6], 0); ASSERT_EQ(mat.data_[7], 0); ASSERT_EQ(mat.data_[8], 1);
}

TYPED_TEST_P(Mat3Test, ConstructorTest_2)
{
    Mat3<TypeParam> mat(
            1, 2, 3,
            4, 5, 6,
            7, 8, 9);
    ASSERT_EQ(mat.m00_, 1); ASSERT_EQ(mat.m01_, 2); ASSERT_EQ(mat.m02_, 3);
    ASSERT_EQ(mat.m10_, 4); ASSERT_EQ(mat.m11_, 5); ASSERT_EQ(mat.m12_, 6);
    ASSERT_EQ(mat.m20_, 7); ASSERT_EQ(mat.m21_, 8); ASSERT_EQ(mat.m22_, 9);
    ASSERT_EQ(mat.data_[0], 1); ASSERT_EQ(mat.data_[1], 2); ASSERT_EQ(mat.data_[2], 3);
    ASSERT_EQ(mat.data_[3], 4); ASSERT_EQ(mat.data_[4], 5); ASSERT_EQ(mat.data_[5], 6);
    ASSERT_EQ(mat.data_[6], 7); ASSERT_EQ(mat.data_[7], 8); ASSERT_EQ(mat.data_[8], 9);
}

TYPED_TEST_P(Mat3Test, ConstructorTest_3)
{
    TypeParam arr[9] = {
        1, 2, 3 ,
        4, 5, 6 ,
        7, 8, 9 ,
    };
    Mat3<TypeParam> mat(arr);

    ASSERT_EQ(mat.m00_, 1); ASSERT_EQ(mat.m01_, 2); ASSERT_EQ(mat.m02_, 3);
    ASSERT_EQ(mat.m10_, 4); ASSERT_EQ(mat.m11_, 5); ASSERT_EQ(mat.m12_, 6);
    ASSERT_EQ(mat.m20_, 7); ASSERT_EQ(mat.m21_, 8); ASSERT_EQ(mat.m22_, 9);
    ASSERT_EQ(mat.data_[0], 1); ASSERT_EQ(mat.data_[1], 2); ASSERT_EQ(mat.data_[2], 3);
    ASSERT_EQ(mat.data_[3], 4); ASSERT_EQ(mat.data_[4], 5); ASSERT_EQ(mat.data_[5], 6);
    ASSERT_EQ(mat.data_[6], 7); ASSERT_EQ(mat.data_[7], 8); ASSERT_EQ(mat.data_[8], 9);
}

TYPED_TEST_P(Mat3Test, MulTest)
{
    Mat3<TypeParam> a(
        1, 2, 3,
        4, 5, 6,
        7, 8, 9);
    Mat3<TypeParam> b(
        2, 3, 4,
        5, 6, 7,
        8, 9, 10);
    auto c = a * b;

    ASSERT_TRUE(Math::IsNearEqual<TypeParam>( 36, c.m00_));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>( 42, c.m01_));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>( 48, c.m02_));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>( 81, c.m10_));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>( 96, c.m11_));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(111, c.m12_));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(126, c.m20_));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(150, c.m21_));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(174, c.m22_));
}

TYPED_TEST_P(Mat3Test, MulTest_2)
{
    Mat3<TypeParam> a(
         1.5, -0.2, 3.3,
         0.2,  0.1, 0.0,
        -7.0,  8.2, 4.2);
    Mat3<TypeParam> b(
        -0.2,  0.3, 2.8,
         5.5,  4.5, 7.9,
         6.3, -9.9, 0.1);
    auto c = a * b;

    ASSERT_TRUE(Math::IsNearEqual<TypeParam>( 19.39, c.m00_));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(-33.12, c.m01_));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(  2.95, c.m02_));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(  0.51, c.m10_));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(  0.51, c.m11_));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(  1.35, c.m12_));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>( 72.96, c.m20_));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>( -6.78, c.m21_));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(  45.6, c.m22_));
}

// -------------------------------------
// Register Test Cases
// -------------------------------------
REGISTER_TYPED_TEST_SUITE_P(
    Mat3Test,
    ConstructorTest,
    ConstructorTest_2,
    ConstructorTest_3,
    MulTest,
    MulTest_2
    );

typedef testing::Types<float, double> Mat3TestSuite;
INSTANTIATE_TYPED_TEST_SUITE_P(Typed, Mat3Test, Mat3TestSuite);
