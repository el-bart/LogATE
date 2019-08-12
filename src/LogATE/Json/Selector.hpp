#pragma once
#include <But/Exception.hpp>
#include <But/assert.hpp>
#include <stack>
#include <string>

namespace LogATE::Json
{

BUT_DEFINE_EXCEPTION(Error, But::Exception, "JSON selector error");
BUT_DEFINE_EXCEPTION(UnexpectedCharacter, Error, "unexpected character");
BUT_DEFINE_EXCEPTION(UnexpectedEndOfStream, Error, "unexpected end of stream");


template<bool HardBreakOnNewLine>
struct SelectorImpl final
{
  void update(char c);

  void eos() const
  {
    if( buffer_.empty() )
      return;
    if( not jsonComplete() )
      BUT_THROW(UnexpectedEndOfStream, "still have " << nestedObjects_ + nestedArrays_ << " of states opened");
  }

  void reset()
  {
    isInString_ = false;
    passThroughNextChar_ = false;
    nestedObjects_ = 0;
    nestedArrays_ = 0;
    buffer_.clear();
  }

  bool jsonComplete() const { return nestedObjects_ == 0 && nestedArrays_ == 0 && not buffer_.empty() && not isInString_; }

  auto str() const { return std::string_view{ buffer_.data(), buffer_.size() }; }

private:
  bool isNewLine(const char c) const { return c == '\r' || c == '\n'; }
  bool isWhitespace(const char c) const { return isNewLine(c) || c == ' ' || c == '\t'; }

  bool isInString_{false};
  bool passThroughNextChar_{false};
  uint32_t nestedObjects_{0};
  uint32_t nestedArrays_{0};
  std::string buffer_;
};


template<bool HardBreakOnNewLine>
inline void SelectorImpl<HardBreakOnNewLine>::update(const char c)
{
  if( HardBreakOnNewLine && isNewLine(c) )
  {
    isInString_ = false;
    passThroughNextChar_ = false;
    nestedObjects_ = 0;
    nestedArrays_ = 0;
    return;
  }

  if(isInString_)
  {
    buffer_.push_back(c);
    if(passThroughNextChar_)
    {
      passThroughNextChar_ = false;
      return;
    }
    if(c=='"')
    {
      BUT_ASSERT(not passThroughNextChar_);
      isInString_ = false;
      return;
    }
    if(c=='\\')
    {
      passThroughNextChar_ = true;
      return;
    }
    return;
  }

  if( isWhitespace(c) )
    return;

  buffer_.push_back(c);

  switch(c)
  {
    case '"':
      isInString_ = true;
      return;
    case '{':
      ++nestedObjects_;
      return;
    case '}':
      if( nestedObjects_ == 0 )
        BUT_THROW(UnexpectedCharacter, "trying to end object that was not opened");
      --nestedObjects_;
      return;
    case '[':
      ++nestedArrays_;
      return;
    case ']':
      if( nestedArrays_ == 0 )
        BUT_THROW(UnexpectedCharacter, "trying to end array that was not opened");
      --nestedArrays_;
      return;
  }
}


using Selector = SelectorImpl<false>;
using NewLineSplit = SelectorImpl<true>;

}
