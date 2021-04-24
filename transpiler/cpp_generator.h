#pragma once

#include "c_based_generator.h"

class cpp_generator final
  : public c_based_generator
  , public stmt_visitor
  , public expr_visitor
{
public:
  cpp_generator(std::ostream& os)
    : c_based_generator(os)
  {}

  void generate(const program& prg) override;

private:
  void visit(const compound_stmt& s) override
  {
    this->indent() << '{' << std::endl;

    this->increase_indent();

    for (const auto& inner_stmt : *s.stmts)
      inner_stmt->accept(*this);

    this->decrease_indent();

    this->indent() << '}' << std::endl;
  }

  void visit(const decl_stmt& s) override
  {
    this->indent() << to_string(s.v->mType) << ' ' << *s.v->name.identifier;

    if (s.v->init_expr) {

      this->os << " = ";

      s.v->init_expr->accept(*this);
    }

    this->os << ';' << std::endl;
  }

  void visit(const return_stmt& s) override
  {
    this->indent() << "return ";

    s.return_value->accept(*this);

    this->os << ';' << std::endl;
  }

  void visit(const int_literal& e) override { this->os << e.value; }

  void visit(const float_literal& e) override { this->os << e.value; }

  void visit(const bool_literal& e) override { this->os << e.value; }

  void visit(const type_constructor& type_ctor) override
  {
    this->os << to_string(type_ctor.mType) << '{';

    for (size_t i = 0; i < type_ctor.args->size(); i++) {

      type_ctor.args->at(i)->accept(*this);

      if ((i + 1) < type_ctor.args->size())
        this->os << ", ";
    }

    this->os << '}';
  }

  void visit(const var_ref& v) override
  {
    assert(v.resolved_var != nullptr);

    if (v.resolved_var->is_global)
      this->os << "prg.";

    this->os << *v.name.identifier;
  }

  void visit(const group_expr& e) override
  {
    os << '(';

    e.mInnerExpr->accept(*this);

    os << ')';
  }

  void visit(const unary_expr& e) override
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

  void visit(const binary_expr& e) override
  {
    e.left->accept(*this);

    this->os << ' ';

    switch (e.k) {
      case binary_expr::kind::ADD:
        this->os << '+';
        break;
      case binary_expr::kind::SUB:
        this->os << '-';
        break;
      case binary_expr::kind::MUL:
        this->os << '*';
        break;
      case binary_expr::kind::DIV:
        this->os << '/';
        break;
      case binary_expr::kind::MOD:
        this->os << '%';
        break;
    }

    this->os << ' ';

    e.right->accept(*this);
  }

  void visit(const member_expr& e) override;

  void generate_int_vec_swizzle(const member_expr& e);

  void generate_float_vec_swizzle(const member_expr& e);

  void generate_builtin_types(const program& prg)
  {
    this->os << "template <typename scalar>" << std::endl;
    this->os << "struct basic_vec2 final { scalar x, y; };" << std::endl;

    this->blank();

    this->os << "template <typename scalar>" << std::endl;
    this->os << "struct basic_vec3 final { scalar x, y, z; };" << std::endl;

    this->blank();

    this->os << "template <typename scalar>" << std::endl;
    this->os << "struct alignas(16) basic_vec4 final { scalar x, y, z, w; };"
             << std::endl;

    this->blank();

    this->os << "using vec2 = basic_vec2<float>;" << std::endl;
    this->os << "using vec3 = basic_vec3<float>;" << std::endl;
    this->os << "using vec4 = basic_vec4<float>;" << std::endl;

    this->blank();

    this->os << "using vec2i = basic_vec2<std::int32_t>;" << std::endl;
    this->os << "using vec3i = basic_vec3<std::int32_t>;" << std::endl;
    this->os << "using vec4i = basic_vec4<std::int32_t>;" << std::endl;

    this->blank();

    this->os << "struct alignas(16) mat4 final { float data[16]; };"
             << std::endl;

    this->blank();

    this->os << "struct framebuf;" << std::endl;

    this->blank();

    this->os << "auto create_framebuf() -> framebuf*;" << std::endl;

    this->blank();

    this->os << "void destroy_framebuf(framebuf* fb);" << std::endl;

    this->blank();

    this->os
      << "void resize_framebuf(framebuf& fb, std::size_t w, std::size_t h);"
      << std::endl;

    this->blank();

    this->os << "void encode_framebuf(const framebuf& fb," << std::endl;
    this->os << "                     unsigned char* rgb_buf," << std::endl;
    this->os << "                     std::size_t x0," << std::endl;
    this->os << "                     std::size_t y0," << std::endl;
    this->os << "                     std::size_t x1," << std::endl;
    this->os << "                     std::size_t y1) noexcept;" << std::endl;

    this->blank();

    this->generate_program_state(prg);

    this->blank();

    this->os << "void run(const program &prg, framebuf &fb) noexcept;"
             << std::endl;
  }

  void generate(const func& fn, const param_list& params)
  {
    this->os << '(';

    if (fn.requires_program_state() || fn.is_main()) {

      if (fn.requires_program_state())
        this->os << "const program& prg";
      else
        this->os << "const program&";

      if (params.size() > 0)
        this->os << ", ";
    }

    for (size_t i = 0; i < params.size(); i++) {

      this->os << this->to_string(params.at(i)->mType);

      this->os << ' ';

      this->os << *params.at(i)->name.identifier;

      if ((i + 1) < params.size())
        this->os << ',';
    }

    this->os << ')';
  }

  void generate_program_state(const program& prg)
  {
    this->os << "struct program final" << std::endl;

    this->os << '{' << std::endl;

    this->increase_indent();

    for (size_t i = 0; i < prg.vars.size(); i++) {

      const auto& v = *prg.vars.at(i);

      this->indent() << to_string(v.mType) << " " << *v.name.identifier;

      if (v.init_expr) {

        this->os << " = ";

        v.init_expr->accept(*this);
      }

      this->os << ';' << std::endl;

      if ((i + 1) < prg.vars.size())
        this->blank();
    }

    this->decrease_indent();

    this->os << "};" << std::endl;
  }

  std::ostream& generate_header(const func& fn)
  {
    if (fn.is_main())
      this->os << "auto shader_main";
    else
      this->os << "auto " << *fn.name.identifier;

    this->generate(fn, *fn.params);

    this->os << " noexcept -> " << this->to_string(fn.mReturnType);

    return this->os;
  }

  void generate_prototypes(const program& prg)
  {
    for (const auto& fn : prg.funcs) {

      this->generate_header(*fn) << ';' << std::endl;

      this->blank();
    }
  }

  void generate_definitions(const program& prg)
  {
    for (const auto& fn : prg.funcs) {

      this->generate_header(*fn) << std::endl;

      fn->body->accept(*this);

      this->blank();
    }
  }
};
