#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <assert.h>
#include <stdint.h>

class Parser;

struct Var;

struct program;

struct scope final
{
  std::map<std::string, const Var*> vars;

  void def(const Var* v);
};

struct symtab final
{
  // @note Does not include global symbols.
  std::vector<scope> local_scopes;

  // This is used for resolving global declarations.
  const program& prg;

  symtab(const program& prg_)
    : prg(prg_)
  {}

  void enter_scope() { this->local_scopes.emplace_back(); }

  void exit_scope()
  {
    assert(!this->local_scopes.empty());
    this->local_scopes.pop_back();
  }

  void def(const Var* v)
  {
    assert(this->local_scopes.size() > 0);

    this->local_scopes.back().def(v);
  }

  const Var* find_var(const std::string& name) const;
};

struct location final
{
  int first_line = 1;

  int first_column = 1;

  int last_line = 1;

  int last_column = 1;
};

std::ostream&
operator<<(std::ostream&, const location& l);

enum class TypeID
{
  Int,
  Bool,
  Float,
  Vec2,
  Vec3,
  Vec4,
  Vec2i,
  Vec3i,
  Vec4i,
  Mat2,
  Mat3,
  Mat4
};

std::ostream&
operator<<(std::ostream&, TypeID typeID);

enum class Variability
{
  Unbound,
  Uniform,
  Varying
};

class Type final
{
public:
  Type(TypeID typeID, Variability variability = Variability::Unbound)
    : mTypeID(typeID)
    , mVariability(variability)
  {}

  TypeID ID() const noexcept { return mTypeID; }

  bool operator==(TypeID typeID) const noexcept { return mTypeID == typeID; }

  bool operator!=(TypeID typeID) const noexcept { return mTypeID != typeID; }

  bool operator==(const Type& other) const noexcept
  {
    return (mTypeID == other.mTypeID) && (mVariability == other.mVariability);
  }

private:
  TypeID mTypeID;

  Variability mVariability;
};

struct decl_name final
{
  std::unique_ptr<std::string> identifier;

  location loc;

  decl_name(std::string* id, const location& l)
    : identifier(id)
    , loc(l)
  {}
};

struct int_literal;
struct bool_literal;
struct float_literal;
struct binary_expr;
struct group_expr;
struct var_ref;
struct unary_expr;
struct type_constructor;
struct member_expr;

class expr_visitor
{
public:
  virtual ~expr_visitor() = default;

  virtual void visit(const bool_literal&) = 0;

  virtual void visit(const int_literal&) = 0;

  virtual void visit(const float_literal&) = 0;

  virtual void visit(const binary_expr&) = 0;

  virtual void visit(const unary_expr&) = 0;

  virtual void visit(const group_expr&) = 0;

  virtual void visit(const var_ref&) = 0;

  virtual void visit(const type_constructor&) = 0;

  virtual void visit(const member_expr&) = 0;
};

struct expr
{
  expr(const location& l)
    : loc(l)
  {}

  virtual ~expr() = default;

  virtual void accept(expr_visitor& v) const = 0;

  virtual bool references_global_var() const = 0;

  virtual void resolve(const symtab&, std::vector<const decl_name*>&) = 0;

  virtual Type GetType() const = 0;

  location loc;
};

using expr_list = std::vector<std::unique_ptr<expr>>;

struct literal_expr : public expr
{
  using expr::expr;

  virtual ~literal_expr() = default;

  void resolve(const symtab&, std::vector<const decl_name*>&) override {}

  bool references_global_var() const override { return false; }
};

struct int_literal final : public literal_expr
{
  uint64_t value;

  int_literal(uint64_t v, const location& l)
    : literal_expr(l)
    , value(v)
  {}

  void accept(expr_visitor& v) const override { v.visit(*this); }

  Type GetType() const override
  {
    return Type(TypeID::Int, Variability::Uniform);
  }
};

struct bool_literal final : public literal_expr
{
  bool value;

  void accept(expr_visitor& v) const override { v.visit(*this); }

  Type GetType() const override
  {
    return Type(TypeID::Bool, Variability::Uniform);
  }
};

struct float_literal final : public literal_expr
{
  double value;

  float_literal(double v, const location& l)
    : literal_expr(l)
    , value(v)
  {}

  void accept(expr_visitor& v) const override { v.visit(*this); }

  Type GetType() const override
  {
    return Type(TypeID::Float, Variability::Uniform);
  }
};

struct Var;

struct var_ref final : public expr
{
  decl_name name;

  const Var* resolved_var = nullptr;

  var_ref(decl_name&& n)
    : expr(n.loc)
    , name(std::move(n))
  {}

  void accept(expr_visitor& v) const override { v.visit(*this); }

  Type GetType() const override;

  bool references_global_var() const override;

  void resolve(const symtab& s,
               std::vector<const decl_name*>& unresolved) override
  {
    assert(this->resolved_var == nullptr);

    this->resolved_var = s.find_var(*name.identifier);

    if (!this->resolved_var)
      unresolved.emplace_back(&name);
  }
};

struct group_expr final : public expr
{
  group_expr(expr* e)
    : expr(e->loc)
    , mInnerExpr(e)
  {}

  void accept(expr_visitor& v) const override { v.visit(*this); }

  Type GetType() const override { return mInnerExpr->GetType(); }

  bool references_global_var() const override
  {
    return mInnerExpr->references_global_var();
  }

  void resolve(const symtab& s,
               std::vector<const decl_name*>& unresolved) override
  {
    mInnerExpr->resolve(s, unresolved);
  }

  std::unique_ptr<expr> mInnerExpr;
};

struct unary_expr final : public expr
{
  enum class kind
  {
    LOGICAL_NOT,
    BITWISE_NOT,
    NEGATE
  };

  unary_expr(expr* e, kind k_, const location& loc_)
    : expr(loc_)
    , mBaseExpr(e)
    , k(k_)
  {}

  void accept(expr_visitor& v) const override { v.visit(*this); }

  Type GetType() const override { return mBaseExpr->GetType(); }

  bool references_global_var() const override
  {
    return this->mBaseExpr->references_global_var();
  }

  void resolve(const symtab& s,
               std::vector<const decl_name*>& unresolved) override
  {
    mBaseExpr->resolve(s, unresolved);
  }

  std::unique_ptr<expr> mBaseExpr;

  kind k;
};

struct binary_expr final : public expr
{
  std::unique_ptr<expr> left;

  std::unique_ptr<expr> right;

  enum class kind
  {
    ADD,
    SUB,
    MUL,
    DIV,
    MOD
  };

  kind k;

  binary_expr(expr* l, expr* r, kind k_, const location& loc)
    : expr(loc)
    , left(l)
    , right(r)
    , k(k_)
  {}

  void accept(expr_visitor& v) const override { v.visit(*this); }

  Type GetType() const override
  {
    return common_type(left->GetType(), right->GetType());
  }

  static Type common_type(Type l, Type r) noexcept
  {
    // TODO
    (void)r;
    return l;
  }

  bool references_global_var() const override
  {
    return left->references_global_var() || right->references_global_var();
  }

  void resolve(const symtab& s,
               std::vector<const decl_name*>& unresolved) override
  {
    left->resolve(s, unresolved);

    right->resolve(s, unresolved);
  }
};

struct type_constructor final : public expr
{
  Type mType;

  std::unique_ptr<expr_list> args;

  type_constructor(Type type, expr_list* args_, const location& loc)
    : expr(loc)
    , mType(type)
    , args(args_)
  {}

  void accept(expr_visitor& v) const override { v.visit(*this); }

  Type GetType() const override { return mType; }

  bool references_global_var() const override
  {
    for (const auto& a : *this->args) {
      if (a->references_global_var())
        return true;
    }

    return false;
  }

  void resolve(const symtab& s,
               std::vector<const decl_name*>& unresolved) override
  {
    for (const auto& a : *this->args)
      a->resolve(s, unresolved);
  }
};

struct swizzle final
{
  std::vector<std::size_t> member_indices;

  bool error_flag = false;

  swizzle(const std::string& pattern);
};

struct member_expr final : public expr
{
  std::unique_ptr<expr> base_expr;

  decl_name member_name;

  member_expr(expr* base_, decl_name&& member, const location& loc)
    : expr(loc)
    , base_expr(base_)
    , member_name(std::move(member))
  {}

  void accept(expr_visitor& v) const override { v.visit(*this); }

  void resolve(const symtab& s,
               std::vector<const decl_name*>& unresolved) override
  {
    base_expr->resolve(s, unresolved);
  }

  bool references_global_var() const override
  {
    return base_expr->references_global_var();
  }

  Type GetType() const override;
};

struct compound_stmt;
struct decl_stmt;
struct return_stmt;

class stmt_visitor
{
public:
  virtual ~stmt_visitor() = default;

  virtual void visit(const compound_stmt&) = 0;

  virtual void visit(const decl_stmt&) = 0;

  virtual void visit(const return_stmt&) = 0;
};

struct stmt
{
  virtual ~stmt() = default;

  virtual void accept(stmt_visitor& v) const = 0;
};

using stmt_list = std::vector<std::unique_ptr<stmt>>;

struct compound_stmt final : public stmt
{
  std::unique_ptr<stmt_list> stmts;

  compound_stmt(stmt_list* stmts_)
    : stmts(stmts_)
  {}

  void accept(stmt_visitor& v) const override { v.visit(*this); }
};

struct decl_stmt final : public stmt
{
  std::unique_ptr<Var> v;

  decl_stmt(Var* v_)
    : v(v_)
  {}

  void accept(stmt_visitor& v) const override { v.visit(*this); }
};

struct return_stmt final : public stmt
{
  std::unique_ptr<expr> return_value;

  return_stmt(expr* rv)
    : return_value(rv)
  {}

  void accept(stmt_visitor& v) const override { v.visit(*this); }
};

using param_list = std::vector<std::unique_ptr<Var>>;

struct func final
{
  Type mReturnType;

  decl_name name;

  std::unique_ptr<param_list> params;

  std::unique_ptr<stmt> body;

  func(Type returnType, decl_name&& n, param_list* p, stmt* b)
    : mReturnType(returnType)
    , name(std::move(n))
    , params(p)
    , body(b)
  {}

  bool requires_program_state() const;

  bool is_main() const noexcept;
};

struct Var final
{
  Type mType;

  decl_name name;

  std::unique_ptr<expr> init_expr;

  bool is_global = false;

  Var(Type type, decl_name&& n, expr* e)
    : mType(type)
    , name(std::move(n))
    , init_expr(e)
  {}
};

struct program final
{
  std::vector<std::unique_ptr<func>> funcs;

  // Global variables
  std::vector<std::unique_ptr<Var>> vars;
};

union semantic_value
{
  program* as_program;

  Var* as_var;

  param_list* as_param_list;

  func* as_func;

  stmt_list* as_stmt_list;

  stmt* as_stmt;

  expr_list* as_expr_list;

  expr* as_expr;

  std::string* as_string;

  TypeID asTypeID;

  Variability asVariability;

  Type* asType;

  uint64_t as_int;

  double as_float;

  char32_t invalid_char;
};

class parse_observer
{
public:
  virtual ~parse_observer() = default;

  virtual void on_program(const program& prg) = 0;

  virtual void on_syntax_error(const location& loc, const char* msg) = 0;
};

#define YY_DECL int yylex(semantic_value* val, location* loc)

YY_DECL;
