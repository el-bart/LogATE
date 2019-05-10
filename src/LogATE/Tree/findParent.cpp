#include "LogATE/Tree/findParent.hpp"

namespace LogATE::Tree
{

std::shared_ptr<Node> findParent(NodeShPtr parent, NodeShPtr node)
{
  for(auto& e: parent->children())
  {
    if( e.get() == node.get() )
      return parent.underlyingPointer();
    auto ptr = findParent(e, node);
    if(ptr)
      return ptr;
  }
  return {};
}

}
