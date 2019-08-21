#pragma once
#include <But/Mpl/FreeOperators.hpp>
#include <string>

namespace LogATE::Tree
{

struct NodeType final { std::string value_; };

BUT_MPL_FREE_OPERATORS_COMPARE(NodeType, .value_)

}
