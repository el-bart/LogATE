#include "LogATE/Log.hpp"
#include "CursATE/Curses/ScrolableWindow.hpp"
#include <But/Optional.hpp>

namespace CursATE::Screen::detail
{

class Marks final
{
public:
  struct Entry
  {
    Entry(LogATE::Log::Key key, std::weak_ptr<Curses::ScrolableWindow> window):
      key_{ std::move(key) },
      window_{ std::move(window) }
    { }
    Entry(LogATE::Log::Key key, But::NotNullShared<Curses::ScrolableWindow> const& window):
      Entry{ std::move(key), window.underlyingPointer() }
    { }

    LogATE::Log::Key key_;
    std::weak_ptr<Curses::ScrolableWindow> window_;
  };

  But::Optional<Entry> find(char k) const;
  void prune();
  void insert(char c, Entry e);
  auto size() const { return data_.size(); }

private:
  std::map<char, Entry> data_;
};

}
