#pragma once

#include <iosfwd>

struct program;

bool
check(const std::string& path, const program& prg, std::ostream& os);
