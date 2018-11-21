#ifndef MENU_H_
#define MENU_H_

#include "api/types.h"

#define MENU_DEBUG 0

enum authentication_mode {
  FULL_AUTHENTICATION_MODE,
  LITE_AUTHENTICATION_MODE
};

typedef enum {
    BOX_STATUS,
    BOX_SETTINGS,
    BOX_WIPE,
    BOX_LOCK,
    BOX_UNLOCK,
    BOX_STATE,
    BOX_DFU,
    BOX_SET_PETPIN,
    BOX_SET_PETNAME,
    BOX_SET_USERPIN,
    BOX_WIPE_AUTHKEYS,
    BOX_WIPE_SMARTCARD,
    BOX_WIPE_STORAGE,
    BOX_START_DFU,
    BOX_RETURN,
    BOX_UNDEFINED
} t_box;

/********************************************
 * Callbacks that need to be declared by the
 * upper layer. These callbacks are called by
 * the libwindow when a signal is received by
 * the touchscreen on a given configured box
 *******************************************/
typedef uint8_t (*cb_menu_handle_settings_req_t)(t_box signal);
typedef uint8_t (*cb_menu_handle_authentication_t)(enum authentication_mode authmode);
typedef uint8_t (*cb_menu_handle_pin_cmd_request_t)(void);

typedef struct {
    cb_menu_handle_settings_req_t       handle_settings;
    cb_menu_handle_authentication_t     handle_auth;
    cb_menu_handle_pin_cmd_request_t    handle_pin_cmd;
} cb_menu_callbacks_t;


uint8_t menu_init(uint32_t width, uint32_t height, cb_menu_callbacks_t *callbacks, bool fw_mode);

void menu_get_events(void);

void menu_update_remaining_tries(uint32_t val);

#endif
