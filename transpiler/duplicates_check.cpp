#include "duplicates_check.h"

#include "decl.h"

namespace {

class DuplicateDeclChecker final : public DeclVisitor
{
public:
  void Visit(const FuncDecl&) override {}

  void Visit(const VarDecl&) override {}

  void Visit(const ModuleImportDecl&) override {}

  void Visit(const ModuleExportDecl&) override {}
};

} // namespace

auto
DuplicatesCheck::Run(const Module& module) -> std::vector<Duplicate>
{
  std::vector<Duplicate> duplicates;

  (void)module;

  return duplicates;
}
