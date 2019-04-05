#pragma once
#include "CursATE/Curses/DataSource.hpp"
#include <map>

namespace CursATE::Curses::detail
{

struct StringDataSource: public DataSource
{
  size_t size() const override { return data_.size(); }

  Id first() const override
  {
    if( data_.empty() )
      return {};
    return data_.begin()->first;
  }

  virtual Id last() const override
  {
    if( data_.empty() )
      return {};
    return data_.rbegin()->first;
  }

  optional_value get(const Id id) const override
  {
    const auto it = data_.find(id);
    if( it == end(data_) )
      return {};
    return value_type{*it};
  }

  optional_value next(const Id id) const override
  {
    auto it = data_.find(id);
    if( it == end(data_) )
      return {};
    ++it;
    if( it == end(data_) )
      return {};
    return value_type{*it};
  }

  optional_value previous(const Id id) const override
  {
    auto it = data_.find(id);
    if( it == end(data_) )
      return {};
    if( it == begin(data_) )
      return {};
    --it;
    if( it == end(data_) )
      return {};
    return value_type{*it};
  }

  void addNewest(std::string str)
  {
    Id id{nextFreeId_};
    nextFreeId_ += nextStep_;
    ++nextStep_;
    data_[id] = std::move(str);
  }

  void removeOldest()
  {
    if( data_.empty() )
      return;
    data_.erase(data_.begin());
  }

  size_t nextFreeId_{42};
  size_t nextStep_{1};
  std::map<Id, std::string> data_;
};

}
