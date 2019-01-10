#include "autoconf.h"
#include "api/string.h"
#include "api/print.h"
#include "api/types.h"
#include "api/syscall.h"
/* touchscreen and tft driver API */
#include "libtft.h"
#include "libtouch.h"
/* local utilities (tft, pin) */
#include "rtc.h"
#include "api/gui_pin.h"
#include "api/libgui.h"

/* tile height is defined as 100 pixels */
#define TILE_HEIGHT 80
#define MAX_MENUS  10
#define MAX_TILES  32

/* tft specific */
extern const int font_width;
extern const int font_height;
extern const int font_blankskip;

uint16_t screen_width = 0;
uint16_t screen_height = 0;

static volatile menu_desc_t current_menu = 0;
static volatile menu_desc_t current_menu_tmp = 0;
static volatile menu_desc_t default_menu = 0;

cb_external_events external_events_cb = 0;

static bool touch_locked = false;

void gui_init(uint16_t width, uint16_t height, cb_external_events ext_events_cb)
{
    screen_width = width;
    screen_height = height;
    external_events_cb = ext_events_cb;

}

/* menu and tile descriptors start at 1. 0 permit to detect empty fields */
menu_desc_t current_menu_id = 1;
tile_desc_t current_tile_id = 1;

typedef enum {
    TILE_EMPTY,
    TILE_REGISTERED
} tile_state_t;

typedef struct {
    tile_state_t   state;
    tile_desc_t    id;
    menu_desc_t    menu;
    tile_colormap_t colormap[2];
    tile_width_t   width;
    tile_height_t  height;
    tile_action_t  action;
    tile_icon_t    icon;
    const char *   text;
    int            x_start;
    int            y_start;
    int            x_end;
    int            y_end;
} tile_t;

typedef struct {
    char        title[16];
    int         lastx;
    int         lasty;
} menu_spec_t;

/* list of menus */
static menu_spec_t      menu_list[MAX_MENUS] = { 0 };
/* list of tiles */
static tile_t tile_list[MAX_TILES] = { 0 };

/*
 * is the tile touched ?
 */
static bool gui_tile_touched(tile_t* tile, int x, int y)
{
    if (tile->menu == current_menu) {
        if (x > tile->x_start && x < tile->x_end &&
                y > tile->y_start && y < tile->y_end) {
            return true;
        }
    }
    return false;
}

/*
 * Execute the given tile action
 */
static void gui_execute_tile(tile_t* tile)
{
    switch (tile->action.type) {
        case TILE_ACTION_MENU:
            current_menu_tmp = tile->action.target.menu;
            break;
        case TILE_ACTION_CB:
            tile->action.target.callback(tile->id);
            break;
        case TILE_ACTION_NONE:
            break;
    }
}


/*
 * Handle the tile touch action
 */
static tile_desc_t gui_handle_tile(menu_desc_t menu, int x, int y)
{
    printf("handling tiles...\n");
    for (uint8_t i = 1; i < current_tile_id; ++i) {
        if (tile_list[i].menu == menu) {
            if (gui_tile_touched(&tile_list[i], x, y)) {
                printf("found tile %d\n", i);
                gui_execute_tile(&tile_list[i]);
                return tile_list[i].id;
            }
        }
    }
    return 0;
}

/*
 * is the given menu descriptor exists ?
 */
static bool gui_menu_exists(menu_desc_t menu)
{
    if (menu > 0 && menu < current_menu_id) {
        return true;
    }
    return false;
}

/*
 * Declare the default (bootup) menu
 */
gui_error_t gui_declare_default_menu(menu_desc_t menu)
{
    if (!gui_menu_exists(menu)) {
        goto err_inval;
    }
    default_menu = menu;
    current_menu = menu;
    current_menu_tmp = menu;
    return GUI_ERR_NONE;

err_inval:
    return GUI_ERR_INVAL;
}


/*
 * Declare a new menu
 */
gui_error_t gui_declare_menu(const char *name, menu_desc_t *menu_desc)
{
    /* sanitize */
    if (!name || !menu_desc) {
        return GUI_ERR_INVAL;
    }
    if (current_menu_id == MAX_MENUS - 1) {
        return GUI_ERR_FULL;
    }

    strncpy(menu_list[current_menu_id].title, name, 15);
    menu_list[current_menu_id].lastx = 0;
    menu_list[current_menu_id].lasty = 0;
    *menu_desc = current_menu_id;
    current_menu_id++;

    return GUI_ERR_NONE;
}

/*
 * Return the given tile width
 */
static uint16_t gui_get_tile_width(tile_t *tile)
{
    switch (tile->width) {
        case TILE_WIDTH_THIRD:
            return screen_width / 3;
        case TILE_WIDTH_HALF:
            return screen_width / 2;
        case TILE_WIDTH_FULL:
            return screen_width;
    }
    return 0;
}

/*
 * Return the given tile height
 */
static uint16_t gui_get_tile_height(tile_t *tile)
{
    switch (tile->height) {
        case TILE_HEIGHT_STD:
            return TILE_HEIGHT;
        case TILE_HEIGHT_DOUBLE:
            return TILE_HEIGHT * 2;
        case TILE_HEIGHT_TRIPLE:
            return TILE_HEIGHT * 3;
    }
    return 0;
}


/*
 * Declare a new tile
 */
gui_error_t gui_declare_tile(menu_desc_t        menu,
                              tile_colormap_t   *colormap,
                              tile_width_t       width,
                              tile_height_t      height,
                              tile_action_t     *action,
                              const char        *text,
                              tile_icon_t       *icon,
                              tile_desc_t       *tile_desc)
{
    /*sanitize */
    if (current_tile_id == MAX_TILES - 1) {
        return GUI_ERR_FULL;
    }
    if (!gui_menu_exists(menu)) {
        goto err_inval;
    }
    if (!colormap || !tile_desc||!action) {
        goto err_inval;
    }
    tile_t * tile = &tile_list[current_tile_id];

    tile->menu = menu;
    if (icon) {
        tile->icon.data = icon->data;
        tile->icon.size = icon->size;
    } else {
        tile->icon.data = 0;
        tile->icon.size = 0;
    }

    memcpy(&(tile->colormap[0]), colormap, sizeof(tile_colormap_t));
    memcpy(&(tile->colormap[1]), &colormap[1], sizeof(tile_colormap_t));

    tile->width = width;
    tile->height = height;

    if (tile->height != TILE_HEIGHT_STD && tile->width != TILE_WIDTH_FULL)
    {
        printf("[E] bigger height are supported only for full width tiles\n");
        goto err_inval;
    }

    tile->action.type = action->type;
    switch (action->type) {
        case TILE_ACTION_MENU:
            tile->action.target.menu = action->target.menu;
            break;
        case TILE_ACTION_CB:
            tile->action.target.callback = action->target.callback;
            break;
        case TILE_ACTION_NONE:
            break;
        default:
            goto err_inval;
            break;
    }
    /* just a reference here */
    tile->text = text;

    /* calculating the tile position, depending on the menu */
    /* FIXME: this does not support insuffisent space by now */
    if (menu_list[tile->menu].lastx + gui_get_tile_width(tile) > screen_width)
    {
        /* tile newline first */
        menu_list[tile->menu].lastx = 0;
        menu_list[tile->menu].lasty += TILE_HEIGHT;
    }
    tile->x_start = menu_list[tile->menu].lastx;
    tile->y_start = menu_list[tile->menu].lasty;
    tile->x_end   = menu_list[tile->menu].lastx + gui_get_tile_width(tile);
    switch (tile->height) {
        case TILE_HEIGHT_STD:
            tile->y_end   = menu_list[tile->menu].lasty + TILE_HEIGHT;
            break;
        case TILE_HEIGHT_DOUBLE:
            tile->y_end   = menu_list[tile->menu].lasty + 2*TILE_HEIGHT;
            break;
        case TILE_HEIGHT_TRIPLE:
            tile->y_end   = menu_list[tile->menu].lasty + 3*TILE_HEIGHT;
            break;
    }

    /* increment menu posx posxy for next tile */
    menu_list[tile->menu].lastx += gui_get_tile_width(tile);
    if (menu_list[tile->menu].lastx == screen_width) {
        menu_list[tile->menu].lastx = 0;
        menu_list[tile->menu].lasty += gui_get_tile_height(tile);
    }

    tile->state = TILE_REGISTERED;
    tile->id = current_tile_id;
    *tile_desc = current_tile_id;
    current_tile_id++;
    return GUI_ERR_NONE;

err_inval:
    return GUI_ERR_INVAL;
}

/*
 * Draw the menu initial background (black background)
 */
static void gui_draw_background(void)
{
    tft_fill_rectangle(0,screen_width,0,screen_height,0,0,0);
}



/*
 * Draw a tile
 */
static void gui_draw_tile(int x1, int x2, int y1, int y2,
        const char *text,
        tile_colormap_t  bg,
        tile_colormap_t  fg,
        const uint8_t *icon,
        const uint32_t icon_size)
{
    const int char_width = font_width/128;
    int posx;
    uint8_t iconvfill = 0;
  // create the box color
  tft_fill_rectangle(x1,x2,y1,y2,0,0,0);
  tft_fill_rectangle(x1+2,x2-2,y1+2,y2-2,bg.r,bg.g,bg.b);
  // add the box icon
  if (icon) {
      uint8_t icon_color[6];

      icon_color[0] = bg.r;
      icon_color[1] = bg.g;
      icon_color[2] = bg.b;
      icon_color[3] = fg.r;
      icon_color[4] = fg.g;
      icon_color[5] = fg.b;
      tft_rle_image(x1 + (((x2 - x1)-45)/2),
              y1 + (((y2 - y1)-45)/2), 45, 45,
              (uint8_t*)icon_color, icon, icon_size);
      iconvfill = 48;
  }
  // add the box title
  //
  if (text) {
      tft_setfg(fg.r,fg.g,fg.b);
      tft_setbg(bg.r,bg.g,bg.b);
      uint8_t chars_per_line;
      uint8_t numlines;
      int posy;
      // 1) calculate the number of char per line in the case
      chars_per_line=((x2 - 5 - x1 + 5)/char_width);
      // 2) calculate the number of lines
      numlines=(strlen(text)/chars_per_line) + 1;
      // 3) calculate the position of the first line depending on
      // the number of lines
      //  ... the middle line...
      if (!icon) {
          posy=(y2-y1-font_height/2)/2;
      } else {
          posy=(((y2 - y1)-45)/2 + iconvfill);
      }
      for (uint8_t i = 1; i < numlines; ++i) {
          // go up of 1 line
          posy -= font_height/2;
      }
      uint8_t missing_chars = strlen(text);
      uint8_t i = 0;
      char line[chars_per_line + 1];
      memset(line, 0x0, chars_per_line + 1);
      do {
          if (missing_chars > chars_per_line) {
              memcpy(line, &(text[i]), chars_per_line);
              posx=(x2-x1-chars_per_line*char_width)/2;
              tft_set_cursor_pos(x1+posx,posy+y1);
              tft_puts((char*)line);
              // print only a part of the string
              missing_chars -= chars_per_line;
              i += chars_per_line;
          } else {
              posx=(x2-x1-strlen(&(text[i]))*char_width)/2;
              tft_set_cursor_pos(x1+posx,posy+y1);
              tft_puts((char*)&(text[i]));
              missing_chars = 0;
          }
          posy += font_height/2;
      } while (missing_chars > 0);
#if 0
      posx=(x2-x1-strlen(text)*char_width)/2;
      tft_set_cursor_pos(x1+posx,(y1 + (((y2 - y1)-45)/2 + iconvfill)));
      tft_setfg(fg.r,fg.g,fg.b);
      tft_setbg(bg.r,bg.g,bg.b);
      tft_puts((char*)text);
#endif
  }
}


/*
 * Draw the gui
 */
static void draw_gui(void)
{
    gui_draw_background();
    for (uint8_t i = 0; i < current_tile_id; ++i) {
        tile_t * tile = &tile_list[i];
        if (tile->menu == current_menu) {
            gui_draw_tile(tile->x_start, tile->x_end, tile->y_start, tile->y_end,
                          tile->text, tile->colormap[0], tile->colormap[1], tile->icon.data, tile->icon.size);
        }
    }
}

/*
 * Gui events loop
 */
void gui_get_events(void)
{
    while(1)
    {
        draw_gui();
        if (!touch_locked) {
          touch_read_X_DFR();/* Ensures that PenIRQ is enabled */
        }
        /*
         * Between touch_read_X_DFR and touch_is_touched, we need to wait a little
         * or touch_is_touched() will return an invalid value
         */
        sys_sleep(10, SLEEP_MODE_INTERRUPTIBLE);

        /* Wait loop for touchscreen to be touched */
        while (!(touch_refresh_pos(),touch_is_touched())) {
            /* handling external events (IPC...) */
            if (external_events_cb) {
                external_events_cb();
            }
            if (!touch_locked) {
                touch_enable_exti();
            }
            sys_yield();
        }
        //Follow the motion on the screen
        while (touch_refresh_pos(),touch_is_touched())
        {
            int posx,posy;
            //touch_refresh_pos();
            posy=touch_getx();
            posx=touch_gety();

            gui_handle_tile(current_menu, posx, posy);
        }
        current_menu = current_menu_tmp;
    }
    return;
}

gui_error_t gui_set_menu(menu_desc_t menu)
{
    if (!gui_menu_exists(menu)) {

        printf("you try to set an unknown menu!\n");
        return GUI_ERR_INVAL;
    }
    current_menu = menu;
    current_menu_tmp = menu;
    draw_gui();
    return GUI_ERR_NONE;
}

void gui_lock_touch(void)
{
    touch_locked = true;
}

void gui_unlock_touch(void)
{
    touch_locked = false;
}