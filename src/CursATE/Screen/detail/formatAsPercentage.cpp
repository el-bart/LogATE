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
  if( pos < 3 )
    str.insert(0, 3-pos, ' ');

  return std::move(str) + '%';
}

}
