#pragma once
#include "LogATE/Tree/Node.hpp"
#include "CursATE/Curses/DataSource.hpp"
#include <functional>

namespace CursATE::detail
{

class LogDataSource final: public Curses::DataSource
{
public:
  LogDataSource(LogATE::Tree::NodeShPtr node, std::function<std::string(LogATE::Log const&)> log2str):
    node_{ std::move(node) },
    log2str_{ std::move(log2str) }
  { }

  size_t size() const override { return node_->logs().withLock()->size(); }

  But::Optional<Id> first() const override;
  But::Optional<Id> last() const override;
  std::map<Id, std::string> get(size_t before, Id id, size_t after) const override;

private:
  LogATE::Tree::NodeShPtr node_;
  std::function<std::string(LogATE::Log const&)> log2str_;
};

}
