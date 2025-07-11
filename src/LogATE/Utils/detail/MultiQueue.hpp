#pragma once
#include "LogATE/Utils/detail/Task.hpp"
#include "LogATE/Tree/NodeName.hpp"
#include "LogATE/Tree/NodeType.hpp"
#include <But/Threading/BasicLockable.hpp>
#include <But/Threading/LockProxyProvider.hpp>
#include <But/Optional.hpp>
#include <But/NotNull.hpp>
#include <queue>

namespace LogATE::Utils::detail
{

struct Queue: But::Threading::BasicLockable<Queue>,
              But::Threading::LockProxyProvider<Queue>
{
  using lock_type = std::unique_lock<Queue>;
  using TaskPtr = std::unique_ptr<TaskBase>;
  using value_type = TaskPtr;
  using queue_type = std::queue<value_type>;

  auto size() const
  {
    BUT_ASSERT( locked() );
    auto total = ui_.size() + batch_.size();
    for(auto& q: filters_)
      total += q.queue_.size();
    return total;
  }

  auto empty() const { return size() == 0u; }

  void waitForNonEmpty(lock_type& lock)
  {
    BUT_ASSERT( locked() );
    if( not empty() )
      return;
    newElement_.wait( lock, [this] { return not this->empty(); } );
  }

  value_type dequeue()
  {
    BUT_ASSERT( locked() );
    BUT_ASSERT( not empty() );
    if( not ui_.empty() )
      return dequeue(ui_);
    if( auto e = dequeue(filters_) )
      return std::move(*e);
    BUT_ASSERT( not batch_.empty() );
    return dequeue(batch_);
  }

  void enqueueUi(value_type e)
  {
    BUT_ASSERT( locked() );
    ui_.push( std::move(e) );
    newElement_.notify_all();
  }
  void enqueueBatch(value_type e)
  {
    BUT_ASSERT( locked() );
    batch_.push( std::move(e) );
    newElement_.notify_all();
  }
  void enqueueFilter(value_type e, Tree::NodeType const& type, Tree::NodeName const& name)
  {
    BUT_ASSERT( locked() );
    auto& q = getQueue(type, name);
    q.push( std::move(e) );
    newElement_.notify_all();
  }

private:
  struct Entry
  {
    Entry(Tree::NodeType type, Tree::NodeName name, queue_type queue):
      type_{ std::move(type) },
      name_{ std::move(name) },
      queue_{ std::move(queue) }
    { }

    // as of C++17 std::deque(&&) is not noexcept, thus w/o this declaration we have a problem...
    Entry(Entry&&) = default;
    Entry& operator=(Entry&&) = default;

    Tree::NodeType type_;
    Tree::NodeName name_;
    queue_type     queue_;
  };

  value_type dequeue(queue_type& q) const
  {
    BUT_ASSERT( not q.empty() );
    auto tmp = std::move( q.front() );
    q.pop();
    return tmp;
  }

  But::Optional<value_type> dequeue(std::vector<Entry>& qs)
  {
    auto max = qs.size();
    for(auto i=0u; i<max; ++i)
    {
      const auto index = nextFilterIndex();
      if( not qs[index].queue_.empty() )
        return dequeue(qs[index].queue_);
    }
    return {};
  }

  queue_type& getQueue(Tree::NodeType const& type, Tree::NodeName const& name)
  {
    for(auto it=begin(filters_); it!=end(filters_); ++it)
      if( it->type_ == type && it->name_ == name )
        return it->queue_;
    filters_.emplace_back( Entry{type, name, {}} );
    return filters_.back().queue_;
  }

  size_t nextFilterIndex()
  {
    auto tmp = nextFilter_;
    if( nextFilter_ + 1u >= filters_.size() )
      nextFilter_ = 0;
    else
      ++nextFilter_;
    return tmp;
  }

  std::condition_variable_any newElement_;
  queue_type ui_;
  size_t nextFilter_{0};
  std::vector<Entry> filters_;
  queue_type batch_;
};

}
