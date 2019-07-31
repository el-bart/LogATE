#pragma once
#include "LogATE/SequenceNumber.hpp"
#include "LogATE/Tree/Path.hpp"
#include "SequenceNumber.hpp"
#include <nlohmann/json.hpp>
#include <But/NotNull.hpp>
#include <string>

namespace LogATE
{

struct AnnotatedLog;

struct Log final
{
  struct Key final
  {
    template<size_t N>
    explicit Key(char const (&value)[N]): Key{ std::string_view{ value, N-1u } } { }
    explicit Key(std::string_view const& value): Key{ std::string{ value.begin(), value.end() } } { }
    explicit Key(std::string value): value_{ But::makeSharedNN<const std::string>( std::move(value) ) } { }

    Key(Key const&) = default;
    Key& operator=(Key const&) = default;

    Key(Key&&) = default;
    Key& operator=(Key&&) = default;

    auto str() const { return *value_; }

    inline auto operator< (Log::Key const& rhs) const { return str() <  rhs.str(); }
    inline auto operator<=(Log::Key const& rhs) const { return str() <= rhs.str(); }
    inline auto operator> (Log::Key const& rhs) const { return str() >  rhs.str(); }
    inline auto operator>=(Log::Key const& rhs) const { return str() >= rhs.str(); }
    inline auto operator==(Log::Key const& rhs) const { return str() == rhs.str(); }
    inline auto operator!=(Log::Key const& rhs) const { return str() != rhs.str(); }

  private:
    But::NotNullShared<const std::string> value_;
  };

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
  auto const& key() const { return key_; }
  auto sequenceNumber() const { return sn_; }
  auto json() const { return nlohmann::json::parse(*str_); }

private:
  struct DirectInitTag{};
  Log(DirectInitTag&&, SequenceNumber sn, std::string in);
  Log(DirectInitTag&&, SequenceNumber sn, std::string in, Key key);
  friend struct AnnotatedLog;

  SequenceNumber sn_;
  But::NotNullShared<const std::string> str_;
  Key key_;
  // TODO: consider adding more structure to a log -> timestamp of receiving, source IP:port, etc...
};


inline auto operator< (Log const& lhs, Log const& rhs) { return lhs.key() <  rhs.key(); }
inline auto operator<=(Log const& lhs, Log const& rhs) { return lhs.key() <= rhs.key(); }
inline auto operator> (Log const& lhs, Log const& rhs) { return lhs.key() >  rhs.key(); }
inline auto operator>=(Log const& lhs, Log const& rhs) { return lhs.key() >= rhs.key(); }
inline auto operator==(Log const& lhs, Log const& rhs) { return lhs.key() == rhs.key(); }
inline auto operator!=(Log const& lhs, Log const& rhs) { return lhs.key() != rhs.key(); }


struct AnnotatedLog final
{
  template<size_t N>
  explicit AnnotatedLog(char const (&str)[N]): AnnotatedLog{ std::string{str} } { }
  explicit AnnotatedLog(char const* str): AnnotatedLog{ std::string{str} } { }
  explicit AnnotatedLog(std::string_view str): AnnotatedLog{ std::string{str} } { }
  /** @brief direct initialization with a string.
   *  @note this c-tor does NOT perform JSON compacting - it is assummed it is already compacted.
   */
  explicit AnnotatedLog(std::string str);
  explicit AnnotatedLog(std::string str, Tree::Path const& keyPath);
  explicit AnnotatedLog(Log log);
  explicit AnnotatedLog(nlohmann::json in);
  AnnotatedLog(SequenceNumber sn, nlohmann::json in);

  AnnotatedLog(AnnotatedLog const&) = default;
  AnnotatedLog& operator=(AnnotatedLog const&) = default;

  AnnotatedLog(AnnotatedLog&&) = default;
  AnnotatedLog& operator=(AnnotatedLog&&) = default;

  auto const& log() const { return log_; }
  auto const& json() const { return json_; }
  auto const& key() const { return log_.key(); }

private:
  Log log_;
  nlohmann::json json_;
};


inline auto operator< (AnnotatedLog const& lhs, AnnotatedLog const& rhs) { return lhs.key() <  rhs.key(); }
inline auto operator<=(AnnotatedLog const& lhs, AnnotatedLog const& rhs) { return lhs.key() <= rhs.key(); }
inline auto operator> (AnnotatedLog const& lhs, AnnotatedLog const& rhs) { return lhs.key() >  rhs.key(); }
inline auto operator>=(AnnotatedLog const& lhs, AnnotatedLog const& rhs) { return lhs.key() >= rhs.key(); }
inline auto operator==(AnnotatedLog const& lhs, AnnotatedLog const& rhs) { return lhs.key() == rhs.key(); }
inline auto operator!=(AnnotatedLog const& lhs, AnnotatedLog const& rhs) { return lhs.key() != rhs.key(); }


struct OrderByKey final
{
  using is_transparent = void;  // voidever...

  auto operator()(AnnotatedLog const& lhs, AnnotatedLog const& rhs) const { return lhs.key() < rhs.key(); };
  auto operator()(Log          const& lhs, Log          const& rhs) const { return lhs.key() < rhs.key(); };
  auto operator()(Log          const& lhs, Log::Key     const& rhs) const { return lhs.key() < rhs;       };
  auto operator()(Log::Key     const& lhs, Log          const& rhs) const { return lhs       < rhs.key(); };
  auto operator()(AnnotatedLog const& lhs, Log::Key     const& rhs) const { return lhs.key() < rhs;       };
  auto operator()(Log::Key     const& lhs, AnnotatedLog const& rhs) const { return lhs       < rhs.key(); };
};

}
