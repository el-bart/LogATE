#pragma once
#include "LogATE/Tree/Node.hpp"
#include "CursATE/Curses/DataSource.hpp"
#include "CursATE/Screen/detail/LogIdIndexCache.hpp"
#include <functional>

namespace CursATE::Screen::detail
{

class LogDataSource final: public Curses::DataSource
{
public:
  LogDataSource(LogATE::Tree::NodeShPtr node, std::function<std::string(LogATE::Log const&)> log2str):
    node_{ std::move(node).underlyingPointer() },
    log2str_{ std::move(log2str) },
    indexCache_{node_}
  { }

  size_t index(Id const& id) const override;
  size_t size() const override;
  But::Optional<Id> nearestTo(Id const& id) const override;
  But::Optional<Id> first() const override;
  But::Optional<Id> last() const override;
  std::map<Id, std::string> get(size_t before, Id const& id, size_t after) const override;

private:
  LogATE::Tree::NodeWeakPtr node_;
  std::function<std::string(LogATE::Log const&)> log2str_;
  detail::LogIdIndexCache indexCache_;
};

}
