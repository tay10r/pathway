#pragma once

#include "type.h"

#include <string>

template<typename Derived>
class TypeEnvironment
{
public:
  virtual ~TypeEnvironment() = default;

  const Type* FindVar(const std::string& name) const
  {
    return static_cast<const Derived*>(this)->FindVarImpl(name);
  }
};
