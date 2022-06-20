#pragma once
#include <Arduino.h>

enum tileType_t
{
  NONE,
  BLIND,
  CEILINGFAN,
  COFFEE,
  DOOR,
  DROPDOWN,
  LIGHT,
  LOCK,
  MUSIC,
  NUMBER,
  ONOFF,
  PLAYER,
  REMOTE,
  ROOM,
  SETTINGS,
  SPEAKER,
  TEXT,
  THERMOMETER,
  WINDOW,
  THREEDPRINT
};

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
