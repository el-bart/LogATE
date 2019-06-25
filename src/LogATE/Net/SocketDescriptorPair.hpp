#pragma once
#include <But/System/Descriptor.hpp>
#include "LogATE/Net/Exception.hpp"

namespace LogATE::Net
{

struct SocketDescriptorPair final
{
  SocketDescriptorPair();

  But::System::Descriptor first;
  But::System::Descriptor second;
};

}
