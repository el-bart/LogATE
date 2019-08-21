#pragma once
#include <But/Mpl/FreeOperators.hpp>
#include <vector>

namespace LogATE::Tree
{

struct Name final { std::string value_; };

BUT_MPL_FREE_OPERATORS_COMPARE(Name, .value_)

}
