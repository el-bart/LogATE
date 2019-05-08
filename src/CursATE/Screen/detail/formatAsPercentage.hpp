#pragma once
#include <But/Exception.hpp>
#include <string>

namespace CursATE::Screen::detail
{
BUT_DEFINE_EXCEPTION(NegativeValuesNotSupported, But::Exception, "negative values are not supported");

std::string formatAsPercentage(const double value);
}
