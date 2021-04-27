#include "type.h"

#include <ostream>

bool
IsVecOrMat(TypeID typeID) noexcept
{
  switch (typeID) {
    case TypeID::Void:
    case TypeID::Int:
    case TypeID::Bool:
    case TypeID::Float:
    case TypeID::Vec2i:
    case TypeID::Vec3i:
    case TypeID::Vec4i:
      break;
    case TypeID::Vec2:
    case TypeID::Vec3:
    case TypeID::Vec4:
    case TypeID::Mat2:
    case TypeID::Mat3:
    case TypeID::Mat4:
      return true;
  }

  return false;
}

bool
IsVecI(TypeID typeID) noexcept
{
  switch (typeID) {
    case TypeID::Void:
    case TypeID::Int:
    case TypeID::Bool:
    case TypeID::Float:
    case TypeID::Vec2:
    case TypeID::Vec3:
    case TypeID::Vec4:
    case TypeID::Mat2:
    case TypeID::Mat3:
    case TypeID::Mat4:
      break;
    case TypeID::Vec2i:
    case TypeID::Vec3i:
    case TypeID::Vec4i:
      return true;
  }

  return false;
}

std::ostream&
operator<<(std::ostream& os, TypeID typeID)
{
  switch (typeID) {
    case TypeID::Void:
      return os << "void";
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

std::ostream&
operator<<(std::ostream& os, Variability variability)
{
  switch (variability) {
    case Variability::Unbound:
      return os << "unbound";
    case Variability::Varying:
      return os << "varying";
    case Variability::Uniform:
      return os << "uniform";
  }

  return os;
}

std::ostream&
operator<<(std::ostream& os, const Type& type)
{
  return os << type.GetVariability() << ' ' << type.ID();
}
