#pragma once
#include "SequenceNumber.hpp"
#include <nlohmann/json.hpp>
#include <But/NotNull.hpp>
#include <string>

namespace LogATE
{

struct Log final
{
  explicit Log(char const* str): Log{ std::string{str} } { }
  explicit Log(std::string const& in);
  explicit Log(nlohmann::json const& in);
  Log(SequenceNumber sn, std::string const& in);
  Log(SequenceNumber sn, nlohmann::json const& in);

  Log(Log const&) = default;
  Log& operator=(Log const&) = default;

  Log(Log&&) = default;
  Log& operator=(Log&&) = default;

  auto const& str() const { return *str_; }
  auto sequenceNumber() const { return sn_; }
  auto json() const { return nlohmann::json::parse(*str_); }

private:
  SequenceNumber sn_;
  But::NotNullShared<const std::string> str_;
  // TODO: consider adding more structure to a log -> timestamp of receiving, source IP:port, etc...
};


struct AnnotatedLog final
{
  explicit AnnotatedLog(Log const& log):
    log_{log},
    json_{ log_.json() }
  { }

  const Log log_;
  const nlohmann::json json_;
};

}
