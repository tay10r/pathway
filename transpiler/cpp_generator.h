#pragma once

#include "c_based_generator.h"

class CPPGenerator final
  : public CBasedGenerator
  , public stmt_visitor
  , public ExprVisitor
{
public:
  CPPGenerator(std::ostream& os)
    : CBasedGenerator(os)
  {}

  void Generate(const Program& program) override;

private:
  void visit(const AssignmentStmt& assignmentStmt) override
  {
    Indent();

    assignmentStmt.LValue().AcceptVisitor(*this);

    this->os << " = ";

    assignmentStmt.RValue().AcceptVisitor(*this);

    this->os << ";" << std::endl;
  }

  void visit(const compound_stmt& s) override
  {
    Indent() << '{' << std::endl;

    IncreaseIndent();

    for (const auto& inner_stmt : *s.stmts)
      inner_stmt->accept(*this);

    DecreaseIndent();

    Indent() << '}' << std::endl;
  }

  void visit(const decl_stmt& s) override
  {
    Indent() << ToString(*s.v->mType) << ' ' << s.v->name.Identifier();

    if (s.v->init_expr) {

      this->os << " = ";

      s.v->init_expr->AcceptVisitor(*this);
    }

    this->os << ';' << std::endl;
  }

  void visit(const return_stmt& s) override
  {
    Indent() << "return ";

    s.return_value->AcceptVisitor(*this);

    this->os << ';' << std::endl;
  }

  void Visit(const IntLiteral& intLiteral) override
  {
    this->os << intLiteral.Value();
  }

  void Visit(const FloatLiteral& e) override
  {
    this->os << std::scientific << e.Value() << "f";
  }

  void Visit(const BoolLiteral& e) override { this->os << e.Value(); }

  void Visit(const type_constructor& type_ctor) override
  {
    this->os << ToString(type_ctor.mType) << '{';

    for (size_t i = 0; i < type_ctor.args->size(); i++) {

      type_ctor.args->at(i)->AcceptVisitor(*this);

      if ((i + 1) < type_ctor.args->size())
        this->os << ", ";
    }

    this->os << '}';
  }

  void Visit(const VarRef& v) override
  {
    assert(v.HasResolvedVar());

    if (v.ResolvedVar().IsUniformGlobal())
      this->os << "frame.";
    else if (v.ResolvedVar().IsVaryingGlobal())
      this->os << "pixel.";

    this->os << v.Identifier();
  }

  void Visit(const GroupExpr& e) override
  {
    os << '(';
    e.Recurse(*this);
    os << ')';
  }

  void Visit(const unary_expr& e) override
  {
    switch (e.k) {
      case unary_expr::kind::LOGICAL_NOT:
        this->os << "!";
        break;
      case unary_expr::kind::BITWISE_NOT:
        this->os << "~";
        break;
      case unary_expr::kind::NEGATE:
        this->os << "-";
        break;
    }
  }

  void Visit(const BinaryExpr& e) override { GenerateBinaryExpr(e); }

  void Visit(const FuncCall& funcCall) override
  {
    os << funcCall.Identifier() << "(";

    const auto& args = funcCall.Args();

    for (size_t i = 0; i < args.size(); i++) {

      args[i]->AcceptVisitor(*this);

      if ((i + 1) < args.size())
        os << ", ";
    }

    os << ")";
  }

  void Visit(const MemberExpr& e) override;

  void GenerateBinaryExpr(const BinaryExpr&);

  void GenerateIntVectorSwizzle(const MemberExpr& e, size_t sourceVecSize);

  void GenerateFloatVectorSwizzle(const MemberExpr& e, size_t sourceVecSize);

  void generate_pixel_state(const Program& program);

  void generate_frame_state(const Program& program);

  void generate_builtin_types(const Program& program);

  void generate(const Func& fn, const param_list& params);

  std::ostream& generate_header(const Func& fn)
  {
    this->os << "auto " << fn.Identifier();

    this->generate(fn, fn.ParamList());

    this->os << " noexcept -> " << ToString(fn.ReturnType());

    return this->os;
  }

  void generate_prototypes(const Program& program)
  {
    for (const auto& fn : program.Funcs()) {

      this->generate_header(*fn) << ';' << std::endl;

      this->Blank();
    }
  }

  void generate_definitions(const Program& program)
  {
    for (const auto& fn : program.Funcs()) {

      this->generate_header(*fn) << std::endl;

      fn->AcceptBodyAccessor(*this);

      this->Blank();
    }
  }
};
