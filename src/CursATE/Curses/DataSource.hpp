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
  struct Id final { std::string value_; };

  virtual ~DataSource() = default;

  virtual size_t size() const = 0;
  virtual size_t index(Id const& id) const = 0;
  virtual But::Optional<Id> nearestTo(Id const& id) const = 0;
  virtual But::Optional<Id> first() const = 0;
  virtual But::Optional<Id> last() const = 0;
  virtual std::map<Id, std::string> get(size_t before, Id const& id, size_t after) const = 0;

protected:
  static auto firstDiffPoint(std::string const& lhs, std::string const& rhs)
  {
    BUT_ASSERT( lhs.size() <= rhs.size() );
    for(auto i=0ul; i<lhs.size(); ++i)
      if( lhs[i] != rhs[i] )
        return i;
    return lhs.size();
  }

  static auto absCharDiff(const char a, const char b)
  {
    const auto ua = unsigned(a);
    const auto ub = unsigned(b);
    if(ua >= ub)
      return ua - ub;
    return ub - ua;
  }

  static double absDiff(std::string const& lhs, std::string const& rhs)
  {
    if( lhs.size() > rhs.size() )
      return absDiff(rhs, lhs);
    BUT_ASSERT( lhs.size() <= rhs.size() );

    const auto maxSize = std::max( lhs.size(), rhs.size() );
    const auto diff = firstDiffPoint(lhs, rhs);
    BUT_ASSERT( maxSize >= diff );
    const auto score = static_cast<double>(maxSize-diff);

    if( diff == lhs.size() )
      return score + ( 1.0 - static_cast<double>(lhs.size()) / rhs.size() );
    BUT_ASSERT( diff < lhs.size() );
    const auto acd = absCharDiff(lhs[diff], rhs[diff]);
    return score + acd/256.0;
  }

  static Id const& closest(Id const& reference, Id const& id1, Id const& id2)
  {
    const auto diff1 = absDiff(reference.value_, id1.value_);
    const auto diff2 = absDiff(reference.value_, id2.value_);
    return ( diff1 >= diff2 ) ? id2 : id1;
  }
};

using DataSourceShNN = But::NotNullShared<DataSource>;


BUT_MPL_FREE_OPERATORS_COMPARE(DataSource::Id, .value_)

}
