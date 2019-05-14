#pragma once
#include "LogATE/Log.hpp"
#include "LogATE/Utils/PrintableStringConverter.hpp"
#include "LogATE/Tree/Path.hpp"
#include "CursATE/Curses/DataSource.hpp"
#include <vector>
#include <string>
#include <But/Optional.hpp>

namespace CursATE::Screen::detail
{

class LogEntryDataSource final: public Curses::DataSource
{
public:
  explicit LogEntryDataSource(LogATE::Log const& log): entries_{ constructEntries(log) } { }

  size_t index(Id id) const override;
  size_t size() const override;
  But::Optional<Id> nearestTo(Id id) const override;
  But::Optional<Id> first() const override;
  But::Optional<Id> last() const override;
  std::map<Id, std::string> get(size_t before, Id id, size_t after) const override;

  LogATE::Tree::Path id2path(Id id) const;
  But::Optional<std::string> id2value(Id id) const;

private:
  struct Entry
  {
    LogATE::Tree::Path path_;
    std::string text_;
    But::Optional<std::string> value_;
  };

  std::vector<Entry> constructEntries(LogATE::Log const& log) const;

  const LogATE::Utils::PrintableStringConverter printable_{};
  const std::vector<Entry> entries_;
};

}
