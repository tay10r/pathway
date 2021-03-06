#include "expr.h"

#include "decl.h"

auto
VarRef::GetType() const -> std::optional<Type>
{
  if (!mResolvedVar)
    return {};

  return mResolvedVar->GetType();
}

auto
Swizzle::Make(const std::string& pattern, size_t vecSize)
  -> std::optional<Swizzle>
{
  Swizzle swizzle;

  for (auto c : pattern) {
    switch (c) {
      case 'r':
      case 'x':
        swizzle.mIndices.emplace_back(0);
        break;
      case 'g':
      case 'y':
        swizzle.mIndices.emplace_back(1);
        break;
      case 'b':
      case 'z':
        swizzle.mIndices.emplace_back(2);
        break;
      case 'a':
      case 'w':
        swizzle.mIndices.emplace_back(3);
        break;
      default:
        return {};
    }
  }

  if (swizzle.mIndices.size() > 4)
    return {};

  for (auto index : swizzle.mIndices) {
    if (index >= vecSize)
      return {};
  }

  return swizzle;
}

namespace {

Type
get_float_vector_member_type(const std::string& pattern, size_t vecSize)
{
  auto swizzle = Swizzle::Make(pattern, vecSize).value();

  switch (swizzle.Size()) {
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
get_int_vector_member_type(const std::string& pattern, size_t vecSize)
{
  auto swizzle = Swizzle::Make(pattern, vecSize).value();

  switch (swizzle.Size()) {
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

auto
MemberExpr::GetType() const -> std::optional<Type>
{
  auto baseType = mBaseExpr->GetType();
  if (!baseType)
    return {};

  switch (baseType->ID()) {
    case TypeID::Void:
    case TypeID::Int:
    case TypeID::Float:
    case TypeID::Bool:
    case TypeID::Mat2:
    case TypeID::Mat3:
    case TypeID::Mat4:
      break;
    case TypeID::Vec2:
      return get_float_vector_member_type(mMemberName.Identifier(), 2);
    case TypeID::Vec3:
      return get_float_vector_member_type(mMemberName.Identifier(), 3);
    case TypeID::Vec4:
      return get_float_vector_member_type(mMemberName.Identifier(), 4);
    case TypeID::Vec2i:
      return get_int_vector_member_type(mMemberName.Identifier(), 2);
    case TypeID::Vec3i:
      return get_int_vector_member_type(mMemberName.Identifier(), 3);
    case TypeID::Vec4i:
      return get_int_vector_member_type(mMemberName.Identifier(), 4);
  }

  return {};
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

auto
BinaryExpr::GetType() const -> std::optional<Type>
{
  auto leftType = mLeftExpr->GetType();

  auto rightType = mRightExpr->GetType();

  if (!leftType || !rightType)
    return {};

  if (leftType == rightType)
    return leftType;

  for (const auto& commonTypeEntry : gCommonTypeTable) {
    if (commonTypeEntry.Match(leftType->ID(), rightType->ID()))
      return Type(commonTypeEntry.CommonType());
  }

  return {};
}

auto
FuncCall::GetType() const -> std::optional<Type>
{
  if (mResolvedFuncs.size() != 1)
    return {};

  return mResolvedFuncs[0]->ReturnType();
}
