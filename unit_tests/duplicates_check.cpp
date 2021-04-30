#include <gtest/gtest.h>

#include "duplicates_check.h"
#include "string_to_module.h"

TEST(DuplicatesCheck, DuplicateGlobalVar)
{
  auto module = StringToModule("int a = 2;\n"
                               "int a = 3;\n");

  auto duplicates = DuplicatesCheck::Run(*module);

  ASSERT_EQ(duplicates.size(), 1);

  EXPECT_EQ(duplicates[0].originalLocation.first_line, 1);
  EXPECT_EQ(duplicates[0].duplicateLocation.last_line, 2);
}

TEST(DuplicatesCheck, DuplicateGlobalVar2)
{
  auto module = StringToModule("int a() { return 0; }\n"
                               "int a = 3;\n");

  auto duplicates = DuplicatesCheck::Run(*module);

  ASSERT_EQ(duplicates.size(), 1);

  EXPECT_EQ(duplicates[0].originalLocation.first_line, 1);
  EXPECT_EQ(duplicates[0].duplicateLocation.last_line, 2);
}

TEST(DuplicatesCheck, DuplicateFunc)
{
  auto module = StringToModule("int a() { return 0; }\n"
                               "int a() { return 0; }\n");

  auto duplicates = DuplicatesCheck::Run(*module);

  ASSERT_EQ(duplicates.size(), 1);

  EXPECT_EQ(duplicates[0].originalLocation.first_line, 1);
  EXPECT_EQ(duplicates[0].duplicateLocation.last_line, 2);
}

TEST(DuplicatesCheck, DuplicateFunc2)
{
  auto module = StringToModule("int a = 0;\n"
                               "int a() { return 0; }\n");

  auto duplicates = DuplicatesCheck::Run(*module);

  ASSERT_EQ(duplicates.size(), 1);

  EXPECT_EQ(duplicates[0].originalLocation.first_line, 1);
  EXPECT_EQ(duplicates[0].duplicateLocation.last_line, 2);
}
