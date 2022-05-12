#pragma once
#include <lvgl.h>
#include <classScreen.h>

class classScreenList
{
private:
 
public:
  classScreenList(void){};
  classScreen &add(int screenIdx, int style);
  classScreen *get(int screenIdx);
  classScreen *getStart(void);
  classScreen *getNext(int screenIdx);
  void remove(int screenIdx);
  bool exist(int screenIdx);
  void sort(void);
  bool show(int screenIdx);
  void showPrev(lv_obj_t* screenObj);
  void showNext(lv_obj_t* screenObj);
  int getSize(void);
};
