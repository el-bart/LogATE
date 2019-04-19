#pragma once
#include "LogATE/Tree/Node.hpp"
#include "CursATE/Curses/DataSource.hpp"
#include <vector>

namespace CursATE::Screen::detail
{

class FilterTreeDataSource final: public Curses::DataSource
{
public:
  explicit FilterTreeDataSource(LogATE::Tree::NodeShPtr root);

  size_t size() const override;
  But::Optional<Id> first() const override;
  But::Optional<Id> last() const override;
  std::map<Id, std::string> get(size_t before, Id id, size_t after) const override;

  Id node2id(LogATE::Tree::NodeShPtr const& selected) const;

private:
  struct Entry
  {
    LogATE::Tree::NodeShPtr node_;
    std::string text_;
  };

  std::vector<Entry> constructEntries(LogATE::Tree::NodeShPtr const& root) const;

  const std::vector<Entry> entries_;
};

}
