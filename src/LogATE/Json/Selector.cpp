#include "LogATE/Json/Selector.hpp"
#include <But/assert.hpp>
#include <ctype.h>

namespace LogATE::Json
{

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
