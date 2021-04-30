#pragma once

#include "module.h"

#include <memory>
#include <string>

auto
StringToModule(const std::string& source) -> std::unique_ptr<Module>;
