#include "common.h"

#include <ostream>

std::ostream&
operator<<(std::ostream& os, const location& l)
{
  return os << l.first_line << ':' << l.first_column;
}

std::ostream&
operator<<(std::ostream& os, TypeID typeID)
{
  switch (typeID) {
    case TypeID::Bool:
      return os << "bool";
    case TypeID::Int:
      return os << "int";
    case TypeID::Float:
      return os << "float";
    case TypeID::Vec2:
      return os << "vec2";
    case TypeID::Vec3:
      return os << "vec3";
    case TypeID::Vec4:
      return os << "vec4";
    case TypeID::Vec2i:
      return os << "vec2i";
    case TypeID::Vec3i:
      return os << "vec3i";
    case TypeID::Vec4i:
      return os << "vec4i";
    case TypeID::Mat2:
      return os << "mat2";
    case TypeID::Mat3:
      return os << "mat3";
    case TypeID::Mat4:
      return os << "mat4";
  }

  return os;
}

void
scope::def(const Var* v)
{
  this->vars.emplace(*v->name.identifier, v);
}

Type
var_ref::GetType() const
{
  assert(this->resolved_var != nullptr);

  return this->resolved_var->mType;
}

bool
var_ref::references_global_var() const
{
  return resolved_var ? resolved_var->is_global : false;
}

swizzle::swizzle(const std::string& pattern)
{
  for (auto c : pattern) {
    switch (c) {
      case 'r':
      case 'x':
        this->member_indices.emplace_back(0);
        break;
      case 'g':
      case 'y':
        this->member_indices.emplace_back(1);
        break;
      case 'b':
      case 'z':
        this->member_indices.emplace_back(2);
        break;
      case 'a':
      case 'w':
        this->member_indices.emplace_back(3);
        break;
      default:
        this->error_flag = true;
        break;
    }
  }

  this->error_flag |= this->member_indices.size() > 4;
}

namespace {

Type
get_float_vector_member_type(const std::string& pattern)
{
  swizzle s(pattern);

  assert(!s.error_flag);

  switch (s.member_indices.size()) {
    case 1:
      break;
    case 2:
      return TypeID::Vec2;
    case 3:
      return TypeID::Vec3;
    case 4:
      return TypeID::Vec4;
  }

  return TypeID::Float;
}

Type
get_int_vector_member_type(const std::string& pattern)
{
  swizzle s(pattern);

  assert(!s.error_flag);

  switch (s.member_indices.size()) {
    case 1:
      break;
    case 2:
      return TypeID::Vec2i;
    case 3:
      return TypeID::Vec3i;
    case 4:
      return TypeID::Vec4i;
  }

  return TypeID::Int;
}

} // namespace

Type
member_expr::GetType() const
{
  switch (base_expr->GetType().ID()) {
    case TypeID::Int:
    case TypeID::Float:
    case TypeID::Bool:
    case TypeID::Mat2:
    case TypeID::Mat3:
    case TypeID::Mat4:
      // These don't have members and this should not be reachable because it
      // will trigger an error.
      assert(false);
      return base_expr->GetType();
    case TypeID::Vec2:
    case TypeID::Vec3:
    case TypeID::Vec4:
      return get_float_vector_member_type(*this->member_name.identifier);
    case TypeID::Vec2i:
    case TypeID::Vec3i:
    case TypeID::Vec4i:
      return get_int_vector_member_type(*this->member_name.identifier);
  }

  assert(false);

  return base_expr->GetType();
}

const Var*
symtab::find_var(const std::string& name) const
{
  for (auto scope_it = this->local_scopes.rbegin();
       scope_it != this->local_scopes.rend();
       scope_it++) {

    auto result = scope_it->vars.find(name);

    if (result != scope_it->vars.end())
      return result->second;
  }

  for (const auto& global : this->prg.vars) {
    if (*global->name.identifier == name)
      return global.get();
  }

  return nullptr;
}

namespace {

class program_state_requirement_checker final : public stmt_visitor
{
public:
  bool get_requirement_flag() const noexcept { return this->requirement_flag; }

  void visit(const decl_stmt& s) override
  {
    if (s.v->init_expr)
      this->requirement_flag |= s.v->init_expr->references_global_var();
  }

  void visit(const return_stmt& s) override
  {
    this->requirement_flag |= s.return_value->references_global_var();
  }

  void visit(const compound_stmt& s) override
  {
    for (const auto& inner_stmt : *s.stmts)
      inner_stmt->accept(*this);
  }

private:
  bool requirement_flag = false;
};

} // namespace

bool
func::requires_program_state() const
{
  program_state_requirement_checker checker;

  this->body->accept(checker);

  return checker.get_requirement_flag();
}

bool
func::is_main() const noexcept
{
  return *this->name.identifier == "main";
}
