#pragma once
#include "LogATE/Tree/Logs.hpp"

namespace LogATE
{

inline auto makeKey(const unsigned num, std::string const& str="some-key")
{
  return Log::Key{ str, LogATE::SequenceNumber{num} };
}

inline auto makeKey()
{
  return makeKey( LogATE::SequenceNumber::next().value_ );
}

inline auto makeLog(const unsigned num, std::string const& json)
{
  return Log{ makeKey(num), json };
}

inline auto makeLog(const unsigned num)
{
  return makeLog( num, R"({ "foo": "bar" })" );
}

inline auto makeLog(std::string const& json)
{
  return makeLog( SequenceNumber::next().value_, json );
}

inline auto makeAnnotatedLog(const unsigned num)
{
  return LogATE::AnnotatedLog{ makeLog(num) };
}

inline auto allSns(Tree::Logs const& logs)
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
