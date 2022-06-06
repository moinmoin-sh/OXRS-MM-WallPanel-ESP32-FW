#include "classRemote.h"
#include "globalDefines.h"

extern lv_color_t colorOn;
extern lv_color_t colorBg;
extern const void* imgBack;
extern "C" const lv_img_dsc_t KodiStyleBlack_png;

void classRemote::_createRemote(void)
{

    // full screen overlay / semi transparent
    ovlPanel = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(ovlPanel);
    lv_obj_set_size(ovlPanel, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_align(ovlPanel, LV_ALIGN_TOP_MID);
    lv_obj_clear_flag(ovlPanel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ovlPanel, colorBg, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ovlPanel, 150, LV_PART_MAIN | LV_STATE_DEFAULT);

    // active upper part / colorBg
    _ovlPanel2 = lv_obj_create(ovlPanel);
    lv_obj_remove_style_all(_ovlPanel2);
    lv_obj_set_size(_ovlPanel2, SCREEN_WIDTH, SCREEN_HEIGHT - 35);
    lv_obj_set_align(_ovlPanel2, LV_ALIGN_TOP_MID);
    lv_obj_clear_flag(_ovlPanel2, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(_ovlPanel2, colorBg, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(_ovlPanel2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    // back button closes overlay
    _btnExit = lv_btn_create(_ovlPanel2);
    lv_obj_set_size(_btnExit, 80, 50);
    lv_obj_align(_btnExit, LV_ALIGN_BOTTOM_LEFT, 5, -5);
    lv_obj_set_style_bg_color(_btnExit, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(_btnExit, WP_OPA_BG_OFF, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *label = lv_label_create(_btnExit);
    lv_label_set_text(label, LV_SYMBOL_LEFT);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    lv_obj_add_event_cb(_btnExit, closeEvt, LV_EVENT_CLICKED, this);
  
    // panel for active controls
    lv_obj_t *_panel = lv_obj_create(_ovlPanel2);
    lv_obj_remove_style_all(_panel);

    lv_obj_set_size(_panel, 310, 310);
    lv_obj_align(_panel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(_panel, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(_panel, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(_panel, WP_OPA_BG_OFF, LV_PART_MAIN | LV_STATE_DEFAULT);

    // image is rendered online from .png
    lv_obj_t* img = lv_img_create(_panel);
    lv_img_set_src(img, &KodiStyleBlack_png);
    lv_obj_align(img, LV_ALIGN_CENTER, 00, 0);

    _btnUp = lv_btn_create(_panel);
    lv_obj_set_size(_btnUp, 100, 100);
    lv_obj_set_style_bg_opa(_btnUp, 0, LV_STATE_DEFAULT);
    lv_obj_align(_btnUp, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_clear_flag(_btnUp, LV_OBJ_FLAG_PRESS_LOCK);
    lv_obj_add_flag(_btnUp, LV_OBJ_FLAG_USER_1);

    _btnDown = lv_btn_create(_panel);
    lv_obj_set_size(_btnDown, 100, 100);
    lv_obj_set_style_bg_opa(_btnDown, 0, LV_STATE_DEFAULT);
    lv_obj_align(_btnDown, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(_btnDown, LV_OBJ_FLAG_USER_2);

    _btnLeft = lv_btn_create(_panel);
    lv_obj_set_size(_btnLeft, 100, 100);
    lv_obj_set_style_bg_opa(_btnLeft, 0, LV_STATE_DEFAULT);
    lv_obj_align(_btnLeft, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_add_flag(_btnLeft, LV_OBJ_FLAG_USER_1);
    lv_obj_add_flag(_btnLeft, LV_OBJ_FLAG_USER_2);

    _btnRight = lv_btn_create(_panel);
    lv_obj_set_size(_btnRight, 100, 100);
    lv_obj_set_style_bg_opa(_btnRight, 0, LV_STATE_DEFAULT);
    lv_obj_align(_btnRight, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_clear_flag(_btnRight, LV_OBJ_FLAG_PRESS_LOCK);
    lv_obj_add_flag(_btnRight, LV_OBJ_FLAG_USER_3);

    _btnOk = lv_btn_create(_panel);
    lv_obj_set_size(_btnOk, 100, 100);
    lv_obj_set_style_bg_opa(_btnOk, 0, LV_STATE_DEFAULT);
    lv_obj_align(_btnOk, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(_btnOk, LV_OBJ_FLAG_USER_1);
    lv_obj_add_flag(_btnOk, LV_OBJ_FLAG_USER_3);

    _btnInfo = lv_btn_create(_panel);
    lv_obj_set_size(_btnInfo, 60, 60);
    lv_obj_align(_btnInfo, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_set_style_bg_opa(_btnInfo, 0, LV_STATE_DEFAULT);
    lv_obj_add_flag(_btnInfo, LV_OBJ_FLAG_USER_2);
    lv_obj_add_flag(_btnInfo, LV_OBJ_FLAG_USER_3);

    _btnList = lv_btn_create(_panel);
    lv_obj_set_size(_btnList, 60, 60);
    lv_obj_align(_btnList, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_set_style_bg_opa(_btnList, 0, LV_STATE_DEFAULT);
    lv_obj_add_flag(_btnList, LV_OBJ_FLAG_USER_1);
    lv_obj_add_flag(_btnList, LV_OBJ_FLAG_USER_2);
    lv_obj_add_flag(_btnList, LV_OBJ_FLAG_USER_3);

    _btnBack = lv_btn_create(_panel);
    lv_obj_set_size(_btnBack, 60, 60);
    lv_obj_set_style_bg_opa(_btnBack, 0, LV_STATE_DEFAULT);
    lv_obj_align(_btnBack, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_add_flag(_btnBack, LV_OBJ_FLAG_USER_4);

    _btnHome = lv_btn_create(_panel);
    lv_obj_set_size(_btnHome, 60, 60);
    lv_obj_set_style_bg_opa(_btnHome, 0, LV_STATE_DEFAULT);
    lv_obj_align(_btnHome, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_add_flag(_btnHome, LV_OBJ_FLAG_USER_1);
    lv_obj_add_flag(_btnHome, LV_OBJ_FLAG_USER_4);
}

classRemote::classRemote(classTile* tile, lv_event_cb_t remoteEventHandler)
{
    _createRemote();
    _callingTile = tile;

    lv_obj_add_event_cb(_btnUp, remoteEventHandler, LV_EVENT_ALL, _callingTile);
    lv_obj_add_event_cb(_btnDown, remoteEventHandler, LV_EVENT_ALL, _callingTile);
    lv_obj_add_event_cb(_btnLeft, remoteEventHandler, LV_EVENT_ALL, _callingTile);
    lv_obj_add_event_cb(_btnRight, remoteEventHandler, LV_EVENT_ALL, _callingTile);
    lv_obj_add_event_cb(_btnOk, remoteEventHandler, LV_EVENT_ALL, _callingTile);
    lv_obj_add_event_cb(_btnInfo, remoteEventHandler, LV_EVENT_ALL, _callingTile);
    lv_obj_add_event_cb(_btnList, remoteEventHandler, LV_EVENT_ALL, _callingTile);
    lv_obj_add_event_cb(_btnBack, remoteEventHandler, LV_EVENT_ALL, _callingTile);
    lv_obj_add_event_cb(_btnHome, remoteEventHandler, LV_EVENT_ALL, _callingTile);
}

void classRemote::closeEvt(lv_event_t* e)
{
    lv_obj_t* btn = lv_event_get_target(e);
    lv_obj_t* panel2 = lv_obj_get_parent(btn);
    lv_obj_t* panel1 = lv_obj_get_parent(panel2);
    lv_obj_del(panel1);
}

classTile* classRemote::getTile(void)
{
    return _callingTile;
}





