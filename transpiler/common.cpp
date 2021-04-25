#include "common.h"

#include <ostream>

std::ostream&
operator<<(std::ostream& os, const location& l)
{
  return os << l.first_line << ':' << l.first_column;
}

std::ostream&
operator<<(std::ostream& os, TypeID typeID)
{
  switch (typeID) {
    case TypeID::Void:
      return os << "void";
    case TypeID::Bool:
      return os << "bool";
    case TypeID::Int:
      return os << "int";
    case TypeID::Float:
      return os << "float";
    case TypeID::Vec2:
      return os << "vec2";
    case TypeID::Vec3:
      return os << "vec3";
    case TypeID::Vec4:
      return os << "vec4";
    case TypeID::Vec2i:
      return os << "vec2i";
    case TypeID::Vec3i:
      return os << "vec3i";
    case TypeID::Vec4i:
      return os << "vec4i";
    case TypeID::Mat2:
      return os << "mat2";
    case TypeID::Mat3:
      return os << "mat3";
    case TypeID::Mat4:
      return os << "mat4";
  }

  return os;
}

std::ostream&
operator<<(std::ostream& os, Variability variability)
{
  switch (variability) {
    case Variability::Unbound:
      return os << "unbound";
    case Variability::Varying:
      return os << "varying";
    case Variability::Uniform:
      return os << "uniform";
  }

  return os;
}

std::ostream&
operator<<(std::ostream& os, const Type& type)
{
  return os << type.GetVariability() << ' ' << type.ID();
}

Type
var_ref::GetType() const
{
  assert(this->resolved_var != nullptr);

  return *this->resolved_var->mType;
}

swizzle::swizzle(const std::string& pattern)
{
  for (auto c : pattern) {
    switch (c) {
      case 'r':
      case 'x':
        this->member_indices.emplace_back(0);
        break;
      case 'g':
      case 'y':
        this->member_indices.emplace_back(1);
        break;
      case 'b':
      case 'z':
        this->member_indices.emplace_back(2);
        break;
      case 'a':
      case 'w':
        this->member_indices.emplace_back(3);
        break;
      default:
        this->error_flag = true;
        break;
    }
  }

  this->error_flag |= this->member_indices.size() > 4;
}

namespace {

Type
get_float_vector_member_type(const std::string& pattern)
{
  swizzle s(pattern);

  assert(!s.error_flag);

  switch (s.member_indices.size()) {
    case 1:
      break;
    case 2:
      return TypeID::Vec2;
    case 3:
      return TypeID::Vec3;
    case 4:
      return TypeID::Vec4;
  }

  return TypeID::Float;
}

Type
get_int_vector_member_type(const std::string& pattern)
{
  swizzle s(pattern);

  assert(!s.error_flag);

  switch (s.member_indices.size()) {
    case 1:
      break;
    case 2:
      return TypeID::Vec2i;
    case 3:
      return TypeID::Vec3i;
    case 4:
      return TypeID::Vec4i;
  }

  return TypeID::Int;
}

} // namespace

Type
member_expr::GetType() const
{
  switch (base_expr->GetType().ID()) {
    case TypeID::Void:
    case TypeID::Int:
    case TypeID::Float:
    case TypeID::Bool:
    case TypeID::Mat2:
    case TypeID::Mat3:
    case TypeID::Mat4:
      // These don't have members and this should not be reachable because it
      // will trigger an error.
      assert(false);
      return base_expr->GetType();
    case TypeID::Vec2:
    case TypeID::Vec3:
    case TypeID::Vec4:
      return get_float_vector_member_type(*this->member_name.identifier);
    case TypeID::Vec2i:
    case TypeID::Vec3i:
    case TypeID::Vec4i:
      return get_int_vector_member_type(*this->member_name.identifier);
  }

  assert(false);

  return base_expr->GetType();
}

namespace {

class CommonTypeInfo final
{
public:
  constexpr CommonTypeInfo(TypeID typeA, TypeID typeB, TypeID commonType)
    : mTypeA(typeA)
    , mTypeB(typeB)
    , mCommonType(commonType)
  {}

  constexpr bool Match(TypeID a, TypeID b) const noexcept
  {
    if ((mTypeA == a) && (mTypeB == b))
      return true;

    if ((mTypeA == b) && (mTypeB == a))
      return true;

    return false;
  }

  constexpr TypeID CommonType() const noexcept { return mCommonType; }

private:
  TypeID mTypeA;
  TypeID mTypeB;
  TypeID mCommonType;
};

const std::array<CommonTypeInfo, 11> gCommonTypeTable{
  { // basic conversions
    CommonTypeInfo(TypeID::Int, TypeID::Bool, TypeID::Int),
    CommonTypeInfo(TypeID::Float, TypeID::Int, TypeID::Float),
    // int and vectors
    CommonTypeInfo(TypeID::Int, TypeID::Vec2i, TypeID::Vec2i),
    CommonTypeInfo(TypeID::Int, TypeID::Vec3i, TypeID::Vec3i),
    CommonTypeInfo(TypeID::Int, TypeID::Vec4i, TypeID::Vec4i),
    /// float and vectors
    CommonTypeInfo(TypeID::Float, TypeID::Vec2, TypeID::Vec2),
    CommonTypeInfo(TypeID::Float, TypeID::Vec3, TypeID::Vec3),
    CommonTypeInfo(TypeID::Float, TypeID::Vec4, TypeID::Vec4),
    // float and matrices
    CommonTypeInfo(TypeID::Float, TypeID::Mat2, TypeID::Mat2),
    CommonTypeInfo(TypeID::Float, TypeID::Mat3, TypeID::Mat3),
    CommonTypeInfo(TypeID::Float, TypeID::Mat4, TypeID::Mat4) }
};

} // namespace

Type
binary_expr::GetType() const
{
  if (left->GetType() == right->GetType())
    return left->GetType();

  for (const auto& commonTypeEntry : gCommonTypeTable) {
    if (commonTypeEntry.Match(left->GetType().ID(), right->GetType().ID()))
      return Type(commonTypeEntry.CommonType());
  }

  assert(false);
}

bool
Var::IsVaryingGlobal() const
{
  return mIsGlobal && mType->IsVaryingOrUnbound();
}

bool
Var::IsUniformGlobal() const
{
  return mIsGlobal && mType->IsUniform();
}

namespace {

class ExprGlobalStateReferenceChecker final : public expr_visitor
{
public:
  bool ReferencesFrameState() const noexcept { return mReferencesFrameState; }
  bool ReferencesPixelState() const noexcept { return mReferencesPixelState; }

  void visit(const bool_literal&) override {}
  void visit(const int_literal&) override {}
  void visit(const float_literal&) override {}

  void visit(const binary_expr& binaryExpr) override
  {
    binaryExpr.Recurse(*this);
  }

  void visit(const unary_expr& unaryExpr) override { unaryExpr.Recurse(*this); }

  void visit(const group_expr& groupExpr) override { groupExpr.Recurse(*this); }

  void visit(const var_ref& varRef) override
  {
    if (!varRef.HasResolvedVar())
      return;

    const auto& var = varRef.ResolvedVar();

    if (!var.IsGlobal())
      return;

    switch (var.GetVariability()) {
      case Variability::Unbound:
      case Variability::Varying:
        mReferencesPixelState |= true;
        break;
      case Variability::Uniform:
        mReferencesFrameState |= true;
        break;
    }
  }

  void visit(const type_constructor& typeConstructor) override
  {
    typeConstructor.Recurse(*this);
  }

  void visit(const member_expr& memberExpr) override
  {
    memberExpr.Recurse(*this);
  }

private:
  bool mReferencesFrameState = false;
  bool mReferencesPixelState = false;
};

class StmtGlobalStateReferenceChecker final : public stmt_visitor
{
public:
  bool ReferencesFrameState() const noexcept { return mReferencesFrameState; }

  bool ReferencesPixelState() const noexcept { return mReferencesPixelState; }

  void visit(const AssignmentStmt& assignmentStmt) override
  {
    CheckExpr(assignmentStmt.LValue());
    CheckExpr(assignmentStmt.RValue());
  }

  void visit(const decl_stmt& s) override
  {
    if (s.v->init_expr)
      CheckExpr(*s.v->init_expr);
  }

  void visit(const return_stmt& s) override { CheckExpr(*s.return_value); }

  void visit(const compound_stmt& s) override
  {
    for (const auto& inner_stmt : *s.stmts)
      inner_stmt->accept(*this);
  }

private:
  void CheckExpr(const expr& e)
  {
    ExprGlobalStateReferenceChecker checker;

    e.accept(checker);

    mReferencesFrameState |= checker.ReferencesFrameState();
    mReferencesPixelState |= checker.ReferencesPixelState();
  }

  bool mReferencesFrameState = false;
  bool mReferencesPixelState = false;
};

} // namespace

bool
Func::ReferencesFrameState() const
{
  StmtGlobalStateReferenceChecker checker;

  this->mBody->accept(checker);

  return checker.ReferencesFrameState();
}

bool
Func::ReferencesPixelState() const
{
  StmtGlobalStateReferenceChecker checker;

  this->mBody->accept(checker);

  return checker.ReferencesPixelState();
}

bool
Func::ReferencesGlobalState() const
{
  return ReferencesFrameState() || ReferencesPixelState();
}

bool
Func::IsEntryPoint() const
{
  return IsPixelSampler() || IsPixelEncoder();
}

bool
Func::IsPixelSampler() const
{
  return *this->mName.identifier == "SamplePixel";
}

bool
Func::IsPixelEncoder() const
{
  return *this->mName.identifier == "EncodePixel";
}

void
Program::AppendGlobalVar(Var* globalVar)
{
  globalVar->MarkAsGlobal();

  mGlobalVars.emplace_back(globalVar);

  switch (globalVar->GetVariability()) {
    case Variability::Uniform:
      mUniformGlobalVars.emplace_back(globalVar);
      break;
    case Variability::Varying:
    case Variability::Unbound:
      mVaryingGlobalVars.emplace_back(globalVar);
      break;
  }
}
