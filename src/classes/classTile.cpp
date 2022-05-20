﻿#include <classTile.h>
#include <globalDefines.h>

extern lv_color_t colorOn;
extern lv_color_t colorBg;
extern "C" const lv_font_t number_OR_50;
extern const void *imgUp;
extern const void *imgDown;

// create the tile
void classTile::_button(lv_obj_t *parent, const void *img)
{
  _parent = parent;
  _img = img;

  // image button
  _btn = lv_imgbtn_create(_parent);

  lv_imgbtn_set_src(_btn, LV_IMGBTN_STATE_RELEASED, img, NULL, NULL);
  lv_obj_set_style_bg_opa(_btn, WP_OPA_BG_OFF, LV_PART_MAIN | LV_IMGBTN_STATE_RELEASED);
  lv_obj_set_style_img_recolor(_btn, lv_color_hex(0xffffff), LV_PART_MAIN | LV_IMGBTN_STATE_RELEASED);
  lv_obj_set_style_img_recolor_opa(_btn, 255, LV_PART_MAIN | LV_IMGBTN_STATE_RELEASED);

  lv_imgbtn_set_src(_btn, LV_IMGBTN_STATE_CHECKED_RELEASED, img, NULL, NULL);
  lv_obj_set_style_bg_opa(_btn, WP_OPA_BG_ON, LV_STATE_CHECKED);
  lv_obj_set_style_img_recolor(_btn, colorOn, LV_STATE_CHECKED);
  lv_obj_set_style_img_recolor_opa(_btn, 255, LV_STATE_CHECKED);

  lv_imgbtn_set_src(_btn, LV_IMGBTN_STATE_CHECKED_PRESSED, img, NULL, NULL);

  lv_obj_clear_flag(_btn, LV_OBJ_FLAG_PRESS_LOCK);

  // main Label (placeholder)
  _label = lv_label_create(_btn);
  lv_obj_set_size(_label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_label_set_text(_label, "Main Label");
  lv_obj_align(_label, LV_ALIGN_BOTTOM_LEFT, 8, -22);
  lv_obj_set_style_text_color(_btn, lv_color_hex(0x000000), LV_STATE_CHECKED);

  // additional Label (show sybol ">" if button liks to new screen)
  _linkedLabel = lv_label_create(_btn);
  lv_obj_set_size(_linkedLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_label_set_text(_linkedLabel, "");
  lv_obj_align(_linkedLabel, LV_ALIGN_TOP_RIGHT, -8, 5);
  lv_obj_set_style_text_color(_btn, lv_color_hex(0x000000), LV_STATE_CHECKED);

  // Sub Label (placeholder)
  _subLabel = lv_label_create(_btn);
  lv_obj_set_size(_label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_label_set_text(_subLabel, "");
  lv_obj_align(_subLabel, LV_ALIGN_BOTTOM_LEFT, 8, -5);
  lv_obj_set_style_text_color(_subLabel, lv_color_hex(0x808080), 0);

  // additional Label (placeholder)
  _numLabel = lv_label_create(_btn);
  lv_obj_set_size(_numLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_label_set_text(_numLabel, "");
  lv_obj_align(_numLabel, LV_ALIGN_TOP_LEFT, 8, 15);
  lv_obj_set_style_text_color(_numLabel, lv_color_hex(0x000000), LV_STATE_CHECKED);

  // additional Label (unit display)
  _unitLabel = lv_label_create(_btn);
  lv_obj_set_size(_unitLabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_label_set_text(_unitLabel, "");
  lv_obj_set_style_text_color(_unitLabel, lv_color_hex(0x000000), LV_STATE_CHECKED);

  // set button and label size from grid
  int width = *lv_obj_get_style_grid_column_dsc_array(parent, 0) - 10;
  int height = *lv_obj_get_style_grid_row_dsc_array(parent, 0) - 10;

  lv_obj_set_width(_btn, width);
  lv_obj_set_height(_btn, height);

  lv_obj_set_size(_label, width - 10, LV_SIZE_CONTENT);

  lv_obj_set_size(_subLabel, width - 10, LV_SIZE_CONTENT);

  btn = _btn;
}

classTile::classTile(lv_obj_t *parent, const void *img)
{
  _button(parent, img);
}

classTile::classTile(lv_obj_t *parent, const void *img, const char *labelText)
{
  _button(parent, img);
  lv_label_set_text(_label, labelText);
}

classTile::~classTile()
{
  if (_btn)
  {
    lv_obj_del(_btn);
  }
}

// initialise existing object
void classTile::begin(lv_obj_t *parent, const void *img, const char *labelText)
{
  _button(parent, img);
  lv_label_set_text(_label, labelText);
}

// supply bookkeeping information and align tile in grid
void classTile::registerTile(int screenIdx, int tileIdx, int type)
{
  tileId.idx.screen = screenIdx;
  tileId.idx.tile = tileIdx;
  _type = type;

  // position tile in grid after tile and screen are known
  int row = (tileIdx - 1) / 2;
  int col = (tileIdx - 1) % 2;
  lv_obj_set_grid_cell(_btn, LV_GRID_ALIGN_CENTER, col, 1, LV_GRID_ALIGN_CENTER, row, 1);
}

void classTile::setLabel(const char *labelText)
{
  lv_label_set_text(_label, labelText);
}

void classTile::setSubLabel(const char *subLabelText)
{
  lv_label_set_text(_subLabel, subLabelText);
}

void classTile::setState(bool state)
{
  _state = state;
  state == false ? lv_obj_clear_state(_btn, LV_STATE_CHECKED) : lv_obj_add_state(_btn, LV_STATE_CHECKED);
  if (_btnUp)
    state == false ? lv_obj_clear_state(_btnUp, LV_STATE_CHECKED) : lv_obj_add_state(_btnUp, LV_STATE_CHECKED);
  if (_btnDown)
    state == false ? lv_obj_clear_state(_btnDown, LV_STATE_CHECKED) : lv_obj_add_state(_btnDown, LV_STATE_CHECKED);
}

void classTile::setColor(lv_color_t color)
{
  if (lv_obj_get_state(_btn) & LV_STATE_CHECKED)
  {
    lv_obj_set_style_img_recolor(_btn, color, LV_STATE_CHECKED);
    lv_obj_set_style_text_color(_numLabel, color, LV_STATE_CHECKED);
    lv_obj_set_style_text_color(_unitLabel, color, LV_STATE_CHECKED);
  }
  else
  {
    lv_obj_set_style_img_recolor(_btn, color, LV_IMGBTN_STATE_RELEASED);
    lv_obj_set_style_text_color(_numLabel, color, LV_IMGBTN_STATE_RELEASED);
    lv_obj_set_style_text_color(_unitLabel, color, LV_IMGBTN_STATE_RELEASED);
  }
}

void classTile::setColor(int red, int green, int blue)
{
  setColor(lv_color_make(red, green, blue));
}

void classTile::setColorToDefault(void)
{
  if (lv_obj_get_state(_btn) & LV_STATE_CHECKED)
  {
    lv_obj_set_style_img_recolor(_btn, colorOn, LV_STATE_CHECKED);
    lv_obj_set_style_text_color(_numLabel, colorOn, LV_STATE_CHECKED);
    lv_obj_set_style_text_color(_unitLabel, colorOn, LV_STATE_CHECKED);
  }
  else
  {
    lv_obj_set_style_img_recolor(_btn, lv_color_hex(0xffffff), LV_IMGBTN_STATE_RELEASED);
    lv_obj_set_style_text_color(_numLabel, lv_color_hex(0xffffff), LV_IMGBTN_STATE_RELEASED);
    lv_obj_set_style_text_color(_unitLabel, lv_color_hex(0xffffff), LV_IMGBTN_STATE_RELEASED);
  }
}

void classTile::setNumber(const char *number, const char *units)
{
  lv_obj_set_style_text_font(_numLabel, &number_OR_50, 0);
  lv_label_set_text(_numLabel, number);
  lv_obj_set_style_text_font(_unitLabel, &lv_font_montserrat_20, 0);
  lv_label_set_text(_unitLabel, units);
  lv_obj_align_to(_unitLabel, _numLabel, LV_ALIGN_OUT_RIGHT_BOTTOM, 5, 5);
}

// this button calls a new screen (linkScreen)
void classTile::setLink(int linkScreen)
{
  _linkedScreen = linkScreen;
  lv_label_set_text(_linkedLabel, LV_SYMBOL_RIGHT);
}

int classTile::getLink(void)
{
  return _linkedScreen;
}

tileId_t classTile::getId(void)
{
  return tileId;
}

int classTile::getType(void)
{
  return _type;
}

bool classTile::getState(void)
{
  return _state;
}

char *classTile::getLabel(void)
{
  return lv_label_get_text(_label);
}

void classTile::addEventHandler(lv_event_cb_t callBack)
{
  // add click detecttion
  lv_imgbtn_set_src(_btn, LV_IMGBTN_STATE_PRESSED, _img, NULL, NULL);
  lv_obj_set_style_bg_opa(_btn, WP_OPA_BG_PRESSED, LV_STATE_PRESSED);
  lv_obj_set_style_img_recolor(_btn, lv_color_hex(0x404040), LV_STATE_PRESSED);
  lv_obj_set_style_img_recolor_opa(_btn, 255, LV_STATE_PRESSED);
  // add event handler
  lv_obj_add_event_cb(_btn, callBack, LV_EVENT_ALL, this);
}

// additional methods for on-tile level control

void classTile::setLevel(int level, bool force)
{
  // early exit if bar visualisation is activ and not forced
  if (lv_obj_is_valid(_ovlPanel) && !force)
    return;

  _level = level;
}

int classTile::getLevel(void)
{
  return _level;
}

void classTile::addLevelControl(lv_event_cb_t downButtonCallBack, lv_event_cb_t upButtonCallBack)
{
  // set button and label size from grid
  int width = (*lv_obj_get_style_grid_column_dsc_array(_parent, 0) - 10) / 2 + 1;
  int height = (*lv_obj_get_style_grid_row_dsc_array(_parent, 0) - 10) / 2 + 1;
 
  // up / down  buttons
  _btnUp = lv_btn_create(_btn);
  lv_obj_set_size(_btnUp, width, height);
  lv_obj_align(_btnUp, LV_ALIGN_TOP_RIGHT, 0, 0);
  lv_obj_set_style_bg_img_src(_btnUp, imgUp, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(_btnUp, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(_btnUp, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(_btnUp, 0, LV_PART_MAIN | LV_STATE_CHECKED);
  lv_obj_set_style_bg_img_recolor(_btnUp, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_CHECKED);
  lv_obj_set_style_bg_img_recolor_opa(_btnUp, 255, LV_PART_MAIN | LV_STATE_CHECKED);
  lv_obj_set_style_bg_img_recolor_opa(_btnUp, 100, LV_PART_MAIN | LV_STATE_PRESSED);

  _btnDown = lv_btn_create(_btn);
  lv_obj_set_size(_btnDown, width, height);
  lv_obj_align(_btnDown, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
  lv_obj_set_style_bg_img_src(_btnDown, imgDown, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(_btnDown, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(_btnDown, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(_btnDown, 0, LV_PART_MAIN | LV_STATE_CHECKED);
  lv_obj_set_style_bg_img_recolor(_btnDown, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_CHECKED);
  lv_obj_set_style_bg_img_recolor_opa(_btnDown, 255, LV_PART_MAIN | LV_STATE_CHECKED);
  lv_obj_set_style_bg_img_recolor_opa(_btnDown, 100, LV_PART_MAIN | LV_STATE_PRESSED);

  // add event handler
  lv_obj_add_event_cb(_btnUp, upButtonCallBack, LV_EVENT_ALL, this);
  lv_obj_add_event_cb(_btnDown, downButtonCallBack, LV_EVENT_ALL, this);

  // reduced width for main label
  lv_obj_set_size(_label, 80, LV_SIZE_CONTENT);
}

void classTile::showOvlBar(int level)
{
  if (lv_obj_is_valid(_ovlPanel))
  {
    lv_obj_del(_ovlPanel);
  }

  _ovlPanel = lv_obj_create(_btn);
  lv_obj_clear_flag(_ovlPanel, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_border_width(_ovlPanel, 0, LV_PART_MAIN);
  lv_obj_set_size(_ovlPanel, 60, 70);
  lv_obj_align(_ovlPanel, LV_ALIGN_TOP_LEFT, 2, 2);
  lv_obj_set_style_bg_color(_ovlPanel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(_ovlPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(_ovlPanel, 255, LV_PART_MAIN | LV_STATE_CHECKED);

  _bar = lv_bar_create(_ovlPanel);
  if (lv_obj_get_state(_btn) & LV_STATE_CHECKED)
    lv_obj_add_state(_bar, LV_STATE_CHECKED);
  lv_bar_set_range(_bar, 0, 100);
  lv_bar_set_value(_bar, level, LV_ANIM_OFF);
  lv_obj_set_size(_bar, 10, 60);
  lv_obj_align(_bar, LV_ALIGN_CENTER, 10, 0);

  lv_obj_set_style_radius(_bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(_bar, colorBg, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(_bar, 60, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_set_style_radius(_bar, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(_bar, colorBg, LV_PART_INDICATOR | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(_bar, 150, LV_PART_INDICATOR | LV_STATE_DEFAULT);
  lv_obj_set_style_opa(_bar, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);

  lv_obj_set_style_bg_color(_bar, colorOn, LV_PART_INDICATOR | LV_STATE_CHECKED);
  lv_obj_set_style_bg_opa(_bar, 255, LV_PART_INDICATOR | LV_STATE_CHECKED);

  // _barLabel
  lv_obj_t *_barLabel = lv_label_create(_ovlPanel);
  lv_obj_set_size(_barLabel, 40, LV_SIZE_CONTENT);
  lv_obj_align(_barLabel, LV_ALIGN_TOP_MID, -20, 000);
  lv_obj_set_y(_barLabel, 38 - (50 * 0 / 100));
  lv_obj_set_style_text_align(_barLabel, LV_TEXT_ALIGN_RIGHT, 0);
  lv_obj_set_style_text_color(_barLabel, lv_color_hex(0x000000), 0);
  lv_label_set_text_fmt(_barLabel, "%d", level);

  lv_obj_del_delayed(_ovlPanel, 2000);
}
