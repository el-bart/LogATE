#pragma once
#include "CursATE/Curses/Exception.hpp"
#include <string>

namespace CursATE::Curses::Field::detail
{

BUT_DEFINE_EXCEPTION(SelectionOutOfRange, Exception, "selection out of range");

struct VisibleSize
{
  size_t start_{0};
  size_t selectionOffset_{0};
  size_t count_{0};
};

VisibleSize resizePaddedVisibleSize(std::string const& in, size_t maxSize, size_t selectedElement);
std::string resizePadded(std::string const& in, size_t maxSize, size_t selectedElement);

}
