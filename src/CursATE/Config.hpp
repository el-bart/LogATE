#pragma once
#include "LogATE/Net/TcpServer.hpp"
#include "LogATE/Tree/Node.hpp"
#include "LogATE/Tree/KeyExtractor.hpp"
#include "LogATE/Printers/OrderedPrettyPrint.hpp"


namespace CursATE
{

struct Config final
{
  LogATE::Net::Port port_{0};
  LogATE::Net::TcpServer::JsonParsingMode jsonParsingMode_{LogATE::Net::TcpServer::JsonParsingMode::HardBreakOnNewLine};
  LogATE::Tree::KeyExtractorShNN keyExtractor_;
  LogATE::Printers::OrderedPrettyPrint::SilentTags silentTags_;
  LogATE::Printers::OrderedPrettyPrint::PriorityTags priorityTags_;
  LogATE::Tree::Node::TrimFields trimFields_;
};

}
