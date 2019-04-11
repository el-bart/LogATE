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

  std::map<Id, std::string> get(size_t before, Id id, size_t after) const override
  {
    std::map<Id, std::string> out;
    const auto it = data_.find(id);
    if( it == end(data_) )
      return out;

    out[it->first] = it->second;

    {
      auto beforeIt = it;
      for(auto i=0u; i<before; ++i)
      {
        if( beforeIt == begin(data_) )
          break;
        --beforeIt;
        out[beforeIt->first] = beforeIt->second;
      }
    }

    {
      auto afterIt = it;
      for(auto i=0u; i<after; ++i)
      {
        ++afterIt;
        if( afterIt == end(data_) )
          break;
        out[afterIt->first] = afterIt->second;
      }
    }

    return out;
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
