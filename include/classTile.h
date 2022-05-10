#pragma once
#include <lvgl.h>

class classTile
{
protected:
  lv_obj_t *_parent = NULL;
  lv_obj_t *_btn = NULL;
  lv_obj_t *_label = NULL;
  lv_obj_t *_subLabel = NULL;
  lv_obj_t *_linkedLabel = NULL;
  lv_obj_t *_unitLabel = NULL;
  lv_obj_t *_numLabel = NULL;
  int _screen = 0;
  int _tile = 0;
  int _type = 0;
  int _linkedScreen = 0;
  bool _state = false;
  const void *_img = NULL;

  void _button(lv_obj_t *parent, const void *img);

public:
  lv_obj_t *btn = NULL;
  int id = 0;

  classTile(void){};
  classTile(lv_obj_t *parent, const void *img);
  classTile(lv_obj_t *parent, const void *img, const char *labelText);
  ~classTile();

  void begin(lv_obj_t *parent, const void *img, const char *labelText);
  void setLabel(const char *labelText);
  void setSubLabel(const char *subLabelText);
  void setState(bool state);
  void setColor(lv_color_t color);
  void setColor(int red, int green, int blue);
  void setColorToDefault(void);
  void setNumber(const char *number, const char *units);
  void registerTile(int screen, int tile, int type);
  void setLink(int linkedScreen);
  int getLink(void);
  int getId(void);
  int getType(void);
  bool getState(void);
  char *getLabel(void);
  void addEventHandler(lv_event_cb_t callBack);
};
