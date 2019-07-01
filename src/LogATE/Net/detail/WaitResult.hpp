#pragma once

namespace LogATE::Net::detail
{

enum class WaitResult
{
  HasData,
  Timeout,
  Interrupted
};

}
