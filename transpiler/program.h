#pragma once

#include "decl.h"

#include <memory>
#include <vector>

class Program final
{
public:
  void AppendFunc(FuncDecl* f) { mFuncs.emplace_back(f); }

  void AppendGlobalVar(VarDecl* globalVar);

  const auto& Funcs() const noexcept { return mFuncs; }

  const auto& GlobalVars() const noexcept { return mGlobalVars; }

  const auto& UniformGlobalVars() const noexcept { return mUniformGlobalVars; }

  const auto& VaryingGlobalVars() const noexcept { return mVaryingGlobalVars; }

private:
  std::vector<std::unique_ptr<FuncDecl>> mFuncs;

  std::vector<std::unique_ptr<VarDecl>> mGlobalVars;

  std::vector<const VarDecl*> mVaryingGlobalVars;

  std::vector<const VarDecl*> mUniformGlobalVars;
};
