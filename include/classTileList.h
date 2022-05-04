#pragma once
#include <lvgl.h>
#include <classTile.h>

class classTileList
{
private:
public:
  classTileList(void){};
  classTile &add(void);
  classTile *get(int screen, int tile);
  void remove(int screen, int tile);
  int size(void);
};