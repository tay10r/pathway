#pragma once

#include "c_based_generator.h"

namespace cpp {

class TypePrinter final
{
public:
  std::string String() const { return mStream.str(); }

  void Visit(const Type& type);

private:
  std::ostringstream mStream;
};

class Generator final : public CBasedGenerator
{
public:
  Generator(std::ostream& os)
    : CBasedGenerator(os)
  {}

  void Generate(const Module& module) override;

protected:
  void GenerateTypeAliases();

  void GenerateParamList(const Module&, const FuncDecl&);

  void GenerateUniformData(const Module&);

  void GenerateVaryingData(const Module&);

  void GenerateInnerNamespaceDecls(const Module& module);

  void GenerateFuncDefs(const Module&);
};

} // namespace cpp
