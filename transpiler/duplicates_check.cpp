#include "duplicates_check.h"

#include "decl.h"
#include "diagnostics.h"
#include "module.h"

#include <map>

namespace {

using Duplicate = DuplicatesCheck::Duplicate;

using Scope = std::map<std::string, Location>;

using SymbolTable = std::vector<Scope>;

class GlobalDuplicatesChecker final : public DeclVisitor
{
public:
  GlobalDuplicatesChecker(std::vector<Duplicate>& duplicates)
    : mDuplicates(duplicates)
  {}

  void Visit(const FuncDecl& funcDecl) override
  {
    auto existingVar = mScope.find(funcDecl.Identifier());

    if (existingVar != mScope.end()) {
      EmitDuplicate(existingVar->second, funcDecl.GetNameLocation());
      return;
    }

    auto mangledName = funcDecl.GetMangledName();

    auto existingFunc = mFuncScope.find(mangledName);

    if (existingFunc != mFuncScope.end()) {
      EmitDuplicate(existingFunc->second, funcDecl.GetNameLocation());
      return;
    }

    mFuncScopeUnmangled.emplace(funcDecl.Identifier(),
                                funcDecl.GetNameLocation());

    mFuncScope.emplace(mangledName, funcDecl.GetNameLocation());
  }

  void Visit(const VarDecl& varDecl) override
  {
    auto existingFunc = mFuncScopeUnmangled.find(varDecl.Identifier());

    if (existingFunc != mFuncScopeUnmangled.end()) {
      EmitDuplicate(existingFunc->second, varDecl.GetNameLocation());
      return;
    }

    auto existing = mScope.find(varDecl.Identifier());

    if (existing != mScope.end()) {
      EmitDuplicate(existing->second, varDecl.GetNameLocation());
      return;
    }

    mScope.emplace(varDecl.Identifier(), varDecl.GetNameLocation());
  }

  void Visit(const ModuleImportDecl&) override {}

  void Visit(const ModuleExportDecl&) override {}

private:
  void EmitDuplicate(const Location& originalLoc, const Location& duplicateLoc)
  {
    Duplicate duplicate{ originalLoc, duplicateLoc };

    mDuplicates.emplace_back(duplicate);
  }

  /// @brief Contains variables and module names.
  Scope mScope;

  Scope mFuncScope;

  Scope mFuncScopeUnmangled;

  std::vector<Duplicate>& mDuplicates;
};

} // namespace

auto
DuplicatesCheck::Run(const Module& module) -> std::vector<Duplicate>
{
  std::vector<Duplicate> duplicates;

  GlobalDuplicatesChecker globalChecker(duplicates);

  module.AcceptDeclVisitor(globalChecker);

  return duplicates;
}

bool
DuplicatesCheck::Check(const Module& module, DiagObserver& diagObserver)
{
  auto duplicates = Run(module);

  if (duplicates.empty())
    return true;

  for (const auto& dup : duplicates) {

    auto dupLoc = dup.duplicateLocation;

    Diag error(dupLoc, DiagID::DuplicateDecl, "needs a different name");

    diagObserver.Observe(error);

    auto origLoc = dup.originalLocation;

    Diag note(origLoc, DiagID::OriginalDecl, "first used here");

    diagObserver.Observe(note);
  }

  return false;
}
