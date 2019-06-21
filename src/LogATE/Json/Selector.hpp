#pragma once
#include <stack>
#include <string>

namespace LogATE::Json
{

struct Selector final
{
  void update(std::byte b);
  bool jsonComplete() const { return state_.empty() && not buffer_.empty(); }
  void reset();
  auto str() const { return std::string{ begin(buffer_), end(buffer_) }; }

private:
  enum class ParserState: uint8_t
  {
    Object,
    Array,
    Value,
    String,
    Number,
    True,
    False,
    Null,
  };
  std::string buffer_;
  std::stack<ParserState> state_;
};

}
