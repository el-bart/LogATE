#include "CursATE/Screen/detail/FilterTreeDataSource.hpp"

namespace CursATE::Screen::detail
{

FilterTreeDataSource::FilterTreeDataSource(LogATE::Tree::NodeShPtr root):
  entries_{ constructEntries(root) }
{ }


size_t FilterTreeDataSource::size() const
{
  return entries_.size();
}


But::Optional<FilterTreeDataSource::Id> FilterTreeDataSource::first() const
{
  BUT_ASSERT( not entries_.empty() );
  return Id{0};
}


But::Optional<FilterTreeDataSource::Id> FilterTreeDataSource::last() const
{
  BUT_ASSERT( not entries_.empty() );
  return Id{ entries_.size()-1u };
}


std::map<FilterTreeDataSource::Id, std::string> FilterTreeDataSource::get(size_t before, Id id, size_t after) const
{
  if( id.value_ > entries_.size() )
    throw std::logic_error{"requested ID in tree, that is out of range"};
  std::map<Id, std::string> out;
  const auto from = ( before > id.value_ ) ? 0u : id.value_ - before;
  const auto to = std::min( entries_.size(), from + before + 1u + after );
  for(auto i=from; i!=to; ++i)
    out[ Id{i} ] = entries_[i].text_;
  return out;
}


FilterTreeDataSource::Id FilterTreeDataSource::node2id(LogATE::Tree::NodeShPtr const& selected) const
{
  for(auto i=0u; i<entries_.size(); ++i)
    if( entries_[i].node_.get() == selected.get() )
      return Id{i};
  BUT_ASSERT( not entries_.empty() );
  // NOTE: entry might be missed, if this was auto-managed node, removed on a fly - default to begin of the list
  return Id{0};
}


LogATE::Tree::NodeShPtr FilterTreeDataSource::id2node(const Id id) const
{
  BUT_ASSERT( id.value_ < entries_.size() );
  return entries_[id.value_].node_;
}


namespace
{
auto nodeName(LogATE::Tree::NodeShPtr const& node)
{
  return node->type().value_ + "::" + node->name().value_;
}

template<typename C>
void appendTree(LogATE::Tree::NodeShPtr const& node, C& out, std::string const& prefix)
{
  using E = typename C::value_type;
  out.push_back( E{ node, prefix + nodeName(node) } );
  const auto newPrefix = prefix + "  ";
  for(auto& c: node->children())
    appendTree(c, out, newPrefix);
}
}


std::vector<FilterTreeDataSource::Entry> FilterTreeDataSource::constructEntries(LogATE::Tree::NodeShPtr const& root) const
{
  std::vector<FilterTreeDataSource::Entry> out;
  appendTree(root, out, "");
  return out;
}

}
