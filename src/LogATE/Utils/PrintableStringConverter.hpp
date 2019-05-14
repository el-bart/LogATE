#pragma once
#include <string>
#include <array>

namespace LogATE::Utils
{

struct PrintableStringConverter final
{
  PrintableStringConverter();

  std::string operator()(std::string const& in) const;

private:
  const std::array<std::string, 256> lut_;
};

}
