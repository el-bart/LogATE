#include "LogATE/Tree/Filter/Explode.hpp"
#include "LogATE/Tree/Filter/AcceptAll.hpp"
#include "LogATE/Tree/Filter/detail/matchesLog.hpp"

namespace LogATE::Tree::Filter
{

namespace
{
auto acceptAllOutput(std::string name)
{
  return But::makeSharedNN<AcceptAll>( Node::Name{std::move(name)} );
}
}

Explode::Explode(Name name, Path path):
  Node{ Type{"Explode"}, std::move(name), {path}},
  path_{std::move(path)},
  nonMatchingChild_{ acceptAllOutput( nonMatchingChildName().value_ ) },
  matchAny_{"", detail::g_defaultRegexType}
{ }

void Explode::insert(Log const& log)
{
  auto values = detail::allValues(log, path_);
  if( values.empty() )
  {
    nonMatchingChild_->insert(log);
    return;
  }
  for(auto value: values)
  {
    auto name = Name{value};
    auto it = children_.find(name);
    if( it == end(children_) )
      it = children_.insert( std::make_pair(name, acceptAllOutput(name.value_)) ).first;
    it->second->insert(log);
  }
}

Explode::Children Explode::children() const
{
  Children out;
  out.reserve( children_.size() + 1 );
  for(auto child: children_)
    out.push_back(child.second);
  out.push_back(nonMatchingChild_);
  return out;
}

void Explode::add(NodePtr node)
{
  BUT_THROW(ExplicitNodeAddNotSupported, "while trying to add node: " << node->name().value_);
}

}
