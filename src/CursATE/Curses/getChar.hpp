#pragma once
#include <But/Optional.hpp>
#include <chrono>

namespace CursATE::Curses
{
But::Optional<int> getChar(std::chrono::milliseconds timeout);
}
