#pragma once
#include <vector>
#include <string>
#include <But/Mpl/FreeOperators.hpp>
#include <But/Exception.hpp>
#include <But/assert.hpp>

namespace LogATE::Tree
{

struct Path final
{
  BUT_DEFINE_EXCEPTION(EmptyNodeInPath, But::Exception, "empty node in path");  // TODO: remove

  struct Entry
  {
    BUT_DEFINE_EXCEPTION(EmptyNode, But::Exception, "empty node");
    BUT_DEFINE_EXCEPTION(InvalidNode, But::Exception, "invalid node");
    BUT_DEFINE_EXCEPTION(InvalidArray, But::Exception, "invalid array");

    explicit Entry(std::string&& str);
    Entry(Entry&&) = default;
    Entry& operator=(Entry&&) = default;
    Entry(Entry const&) = default;
    Entry& operator=(Entry const&) = default;

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
  };

  using Data = std::vector<std::string>;    // TODO: replace with Entry

  static Path parse(std::string const& str);

  Path() = default;
  explicit Path(std::vector<std::string> value): value_{ std::move(value) } { }

  Path(Path const&) = default;
  Path& operator=(Path const&) = default;
  Path(Path&&) = default;
  Path& operator=(Path&&) = default;

  std::string str() const;

  auto empty() const { return value_.empty(); }
  auto absolute() const { return not empty() && value_[0] == "."; }
  auto begin() const { return value_.begin(); }
  auto end() const { return value_.end(); }
  auto const& data() const { return value_; }

private:
  Data value_;
};

BUT_MPL_FREE_OPERATORS_COMPARE(Path, .data())

}
