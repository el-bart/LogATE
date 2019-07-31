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
    Key(char const (&value)[N], const SequenceNumber sn): Key{ std::string_view{ value, N-1u }, sn } { }
    Key(std::string_view const& value, const SequenceNumber sn): Key{ std::string{ value.begin(), value.end() }, sn } { }
    Key(std::string value, const SequenceNumber sn): value_{ But::makeSharedNN<const std::string>( std::move(value) ) }, sn_{sn} { }

    Key(Key const&) = default;
    Key& operator=(Key const&) = default;

    Key(Key&&) = default;
    Key& operator=(Key&&) = default;

    auto str() const { return *value_; }
    auto sequenceNumber() const { return sn_; }

    inline auto operator< (Log::Key const& rhs) const { return starshipOperator(*this, rhs) == -1; }
    inline auto operator> (Log::Key const& rhs) const { return starshipOperator(*this, rhs) == +1; }
    inline auto operator<=(Log::Key const& rhs) const { return starshipOperator(*this, rhs) <=  0; }
    inline auto operator>=(Log::Key const& rhs) const { return starshipOperator(*this, rhs) >=  0; }
    inline auto operator==(Log::Key const& rhs) const { return starshipOperator(*this, rhs) ==  0; }
    inline auto operator!=(Log::Key const& rhs) const { return starshipOperator(*this, rhs) !=  0; }

  private:
    inline int starshipOperator(Log::Key const& lhs, Log::Key const& rhs) const
    {
      if( lhs.sequenceNumber() == rhs.sequenceNumber() )
      {
        BUT_ASSERT( lhs.str() == rhs.str() );
        return 0;
      }
      return starshipOperator( lhs.str(), rhs.str() );
    }

    inline int starshipOperator(std::string const& lhs, std::string const& rhs) const
    {
      const auto common = std::min( lhs.size(), rhs.size() );
      for(auto i=0u; i<common; ++i)
      {
        if( lhs[i] < rhs[i] )
          return -1;
        if( lhs[i] > rhs[i] )
          return +1;
      }
      if( lhs.size() == rhs.size() )
        return 0;
      return ( lhs.size() <  rhs.size() ) ? -1 : +1;
    }

    But::NotNullShared<const std::string> value_;
    SequenceNumber sn_;
  };

  Log(Key key, char const* in): Log{ std::move(key), std::string{in} } { }
  Log(Key key, std::string const& in);
  Log(Key key, nlohmann::json const& in);

  Log(Log const&) = default;
  Log& operator=(Log const&) = default;

  Log(Log&&) = default;
  Log& operator=(Log&&) = default;

  auto const& str() const { return *str_; }
  auto const& key() const { return key_; }
  auto sequenceNumber() const { return key_.sequenceNumber(); }
  auto json() const { return nlohmann::json::parse(*str_); }

private:
  struct DirectInitTag{};
  Log(DirectInitTag&&, Key&& key, std::string&& in);
  friend struct AnnotatedLog;

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
  explicit AnnotatedLog(Log log);
  /** @brief direct initialization with a string.
   *  @note this c-tor does NOT perform JSON compacting - it is assummed it is already compacted.
   *  @note syntax of JSON is still being checked.
   */
  AnnotatedLog(std::string str, Tree::Path const& keyPath);

  AnnotatedLog(AnnotatedLog const&) = default;
  AnnotatedLog& operator=(AnnotatedLog const&) = default;

  AnnotatedLog(AnnotatedLog&&) = default;
  AnnotatedLog& operator=(AnnotatedLog&&) = default;

  auto const& log() const { return log_; }
  auto const& json() const { return json_; }
  auto const& key() const { return log_.key(); }

private:
  nlohmann::json json_;
  Log log_;
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
