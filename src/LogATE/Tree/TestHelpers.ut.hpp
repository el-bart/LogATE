#pragma once
#include "LogATE/Tree/Logs.hpp"

namespace LogATE
{
namespace Tree
{

inline auto makeLog(unsigned num)
{
  auto log = LogATE::json2log(R"({ "foo": "bar" })");
  log.sn_.value_ = num;
  return log;
}

inline auto allSns(Logs const& logs)
{
  std::vector<SequenceNumber> out;
  auto locked = logs.withLock();
  out.reserve( locked->size() );
  for(auto it=locked->begin(); it!=locked->end(); ++it)
    out.push_back( it->sn_ );
  return out;
}

inline auto makeSns(std::initializer_list<unsigned> lst)
{
  std::vector<SequenceNumber> out;
  out.reserve( lst.size() );
  for(auto sn: lst)
    out.push_back( SequenceNumber{sn} );
  return out;
}

inline auto logs2sns(std::vector<LogATE::Log> const& in)
{
  std::vector<SequenceNumber> out;
  out.reserve(in.size());
  for(auto log: in)
    out.push_back(log.sn_);
  return out;
}

}
}
