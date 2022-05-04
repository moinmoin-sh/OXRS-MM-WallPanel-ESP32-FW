#pragma once

enum tileType_t
{
  NONE,
  BLIND,
  COFFEE,
  DOOR,
  LIGHT,
  NUMBER,
  ONOFF,
  ROOM,
  SETTINGS,
  THERMOMETER,
  WINDOW
};

#define WP_OPA_BG_OFF 26
#define WP_OPA_BG_ON 255
#define WP_OPA_BG_PRESSED 128

#define SCREEN_ALL_MAX 9
#define SCREEN_GRID_MAX 8
#define SCREEN_HOME 0
#define SCREEN_SETTINGS (SCREEN_ALL_MAX - 1)
