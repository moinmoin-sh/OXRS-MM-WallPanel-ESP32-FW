#include <classTile.h>
#include <globalDefines.h>

extern lv_color_t colorOn;
extern lv_color_t colorBg;
extern "C" const lv_font_t number_OR_50;

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
