#include "program.h"

void
Program::AppendGlobalVar(VarDecl* globalVar)
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

std::string
Program::GetModuleName() const
{
  if (!HasMainModuleDecl())
    return "untitled";

  const auto& moduleDecl = MainModuleDecl();

  return moduleDecl.GetModuleName().ToSingleIdentifier();
}
