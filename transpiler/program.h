#pragma once

#include "abort.h"
#include "decl.h"

#include <memory>
#include <vector>

class Program final
{
public:
  void AppendFunc(FuncDecl* f) { mFuncs.emplace_back(f); }

  void AppendGlobalVar(VarDecl* globalVar);

  void AppendModuleImportDecl(ModuleImportDecl* importDecl)
  {
    mModuleImports.emplace_back(importDecl);
  }

  bool HasMainModuleDecl() const noexcept { return !!mMainModuleDecl; }

  const auto& Funcs() const noexcept { return mFuncs; }

  const auto& GlobalVars() const noexcept { return mGlobalVars; }

  const auto& UniformGlobalVars() const noexcept { return mUniformGlobalVars; }

  const auto& VaryingGlobalVars() const noexcept { return mVaryingGlobalVars; }

  const ModuleExportDecl& MainModuleDecl() const noexcept
  {
    if (!mMainModuleDecl) {
      ABORT("Main module decl was accessed, but is null.");
    }

    return *mMainModuleDecl;
  }

  void SetMainModuleDecl(ModuleExportDecl* moduleDecl)
  {
    mMainModuleDecl.reset(moduleDecl);
  }

  std::string GetModuleName() const;

private:
  std::unique_ptr<ModuleExportDecl> mMainModuleDecl;

  std::vector<std::unique_ptr<FuncDecl>> mFuncs;

  std::vector<std::unique_ptr<VarDecl>> mGlobalVars;

  std::vector<std::unique_ptr<ModuleImportDecl>> mModuleImports;

  std::vector<const VarDecl*> mVaryingGlobalVars;

  std::vector<const VarDecl*> mUniformGlobalVars;
};
