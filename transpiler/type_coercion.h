#pragma once

#include "expr.h"
#include "type_environment.h"
#include "type_inference.h"

/// @brief This class is used to modify the types of expressions by inserting
/// implicit casts so that they are compatible with the destination type.
///
/// @note This also coerces variability.
template<typename DerivedTypeEnv>
class TypeCoercionMutator : public ExprMutator
{
public:
  using TypeEnv = TypeEnvironment<DerivedTypeEnv>;

  TypeCoercionMutator(const Type& dstType, const TypeEnv& typeEnv)
    : mTypeEnv(typeEnv)
  {}

  ~TypeCoercionMutator() = default;

  /// @brief Gets the result of the type coercion.
  ///
  /// @return This function will not return a type if the coercion failed. This
  /// will happen if there's a semantic error in the expression.
  auto ResultType() -> std::optional<Type>;

  void Mutate(FuncCall&) const override {}

  void Mutate(IntLiteral&) const override {}

  void Mutate(BoolLiteral&) const override {}

  void Mutate(FloatLiteral&) const override {}

  void Mutate(BinaryExpr&) const override {}

  void Mutate(unary_expr&) const override {}

  void Mutate(GroupExpr&) const override {}

  void Mutate(VarRef&) const override {}

  void Mutate(type_constructor&) const override {}

  void Mutate(MemberExpr&) const override {}

private:
  std::optional<Type> mResultType;

  Type mDstType;

  const TypeEnv& mTypeEnv;
};
