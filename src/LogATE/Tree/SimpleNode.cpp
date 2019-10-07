#include "LogATE/Tree/SimpleNode.hpp"

namespace LogATE::Tree
{

bool SimpleNode::insert(AnnotatedLog const& log)
{
  if( not matches(log) )
    return false;
  logs()->withLock()->insert( log.log() );
  insertToChildren(log);
  return true;
}


SimpleNode::Children SimpleNode::children() const
{
  const Lock lock{mutex_};
  return children_;
}


std::shared_ptr<Node> SimpleNode::remove(NodeShPtr node)
{
  const Lock lock{mutex_};
  for(auto it=begin(children_); it!=end(children_); ++it)
    if( it->get() == node.get() )
    {
      auto ptr = *it;
      children_.erase(it);
      return ptr.underlyingPointer();
    }
  return {};
}


NodeShPtr SimpleNode::addImpl(NodePtr node)
{
  auto shared = NodeShPtr{std::move(node)};
  {
    const Lock lock{mutex_};
    children_.push_back(shared);
  }
  passAllLogsToChild(shared);
  return shared;
}


namespace
{
inline void insertToChild(NodeShPtr const& child, AnnotatedLog const& log)
{
  try
  {
    child->insert(log);
  }
  catch(...)
  {
    // whatever...
  }
}
}


void SimpleNode::insertToChildren(AnnotatedLog const& log)
{
  for(auto& c: children_)
    insertToChild(c, log);
}


namespace
{
void insertChunk(NodeWeakPtr weakNode, std::vector<Log> chunk)
{
  const auto sp = weakNode.lock();  // weak pointer is crucial here, to avoid cyclic dependencies (node does keep thread pool!)
  if(not sp)
    return;
  for(auto&& log: std::move(chunk))
    insertToChild(NodeShPtr{sp}, AnnotatedLog{log});
}


But::Optional<Log::Key> getFirstKey(But::NotNullShared<Logs> const& logs)
{
  const auto ll = logs->withLock();
  if( ll->empty() )
    return {};
  return ll->first().key();
}


using WorkersWeakPtr = std::weak_ptr<Utils::WorkerThreads>;

void enqueueInsertionOfAllChunks(NodeWeakPtr weakNode, WorkersWeakPtr weakWorkers, But::NotNullShared<Logs> logs)
{
  const auto lastKeyOpt = getFirstKey(logs);
  if(not lastKeyOpt)
    return;
  auto lastKey = *lastKeyOpt;
  constexpr auto chunkSize = 100'000;

  auto done = false;
  while(not done)
  {
    auto workers = weakWorkers.lock();
    if(not workers)
      return;
    const auto sp = weakNode.lock();
    if(not sp)
      return;

    auto chunk = logs->withLock()->from(lastKey, chunkSize);
    if( chunk.empty() )
      return;
    if( chunk.size() == 1 )
      done = true;

    lastKey = chunk.back().key();
    workers->enqueueFilter( sp->type(), sp->name(), [weakNode, logs=std::move(chunk)] { insertChunk(weakNode, std::move(logs)); } );
  }
}
}


void SimpleNode::passAllLogsToChild(NodeShPtr child)
{
  workers_->enqueueFilter( type(), name(),
                          [logsPtr=logs(),
                           node=NodeWeakPtr{child.underlyingPointer()},
                           workers=WorkersWeakPtr{workers_.underlyingPointer()}]
                          {
                            enqueueInsertionOfAllChunks(node, workers, logsPtr);
                          } );
}

}
