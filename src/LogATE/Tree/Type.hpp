#pragma once
#include <But/Mpl/FreeOperators.hpp>
#include <vector>

namespace LogATE::Tree
{

struct Type final { std::string value_; };

BUT_MPL_FREE_OPERATORS_COMPARE(Type, .value_)

}
