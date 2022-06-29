#pragma once
#include <Arduino.h>
#include <string>
using namespace std;

enum tileType_t
{
  TT_NONE,
  TT_BUTTON,
  TT_BUTTON_LEVEL_UP,
  TT_BUTTON_LEVEL_DOWN,
  TT_INDICATOR,
  TT_COLOR_PICKER,
  TT_DROPDOWN,
  TT_KEYPAD,
  TT_KEYPAD_BLOCKING,
  TT_REMOTE,
  TT_LINK,
  TT_MEDIAPLAYER,
  TT_TYPE_COUNT // must be the last element
};

// entry for the tile_type_LUT
typedef struct
{
  int type;
  const char *typeStr;
  const void *img;
} tileTypeLutEntry_t;

typedef struct
{
  string iconStr;
  const void *img;
} imgListElement_t;

#define WP_OPA_BG_OFF     26
#define WP_OPA_BG_ON      255
#define WP_OPA_BG_PRESSED 128

#define SCREEN_WIDTH      320
#define SCREEN_HEIGHT     480

#define SCREEN_START      1
#define SCREEN_END        8
#define SCREEN_HOME       SCREEN_START
#define SCREEN_SETTINGS   (SCREEN_END + 1)

#define TILE_START        1
#define TILE_END          6

typedef union
{
  struct
  {
    uint8_t tile;
    uint8_t screen;
  } idx;
  uint16_t id;
} tileId_t;
