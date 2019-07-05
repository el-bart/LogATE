#pragma once
#include "CursATE/Curses/DataSource.hpp"
#include <vector>
#include <string>

namespace CursATE::Screen::detail
{

class ConstStringDataSource final: public Curses::DataSource
{
public:
  explicit ConstStringDataSource(std::vector<std::string> entries):
    entries_{ std::move(entries) }
  { }

  size_t index(Id const& id) const override;
  size_t size() const override;
  But::Optional<Id> nearestTo(Id const& id) const override;
  But::Optional<Id> first() const override;
  But::Optional<Id> last() const override;
  std::map<Id, std::string> get(size_t before, Id const& id, size_t after) const override;

private:
  const std::vector<std::string> entries_;
};

}
