#pragma once
#include <But/NotNull.hpp>
#include <But/Mpl/FreeOperators.hpp>
#include <string>
#include <map>

namespace CursATE::Curses
{

struct DataSource
{
  struct Id { size_t value_{0}; };

  virtual ~DataSource() = default;

  virtual size_t size() const = 0;
  virtual Id first() const = 0;
  virtual Id last() const = 0;
  virtual std::map<Id, std::string> get(size_t before, Id id, size_t after) const = 0;
};
using DataSourceShNN = But::NotNullShared<DataSource>;


BUT_MPL_FREE_OPERATORS_COMPARE(DataSource::Id, .value_)

}
