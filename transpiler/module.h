#pragma once

#include "abort.h"
#include "decl.h"

#include <memory>
#include <vector>

class Module final
{
public:
  void AppendFunc(FuncDecl* f);

  void AppendGlobalVar(VarDecl* globalVar);

  void AppendModuleImportDecl(ModuleImportDecl* importDecl);

  bool HasModuleExportDecl() const noexcept
  {
    return !mModuleExportDecls.empty();
  }

  const auto& Funcs() const noexcept { return mFuncs; }

  const auto& GlobalVars() const noexcept { return mGlobalVars; }

  const auto& UniformGlobalVars() const noexcept { return mUniformGlobalVars; }

  const auto& VaryingGlobalVars() const noexcept { return mVaryingGlobalVars; }

  const ModuleExportDecl& GetModuleExportDecl() const noexcept
  {
    if (mModuleExportDecls.empty()) {
      ABORT("Module export declaration was accessed, but is null.");
    }

    return *mModuleExportDecls[0];
  }

  void SetModuleExportDecl(ModuleExportDecl* moduleExportDecl);

  std::string GetModuleName() const;

private:
  /// @brief Contains the module declarations in the order that they appear in
  /// the file.
  std::vector<const Decl*> mDeclList;

  /// @note There should only be one of these, but if there are more then they
  /// are appended to this list in the order they appear.
  std::vector<std::unique_ptr<ModuleExportDecl>> mModuleExportDecls;

  std::vector<std::unique_ptr<FuncDecl>> mFuncs;

  std::vector<std::unique_ptr<VarDecl>> mGlobalVars;

  std::vector<std::unique_ptr<ModuleImportDecl>> mModuleImports;

  std::vector<const VarDecl*> mVaryingGlobalVars;

  std::vector<const VarDecl*> mUniformGlobalVars;
};
