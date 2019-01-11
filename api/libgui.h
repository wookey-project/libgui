#ifndef MENU2_H_
#define MENU2_H_

/* up to 255 menus */
typedef uint8_t menu_desc_t;
/* up to 255 tiles */
typedef uint8_t tile_desc_t;

typedef enum {
    GUI_ERR_NONE,
    GUI_ERR_FULL,
    GUI_ERR_INVAL
} gui_error_t;

typedef void (*gui_callback_t)(tile_desc_t tile);

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} tile_colormap_t;

typedef enum {
    COLOR_BG = 0,
    COLOR_FG = 1
} colormap_field_t;

typedef enum {
    TILE_WIDTH_THIRD,
    TILE_WIDTH_HALF,
    TILE_WIDTH_FULL,
} tile_width_t;

typedef enum {
    TILE_HEIGHT_HALF,
    TILE_HEIGHT_STD,
    TILE_HEIGHT_DOUBLE,
    TILE_HEIGHT_TRIPLE
} tile_height_t;

typedef enum {
    TILE_ACTION_MENU,
    TILE_ACTION_CB,
    TILE_ACTION_NONE,
} tile_action_type_t;

typedef union {
    menu_desc_t     menu;
    gui_callback_t  callback;
} u_tile_action_target_t;

typedef struct {
    tile_action_type_t      type;
    u_tile_action_target_t  target;
} tile_action_t;

typedef struct {
    uint32_t size;
    const uint8_t *data;
} tile_icon_t;

typedef enum {
    TXT_ALIGN_LEFT,
    TXT_ALIGN_CENTER,
    TXT_ALIGN_RIGHT
} tile_text_align_t;

typedef struct {
    const char *      text;
    tile_text_align_t align;
} tile_text_t;

typedef void (*cb_external_events)(bool *refresh_gui_after);

void gui_init(uint16_t width, uint16_t height, cb_external_events external_events_cb);

gui_error_t gui_declare_menu(const char *name, menu_desc_t *menu_desc);

gui_error_t gui_declare_default_menu(menu_desc_t menu);

gui_error_t gui_set_menu(menu_desc_t menu);

gui_error_t gui_declare_tile(menu_desc_t        menu,
                              tile_colormap_t   *colormap,
                              tile_width_t       width,
                              tile_height_t      height,
                              tile_action_t     *action,
                              tile_text_t       *text,
                              tile_icon_t       *icon,
                              tile_desc_t       *tile_desc);

/*
 * synchronous and immediate gui refresh
 */
void gui_force_refresh(void);

void gui_get_events(void);

void gui_lock_touch(void);

void gui_unlock_touch(void);

#endif/*!MENU2_H_*/
