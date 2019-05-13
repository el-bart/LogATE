#pragma once
#include <But/Exception.hpp>
#include <string>

namespace CursATE::Screen::detail
{
BUT_DEFINE_EXCEPTION(NegativeValuesNotSupported, But::Exception, "negative values are not supported");

std::string formatAsPercentage(const double value);
std::string padLeftWithSpaces(std::string str, size_t maxLen);
std::string nOFm(size_t n, size_t m);
std::string nOFmWithPercent(size_t n, size_t m);
}
