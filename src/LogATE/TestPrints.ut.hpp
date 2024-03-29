#pragma once
#include "LogATE/SequenceNumber.hpp"
#include "LogATE/Log.hpp"
#include "LogATE/Tree/Path.hpp"
#include <iostream>
#include <vector>

namespace LogATE
{

inline std::ostream& operator<<(std::ostream& os, const SequenceNumber sn)
{
  return os << "S/N=" << sn.value_;
}

inline std::ostream& operator<<(std::ostream& os, Log::Key const& key)
{
  return os << "Key=(" << key.str() << "/" << key.sequenceNumber().value_ << ")";
}

inline std::ostream& operator<<(std::ostream& os, Log const& log)
{
  return os << "< " << log.key() << " ; " << log.json() << " >";
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

namespace Tree
{

inline std::ostream& operator<<(std::ostream& os, Tree::Path::Entry const& entry)
{
  os << entry.name();
  if( entry.isArray() )
  {
    os << '[';
    if( entry.hasIndex() )
      os << entry.index();
    os << ']';
  }
  return os;
}

inline std::ostream& operator<<(std::ostream& os, Tree::Path const& path)
{
  return LogATE::printVector(os, path.data());
}

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
