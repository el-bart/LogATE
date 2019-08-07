#pragma once
#include "LogATE/Log.hpp"
#include <But/Exception.hpp>
#include <string>

namespace LogATE::Utils
{

BUT_DEFINE_EXCEPTION(StringDoesNotRepresentKey, But::Exception, "string does not represent key");

LogATE::Log::Key string2key(std::string const& in);

}
