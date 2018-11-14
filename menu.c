#include "autoconf.h"
#include "api/string.h"
#include "api/print.h"
#include "api/types.h"
#include "api/syscall.h"
/* touchscreen and tft driver API */
#include "libtft.h"
#include "libtouch.h"
/* local utilities (tft, pin) */
#include "api/gui_menu.h"
#include "rtc.h"
#include "api/gui_pin.h"

// images
#include "img/smiley.h"
#include "img/settings.h"
#include "img/wipe.h"
#include "img/lock.h"
#include "img/unlock.h"
#include "img/state.h"
#include "img/pin.h"
#include "img/petpin.h"
#include "img/petname.h"
#include "img/return.h"
#include "img/massstorage.h"
#include "img/smartcard.h"

/* menu background colors (in RGB mode) */
#define MENU_STATUS_BG    53,  88, 157
#define MENU_SETTINGS_BG   0, 159, 155
#define MENU_WIPE_BG     231,  92,  76
#define MENU_LOCK_BG     141,  78, 159
#define MENU_STATE_BG     49, 173,  89



extern const int font_width;
extern const int font_height;
extern const int font_blankskip;

/**
 * List of menu screens
 */
typedef enum {
    MENU_MAIN     = 0,
    MENU_SETTINGS = 1,
    MENU_WIPE     = 2,
    MENU_STATUS   = 3,
    MENU_FAILURE  = 4
} t_current_menu;

/* Fail message management */
static const char *fail_msg[] =
{
    "Pet Pin",
    "Pet Name",
    "User Pin"
};
enum fail_id {
    FAIL_PETPIN,
    FAIL_PETNAME,
    FAIL_USERPIN
};
/* current number of remaining tries for token pin */
static uint32_t pin_remaining_tries = CONFIG_AUTH_TOKEN_MAX_PIN;

/* current menu */
static volatile t_current_menu menu = MENU_MAIN;

/* current fail message */
static uint8_t current_fail_msg = 0;

/* screen size (width and height) */
static int screen_width = 0;
static int screen_height = 0;

/* callbacks list, registered by the upper layer when caller menu_init() */
static cb_menu_callbacks_t menu_cb = { 0 };

/*
 * Initialize the menu configuration.
 */
uint8_t menu_init(uint32_t width, uint32_t height, cb_menu_callbacks_t *callbacks)
{
  if (!callbacks) {
      goto err;
  }
  /* set upper layer callbacks */
  menu_cb.handle_settings = callbacks->handle_settings;
  menu_cb.handle_auth     = callbacks->handle_auth;
  menu_cb.handle_pin_cmd  = callbacks->handle_pin_cmd;
  /* set screen size */
  screen_width = width;
  screen_height = height;
err:
      return 1;
}

/* draw a black background */
static void draw_background(void)
{
    tft_fill_rectangle(0,screen_width,0,screen_height,0,0,0);
}


void menu_update_remaining_tries(uint32_t val)
{
    pin_remaining_tries = val;
}


t_box get_box(int x, int y)
{
    t_box box = BOX_UNDEFINED;
    switch (menu) {
        case MENU_MAIN:
        {
            if (x > 0 && x < (screen_width / 2) && y > 0 && y < 200) {
                box = BOX_STATUS;
            }
            if (x > (screen_width / 2) && x < 240 && y > 0 && y < 100) {
                box = BOX_SETTINGS;
            }
            if (x > (screen_width / 2) && x < 240 && y > 100 && y < 200) {
                box = BOX_WIPE;
            }
            if (x > 0 && x < (screen_width / 3) && y > 200 && y < 300) {
                box = BOX_LOCK;
            }
            if (x > (screen_width / 3) && x < ((2*screen_width) / 3) && y > 200 && y < 300) {
                box = BOX_UNLOCK;
            }
            if (x > ((2*screen_width) / 3) && x < screen_width && y > 200 && y < 300) {
                box = BOX_STATE;
            }
            break;
        }
        case MENU_STATUS:
        {
            if (x > 0 && x < screen_width && y > (screen_height - 50) && y < screen_height) {
                box = BOX_RETURN;
            }
            break;
        }

        case MENU_SETTINGS:
        {
            if (x > 0 && x < 240 && y > 0 && y < 90) {
                box = BOX_SET_PETPIN;
            }
            if (x > 0 && x < 240 && y > 90 && y < 180) {
                box = BOX_SET_PETNAME;
            }
            if (x > 0 && x < 240 && y > 180 && y < 270) {
                box = BOX_SET_USERPIN;
            }
            if (x > 0 && x < screen_width && y > (screen_height - 50) && y < screen_height) {
                box = BOX_RETURN;
            }
            break;
        }
        case MENU_WIPE:
        {
            if (x > 0 && x < screen_width && y > 0 && y < 90) {
                box = BOX_WIPE_AUTHKEYS;
            }
            if (x > 0 && x < screen_width && y > 90 && y < 180) {
                box = BOX_WIPE_SMARTCARD;
            }
            if (x > 0 && x < screen_width && y > 180 && y < 270) {
                box = BOX_WIPE_STORAGE;
            }
            if (x > 0 && x < screen_width && y > (screen_height - 50) && y < screen_height) {
                box = BOX_RETURN;
            }
            break;
        }
        
        case MENU_FAILURE:
        {
            if (x > 95 && x < 145 && y > 240 && y < 290) {
                box = BOX_RETURN;
            }
            break;
        }

        default:
        {
            break;
        }
    }
    return box;
}

static void draw_menubox(int x1, int x2, int y1, int y2,
        char *c,
        uint8_t r, uint8_t g, uint8_t b,
        const uint8_t *icon_colormap,
        const uint8_t *icon,
        const uint32_t icon_size)
{
    const int char_width = font_width/128;
    int posx;
    uint8_t iconvfill = 0;
  // create the box color
  tft_fill_rectangle(x1,x2,y1,y2,0,0,0);
  tft_fill_rectangle(x1+2,x2-2,y1+2,y2-2,r,g,b);
  // add the box icon
  if (icon) {
      tft_rle_image(x1 + (((x2 - x1)-45)/2),
              y1 + (((y2 - y1)-45)/2), 45, 45,
              icon_colormap, icon, icon_size);
      iconvfill = 48;
  }
  // add the box title
  //
  if (c) {
      posx=(x2-x1-strlen(c)*char_width)/2;
      tft_set_cursor_pos(x1+posx,(y1 + (((y2 - y1)-45)/2 + iconvfill)));
      tft_setfg(255,255,255);
      tft_setbg(r,g,b);
      tft_puts(c);
  }
}


void draw_menu(void)
{
    const int char_width = font_width/128;
    int cury;
  
    draw_background();
    switch (menu) {
        case MENU_MAIN:
        {
            draw_menubox(0,(screen_width/2),0,200,
                    "status\0",
                    MENU_STATUS_BG,
                    smiley_colormap,
                    smiley,
                    sizeof(smiley));
            draw_menubox((screen_width/2),screen_width,0,100,
                    "setting\0",
                    MENU_SETTINGS_BG,
                    settings_colormap,
                    settings,
                    sizeof(settings));
            draw_menubox((screen_width/2),screen_width,100,200,
                    "wipe\0",
                    MENU_WIPE_BG,
                    wipe_colormap,
                    wipe,
                    sizeof(wipe));
            draw_menubox(0,(screen_width/3),200,300,
                    "lck\0",
                    MENU_LOCK_BG,
                    nlock_colormap,
                    nlock,
                    sizeof(nlock));
            draw_menubox((screen_width/3),(2*screen_width/3),200,300,
                    "unlck\0",
                    MENU_LOCK_BG,
                    unlock_colormap,
                    unlock,
                    sizeof(unlock));
            draw_menubox((2*screen_width/3),screen_width,200,300,
                    "state\0",
                    MENU_STATE_BG,
                    state_colormap,
                    state,
                    sizeof(state));
            break;
        }
        case MENU_STATUS:
        {
            cury = 0;
            // print status information
            tft_fill_rectangle(0,screen_width,0,(screen_height - 50),53,88,157);
            tft_setfg(255,255,255);
            tft_setbg(53,88,157);
            tft_set_cursor_pos(0,cury);
            tft_puts("crypto:");
            cury += font_height/2;
#if CONFIG_AES256_CBC_ESSIV
            tft_set_cursor_pos(screen_width - (14*char_width),cury);
            tft_puts("AES_CBC_ESSIV");
#else
            tft_set_cursor_pos(screen_width - (5*char_width),cury);
            tft_puts("3DES");
#endif
            cury += font_height/2;
            tft_set_cursor_pos(0,cury);
            tft_puts("Update:");

            cury += font_height/2;

            tft_set_cursor_pos(40,cury);
            tft_puts("DFU");
            tft_set_cursor_pos(screen_width - (4*char_width),cury);
#if CONFIG_FIRMWARE_DFU
            tft_puts(" on");
#else
            tft_puts("off");
#endif

            cury += font_height/2;

            tft_set_cursor_pos(40,cury);
            tft_puts("DB");
            tft_set_cursor_pos(screen_width - (4*char_width),cury);
#if CONFIG_FIRMWARE_MODE_DUAL_BANK
            tft_puts(" on");
#else
            tft_puts("off");
#endif

            cury += font_height/2;

            tft_set_cursor_pos(0,cury);
            tft_puts("FW version");

            cury += font_height/2;

            tft_set_cursor_pos(screen_width - (6*char_width),cury);
            tft_puts("0.0.1");

            cury += font_height/2;

            tft_set_cursor_pos(0,cury);
            tft_puts("Pin tries:");
            tft_set_cursor_pos(screen_width - (2*char_width),cury);
            tft_putc('0'+pin_remaining_tries);

            // return button
            draw_menubox(0,screen_width,screen_height-50,screen_height,
                    0,
                    133,135,132,
                    returning_colormap,
                    returning,
                    sizeof(returning));
            break;

        }

        case MENU_FAILURE:
        {
            cury = 20;
            // print status information
            tft_fill_rectangle(0,screen_width,0,screen_height,255,255,255);
            tft_setfg(190,0,0);
            tft_setbg(255,255,255);
            tft_set_cursor_pos((screen_width/2) - (4*char_width),cury);
            tft_puts("Failure!");
            cury += font_height;
            tft_set_cursor_pos((screen_width/2) - (4*char_width),cury);
            tft_puts("Invalid ");
            cury += font_height/2;
            tft_set_cursor_pos((screen_width/2) - ((strlen(fail_msg[current_fail_msg])/2)*char_width),cury);
            tft_puts((char*)fail_msg[current_fail_msg]);
#
            draw_menubox(95,145,240,290,
                    0,
                    133,135,132,
                    returning_colormap,
                    returning,
                    sizeof(returning));
            break;
        }


        case MENU_SETTINGS:
        {
            draw_menubox(0,screen_width,0,90,
                    "set pet pin\0",
                    0,159,155,
                    petpin_colormap,
                    petpin,
                    sizeof(petpin));
            draw_menubox(0,screen_width,90,180,
                    "set pet name\0",
                    0,159,155,
                    petname_colormap,
                    petname,
                    sizeof(petname));
            draw_menubox(0,screen_width,180,270,
                    "set user pin\0",
                    0,159,155,
                    userpin_colormap,
                    userpin,
                    sizeof(userpin));
            draw_menubox(0,screen_width,screen_height-50,screen_height,
                    0,
                    133,135,132,
                    returning_colormap,
                    returning,
                    sizeof(returning));
            break;

        }
        case MENU_WIPE:
        {
            draw_menubox(0,screen_width,0,90,
                    "Wipe auth keys\0",
                    231,92,76,
                    wipe_colormap,
                    wipe,
                    sizeof(wipe));
            draw_menubox(0,screen_width,90,180,
                    "Wipe smartcard\0",
                    231,92,76,
                    smartcard_colormap,
                    smartcard,
                    sizeof(smartcard));
            draw_menubox(0,screen_width,180,270,
                    "Wipe massstorage\0",
                    231,92,76,
                    massstorage_colormap,
                    massstorage,
                    sizeof(massstorage));
            draw_menubox(0,screen_width,screen_height-50,screen_height,
                    0,
                    133,135,132,
                    returning_colormap,
                    returning,
                    sizeof(returning));
            break;

        }
        default: {
            break;
        }
    }

}

void menu_get_events(void)
{
    const int char_width = font_width/128;

    t_current_menu nextmenu = menu;

    if (menu_cb.handle_settings == 0 ||
        menu_cb.handle_auth == 0 ||
        menu_cb.handle_pin_cmd == 0) {
        printf("callbacks not properly declared ! Leaving...\n");
        goto err;
    }
    while(1)
    {
        draw_menu();
        touch_read_X_DFR();/* Ensures that PenIRQ is enabled */
        /*
         * Between touch_read_X_DFR and touch_is_touched, we need to wait a little
         * or touch_is_touched() will return an invalid value
         */
        sys_sleep(10, SLEEP_MODE_INTERRUPTIBLE);

        /* Wait loop for touchscreen to be touched */
        while (!(touch_refresh_pos(),touch_is_touched())) {
            touch_enable_exti();
            if (menu == MENU_STATUS) {
                //printf("[not touched] main menu\n");
                uint64_t ts = 0;
                char *timestamp = 0;
                /* Here we sleep for 1 second in order to print the
                 * current timestamp on the menubar every second
                 * We can be interrupted by the touchscreen if needed
                 */
                sys_sleep(1000, SLEEP_MODE_INTERRUPTIBLE);
                //printf("[not touched] print timestamp\n");
                sys_get_systick(&ts, PREC_MILLI);
                timestamp = get_timestamp((uint32_t)(ts / 1000));
                tft_set_cursor_pos(120 - ((8*char_width)/2),240);
                tft_setfg(235,235,235);
                tft_setbg(53,88,157);
                tft_puts(timestamp);
            } else {
                sys_yield();
            }
        }
        //Follow the motion on the screen
        while (touch_refresh_pos(),touch_is_touched())
        {

            int posx,posy;
            //touch_refresh_pos();
            posy=touch_getx();
            posx=touch_gety();
            //printf("[touched] get pos: x:%d, y:%d\n", posx, posy);

            t_box box = get_box(posx, posy); 

            switch (box) {
                case BOX_SETTINGS:
                    {
#if MENU_DEBUG
                        printf("[touched] box settings pushed !\n");
#endif
                        nextmenu = MENU_SETTINGS;
                        break;
                    }
                case BOX_STATUS:
                    {
#if MENU_DEBUG
                        printf("[touched] box status pushed !\n");
#endif
                        nextmenu = MENU_STATUS;
                        break;
                    }

                case BOX_WIPE:
                    {
#if MENU_DEBUG
                        printf("[touched] box wipe pushed !\n");
#endif
                        nextmenu = MENU_WIPE;
                        break;
                    }
                case BOX_LOCK:
                    {
#if MENU_DEBUG
                        printf("[touched] box lock pushed !\n");
#endif
                        if (menu_cb.handle_settings) {
                            menu_cb.handle_settings(BOX_LOCK);
                        } 
                        break;
                    }
                case BOX_RETURN:
                    {
#if MENU_DEBUG
                        printf("[touched] box return pushed !\n");
#endif
                        nextmenu = MENU_MAIN;
                        break;
                    }
                case BOX_SET_PETPIN:
                    {
#if MENU_DEBUG
                        printf("[touched] box set petpin pushed !\n");
#endif
                        /* inform SMART that an authentication phase is requested */
                        menu_cb.handle_settings(BOX_SET_PETPIN);
                        /* handle the authentication phase with smart */
                        if (menu_cb.handle_auth(LITE_AUTHENTICATION_MODE)) {
                            printf("fail to handle authentication ! leaving...\n");
                            current_fail_msg = FAIL_USERPIN;
                            nextmenu = MENU_FAILURE;
                            continue;
                        }
                        if (menu_cb.handle_pin_cmd()) {
                            printf("fail to handle pin cmd request\n");
                            continue;
                        }
                        break;
                    }
                case BOX_SET_PETNAME:
                    {
#if MENU_DEBUG
                        printf("[touched] box set petname pushed !\n");
#endif
                        /* inform SMART that an authentication phase is requested */
                        menu_cb.handle_settings(BOX_SET_PETNAME);
                        /* handle the authentication phase with smart */
                        if (menu_cb.handle_auth(LITE_AUTHENTICATION_MODE)) {
                            printf("fail to handle authentication ! leaving...\n");
                            current_fail_msg = FAIL_USERPIN;
                            nextmenu = MENU_FAILURE;
                            continue;
                        }
                        if (menu_cb.handle_pin_cmd()) {
                            printf("fail to handle pin cmd request\n");
                            continue;
                        }
                        break;
                    }
                case BOX_SET_USERPIN:
                    {
#if MENU_DEBUG
                        printf("[touched] box set userpin pushed !\n");
#endif
                        menu_cb.handle_settings(BOX_SET_USERPIN);
                        /* handle the authentication phase with smart */
                        if (menu_cb.handle_auth(LITE_AUTHENTICATION_MODE)) {
                            printf("fail to handle authentication ! leaving...\n");
                            current_fail_msg = FAIL_USERPIN;
                            nextmenu = MENU_FAILURE;
                            continue;
                        }
                        if (menu_cb.handle_pin_cmd()) {
                            printf("fail to handle pin cmd request\n");
                            continue;
                        }
                        break;
                    }
                default:
                    {
                        break;
                    }
            }
        }
        menu = nextmenu;
    }
err:
    return;
}
