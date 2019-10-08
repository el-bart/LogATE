#pragma once
#include "LogATE/Utils/WorkerThreads.hpp"
#include "LogATE/Tree/Logs.hpp"
#include "But/Optional.hpp"
#include "But/NotNull.hpp"
#include <future>
#include <functional>

namespace LogATE::Tree
{

struct Search
{
  struct Result
  {
    std::future<But::Optional<Log::Key>> value_;
    But::NotNullShared<std::atomic<bool>> cancel_;
  };

  enum class Direction
  {
    Forward,
    Backward
  };

  using LogsPtr = But::NotNullShared<const Logs>;
  using Query = std::function<bool(Log const&)>;

  explicit Search(Utils::WorkerThreadsShPtr workers, uint64_t chunkSize = 10'000);

  Result search(LogsPtr logs, Log::Key startPoint, Direction dir, Query q);

private:
  Utils::WorkerThreadsShPtr workers_;
  const uint64_t chunkSize_;
};

}
