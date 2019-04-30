#pragma once
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
  WorkerThreads(): WorkerThreads{ std::max(1u, std::thread::hardware_concurrency()) } { }

  template<typename F>
  auto enqueue(F&& f)
  {
    ++nonProcessed_;
    return pool_.run( [this, ff=std::forward<F>(f)] {
                                                      const auto guard = But::makeGuard( [&]{ --this->nonProcessed_; } );
                                                      return ff();
                                                    } );
  }

  void waitForAll();

  auto threads() const { return pool_.size(); }
  //auto tasks() const { return pool_.enqueued();  };   // TODO: after BUT upgrade

private:
  explicit WorkerThreads(const size_t threads):
    pool_{ But::Threading::ThreadsCount{threads} }
  { }

  std::atomic<size_t> nonProcessed_{0}; // TODO: workaround until WorkerPool::enqueued() can be queried for size
  But::Threading::ThreadPool<But::Threading::Policy::Std> pool_;
};


using WorkerThreadsShPtr = But::NotNullShared<WorkerThreads>;

}
