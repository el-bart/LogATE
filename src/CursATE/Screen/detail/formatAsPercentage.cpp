#include "CursATE/Screen/detail/formatAsPercentage.hpp"


namespace CursATE::Screen::detail
{

std::string formatAsPercentage(const double value)
{
  if(value < 0)
    BUT_THROW(NegativeValuesNotSupported, "got: " << std::fixed << value);

  auto str = std::to_string(value * 100.0);
  const auto pos = str.find('.');

  // trim anything after 2 decimal places
  if( pos+3 >= str.size() )
    return str;
  auto it = str.begin() + pos + 3;
  str.erase(it, str.end());

  // add leading spaces for 100.00% to fit exactly
  return padLeftWithSpaces( std::move(str), sizeof("100.00")-1 ) + '%';
}


std::string padLeftWithSpaces(std::string str, const size_t maxLen)
{
  if( str.size() >= maxLen )
    return str;
  const auto missstrg = maxLen - str.size();
  const auto space = ' ';
  str.insert(0, missstrg, space);
  return str;
}


std::string nOFm(const size_t n, const size_t m)
{
  auto M = std::to_string(m);
  auto N = padLeftWithSpaces( std::to_string(n), M.size() );
  return std::move(N) + "/" + std::move(M);
}


std::string nOFmWithPercent(const size_t n, const size_t m)
{
  const auto p = (m!=0) ?  static_cast<double>(n) / static_cast<double>(m) : 1.0;
  return nOFm(n,m) + " (" + formatAsPercentage(p) + ")";
}

}
