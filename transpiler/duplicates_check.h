#pragma once

#include "location.h"

#include <vector>

class Module;

class DuplicatesCheck final
{
public:
  struct Duplicate final
  {
    Location originalLocation;

    Location duplicateLocation;
  };

  static auto Run(const Module&) -> std::vector<Duplicate>;
};
