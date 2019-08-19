#include "LogATE/Log.hpp"
#include "LogATE/Tree/Node.hpp"
#include <But/Optional.hpp>

namespace CursATE::Screen::detail
{

class Marks final
{
public:
  struct Entry
  {
    Entry(LogATE::Log::Key key, LogATE::Tree::NodeWeakPtr node):
      key_{ std::move(key) },
      node_{ std::move(node) }
    { }
    Entry(LogATE::Log::Key key, LogATE::Tree::NodeShPtr const& node):
      Entry{ std::move(key), node.underlyingPointer() }
    { }

    LogATE::Log::Key key_;
    LogATE::Tree::NodeWeakPtr node_;
  };

  But::Optional<Entry> find(char k) const;
  void prune();
  void insert(char c, Entry e);
  auto size() const { return data_.size(); }

private:
  std::map<char, Entry> data_;
};

}
