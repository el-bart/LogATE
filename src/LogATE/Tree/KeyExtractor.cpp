#include "LogATE/Tree/KeyExtractor.hpp"

namespace LogATE::Tree
{

std::string KeyExtractor::getKey(nlohmann::json const& in) const
{
  (void)in;
  throw 42; // TODO
}

}
