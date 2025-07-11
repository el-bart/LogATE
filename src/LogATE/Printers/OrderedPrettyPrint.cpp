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

OrderedPrettyPrint::OrderedPrettyPrint(Config const& config):
  silentTags_{ makeSilentTags(config.silentTags_.tags_) },
  priorityTags_{ config.priorityTags_ },
  paddedFields_{ config.paddedFields_ }
{ }


std::string OrderedPrettyPrint::operator()(LogATE::Log const& in) const
{
  std::stringstream ss;
  const auto snDigits = detail::maxDigits( SequenceNumber::lastIssued().value_ );
  ss << std::setw(snDigits) << std::setfill('0') << in.sequenceNumber().value_ << std::setw(0) << " ";
  ss << in.key().str() << " ";
  const auto & value = in.json();
  if( value.is_object() )
  {
    auto printBrace = false;
    constructObject(ss, value, printBrace);
  }
  else
    printNode( ss, value, paddingFor( in.key().str() ) );
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

void OrderedPrettyPrint::printNode(std::stringstream& ss, nlohmann::json const& in, unsigned const padding) const
{
  if( in.is_object() ) { constructObject(ss, in); return; }
  if( in.is_array() )  { constructArray(ss, in); return; }
  constructValue(ss, in, padding);
}

void OrderedPrettyPrint::printNode(std::stringstream& ss, std::string const& key, nlohmann::json const& value) const
{
  if( not isSilent(key) )
    ss << printable_(key) << "=";
  printNode( ss, value, paddingFor(key) );
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

void OrderedPrettyPrint::constructValue(std::stringstream& ss, nlohmann::json const& in, unsigned padding) const
{
  if( in.is_string() )         { ss << std::setw(padding) << std::setfill(' ') << printable_( in.get<std::string>() ); return; }
  if( in.is_number_integer() ) { ss << std::setw(padding) << std::setfill(' ') << in.get<int64_t>(); return; }
  if( in.is_number_float() )   { ss << std::setw(padding) << std::setfill(' ') << normalFloat( in.get<double>() ); return; }
  if( in.is_boolean() )        { ss << std::setw(padding) << std::setfill(' ') << ( in.get<bool>() ? "true" : "false" ); return; }

  throw std::logic_error{"OrderedPrettyPrint::constructValue(): unsupported value type"};
}

void OrderedPrettyPrint::constructObject(std::stringstream& ss, nlohmann::json const& in, bool printBrace) const
{
  BUT_ASSERT( in.is_object() );
  auto addSpace = false;
  if(printBrace)
  {
    ss << "{";
    addSpace = true;
  }

  for(auto it: prioritizedIteratorsVector(priorityTags_.tags_, in))
  {
    if(addSpace)
      ss << ' ';
    else
      addSpace = true;
    BUT_ASSERT( it != end(in) );
    printNode(ss, it.key(), it.value());
  }

  if(printBrace)
    ss << " }";
}

void OrderedPrettyPrint::constructArray(std::stringstream& ss, nlohmann::json const& in) const
{
  BUT_ASSERT( in.is_array() );
  ss << "[";

  for(auto& e: in)
  {
    ss << ' ';
    printNode(ss, e, 0);
  }

  ss << " ]";
}

bool OrderedPrettyPrint::isSilent(std::string const& tag) const
{
  return silentTags_.find(tag) != end(silentTags_);
}


unsigned OrderedPrettyPrint::paddingFor(std::string const& tag) const
{
  auto const& pf = paddedFields_.padding_;
  auto const it = pf.find(tag);
  if( it == pf.end() )
    return 0;
  return it->second;
}

}
