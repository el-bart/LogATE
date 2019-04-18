#include "LogATE/Tree/FilterFactory.hpp"
#include "LogATE/Tree/Filter/Grep.hpp"
#include "LogATE/Tree/Filter/Explode.hpp"
#include "LogATE/Tree/Filter/AcceptAll.hpp"

namespace LogATE::Tree
{

namespace
{
std::unique_ptr<Node> buildAcceptAll(FilterFactory::Name name, FilterFactory::Options options)
{
  if( not options.empty() )
    BUT_THROW(FilterFactory::UnknownOption, "filter " << name.value_ << " does not expect any erguments, yet provided: " << options.size());
  return std::make_unique<Filter::AcceptAll>( std::move(name) );
}
}


FilterFactory::FilterFactory()
{
  factory_.add( Type{"AcceptAll"}, buildAcceptAll );
  // TODO
}

NodePtr FilterFactory::build(Type type, Name name, Options options)
{
  auto ptr = factory_.build( std::move(type), std::move(name), std::move(options) );
  return NodePtr{ std::move(ptr) };
}

}
