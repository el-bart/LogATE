#include "CursATE/Curses/Init.hpp"
#include "CursATE/Curses/ScrolableWindow.hpp"
#include "CursATE/Curses/CursorVisibility.hpp"
#include "CursATE/Curses/detail/StringDataSource.ut.hpp"
#include "CursATE/Curses/ctrl.hpp"
#include <sstream>

using namespace CursATE::Curses;


struct Location
{
  DataSourceShNN ds_;
  size_t n_{0};

  std::string operator()(size_t pos)
  {
    std::stringstream ss;
    ss << pos << "/" << ds_->size() << " (iteration: " << n_ << ")";
    for(auto i=0u; i<n_; ++i)
      ss << "0123456789"[i%10];
    ++n_;
    return ss.str();
  }
};


int main()
{
  const Init init;
  set(CursorVisibility::Invisible);
  auto dataSource = But::makeSharedNN<detail::StringDataSource>();
  ScrolableWindow win{ dataSource,
                       ScreenPosition{Row{2}, Column{10}},
                       ScreenSize{Rows{12}, Columns{60}},
                       Window::Boxed::True,
                       Location{dataSource} };

  if(true) // preinit
    for(auto i=0; i<5; ++i)
      dataSource->addNewest( "preinit #" + std::to_string(i) );

  auto lastKey = std::string{"no key pressed yet"};
  auto quit = false;
  for(auto i=0; not quit; ++i)
  {
    mvprintw(0,0, "iteration %d (last pressed key: %s)", i, lastKey.c_str());
    clrtoeol();
    refresh();
    win.refresh();
    const auto ch = getch();
    switch(ch)
    {
      case 'q': quit = true; break;

      case 'a': lastKey = "a"; dataSource->addNewest( "data from iteration " + std::to_string(i) ); break;
      case 'd': lastKey = "d"; dataSource->removeOldest(); break;

      case KEY_UP:    lastKey = "up";    win.selectUp(); break;
      case KEY_DOWN:  lastKey = "down";  win.selectDown(); break;
      case KEY_LEFT:  lastKey = "left";  win.scrollLeft(); break;
      case KEY_RIGHT: lastKey = "right"; win.scrollRight(); break;

      case KEY_PPAGE: lastKey = "pg-up";   win.selectPageUp(); break;
      case KEY_NPAGE: lastKey = "pg-down"; win.selectPageDown(); break;

      case ctrl(KEY_HOME): lastKey = "^home"; win.selectFirst(); break;
      case ctrl(KEY_END):  lastKey = "^end";  win.selectLast(); break;
      case KEY_HOME: lastKey = "home"; win.scrollToLineBegin(); break;
      case KEY_END:  lastKey = "end";  win.scrollToLineEnd(); break;

      default: lastKey = "unknown key"; break;
    }
    lastKey += ": " + std::to_string(ch) + " / " + keyname(ch);
  }
}
