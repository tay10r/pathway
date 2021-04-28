#pragma once

#include "decl.h"
#include "expr.h"
#include "type_environment.h"

/// @brief The job of this class is to compute the type of an expression
/// belonging to an abstract type environment. The type environment is
/// abstracted for the purpose of testing.
///
/// @tparam DerivedEnv Should implement the functions in @ref TypeEnvironment.
template<typename DerivedEnv>
class TypeInferenceEngine final : public ExprVisitor
{
public:
  TypeInferenceEngine(const TypeEnvironment<DerivedEnv>& environment)
    : mEnvironment(environment)
  {}

  Type GetType() const noexcept { return mType; }

  bool Success() const noexcept { return mSuccess; }

  void Visit(const BoolLiteral&) override
  {
    mSuccess = true;
    mType = Type(TypeID::Bool, Variability::Unbound);
  }

  void Visit(const IntLiteral&) override
  {
    mSuccess = true;
    mType = Type(TypeID::Int, Variability::Unbound);
  }

  void Visit(const FloatLiteral&) override
  {
    mSuccess = true;
    mType = Type(TypeID::Float, Variability::Unbound);
  }

  void Visit(const FuncCall& funcCall) override
  {
    const auto& funcDecl = funcCall.GetFuncDecl();

    mType = funcDecl.ReturnType();
    mSuccess = true;
  }

  void Visit(const BinaryExpr& binaryExpr) override
  {
    TypeInferenceEngine subEngineA(mEnvironment);
    TypeInferenceEngine subEngineB(mEnvironment);

    binaryExpr.LeftExpr().AcceptVisitor(subEngineA);

    binaryExpr.RightExpr().AcceptVisitor(subEngineB);

    if (!subEngineA.Success() || !subEngineB.Success())
      return;

    auto typeA = subEngineA.GetType();
    auto typeB = subEngineB.GetType();

    if (typeA == typeB) {
      mType = typeA;
      mSuccess = true;
      return;
    }

    std::optional<Variability> variability;

    if (typeA.GetVariability() == typeB.GetVariability()) {
      variability = typeA.GetVariability();
    }

    // TODO : variability

    std::optional<TypeID> typeID;

    if (typeA.ID() == typeB.ID()) {
      typeID = typeA.ID();
    } else if ((typeA.ID() == TypeID::Float) && IsVecOrMat(typeB.ID())) {
      typeID = typeB.ID();
    } else if (IsVecOrMat(typeA.ID()) && (typeB.ID() == TypeID::Float)) {
      typeID = typeA.ID();
    }

    if (variability && typeID) {
      mType = Type(*typeID, *variability);
      mSuccess = true;
    }
  }

  void Visit(const UnaryExpr&) override {}

  void Visit(const GroupExpr& groupExpr) override { groupExpr.Recurse(*this); }

  void Visit(const VarRef& varRef) override
  {
    const Type* type = mEnvironment.FindVar(varRef.Identifier());

    if (!type)
      return;

    mType = *type;

    mSuccess = true;
  }

  void Visit(const TypeConstructor&) override {}

  void Visit(const MemberExpr& memberExpr) override
  {
    TypeInferenceEngine<DerivedEnv> subEngine(mEnvironment);

    memberExpr.BaseExpr().AcceptVisitor(subEngine);

    if (!subEngine.Success())
      return;

    auto baseType = subEngine.GetType();

    const auto& memberName = memberExpr.MemberName();

    std::array<TypeID, 4> floatTypeIDs{
      TypeID::Float, TypeID::Vec2, TypeID::Vec3, TypeID::Vec4
    };

    std::array<TypeID, 4> intTypeIDs{
      TypeID::Int, TypeID::Vec2i, TypeID::Vec3i, TypeID::Vec4i
    };

    auto variability = baseType.GetVariability();

    switch (baseType.ID()) {
      case TypeID::Void:
      case TypeID::Bool:
      case TypeID::Int:
      case TypeID::Float:
      case TypeID::Mat2:
      case TypeID::Mat3:
      case TypeID::Mat4:
        return;
      case TypeID::Vec2:
        CheckVectorMemberType(memberName, variability, 2, floatTypeIDs);
        break;
      case TypeID::Vec3:
        CheckVectorMemberType(memberName, variability, 3, floatTypeIDs);
        break;
      case TypeID::Vec4:
        CheckVectorMemberType(memberName, variability, 4, floatTypeIDs);
        break;
      case TypeID::Vec2i:
        CheckVectorMemberType(memberName, variability, 2, intTypeIDs);
        break;
      case TypeID::Vec3i:
        CheckVectorMemberType(memberName, variability, 3, intTypeIDs);
        break;
      case TypeID::Vec4i:
        CheckVectorMemberType(memberName, variability, 4, intTypeIDs);
        break;
    }
  }

private:
  void CheckVectorMemberType(const DeclName& memberName,
                             Variability variability,
                             size_t sourceVectorSize,
                             const std::array<TypeID, 4>& typeIDs)
  {
    auto swizzle = Swizzle::Make(memberName.Identifier(), sourceVectorSize);

    if (!swizzle)
      return;

    switch (swizzle->Size()) {
      case 1:
        mType = Type(typeIDs[0], variability);
        mSuccess = true;
        break;
      case 2:
        mType = Type(typeIDs[1], variability);
        mSuccess = true;
        break;
      case 3:
        mType = Type(typeIDs[2], variability);
        mSuccess = true;
        break;
      case 4:
        mType = Type(typeIDs[3], variability);
        mSuccess = true;
        break;
    }
  }

  const TypeEnvironment<DerivedEnv>& mEnvironment;

  bool mSuccess = false;

  Type mType = Type(TypeID::Void);
};
