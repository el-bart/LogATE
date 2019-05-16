#include "LogATE/Tree/Filter/BinarySplit.hpp"
#include "LogATE/Tree/Filter/AcceptAll.hpp"

namespace LogATE::Tree::Filter
{

BinarySplit::BinarySplit(Utils::WorkerThreadsShPtr workers, Name name, NodePtr matched):
  Node{ workers, Type{"BinarySplit"}, std::move(name), {} },
  matched_{ std::move(matched) },
  unmatched_{ But::makeSharedNN<AcceptAll>( workers, nonMatchingChildName() ) }
{ }


bool BinarySplit::insert(AnnotatedLog const& log)
{
  logs().withLock()->insert(log.log_);
  if( matched_->insert(log) )
    return true;
  unmatched_->insert(log);
  return true;
}


BinarySplit::Children BinarySplit::children() const
{
  return {matched_, unmatched_};
}


bool BinarySplit::remove(NodeShPtr node)
{
  BUT_THROW(ExplicitNodeRemoveNotSupported, "while trying to remove node: " << node->name().value_);
}


NodeShPtr BinarySplit::addImpl(NodePtr node)
{
  BUT_THROW(ExplicitNodeAddNotSupported, "while trying to add node: " << node->name().value_);
}

}
