#include <gtest/gtest.h>

#include "string_to_expr.h"
#include "type_inference.h"

namespace {

class FakeEnv final : public TypeEnvironment<FakeEnv>
{
public:
  void Define(const std::string& name, Type type)
  {
    mVarMap.emplace(name, type);
  }

  const Type* FindVarImpl(const std::string& name) const
  {
    auto it = mVarMap.find(name);
    if (it == mVarMap.end())
      return nullptr;
    else
      return &it->second;
  }

private:
  std::map<std::string, Type> mVarMap;
};

std::string
RunTest(const FakeEnv& env, Expr* rawExprPtr);

} // namespace

TEST(TypeInference, BinaryExpr_UniformFloat_VaryingFloat)
{
  FakeEnv fakeEnv;

  fakeEnv.Define("foo", Type(TypeID::Float, Variability::Uniform));
  fakeEnv.Define("bar", Type(TypeID::Float, Variability::Varying));

  auto expr = StringToExpr("foo * bar");

  auto res = RunTest(fakeEnv, expr.release());

  // requires type coercion

  EXPECT_EQ(res, "failure");
}

TEST(TypeInference, BinaryExpr_FloatScalar_FloatVec)
{
  FakeEnv fakeEnv;

  fakeEnv.Define("foo", Type(TypeID::Vec2));

  auto expr = StringToExpr("2.0 * foo");

  auto res = RunTest(fakeEnv, expr.release());

  EXPECT_EQ(res, "success:unbound:vec2");
}

TEST(TypeInference, BinaryExpr_IntScalar_FloatVec)
{
  FakeEnv fakeEnv;

  fakeEnv.Define("foo", Type(TypeID::Vec2));

  auto expr = StringToExpr("2 * foo");

  auto res = RunTest(fakeEnv, expr.release());

  // requires type coercion

  EXPECT_EQ(res, "failure");
}

TEST(TypeInference, BinaryExpr_Int_Float)
{
  FakeEnv fakeEnv;

  auto expr = StringToExpr("2 * 2.0");

  auto res = RunTest(fakeEnv, expr.release());

  // valid but requires type coercion first

  EXPECT_EQ(res, "failure");
}

TEST(TypeInference, BinaryExpr_Symmetric)
{
  FakeEnv fakeEnv;

  auto expr = StringToExpr("2 * 2");

  auto res = RunTest(fakeEnv, expr.release());

  EXPECT_EQ(res, "success:unbound:int");
}

TEST(TypeInference, MemberExprVec2ix)
{
  FakeEnv fakeEnv;

  fakeEnv.Define("foo", Type(TypeID::Vec2i));

  auto expr = StringToExpr("foo.x");

  auto res = RunTest(fakeEnv, expr.release());

  EXPECT_EQ(res, "success:unbound:int");
}

TEST(TypeInference, MemberExprVec2iy)
{
  FakeEnv fakeEnv;

  fakeEnv.Define("foo", Type(TypeID::Vec2i));

  auto expr = StringToExpr("foo.y");

  auto res = RunTest(fakeEnv, expr.release());

  EXPECT_EQ(res, "success:unbound:int");
}

TEST(TypeInference, MemberExprVec2ixy)
{
  FakeEnv fakeEnv;

  fakeEnv.Define("foo", Type(TypeID::Vec2i));

  auto expr = StringToExpr("foo.xy");

  auto res = RunTest(fakeEnv, expr.release());

  EXPECT_EQ(res, "success:unbound:vec2i");
}

TEST(TypeInference, MemberExprVec2ixyz)
{
  FakeEnv fakeEnv;

  fakeEnv.Define("foo", Type(TypeID::Vec2i));

  auto expr = StringToExpr("foo.xyz");

  auto res = RunTest(fakeEnv, expr.release());

  EXPECT_EQ(res, "failure");
}

TEST(TypeInference, MemberExprVec2ixxyy)
{
  FakeEnv fakeEnv;

  fakeEnv.Define("foo", Type(TypeID::Vec2i, Variability::Uniform));

  auto expr = StringToExpr("foo.xxyy");

  auto res = RunTest(fakeEnv, expr.release());

  EXPECT_EQ(res, "success:uniform:vec4i");
}

TEST(TypeInference, MemberExprVec2x)
{
  FakeEnv fakeEnv;

  fakeEnv.Define("foo", Type(TypeID::Vec2));

  auto expr = StringToExpr("foo.x");

  auto res = RunTest(fakeEnv, expr.release());

  EXPECT_EQ(res, "success:unbound:float");
}

TEST(TypeInference, MemberExprVec2y)
{
  FakeEnv fakeEnv;

  fakeEnv.Define("foo", Type(TypeID::Vec2));

  auto expr = StringToExpr("foo.y");

  auto res = RunTest(fakeEnv, expr.release());

  EXPECT_EQ(res, "success:unbound:float");
}

TEST(TypeInference, MemberExprVec2xy)
{
  FakeEnv fakeEnv;

  fakeEnv.Define("foo", Type(TypeID::Vec2));

  auto expr = StringToExpr("foo.xy");

  auto res = RunTest(fakeEnv, expr.release());

  EXPECT_EQ(res, "success:unbound:vec2");
}

TEST(TypeInference, MemberExprVec2xx)
{
  FakeEnv fakeEnv;

  fakeEnv.Define("foo", Type(TypeID::Vec2));

  auto expr = StringToExpr("foo.xx");

  auto res = RunTest(fakeEnv, expr.release());

  EXPECT_EQ(res, "success:unbound:vec2");
}

TEST(TypeInference, MemberExprVec2yy)
{
  FakeEnv fakeEnv;

  fakeEnv.Define("foo", Type(TypeID::Vec2));

  auto expr = StringToExpr("foo.yy");

  auto res = RunTest(fakeEnv, expr.release());

  EXPECT_EQ(res, "success:unbound:vec2");
}

TEST(TypeInference, MemberExprVec2xyz)
{
  FakeEnv fakeEnv;

  fakeEnv.Define("foo", Type(TypeID::Vec2));

  auto expr = StringToExpr("foo.xyz");

  auto res = RunTest(fakeEnv, expr.release());

  EXPECT_EQ(res, "failure");
}

TEST(TypeInference, MemberExprVec2xxyy)
{
  FakeEnv fakeEnv;

  fakeEnv.Define("foo", Type(TypeID::Vec2, Variability::Varying));

  auto expr = StringToExpr("foo.xxyy");

  auto res = RunTest(fakeEnv, expr.release());

  EXPECT_EQ(res, "success:varying:vec4");
}

TEST(TypeInference, GroupExpr)
{
  FakeEnv fakeEnv;

  std::unique_ptr<IntLiteral> intLiteral(new IntLiteral(0, Location()));

  auto res = RunTest(fakeEnv, new GroupExpr(intLiteral.release(), Location()));

  EXPECT_EQ(res, "success:unbound:int");
}

TEST(TypeInference, VarRef)
{
  FakeEnv fakeEnv;

  fakeEnv.Define("foo", Type(TypeID::Mat4, Variability::Uniform));

  auto result = RunTest(fakeEnv, new VarRef("foo", Location()));

  EXPECT_EQ(result, "success:uniform:mat4");
}

TEST(TypeInference, VarRefFail)
{
  FakeEnv fakeEnv;

  fakeEnv.Define("foo", Type(TypeID::Mat4, Variability::Uniform));

  auto result = RunTest(fakeEnv, new VarRef("bar", Location()));

  EXPECT_EQ(result, "failure");
}

TEST(TypeInference, IntLiteral)
{
  FakeEnv fakeEnv;

  auto result = RunTest(fakeEnv, new IntLiteral(0, Location()));

  EXPECT_EQ(result, "success:unbound:int");
}

TEST(TypeInference, BoolLiteral)
{
  FakeEnv fakeEnv;

  auto result = RunTest(fakeEnv, new BoolLiteral(false, Location()));

  EXPECT_EQ(result, "success:unbound:bool");
}

TEST(TypeInference, FloatLiteral)
{
  FakeEnv fakeEnv;

  auto result = RunTest(fakeEnv, new FloatLiteral(0.0, Location()));
}

namespace {

std::string
RunTest(const FakeEnv& env, Expr* rawExprPtr)
{
  UniqueExprPtr expr(rawExprPtr);

  TypeInferenceEngine<FakeEnv> engine(env);

  expr->AcceptVisitor(engine);

  if (!engine.Success())
    return "failure";

  std::ostringstream outputStream;

  outputStream << "success:";

  auto type = engine.GetType();

  switch (type.GetVariability()) {
    case Variability::Unbound:
      outputStream << "unbound";
      break;
    case Variability::Varying:
      outputStream << "varying";
      break;
    case Variability::Uniform:
      outputStream << "uniform";
      break;
  }

  outputStream << ':';

  switch (type.ID()) {
    case TypeID::Void:
      outputStream << "void";
      break;
    case TypeID::Int:
      outputStream << "int";
      break;
    case TypeID::Bool:
      outputStream << "bool";
      break;
    case TypeID::Float:
      outputStream << "float";
      break;
    case TypeID::Vec2:
      outputStream << "vec2";
      break;
    case TypeID::Vec3:
      outputStream << "vec3";
      break;
    case TypeID::Vec4:
      outputStream << "vec4";
      break;
    case TypeID::Vec2i:
      outputStream << "vec2i";
      break;
    case TypeID::Vec3i:
      outputStream << "vec3i";
      break;
    case TypeID::Vec4i:
      outputStream << "vec4i";
      break;
    case TypeID::Mat2:
      outputStream << "mat2";
      break;
    case TypeID::Mat3:
      outputStream << "mat3";
      break;
    case TypeID::Mat4:
      outputStream << "mat4";
      break;
  }

  return outputStream.str();
}

} // namespace
