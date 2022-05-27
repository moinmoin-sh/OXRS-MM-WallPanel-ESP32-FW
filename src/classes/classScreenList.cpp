#include "classScreenList.h"
#include <list>
#include <algorithm>

using std::list;

// THE list that holds all screens
std::list<classScreen> _listScreens;

classScreen &classScreenList::add(int screenIdx, int style)
{
  _listScreens.emplace_back(classScreen(screenIdx, style));
  return _listScreens.back();
}

classScreen *classScreenList::get(int screenIdx)
{
  std::list<classScreen>::iterator it;
  it = std::find_if(std::begin(_listScreens), std::end(_listScreens),
                    [&](classScreen const &p)
                    { return p.screenIdx == screenIdx; });
  if (it == _listScreens.end())
  {
    return NULL;
  }
  else
  {
    return it.operator->();
  }
}

classScreen *classScreenList::getStart(void)
{
  std::list<classScreen>::iterator it = _listScreens.begin();
  if (it == _listScreens.end())
  {
    return NULL;
  }
  else
  {
    return it.operator->();
  }
}

classScreen *classScreenList::getNext(int screenIdx)
{
  std::list<classScreen>::iterator it;
  it = std::find_if(std::begin(_listScreens), std::end(_listScreens),
                    [&](classScreen const &p)
                    { return p.screenIdx == screenIdx; });
  if (it == _listScreens.end()) return NULL;
  it++;
  if (it == _listScreens.end())
  {
    return NULL;
  }
  else
  {
    return it.operator->();
  }
}

bool classScreenList::exist(int screenIdx)
{
  std::list<classScreen>::iterator it;
  it = std::find_if(std::begin(_listScreens), std::end(_listScreens),
                    [&](classScreen const &p)
                    { return p.screenIdx == screenIdx; });
  if (it == _listScreens.end())
  {
      return false;
  }
  else
  {
      return true;
  }
}
// remove tile[screen][tile] from list (if exist)
void classScreenList::remove(int screenIdx)
{
  _listScreens.remove_if([&](classScreen const &p)
                         { return p.screenIdx == screenIdx; });
}


// sort screen list in ascending order
void classScreenList::sort(void)
{
//    bool cmp(const Type1 & a, const Type2 & b);
//    _listScreens.sort( compareScreens);
_listScreens.sort(
    [&](classScreen const &p1, classScreen const &p2)
    { return p1.screenIdx < p2.screenIdx; });
}

// show selected screen if available
bool classScreenList::show(int screenIdx)
{
  if (classScreen *sPtr = get(screenIdx))
  {
    lv_disp_load_scr(sPtr->screen);
    return true;
  }
  else
  {
      return false;
  }
}

// show previous screen in list 
void classScreenList::showPrev(lv_obj_t* screenObj)
{
  std::list<classScreen>::iterator it;
  it = std::find_if(std::begin(_listScreens), std::end(_listScreens),
      [&](classScreen const& p)
      { return p.screen == screenObj; });
  if (it != _listScreens.begin())
      it--;
  show(it->screenIdx);
}

// show next screen in list
void classScreenList::showNext(lv_obj_t* screenObj)
{
  std::list<classScreen>::iterator it;
  it = std::find_if(std::begin(_listScreens), std::end(_listScreens),
      [&](classScreen const& p)
      { return p.screen == screenObj; });
  if (it != _listScreens.end())
      it++;
  show(it->screenIdx);
}

void classScreenList::showByIndex(int index)
{
  std::list<classScreen>::iterator it;
  it = std::begin(_listScreens);
  while (index--)
  {
    it++;
  }
  lv_disp_load_scr(it->screen);
}

// return size from list
int classScreenList::getSize(void)
{
  return _listScreens.size();
}

// make list to show available screens in drop down
// return drop down list index of current screen
int classScreenList::makeDropDownList(char *list, lv_obj_t *screenObj)
{
  int i = 0;
  int retVal = 0;
  list[0] = 0;
  std::list<classScreen>::iterator it;
  it = std::begin(_listScreens);
  strcat(list, it->getLabel());
  while (++it != _listScreens.end())
  {
    strcat(list, "\n");
    strcat(list, it->getLabel());
    i++;
    if (it->screen == screenObj) retVal = i;
  }

  return retVal;
}
