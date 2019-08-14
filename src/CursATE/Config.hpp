#pragma once
#include "LogATE/Net/TcpServer.hpp"
#include "LogATE/Printers/OrderedPrettyPrint.hpp"


namespace CursATE
{

struct Config final
{
  LogATE::Net::Port port_{4242};
  LogATE::Net::TcpServer::JsonParsingMode jsonParsingMode_{LogATE::Net::TcpServer::JsonParsingMode::HardBreakOnNewLine};
  LogATE::Tree::Path keyPath_;
  LogATE::Printers::OrderedPrettyPrint::SilentTags silentTags_;
  LogATE::Printers::OrderedPrettyPrint::PriorityTags priorityTags_;
};

}
