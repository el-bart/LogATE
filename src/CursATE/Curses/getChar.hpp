#pragma once
#include <But/Optional.hpp>
#include <chrono>
#include "CursATE/Curses/ctrl.hpp"

namespace CursATE::Curses
{
But::Optional<int> getChar(std::chrono::milliseconds timeout);
}
