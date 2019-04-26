#pragma once
#include "LogATE/Log.hpp"
#include <string>
#include <vector>

namespace CursATE::Screen::LogDisplay
{

class OrderedPrettyPrint final
{
public:
  OrderedPrettyPrint() { }
  explicit OrderedPrettyPrint(std::vector<std::string> priorityNodes): priorityNodes_{ std::move(priorityNodes) } { }

  std::string operator()(LogATE::Log const& in) const;

private:
  std::vector<std::string> priorityNodes_;
};

}
