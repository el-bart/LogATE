#include "LogATE/Printers/OrderedPrettyPrint.hpp"
#include "LogATE/Printers/detail/maxDigits.hpp"
#include <iomanip>

namespace LogATE::Printers
{

namespace
{
auto makeSilentTags(std::vector<std::string> const& in)
{
  std::unordered_set<std::string> out;
  for(auto& e: in)
    out.insert(e);
  return out;
}
}

OrderedPrettyPrint::OrderedPrettyPrint(PriorityTags const& priorityTags, SilentTags const& silentTags):
  silentTags_{ makeSilentTags(silentTags.tags_) },
  priorityTags_{ priorityTags }
{ }


std::string OrderedPrettyPrint::operator()(LogATE::Log const& in) const
{
  std::stringstream ss;
  const auto snDigits = detail::maxDigits( SequenceNumber::lastIssued().value_ );
  ss << std::setw(snDigits) << std::setfill('0') << in.sequenceNumber().value_ << std::setw(0) << " ";
  const auto & value = in.json();
  if( value.is_array() )
    constructArray(ss, value);
  else
    constructString(ss, value);
  return ss.str();
}


namespace
{
auto normalFloat(double in)
{
  auto str = std::to_string(in);
  while( not str.empty() && ( *str.rbegin() == '0' || *str.rbegin() == '.' ) )
    str.erase( str.size()-1 );
  return str;
}
}

void OrderedPrettyPrint::printNode(std::stringstream& ss, std::string const& key, nlohmann::json const& value) const
{
  if( not isSilent(key) )
    ss << printable_(key) << "=";

  if( value.is_string() )         { ss << printable_( value.get<std::string>() ); return; }
  if( value.is_number_integer() ) { ss << value.get<int64_t>(); return; }
  if( value.is_number_float() )   { ss << normalFloat( value.get<double>() ); return; }
  if( value.is_boolean() )        { ss << ( value.get<bool>() ? "true" : "false" ); return; }
  if( value.is_object() )         { constructObject(ss, value); return; }
  if( value.is_array() )          { constructArray(ss, value); return; }

  throw std::logic_error{"OrderedPrettyPrint::printNode(): unsupported value type"};
}


namespace
{
auto prioritizedIteratorsVector(std::vector<std::string> const& priorityTags, nlohmann::json const& in)
{
  std::vector<nlohmann::json::const_iterator> tmp;
  tmp.reserve( in.size() );
  for(auto it=begin(in); it!=end(in); ++it)
    tmp.push_back(it);

  std::vector<nlohmann::json::const_iterator> out;
  out.reserve( tmp.size() );
  for(auto& t: priorityTags)
    if( in.count(t) )
    {
      const auto it = std::find_if( begin(tmp), end(tmp), [&](auto& e) { return t == e.key(); } );
      BUT_ASSERT( it != end(tmp) );
      out.push_back(*it);
      tmp.erase(it);
    }
  out.insert( end(out), begin(tmp), end(tmp) );
  return out;
}
}

void OrderedPrettyPrint::constructString(std::stringstream& ss, nlohmann::json const& in) const
{
  auto printSpace = false;
  for(auto it: prioritizedIteratorsVector(priorityTags_.tags_, in))
  {
    if(printSpace)
      ss << ' ';
    else
      printSpace = true;
    BUT_ASSERT( it != end(in) );
    printNode(ss, it.key(), it.value());
  }
}

void OrderedPrettyPrint::constructObject(std::stringstream& ss, nlohmann::json const& in) const
{
  BUT_ASSERT( in.is_object() );
  ss << "{ ";
  constructString(ss, in);
  ss << " }";
}

void OrderedPrettyPrint::constructArray(std::stringstream& ss, nlohmann::json const& in) const
{
  BUT_ASSERT( in.is_array() );
  ss << "[ ";
  constructString(ss, in);
  ss << " ]";
}

bool OrderedPrettyPrint::isSilent(std::string const& tag) const
{
  return silentTags_.find(tag) != end(silentTags_);
}

}
