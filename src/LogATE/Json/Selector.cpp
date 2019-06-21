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
  if( state_.empty() && isWhitespace(c) )
    return;
  if( jsonNotStarted() )
    startNew(c);
  else
    updateExisting(c);
}


void Selector::eos()
{
  if( state_.empty() )
    return;
  if( state_.size() == 1 && state_.top() == ParserState::InsideNumber )
  {
    state_.pop();
    return;
  }
  BUT_THROW(UnexpectedEndOfStream, "still have " << state_.size() << " of states opened");
}


void Selector::reset()
{
  buffer_.clear();
  while( not state_.empty() )
    state_.pop();
}


void Selector::updateExisting(const char c)
{
  if( state_.empty() )
    BUT_THROW(InvalidParserState, "while processing '" << c << "' character, internal state is empty, buf buffer is not");
  switch( state_.top() )
  {
    case ParserState::InsideArrayElement: updateArrayElement(c); return;
    case ParserState::InsideArray: updateArray(c); return;
    case ParserState::InsideObjectExpectValue: updateObjectExpectValue(c); return;
    case ParserState::InsideObjectExpectColon: updateObjectExpectColon(c); return;
    case ParserState::InsideObjectExpectEnd: updateObjectExpectEnd(c); return;
    case ParserState::InsideObject: updateObject(c); return;
    case ParserState::InsideObjectKey: updateObjectKey(c); return;
    case ParserState::InsideNumber: updateNumber(c); return;
    case ParserState::InsideNull: updateNull(c); return;
    case ParserState::InsideBoolFalse: updateBoolFalse(c); return;
    case ParserState::InsideBoolTrue: updateBoolTrue(c); return;
    case ParserState::InsideString: updateString(c); return;
    case ParserState::AcceptNextCharacter: buffer_.push_back(c); state_.pop(); return;
  }
  BUT_ASSERT(!"how on earth did you get here?!");
}


void Selector::updateObject(const char c)
{
  if( isWhitespace(c) )
    return;
  if(c == '"')
  {
    startObjectKey();
    return;
  }
  if(c == '}')
  {
    buffer_.push_back(c);
    state_.pop();
    return;
  }
  BUT_THROW(UnexpectedCharacter, "when updating object - got '" << c << "'");
}


void Selector::updateObjectKey(const char c)
{
  buffer_.push_back(c);
  if(c == '"')
  {
    state_.pop();
    state_.push(ParserState::InsideObjectExpectColon);
    return;
  }
  if(c == '\\')
  {
    state_.push(ParserState::AcceptNextCharacter);
    return;
  }
}


void Selector::updateObjectExpectColon(const char c)
{
  if( isWhitespace(c) )
    return;
  if(c == ':')
  {
    buffer_.push_back(c);
    state_.pop();
    state_.push(ParserState::InsideObjectExpectValue);
    return;
  }
  BUT_THROW(UnexpectedCharacter, "when updating object - expected ':' or a whitespace, got '" << c << "'");
}


void Selector::updateObjectExpectEnd(const char c)
{
  if( isWhitespace(c) )
    return;
  if(c == ',')
  {
    buffer_.push_back(c);
    state_.pop();
    state_.push(ParserState::InsideObject);
    return;
  }
  if(c == '}')
  {
    buffer_.push_back(c);
    state_.pop();
    return;
  }
  BUT_THROW(UnexpectedCharacter, "when expecting object end - expected '}' or ',' or a whitespace, got '" << c << "'");
}


void Selector::updateObjectExpectValue(const char c)
{
  if( isWhitespace(c) )
    return;
  state_.pop();
  state_.push(ParserState::InsideObjectExpectEnd);
  startNew(c);
}


void Selector::updateArray(const char c)
{
  if( isWhitespace(c) )
    return;
  if(c == ']')
  {
    buffer_.push_back(c);
    state_.pop();
    return;
  }
  state_.pop();
  state_.push(ParserState::InsideArrayElement);
  startNew(c);
}


void Selector::updateArrayElement(const char c)
{
  if( isWhitespace(c) )
    return;
  if(c == ',')
  {
    state_.pop();
    startNew(' ');
    return;
  }
  if(c == ']')
  {
    buffer_.push_back(c);
    BUT_ASSERT( state_.top() == ParserState::InsideArrayElement );
    state_.pop();
    return;
  }
  BUT_THROW(UnexpectedCharacter, "when expecting array end - expected ']' or ',' or a whitespace, got '" << c << "'");
}


void Selector::updateString(const char c)
{
  buffer_.push_back(c);
  if(c == '"')
  {
    state_.pop();
    return;
  }
  if(c == '\\')
  {
    state_.push(ParserState::AcceptNextCharacter);
    return;
  }
}


void Selector::updateBoolTrue(const char c)
{
  buffer_.push_back(c);
  if(c == 'r' || c == 'u')
    return;
  if(c == 'e')
  {
    state_.pop();
    return;
  }
  BUT_THROW(InvalidBoolean, "unexpected char '" << c << "' in bool/true");
}


void Selector::updateBoolFalse(const char c)
{
  buffer_.push_back(c);
  if(c == 'a' || c == 'l' || c == 's')
    return;
  if(c == 'e')
  {
    state_.pop();
    return;
  }
  BUT_THROW(InvalidBoolean, "unexpected char '" << c << "' in bool/false");
}


void Selector::updateNumber(const char c)
{
  if( isdigit(c) || c == 'e' || c == 'E' || c == '+' || c == '-' || c == '.' )
  {
    buffer_.push_back(c);
    return;
  }
  state_.pop();
  update(c);
}


void Selector::updateNull(const char c)
{
  if(c == 'u')
  {
    buffer_.push_back(c);
    return;
  }
  if(c == 'l')
  {
    BUT_ASSERT( not buffer_.empty() );
    if( buffer_.back() == 'l' )
      state_.pop();
    buffer_.push_back(c);
    return;
  }
  BUT_THROW(InvalidNull, "unexpected char '" << c << "' in null value");
}


void Selector::startNew(char c)
{
  switch(c)
  {
    case '{': startObject(); return;
    case '[': startArray(); return;
    case '"': startString(); return;
    case 't': startBoolTrue(); return;
    case 'f': startBoolFalse(); return;
    case 'n': startNull(); return;
  }
  if( isdigit(c) || c == '-' )
  {
    startNumber(c);
    return;
  }
  if( isWhitespace(c) )
    return;
  BUT_THROW(UnexpectedCharacter, "got '" << c << "' where new object was expected");
}


void Selector::startObject()
{
  buffer_.push_back('{');
  state_.push(ParserState::InsideObject);
}


void Selector::startObjectKey()
{
  buffer_.push_back('"');
  BUT_ASSERT( state_.top() == ParserState::InsideObject );
  state_.pop();
  state_.push(ParserState::InsideObjectKey);
}


void Selector::startArray()
{
  buffer_.push_back('[');
  state_.push(ParserState::InsideArray);
}


void Selector::startString()
{
  buffer_.push_back('"');
  state_.push(ParserState::InsideString);
}


void Selector::startBoolTrue()
{
  buffer_.push_back('t');
  state_.push(ParserState::InsideBoolTrue);
}


void Selector::startBoolFalse()
{
  buffer_.push_back('f');
  state_.push(ParserState::InsideBoolFalse);
}


void Selector::startNumber(const char c)
{
  buffer_.push_back(c);
  state_.push(ParserState::InsideNumber);
}


void Selector::startNull()
{
  buffer_.push_back('n');
  state_.push(ParserState::InsideNull);
}

}
