#include "LogATE/Utils/PrintableStringConverter.hpp"
#include <sstream>
#include <cstring>

namespace LogATE::Utils
{

namespace
{
auto toHex(unsigned n)
{
  char buf[6];
  sprintf(buf, "\\x%02X", n);
  return std::string{buf};
}

auto makeLut()
{
  std::array<std::string, 256> lut;

  for(auto i=0u; i<256; ++i)
  {
    if( isprint(i) )
      lut[i] = char(i);
    else
      lut[i] = toHex(i);
  }

  lut[0x00] = "\\0";
  lut[0x07] = "\\a";
  lut[0x08] = "\\b";
  lut[0x09] = "\\t";
  lut[0x0a] = "\\n";
  lut[0x0b] = "\\v";
  lut[0x0c] = "\\f";
  lut[0x0d] = "\\r";

  return lut;
}

auto lutCopy()
{
  static const auto lut = makeLut();
  return lut;
}
}


PrintableStringConverter::PrintableStringConverter():
  lut_{ lutCopy() }
{ }


std::string PrintableStringConverter::operator()(std::string const& in) const
{
  std::stringstream ss;
  for(auto c: in)
    ss << lut_[ static_cast<uint8_t>(c) ];
  return ss.str();
}

}
