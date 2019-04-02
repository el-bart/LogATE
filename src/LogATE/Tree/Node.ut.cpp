#include <doctest/doctest.h>
#include "LogATE/Tree/Node.hpp"
#include "LogATE/Tree/Filter/AcceptAll.hpp"
#include "LogATE/Tree/TestHelpers.ut.hpp"
#include <But/Optional.hpp>
#include <utility>
#include <set>

using LogATE::json2log;
using LogATE::Log;
using LogATE::SequenceNumber;
using LogATE::Tree::Node;
using LogATE::Tree::NodeShPtr;
using LogATE::Tree::makeLog;
using LogATE::Tree::Filter::AcceptAll;

namespace
{

auto sampleNode()
{
  return But::makeUniqueNN<AcceptAll>(Node::Name{"whatever"});
}

auto makeTree()
{
  auto root = sampleNode();
  for(auto i=0; i<2; ++i)
    root->add( sampleNode() );
  root->children()[0]->add( sampleNode() );
  return NodeShPtr{ std::move(root) };
}

using SnPair = std::pair<SequenceNumber, SequenceNumber>;

But::Optional<SnPair> nodeMinMaxId(NodeShPtr ptr)
{
  const auto ll = ptr->logs().withLock();
  if( ll->empty() )
    return {};
  return std::make_pair( ll->first().sn_, ll->last().sn_ );
}

void treeMinMaxId(NodeShPtr root, std::set<SnPair>& out)
{
  const auto opt = nodeMinMaxId(root);
  if(opt)
    out.insert(*opt);
  for(auto node: root->children())
    treeMinMaxId(node, out);
}

auto treeMinMaxId(NodeShPtr root)
{
  std::set<SnPair> out;
  treeMinMaxId(root, out);
  return out;
}

TEST_SUITE("Tree::Node")
{

TEST_CASE("prunning logs works recursively")
{
  auto root = makeTree();
  REQUIRE( treeMinMaxId(root).size() == 0 );

  for(auto id=0; id<10; ++id)
    root->insert( makeLog(id) );
  {
    const auto allMinMax = treeMinMaxId(root);
    REQUIRE( allMinMax.size() == 1 );
    CHECK( allMinMax.begin()->first == SequenceNumber{0} );
    CHECK( allMinMax.begin()->second == SequenceNumber{9} );
  }

  root->pruneUpTo( SequenceNumber{2} );
  {
    const auto allMinMax = treeMinMaxId(root);
    REQUIRE( allMinMax.size() == 1 );
    CHECK( allMinMax.begin()->first == SequenceNumber{2} );
    CHECK( allMinMax.begin()->second == SequenceNumber{9} );
  }

  root->pruneUpTo( SequenceNumber{8} );
  {
    const auto allMinMax = treeMinMaxId(root);
    REQUIRE( allMinMax.size() == 1 );
    CHECK( allMinMax.begin()->first == SequenceNumber{8} );
    CHECK( allMinMax.begin()->second == SequenceNumber{9} );
  }

  root->pruneUpTo( SequenceNumber{69} );
  {
    const auto allMinMax = treeMinMaxId(root);
    REQUIRE( allMinMax.size() == 0 );
  }
}

}

}
