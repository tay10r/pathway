#include <gtest/gtest.h>

#include "duplicates_check.h"
#include "string_to_module.h"

TEST(DuplicatesCheck, DuplicateGlobal)
{
  auto module = StringToModule("int a = 2;\n"
                               "int a = 3;\n");

  auto duplicates = DuplicatesCheck::Run(*module);

  ASSERT_EQ(duplicates.size(), 1);
}
