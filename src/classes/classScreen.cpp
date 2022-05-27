#include <classScreen.h>
#include <classTile.h>
#include <globalDefines.h>

extern lv_color_t colorOn;
extern lv_color_t colorBg;
// extern classScreen *screenDsc[9];

// grid definitions
// sub screens 3 x 2 + home button
#define FOOTER_HEIGHT 25
#define COLS_2X3 2
#define ROWS_2X3 3
#define COL_SIZE_2X3 (SCREEN_WIDTH / COLS_2X3) - 2
#define ROW_SIZE_2X3 ((SCREEN_HEIGHT - FOOTER_HEIGHT) / ROWS_2X3) - 2
static lv_coord_t colDsc_2X3[] = {COL_SIZE_2X3, COL_SIZE_2X3, LV_GRID_TEMPLATE_LAST};
static lv_coord_t rowDsc_2X3[] = {ROW_SIZE_2X3, ROW_SIZE_2X3, ROW_SIZE_2X3, LV_GRID_TEMPLATE_LAST};

classScreen::classScreen(int number, int style)
{
  screenIdx = number;

  screen = lv_obj_create(NULL);
  lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(screen, colorBg, LV_PART_MAIN);

  if (style == 1)
  {
    //*Create a container with grid
    lv_obj_t *cont = lv_obj_create(screen);

    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_align(cont, LV_ALIGN_TOP_MID);
    lv_obj_set_layout(cont, LV_LAYOUT_GRID);
    lv_obj_set_style_pad_top(cont, 0, 0);
    lv_obj_set_style_pad_left(cont, 2, 0);
    lv_obj_set_grid_dsc_array(cont, colDsc_2X3, rowDsc_2X3);

    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_PRESS_LOCK);

    container = cont;
  }
  // placeholder for swipe detection
  _btnFooter = lv_imgbtn_create(screen);
  lv_imgbtn_set_src(_btnFooter, LV_IMGBTN_STATE_RELEASED, NULL, NULL, NULL);
  lv_obj_set_size(_btnFooter, 200, 40);
  lv_obj_align(_btnFooter, LV_ALIGN_BOTTOM_MID, 0, 0);

  _label = lv_label_create(screen);
  lv_obj_align(_label, LV_ALIGN_BOTTOM_MID, 0, -5);
  lv_obj_set_style_text_font(_label, &lv_font_montserrat_20, 0);
  lv_label_set_text_fmt(_label, "Screen %d", screenIdx);

  _labelWarning = lv_label_create(screen);
  lv_obj_align(_labelWarning, LV_ALIGN_BOTTOM_RIGHT, -45, -5);
  lv_label_set_text(_labelWarning, "");
}

int classScreen::getScreenNumber(void)
{
  return screenIdx;
}

void classScreen::setLabel(const char *labelText)
{
  lv_label_set_text(_label, labelText);
}

void classScreen::updateBgColor(void)
{
  lv_obj_set_style_bg_color(screen, colorBg, LV_PART_MAIN);
}

void classScreen::createHomeButton(lv_event_cb_t callBack, const void *img)
{
  _btnHome = lv_btn_create(screen);
  lv_obj_set_size(_btnHome, 45, 40);
  lv_obj_set_style_bg_opa(_btnHome, 0, LV_STATE_DEFAULT);
  lv_obj_align(_btnHome, LV_ALIGN_BOTTOM_LEFT, 0, 0);
  lv_obj_clear_flag(_btnHome, LV_OBJ_FLAG_PRESS_LOCK);
  _btnHomeImg = lv_img_create(screen);
  lv_img_set_src(_btnHomeImg, img);
  lv_obj_align(_btnHomeImg, LV_ALIGN_BOTTOM_LEFT, 15, -6);
  lv_obj_add_event_cb(_btnHome, callBack, LV_EVENT_ALL, this);
}

void classScreen::createSettingsButton(lv_event_cb_t callBack, const void *img)
{
  _btnSettings = lv_btn_create(screen);
  lv_obj_set_size(_btnSettings, 45, 40);
  lv_obj_set_style_bg_opa(_btnSettings, 0, LV_STATE_DEFAULT);
  lv_obj_align(_btnSettings, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
  lv_obj_clear_flag(_btnSettings, LV_OBJ_FLAG_PRESS_LOCK);
  lv_obj_add_event_cb(_btnSettings, callBack, LV_EVENT_ALL, this);
  _btnSettingsImg = lv_img_create(screen);
  lv_img_set_src(_btnSettingsImg, img);
  lv_obj_align(_btnSettingsImg, LV_ALIGN_BOTTOM_RIGHT, -15, -6);
}

void classScreen::showConnectionStatus(bool connected)
{
  if (connected)
  {
    lv_label_set_text(_labelWarning, "");
  }
  else
  {
    lv_obj_set_style_text_font(_labelWarning, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(_labelWarning, lv_color_make(180, 100, 10), 0);
    lv_label_set_text(_labelWarning, LV_SYMBOL_WARNING);
  }
}

void classScreen::adWipeEventHandler(lv_event_cb_t callBack)
{
  //    lv_obj_set_gesture_parent(_btnFooter, false);
  lv_obj_clear_flag(_btnFooter, LV_OBJ_FLAG_GESTURE_BUBBLE);
  lv_obj_add_event_cb(_btnFooter, callBack, LV_EVENT_GESTURE, this);
}

void classScreen::adScreenEventHandler(lv_event_cb_t callBack)
{
  lv_obj_add_event_cb(screen, callBack, LV_EVENT_ALL, this);
}
