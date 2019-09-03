#pragma once
#include "LogATE/Utils/detail/Task.hpp"
#include "LogATE/Utils/detail/MultiQueue.hpp"
#include "LogATE/Tree/NodeName.hpp"
#include "LogATE/Tree/NodeType.hpp"
#include <But/Threading/BasicLockable.hpp>
#include <But/Threading/LockProxyProvider.hpp>
#include <But/Threading/JoiningThread.hpp>
#include <But/Optional.hpp>
#include <But/NotNull.hpp>
#include <But/Guard.hpp>
#include <atomic>
#include <thread>
#include <future>
#include <functional>

namespace LogATE::Utils
{

class WorkerThreads
{
public:
  WorkerThreads(): WorkerThreads{ std::thread::hardware_concurrency() } { }
  explicit WorkerThreads(const size_t threadsCount)
  {
    const auto count = std::max<size_t>(threadsCount, 1u);
    threads_.reserve(count);
    for(size_t i=0; i<count; ++i)
      threads_.emplace_back( &WorkerThreads::processingLoop, this );
  }

  ~WorkerThreads();

  template<typename F>
  auto enqueueBatch(F&& f)
  {
    auto tp = makeTask( std::forward<F>(f) );
    q_->withLock()->enqueueBatch( std::move(tp.first) );
    return std::move( tp.second );
  }

  template<typename F>
  auto enqueueUi(F&& f)
  {
    auto tp = makeTask( std::forward<F>(f) );
    q_->withLock()->enqueueUi( std::move(tp.first) );
    return std::move( tp.second );
  }

  template<typename F>
  auto enqueueFilter(Tree::NodeType const& type, Tree::NodeName const& name, F&& f)
  {
    auto tp = makeTask( std::forward<F>(f) );
    q_->withLock()->enqueueFilter( std::move(tp.first), type, name );
    return std::move( tp.second );
  }

  void waitForAll();

  auto threads() const { return threads_.size(); }
  //auto tasks() const { return pool_.enqueued();  };   // TODO: after BUT upgrade
  auto running() const { return q_->withLock()->size(); }

private:
  using TaskPtr = std::unique_ptr<detail::TaskBase>;

  template<typename F>
  auto makeTask(F&& f)
  {
    using Ret = decltype(f());
    std::promise<Ret> promise;
    auto future = promise.get_future();
    std::function<Ret()> ff{ std::forward<F>(f) };
    auto task = detail::makeTask( std::move(promise), std::move(ff) );
    return std::make_pair( std::move(task), std::move(future) );
  }

  void enqueueTask(TaskPtr task);
  void processingLoop() noexcept;

  But::NotNullUnique<detail::Queue> q_{ But::makeUniqueNN<detail::Queue>() };
  std::atomic<bool> quit_{false};
  std::vector<But::Threading::JoiningThread<std::thread>> threads_;
};


using WorkerThreadsShPtr = But::NotNullShared<WorkerThreads>;

}
