#include "CursATE/Screen/detail/splitIntoLines.hpp"
#include "LogATE/Utils/PrintableStringConverter.hpp"
#include <But/assert.hpp>
#include <sstream>
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
auto hasEscapedCr(const std::string::const_iterator begin, const std::string::const_iterator end)  { return hasEscaped('n', begin, end); }
auto hasEscapedLf(const std::string::const_iterator begin, const std::string::const_iterator end)  { return hasEscaped('r', begin, end); }
auto hasEscapedTab(const std::string::const_iterator begin, const std::string::const_iterator end) { return hasEscaped('t', begin, end); }

auto findEndOfEscapeSequence(const std::string::const_iterator begin, const std::string::const_iterator end)
{
  BUT_ASSERT(begin!=end);
  BUT_ASSERT( isEscape(*begin) );
  const auto next = begin+1;
  if(*next=='x')
  {
    BUT_ASSERT( std::distance(begin, end) >= 4 );
    return begin+4;
  }
  return begin+2;
}

auto endOfEscapeSequenceBlock(const std::string::const_iterator it, const std::string::const_iterator end)
{
  BUT_ASSERT(it!=end);
  BUT_ASSERT( isEscape(*it) );
  if( hasEscapedCr(it, end) )
  {
    if( hasEscapedLf(it+2, end) )
      return it+4;
    return it+2;
  }
  if( hasEscapedLf(it, end) )
  {
    if( hasEscapedCr(it+2, end) )
      return it+4;
    return it+2;
  }
  return findEndOfEscapeSequence(it, end);
}

auto findNextLineBreak(const std::string::const_iterator begin, const std::string::const_iterator end, const size_t cols)
{
  const auto inRange = static_cast<size_t>( std::distance(begin, end) );
  const auto effectiveEnd = ( inRange >= cols ) ? begin+cols : end;

  auto it = begin;
  auto goodEOL = begin;
  auto possibleEOL = begin;

  while(it!=end)
  {
    if(effectiveEnd <= it)
      break;
    if(*it==' ')
    {
      ++it;
      goodEOL = it;
      possibleEOL = it;
      continue;
    }
    if(*it=='\\')
    {
      goodEOL = it;
      possibleEOL = it;
      const auto eoe = endOfEscapeSequenceBlock(it, end);
      BUT_ASSERT( eoe <= end );
      if( effectiveEnd < eoe )
        break;
      const auto prev = it;
      goodEOL = eoe;
      possibleEOL = eoe;
      it = eoe;
      if( hasEscapedCr(prev, end) || hasEscapedLf(prev, end) )
        break;
      continue;
    }
    ++it;
    possibleEOL = it;
  }

  if(it==end)
    return end;
  if(goodEOL!=begin)
    return goodEOL;
  return possibleEOL;
}
}

std::vector<std::string> splitIntoLines(std::string const& in, const size_t availableColumns)
{
  if( in.empty() )
    return {};
  if( not std::all_of( begin(in), end(in), isprint ) )
    std::logic_error{"detail::splitIntoLines(): input sequence shall already be escaped"};
  const auto columns = std::max<size_t>(4u, availableColumns);
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
