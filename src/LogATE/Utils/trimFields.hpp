#pragma once
#include "LogATE/Log.hpp"
#include "LogATE/Tree/Node.hpp"

namespace LogATE::Utils
{
Log trimFields(Log const& in, Tree::Node::TrimFields const& tf);
}
