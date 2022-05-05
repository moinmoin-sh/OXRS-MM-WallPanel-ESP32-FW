#pragma once
#include <lvgl.h>

class classScreen
{
private:
  lv_obj_t *_parent = NULL;
  lv_obj_t *_label = NULL;
  lv_obj_t *_labelWarning = NULL;
  lv_obj_t *_btnHome = NULL;
  lv_obj_t *_btnSettings = NULL;
  lv_obj_t *_btnFooter = NULL;
  int _screenNumber;

public:
  lv_obj_t *screen = NULL;
  lv_obj_t *container = NULL;

  classScreen(void){};
  classScreen(int screenNumber, int style);
  int getScreenNumber(void);
  void setLabel(const char *labelText);
  void updateBgColor(void);
  void createHomeButton(lv_event_cb_t callBack, const void *img);
  void createSettingsButton(lv_event_cb_t callBack, const void *img);
  void showConnectionStatus(bool connected);
  void adWipeEventHandler(lv_event_cb_t callBack);
  void adScreenEventHandler(lv_event_cb_t callBack);
};
