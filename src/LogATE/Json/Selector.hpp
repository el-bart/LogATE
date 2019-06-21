#pragma once
#include <But/Exception.hpp>
#include <stack>
#include <string>

namespace LogATE::Json
{

struct Selector final
{
  BUT_DEFINE_EXCEPTION(Error, But::Exception, "JSON selector error");
  BUT_DEFINE_EXCEPTION(InvalidParserState, Error, "invalid parser state");
  BUT_DEFINE_EXCEPTION(UnexpectedCharacter, Error, "unexpected character");

  void update(char c);
  void reset();
  auto jsonComplete() const { return state_.empty() && not buffer_.empty(); }
  auto str() const { return std::string{ begin(buffer_), end(buffer_) }; }

private:
  enum class ParserState: uint8_t
  {
    AcceptNextCharacter,
    InsideString
  };

  auto jsonNotStarted() const { return state_.empty() && buffer_.empty(); }

  void updateExisting(char c);
  void updateObject(char c);
  void updateArray(char c);
  void updateString(char c);
  void updateBoolTrue(char c);
  void updateBoolFalse(char c);
  void updateNumber(char c);

  void startNew(char c);
  void startObject();
  void startArray();
  void startString();
  void startBoolTrue();
  void startBoolFalse();
  void startNumber();

  std::string buffer_;
  std::stack<ParserState> state_;
};

}
