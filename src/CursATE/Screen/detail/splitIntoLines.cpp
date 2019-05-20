#include "CursATE/Screen/detail/splitIntoLines.hpp"
#include "LogATE/Utils/PrintableStringConverter.hpp"
#include <But/assert.hpp>
#include <sstream>
#include <iostream>                 
#include <algorithm>


namespace CursATE::Screen::detail
{

namespace
{
auto isEscape(const char c) { return c == '\\'; }
auto isCr(const char c) { return c == 'n'; }
auto isLf(const char c) { return c == 'r'; }
auto isEolChar(const char c) { return isCr(c) || isLf(c); }

auto hasEscaped(char const c, const std::string::const_iterator begin, const std::string::const_iterator end)
{
  if(begin==end)
    return false;
  if( not isEscape(*begin) )
    return false;
  const auto it = begin + 1;
  BUT_ASSERT(it!=end && "invalid sequence provided - break should not be in the middle of an escape sequence");
  return *it==c;
}
auto hasEscapedCr(const std::string::const_iterator begin, const std::string::const_iterator end) { return hasEscaped('n', begin, end); }
auto hasEscapedLf(const std::string::const_iterator begin, const std::string::const_iterator end) { return hasEscaped('r', begin, end); }

auto findFirstEol(const std::string::const_iterator begin, const std::string::const_iterator end)
{
  for(auto it=begin; it!=end; ++it)
  {
    if( hasEscapedCr(it, end) )
    {
      if( hasEscapedLf(it+2, end) )
        return it+4;
      return it+2;
    }
    if( hasEscapedCr(it, end) )
    {
      if( hasEscapedLf(it+2, end) )
        return it+4;
      return it+2;
    }
    if( isEscape(*it) )
    {
      ++it;
      BUT_ASSERT(it!=end && "invalid sequence - cannot end with broken escape sequence");
    }
  }
  return end;
}

auto findLastInLineBeforeSplitEscape(const std::string::const_iterator begin, const std::string::const_iterator end, const size_t cols)
{
  const auto inRange = static_cast<size_t>( std::distance(begin, end) );
  const auto effectiveEnd = ( inRange >= cols ) ? begin+cols : end;
  for(auto it=begin; it!=effectiveEnd; ++it)
  {
    if( not isEscape(*it) )
      continue;
    if(it+1 == effectiveEnd)
      return it;
    ++it;
  }
  return effectiveEnd;
}

auto findNextLineBreak(const std::string::const_iterator begin, const std::string::const_iterator end, const size_t cols)
{
  const auto newEnd = findLastInLineBeforeSplitEscape(begin, end, cols);
  std::cout << "\nEE:|" << std::string{begin, newEnd} << "| of |" << std::string{begin, end} << "| @ " << cols << " cols" << std::endl;         
  return findFirstEol(begin, newEnd);
}
}

std::vector<std::string> splitIntoLines(std::string const& in, const size_t availableColumns)
{
  if( in.empty() )
    return {};
  if( not std::all_of( begin(in), end(in), isprint ) )
    std::logic_error{"detail::splitIntoLines(): input sequence shall already be escaped"};
  const auto columns = std::max<size_t>(1u, availableColumns);
  std::vector<std::string> out;
  auto it = begin(in);
  while( it != end(in) )
  {
    const auto lb = findNextLineBreak(it, end(in), columns);
    out.emplace_back(it, lb);
    it = lb;
  }
  return out;
}

}
