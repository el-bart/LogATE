#include <doctest/doctest.h>
#include "LogATE/Tree/Node.hpp"
#include "LogATE/Tree/Filter/AcceptAll.hpp"
#include "LogATE/TestHelpers.ut.hpp"
#include <But/Optional.hpp>
#include <utility>
#include <set>

using LogATE::Log;
using LogATE::SequenceNumber;
using LogATE::Tree::Node;
using LogATE::Tree::NodeShPtr;
using LogATE::Tree::Path;
using LogATE::makeKey;
using LogATE::makeLog;
using LogATE::Tree::Filter::AcceptAll;
using LogATE::Utils::WorkerThreads;
using LogATE::Utils::WorkerThreadsShPtr;

namespace
{

auto sampleNode(WorkerThreadsShPtr workers)
{
  return But::makeUniqueNN<AcceptAll>( std::move(workers), Node::Name{"whatever"} );
}

auto makeTree(WorkerThreadsShPtr workers)
{
  auto root = sampleNode(workers);
  for(auto i=0; i<2; ++i)
    root->add( sampleNode(workers) );
  root->children()[0]->add( sampleNode(workers) );
  return NodeShPtr{ std::move(root) };
}

using SnPair = std::pair<SequenceNumber, SequenceNumber>;

But::Optional<SnPair> nodeMinMaxId(NodeShPtr ptr)
{
  const auto ll = ptr->logs().withLock();
  if( ll->empty() )
    return {};
  return std::make_pair( ll->first().sequenceNumber(), ll->last().sequenceNumber() );
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
  const auto workers = But::makeSharedNN<WorkerThreads>();
  auto root = makeTree(workers);
  REQUIRE( treeMinMaxId(root).size() == 0 );

  for(auto id=0; id<10; ++id)
    root->insert( LogATE::AnnotatedLog{ makeLog(id) } );
  {
    const auto allMinMax = treeMinMaxId(root);
    REQUIRE( allMinMax.size() == 1 );
    CHECK( allMinMax.begin()->first == SequenceNumber{0} );
    CHECK( allMinMax.begin()->second == SequenceNumber{9} );
  }

  root->pruneUpTo( makeKey(2) );
  workers->waitForAll();
  {
    const auto allMinMax = treeMinMaxId(root);
    REQUIRE( allMinMax.size() == 1 );
    CHECK( allMinMax.begin()->first == SequenceNumber{2} );
    CHECK( allMinMax.begin()->second == SequenceNumber{9} );
  }

  root->pruneUpTo( makeKey(8) );
  workers->waitForAll();
  {
    const auto allMinMax = treeMinMaxId(root);
    REQUIRE( allMinMax.size() == 1 );
    CHECK( allMinMax.begin()->first == SequenceNumber{8} );
    CHECK( allMinMax.begin()->second == SequenceNumber{9} );
  }

  root->pruneUpTo( makeKey(69) );
  workers->waitForAll();
  {
    const auto allMinMax = treeMinMaxId(root);
    REQUIRE( allMinMax.size() == 0 );
  }
}


auto sampleTrimNode(WorkerThreadsShPtr workers, Node::TrimFields tf)
{
  return But::makeUniqueNN<AcceptAll>( std::move(workers), Node::Name{"whatever"}, std::move(tf) );
}

TEST_CASE("trimm nodes are inherited from parent node")
{
  const auto workers = But::makeSharedNN<WorkerThreads>();

  auto a = sampleTrimNode( workers, Node::TrimFields{ Path::parse("xxx") } );
  CHECK( a->trimFields().size() == 1 );

  auto b = sampleTrimNode( workers, Node::TrimFields{ Path::parse("foo"), Path::parse("bar") } );
  CHECK( b->trimFields().size() == 2 );

  auto c = sampleTrimNode( workers, Node::TrimFields{ Path::parse("one"), Path::parse("more"), Path::parse("path") } );
  CHECK( c->trimFields().size() == 3 );

  const auto cs = b->add( std::move(c) );
  CHECK( cs->trimFields().size() == 3+2 );
  CHECK( b->trimFields().size() == 2 );
  CHECK( a->trimFields().size() == 1 );

  const auto bs = a->add( std::move(b) );
  CHECK( cs->trimFields().size() == 3+2+1 );
  CHECK( bs->trimFields().size() == 2+1 );
  CHECK( a->trimFields().size() == 1 );
}

}

}
