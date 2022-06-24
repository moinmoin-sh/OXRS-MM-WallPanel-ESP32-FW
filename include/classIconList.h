#pragma once
#include <lvgl.h>
#include "globalDefines.h"

class classIconList
{
private:
    // THE list that holds all icons
    std::list<imgListElement_t> _listIcons;
    std::list<imgListElement_t>::iterator _iterator;

public:
  classIconList(void){};
  void add(imgListElement_t element);
  void remove(const char* iconStr);
  void sort(void);
  const void* getIcon(const char* iconStr);
  void setIteratorStart(void);
  const char* getNextStr(void);
  int size(void);
};
