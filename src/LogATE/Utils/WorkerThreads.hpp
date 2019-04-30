#pragma once
#include <But/Threading/ThreadPool.hpp>
#include <But/Threading/Policy/Std.hpp>
#include <But/NotNull.hpp>

namespace LogATE::Utils
{

class WorkerThreads
{
public:
  WorkerThreads(): WorkerThreads{ std::max(1u, std::thread::hardware_concurrency()) } { }

  template<typename F>
  auto enqueue(F&& f) { return pool_.run( std::forward<F>(f) ); }

  auto threads() const { return pool_.size(); }
  //auto tasks() const { return pool_.enqueued();  };   // TODO: after BUT upgrade

private:
  explicit WorkerThreads(const size_t threads):
    pool_{ But::Threading::ThreadsCount{threads} }
  { }

  But::Threading::ThreadPool<But::Threading::Policy::Std> pool_;
};


using WorkerThreadsShPtr = But::NotNullShared<WorkerThreads>;

}
