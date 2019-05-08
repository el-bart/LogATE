#include "CursATE/Screen/Search.hpp"
#include "CursATE/Curses/Form.hpp"

namespace CursATE::Screen
{

But::Optional<LogATE::SequenceNumber> Search::process(LogATE::Tree::NodeShPtr node, const LogATE::SequenceNumber currentSelection)
{
  updateSearchPattern();
  (void)currentSelection;
  (void)node;
  return {};
}


bool Search::updateSearchPattern()
{
  auto form = Curses::makeForm(
        Curses::KeyShortcuts{
          {'o', "search"},
          {'s', "search"},
          {'c', "cancel"},
          {'q', "cancel"}
        },
        Curses::Field::Input{"key", keyQuery_},
        Curses::Field::Input{"value", valueQuery_},
        Curses::Field::Button{"search"},
        Curses::Field::Button{"cancel"}
      );
  auto ret = form.process();

  if( ret[3] == "true" )            // cancel
    return false;

  BUT_ASSERT( ret[2] == "true" );   // search
  keyQuery_ = std::move(ret[0]);
  valueQuery_ = std::move(ret[1]);
  return true;
}

}
