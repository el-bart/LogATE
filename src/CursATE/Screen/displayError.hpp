#pragma once
#include <But/Exception.hpp>
#include <string>
#include <vector>

namespace CursATE::Screen
{
BUT_DEFINE_EXCEPTION(ErrorWindowTooSmall, But::Exception, "error window too small");

void displayError(std::vector<std::string> const& lines);
void displayError(std::exception const& ex);
}
