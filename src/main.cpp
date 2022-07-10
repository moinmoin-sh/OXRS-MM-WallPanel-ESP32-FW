/**
  ESP32 wall panel firmware for the Open eXtensible Rack System

  See https://oxrs.io/docs/firmware/state-io-esp32.html for documentation.

  Compile options:
    ESP32

  Bugs/Features:
    See GitHub issues list

  Copyright 2019-2022 SuperHouse Automation Pty Ltd
*/

/*--------------------------- Defines -----------------------------------*/
// LCD backlight control
// TFT_BL GPIO pin defined in user_setup.h of tft_eSPI
// setting PWM properties
#define BL_PWM_FREQ 5000
#define BL_PWM_CHANNEL 0
#define BL_PWM_RESOLUTION 8

// I2C touch controller FT6336U
#define I2C_SDA 18
#define I2C_SCL 19
#define INT_N_PIN 39

/*--------------------------- Libraries ----------------------------------*/
#include <globalDefines.h>
#include <classTile.h>
#include <classTileList.h>
#include <classScreen.h>
#include <classScreenList.h>
#include <classScreenSettings.h>
#include <classDropDown.h>
#include <classRemote.h>
#include <classKeyPad.h>
#include <classIconList.h>
#include <base64.hpp>
#include <TFT_eSPI.h>
#include <lvgl.h>
#include <classFT6336U.h>

#include <OXRS_WT32.h> // WT32 support

/*--------------------------- Constants ----------------------------------*/
// Serial
#define SERIAL_BAUD_RATE 115200
/*--------------------------- Global Variables ---------------------------*/

// iosicons
extern "C" const lv_img_dsc_t ios_room_60;
extern "C" const lv_img_dsc_t ios_up;
extern "C" const lv_img_dsc_t ios_up_l;
extern "C" const lv_img_dsc_t ios_up_nb_30;
extern "C" const lv_img_dsc_t ios_down;
extern "C" const lv_img_dsc_t ios_down_nb_30;
extern "C" const lv_img_dsc_t ios_bulb_60;
extern "C" const lv_img_dsc_t ios_door_60;
extern "C" const lv_img_dsc_t ios_coffee_60;
extern "C" const lv_img_dsc_t ios_window_60;
extern "C" const lv_img_dsc_t ios_blind_60;
extern "C" const lv_img_dsc_t ios_settings_25;
extern "C" const lv_img_dsc_t ios_settings_25_l;
extern "C" const lv_img_dsc_t ios_back_25;
extern "C" const lv_img_dsc_t ios_back_25_l;
extern "C" const lv_img_dsc_t ios_home_25;
extern "C" const lv_img_dsc_t ios_home_25_l;
extern "C" const lv_img_dsc_t ios_thermometer_60;
extern "C" const lv_img_dsc_t ui_img_austins_black_320x70_png;
extern "C" const lv_img_dsc_t splash;
extern "C" const lv_img_dsc_t ios_onoff_60;
extern "C" const lv_img_dsc_t ios_speaker_60;
extern "C" const lv_img_dsc_t ios_t_60;
extern "C" const lv_img_dsc_t ios_prev_40;
extern "C" const lv_img_dsc_t ios_next_40;
extern "C" const lv_img_dsc_t ios_play_60;
extern "C" const lv_img_dsc_t ios_pause_60;
extern "C" const lv_img_dsc_t ios_music_60;
extern "C" const lv_img_dsc_t ios_3dprint_60;
extern "C" const lv_img_dsc_t ios_remote_60;
extern "C" const lv_img_dsc_t oxrs_splash_2206_png;
extern "C" const lv_img_dsc_t AustinsBlack_png;
extern "C" const lv_img_dsc_t ios_locked_60;
extern "C" const lv_img_dsc_t ios_unlocked_60;
extern "C" const lv_img_dsc_t ios_ceiling_fan_60;
extern "C" const lv_img_dsc_t ios_left_30;
extern "C" const lv_img_dsc_t ios_right_30;

const void *imgBlind = &ios_blind_60;
const void *imgBulb = &ios_bulb_60;
const void *imgWindow = &ios_window_60;
const void *imgDoor = &ios_door_60;
const void *imgCoffee = &ios_coffee_60;
const void *imgSettings = &ios_settings_25_l;
const void *imgUp = &ios_up_nb_30;
const void *imgDown = &ios_down_nb_30;
const void *imgBack = &ios_back_25_l;
const void *imgHome = &ios_home_25_l;
const void *imgRoom = &ios_room_60;
const void *imgThermo = &ios_thermometer_60;
const void *imgAustin = &AustinsBlack_png;
const void *imgSplash = &oxrs_splash_2206_png;
const void *imgOnOff = &ios_onoff_60;
const void *imgSpeaker = &ios_speaker_60;
const void *imgText = &ios_t_60;
const void *imgPrev = &ios_prev_40;
const void *imgNext = &ios_next_40;
const void *imgPlay = &ios_play_60;
const void *imgPause = &ios_pause_60;
const void *imgMusic = &ios_music_60;
const void *img3dPrint = &ios_3dprint_60;
const void *imgRemote = &ios_remote_60;
const void *imgLocked = &ios_locked_60;
const void *imgUnLocked = &ios_unlocked_60;
const void *imgCeilingFan = &ios_ceiling_fan_60;
const void *imgLeft = &ios_left_30;
const void *imgRight = &ios_right_30;

int _actBackLight;
int _retainedBackLight;
connectionState_t _connectionState = CONNECTED_NONE;
uint32_t _noActivityTimeOutToHome = 0L;
uint32_t _noActivityTimeOutToSleep = 0L;

#define DEFAULT_COLOR_ON_RED   91
#define DEFAULT_COLOR_ON_GREEN 190
#define DEFAULT_COLOR_ON_BLUE  91
lv_color_t colorOn;
lv_color_t colorBg;

/*--------------------------- Global Objects -----------------------------*/

// WT32 handler
OXRS_WT32 wt32;

// the tile_style_LUT
styleLutEntry_t styleLut[TS_STYLE_COUNT] = {0};

// iconVault holds all icon image name and reference
classIconList iconVault = classIconList();

// screenVault holds all screens
classScreenList screenVault = classScreenList();

// tileVault holds all tiles
classTileList tileVault = classTileList();

// the Settings Screen
classScreenSettings screenSettings = classScreenSettings();

// drop down overlay
classDropDown dropDownOverlay = classDropDown();

// remote overlay
classRemote remoteControl = classRemote();

// key pad overlay
classKeyPad keyPad = classKeyPad();

/*--------------------------- screen / lvgl relevant  -----------------------------*/

// Change to your screen resolution
static const uint16_t screenWidth = SCREEN_WIDTH;
static const uint16_t screenHeight = SCREEN_HEIGHT;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];
lv_indev_t *myInputDevice;

// TFT instance
TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight);
// touch controller instance
classFT6336U ft6336u = classFT6336U(I2C_SDA, I2C_SCL, INT_N_PIN);

#if LV_USE_LOG != 0
// Serial debugging if enabled
void my_print(const char *buf)
{
  Serial.printf(buf);
  Serial.flush();
}
#endif

/*-----------------  Icon_Vault and tile_style_LUT handler  -----------------------*/
// initialise the Icon_Vault
void initIconVault(void)
{
  iconVault.add({string("_blind"), imgBlind});
  iconVault.add({string("_bulb"), imgBulb});
  iconVault.add({string("_ceilingfan"), imgCeilingFan});
  iconVault.add({string("_coffee"), imgCoffee});
  iconVault.add({string("_door"), imgDoor});
  iconVault.add({string("_locked"), imgLocked});
  iconVault.add({string("_unlocked"), imgUnLocked});
  iconVault.add({string("_musik"), imgMusic});
  iconVault.add({string("_remote"), imgRemote});
  iconVault.add({string("_speaker"), imgSpeaker});
  iconVault.add({string("_window"), imgWindow});
  iconVault.add({string("_3dprint"), img3dPrint});
  iconVault.add({string("_onoff"), imgOnOff});
  iconVault.add({string("_play"), imgPlay});
  iconVault.add({string("_pause"), imgPause});
  iconVault.add({string("_thermometer"), imgThermo});
}

// initialise the tile_style_LUT
void initStyleLut(void)
{
  styleLut[TS_NONE] = {TS_NONE, "", NULL};
  styleLut[TS_BUTTON] = {TS_BUTTON, "button", NULL};
  styleLut[TS_BUTTON_LEVEL_UP] = {TS_BUTTON_LEVEL_UP, "buttonLevelUp", NULL};
  styleLut[TS_BUTTON_LEVEL_DOWN] = {TS_BUTTON_LEVEL_DOWN, "buttonLevelDown", NULL};
  styleLut[TS_BUTTON_UP_DOWN] = {TS_BUTTON_UP_DOWN, "buttonUpDown", NULL};
  styleLut[TS_BUTTON_LEFT_RIGHT] = {TS_BUTTON_LEFT_RIGHT, "buttonLeftRight", NULL};
  styleLut[TS_BUTTON_PREV_NEXT] = {TS_BUTTON_PREV_NEXT, "buttonPrevNext", NULL};
  styleLut[TS_INDICATOR] = {TS_INDICATOR, "indicator", NULL};
  styleLut[TS_COLOR_PICKER] = {TS_COLOR_PICKER, "colorPicker", NULL};
  styleLut[TS_DROPDOWN] = {TS_DROPDOWN, "dropDown", NULL};
  styleLut[TS_KEYPAD] = {TS_KEYPAD, "keyPad", NULL};
  styleLut[TS_KEYPAD_BLOCKING] = {TS_KEYPAD_BLOCKING, "keyPadBlocking", NULL};
  styleLut[TS_REMOTE] = {TS_REMOTE, "remote", imgRemote};
  styleLut[TS_LINK] = {TS_LINK, "link", NULL};
}

// converts a style string to its enum
int parseInputStyle(const char *inputStyle)
{
  int i;
  for (i = 0; i < TS_STYLE_COUNT; i++)
  {
    if (styleLut[i].styleStr)
      if (strcmp(styleLut[i].styleStr, inputStyle) == 0)
        break;
  }
  // in range check
  if (i >= TS_STYLE_COUNT) i = 0;
  return (styleLut[i].style);
}

// converts a style enum to its string
const char *styleEnum2Str(int styleEnum)
{
  // in range check
  if (styleEnum >= TS_STYLE_COUNT) styleEnum = 0;
  return styleLut[styleEnum].styleStr;
}

/*--------------------------- publish with MQTT  -----------------------------*/

// publish Tile Event
// {"screen":1, "tile":1, "type":"button", "event":"single" , "state":"on"}
void publishTileEvent(classTile *tPtr, const char *event)
{
  StaticJsonDocument<128> json;
  json["screen"] = tPtr->getScreenIdx();
  json["tile"] = tPtr->getTileIdx();
  json["style"] = tPtr->getStyleStr();
  json["type"] = "button";
  json["event"] = event;
  json["state"] = (tPtr->getState() == true) ? "on" : "off";

  wt32.publishStatus(json.as<JsonVariant>());
}

// publish remote button Event
// {"screen":1, "tile":1, "style":remote, "type":"up", "event":"single" }
void publishRemoteEvent(classTile *tPtr, int btnIndex, const char *event)
{
  StaticJsonDocument<128> json;
  json["screen"] = tPtr->getScreenIdx();
  json["tile"] = tPtr->getTileIdx();
  json["style"] = tPtr->getStyleStr();
  switch (btnIndex)
  {
  case 1:    json["type"] = "up";    break;
  case 2:    json["type"] = "down";  break;
  case 3:    json["type"] = "left";  break;
  case 4:    json["type"] = "right"; break;
  case 5:    json["type"] = "ok";    break;
  case 6:    json["type"] = "info";  break;
  case 7:    json["type"] = "list";  break;
  case 8:    json["type"] = "back";  break;
  case 9:    json["type"] = "home";  break;
  }
  json["event"] = event;

  wt32.publishStatus(json.as<JsonVariant>());
}

// publish dropdown change Event
// {"screen":1, "tile":1, "type":"dropdown", "event":"change" , "state":1}
void publishDropDownEvent(classTile *tPtr, int listIndex)
{
  StaticJsonDocument<128> json;
  json["screen"] = tPtr->getScreenIdx();
  json["tile"] = tPtr->getTileIdx();
  json["style"] = tPtr->getStyleStr();
  json["type"] = "dropDown";
  json["event"] = "selection";
  json["state"] = listIndex;

  wt32.publishStatus(json.as<JsonVariant>());
}

// publish key pad change Event
// {"screen":1, "tile":1, "style":"light", "type":"button", "event":"key", "state":"off", "keycode":"1234" }
void publishKeyPadEvent(classTile *tPtr, const char *key)
{
  StaticJsonDocument<128> json;
  json["screen"] = tPtr->getScreenIdx();
  json["tile"] = tPtr->getTileIdx();
  json["style"] = tPtr->getStyleStr();
  json["type"] = "button";
  json["event"] = "key";
  json["state"] = (tPtr->getState() == true) ? "on" : "off";
  json["keyCode"] = key;

  wt32.publishStatus(json.as<JsonVariant>());
}

// publish Level change Event
// {"screen":1, "tile":1, "type":"level", "event":"change" , "state":50}
void publishLevelEvent(classTile *tPtr, const char *event, int value)
{
  StaticJsonDocument<128> json;
  json["screen"] = tPtr->getScreenIdx();
  json["tile"] = tPtr->getTileIdx();
  json["style"] = tPtr->getStyleStr();
  json["type"] = "level";
  json["event"] = event;
  json["state"] = value;

  wt32.publishStatus(json.as<JsonVariant>());
}

// publish prev / next Event
// {"screen":1, "tile":1, "type":"prev"|"next", "event":"single"|"hold" }
void publishPrevNextEvent(classTile *tPtr, const char *type, const char *event)
{
  StaticJsonDocument<128> json;
  json["screen"] = tPtr->getScreenIdx();
  json["tile"] = tPtr->getTileIdx();
  json["style"] = tPtr->getStyleStr();
  json["type"] = type;
  json["event"] = event;

  wt32.publishStatus(json.as<JsonVariant>());
}

// publish selector Event
// {"screen":1, "tile":1, "type":"prev"|"next", "event":"single"|"hold" }
void publishSelectorEvent(classTile *tPtr, int index)
{
  StaticJsonDocument<128> json;
  json["screen"] = tPtr->getScreenIdx();
  json["tile"] = tPtr->getTileIdx();
  json["style"] = tPtr->getStyleStr();
  json["type"] = "selector";
  json["event"] = "selection";
  json["state"] = index;

  wt32.publishStatus(json.as<JsonVariant>());
}

// publish Screen Event
// {"screen":1, "type":"screen", "event":"change" , "state":"unloaded"}
void publishScreenEvent(int screenIdx, const char *state)
{
  StaticJsonDocument<128> json;
  json["screen"] = screenIdx;
  json["type"] = "screen";
  json["event"] = "change";
  json["state"] = state;

  wt32.publishStatus(json.as<JsonVariant>());
}

// publish backlight change event
//   “type” : “backlight”, “event” : “change”, “state” : “sleep” |”awake”, “brightness” : <number>
void publishBacklightEvent(int brightness)
{
  StaticJsonDocument<128> json;
  json["type"] = "“backlight”";
  json["event"] = "change";
  json["state"] = (brightness != 0) ? "awake" : "sleep" ;
  json["brightness"] = brightness;

  wt32.publishStatus(json.as<JsonVariant>());
}

  // publish message box closed Event
  // {"screen":0, "type":"message", "event":"open" , "state":"open"}
  void publishMsgBoxEvent(const char *event, const char *state)
  {
    StaticJsonDocument<128> json;
    json["screen"] = 0;
    json["type"] = "message";
    json["event"] = event;
    json["state"] = state;

    wt32.publishStatus(json.as<JsonVariant>());
  }

  // publish local Backlight change
  //{"backlight:" 50}
  void publishBackLightTelemetry(void)
  {
    StaticJsonDocument<32> json;
    json["backlight"] = _actBackLight;
    wt32.publishTelemetry(json.as<JsonVariant>());
  }

  /*
      backlight of LCD handling
  */

  // low level setting
  void _setBackLightLED(int val)
  {
    if (val > 100)   val = 100;
    if (val < 0)     val = 0;

    ledcWrite(BL_PWM_CHANNEL, 255 * val / 100);
    _actBackLight = val;
  }

  void _setBackLight(int val)
  {
    _setBackLightLED(val);
    screenSettings.setSlider(val);
    publishBacklightEvent(val);
  }

  // update the slider
  // void setBackLightSliderValue(int value)
  // {
  //   screenSettings.setSlider(value);
  // }

  /*
     lcd interface
     transfer pixel data range to lcd
  */
  void my_disp_flush(lv_disp_drv_t * disp, const lv_area_t *area, lv_color_t *color_p)
  {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)&color_p->full, w * h, true);

    lv_disp_flush_ready(disp);
  }

  /*
     touch pad interface
     test for touch and report RELEASED / or PRESSED + x/y back
  */
  void my_touchpad_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data)
  {
    typePoint ts;
    bool touched = ft6336u.readTouchPoint(&ts);

    // no touch detected
    if (!touched)
    {
      data->state = LV_INDEV_STATE_REL;
      return;
    }
    // touch detected while backlight = 0
    if (_actBackLight == 0)
    {
      _setBackLight(_retainedBackLight);
      delay(200);
      // mimic keypress in top/left corner (no sensitive area)
      ts.x = 0;
      ts.y = 0;
    }
    // get coordinates and write into point structure
    data->point.x = ts.x;
    data->point.y = ts.y;
    data->state = LV_INDEV_STATE_PR;

    Serial.print("Data x,y ");
    Serial.print(data->point.x);
    Serial.print(",");
    Serial.println(data->point.y);
  }

  // check for timeout inactivity timeout
  void checkNoAvtivity(void)
  {
    // observer disabled
    if (_noActivityTimeOutToHome != 0)
    {
      // Screen is HomeScreen ?
      if (lv_scr_act() != screenVault.get(SCREEN_HOME)->screen)
      {
        // time elapsed, jump to HomeScreen
        if (lv_disp_get_inactive_time(NULL) > _noActivityTimeOutToHome)
          screenVault.show(SCREEN_HOME);
      }
    }
    if (_noActivityTimeOutToSleep != 0)
    {
      // is in sleep allready ?
      if (_actBackLight > 0)
      {
        // time elapsed, jump to HomeScreen
        if (lv_disp_get_inactive_time(NULL) > _noActivityTimeOutToSleep)
        {
          _retainedBackLight = _actBackLight;
          _setBackLight(0);
        }
      }
    }
  }

  /*
   * ui helper functions
   */

  void defaultOnColorConfig(int red, int green, int blue)
  {
    // all zero is defined as unset, so set default
    if ((red + green + blue) == 0)
    {
      colorOn = lv_color_make(DEFAULT_COLOR_ON_RED, DEFAULT_COLOR_ON_GREEN, DEFAULT_COLOR_ON_BLUE);
    }
    else
    {
      colorOn = lv_color_make(red, green, blue);
    }
  }

  void defaultThemeColorConfig(int red, int green, int blue)
  {
    // all zero is defined as unset, so set default
    if ((red + green + blue) == 0)
    {
      colorBg = lv_color_make(0, 0, 0);
    }
    else
    {
      colorBg = lv_color_make(red, green, blue);
    }
  }

  // update info text panel on screenSettings
  void updateInfoText(void)
  {
    char buffer[40];

    lv_obj_t *table = screenSettings.getInfoPanel();
    lv_table_set_row_cnt(table, 8);
    lv_table_set_col_cnt(table, 2);

    lv_table_set_cell_value(table, 0, 0, "Name:");
    lv_table_set_cell_value(table, 0, 1, STRINGIFY(FW_NAME));
    lv_table_set_cell_value(table, 1, 0, "Maker:");
    lv_table_set_cell_value(table, 1, 1, STRINGIFY(FW_MAKER));
    lv_table_set_cell_value(table, 2, 0, "Version:");
    lv_table_set_cell_value(table, 2, 1, STRINGIFY(FW_VERSION));

    lv_table_set_cell_value(table, 4, 0, "MAC:");
    wt32.getMACAddressTxt(buffer);
    lv_table_set_cell_value(table, 4, 1, buffer);

    lv_table_set_cell_value(table, 5, 0, "IP:");
    wt32.getIPAddressTxt(buffer);
    lv_table_set_cell_value(table, 5, 1, buffer);

    lv_table_set_cell_value(table, 6, 0, "MODE:");
    #ifndef WIFI_MODE
      lv_table_set_cell_value(table, 6, 1, "Ethernet");
    #else
      lv_table_set_cell_value(table, 6, 1, "WiFi");
    #endif

      lv_table_set_cell_value(table, 7, 0, "MQTT:");
      wt32.getMQTTTopicTxt(buffer);
      lv_table_set_cell_value(table, 7, 1, buffer);
  }

  // check for changes in IP/MQTT connection and update warning sign in footer
  void updateConnectionStatus(void)
  {
    connectionState_t connectionState = wt32.getConnectionState();
    if (_connectionState != connectionState)
    {
      _connectionState = connectionState;
      // update footers in all screens
      classScreen *sPtr = screenVault.getStart();
      do
      {
        sPtr->showConnectionStatus(_connectionState == CONNECTED_MQTT);
      } while ((sPtr = screenVault.getNext(sPtr->screenIdx)));

      // update info text to reflect actual status
      updateInfoText();
    }
  }

  // screen selection via mqtt
  void selectScreen(int screenIdx)
  {
    screenVault.show(screenIdx);
  }

  /*--------------------------- Event Handler ------------------------------------*/

  // screen event handler
  // detects unload and load
  void screenEventHandler(lv_event_t * e)
  {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_SCREEN_UNLOAD_START)
    {
      classScreen *sPtr = (classScreen *)lv_event_get_user_data(e);
      publishScreenEvent(sPtr->screenIdx, "unloaded");
    }
    if (code == LV_EVENT_SCREEN_LOADED)
    {
      classScreen *sPtr = (classScreen *)lv_event_get_user_data(e);
      publishScreenEvent(sPtr->screenIdx, "loaded");
    }
  }

  // message box closed event handler
  void msgBoxClosedEventHandler(lv_event_t * e)
  {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_DELETE)
      publishMsgBoxEvent("close", "closed");
  }

  // Up / Down Button Event Handler
  static void upDownEventHandler(lv_event_t * e)
  {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);

    if ((code == LV_EVENT_SHORT_CLICKED) || (code == LV_EVENT_LONG_PRESSED) || (code == LV_EVENT_LONG_PRESSED_REPEAT))
    {
      // get tile* of clicked tile from USER_DATA
      classTile *tPtr = (classTile *)lv_event_get_user_data(e);
      int levelStart = tPtr->getLevelStart();
      int levelStop = tPtr->getLevelStop();
      int levelLargeStep = tPtr->getLevelLargeStep();
      int level = tPtr->getLevel();
      // short increments 1; long increments 5
      int increment = (code == LV_EVENT_SHORT_CLICKED) ? 1 : levelLargeStep;
      int direction = lv_obj_has_flag(btn, LV_OBJ_FLAG_USER_1) ? 1 : -1;
      increment *= direction;
      // calc new value and limit to 0...100
      level += increment;
      if (level > levelStop)
        level = levelStop;
      if (level < levelStart)
        level = levelStart;
      tPtr->setLevel(level, true);
      tPtr->showOvlBar(level);
      // send event
      publishLevelEvent(tPtr, (direction == 1) ? "up" : "down", level);
    }
  }

  // previous/next event handler
  static void prevNextEventHandler(lv_event_t * e)
  {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    const char *type = NULL;

    if ((code == LV_EVENT_SHORT_CLICKED) || (code == LV_EVENT_LONG_PRESSED) || (code == LV_EVENT_LONG_PRESSED_REPEAT))
    {
      classTile *tPtr = (classTile *)lv_event_get_user_data(e);
      // has selector
      if (tPtr->getSelectorValid())
      {
        int index = tPtr->getSelectorIndex();
        lv_obj_has_flag(btn, LV_OBJ_FLAG_USER_1) ? index-- : index++;
        tPtr->showSelector(index);
        publishSelectorEvent(tPtr, tPtr->getSelectorIndex());
      }
      // up / down events only
      else
      {
        if (tPtr->getStyle() == TS_BUTTON_PREV_NEXT)
          type = lv_obj_has_flag(btn, LV_OBJ_FLAG_USER_1) ? "prev" : "next";
        if (tPtr->getStyle() == TS_BUTTON_LEFT_RIGHT)
          type = lv_obj_has_flag(btn, LV_OBJ_FLAG_USER_1) ? "left" : "right";
        if (tPtr->getStyle() == TS_BUTTON_UP_DOWN)
          type = lv_obj_has_flag(btn, LV_OBJ_FLAG_USER_1) ? "up" : "down";
        const char *event = (code == LV_EVENT_SHORT_CLICKED) ? "single" : "hold";
        publishPrevNextEvent(tPtr, type, event);
      }
    }
  }

  // key pad event handler
  static void keyPadEventHandler(lv_event_t * e)
  {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_SHORT_CLICKED)
    {
      classTile *tPtr = (classTile *)lv_event_get_user_data(e);
      uint32_t id = lv_btnmatrix_get_selected_btn(obj);
      const char *txt = lv_btnmatrix_get_btn_text(obj, id);

      LV_LOG_USER("%s was pressed\n", txt);
      if (strcmp(txt, LV_SYMBOL_BACKSPACE) == 0)
      {
        keyPad.delChar();
      }
      else if (strcmp(txt, LV_SYMBOL_NEW_LINE) == 0)
      {
        if ((strlen(keyPad.getKey()) == 0) && !(tPtr->getStyle() == TS_KEYPAD_BLOCKING))
          keyPad.close();
        if (strlen(keyPad.getKey()) > 0)
          publishKeyPadEvent(tPtr, keyPad.getKey());
      }
      else
      {
        keyPad.addChar(txt[0]);
      }
    }
  }

  // remote control
  static void navigationButtonEventHandler(lv_event_t * e)
  {
    int btnIndex = 0;
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if ((code == LV_EVENT_SHORT_CLICKED) || (code == LV_EVENT_LONG_PRESSED))
    {
      if (lv_obj_has_flag(obj, LV_OBJ_FLAG_USER_1))
        btnIndex += 1;
      if (lv_obj_has_flag(obj, LV_OBJ_FLAG_USER_2))
        btnIndex += 2;
      if (lv_obj_has_flag(obj, LV_OBJ_FLAG_USER_3))
        btnIndex += 4;
      if (lv_obj_has_flag(obj, LV_OBJ_FLAG_USER_4))
        btnIndex += 8;
      classTile *tPtr = (classTile *)lv_event_get_user_data(e);
      if (code == LV_EVENT_SHORT_CLICKED)
        publishRemoteEvent(tPtr, btnIndex, "single");
      else
        publishRemoteEvent(tPtr, btnIndex, "hold");
    }
  }

  // drop down event handler
  static void dropDownEventHandler(lv_event_t * e)
  {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    // CANCEL fired when drop down list closed
    if (code == LV_EVENT_CANCEL)
    {
      classTile *tPtr = (classTile *)lv_event_get_user_data(e);
      char buf[64];
      lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
      int listIndex = lv_dropdown_get_selected(obj) + 1;
      publishDropDownEvent(tPtr, listIndex);
      tPtr->setIconText(buf);
      tPtr->setDropDownIndex(listIndex);
      dropDownOverlay.close();
    }
  }

  // screen select drop down Event Handler
  static void screenDropDownEventHandler(lv_event_t * e)
  {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_CANCEL)
    {
      int listIndex = lv_dropdown_get_selected(obj);
      screenVault.showByIndex(listIndex);
      dropDownOverlay.close();
    }
  }

  // general Tile Event Handler
  static void tileEventHandler(lv_event_t * e)
  {
    lv_event_code_t code = lv_event_get_code(e);

    if ((code == LV_EVENT_SHORT_CLICKED) || (code == LV_EVENT_LONG_PRESSED))
    {
      // get tile* of clicked tile from USER_DATA
      classTile *tPtr = (classTile *)lv_event_get_user_data(e);
      tileId_t tileId = tPtr->getId();
      int linkedScreen = tPtr->getLink();
      if (code == LV_EVENT_SHORT_CLICKED)
      {
        // button has link -> call linked screen
        if (linkedScreen > 0)
        {
          screenVault.show(linkedScreen);
        }
        // button is style DROPDOWN -> show drop down overlay
        else if (tPtr->getStyle() == TS_DROPDOWN)
        {
          dropDownOverlay = classDropDown(tPtr, dropDownEventHandler);
          dropDownOverlay.open();
        }
        // button is style REMOTE -> show remote overlay
        else if (tPtr->getStyle() == TS_REMOTE)
        {
          remoteControl = classRemote(tPtr, navigationButtonEventHandler);
        }
        // keypad is enabled for this tile
        else if (tPtr->getKeyPadEnable())
        {
          keyPad = classKeyPad(tPtr, keyPadEventHandler);
        }

        //  publish click event
        else
        {
          publishTileEvent(tPtr, "single");
        }
      }
      // long press detected
      else
      {
        publishTileEvent(tPtr, "hold");
      }
    }
  }

  // screen footer button Event handler
  //    HomeButton            -> displays Home screen
  //    SettingsButton        -> displays Settings
  //    Center Button (label) -> show screen select drop down
  static void footerButtonEventHandler(lv_event_t * e)
  {
    lv_event_code_t event = lv_event_get_code(e);
    lv_obj_t *ta = lv_event_get_target(e);
    if (event == LV_EVENT_SHORT_CLICKED)
    {
      if (lv_obj_has_flag(ta, LV_OBJ_FLAG_USER_1))
        screenVault.show(SCREEN_HOME);
      if (lv_obj_has_flag(ta, LV_OBJ_FLAG_USER_3))
        screenVault.show(SCREEN_SETTINGS);
      if (lv_obj_has_flag(ta, LV_OBJ_FLAG_USER_2))
      {
        dropDownOverlay = classDropDown(NULL, screenDropDownEventHandler);
        char buf[256];
        int index = screenVault.makeDropDownList(buf, lv_scr_act()) + 1;
        dropDownOverlay.setDropDownList(buf);
        dropDownOverlay.setDropDownIndex(index);
        dropDownOverlay.setDropDownLabel("Select Screen");
        dropDownOverlay.open();
      }
    }
  }

  // BackLight slider event handler
  static void backLightSliderEventHandler(lv_event_t * e)
  {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *slider = lv_event_get_target(e);

    if (code == LV_EVENT_VALUE_CHANGED)
    {
      classScreenSettings *sPtr = (classScreenSettings *)lv_event_get_user_data(e);
      sPtr->setSlider((int)lv_slider_get_value(slider));
      _setBackLightLED(lv_slider_get_value(slider));
    }
    if (code == LV_EVENT_RELEASED)
    {
      _setBackLight(lv_slider_get_value(slider));
    }
  }

  // show modal message box on screen
  void _showMsgBox(const char *title, const char *text)
  {
    lv_obj_t *mbox1 = lv_msgbox_create(NULL, title, text, NULL, true);

    lv_obj_t *cbtn = lv_msgbox_get_close_btn(mbox1);
    lv_obj_set_style_bg_color(cbtn, lv_color_make(128, 30, 0), 0);
    lv_obj_set_style_bg_opa(cbtn, 255, 0);
    lv_obj_center(mbox1);

    lv_obj_add_event_cb(mbox1, msgBoxClosedEventHandler, LV_EVENT_ALL, NULL);
    publishMsgBoxEvent("open", "open");
  }

  // create screen for tiles in screenVault if not exists
  void createScreen(int screenIdx)
  {
    // exit if screenIdx exits
    if (screenVault.exist(screenIdx))
      return;
    // create new screen with grid container
    classScreen &ref = screenVault.add(screenIdx, 1);
    ref.createHomeButton(footerButtonEventHandler, imgHome);
    ref.createSettingsButton(footerButtonEventHandler, imgSettings);
    ref.adScreenEventHandler(screenEventHandler);
    // sort screenIdx in ascending order
    screenVault.sort();
  }

  /**
    Config handler
   */

  // icon list for config
  void createIconEnum(JsonObject parent)
  {
    JsonArray styleEnum = parent.createNestedArray("enum");

    string iconStr;
    iconVault.setIteratorStart();
    while ((iconStr = iconVault.getNextStr()) != "")
    {
      styleEnum.add(iconStr);
    }
  }

  // style list for config
  void createInputStyleEnum(JsonObject parent)
  {
    JsonArray styleEnum = parent.createNestedArray("enum");

    for (int i = 1; i < TS_STYLE_COUNT; i++)
    {
      styleEnum.add(styleLut[i].styleStr);
    }
  }

  // Create any tile on any screen
  void createTile(const char *styleStr, int screenIdx, int tileIdx, const char *iconStr, const char *label, int linkedScreen, int levelStart, int levelStop)
  {
    const void *img = NULL;
    int style;

    // create screen if not exist
    createScreen(screenIdx);

    // delete tile reference if exist
    tileVault.remove(screenIdx, tileIdx);

    // get the tile_style
    style = parseInputStyle(styleStr);

    // get the icon image
    if (iconStr)
      img = iconVault.getIcon(string(iconStr));
    if (!img)
      img = styleLut[style].imgDefault;

    // create new Tile
    classTile &ref = tileVault.add();
    ref.begin(screenVault.get(screenIdx)->container, img, label);
    ref.registerTile(screenIdx, tileIdx, style, styleStr);

    // handle tiles depending on style capabilities
    if ((style == TS_LINK) && linkedScreen)
    {
      ref.setLink(linkedScreen);
      // create screen if not exist
      createScreen(linkedScreen);
    }

    // set the event handler if NOT (INDICATOR_*)
    if (style != TS_INDICATOR)
    {
      ref.addEventHandler(tileEventHandler);
    }

    // enable on-tile level control (bottom-up)
    if (style == TS_BUTTON_LEVEL_UP)
    {
      ref.addUpDownControl(upDownEventHandler, imgUp, imgDown);
    }

    // enable on-tile level control (top-down)
    if (style == TS_BUTTON_LEVEL_DOWN)
    {
      ref.setTopDownMode(true);
      ref.addUpDownControl(upDownEventHandler, imgUp, imgDown);
    }

    // set levelrange
    if ((style == TS_BUTTON_LEVEL_UP) || (style == TS_BUTTON_LEVEL_DOWN))
    {
      if ((levelStart != 0) || (levelStop != 0))
      {
        // allow increasing only (Stop > Start)
        if(levelStop < levelStart)
        {
          wt32.println(F("[wpan] invalid level range."));
        }
        else
        {
          ref.setLevelStartStop(levelStart, levelStop);
        }
      }
    }

    // enable prev/next control (button events only)
    if (style == TS_BUTTON_PREV_NEXT)
    {
      ref.addUpDownControl(prevNextEventHandler, imgPrev, imgNext);
    }

    // enable up/down control (button events only)
    if (style == TS_BUTTON_UP_DOWN)
    {
      ref.addUpDownControl(prevNextEventHandler, imgUp, imgDown);
    }

    // enable left/right control (button events only)
    if (style == TS_BUTTON_LEFT_RIGHT)
    {
      ref.addUpDownControl(prevNextEventHandler, imgLeft, imgRight);
    }

    // enable key pad popup, set img for state ON to imgLocked
    if ((style == TS_KEYPAD) || (style == TS_KEYPAD_BLOCKING))
    {
      ref.setKeyPadEnable(true);
    }

    // set indicator for modal screen
    if ((style == TS_DROPDOWN) || (style == TS_REMOTE) || (style == TS_KEYPAD) || (style == TS_KEYPAD_BLOCKING))
    {
      ref.setDropDownIndicator();
    }
  }

  /**
   * Config Handler
   */

  void jsonOnColorConfig(JsonVariant json)
  {
    uint8_t red, green, blue;

    red = (uint8_t)json["red"].as<int>();
    green = (uint8_t)json["green"].as<int>();
    blue = (uint8_t)json["blue"].as<int>();

    defaultOnColorConfig(red, green, blue);
  }

  void jsonThemeColorConfig(JsonVariant json)
  {
    uint8_t red, green, blue;

    red = (uint8_t)json["red"].as<int>();
    green = (uint8_t)json["green"].as<int>();
    blue = (uint8_t)json["blue"].as<int>();

    // update all instances
    defaultThemeColorConfig(red, green, blue);
    classScreen *sPtr = screenVault.getStart();
    do
    {
      if (sPtr) sPtr->updateBgColor();
    } while ((sPtr = screenVault.getNext(sPtr->screenIdx)));
  }

  void jsonTilesConfig(int screenIdx, JsonVariant json)
  {
    if ((screenIdx < SCREEN_START) || (screenIdx > SCREEN_END))
    {
      wt32.print(F("[wpan] invalid screen: "));
      wt32.println(screenIdx);
      return;
    }

    int tileIdx = json["tile"].as<int>();
    if ((tileIdx < TILE_START) || (tileIdx > TILE_END))
    {
      wt32.print(F("[wpan] invalid tile: "));
      wt32.println(tileIdx);
      return;
    }

    createTile(json["style"], screenIdx, tileIdx, json["icon"], json["label"], json["link"], json["levelStart"], json["levelStop"]);
  }

  void jsonConfig(JsonVariant json)
  {
    if (json.containsKey("color"))
    {
      jsonOnColorConfig(json["color"]);
    }

    if (json.containsKey("colortheme"))
    {
      jsonThemeColorConfig(json["colortheme"]);
    }

    if (json.containsKey("noActivitySecondsToHome"))
    {
      _noActivityTimeOutToHome = json["noActivitySecondsToHome"].as<int>() * 1000;
    }

    if (json.containsKey("noActivitySecondsToSleep"))
    {
      _noActivityTimeOutToSleep = json["noActivitySecondsToSleep"].as<int>() * 1000;
    }

    if (json.containsKey("screens"))
    {
      for (JsonVariant screenJson : json["screens"].as<JsonArray>())
      {
        int screenIdx = screenJson["screen"].as<int>();
        createScreen(screenIdx);
        screenVault.get(screenIdx)->setLabel(screenJson["label"]);
        for (JsonVariant tileJson : screenJson["tiles"].as<JsonArray>())
        {
          jsonTilesConfig(screenIdx, tileJson);
        }
      }
    }
  }

  void screenConfigSchema(JsonVariant json)
  {
    // screens
    JsonObject screens = json.createNestedObject("screens");
    screens["title"] = "Screen Configuration";
    screens["description"] = "Add Screen(s). Screen 1 is the HomeScreen and needs to be configured!";
    screens["type"] = "array";

    JsonObject items2 = screens.createNestedObject("items");
    items2["type"] = "object";

    JsonObject properties2 = items2.createNestedObject("properties");

    JsonObject screen = properties2.createNestedObject("screen");
    screen["title"] = "Index";
    screen["type"] = "integer";
    screen["minimum"] = SCREEN_START;
    screen["maximum"] = SCREEN_END;

    JsonObject label2 = properties2.createNestedObject("label");
    label2["title"] = "Label";
    label2["type"] = "string";

    JsonArray required2 = items2.createNestedArray("required");
    required2.add("screen");
    required2.add("label");

    // tiles on screen
    JsonObject tiles = properties2.createNestedObject("tiles");
    tiles["title"] = "Tile Configuration";
    tiles["description"] = "Add Tiles to screen.";
    tiles["type"] = "array";

    JsonObject items3 = tiles.createNestedObject("items");
    items3["type"] = "object";

    JsonObject properties3 = items3.createNestedObject("properties");

    JsonObject tile3 = properties3.createNestedObject("tile");
    tile3["title"] = "Index";
    tile3["type"] = "integer";
    tile3["minimum"] = TILE_START;
    tile3["maximum"] = TILE_END;

    JsonObject type3 = properties3.createNestedObject("style");
    type3["title"] = "Style";
    createInputStyleEnum(type3);

    JsonObject icon = properties3.createNestedObject("icon");
    icon["title"] = "Icon";
    createIconEnum(icon);

    JsonObject label3 = properties3.createNestedObject("label");
    label3["title"] = "Label";
    label3["type"] = "string";

    JsonObject link = properties3.createNestedObject("link");
    link["title"] = "Linked Screen Index";
    link["description"] = "Required if Tile Style is 'link'.";
    link["type"] = "integer";
    link["minimum"] = SCREEN_START;
    link["maximum"] = SCREEN_END;

    JsonObject levelStart = properties3.createNestedObject("levelStart");
    levelStart["title"] = "Start value for level control.";
    levelStart["description"] = "Optional range start if Tile Style is 'buttonLevelxx'.";
    levelStart["type"] = "integer";

    JsonObject levelStop = properties3.createNestedObject("levelStop");
    levelStop["title"] = "Stop value for level control.";
    levelStop["description"] = "Optional range stop if Tile Style is 'buttonLevelxx'.";
    levelStop["type"] = "integer";

    JsonArray required3 = items3.createNestedArray("required");
    required3.add("tile");
    required3.add("style");

    // default Theme color
    JsonObject colortheme = json.createNestedObject("colortheme");
    colortheme["title"] = "Set Theme Color.";
    colortheme["description"] = "Enter your preferred RGB values.(Default [0, 0, 0])";

    JsonObject properties5 = colortheme.createNestedObject("properties");

    JsonObject red5 = properties5.createNestedObject("red");
    red5["title"] = "Red";
    red5["type"] = "integer";
    red5["minimum"] = 0;
    red5["maximum"] = 255;

    JsonObject green5 = properties5.createNestedObject("green");
    green5["title"] = "Green";
    green5["type"] = "integer";
    green5["minimum"] = 0;
    green5["maximum"] = 255;

    JsonObject blue5 = properties5.createNestedObject("blue");
    blue5["title"] = "Blue";
    blue5["type"] = "integer";
    blue5["minimum"] = 0;
    blue5["maximum"] = 255;

    // default ON color
    JsonObject color = json.createNestedObject("color");
    color["title"] = "Defaut Icon Color for ON state.";
    color["description"] = "Set your preferred RGB values.(Default [91, 190, 91])";

    JsonObject properties4 = color.createNestedObject("properties");

    JsonObject red = properties4.createNestedObject("red");
    red["title"] = "Red";
    red["type"] = "integer";
    red["minimum"] = 0;
    red["maximum"] = 255;

    JsonObject green = properties4.createNestedObject("green");
    green["title"] = "Green";
    green["type"] = "integer";
    green["minimum"] = 0;
    green["maximum"] = 255;

    JsonObject blue = properties4.createNestedObject("blue");
    blue["title"] = "Blue";
    blue["type"] = "integer";
    blue["minimum"] = 0;
    blue["maximum"] = 255;

    // noActivity timeout
    JsonObject noActivitySecondsToHome = json.createNestedObject("noActivitySecondsToHome");
    noActivitySecondsToHome["title"] = "Return to HomeScreen after Timeout (seconds) of no activity";
    noActivitySecondsToHome["description"] = "Display shows HomeScreen after Timeout (seconds) of no activity. 0 disables.";
    noActivitySecondsToHome["type"] = "integer";
    noActivitySecondsToHome["minimum"] = 0;
    noActivitySecondsToHome["maximum"] = 600;

    // noActivity timeout
    JsonObject noActivitySecondsToSleep = json.createNestedObject("noActivitySecondsToSleep");
    noActivitySecondsToSleep["title"] = "Set Screen to sleep (backlight off) after Timeout (seconds) of no activity";
    noActivitySecondsToSleep["description"] = "Screen is dimmed to 0 after Timeout (seconds) of no activity. 0 disables.";
    noActivitySecondsToSleep["type"] = "integer";
    noActivitySecondsToSleep["minimum"] = 0;
    noActivitySecondsToSleep["maximum"] = 3600;
  }

  void setConfigSchema()
  {
    // Define our config schema
    StaticJsonDocument<4096> json;
    JsonVariant config = json.as<JsonVariant>();

    screenConfigSchema(config);

    // Pass our config schema down to the WT32 library
    wt32.setConfigSchema(config);
  }

  /**
    Command handler
   */

  // decode base64 encoded png image to ps_ram
  lv_img_dsc_t *decodeBase64ToImg(const char *imageBase64)
  {
    // decode image into ps_ram
    // TODO :
    //    check if ps_alloc successful
    size_t inLen = strlen(imageBase64);
    // exit if no data to decode
    if (inLen == 0)
      return NULL;

    size_t outLen = BASE64::decodeLength(imageBase64);
    uint8_t *raw = (uint8_t *)ps_malloc(outLen);
    BASE64::decode(imageBase64, raw);

    // calc width and height from image file (start @ pos [16])
    uint32_t size[2];
    memcpy(&size[0], raw + 16, 8);

    // prepaare image descriptor
    lv_img_dsc_t *imgPng = (lv_img_dsc_t *)ps_malloc(sizeof(lv_img_dsc_t));
    imgPng->header.cf = LV_IMG_CF_RAW_ALPHA;
    imgPng->header.always_zero = 0;
    imgPng->header.reserved = 0;
    imgPng->header.w = (lv_coord_t)((size[0] & 0xff000000) >> 24) + ((size[0] & 0x00ff0000) >> 8);
    imgPng->header.h = (lv_coord_t)((size[1] & 0xff000000) >> 24) + ((size[1] & 0x00ff0000) >> 8);
    imgPng->data_size = outLen;
    imgPng->data = raw;

    return imgPng;
  }

  void jsonSetBackLightCommand(JsonVariant json)
  {
    int blValue = -1;
    if (json.containsKey("brightness"))
    {
      blValue = json["brightness"].as<int>();
      if (blValue < 1)    blValue = 1;
      if (blValue > 100)  blValue = 100;
    }
    if (json.containsKey("state"))
    {
      if (strcmp(json["state"], "sleep") == 0)
      {
        _retainedBackLight = _actBackLight;
        blValue = 0;
      }
      if (strcmp(json["state"], "awake") == 0)
      {
        blValue = _retainedBackLight;
      }
    }
    if (blValue > -1)
    {
      lv_disp_trig_activity(NULL);
      _setBackLight(blValue);
    }
  }

  void jsonTilesCommand(JsonVariant json)
  {
    int screenIdx = json["screen"].as<int>();
    if ((screenIdx < SCREEN_START) || (screenIdx > SCREEN_END))
    {
      wt32.print(F("[wpan] invalid screen: "));
      wt32.println(screenIdx);
      return;
    }

    int tileIdx = json["tile"].as<int>();
    if ((tileIdx < TILE_START) || (tileIdx > TILE_END))
    {
      wt32.print(F("[wpan] invalid tile: "));
      wt32.println(tileIdx);
      return;
    }

    classTile *tile = tileVault.get(screenIdx, tileIdx);
    if (!tile)
    {
      wt32.print(F("[wpan] screen/tile not found: "));
      wt32.print(screenIdx);
      wt32.print(F("/"));
      wt32.println(tileIdx);
      return;
    }

    if (json.containsKey("state"))
    {
      const char *state = json["state"];
      if (strcmp(state, "on") == 0)
      {
        tile->setState(true);
      }
      else if (strcmp(state, "off") == 0)
      {
        tile->setState(false);
      }
      else
      {
        wt32.print(F("[wpan] invalid state: "));
        wt32.println(state);
      }
    }

    if (json.containsKey("sublabel"))
    {
      tile->setSubLabel(json["sublabel"]);
    }

    if (json.containsKey("level"))
    {
      tile->setLevel(json["level"].as<int>(), false);
    }

    if (json.containsKey("color"))
    {
      int red = json["color"][0];
      int green = json["color"][1];
      int blue = json["color"][2];

      // if all zero reset to colorOn
      if ((red + green + blue) == 0)
      {
        tile->setColorToDefault();
      }
      else
      {
        tile->setColor(red, green, blue);
      }
    }

    if (json.containsKey("icon"))
    {
      tile->setIcon(iconVault.getIcon(json["icon"]));
    }

    if (json.containsKey("number") || json.containsKey("units"))
    {
      tile->setNumber(json["number"], json["units"]);
    }

    if (json.containsKey("text"))
    {
      tile->setIconText(json["text"]);
    }

    if (json.containsKey("image"))
    {
      tile->setBgImage(decodeBase64ToImg(json["image"]), json["zoom"], json["posOffset"][0], json["posOffset"][1]);
    }

    if (json.containsKey("dropDownList"))
    {
      tile->setDropDownList(json["dropDownList"]);
    }

    if (json.containsKey("dropDownSelect"))
    {
      tile->setDropDownIndex(json["dropDownSelect"].as<uint>());
    }

    if (json.containsKey("dropDownLabel"))
    {
      tile->setDropDownLabel(json["dropDownLabel"]);
    }

    if (json.containsKey("selectorList"))
    {
      tile->setSelectorList(json["selectorList"]);
    }

    if (json.containsKey("selectorSelect"))
    {
      tile->setSelectorIndex(json["selectorSelect"].as<uint>());
    }
  }

  void jsonSetLockStateCommand(const char *lockState)
  {
    // early exit if no valid keypad exist
    if (!lv_obj_is_valid(keyPad.ovlPanel))
      return;

    if (strcmp(lockState, "locked") == 0)
    {
      keyPad.setLockState(true);
    }
    if (strcmp(lockState, "unlocked") == 0)
    {
      keyPad.setLockState(false);
    }
    if (strcmp(lockState, "failed") == 0)
    {
      keyPad.setFailed();
    }
    if (strcmp(lockState, "close") == 0)
    {
      keyPad.close();
    }
  }

  // add icon from bas64 coded .png image
  void jsonAddIcon(JsonVariant json)
  {
    // decode image into ps_ram
    // TODO :
    //    check if ps_alloc successful

    // check if named icon exist, if yes -> get descriptor
    lv_img_dsc_t *oldIcon = (lv_img_dsc_t *)iconVault.getIcon(json["name"]);

    // decode new icon
    lv_img_dsc_t *iconPng = decodeBase64ToImg(json["image"]);

    // add custom icon to iconVault (deletes possible existing one)
    string iconStr = json["name"];
    iconVault.add({iconStr, iconPng});

    // free ps_ram heap if named icon existed allready
    if (oldIcon)
    {
      free((void *)oldIcon->data);
      free(oldIcon);
    }

    // update configutation
    setConfigSchema();
  }

  void jsonCommand(JsonVariant json)
  {
    if (json.containsKey("backlight"))
    {
      jsonSetBackLightCommand(json["backlight"]);
    }

    if (json.containsKey("message"))
    {
      _showMsgBox(json["message"]["title"], json["message"]["text"]);
    }

    if (json.containsKey("tiles"))
    {
      for (JsonVariant states : json["tiles"].as<JsonArray>())
      {
        jsonTilesCommand(states);
      }
    }

    if (json.containsKey("keyPad"))
    {
      jsonSetLockStateCommand(json["keyPad"]["state"]);
    }

    if (json.containsKey("screens"))
    {
      int screenIdx = json["screens"]["load"].as<int>();

      wt32.print(F("[wpan] screen select: "));
      wt32.println(screenIdx);

      selectScreen(screenIdx);
    }

    if (json.containsKey("addIcon"))
    {
      jsonAddIcon(json["addIcon"]);
    }
  }

  /**
    init and start LVGL pages
   */

  // define the defaults for lvgl objects
  static lv_style_t style_my_btn;

  static void new_theme_apply_cb(lv_theme_t * th, lv_obj_t * obj)
  {
    LV_UNUSED(th);
    if ((lv_obj_check_type(obj, &lv_btn_class)) || (lv_obj_check_type(obj, &lv_imgbtn_class)))
    {
      lv_obj_add_style(obj, &style_my_btn, 0);
    }
  }
  static void new_theme_init_and_set(void)
  {
    /*Initialize the styles*/
    lv_style_init(&style_my_btn);
    lv_style_set_bg_color(&style_my_btn, lv_color_hex(0xffffff));
    lv_style_set_radius(&style_my_btn, 5);
    /*Initialize the new theme from the current theme*/
    lv_theme_t *th_act = lv_disp_get_theme(NULL);
    static lv_theme_t th_new;
    th_new = *th_act;
    /*Set the parent theme and the style apply callback for the new theme*/
    lv_theme_set_parent(&th_new, th_act);
    lv_theme_set_apply_cb(&th_new, new_theme_apply_cb);
    /*Assign the new theme to the current display*/
    lv_disp_set_theme(NULL, &th_new);
  }

  // initialize ui
  void ui_init(void)
  {
    new_theme_init_and_set();

    // HomeScreen
    createScreen(SCREEN_HOME);

    // setup Settings Screen as screen[SCREEN_SETTINGS]
    classScreen &ref = screenVault.add(SCREEN_SETTINGS, 0);
    screenSettings = classScreenSettings(ref.screen, imgAustin);
    screenSettings.addEventHandler(backLightSliderEventHandler);
    ref.createHomeButton(footerButtonEventHandler, imgHome);
    ref.adScreenEventHandler(screenEventHandler);
    ref.setLabel("Settings");
  }
  /**
    Setup
  */
  void setup()
  {
    // Start serial and let settle
    Serial.begin(SERIAL_BAUD_RATE);
    delay(1000);
    Serial.println(F("[wpan] starting up..."));

    // initialise the Tile_Style_LUT and Img_LUT for later use
    initStyleLut();
    initIconVault();

    // set up for backlight dimming (PWM)
    ledcSetup(BL_PWM_CHANNEL, BL_PWM_FREQ, BL_PWM_RESOLUTION);
    ledcAttachPin(TFT_BL, BL_PWM_CHANNEL);
    ledcWrite(BL_PWM_CHANNEL, 0);

    // start lvgl
    lv_init();
    lv_img_cache_set_size(10);
    String LVGL_Arduino = "Hello Arduino! ";
    LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
    Serial.println(LVGL_Arduino);
    Serial.println("I am LVGL_Arduino");
#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print); // register print function for debugging
#endif

  // start tft library
  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);

  // touch pad
  ft6336u.begin();
  pinMode(39, INPUT);

  // innitialize draw buffer
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 10);
  // Initialize the display
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  // settings for display driver
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  // Initialize the input device driver (touch panel)
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  myInputDevice = lv_indev_drv_register(&indev_drv);
  // set timings for LongPress ,RepeatTime and gesture detect
  indev_drv.long_press_time = 500;
  indev_drv.long_press_repeat_time = 200;
  indev_drv.gesture_limit = 40;

  // set colors to default, may be updated later from config handler
  defaultOnColorConfig(0, 0, 0);
  defaultThemeColorConfig(0, 0, 0);

  // show splash screen
  _setBackLightLED(20);
  lv_obj_t *img1 = lv_img_create(lv_scr_act());
  lv_img_set_src(img1, imgSplash);
  lv_obj_align(img1, LV_ALIGN_CENTER, 0, 0);
  lv_timer_handler();
  _actBackLight = 50;
  _setBackLightLED(_actBackLight);
  _retainedBackLight = _actBackLight;

  // start the screen to make sure everything is initialised
  ui_init();

  Serial.println("Setup done");

  // Start WT32 hardware
  wt32.begin(jsonConfig, jsonCommand);

  // Set up config/command schema (for self-discovery and adoption)
  setConfigSchema();

  // prepare info text
  updateInfoText();

  // show HomeScreen
  screenVault.show(SCREEN_HOME);
}

/**
  Main processing loop
*/
void loop()
{
  // Let WT32 hardware handle any events etc
  wt32.loop();
  updateConnectionStatus();
  checkNoAvtivity();

  // let the GUI do its work
  lv_timer_handler();

  delay(3);
}
