#include "LogATE/Tree/Filter/AcceptAll.hpp"
#include <regex>

namespace LogATE::Tree::Filter
{

AcceptAll::AcceptAll(Utils::WorkerThreadsShPtr workers, Name name):
  SimpleNode{ std::move(workers), Type{"AcceptAll"}, std::move(name), {} }
{
}

bool AcceptAll::matches(Log const&) const
{
  return true;
}

}
