#pragma once

#include <iosfwd>

enum class TypeID
{
  Void,
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

bool
IsVecOrMat(TypeID typeID) noexcept;

bool
IsVecI(TypeID typeID) noexcept;

std::ostream&
operator<<(std::ostream&, TypeID typeID);

enum class Variability
{
  Unbound,
  Uniform,
  Varying
};

std::ostream&
operator<<(std::ostream&, Variability variability);

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

  bool operator!=(const Type& other) const noexcept
  {
    return (mTypeID != other.mTypeID) || (mVariability != other.mVariability);
  }

  Variability GetVariability() const noexcept { return mVariability; }

  bool IsVaryingOrUnbound() const noexcept
  {
    return (mVariability == Variability::Varying) ||
           (mVariability == Variability::Unbound);
  }

  bool IsUniform() const noexcept
  {
    return mVariability == Variability::Uniform;
  }

private:
  TypeID mTypeID;

  Variability mVariability;
};

std::ostream&
operator<<(std::ostream&, const Type& type);
