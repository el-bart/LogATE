#pragma once
#include <vector>
#include <string>
#include <But/Exception.hpp>
#include <But/assert.hpp>

namespace LogATE::Tree
{

struct Path final
{
  struct Entry
  {
    BUT_DEFINE_EXCEPTION(EmptyNode, But::Exception, "empty node");
    BUT_DEFINE_EXCEPTION(InvalidNode, But::Exception, "invalid node");
    BUT_DEFINE_EXCEPTION(InvalidArray, But::Exception, "invalid array");

    explicit Entry(std::string str);
    Entry(Entry&&) = default;
    Entry& operator=(Entry&&) = default;
    Entry(Entry const&) = default;
    Entry& operator=(Entry const&) = default;

    bool operator==(Entry const& rhs) const
    {
      return index_    == rhs.index_    &&
             hasIndex_ == rhs.hasIndex_ &&
             isArray_  == rhs.isArray_  &&
             name_     == rhs.name_;
    }
    bool operator!=(Entry const& rhs) const { return not ( *this == rhs ); }

    std::string str() const;
    auto& name() const { return name_; }
    auto isArray() const { return isArray_; }
    auto hasIndex() const { BUT_ASSERT( isArray() ); return hasIndex_; }
    auto index() const { BUT_ASSERT( hasIndex() ); return index_; }

  private:
    std::string name_;
    bool isArray_{false};
    bool hasIndex_{false};
    uint64_t index_{0};
    std::string str_;
  };

  using Data = std::vector<Entry>;

  static Path parse(std::string const& str);
  static Path build(std::vector<std::string> nodes);

  Path() = default;
  explicit Path(std::vector<std::string> nodes):
    Path{ build( std::move(nodes) ) }
  { }

  Path(Path const&) = default;
  Path& operator=(Path const&) = default;
  Path(Path&&) = default;
  Path& operator=(Path&&) = default;

  std::string str() const;

  auto empty() const { return value_.empty(); }
  auto absolute() const { return isAbsolute_; }
  auto begin() const { return value_.begin(); }
  auto end() const { return value_.end(); }
  auto const& data() const { return value_; }
  auto& last() const { BUT_ASSERT( not empty() ); return value_.back(); }

private:
  Path(Data value, bool isAbsolute):
    value_{ std::move(value) },
    isAbsolute_{ isAbsolute }
  { }

  Data value_;
  bool isAbsolute_{false};
};

inline bool operator==(Path const& lhs, Path const& rhs) { return lhs.absolute() == rhs.absolute() && lhs.data() == rhs.data(); }
inline bool operator!=(Path const& lhs, Path const& rhs) { return not ( lhs == rhs ); }

}
