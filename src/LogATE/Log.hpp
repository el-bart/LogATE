#pragma once
#include "SequenceNumber.hpp"
#include <nlohmann/json.hpp>
#include <But/NotNull.hpp>
#include <string>

namespace LogATE
{

struct Log final
{
  static Log acceptRawString(std::string in);
  static Log acceptRawString(SequenceNumber sn, std::string in);

  explicit Log(char const* in): Log{ std::string{in} } { }
  explicit Log(std::string const& in);
  explicit Log(nlohmann::json const& in);
  Log(SequenceNumber sn, char const* in): Log{ sn, std::string{in} } { }
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
  struct DirectInitTag{};
  Log(DirectInitTag&&, SequenceNumber sn, std::string in);

  SequenceNumber sn_;
  But::NotNullShared<const std::string> str_;
  // TODO: consider adding more structure to a log -> timestamp of receiving, source IP:port, etc...
};


struct AnnotatedLog final
{
  explicit AnnotatedLog(Log log);
  explicit AnnotatedLog(nlohmann::json in);
  AnnotatedLog(SequenceNumber sn, nlohmann::json in);

  AnnotatedLog(AnnotatedLog const&) = default;
  AnnotatedLog& operator=(AnnotatedLog const&) = default;

  AnnotatedLog(AnnotatedLog&&) = default;
  AnnotatedLog& operator=(AnnotatedLog&&) = default;

  auto const& log() const { return log_; }
  auto const& json() const { return json_; }

private:
  Log log_;
  nlohmann::json json_;
};

}
