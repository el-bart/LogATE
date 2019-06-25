#pragma once
#include <But/Exception.hpp>
#include <stack>
#include <vector>
#include <string>

namespace LogATE::Json
{

struct Selector final
{
  BUT_DEFINE_EXCEPTION(Error, But::Exception, "JSON selector error");
  BUT_DEFINE_EXCEPTION(UnexpectedCharacter, Error, "unexpected character");
  BUT_DEFINE_EXCEPTION(UnexpectedEndOfStream, Error, "unexpected end of stream");

  void update(char c);
  void eos();
  void reset();
  auto jsonComplete() const { return nestedObjects_ == 0 && nestedArrays_ == 0 && not buffer_.empty() && not isInString_; }
  auto str() const { return std::string{ begin(buffer_), end(buffer_) }; }

private:
  bool isInString_{false};
  bool passThroughNextChar_{false};
  uint32_t nestedObjects_{0};
  uint32_t nestedArrays_{0};
  std::string buffer_;
};

}
