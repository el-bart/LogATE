#pragma once
#include "LogATE/Tree/Logs.hpp"

namespace LogATE
{
namespace Tree
{

inline auto makeLog(const unsigned num)
{
  return Log{ LogATE::SequenceNumber{num}, R"({ "foo": "bar" })" };
}

inline auto makeAnnotatedLog(const unsigned num)
{
  return LogATE::AnnotatedLog{ makeLog(num) };
}

inline auto allSns(Logs const& logs)
{
  std::vector<SequenceNumber> out;
  auto locked = logs.withLock();
  out.reserve( locked->size() );
  for(auto it=locked->begin(); it!=locked->end(); ++it)
    out.push_back( it->sequenceNumber() );
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
    out.push_back( log.sequenceNumber() );
  return out;
}

}
}
