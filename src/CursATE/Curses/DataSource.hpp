#pragma once
#include <But/NotNull.hpp>
#include <But/Optional.hpp>
#include <But/Mpl/FreeOperators.hpp>
#include <string>
#include <utility>

namespace CursATE::Curses
{

struct DataSource
{
  struct Id { size_t value_{0}; };

  using value_type = std::pair<Id, std::string>;
  using optional_value = But::Optional<value_type>;

  virtual ~DataSource() = default;

  virtual size_t size() const = 0;
  virtual Id first() const = 0;
  virtual Id last() const = 0;
  virtual optional_value get(Id id) const = 0;
  virtual optional_value next(Id id) const = 0;
  virtual optional_value previous(Id id) const = 0;
};
using DataSourceShNN = But::NotNullShared<DataSource>;


BUT_MPL_FREE_OPERATORS_COMPARE(DataSource::Id, .value_)

}
