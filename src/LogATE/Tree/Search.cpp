#include "LogATE/Tree/Search.hpp"

namespace LogATE::Tree
{

Search::Search(Utils::WorkerThreadsShPtr workers, const uint64_t chunkSize):
  workers_{ std::move(workers) },
  chunkSize_{chunkSize}
{ }


Search::Result Search::search(LogsPtr logs, Log::Key startPoint, Direction dir, Query q)
{
  (void)logs;
  (void)startPoint;
  (void)dir;
  (void)q;
  (void)chunkSize_;
  (void)workers_;
  throw 42;
}

}
