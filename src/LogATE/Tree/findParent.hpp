#pragma once
#include "LogATE/Tree/Node.hpp"

namespace LogATE::Tree
{
std::shared_ptr<Node> findParent(NodeShPtr parent, NodeShPtr node);
}
