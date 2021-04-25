#include "check.h"

#include "common.h"

#include <ostream>

namespace {

struct check_context final
{
  std::string path;

  const Program& program;

  check_context(const check_context&) = delete;

  check_context(const std::string& path_,
                const Program& program_,
                std::ostream& es_)
    : path(path_)
    , program(program_)
    , es(es_)
  {}

  size_t get_error_count() const noexcept { return this->error_count; }

  std::ostream& emit_error(const location& l)
  {
    this->error_count++;

    this->es << this->path << ':' << l << ": ";

    return this->es;
  }

  void missing_entry_point()
  {
    this->es << path << ": missing entry point 'main'" << std::endl;

    this->error_count++;
  }

private:
  size_t error_count = 0;

  std::ostream& es;
};

class return_type_checker final : public stmt_visitor
{
public:
  return_type_checker(Type returnType, check_context& ctx_)
    : mReturnType(returnType)
    , ctx(ctx_)
  {}

  void visit(const AssignmentStmt&) override {}

  void visit(const compound_stmt& s) override
  {
    for (const auto& inner_stmt : *s.stmts)
      inner_stmt->accept(*this);
  }

  void visit(const decl_stmt&) override {}

  void visit(const return_stmt& s) override
  {
    const auto& ret_value = *s.return_value;

    if (!(ret_value.GetType() == mReturnType)) {
      this->ctx.emit_error(ret_value.loc)
        << "expression should return type '" << mReturnType.ID() << "' not '"
        << ret_value.GetType().ID() << "'" << std::endl;
    }
  }

private:
  Type mReturnType;

  check_context& ctx;
};

class checker final
{
public:
  checker(check_context& ctx_)
    : ctx(ctx_)
  {}

  void run_all_checks()
  {
    run_type_checks();

    check_entry_point();
  }

  void run_type_checks()
  {
    for (const auto& fn : ctx.program.Funcs())
      this->type_check(*fn);

    // TODO : global vars
  }

  void type_check(const func& fn)
  {
    return_type_checker ret_checker(*fn.mReturnType, this->ctx);

    fn.body->accept(ret_checker);
  }

  void check_entry_point()
  {
    const func* entry = nullptr;

    for (const auto& fn : this->ctx.program.Funcs()) {

      if (!fn->is_main())
        continue;

      if (entry) {

        ctx.emit_error(fn->name.loc)
          << "only one declaration of 'main' can exist." << std::endl;

        return;
      }

      check_entry_point_signature(*fn);

      entry = fn.get();
    }

    if (!entry)
      ctx.missing_entry_point();
  }

  void check_entry_point_signature(const func& fn)
  {
    if (*fn.mReturnType != TypeID::Vec3) {
      this->ctx.emit_error(fn.name.loc)
        << "return type should be type 'vec3'" << std::endl;
    }

    if (fn.params->size() != 1) {
      this->ctx.emit_error(fn.name.loc)
        << "there should only be one parameter to this function." << std::endl;
    }

    if (fn.params->size() < 1)
      return;

    if (*fn.params->at(0)->mType != TypeID::Vec2) {
      this->ctx.emit_error(fn.name.loc)
        << "parameter should be type 'vec2'" << std::endl;
    }
  }

private:
  check_context& ctx;
};

} // namespace

bool
check(const std::string& path, const Program& program, std::ostream& es)
{
  check_context ctx(path, program, es);

  checker c(ctx);

  c.run_all_checks();

  return ctx.get_error_count() == 0;
}
