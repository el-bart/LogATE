#include <doctest/doctest.h>
#include "LogATE/Tree/SimpleNode.hpp"
#include "LogATE/Tree/TestHelpers.ut.hpp"
#include "LogATE/TestPrints.ut.hpp"

using LogATE::Tree::Node;
using LogATE::Tree::SimpleNode;
using LogATE::Tree::makeLog;
using LogATE::Tree::makeSns;
using LogATE::Tree::logs2sns;
using SN = LogATE::SequenceNumber;

namespace
{

TEST_SUITE("Tree::SimpleNode")
{

struct TestFilter: SimpleNode
{
  TestFilter():
    SimpleNode(Node::Type{"test"}, Node::Name{"testing"}, {})
  { }

private:
  bool matches(LogATE::Log const& log) const override
  {
    return ( log.sn_.value_ % 2 ) == 0;
  }
};

struct Fixture
{
  auto allSns() const { return LogATE::Tree::allSns( tf_.logs() ); }

  TestFilter tf_;
};

TEST_CASE_FIXTURE(Fixture, "xxxx")
{
  // TODO
}

}

}
