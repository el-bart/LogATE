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
  BUT_DEFINE_EXCEPTION(InvalidParserState, Error, "invalid parser state");
  BUT_DEFINE_EXCEPTION(UnexpectedCharacter, Error, "unexpected character");
  BUT_DEFINE_EXCEPTION(InvalidBoolean, Error, "invalid boolean");
  BUT_DEFINE_EXCEPTION(InvalidNull, Error, "invalid null");
  BUT_DEFINE_EXCEPTION(UnexpectedEndOfStream, Error, "unexpected end of stream");

  void update(char c);
  void eos();
  void reset();
  auto jsonComplete() const { return state_.empty() && not buffer_.empty(); }
  auto str() const { return std::string{ begin(buffer_), end(buffer_) }; }

private:
  enum class ParserState: uint8_t
  {
    InsideObject,
    InsideObjectKey,
    InsideObjectExpectColon,
    InsideObjectExpectValue,
    InsideObjectExpectEnd,
    InsideNumber,
    InsideNull,
    InsideBoolFalse,
    InsideBoolTrue,
    AcceptNextCharacter,
    InsideString
  };

  auto jsonNotStarted() const { return state_.empty() && buffer_.empty(); }

  void updateExisting(char c);
  void updateObject(char c);
  void updateObjectKey(char c);
  void updateObjectExpectColon(char c);
  void updateObjectExpectEnd(char c);
  void updateObjectExpectValue(char c);
  void updateArray(char c);
  void updateString(char c);
  void updateBoolTrue(char c);
  void updateBoolFalse(char c);
  void updateNumber(char c);
  void updateNull(char c);

  void startNew(char c);
  void startObject();
  void startObjectKey();
  void startArray();
  void startString();
  void startBoolTrue();
  void startBoolFalse();
  void startNumber(char c);
  void startNull();

  std::string buffer_;
  std::stack<ParserState, std::vector<ParserState>> state_;
};

}
