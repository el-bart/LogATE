#pragma once
#include "LogATE/SequenceNumber.hpp"
#include "LogATE/Log.hpp"
#include <iostream>
#include <vector>

namespace LogATE
{

inline std::ostream& operator<<(std::ostream& os, const SequenceNumber sn)
{
  return os << "S/N=" << sn.value_;
}

inline std::ostream& operator<<(std::ostream& os, Log const& log)
{
  return os << "< " << log.sn_ << " ; " << *log.log_ << " >";
}


template<typename T>
inline std::ostream& printVector(std::ostream& os, std::vector<T> const& c)
{
  auto first = true;
  os << "[ ";
  for(auto const& e: c)
  {
    if(first)
      first = false;
    else
      os << ", ";
    os << e;
  }
  os << " ]";
  return os;
}

}

namespace std
{

inline std::ostream& operator<<(std::ostream& os, vector<LogATE::Log> const& logs)
{
  return LogATE::printVector(os, logs);
}

inline std::ostream& operator<<(std::ostream& os, vector<LogATE::SequenceNumber> const& sns)
{
  return LogATE::printVector(os, sns);
}

}
