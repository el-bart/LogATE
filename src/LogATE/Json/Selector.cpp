#include "LogATE/Json/Selector.hpp"
#include <But/assert.hpp>

namespace LogATE::Json
{

void Selector::update(const char c)
{
  if( jsonNotStarted() )
    startNew(c);
  else
    updateExisting(c);
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
  (void)c;
  // TODO
}


void Selector::updateArray(const char c)
{
  (void)c;
  // TODO
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
  (void)c;
  // TODO
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
    // number:
    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': startNumber(); return;
    // white space:
    case ' ':
    case '\t':
    case '\n':
    case '\r': return;
  }
  BUT_THROW(UnexpectedCharacter, "got '" << c << "' where new object was expected");
}


void Selector::startObject()
{
  // TODO
}


void Selector::startArray()
{
  // TODO
}


void Selector::startString()
{
  buffer_.push_back('"');
  state_.push( ParserState::InsideString );
}


void Selector::startBoolTrue()
{
  buffer_.push_back('t');
  state_.push( ParserState::InsideBoolTrue );
}


void Selector::startBoolFalse()
{
  buffer_.push_back('f');
  state_.push( ParserState::InsideBoolFalse );
}


void Selector::startNumber()
{
  // TODO
}


void Selector::startNull()
{
  buffer_.push_back('n');
  state_.push( ParserState::InsideNull );
}

}
