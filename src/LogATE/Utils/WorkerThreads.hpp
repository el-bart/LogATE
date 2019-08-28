#pragma once
#include "LogATE/Utils/detail/Task.hpp"
#include "LogATE/Tree/NodeName.hpp"
#include "LogATE/Tree/NodeType.hpp"
#include <But/Threading/Fifo.hpp>
#include <But/Threading/JoiningThread.hpp>
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
    return enqueue( std::forward<F>(f) );
  }

  template<typename F>
  auto enqueueUi(F&& f)
  {
    return enqueue( std::forward<F>(f) );
  }

  template<typename F>
  auto enqueueFilter(Tree::NodeType const& type, Tree::NodeName const& name, F&& f)
  {
    (void)type;
    (void)name;
    return enqueue( std::forward<F>(f) );
  }

  void waitForAll();

  auto threads() const { return threads_.size(); }
  //auto tasks() const { return pool_.enqueued();  };   // TODO: after BUT upgrade
  auto running() const { return q_.withLock()->size(); }

private:
  using TaskPtr = std::unique_ptr<detail::TaskBase>;
  using Queue = But::Threading::Fifo<TaskPtr>;

  template<typename F>
  auto enqueue(F&& f)
  {
    using Ret = decltype(f());
    std::promise<Ret> promise;
    auto future = promise.get_future();
    std::function<Ret()> ff{ std::forward<F>(f) };
    auto task = detail::makeTask( std::move(promise), std::move(ff) );
    enqueueTask( std::move(task) );
    return future;
  }

  void enqueueTask(TaskPtr task);
  void processingLoop() noexcept;

  std::atomic<uint64_t> nonProcessed_{0};   // TODO: workaround until WorkerPool::enqueued() can be queried for size
  mutable Queue q_; // TODO: tmp...
  std::atomic<bool> quit_{false};
  std::vector<But::Threading::JoiningThread<std::thread>> threads_;
};


using WorkerThreadsShPtr = But::NotNullShared<WorkerThreads>;

}
