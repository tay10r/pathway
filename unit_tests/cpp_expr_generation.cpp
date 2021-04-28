#include <gtest/gtest.h>

#include "cpp_expr_generator.h"

#include "string_to_expr.h"

namespace {

class FakeExprEnv final : public cpp::ExprEnvironment<FakeExprEnv>
{
public:
  auto GetVarOriginImpl(const VarRef& varRef) const
    -> std::optional<cpp::VarOrigin>
  {
    auto it = mVarOriginMap.find(varRef.Identifier());
    if (it == mVarOriginMap.end())
      return {};
    else
      return it->second;
  }

  auto GetGlobalsUsageImpl(const FuncCall& funcCall) const
    -> std::optional<cpp::GlobalsUsage>
  {
    auto it = mGlobalsUsageMap.find(funcCall.Identifier());

    if (it == mGlobalsUsageMap.end())
      return {};

    return it->second;
  }

  bool IsVectorTypeImpl(const Expr&) const { return mExpectingVectorType; }

  void DefineVarOrigin(const std::string& name, cpp::VarOrigin origin)
  {
    mVarOriginMap.emplace(name, origin);
  }

  void DefineGlobalsUsage(const std::string& name, cpp::GlobalsUsage usage)
  {
    mGlobalsUsageMap.emplace(name, usage);
  }

  void ExpectVectorType() { mExpectingVectorType = true; }

private:
  bool mExpectingVectorType = false;

  std::map<std::string, cpp::GlobalsUsage> mGlobalsUsageMap;

  std::map<std::string, cpp::VarOrigin> mVarOriginMap;
};

std::string
RunTest(const FakeExprEnv&, const std::string& str);

} // namespace

TEST(CppExpr, MemberExprAsStructField)
{
  FakeExprEnv env;

  auto out = RunTest(env, "a.memb");

  EXPECT_EQ(out, "a.memb");
}

TEST(CppExpr, MemberExprAsSwizzle)
{
  FakeExprEnv env;

  env.ExpectVectorType();

  auto out = RunTest(env, "a.xzy");

  EXPECT_EQ(out, "swizzle<0, 2, 1>::get(a)");
}

TEST(CppExpr, GroupExpr)
{
  FakeExprEnv env;

  auto out = RunTest(env, "(1 + 2)");

  EXPECT_EQ(out, "(int_type(1) + int_type(2))");
}

TEST(CppExpr, UnaryExpr)
{
  FakeExprEnv env;

  auto out = RunTest(env, "!13");

  EXPECT_EQ(out, "!int_type(13)");
}

TEST(CppExpr, FuncCallRequiringUniformGlobals_2)
{
  FakeExprEnv env;

  cpp::GlobalsUsage fooUsage;

  fooUsage.usesUniformGlobals = true;

  env.DefineGlobalsUsage("foo", fooUsage);

  auto out = RunTest(env, "foo()");

  EXPECT_EQ(out, "foo(frame)");
}

TEST(CppExpr, FuncCallRequiringUniformGlobals)
{
  FakeExprEnv env;

  cpp::GlobalsUsage fooUsage;

  fooUsage.usesUniformGlobals = true;

  env.DefineGlobalsUsage("foo", fooUsage);

  auto out = RunTest(env, "foo(2)");

  EXPECT_EQ(out, "foo(frame, int_type(2))");
}

TEST(CppExpr, FuncCall)
{
  FakeExprEnv env;

  auto out = RunTest(env, "foo(2)");

  EXPECT_EQ(out, "foo(int_type(2))");
}

TEST(CppExpr, MulExpr_2)
{
  FakeExprEnv env;

  env.DefineVarOrigin("foo", cpp::VarOrigin::UniformGlobal);
  env.DefineVarOrigin("bar", cpp::VarOrigin::VaryingGlobal);

  auto out = RunTest(env, "foo * bar");

  EXPECT_EQ(out, "frame.foo * this->bar");
}

TEST(CppExpr, MulExpr)
{
  FakeExprEnv env;

  auto out = RunTest(env, "foo * bar");

  EXPECT_EQ(out, "foo * bar");
}

TEST(CppExpr, BoolLiteral)
{
  FakeExprEnv env;

  auto out = RunTest(env, "false");

  EXPECT_EQ(out, "false");
}

TEST(CppExpr, FloatLiteral)
{
  FakeExprEnv env;

  auto out = RunTest(env, "1.0");

  EXPECT_EQ(out, "float_type(1)");
}

TEST(CppExpr, IntLiteral)
{
  FakeExprEnv env;

  auto out = RunTest(env, "1");

  EXPECT_EQ(out, "int_type(1)");
}

namespace {

std::string
RunTest(const FakeExprEnv& exprEnv, const std::string& src)
{
  auto expr = StringToExpr(src);

  cpp::ExprGenerator<FakeExprEnv> generator(exprEnv);

  expr->AcceptVisitor(generator);

  return generator.String();
}

} // namespace
