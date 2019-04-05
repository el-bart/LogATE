#include "CursATE/Curses/Init.hpp"
#include "CursATE/Curses/ScrolableWindow.hpp"
#include "CursATE/Curses/ctrl.hpp"

using namespace CursATE::Curses;

struct StringDataSource: public ScrolableWindow::DataSource
{
  size_t size() const override { return data_.size(); }

  Id first() const override
  {
    if( data_.empty() )
      return {};
    return data_.begin()->first;
  }

  virtual Id last() const override
  {
    if( data_.empty() )
      return {};
    return data_.rbegin()->first;
  }

  std::map<Id, std::string> get(size_t before, Id id, size_t after) const override
  {
    std::map<Id, std::string> out;
    const auto it = data_.find(id);
    if( it == end(data_) )
      return out;

    out[it->first] = it->second;

    {
      auto beforeIt = it;
      for(auto i=0u; i<before; ++i)
      {
        if( beforeIt == begin(data_) )
          break;
        --beforeIt;
        out[beforeIt->first] = beforeIt->second;
      }
    }

    {
      auto afterIt = it;
      for(auto i=0u; i<after; ++i)
      {
        ++afterIt;
        if( afterIt == end(data_) )
          break;
        out[afterIt->first] = afterIt->second;
      }
    }

    return out;
  }

  void addNewest(std::string str)
  {
    Id id{nextFreeId_+=str.size()+1};
    data_[id] = std::move(str);
  }

  void removeOldest()
  {
    if( data_.empty() )
      return;
    data_.erase(data_.begin());
  }

  size_t nextFreeId_{42};
  std::map<Id, std::string> data_;
};


int main()
{
  const Init init;
  auto dataSource = But::makeSharedNN<StringDataSource>();
  ScrolableWindow win{ dataSource, ScreenPosition{Row{2}, Column{10}}, ScreenSize{Rows{12}, Columns{60}}, Window::Boxed::True };

  auto quit = false;
  for(auto i=0; not quit; ++i)
  {
    mvprintw(0,0, "iteration %d", i);
    win.refresh();
    switch( getch() )
    {
      case 'q': quit = true; break;

      case 'a': dataSource->addNewest( "data from iteration " + std::to_string(i) ); break;
      case 'd': dataSource->removeOldest(); break;

      case KEY_UP:    win.scrollUp(); break;
      case KEY_DOWN:  win.scrollDown(); break;
      case KEY_LEFT:  win.scrollLeft(); break;
      case KEY_RIGHT: win.scrollRight(); break;

      case KEY_PPAGE: win.scrollPageUp(); break;
      case KEY_NPAGE: win.scrollPageDown(); break;

      case ctrl(KEY_HOME): win.scrollToListBegin(); break;
      case ctrl(KEY_END):  win.scrollToListEnd(); break;
      case KEY_HOME: win.scrollToLineBegin(); break;
      case KEY_END:  win.scrollToLineEnd(); break;
    }
  }
}
