#include "LogATE/Utils/value2str.hpp"

namespace LogATE::Utils
{

But::Optional<std::string> value2str(nlohmann::json const& node)
{
  if( node.is_string() )
    return node.get<std::string>();
  if( node.is_number_integer() )
    return std::to_string( node.get<int64_t>() );
  if( node.is_number_float() )
    return std::to_string( node.get<double>() );
  if( node.is_boolean() )
    return std::string{ node.get<bool>() ? "true" : "false" };
  return {};
}

}
