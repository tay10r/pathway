#include "cpp_generator_v2.h"

#include "cpp_stmt_generator.h"

namespace cpp {

void
TypePrinter::Visit(const Type& type)
{
  switch (type.ID()) {
    case TypeID::Void:
      mStream << "void";
      break;
    case TypeID::Int:
      mStream << "int_type";
      break;
    case TypeID::Bool:
      mStream << "bool";
      break;
    case TypeID::Float:
      mStream << "float";
      break;
    case TypeID::Vec2:
      mStream << "vec2";
      break;
    case TypeID::Vec3:
      mStream << "vec3";
      break;
    case TypeID::Vec4:
      mStream << "vec4";
      break;
    case TypeID::Vec2i:
      mStream << "vec2i";
      break;
    case TypeID::Vec3i:
      mStream << "vec3i";
      break;
    case TypeID::Vec4i:
      mStream << "vec4i";
      break;
    case TypeID::Mat2:
      mStream << "mat2";
      break;
    case TypeID::Mat3:
      mStream << "mat3";
      break;
    case TypeID::Mat4:
      mStream << "mat4";
      break;
  }
}

void
Generator::Generate(const Program& program)
{
  if (!program.HasMainModuleDecl())
    return;

  os << "#pragma once" << std::endl;

  Blank();

  os << "#include <pathway.h>" << std::endl;

  Blank();

  const auto& moduleName = program.MainModuleDecl().GetModuleName();

  const auto& ids = moduleName.Identifiers();

  for (const auto& id : ids) {

    os << "namespace " << *id << " {" << std::endl;

    Blank();
  }

  GenerateInnerNamespaceDecls(program);

  for (auto it = ids.rbegin(); it != ids.rend(); it++) {

    const auto& id = *it;

    Blank();

    os << "} // namespace " << *id << std::endl;
  }
}

void
Generator::GenerateTypeAliases()
{
  Indent() << "using vec2 = vector<float_type, 2>;" << std::endl;
  Indent() << "using vec3 = vector<float_type, 3>;" << std::endl;
  Indent() << "using vec4 = vector<float_type, 4>;" << std::endl;

  Blank();

  Indent() << "using vec2i = vector<int_type, 2>;" << std::endl;
  Indent() << "using vec3i = vector<int_type, 3>;" << std::endl;
  Indent() << "using vec4i = vector<int_type, 4>;" << std::endl;

  Blank();

  Indent() << "using mat2 = matrix<float_type, 2, 2>;" << std::endl;
  Indent() << "using mat3 = matrix<float_type, 3, 3>;" << std::endl;
  Indent() << "using mat4 = matrix<float_type, 4, 4>;" << std::endl;
}

void
Generator::GenerateInnerNamespaceDecls(const Program& program)
{
  GenerateUniformData(program);

  Blank();

  GenerateVaryingData(program);

  Blank();

  Indent() << "// Implementation details below." << std::endl;

  GenerateFuncDefs(program);
}

void
Generator::GenerateUniformData(const Program& program)
{
  os << "template <typename float_type, typename int_type>" << std::endl;
  os << "struct uniform_data final" << std::endl;
  os << '{' << std::endl;

  IncreaseIndent();

  GenerateTypeAliases();

  for (const auto& var : program.UniformGlobalVars()) {

    Blank();

    TypePrinter typePrinter;

    typePrinter.Visit(var->GetType());

    Indent() << typePrinter.String() << ' ' << var->Identifier();

    if (var->HasInitExpr()) {
      // TODO
      ;
    }

    os << ';' << std::endl;
  }

  DecreaseIndent();

  os << "};" << std::endl;
}

void
Generator::GenerateVaryingData(const Program& program)
{
  os << "template <typename float_type, typename int_type>" << std::endl;
  os << "struct varying_data final" << std::endl;
  os << '{' << std::endl;

  IncreaseIndent();

  GenerateTypeAliases();

  Blank();

  Indent() << "using uniform_data_type = uniform_data<float_type, int_type>;"
           << std::endl;

  for (const auto& var : program.VaryingGlobalVars()) {

    Blank();

    TypePrinter typePrinter;

    typePrinter.Visit(var->GetType());

    Indent() << typePrinter.String() << ' ' << var->Identifier();

    if (var->HasInitExpr()) {
      // TODO
      ;
    }

    os << ';' << std::endl;
  }

  for (const auto& func : program.Funcs()) {

    Blank();

    if (func->IsPixelSampler()) {
      Indent() << "auto operator()(const uniform_data_type& frame, vec2 uv) "
                  "noexcept -> void;"
               << std::endl;
      continue;
    } else if (func->IsPixelEncoder()) {
      Indent() << "auto operator()(const uniform_data_type& frame) const "
                  "noexcept -> vec4;"
               << std::endl;
      continue;
    }

    TypePrinter typePrinter;

    typePrinter.Visit(func->ReturnType());

    Indent() << "auto " << func->Identifier();

    GenerateParamList(program, *func);

    os << " noexcept -> " << typePrinter.String() << ';' << std::endl;
  }

  DecreaseIndent();

  os << "};" << std::endl;
}

void
Generator::GenerateParamList(const Program& program, const FuncDecl& funcDecl)
{
  (void)program;

  std::vector<std::string> paramStrings;

  if (funcDecl.ReferencesFrameState())
    paramStrings.emplace_back("const uniform_data& frame");
  else if (funcDecl.IsEntryPoint())
    paramStrings.emplace_back("const uniform_data&");

  for (const auto& param : funcDecl.GetParamList()) {

    TypePrinter typePrinter;

    typePrinter.Visit(param->GetType());

    std::ostringstream paramStream;
    paramStream << typePrinter.String();
    paramStream << ' ';
    paramStream << param->Identifier();

    paramStrings.emplace_back(paramStream.str());
  }

  os << '(';

  for (size_t i = 0; i < paramStrings.size(); i++) {

    os << paramStrings[i];

    if ((i + 1) < paramStrings.size())
      os << ", ";
  }

  os << ')';
}

void
Generator::GenerateFuncDefs(const Program& program)
{
  for (const auto& func : program.Funcs()) {

    Blank();

    Indent() << "template <typename float_type, typename int_type>"
             << std::endl;

    Indent() << "auto varying_data::";

    if (func->IsPixelSampler()) {
      os << "operator()(vec2 uv) noexcept -> ";
    } else if (func->IsPixelEncoder()) {
      os << "operator()() const noexcept -> ";
    } else {
      os << func->Identifier();
      GenerateParamList(program, *func);
      os << " noexcept -> ";
    }

    TypePrinter typePrinter;

    typePrinter.Visit(func->ReturnType());

    os << typePrinter.String() << std::endl;

    StmtGenerator stmtGenerator(program);

    func->AcceptBodyVisitor(stmtGenerator);

    os << stmtGenerator.String();
  }
}

} // namespace cpp
