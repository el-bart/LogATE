#pragma once
#include <But/Exception.hpp>

namespace LogATE::Net
{
BUT_DEFINE_EXCEPTION(Exception, ::But::Exception, "network error");
}
