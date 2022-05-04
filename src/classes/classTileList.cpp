#include <classTileList.h>
#include <list>
#include <algorithm>

using std::list;

// THE list that holds all tiles
std::list<classTile> _listTiles;

/*
 * tileList handler
 */
// search tile[screen][tile]

classTile &classTileList::add(void)
{
  _listTiles.emplace_back(classTile());
  return _listTiles.back();
}

classTile *classTileList::get(int screen, int tile)
{
  // id is generated from screen * 100 + tile
  int searchId = screen * 100 + tile;
  std::list<classTile>::iterator it;
  it = std::find_if(std::begin(_listTiles), std::end(_listTiles),
                    [&](classTile const &p)
                    { return p.id == searchId; });
  if (it == _listTiles.end())
  {
    return NULL;
  }
  else
  {
    return it.operator->();
  }
}

// remove tile[screen][tile] from list (if exist)
void classTileList::remove(int screen, int tile)
{
  // id is generated from screen * 100 + tile
  int searchId = screen * 100 + tile;

  _listTiles.remove_if([&](classTile const &p)
                       { return p.id == searchId; });
}

// return size from list
int classTileList::size(void)
{
  return _listTiles.size();
}
