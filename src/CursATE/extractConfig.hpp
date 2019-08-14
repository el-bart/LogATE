#pragma once
#include "CursATE/Config.hpp"
#include <But/Exception.hpp>
#include <But/Optional.hpp>

namespace CursATE
{

BUT_DEFINE_EXCEPTION(InvalidConfig, But::Exception, "invalid config");

But::Optional<Config> extractConfig(int argc, char** argv);

}
