#include "LogATE/Tree/FilterFactory.hpp"
#include "LogATE/Tree/Filter/Grep.hpp"
#include "LogATE/Tree/Filter/Explode.hpp"
#include "LogATE/Tree/Filter/AcceptAll.hpp"

namespace LogATE::Tree
{

namespace
{
auto extractOption(std::string const& type, FilterFactory::Options& options, std::string const& name)
{
  auto it = options.find(name);
  if( it == end(options) )
    BUT_THROW(FilterFactory::MissingOption, "'" << type << "' filter is missing option '" << name << "'");
  auto value = std::move(it->second);
  options.erase(it);
  return value;
}

auto extractPath(std::string const& type, FilterFactory::Options& options, std::string const& name)
{
  const auto str = extractOption(type, options, name);
  return Path::parse(str);
}

std::unique_ptr<Node> buildAcceptAll(FilterFactory::Name name, FilterFactory::Options options)
{
  if( not options.empty() )
    BUT_THROW(FilterFactory::UnknownOption, "filter " << name.value_ << " does not expext any options; "
                                            << "unknown option: " << options.begin()->first);
  return std::make_unique<Filter::AcceptAll>( std::move(name) );
}

std::unique_ptr<Node> buildExplode(FilterFactory::Name name, FilterFactory::Options options)
{
  const auto type = std::string{"Explode"};
  auto path = extractPath(type, options, "Path");
  if( not options.empty() )
    BUT_THROW(FilterFactory::UnknownOption, "filter " << name.value_ << " unknown option: " << options.begin()->first);
  return std::make_unique<Filter::Explode>( std::move(name), std::move(path) );
}

auto extractCompare(std::string const& type, FilterFactory::Options& options, std::string const& name)
{
  const auto str = extractOption(type, options, name);
  if( str == "Key" )
    return Filter::Grep::Compare::Key;
  if( str == "Value" )
    return Filter::Grep::Compare::Value;
  BUT_THROW(FilterFactory::InvalidValue, "'" << name << "' value invalid: " << str);
}

auto extractCase(std::string const& type, FilterFactory::Options& options, std::string const& name)
{
  const auto str = extractOption(type, options, name);
  if( str == "Sensitive" )
    return Filter::Grep::Case::Sensitive;
  if( str == "Insensitive" )
    return Filter::Grep::Case::Insensitive;
  BUT_THROW(FilterFactory::InvalidValue, "'" << name << "' value invalid: " << str);
}

auto extractSearch(std::string const& type, FilterFactory::Options& options, std::string const& name)
{
  const auto str = extractOption(type, options, name);
  if( str == "Regular" )
    return Filter::Grep::Search::Regular;
  if( str == "Inverse" )
    return Filter::Grep::Search::Inverse;
  BUT_THROW(FilterFactory::InvalidValue, "'" << name << "' value invalid: " << str);
}

std::unique_ptr<Node> buildGrep(FilterFactory::Name name, FilterFactory::Options options)
{
  const auto type = std::string{"Grep"};
  auto path = extractPath(type, options, "Path");
  auto regex = extractOption(type, options, "regex");
  const auto compare = extractCompare(type, options, "Compare");
  const auto searchCase = extractCase(type, options, "Case");
  const auto search = extractSearch(type, options, "Search");
  if( not options.empty() )
    BUT_THROW(FilterFactory::UnknownOption, "filter " << name.value_ << " unknown option: " << options.begin()->first);
  return std::make_unique<Filter::Grep>( std::move(name), std::move(path), std::move(regex), compare, searchCase, search );
}
}


FilterFactory::FilterFactory()
{
  factory_.add( Type{"AcceptAll"}, buildAcceptAll );
  factory_.add( Type{"Explode"}, buildExplode );
  factory_.add( Type{"Grep"}, buildGrep );
}

NodePtr FilterFactory::build(Type type, Name name, Options options)
{
  auto ptr = factory_.build( std::move(type), std::move(name), std::move(options) );
  return NodePtr{ std::move(ptr) };
}

}
