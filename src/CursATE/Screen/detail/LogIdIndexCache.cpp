#include "CursATE/Screen/detail/LogIdIndexCache.hpp"
#include <vector>
#include <mutex>

namespace CursATE::Screen::detail
{

size_t LogIdIndexCache::index(LogATE::Log::Key key) const
{
  const auto node = node_.lock();
  if(not node)
    return 0;
  const auto ll = node->logs().withLock();
  if( ll->empty() )
    return 0;

  const auto it = ll->lower_bound(key);
  if( it == ll->end() )
    return 0;
  if( it->key() != key )
    return 0;
  return std::distance( ll->begin(), it );  // TODO: LINEAR!!!!!!!!!!!!!
}

}
