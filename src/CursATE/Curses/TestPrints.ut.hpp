#pragma once
#include "LogATE/TestPrints.ut.hpp"
#include "CursATE/Curses/DataSource.hpp"
#include <iostream>
#include <vector>

namespace CursATE::Curses
{

inline std::ostream& operator<<(std::ostream& os, const DataSource::Id id)
{
  return os << "ID=" << id.value_;
}

}

namespace std
{

inline std::ostream& operator<<(std::ostream& os, vector<pair<CursATE::Curses::DataSource::Id, string>> const& lines)
{
  vector<string> tmp;
  for(auto& e: lines)
    tmp.push_back( std::to_string(e.first.value_) + ":" + e.second );
  return LogATE::printVector(os, tmp);
}

}
