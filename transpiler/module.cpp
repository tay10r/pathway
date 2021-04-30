#include "module.h"

void
Module::AppendFunc(FuncDecl* f)
{
  mFuncs.emplace_back(f);

  mDeclList.emplace_back(f);
}

void
Module::AppendGlobalVar(VarDecl* globalVar)
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

  mDeclList.emplace_back(globalVar);
}

void
Module::AppendModuleImportDecl(ModuleImportDecl* importDecl)
{
  mModuleImports.emplace_back(importDecl);

  mDeclList.emplace_back(importDecl);
}

void
Module::SetModuleExportDecl(ModuleExportDecl* moduleExportDecl)
{
  mModuleExportDecls.emplace_back(moduleExportDecl);

  mDeclList.emplace_back(moduleExportDecl);
}

std::string
Module::GetModuleName() const
{
  if (!HasModuleExportDecl())
    return "untitled";

  const auto& exportDecl = GetModuleExportDecl();

  return exportDecl.GetModuleName().ToSingleIdentifier();
}
