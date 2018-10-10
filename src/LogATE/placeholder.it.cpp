#include <gtest/gtest.h>
#include <LogATE/placeholder.hpp>

namespace
{

struct PlaceholderTests: public testing::Test
{
};


TEST_F(PlaceholderTests, ValidIsOk)
{
  EXPECT_EQ(42, answer());
}

}
