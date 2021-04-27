#pragma once

#include "location.h"

#include <memory>
#include <string>

class DeclName final
{
public:
  DeclName(std::string* identifier, const Location& location)
    : mIdentifier(identifier)
    , mLocation(location)
  {}

  const std::string& Identifier() const noexcept { return *mIdentifier; }

  const Location& GetLocation() const noexcept { return mLocation; }

private:
  std::unique_ptr<std::string> mIdentifier;

  Location mLocation;
};
