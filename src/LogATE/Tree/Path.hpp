#pragma once
#include <vector>
#include <string>
#include <But/Mpl/FreeOperators.hpp>
#include <But/Exception.hpp>

namespace LogATE::Tree
{

struct Path final
{
  using Data = std::vector<std::string>;
  BUT_DEFINE_EXCEPTION(EmptyNodeInPath, But::Exception, "empty node in path");

  static Path parse(std::string const& str);

  Path() = default;
  explicit Path(std::vector<std::string> value): value_{ std::move(value) } { }

  Path(Path const&) = default;
  Path& operator=(Path const&) = default;
  Path(Path&&) = default;
  Path& operator=(Path&&) = default;

  std::string str() const;

  auto empty() const { return value_.empty(); }
  auto root() const { return not empty() && value_[0] == "."; }
  auto begin() const { return value_.begin(); }
  auto end() const { return value_.end(); }
  auto const& data() const { return value_; }

private:
  Data value_;
};

BUT_MPL_FREE_OPERATORS_COMPARE(Path, .data())

}
