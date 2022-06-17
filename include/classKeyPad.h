#pragma once
#include <lvgl.h>
#include "classTile.h"
#include "classScreen.h"

class classKeyPad

{
private:
  classTile *_callingTile = NULL;
  lv_obj_t *_parent = NULL;
  lv_obj_t *_ovlPanel2 = NULL;
  lv_obj_t *_btnm1 = NULL;
  lv_obj_t *_imgLockState = NULL;
  lv_obj_t *pwd_ta = NULL;
  const void *_imgLocked;
  const void *_imgUnLocked;

  void _createKeyPad(void);

public:
  lv_obj_t *ovlPanel = NULL;

  classKeyPad(void){};
  classKeyPad(classTile *tile, lv_event_cb_t keyPadEventHandler);
  void setLockState(bool state);  
  void close(void);
  void addChar(char value);
  void delChar(void);
  void setFailed(void);

  const char *getKey(void);
  classTile *getTile(void);
};
