#include "cpp_generator_v2.h"

#include "cpp_expr_environment_impl.h"
#include "cpp_expr_generator.h"
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
Generator::Generate(const Module& module)
{
  if (!module.HasModuleExportDecl())
    return;

  os << "#pragma once" << std::endl;

  Blank();

  os << "#include <pathway.h>" << std::endl;

  Blank();

  const auto& moduleName = module.GetModuleExportDecl().GetModuleName();

  const auto& ids = moduleName.Identifiers();

  for (const auto& id : ids) {

    os << "namespace " << *id << " {" << std::endl;

    Blank();
  }

  GenerateInnerNamespaceDecls(module);

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
Generator::GenerateInnerNamespaceDecls(const Module& module)
{
  Indent() << "using namespace pathway;" << std::endl;

  Blank();

  GenerateUniformData(module);

  Blank();

  GenerateVaryingData(module);

  Blank();

  Indent() << "// Implementation details below." << std::endl;

  GenerateFuncDefs(module);
}

void
Generator::GenerateUniformData(const Module& module)
{
  os << "template <typename float_type, typename int_type>" << std::endl;
  os << "struct uniform_data final" << std::endl;
  os << '{' << std::endl;

  IncreaseIndent();

  GenerateTypeAliases();

  for (const auto& var : module.UniformGlobalVars()) {

    Blank();

    TypePrinter typePrinter;

    typePrinter.Visit(var->GetType());

    Indent() << typePrinter.String() << ' ' << var->Identifier();

    if (var->HasInitExpr()) {

      ExprEnvironmentImpl exprEnv(module);

      ExprGenerator exprGenerator(exprEnv);

      var->InitExpr().AcceptVisitor(exprGenerator);

      os << " = " << exprGenerator.String();
    }

    os << ';' << std::endl;
  }

  DecreaseIndent();

  os << "};" << std::endl;
}

void
Generator::GenerateVaryingData(const Module& module)
{
  os << "template <typename float_type, typename int_type>" << std::endl;
  os << "struct varying_data final" << std::endl;
  os << '{' << std::endl;

  IncreaseIndent();

  GenerateTypeAliases();

  Blank();

  Indent() << "using uniform_data_type = uniform_data<float_type, int_type>;"
           << std::endl;

  for (const auto& var : module.VaryingGlobalVars()) {

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

  for (const auto& func : module.Funcs()) {

    Blank();

    if (func->IsPixelSampler()) {
      Indent() << "auto operator()(const uniform_data_type& frame, vec2 "
                  "uv_min, vec2 uv_max) noexcept -> void;"
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

    GenerateParamList(module, *func);

    os << " noexcept -> " << typePrinter.String() << ';' << std::endl;
  }

  DecreaseIndent();

  os << "};" << std::endl;
}

void
Generator::GenerateParamList(const Module& module, const FuncDecl& funcDecl)
{
  (void)module;

  std::vector<std::string> paramStrings;

  if (funcDecl.ReferencesFrameState())
    paramStrings.emplace_back("const uniform_data_type& frame");
  else if (funcDecl.IsEntryPoint())
    paramStrings.emplace_back("const uniform_data_type&");

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
Generator::GenerateFuncDefs(const Module& module)
{
  for (const auto& func : module.Funcs()) {

    Blank();

    Indent() << "template <typename float_type, typename int_type>"
             << std::endl;

    Indent() << "auto varying_data<float_type, int_type>::";

    if (func->IsPixelSampler()) {
      if (func->ReferencesFrameState())
        os << "operator()(const uniform_data_type& frame, vec2 uv_min, vec2 "
              "uv_max) noexcept -> ";
      else
        os << "operator()(const uniform_data_type&, vec2 uv_min, vec2 uv_max) "
              "noexcept -> ";
    } else if (func->IsPixelEncoder()) {
      if (func->ReferencesFrameState())
        os << "operator()(const uniform_data_type& frame) const noexcept -> ";
      else
        os << "operator()(const uniform_data_type&) const noexcept -> ";
    } else {
      os << func->Identifier();
      GenerateParamList(module, *func);
      os << " noexcept -> ";
    }

    TypePrinter typePrinter;

    typePrinter.Visit(func->ReturnType());

    os << typePrinter.String() << std::endl;

    StmtGenerator stmtGenerator(module);

    func->AcceptBodyVisitor(stmtGenerator);

    os << stmtGenerator.String();
  }
}

} // namespace cpp
