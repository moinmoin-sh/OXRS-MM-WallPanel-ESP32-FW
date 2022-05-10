#include <classLevel.h>
#include <globalDefines.h>

extern lv_color_t colorOn;
extern lv_color_t colorBg;
extern const void *imgBack;
extern const void *imgUp;
extern const void *imgDown;

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 480

classLevel::classLevel(classTile *tile)
{
  _callingTile = tile;

  // full screen overlay / semi transparent
  _ovlPanel = lv_obj_create(lv_scr_act());
  lv_obj_remove_style_all(_ovlPanel);
  lv_obj_set_size(_ovlPanel, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_set_align(_ovlPanel, LV_ALIGN_TOP_MID);
  lv_obj_clear_flag(_ovlPanel, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(_ovlPanel, colorBg, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(_ovlPanel, 150, LV_PART_MAIN | LV_STATE_DEFAULT);

  // active upper part / colorBg
  _ovlPanel2 = lv_obj_create(_ovlPanel);
  lv_obj_remove_style_all(_ovlPanel2);
  lv_obj_set_size(_ovlPanel2, SCREEN_WIDTH, SCREEN_HEIGHT - 35);
  lv_obj_set_align(_ovlPanel2, LV_ALIGN_TOP_MID);
  lv_obj_clear_flag(_ovlPanel2, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(_ovlPanel2, colorBg, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(_ovlPanel2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  // back button closes overlay
  _btnBack = lv_btn_create(_ovlPanel2);
  lv_obj_set_size(_btnBack, 80, 50);
  lv_obj_align(_btnBack, LV_ALIGN_BOTTOM_LEFT, 5, -5);
  lv_obj_set_style_bg_color(_btnBack, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(_btnBack, WP_OPA_BG_OFF, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *label = lv_label_create(_btnBack);
  lv_label_set_text(label, LV_SYMBOL_LEFT);
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

  lv_obj_add_event_cb(_btnBack, closeEvt, LV_EVENT_CLICKED, this);

  // panel for active controls
  lv_obj_t *_panel = lv_obj_create(_ovlPanel2);
  lv_obj_remove_style_all(_panel);

  lv_obj_set_size(_panel, 310, 120);
  lv_obj_align(_panel, LV_ALIGN_CENTER, 0, 0);
  lv_obj_clear_flag(_panel, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_set_style_bg_color(_panel, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(_panel, WP_OPA_BG_OFF, LV_PART_MAIN | LV_STATE_DEFAULT);

  // copied label from initiator tile
  _label = lv_label_create(_ovlPanel2);
  lv_label_set_text(_label, tile->getLabel());
  lv_obj_align_to(_label, _panel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

  // up / down  buttons
  _btnUp = lv_btn_create(_panel);
  lv_obj_set_size(_btnUp, 120, 50);
  lv_obj_align(_btnUp, LV_ALIGN_TOP_LEFT, 5, 5);
  lv_obj_set_style_bg_color(_btnUp, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_img_src(_btnUp, imgUp, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(_btnUp, WP_OPA_BG_OFF, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(_btnUp, WP_OPA_BG_PRESSED, LV_PART_MAIN | LV_STATE_PRESSED);

  _btnDown = lv_btn_create(_panel);
  lv_obj_set_size(_btnDown, 120, 50);
  lv_obj_align(_btnDown, LV_ALIGN_TOP_LEFT, 5, 65);
  lv_obj_set_style_bg_color(_btnDown, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_img_src(_btnDown, imgDown, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(_btnDown, WP_OPA_BG_OFF, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(_btnDown, WP_OPA_BG_PRESSED, LV_PART_MAIN | LV_STATE_PRESSED);

  // _bar
  _bar = lv_bar_create(_panel);
  lv_bar_set_range(_bar, 0, 100);
  lv_bar_set_value(_bar, 50, LV_ANIM_OFF);

  lv_obj_set_size(_bar, 20, 110);
  lv_obj_align(_bar, LV_ALIGN_TOP_LEFT, 250, 5);

  lv_obj_set_style_radius(_bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(_bar, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(_bar, 60, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_set_style_radius(_bar, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(_bar, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(_bar, 150, LV_PART_INDICATOR | LV_STATE_DEFAULT);
  lv_obj_set_style_opa(_bar, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);

  // _barLabel
  _barLabel = lv_label_create(_panel);
  lv_obj_set_size(_barLabel, 50, LV_SIZE_CONTENT);
  lv_obj_set_style_text_align(_barLabel, LV_TEXT_ALIGN_RIGHT, 0);
  lv_obj_align(_barLabel, LV_ALIGN_TOP_LEFT, 190, 100);

  // change button hold repeat timing
  lv_indev_t * indev = lv_indev_get_act();
  indev->driver->long_press_repeat_time = 100;

  // initialise display
  setValue(0);
}

void classLevel::closeEvt(lv_event_t *e)
{
  lv_obj_t *btn = lv_event_get_target(e);
  lv_obj_t *panel2 = lv_obj_get_parent(btn);
  lv_obj_t *ovlPanel = lv_obj_get_parent(panel2);
  lv_obj_del(ovlPanel);

  // restore button hold repeat timing
  lv_indev_t *indev = lv_indev_get_act();
  indev->driver->long_press_repeat_time = 500;
}

lv_obj_t *classLevel::getBtnUp(void)
{
  return _btnUp;
}

classTile *classLevel::getTile(void)
{
  return _callingTile;
}

void classLevel::adUpDownEventHandler(lv_event_cb_t callBack)
{
  lv_obj_add_event_cb(_btnUp, callBack, LV_EVENT_ALL, this);
  lv_obj_add_event_cb(_btnDown, callBack, LV_EVENT_ALL, this);
}

void classLevel::setValue(int value)
{
  // exit if overlay not valid
  if (!lv_obj_is_valid(_ovlPanel)) return;

  if (value > 100)
    value = 100;
  if (value < 0)
    value = 0;

  lv_bar_set_value(_bar, value, LV_ANIM_OFF);
  lv_obj_set_y(_barLabel, 100 - (95 * value / 100));
  lv_obj_set_style_text_align(_barLabel, LV_TEXT_ALIGN_RIGHT, 0);
  lv_label_set_text_fmt(_barLabel, "%d %%", value);
}

int classLevel::getValue(void)
{
  return (lv_bar_get_value(_bar));
}
