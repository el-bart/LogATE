#pragma once
#include "LogATE/Tree/NodeName.hpp"
#include "LogATE/Tree/NodeType.hpp"
#include <But/Threading/ThreadPool.hpp>
#include <But/Threading/Policy/Std.hpp>
#include <But/NotNull.hpp>
#include <But/Guard.hpp>
#include <atomic>

namespace LogATE::Utils
{

class WorkerThreads
{
public:
  WorkerThreads(): WorkerThreads{ std::thread::hardware_concurrency() } { }
  explicit WorkerThreads(const size_t threads): pool_{ But::Threading::ThreadsCount{ std::max<size_t>(1, threads) } } { }

  template<typename F>
  auto enqueueBatch(F&& f)
  {
    ++nonProcessed_;
    return pool_.run( [this, ff=std::forward<F>(f)]()mutable {
                                                      const auto guard = But::makeGuard( [&]{ --this->nonProcessed_; } );
                                                      return ff();
                                                    } );
  }

  template<typename F>
  auto enqueueUi(F&& f)
  {
    ++nonProcessed_;
    return pool_.run( [this, ff=std::forward<F>(f)]()mutable {
                                                      const auto guard = But::makeGuard( [&]{ --this->nonProcessed_; } );
                                                      return ff();
                                                    } );
  }

  template<typename F>
  auto enqueueFilter(Tree::NodeType const& type, Tree::NodeName const& name, F&& f)
  {
    (void)type;
    (void)name;
    ++nonProcessed_;
    return pool_.run( [this, ff=std::forward<F>(f)]()mutable {
                                                      const auto guard = But::makeGuard( [&]{ --this->nonProcessed_; } );
                                                      return ff();
                                                    } );
  }

  void waitForAll();

  auto threads() const { return pool_.size(); }
  //auto tasks() const { return pool_.enqueued();  };   // TODO: after BUT upgrade
  auto running() const { return nonProcessed_.load(); }

private:
  std::atomic<uint64_t> nonProcessed_{0};   // TODO: workaround until WorkerPool::enqueued() can be queried for size
  But::Threading::ThreadPool<But::Threading::Policy::Std> pool_;
};


using WorkerThreadsShPtr = But::NotNullShared<WorkerThreads>;

}
