#pragma once
#include "LogATE/Net/TcpServer.hpp"
#include "LogATE/Printers/OrderedPrettyPrint.hpp"


namespace CursATE
{

struct Config final
{
  LogATE::Net::Port port_{4242};
  LogATE::Net::TcpServer::JsonParsingMode jsonParsingMode_{ LogATE::Net::TcpServer::JsonParsingMode::ParseToEndOfJson };
  LogATE::Tree::Path keyPath_{ LogATE::Tree::Path::parse(".But::PreciseDT") };
  LogATE::Printers::OrderedPrettyPrint::SilentTags silentTags_{{"But::PreciseDT", "Priority", "ComponentId", "UniqueId", "string"}};
  LogATE::Printers::OrderedPrettyPrint::PriorityTags priorityTags_{{"But::PreciseDT", "Priority", "ComponentId", "UniqueId", "But::ThreadNo", "string"}};
};

}
