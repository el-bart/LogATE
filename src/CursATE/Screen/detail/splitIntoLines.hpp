#pragma once
#include <string>
#include <vector>

namespace CursATE::Screen::detail
{
std::vector<std::string> splitIntoLines(std::string const& in, const size_t availableColumns);
}
