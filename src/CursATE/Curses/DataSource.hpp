#pragma once
#include <But/NotNull.hpp>
#include <But/Optional.hpp>
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
  virtual size_t index(Id id) const = 0;
  virtual But::Optional<Id> nearestTo(Id id) const = 0;
  virtual But::Optional<Id> first() const = 0;
  virtual But::Optional<Id> last() const = 0;
  virtual std::map<Id, std::string> get(size_t before, Id id, size_t after) const = 0;

protected:
  auto absDiff(size_t lhs, size_t rhs) const
  {
    if( lhs > rhs )
      return lhs - rhs;
    return rhs - lhs;
  }

  Id closest(const Id reference, const Id id1, const Id id2) const
  {
    const auto diff1 = absDiff(reference.value_, id1.value_);
    const auto diff2 = absDiff(reference.value_, id2.value_);
    return ( diff1 >= diff2 ) ? id2 : id1;
  }
};

using DataSourceShNN = But::NotNullShared<DataSource>;


BUT_MPL_FREE_OPERATORS_COMPARE(DataSource::Id, .value_)

}
