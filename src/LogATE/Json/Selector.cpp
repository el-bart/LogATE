#include "LogATE/Json/Selector.hpp"
#include <But/assert.hpp>
#include <ctype.h>

namespace LogATE::Json
{

namespace
{
auto isWhitespace(const char c)
{
  return c == ' ' || c == '\r' || c == '\n' || c == '\t';
}
}


void Selector::update(const char c)
{
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


void Selector::eos()
{
  if( buffer_.empty() )
    return;
  if( not jsonComplete() )
    BUT_THROW(UnexpectedEndOfStream, "still have " << nestedObjects_ + nestedArrays_ << " of states opened");
}


void Selector::reset()
{
  isInString_ = false;
  passThroughNextChar_ = false;
  nestedObjects_ = 0;
  nestedArrays_ = 0;
  buffer_.clear();
}

}
