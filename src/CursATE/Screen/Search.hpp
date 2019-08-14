#pragma once
#include "LogATE/Tree/Node.hpp"
#include <string>

namespace CursATE::Screen
{

class Search final
{
public:
  enum class Direction
  {
    Forward,
    Backward
  };

  struct Result
  {
    enum class Status
    {
      Canceled,
      Found,
      NotFound
    };

    static Result canceled() { return { Status::Canceled, {} }; }
    static Result notFound() { return { Status::NotFound, {} }; }
    static Result found(LogATE::Log::Key key) { return { Status::Found, std::move(key) }; }

    Status status_{Status::Canceled};
    But::Optional<LogATE::Log::Key> key_;
  };

  explicit Search(LogATE::Utils::WorkerThreadsShPtr workers):
    workers_{ std::move(workers) }
  { }

  Search(Search const&) = delete;
  Search& operator=(Search const&) = delete;
  Search(Search&&) = delete;
  Search& operator=(Search&&) = delete;

  Result process(LogATE::Tree::NodeShPtr node, LogATE::Log::Key const& currentSelection, Direction dir);
  Result processAgain(LogATE::Tree::NodeShPtr node, LogATE::Log::Key const& currentSelection, Direction dir);

private:
  bool updateSearchPattern();
  Result triggerSearch(LogATE::Tree::NodeShPtr node, LogATE::Log::Key const& currentSelection, Direction dir);

  LogATE::Utils::WorkerThreadsShPtr workers_;
  std::string keyQuery_;
  std::string valueQuery_;
};

}
