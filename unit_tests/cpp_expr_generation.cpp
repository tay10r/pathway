#include <gtest/gtest.h>

#include "cpp_expr_generator.h"

#include "string_to_expr.h"

namespace {

std::string
RunTest(const UniqueExprPtr& expr);

} // namespace

TEST(CppExpr, MulExprUniform)
{
  auto out = RunTest(StringToExpr("foo * bar"));

  EXPECT_EQ(out,
            "i32: foo * bar\n"
            "i64: foo * bar\n"
            "sse2_i32x4: foo * bar\n");
}

TEST(CppExpr, MulExprVarying)
{
  auto out = RunTest(StringToExpr("foo * bar"));

  EXPECT_EQ(out,
            "i32: foo * bar\n"
            "i64: foo * bar\n"
            "sse2_i32x4: _mm_mul_ps(foo, bar)\n");
}

TEST(CppExpr, IntLiteral)
{
  auto out = RunTest(StringToExpr("1"));
  EXPECT_EQ(out,
            "i32: 1l\n"
            "i64: 1ll\n"
            "sse2_i32x4: 1l\n");
}

namespace {

std::string
RunTest(const UniqueExprPtr& expr)
{
  std::vector<std::shared_ptr<cpp::DataMode>> dataModes;

  dataModes.emplace_back(new cpp::I32_DataMode());
  dataModes.emplace_back(new cpp::I64_DataMode());
  dataModes.emplace_back(new cpp::SSE2_I32_DataMode());

  std::ostringstream outputStream;

  for (auto dataMode : dataModes) {

    cpp::ExprGenerator generator(dataMode);

    expr->AcceptVisitor(generator);

    outputStream << dataMode->ModeID() << ": ";

    outputStream << generator.String();

    outputStream << "\n";
  }

  return outputStream.str();
}

} // namespace
