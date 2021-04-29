#include <gtest/gtest.h>

#include "pathway.h"

using namespace pathway;

TEST(Runtime, VectorAddScalar)
{
  auto a = vector_constructor<3>::make(1, 2, 3);

  auto b = a + 5;

  EXPECT_EQ(b.at<0>(), 6);
  EXPECT_EQ(b.at<1>(), 7);
  EXPECT_EQ(b.at<2>(), 8);
}

TEST(Runtime, VectorSubScalar)
{
  auto a = vector_constructor<3>::make(1, 2, 3);

  auto b = a - 5;

  EXPECT_EQ(b.at<0>(), -4);
  EXPECT_EQ(b.at<1>(), -3);
  EXPECT_EQ(b.at<2>(), -2);
}

TEST(Runtime, VectorMulScalar)
{
  auto a = vector_constructor<3>::make(1, 2, 3);

  auto b = a * 5;

  EXPECT_EQ(b.at<0>(), 5);
  EXPECT_EQ(b.at<1>(), 10);
  EXPECT_EQ(b.at<2>(), 15);
}

TEST(Runtime, VectorDivScalar)
{
  auto a = vector_constructor<3>::make(5, 10, 15);

  auto b = a / 5;

  EXPECT_EQ(b.at<0>(), 1);
  EXPECT_EQ(b.at<1>(), 2);
  EXPECT_EQ(b.at<2>(), 3);
}

TEST(Runtime, VectorConstructorWithVectorArguments)
{
  auto a = vector_constructor<2>::make(2, 3);
  auto b = vector_constructor<3>::make(5, 7, 11);

  auto v = vector_constructor<5>::make(a, b);
  EXPECT_EQ(v.at<0>(), 2);
  EXPECT_EQ(v.at<1>(), 3);
  EXPECT_EQ(v.at<2>(), 5);
  EXPECT_EQ(v.at<3>(), 7);
  EXPECT_EQ(v.at<4>(), 11);
}

TEST(Runtime, DiagonalMatrixConstructor)
{
  // 2 0 0 0
  // 0 2 0 0
  // 0 0 2 0
  auto m = matrix_constructor<3, 4>::make(2);

  EXPECT_EQ(m.at<0>().at<0>(), 2);
  EXPECT_EQ(m.at<0>().at<1>(), 0);
  EXPECT_EQ(m.at<0>().at<2>(), 0);

  EXPECT_EQ(m.at<1>().at<0>(), 0);
  EXPECT_EQ(m.at<1>().at<1>(), 2);
  EXPECT_EQ(m.at<1>().at<2>(), 0);

  EXPECT_EQ(m.at<2>().at<0>(), 0);
  EXPECT_EQ(m.at<2>().at<1>(), 0);
  EXPECT_EQ(m.at<2>().at<2>(), 2);

  EXPECT_EQ(m.at<3>().at<0>(), 0);
  EXPECT_EQ(m.at<3>().at<1>(), 0);
  EXPECT_EQ(m.at<3>().at<2>(), 0);
}

TEST(Runtime, UniformVectorConstructor)
{
  auto v = vector_constructor<3>::make(42);
  EXPECT_EQ(v.at<0>(), 42);
  EXPECT_EQ(v.at<1>(), 42);
  EXPECT_EQ(v.at<2>(), 42);
}

TEST(Runtime, Swizzle)
{
  auto a = make_vec3(3, 5, 7);

  auto b = swizzle<2, 0, 1>::get(a);

  EXPECT_EQ(b.at<0>(), 7);
  EXPECT_EQ(b.at<1>(), 3);
  EXPECT_EQ(b.at<2>(), 5);
}

TEST(Runtime, MatrixAdd)
{
  mat2<int> a;
  a.at<0>() = make_vec2(1, 2);
  a.at<1>() = make_vec2(3, 5);

  mat2<int> b;
  b.at<0>() = make_vec2(7, 11);
  b.at<1>() = make_vec2(13, 17);

  auto c = a + b;
  EXPECT_EQ(c.at<0>().at<0>(), 8);
  EXPECT_EQ(c.at<0>().at<1>(), 13);
  EXPECT_EQ(c.at<1>().at<0>(), 16);
  EXPECT_EQ(c.at<1>().at<1>(), 22);
}

TEST(Runtime, MatrixSub)
{
  mat2<int> a;
  a.at<0>() = make_vec2(1, 2);
  a.at<1>() = make_vec2(3, 5);

  mat2<int> b;
  b.at<0>() = make_vec2(7, 11);
  b.at<1>() = make_vec2(13, 17);

  auto c = a - b;
  EXPECT_EQ(c.at<0>().at<0>(), -6);
  EXPECT_EQ(c.at<0>().at<1>(), -9);
  EXPECT_EQ(c.at<1>().at<0>(), -10);
  EXPECT_EQ(c.at<1>().at<1>(), -12);
}

namespace {

struct FakeUniformData final
{
  float mMagicValue;
};

struct FakeVaryingData final
{
  auto operator()(const FakeUniformData&) const noexcept -> vec3<float>
  {
    return make_vec3(mR, mG, mB);
  }

  void operator()(const FakeUniformData& uniformData,
                  const vec2<float>& uvMin,
                  const vec2<float>& uvMax) noexcept
  {
    float uCenter = (uvMin.at<0>() + uvMax.at<0>()) * 0.5f;
    float vCenter = (uvMin.at<1>() + uvMax.at<1>()) * 0.5f;

    mR = uCenter;
    mG = vCenter;
    mB = uniformData.mMagicValue;
  }

  float mR = 0;
  float mG = 0;
  float mB = 0;
};

} // namespace

TEST(Runtime, FrameSample)
{
  pathway::frame<FakeUniformData, FakeVaryingData, float> frame;

  frame.resize(2, 3);

  frame.get_uniform_data().mMagicValue = 0.3;

  frame.sample_pixels();

  unsigned char rgbBuffer[18];

  frame.encode_rgb(rgbBuffer);

  // y = 0

  EXPECT_EQ(rgbBuffer[0], 63);
  EXPECT_EQ(rgbBuffer[1], 42);
  EXPECT_EQ(rgbBuffer[2], 76);

  EXPECT_EQ(rgbBuffer[3], 191);
  EXPECT_EQ(rgbBuffer[4], 42);
  EXPECT_EQ(rgbBuffer[5], 76);

  // y = 1

  EXPECT_EQ(rgbBuffer[6], 63);
  EXPECT_EQ(rgbBuffer[7], 127);
  EXPECT_EQ(rgbBuffer[8], 76);

  EXPECT_EQ(rgbBuffer[9], 191);
  EXPECT_EQ(rgbBuffer[10], 127);
  EXPECT_EQ(rgbBuffer[11], 76);

  // y = 2

  EXPECT_EQ(rgbBuffer[12], 63);
  EXPECT_EQ(rgbBuffer[13], 212);
  EXPECT_EQ(rgbBuffer[14], 76);

  EXPECT_EQ(rgbBuffer[15], 191);
  EXPECT_EQ(rgbBuffer[16], 212);
  EXPECT_EQ(rgbBuffer[17], 76);
}
