#include "check.h"

#include "module.h"

#include <ostream>

namespace {

struct check_context final
{
  std::string path;

  const Module& module;

  check_context(const check_context&) = delete;

  check_context(const std::string& path_,
                const Module& module_,
                std::ostream& es_)
    : path(path_)
    , module(module_)
    , es(es_)
  {}

  size_t get_error_count() const noexcept { return this->error_count; }

  std::ostream& emit_error(const Location& l)
  {
    this->error_count++;

    this->es << this->path << ':' << l << ": ";

    return this->es;
  }

  void MissingPixelSampler()
  {
    this->es << path << ": missing entry point 'SamplePixel'" << std::endl;

    this->error_count++;
  }

  void MissingPixelEncoder()
  {
    this->es << path << ": missing entry point 'EncodePixel'" << std::endl;

    this->error_count++;
  }

private:
  size_t error_count = 0;

  std::ostream& es;
};

class ReturnStmtTypeChecker final : public StmtVisitor
{
public:
  ReturnStmtTypeChecker(Type expectedReturnType, check_context& ctx_)
    : mExpectedReturnType(expectedReturnType)
    , ctx(ctx_)
  {}

  void Visit(const AssignmentStmt&) override {}

  void Visit(const CompoundStmt& compoundStmt) override
  {
    compoundStmt.Recurse(*this);
  }

  void Visit(const DeclStmt&) override {}

  void Visit(const ReturnStmt& s) override
  {
    const auto& ret_value = s.ReturnValue();

    if (ret_value.GetType() != mExpectedReturnType) {
      this->ctx.emit_error(ret_value.GetLocation())
        << "expression should return type '" << mExpectedReturnType << "' not '"
        << *ret_value.GetType() << "'" << std::endl;
    }
  }

private:
  Type mExpectedReturnType;

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

    CheckEntryPoints();
  }

  void run_type_checks()
  {
    for (const auto& fn : ctx.module.Funcs())
      this->type_check(*fn);

    // TODO : global vars
  }

  void type_check(const FuncDecl& func)
  {
    ReturnStmtTypeChecker returnStmtTypeChecker(func.ReturnType(), this->ctx);

    func.AcceptBodyVisitor(returnStmtTypeChecker);
  }

  void CheckEntryPoints()
  {
    const FuncDecl* pixelSampler = nullptr;
    const FuncDecl* pixelEncoder = nullptr;

    for (const auto& func : this->ctx.module.Funcs()) {

      if (func->IsPixelSampler()) {
        if (pixelSampler) {
          ctx.emit_error(func->GetNameLocation())
            << "only one declaration of '" << func->Identifier()
            << "' can exist" << std::endl;
        } else {
          pixelSampler = func.get();
        }
        continue;
      }

      if (func->IsPixelEncoder()) {
        if (pixelEncoder) {
          ctx.emit_error(func->GetNameLocation())
            << "only one declaration of '" << func->Identifier()
            << "' can exist" << std::endl;
        } else {
          pixelEncoder = func.get();
        }
        continue;
      }
    }

    if (pixelSampler) {
      CheckPixelSamplerSignature(*pixelSampler);
    } else {
      ctx.MissingPixelSampler();
    }

    if (pixelEncoder) {
      CheckPixelEncoderSignature(*pixelEncoder);
    } else {
      ctx.MissingPixelEncoder();
    }
  }

  void CheckPixelSamplerSignature(const FuncDecl& fn)
  {
    if (fn.ReturnType() != TypeID::Void) {
      this->ctx.emit_error(fn.GetNameLocation())
        << "return type should be type 'void'" << std::endl;
    }

    if (fn.GetParamList().size() != 2) {
      this->ctx.emit_error(fn.GetNameLocation())
        << "there should be two 'vec2' parameters to this function."
        << std::endl;
      return;
    }

    if (fn.GetParamList().at(0)->GetTypeID() != TypeID::Vec2) {
      this->ctx.emit_error(fn.GetNameLocation())
        << "1st parameter should be type 'vec2'" << std::endl;
    }

    if (fn.GetParamList().at(1)->GetTypeID() != TypeID::Vec2) {
      this->ctx.emit_error(fn.GetNameLocation())
        << "2nd parameter should be type 'vec2'" << std::endl;
    }
  }

  void CheckPixelEncoderSignature(const FuncDecl& fn)
  {
    if (fn.ReturnType() != TypeID::Vec4) {
      this->ctx.emit_error(fn.GetNameLocation())
        << "return type should be type 'vec4'" << std::endl;
    }

    if (!fn.GetParamList().empty()) {
      this->ctx.emit_error(fn.GetNameLocation())
        << "there should be no parameters to this function." << std::endl;
    }
  }

private:
  check_context& ctx;
};

} // namespace

bool
check(const std::string& path, const Module& module, std::ostream& es)
{
  check_context ctx(path, module, es);

  checker c(ctx);

  c.run_all_checks();

  return ctx.get_error_count() == 0;
}
