#include <gtest/gtest.h>
#include "../matrix.h"

template<typename T>
class Mat4Test : public testing::Test
{
public:
    Mat4Test()
    {
    }

    virtual ~Mat4Test()
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

TYPED_TEST_SUITE_P(Mat4Test);

TYPED_TEST_P(Mat4Test, ConstructorTest)
{
    Mat4<TypeParam> mat;

    ASSERT_EQ(mat.m00_, 1); ASSERT_EQ(mat.m01_, 0); ASSERT_EQ(mat.m02_, 0); ASSERT_EQ(mat.m03_, 0);
    ASSERT_EQ(mat.m10_, 0); ASSERT_EQ(mat.m11_, 1); ASSERT_EQ(mat.m12_, 0); ASSERT_EQ(mat.m13_, 0);
    ASSERT_EQ(mat.m20_, 0); ASSERT_EQ(mat.m21_, 0); ASSERT_EQ(mat.m22_, 1); ASSERT_EQ(mat.m23_, 0);
    ASSERT_EQ(mat.m30_, 0); ASSERT_EQ(mat.m31_, 0); ASSERT_EQ(mat.m32_, 0); ASSERT_EQ(mat.m33_, 1);
    ASSERT_EQ(mat.data_[0],  1); ASSERT_EQ(mat.data_[1],  0); ASSERT_EQ(mat.data_[2],  0); ASSERT_EQ(mat.data_[3],  0);
    ASSERT_EQ(mat.data_[4],  0); ASSERT_EQ(mat.data_[5],  1); ASSERT_EQ(mat.data_[6],  0); ASSERT_EQ(mat.data_[7],  0);
    ASSERT_EQ(mat.data_[8],  0); ASSERT_EQ(mat.data_[9],  0); ASSERT_EQ(mat.data_[10], 1); ASSERT_EQ(mat.data_[11], 0);
    ASSERT_EQ(mat.data_[12], 0); ASSERT_EQ(mat.data_[13], 0); ASSERT_EQ(mat.data_[14], 0); ASSERT_EQ(mat.data_[15], 1);
}

TYPED_TEST_P(Mat4Test, ConstructorTest_2)
{
    Mat4<TypeParam> mat(
            1, 2, 3, 4,
            5, 6, 7, 8,
            9, 10, 11, 12,
            13, 14, 15, 16
            );

    ASSERT_EQ(mat.m00_, 1);  ASSERT_EQ(mat.m01_, 2);  ASSERT_EQ(mat.m02_, 3);  ASSERT_EQ(mat.m03_, 4);
    ASSERT_EQ(mat.m10_, 5);  ASSERT_EQ(mat.m11_, 6);  ASSERT_EQ(mat.m12_, 7);  ASSERT_EQ(mat.m13_, 8);
    ASSERT_EQ(mat.m20_, 9);  ASSERT_EQ(mat.m21_, 10); ASSERT_EQ(mat.m22_, 11); ASSERT_EQ(mat.m23_, 12);
    ASSERT_EQ(mat.m30_, 13); ASSERT_EQ(mat.m31_, 14); ASSERT_EQ(mat.m32_, 15); ASSERT_EQ(mat.m33_, 16);
    ASSERT_EQ(mat.data_[0],   1); ASSERT_EQ(mat.data_[1],   2); ASSERT_EQ(mat.data_[2],   3); ASSERT_EQ(mat.data_[3],   4);
    ASSERT_EQ(mat.data_[4],   5); ASSERT_EQ(mat.data_[5],   6); ASSERT_EQ(mat.data_[6],   7); ASSERT_EQ(mat.data_[7],   8);
    ASSERT_EQ(mat.data_[8],   9); ASSERT_EQ(mat.data_[9],  10); ASSERT_EQ(mat.data_[10], 11); ASSERT_EQ(mat.data_[11], 12);
    ASSERT_EQ(mat.data_[12], 13); ASSERT_EQ(mat.data_[13], 14); ASSERT_EQ(mat.data_[14], 15); ASSERT_EQ(mat.data_[15], 16);
}

TYPED_TEST_P(Mat4Test, ConstructorTest_3)
{
    TypeParam arr[16] = {
         1,  2,  3,  4,
         5,  6,  7,  8,
         9, 10, 11, 12,
        13, 14, 15, 16 
    };
    Mat4<TypeParam> mat(arr);

    ASSERT_EQ(mat.m00_, 1);  ASSERT_EQ(mat.m01_, 2);  ASSERT_EQ(mat.m02_, 3);  ASSERT_EQ(mat.m03_, 4);
    ASSERT_EQ(mat.m10_, 5);  ASSERT_EQ(mat.m11_, 6);  ASSERT_EQ(mat.m12_, 7);  ASSERT_EQ(mat.m13_, 8);
    ASSERT_EQ(mat.m20_, 9);  ASSERT_EQ(mat.m21_, 10); ASSERT_EQ(mat.m22_, 11); ASSERT_EQ(mat.m23_, 12);
    ASSERT_EQ(mat.m30_, 13); ASSERT_EQ(mat.m31_, 14); ASSERT_EQ(mat.m32_, 15); ASSERT_EQ(mat.m33_, 16);
    ASSERT_EQ(mat.data_[0],   1); ASSERT_EQ(mat.data_[1],   2); ASSERT_EQ(mat.data_[2],   3); ASSERT_EQ(mat.data_[3],   4);
    ASSERT_EQ(mat.data_[4],   5); ASSERT_EQ(mat.data_[5],   6); ASSERT_EQ(mat.data_[6],   7); ASSERT_EQ(mat.data_[7],   8);
    ASSERT_EQ(mat.data_[8],   9); ASSERT_EQ(mat.data_[9],  10); ASSERT_EQ(mat.data_[10], 11); ASSERT_EQ(mat.data_[11], 12);
    ASSERT_EQ(mat.data_[12], 13); ASSERT_EQ(mat.data_[13], 14); ASSERT_EQ(mat.data_[14], 15); ASSERT_EQ(mat.data_[15], 16);
}

TYPED_TEST_P(Mat4Test, MulTest)
{
    Mat4<TypeParam> a(
         1,  2,  3,  4,
         5,  6,  7,  8,
         9, 10, 11, 12,
        13, 14, 15, 16
    );
    Mat4<TypeParam> b(
         2,  3,  4,  5,
         6,  7,  8,  9,
        10, 11, 12, 13,
        14, 15, 16, 17
    );
    auto c = a * b;

    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(100, c.m00_));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(110, c.m01_));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(120, c.m02_));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(130, c.m03_));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(228, c.m10_));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(254, c.m11_));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(280, c.m12_));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(306, c.m13_));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(356, c.m20_));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(398, c.m21_));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(440, c.m22_));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(482, c.m23_));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(484, c.m30_));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(542, c.m31_));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(600, c.m32_));
    ASSERT_TRUE(Math::IsNearEqual<TypeParam>(658, c.m33_));
}

// -------------------------------------
// Register Test Cases
// -------------------------------------
REGISTER_TYPED_TEST_SUITE_P(
    Mat4Test,
    ConstructorTest,
    ConstructorTest_2,
    ConstructorTest_3,
    MulTest
    );

typedef testing::Types<float, double> Mat4TestSuite;
INSTANTIATE_TYPED_TEST_SUITE_P(Typed, Mat4Test, Mat4TestSuite);
