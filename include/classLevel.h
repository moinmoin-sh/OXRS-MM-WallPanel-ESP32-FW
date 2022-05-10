#pragma once
#include <lvgl.h>
#include <classTile.h>

class classLevel
{
private:
  classTile *_callingTile = NULL;
  lv_obj_t *_ovlPanel = NULL;
  lv_obj_t *_ovlPanel2 = NULL;
  lv_obj_t *_btnBack = NULL;
  lv_obj_t *_btnUp = NULL;
  lv_obj_t *_btnDown = NULL;
  lv_obj_t *_label = NULL;
  lv_obj_t *_bar = NULL;
  lv_obj_t *_barLabel = NULL;

public:
  classLevel(void){};
  classLevel(classTile *tile);
  void close(void);
  lv_obj_t *getBtnUp(void);
  void setValue(int value);
  int getValue(void);
  classTile *getTile(void);
  static void closeEvt(lv_event_t *e);
  void adBackEventHandler(lv_event_cb_t callBack);
  void adUpDownEventHandler(lv_event_cb_t callBack);
};
