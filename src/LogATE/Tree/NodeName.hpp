#pragma once
#include <But/Mpl/FreeOperators.hpp>
#include <string>

namespace LogATE::Tree
{

struct NodeName final { std::string value_; };

BUT_MPL_FREE_OPERATORS_COMPARE(NodeName, .value_)

}
