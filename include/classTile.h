#pragma once
#include <lvgl.h>
#include <globalDefines.h>

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
  lv_obj_t *_btnUp = NULL;
  lv_obj_t *_btnDown = NULL;
  lv_obj_t *_levelLabel = NULL;

  int _screenIdx = 0;
  int _tileIdx = 0;
  int _type = 0;
  int _linkedScreen = 0;
  bool _state = false;
  int _level = 0;
  const void *_img = NULL;

  void _button(lv_obj_t *parent, const void *img);

public:
  tileId_t tileId;
  lv_obj_t *btn = NULL;

  classTile(void){};
  classTile(lv_obj_t *parent, const void *img);
  classTile(lv_obj_t *parent, const void *img, const char *labelText);
  ~classTile();

  void begin(lv_obj_t *parent, const void *img, const char *labelText);
  void registerTile(int screenIdx, int tileIdx, int type);
  void setLabel(const char *labelText);
  void setSubLabel(const char *subLabelText);
  void setState(bool state);
  void setColor(lv_color_t color);
  void setColor(int red, int green, int blue);
  void setColorToDefault(void);
  void setNumber(const char *number, const char *units);
  void setLink(int linkedScreen);
  
  int getLink(void);
  tileId_t getId(void);
  int getType(void);
  bool getState(void);
  char *getLabel(void);
  
  void addEventHandler(lv_event_cb_t callBack);

  void setLevel(int level);
  int getLevel(void);
  void addLevelControl(lv_event_cb_t downButtonCallBack, lv_event_cb_t upButtonCallBack);
};