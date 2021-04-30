#pragma once

#include "location.h"

#include <vector>

class DiagObserver;
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

  /// @return True for success, false if there was an error.
  static bool Check(const Module&, DiagObserver&);
};
