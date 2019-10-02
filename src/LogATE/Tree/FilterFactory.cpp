#include "LogATE/Tree/FilterFactory.hpp"
#include "LogATE/Tree/Filter/To.hpp"
#include "LogATE/Tree/Filter/From.hpp"
#include "LogATE/Tree/Filter/Grep.hpp"
#include "LogATE/Tree/Filter/Explode.hpp"
#include "LogATE/Tree/Filter/BinarySplit.hpp"
#include "LogATE/Tree/Filter/AcceptAll.hpp"
#include "LogATE/Utils/string2key.hpp"
#include <boost/lexical_cast.hpp>


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

auto extractTrimFields(std::string const& type, FilterFactory::Options& options, std::string const& name)
{
  (void)type;
  Node::TrimFields tf;
  auto it = options.find(name);
  if( it == end(options) )
    return tf;
  const auto array = nlohmann::json::parse(it->second);
  if( not array.is_array() )
    BUT_THROW(FilterFactory::InvalidValue, "'" << type << "' filter's 'Trim' option expects JSON array");
  for(auto& e: array)
    tf.push_back( Path::parse( e.get<std::string>() ) );
  options.erase(it);
  return tf;
}

std::unique_ptr<Node> buildAcceptAll(Utils::WorkerThreadsShPtr workers, FilterFactory::Name name, FilterFactory::Options options)
{
  const auto type = std::string{"AcceptAll"};
  auto tf = extractTrimFields(type, options, "Trim");
  if( not options.empty() )
    BUT_THROW(FilterFactory::UnknownOption, "filter " << name.value_ << "; unknown option: " << options.begin()->first);
  return std::make_unique<Filter::AcceptAll>( std::move(workers), std::move(name), std::move(tf) );
}

auto extractKey(std::string const& type, FilterFactory::Options& options, std::string const& name)
{
  const auto str = extractOption(type, options, name);
  try
  {
    return Utils::string2key(str);
  }
  catch(std::exception const& ex)
  {
    BUT_THROW(FilterFactory::InvalidValue, "'" << name << "' value invalid: " << str);
  }
}

std::unique_ptr<Node> buildFrom(Utils::WorkerThreadsShPtr workers, FilterFactory::Name name, FilterFactory::Options options)
{
  const auto type = std::string{"From"};
  const auto edge = extractKey(type, options, "Edge");
  if( not options.empty() )
    BUT_THROW(FilterFactory::UnknownOption, "filter " << name.value_ << "; unknown option: " << options.begin()->first);
  return std::make_unique<Filter::From>( std::move(workers), std::move(name), edge );
}

std::unique_ptr<Node> buildTo(Utils::WorkerThreadsShPtr workers, FilterFactory::Name name, FilterFactory::Options options)
{
  const auto type = std::string{"To"};
  const auto edge = extractKey(type, options, "Edge");
  if( not options.empty() )
    BUT_THROW(FilterFactory::UnknownOption, "filter " << name.value_ << "; unknown option: " << options.begin()->first);
  return std::make_unique<Filter::To>( std::move(workers), std::move(name), edge );
}

std::unique_ptr<Node> buildExplode(Utils::WorkerThreadsShPtr workers, FilterFactory::Name name, FilterFactory::Options options)
{
  const auto type = std::string{"Explode"};
  auto path = extractPath(type, options, "Path");
  if( not options.empty() )
    BUT_THROW(FilterFactory::UnknownOption, "filter " << name.value_ << " unknown option: " << options.begin()->first);
  return std::make_unique<Filter::Explode>( std::move(workers), std::move(name), std::move(path) );
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

auto extractTrim(std::string const& type, FilterFactory::Options& options, std::string const& name)
{
  const auto str = extractOption(type, options, name);
  if( str == "False" )
    return Filter::Grep::Trim::False;
  if( str == "True" )
    return Filter::Grep::Trim::True;
  BUT_THROW(FilterFactory::InvalidValue, "'" << name << "' value invalid: " << str);
}

std::unique_ptr<Node> buildGrep(Utils::WorkerThreadsShPtr workers, FilterFactory::Name name, FilterFactory::Options options)
{
  const auto type = std::string{"Grep"};
  auto path = extractPath(type, options, "Path");
  auto regex = extractOption(type, options, "regex");
  const auto compare = extractCompare(type, options, "Compare");
  const auto searchCase = extractCase(type, options, "Case");
  const auto trim = extractTrim(type, options, "Trim");
  const auto search = extractSearch(type, options, "Search");
  if( not options.empty() )
    BUT_THROW(FilterFactory::UnknownOption, "filter " << name.value_ << " unknown option: " << options.begin()->first);
  return std::make_unique<Filter::Grep>( std::move(workers),
                                         std::move(name),
                                         std::move(path),
                                         std::move(regex),
                                         compare,
                                         searchCase,
                                         search,
                                         trim );
}

std::unique_ptr<Node> buildBinarySplit(Utils::WorkerThreadsShPtr workers, FilterFactory::Name name, FilterFactory::Options options)
{
  auto grep = NodePtr{ buildGrep( workers, FilterFactory::Name{"<matched>"}, std::move(options) ) };
  BUT_ASSERT( options.empty() );
  return std::make_unique<Filter::BinarySplit>( std::move(workers), std::move(name), std::move(grep) );
}
}


FilterFactory::FilterFactory(Utils::WorkerThreadsShPtr workers):
  workers_{ std::move(workers) }
{
  factory_.add( Type{"AcceptAll"}, buildAcceptAll );
  factory_.add( Type{"Explode"}, buildExplode );
  factory_.add( Type{"BinarySplit"}, buildBinarySplit );
  factory_.add( Type{"From"}, buildFrom );
  factory_.add( Type{"To"}, buildTo );
  factory_.add( Type{"Grep"}, buildGrep );
}

NodePtr FilterFactory::build(Type type, Name name, Options options)
{
  auto workersCopy = workers_;
  auto ptr = factory_.build( std::move(type), std::move(workersCopy), std::move(name), std::move(options) );
  return NodePtr{ std::move(ptr) };
}

}
