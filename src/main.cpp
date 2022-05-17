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
const void *imgAustin = &ui_img_austins_black_320x70_png;
const void *imgSplash = &splash;
const void *imgOnOff = &ios_onoff_60;
const void *imgSpeaker = &ios_speaker_60;

int _act_BackLight;
connectionState_t _connectionState = CONNECTED_NONE;
uint32_t _noActivityTimeOut = 0L;

#define DEFAULT_COLOR_ON_RED   91
#define DEFAULT_COLOR_ON_GREEN 190
#define DEFAULT_COLOR_ON_BLUE  91
lv_color_t colorOn;
lv_color_t colorBg;

/*--------------------------- Global Objects -----------------------------*/

// screenVault holds all screens
classScreenList screenVault = classScreenList();

// tileVault holds all tiles
classTileList tileVault = classTileList();

// the Settings Screen
classScreenSettings screenSettings = classScreenSettings();

// WT32 handler
OXRS_WT32 wt32;

/*--------------------------- screen / lvgl relevant  -----------------------------*/

// Change to your screen resolution
static const uint16_t screenWidth = 320;
static const uint16_t screenHeight = 480;

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

/*--------------------------- publish with MQTT  -----------------------------*/

// publish Tile Event
// {"screen":1, "tile":1, "type":"button", "event":"single" , "state":"on"}
void publishTileEvent(int screenIdx, int tileIdx, bool state)
{
  StaticJsonDocument<128> json;
  json["screen"] = screenIdx;
  json["tile"] = tileIdx;
  json["type"] = "button";
  json["event"] = "single";
  state == true ? json["state"] = "on" : json["state"] = "off";

  wt32.publishStatus(json.as<JsonVariant>());
}

// publish Level change Event
// {"screen":1, "tile":1, "type":"level", "event":"change" , "state":50}
void publishLevelEvent(int screenIdx, int tileIdx, const char *event, int value)
{
  StaticJsonDocument<128> json;
  json["screen"] = screenIdx;
  json["tile"] = tileIdx;
  json["type"] = "level";
  json["event"] = event;
  json["state"] = value;

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

// publish local Backlight change
//{"backlight:" 50}
void publishBackLightTelemetry(void)
{
  char payload[8];
  sprintf(payload, "%d", _act_BackLight);

  StaticJsonDocument<32> json;
  json["backlight"] = payload;
  wt32.publishTelemetry(json.as<JsonVariant>());
}

/*
   set backlight of LCD (val in % [0..100], sendTelemrtry = true -> publish tele/)
*/
void _setBackLight(int val, bool sendTelemetry)
{
  if (val > 100)
    val = 100;
  ledcWrite(BL_PWM_CHANNEL, 255 * val / 100);
  _act_BackLight = val;
  if (sendTelemetry)
    publishBackLightTelemetry();
}

// update the slider
void setBackLightSliderValue(int value)
{
  screenSettings.setSlider(value);
}

/*
   lcd interface
   transfer pixel data range to lcd
*/
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
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
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  typePoint ts;
  bool touched = ft6336u.readTouchPoint(&ts);

  // no touch detected
  if (!touched)
  {
    data->state = LV_INDEV_STATE_REL;
    return;
  }
  // touch at low backlight
  if (_act_BackLight == 0)
  {
    _setBackLight(50, true);
    setBackLightSliderValue(50);
    data->state = LV_INDEV_STATE_REL;
    delay(200);
    return;
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
  if (_noActivityTimeOut == 0)
    return;
  // Screen is HomeScreen
  if (lv_scr_act() == screenVault.get(SCREEN_HOME)->screen)
    return;
  // time elapsed, jump to HomeScreen
  if (lv_disp_get_inactive_time(NULL) > _noActivityTimeOut)
    screenVault.show(SCREEN_HOME);
}

/*
 * ui helper functions
 */


void _showMsgBox(const char *title, const char *text)
{
  lv_obj_t *mbox1 = lv_msgbox_create(NULL, title, text, NULL, true);

  lv_obj_t *cbtn = lv_msgbox_get_close_btn(mbox1);
  lv_obj_set_style_bg_color(cbtn, lv_color_make(128, 30, 0), 0);
  lv_obj_set_style_bg_opa(cbtn, 255, 0);

  lv_obj_center(mbox1);
}

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
  char buffer2[40];

  lv_obj_t *_infoTextArea = screenSettings.getInfoPanel();

  sprintf(buffer, "Name:\t%s\n", STRINGIFY(FW_NAME));
  lv_textarea_set_text(_infoTextArea, buffer);
  sprintf(buffer, "Maker:\t%s\n", STRINGIFY(FW_MAKER));
  lv_textarea_add_text(_infoTextArea, buffer);
  sprintf(buffer, "Version:\t%s\n", STRINGIFY(FW_VERSION));
  lv_textarea_add_text(_infoTextArea, buffer);
  lv_textarea_add_text(_infoTextArea, "\n");

  wt32.getMACAddressTxt(buffer2);
  sprintf(buffer, "MAC:\t%s\n", buffer2);
  lv_textarea_add_text(_infoTextArea, buffer);

  wt32.getIPAddressTxt(buffer2);
  sprintf(buffer, "IP:\t%s\n", buffer2);
  lv_textarea_add_text(_infoTextArea, buffer);

  wt32.getMQTTTopicTxt(buffer2);
  sprintf(buffer, "MQTT:\t%s\n", buffer2);
  lv_textarea_add_text(_infoTextArea, buffer);
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

// WipeEvent Handler
static void wipeEventHandler(lv_event_t *e)
{
  lv_dir_t dir = lv_indev_get_gesture_dir(myInputDevice);
 
  switch (dir)
  {
  case LV_DIR_LEFT:
    screenVault.showNext(lv_scr_act());
    break;
  case LV_DIR_RIGHT:
    screenVault.showPrev(lv_scr_act());
    break;
  }
}

// screen event handler
// detects unload and load
void screenEventHandler(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_SCREEN_UNLOAD_START)
  {
    classScreen *sPtr = (classScreen *)lv_event_get_user_data(e);
    int screenIdx = sPtr->screenIdx;
    printf("Screen UNLOAD Event received: Screen : %d\n", screenIdx);
    publishScreenEvent(screenIdx, "unloaded");
  }
  if (code == LV_EVENT_SCREEN_LOADED)
  {
    classScreen *sPtr = (classScreen *)lv_event_get_user_data(e);
    int screenIdx = sPtr->screenIdx;
    printf("Screen LOAD Event received: Screen : %d\n", screenIdx);
    publishScreenEvent(screenIdx, "loaded");
  }
}

// Up / Down Button Event Handler
static void upDownEventHandler(lv_event_t *e, int direction)
{
  lv_event_code_t code = lv_event_get_code(e);
  if ((code == LV_EVENT_SHORT_CLICKED) || (code == LV_EVENT_LONG_PRESSED) || (code == LV_EVENT_LONG_PRESSED_REPEAT))
  {
    // short increments 1; long increments 5
    int inc = 0;
    if (code == LV_EVENT_SHORT_CLICKED) { inc = 1; }
    else                                { inc = 5; }
    inc *= direction;
    // get tile* of clicked tile from USER_DATA
    classTile *tPtr = (classTile *)lv_event_get_user_data(e);
    int level = tPtr->getLevel();
    // calc new value and limit to 0...100
    level += inc;
    if (level > 100) level = 100;
    if (level < 0)   level = 0;
    tPtr->setLevel(level);
    tPtr->showOvlBar(level);
    // send event
    tileId_t tileId = tPtr->getId();
//    printf("screen:%d  tile:%d  level:%d\n", tileId.idx.screen, tileId.idx.tile, level);
    publishLevelEvent(tileId.idx.screen, tileId.idx.tile, "change", level);
  }
}

// Up  Button Event Handler
static void upButtonEventHandler(lv_event_t *e)
{
  upDownEventHandler(e, +1);
}

// Down  Button Event Handler
static void downButtonEventHandler(lv_event_t *e)
{
  upDownEventHandler(e, -1);
}

// Tile Event Handler
static void tileEventHandler(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_SHORT_CLICKED)
  {
    // get tile* of clicked tile from USER_DATA
    classTile *tPtr = (classTile *)lv_event_get_user_data(e);
    tileId_t tileId = tPtr->getId();
    int screenIdx = tileId.idx.screen;
    int tileIdx = tileId.idx.tile;
    int linkedScreen = tPtr->getLink();
    bool state = tPtr->getState();
    printf("Click Event received: Screen : %d; Tile : %d; Link : %d; State : %d\n", screenIdx, tileIdx, linkedScreen, state);
    // button has link -> call linked screen
    if (linkedScreen > 0)
    {
      screenVault.show(linkedScreen);
    }
    //  publish click event
    else
    {
      publishTileEvent(screenIdx, tileIdx, state);
    }
  }

  if (code == LV_EVENT_LONG_PRESSED)
  {
    // use internal and call pop-up
  }
}

// screen footer button Event handler
//    HomeButton short  -> displays Home screen
//    SettingsButton    -> displays Settings
static void footerButtonEventHandler(lv_event_t *e)
{
  lv_event_code_t event = lv_event_get_code(e);
  lv_obj_t *ta = lv_event_get_target(e);
  if (event == LV_EVENT_SHORT_CLICKED)
  {
    // left side clicked -> HomeButton
    if (ta->coords.x1 < 160)
    {
      screenVault.show(SCREEN_HOME);
    }
    // right side clicked -> SettingsButton
    else
    {
      screenVault.show(SCREEN_SETTINGS);
    }
  }
}

// BackLight slider event handler
static void backLightSliderEventHandler(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *slider = lv_event_get_target(e);

  if (code == LV_EVENT_VALUE_CHANGED)
  {
    classScreenSettings *sPtr = (classScreenSettings *)lv_event_get_user_data(e);
    sPtr->setSlider((int)lv_slider_get_value(slider));
    _setBackLight(lv_slider_get_value(slider), false);
  }
  if (code == LV_EVENT_RELEASED)
  {
    _setBackLight(lv_slider_get_value(slider), true);
  }
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
  ref.adWipeEventHandler(wipeEventHandler);
  ref.adScreenEventHandler(screenEventHandler);
  // sort screenIdx in ascending order
  screenVault.sort();
}

/**
  Config handler
 */

const void *getIconFromType(int tileType)
{
  const void *img = NULL;
  switch (tileType)
  {
  case BLIND:
    img = imgBlind;
    break;
  case COFFEE:
    img = imgCoffee;
    break;
  case DOOR:
    img = imgDoor;
    break;
  case LIGHT:
    img = imgBulb;
    break;
  case NUMBER:
    img = NULL;
    break;
  case ONOFF:
    img = imgOnOff;
    break;
  case ROOM:
    img = imgRoom;
    break;
  case SETTINGS:
    img = imgSettings;
    break;
  case SPEAKER:
    img = imgSpeaker;
    break;
  case THERMOMETER:
    img = imgThermo;
    break;
  case WINDOW:
    img = imgWindow;
    break;
  }
  return img;
}

/*
 * Create any tile on any screen
 */
void createTile(int tileType, int screenIdx, int tileIdx, const char *label, bool noClick, int linkedScreen, bool enOnTileLevelControl)
{
  const void *img;
  // exit if screen or tile out of range
  if ((screenIdx < SCREEN_START) || (screenIdx > SCREEN_END) || (tileIdx < TILE_START) || (tileIdx > TILE_END))
  {
    printf("Config error. screen or tile out of range\n");
    return;
  }
  // TODO check if entry to be deleted
  // Deletion of tiles via config not defined yet, requires reboot
  if (tileType == NONE)
  {
  };

  // create screen if not exist
  createScreen(screenIdx);

  // delete icon reference if exist
  tileVault.remove(screenIdx, tileIdx);

  // set icon tileType dependent
  img = getIconFromType(tileType);

  // create new Tile
  classTile &ref = tileVault.add();
  ref.begin(screenVault.get(screenIdx)->container, img, label);
  ref.registerTile(screenIdx, tileIdx, tileType);

  // handle icons depending on tileType capabilities
  if (linkedScreen)
  {
    ref.setLink(linkedScreen);
    // create screen if not exist
    createScreen(linkedScreen);
  }

  // set the event handler
  if (linkedScreen || !noClick)
  {
    ref.addEventHandler(tileEventHandler);
  }

  // enable on-tile level control
  if (enOnTileLevelControl)
  {
    ref.addLevelControl(downButtonEventHandler, upButtonEventHandler);
}
}

// type list for config
void createInputTypeEnum(JsonObject parent)
{
  JsonArray typeEnum = parent.createNestedArray("enum");

  typeEnum.add("blind");
  typeEnum.add("coffee");
  typeEnum.add("door");
  typeEnum.add("light");
  typeEnum.add("number");
  typeEnum.add("onoff");
  typeEnum.add("room");
  typeEnum.add("speaker");
  typeEnum.add("thermometer");
  typeEnum.add("window");
}

// decode type from input
int parseInputType(const char *inputType)
{
  if (strcmp(inputType, "blind") == 0)        { return BLIND; }
  if (strcmp(inputType, "coffee") == 0)       { return COFFEE; }
  if (strcmp(inputType, "door") == 0)         { return DOOR; }
  if (strcmp(inputType, "light") == 0)        { return LIGHT; }
  if (strcmp(inputType, "number") == 0)       { return NUMBER; };
  if (strcmp(inputType, "onoff") == 0)        { return ONOFF; };
  if (strcmp(inputType, "room") == 0)         { return ROOM; }
  if (strcmp(inputType, "speaker") == 0)      { return SPEAKER; }
  if (strcmp(inputType, "thermometer") == 0)  { return THERMOMETER; }
  if (strcmp(inputType, "window") == 0)       { return WINDOW; }

  return NONE;
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
    sPtr->updateBgColor();
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

  createTile(parseInputType(json["type"]), screenIdx, tileIdx, json["label"], json["noClick"], json["link"], json["enOnTileLevelControl"]);
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

  if (json.containsKey("noActivitySeconds"))
  {
    _noActivityTimeOut = json["noActivitySeconds"].as<int>() * 1000;
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
  screens["title"] = "screens Configuration";
  screens["description"] = "Add Screen(s). Screen 1 is the HomeScreen and needs to be configured!";
  screens["type"] = "array";

  JsonObject items2 = screens.createNestedObject("items");
  items2["type"] = "object";

  JsonObject properties2 = items2.createNestedObject("properties");

  JsonObject screen = properties2.createNestedObject("screen");
  screen["title"] = "screen";
  screen["type"] = "integer";
  screen["minimum"] = SCREEN_START;
  screen["maximum"] = SCREEN_END;

  JsonObject label2 = properties2.createNestedObject("label");
  label2["title"] = "Label";
  label2["type"] = "string";

  // tiles on screen
  JsonObject tiles = properties2.createNestedObject("tiles");
  tiles["title"] = "screen Configuration";
  tiles["description"] = "Add Tiles to screen.";
  tiles["type"] = "array";

  JsonObject items3 = tiles.createNestedObject("items");
  items3["type"] = "object";

  JsonObject properties3 = items3.createNestedObject("properties");

  JsonObject tile3 = properties3.createNestedObject("tile");
  tile3["title"] = "Tile";
  tile3["type"] = "integer";
  tile3["minimum"] = TILE_START;
  tile3["maximum"] = TILE_END;

  JsonObject type3 = properties3.createNestedObject("type");
  type3["title"] = "Type";
  createInputTypeEnum(type3);

  JsonObject label3 = properties3.createNestedObject("label");
  label3["title"] = "Label";
  label3["type"] = "string";

  JsonObject enOnTileLevelControl = properties3.createNestedObject("enOnTileLevelControl");
  enOnTileLevelControl["title"] = "Enable on-tile level control (up/down buttons).";
  enOnTileLevelControl["type"] = "boolean";

  JsonObject noClick = properties3.createNestedObject("noClick");
  noClick["title"] = "Disable Tile clicks";
  noClick["type"] = "boolean";

  JsonObject link = properties3.createNestedObject("link");
  link["title"] = "Optional, select screen number if Tile links to new Screen.";
  link["type"] = "integer";
  link["minimum"] = SCREEN_START;
  link["maximum"] = SCREEN_END;

  JsonArray required3 = items3.createNestedArray("required");
  required3.add("tile");
  required3.add("type");

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
  JsonObject noActivitySeconds = json.createNestedObject("noActivitySeconds");
  noActivitySeconds["title"] = "Return to HomeScreen after Timeout (seconds) of no activity";
  noActivitySeconds["description"] = "Display shows HomeScreen after Timeout (seconds) of no activity. 0 disables.";
  noActivitySeconds["type"] = "integer";
  noActivitySeconds["minimum"] = 0;
  noActivitySeconds["maximum"] = 600;
}

void setConfigSchema()
{
  // Define our config schema
  StaticJsonDocument<2048> json;
  JsonVariant config = json.as<JsonVariant>();

  screenConfigSchema(config);

  // Pass our config schema down to the WT32 library
  wt32.setConfigSchema(config);
}

/**
  Command handler
 */

void jsonSetStateCommand(JsonVariant json)
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
    const char * state = json["state"];
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
    tile->setLevel(json["level"].as<int>());
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
  
  if (json.containsKey("number") || json.containsKey("units"))
  {
    tile->setNumber(json["number"], json["units"]);
  }
}

void jsonCommand(JsonVariant json)
{
  if (json.containsKey("backlight"))
  {
    int blValue = json["backlight"]["brightness"].as<int>();
    _setBackLight(blValue, true);
    setBackLightSliderValue(blValue);
  }

  if (json.containsKey("message"))
  {
    _showMsgBox(json["message"]["title"], json["message"]["text"]);
  }

  if (json.containsKey("setstate"))
  {
    for (JsonVariant states : json["setstate"].as<JsonArray>())
    {
      jsonSetStateCommand(states);
    }
  }

  if (json.containsKey("screen"))
  {
    int screenIdx = json["screen"]["select"].as<int>();

    wt32.print(F("[wpan] screen select: "));
    wt32.println(screenIdx);

    selectScreen(screenIdx);
  }
}

/**
  init and start LVGL pages
 */

// define the defaults for lvgl objects
static lv_style_t style_my_btn;

static void new_theme_apply_cb(lv_theme_t *th, lv_obj_t *obj)
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
  ref.adWipeEventHandler(wipeEventHandler);
  ref.adScreenEventHandler(screenEventHandler);
  ref.setLabel("Settings");

  updateInfoText();

  // show HomeScreen
  screenVault.show(SCREEN_HOME);
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

  // set up for backlight dimming (PWM)
  ledcSetup(BL_PWM_CHANNEL, BL_PWM_FREQ, BL_PWM_RESOLUTION);
  ledcAttachPin(TFT_BL, BL_PWM_CHANNEL);
  ledcWrite(BL_PWM_CHANNEL, 0);

  // start lvgl
  lv_init();
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
  // set timings for LongPress ,RepeatTime and swipe detect
  indev_drv.long_press_time = 500;
  indev_drv.long_press_repeat_time = 200;
  indev_drv.gesture_limit = 40;

  // set colors to default, may be updated later from config handler
  defaultOnColorConfig(0, 0, 0);
  defaultThemeColorConfig(0, 0, 0);

  // show splash screen
  lv_obj_t *img1 = lv_img_create(lv_scr_act());
  lv_img_set_src(img1, imgSplash);
  lv_obj_align(img1, LV_ALIGN_CENTER, 0, 0);
  lv_timer_handler();
  _setBackLight(50, false);

  // start the screen to make sure everything is initialised
  ui_init();

  Serial.println("Setup done");

  // Start WT32 hardware
  wt32.begin(jsonConfig, jsonCommand);

  // Set up config/command schema (for self-discovery and adoption)
  setConfigSchema();
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
