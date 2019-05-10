#include "LogATE/Tree/Filter/AcceptAll.hpp"
#include <regex>

namespace LogATE::Tree::Filter
{

AcceptAll::AcceptAll(Utils::WorkerThreadsShPtr workers, Name name, TrimFields tf):
  SimpleNode{ std::move(workers), Type{"AcceptAll"}, std::move(name), std::move(tf) }
{
}

bool AcceptAll::matches(AnnotatedLog const&) const
{
  return true;
}

}
