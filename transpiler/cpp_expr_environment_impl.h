#pragma once

#include "cpp_expr_generator.h"

#include "module.h"

namespace cpp {

class ExprEnvironmentImpl final : public ExprEnvironment<ExprEnvironmentImpl>
{
public:
  ExprEnvironmentImpl(const Module& module)
    : mModule(module)
  {}

  auto GetGlobalsUsageImpl(const FuncCall& funcCall) const
    -> std::optional<GlobalsUsage>
  {
    if (!funcCall.Resolved())
      return {};

    const auto& funcDecl = funcCall.GetFuncDecl();

    return GlobalsUsage{ funcDecl.ReferencesFrameState(),
                         funcDecl.ReferencesPixelState() };
  }

  auto GetVarOriginImpl(const VarRef& varRef) const -> std::optional<VarOrigin>
  {
    for (const auto& var : mModule.GlobalVars()) {

      if (var->Identifier() != varRef.Identifier())
        continue;

      if (var->IsVaryingGlobal())
        return VarOrigin::VaryingGlobal;
      else if (var->IsUniformGlobal())
        return VarOrigin::UniformGlobal;
    }

    return {};
  }

  auto GetVectorComponentCountImpl(const Expr& expr) const
    -> std::optional<size_t>
  {
    auto type = expr.GetType();
    if (!type)
      return {};

    switch (type->ID()) {
      case TypeID::Void:
      case TypeID::Int:
      case TypeID::Bool:
      case TypeID::Float:
      case TypeID::Mat2:
      case TypeID::Mat3:
      case TypeID::Mat4:
        return {};
      case TypeID::Vec2:
      case TypeID::Vec2i:
        return 2;
      case TypeID::Vec3:
      case TypeID::Vec3i:
        return 3;
      case TypeID::Vec4:
      case TypeID::Vec4i:
        return 4;
    }

    return {};
  }

private:
  const Module& mModule;
};

} // namespace cpp
