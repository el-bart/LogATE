#pragma once
#include "CursATE/Curses/Exception.hpp"

namespace CursATE::Curses
{

struct Init
{
  BUT_DEFINE_EXCEPTION(EnvSetupFailed, Exception, "env setup failed");
  BUT_DEFINE_EXCEPTION(ColorsNotSupported, Exception, "colors not supported");

  Init();
  ~Init();

  Init(Init const&) = delete;
  Init& operator=(Init const&) = delete;

  Init(Init&&) = delete;
  Init& operator=(Init&&) = delete;
};

}
