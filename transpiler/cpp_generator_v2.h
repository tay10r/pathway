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

  void Generate(const Program& program) override;

protected:
  void GenerateTypeAliases();

  void GenerateParamList(const Program&, const FuncDecl&);

  void GenerateUniformData(const Program&);

  void GenerateVaryingData(const Program&);

  void GenerateInnerNamespaceDecls(const Program& program);

  void GenerateFuncDefs(const Program&);
};

} // namespace cpp
