#pragma once
#include "CursATE/Curses/Exception.hpp"
#include <string>

namespace CursATE::Curses::Field::detail
{

BUT_DEFINE_EXCEPTION(SelectionOutOfRange, Exception, "selection out of range");

std::string resizePadded(std::string const& in, size_t maxSize, size_t selectedElement);

}
