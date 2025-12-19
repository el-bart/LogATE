#include "CursATE/Screen/detail/FilterWindows.hpp"
#include "CursATE/Screen/detail/LogDataSource.hpp"
#include "CursATE/Screen/detail/formatAsPercentage.hpp"
#include "LogATE/Utils/trimFields.hpp"
#include <sstream>

using LogATE::Log;
using LogATE::Utils::trimFields;

namespace CursATE::Screen::detail
{

FilterWindows::FilterWindows(std::function<std::string(LogATE::Log const&)> log2str,
                             std::function<size_t()> inputErrors,
                             std::function<std::string()> workerThreadsStats):
  log2str_{ std::move(log2str) },
  inputErrors_{ std::move(inputErrors) },
  workerThreadsStats_{ std::move(workerThreadsStats) }
{ }


But::NotNullShared<Curses::ScrolableWindow> FilterWindows::window(LogATE::Tree::NodeShPtr const& node)
{
  auto f = [&](auto const& e) { return e.node_.lock().get() == node.get(); };
  const auto it = std::find_if( begin(entries_), end(entries_), f );
  if( it != end(entries_) )
    return it->win_;
  const auto win = newWindow(node);
  win->selectLast();
  entries_.push_back( Element{ node.underlyingPointer(), win } );
  return win;
}


void FilterWindows::prune()
{
  auto f = [&](auto const& e) { return e.node_.lock().get() == nullptr; };
  const auto it = std::remove_if( begin(entries_), end(entries_), f );
  entries_.erase(it, end(entries_));
}


But::NotNullShared<Curses::ScrolableWindow> FilterWindows::newWindow(LogATE::Tree::NodeShPtr node) const
{
  auto const n = node;
  auto trimmedLog2str = [toStr=log2str_, tf=node->trimFields()](Log const& in) { return toStr( trimFields(in, tf) ); };
  const auto ds = But::makeSharedNN<LogDataSource>( std::move(node), std::move(trimmedLog2str) );
  const auto sp = Curses::ScreenPosition{Curses::Row{0}, Curses::Column{0}};
  const auto ss = Curses::ScreenSize::global();
  auto caps = Curses::Window::Captions{n->type().value_ + "::" + n->name().value_, "press 'q' to quit"};
  auto status = [ds, thStats = workerThreadsStats_, errCnt = inputErrors_](const size_t pos) {
    std::stringstream ss;
    ss << thStats() << " ";
    ss << detail::nOFmWithPercent(pos, ds->size()) << " ";
    ss << "E:" << errCnt();
    return ss.str();
  };
  return But::makeSharedNN<Curses::ScrolableWindow>(ds, sp, ss, std::move(caps), std::move(status));
}

}
