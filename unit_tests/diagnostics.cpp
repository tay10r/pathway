#include <gtest/gtest.h>

#include "diagnostics.h"

TEST(Diagnostics, GetClippedLocation)
{
  Location loc1{ 1, 7, 1, 7 };

  auto range1 = ConsoleDiagObserver::GetClippedLocation(1, " line 1", loc1);

  EXPECT_EQ(range1.index, 6);
  EXPECT_EQ(range1.length, 1);

  Location loc2{ 2, 2, 3, 5 };

  auto range2 = ConsoleDiagObserver::GetClippedLocation(2, " line 2", loc2);
  auto range3 = ConsoleDiagObserver::GetClippedLocation(3, " line 3", loc2);

  EXPECT_EQ(range2.index, 1);
  EXPECT_EQ(range2.length, 6);

  EXPECT_EQ(range3.index, 0);
  EXPECT_EQ(range3.length, 5);
}

TEST(Diagnostics, GetLineView)
{
  std::string_view lines = " line 1\n"
                           " line 2\n"
                           " line 3\n";

  auto ln1 = ConsoleDiagObserver::GetLineView(1, lines);
  auto ln2 = ConsoleDiagObserver::GetLineView(2, lines);
  auto ln3 = ConsoleDiagObserver::GetLineView(3, lines);

  EXPECT_EQ(ln1, " line 1");
  EXPECT_EQ(ln2, " line 2");
  EXPECT_EQ(ln3, " line 3");
}
