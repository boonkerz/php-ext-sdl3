#include <SDL3/SDL_stdinc.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_sdl3.h"
#include "helper.h"
#include "sdl3_image.h"
#include "sdl3_ttf.h"
#include "sdl3_events.h"
#include <SDL3/SDL.h>
#include <math.h>
#include <string.h>

#ifdef HAVE_LIBNOTIFY
#include <libnotify/notify.h>
#endif

// Cross-platform tray support (zserge/tray)
#ifdef _WIN32
#define TRAY_WINAPI
#elif defined(__APPLE__)
#define TRAY_APPKIT
#elif defined(HAVE_TRAY_APPINDICATOR)
#define TRAY_APPINDICATOR
#endif

#include "tray-lib/tray.h"

// Resource handles (nicht static, damit sie in anderen Modulen verfügbar sind)
int le_sdl_window;
int le_sdl_renderer;
int le_sdl_texture;

// Destructor for window resource
static void sdl_window_dtor(zend_resource *rsrc) {
    SDL_Window *win = (SDL_Window *)rsrc->ptr;
    if (win) {
        SDL_DestroyWindow(win);
    }
}

// Destructor for renderer resource
static void sdl_renderer_dtor(zend_resource *rsrc) {
    SDL_Renderer *ren = (SDL_Renderer *)rsrc->ptr;
    if (ren) {
        SDL_DestroyRenderer(ren);
    }
}

// Destructor for texture resource
static void sdl_texture_dtor(zend_resource *rsrc) {
    SDL_Texture *tex = (SDL_Texture *)rsrc->ptr;
    if (tex) {
        SDL_DestroyTexture(tex);
    }
}

// --- Tray integration state ---
static struct tray g_tray;
static struct tray_menu *g_tray_menu = NULL;
static int g_tray_menu_count = 0;
static int g_tray_last_index = -1;
static int g_tray_last_clicked = 0;

static void php_tray_menu_cb(struct tray_menu *m) {
    if (!m || m->context == NULL) {
        return;
    }
    intptr_t idx = (intptr_t)m->context;
    g_tray_last_index = (int)idx;
    g_tray_last_clicked = 1;

    // Signal the native tray loop to exit; tray_poll()
    // will then see a negative result and return false.
    //tray_exit();
}

PHP_MINIT_FUNCTION(sdl3) {
    le_sdl_window = zend_register_list_destructors_ex(sdl_window_dtor, NULL, "SDL_Window", module_number);
    le_sdl_renderer = zend_register_list_destructors_ex(sdl_renderer_dtor, NULL, "SDL_Renderer", module_number);
    le_sdl_texture = zend_register_list_destructors_ex(sdl_texture_dtor, NULL, "SDL_Texture", module_number);

    // Registriere SDL3_image und SDL3_ttf Resources
    sdl3_image_register_resources(module_number);
    sdl3_ttf_register_resources(module_number);

    // Registriere Event-Konstanten
    sdl3_events_register_constants(module_number);

    // SDL Init Flags
    REGISTER_LONG_CONSTANT("SDL_INIT_VIDEO", SDL_INIT_VIDEO, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_INIT_AUDIO", SDL_INIT_AUDIO, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_INIT_EVENTS", SDL_INIT_EVENTS, CONST_CS | CONST_PERSISTENT);

    // SDL Window Flags
    REGISTER_LONG_CONSTANT("SDL_WINDOW_FULLSCREEN", SDL_WINDOW_FULLSCREEN, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_WINDOW_OPENGL", SDL_WINDOW_OPENGL, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_WINDOW_HIDDEN", SDL_WINDOW_HIDDEN, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_WINDOW_BORDERLESS", SDL_WINDOW_BORDERLESS, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_WINDOW_RESIZABLE", SDL_WINDOW_RESIZABLE, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_WINDOW_MINIMIZED", SDL_WINDOW_MINIMIZED, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_WINDOW_MAXIMIZED", SDL_WINDOW_MAXIMIZED, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_WINDOW_HIGH_PIXEL_DENSITY", SDL_WINDOW_HIGH_PIXEL_DENSITY, CONST_CS | CONST_PERSISTENT);

    // SDL Pixel Formats
    REGISTER_LONG_CONSTANT("SDL_PIXELFORMAT_RGBA8888", SDL_PIXELFORMAT_RGBA8888, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_PIXELFORMAT_ARGB8888", SDL_PIXELFORMAT_ARGB8888, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_PIXELFORMAT_BGRA8888", SDL_PIXELFORMAT_BGRA8888, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_PIXELFORMAT_ABGR8888", SDL_PIXELFORMAT_ABGR8888, CONST_CS | CONST_PERSISTENT);

    // SDL Texture Access
    REGISTER_LONG_CONSTANT("SDL_TEXTUREACCESS_STATIC", SDL_TEXTUREACCESS_STATIC, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_TEXTUREACCESS_STREAMING", SDL_TEXTUREACCESS_STREAMING, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_TEXTUREACCESS_TARGET", SDL_TEXTUREACCESS_TARGET, CONST_CS | CONST_PERSISTENT);

    // SDL Blend Modes
    REGISTER_LONG_CONSTANT("SDL_BLENDMODE_NONE", SDL_BLENDMODE_NONE, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_BLENDMODE_BLEND", SDL_BLENDMODE_BLEND, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_BLENDMODE_ADD", SDL_BLENDMODE_ADD, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_BLENDMODE_MOD", SDL_BLENDMODE_MOD, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_BLENDMODE_MUL", SDL_BLENDMODE_MUL, CONST_CS | CONST_PERSISTENT);

    return SUCCESS;
}

PHP_FUNCTION(sdl_init) {
    zend_long flags;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &flags) == FAILURE) {
        RETURN_THROWS();
    }
    if (SDL_Init((Uint32)flags) < 0) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_FUNCTION(sdl_quit) {
    SDL_Quit();
}

PHP_FUNCTION(sdl_create_window) {
    char *title;
    size_t title_len;
    zend_long w, h, flags = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "sll|l", &title, &title_len, &w, &h, &flags) == FAILURE) {
        RETURN_THROWS();
    }

    SDL_Window *win = SDL_CreateWindow(title, (int)w, (int)h, (SDL_WindowFlags)flags);
    if (!win) {
        RETURN_FALSE;
    }
    RETURN_RES(zend_register_resource(win, le_sdl_window));
}

PHP_FUNCTION(sdl_destroy_window) {
    zval *win_res;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &win_res) == FAILURE) {
        RETURN_THROWS();
    }

    // This will call the destructor and free the resource
    zend_list_close(Z_RES_P(win_res));
    RETURN_TRUE;
}

PHP_FUNCTION(sdl_destroy_renderer) {
    zval *ren_res;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &ren_res) == FAILURE) {
        RETURN_THROWS();
    }

    // This will call the destructor and free the resource
    zend_list_close(Z_RES_P(ren_res));
    RETURN_TRUE;
}

PHP_FUNCTION(sdl_get_window_id) {
    zval *win_res;
    SDL_Window *win;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &win_res) == FAILURE) {
        RETURN_THROWS();
    }

    win = (SDL_Window *)zend_fetch_resource(Z_RES_P(win_res), "SDL_Window", le_sdl_window);
    if (!win) {
        RETURN_FALSE;
    }

    SDL_WindowID window_id = SDL_GetWindowID(win);
    RETURN_LONG(window_id);
}

PHP_FUNCTION(sdl_create_renderer) {
    zval *win_res;
    SDL_Window *win;
    char *renderer_name = NULL;
    size_t renderer_name_len = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r|s", &win_res, &renderer_name, &renderer_name_len) == FAILURE) {
        RETURN_THROWS();
    }

    win = (SDL_Window *)zend_fetch_resource(Z_RES_P(win_res), "SDL_Window", le_sdl_window);
    if (!win) {
        RETURN_FALSE;
    }

    SDL_Renderer *ren = SDL_CreateRenderer(win, renderer_name_len > 0 ? renderer_name : NULL);
    if (!ren) {
        RETURN_FALSE;
    }
    RETURN_RES(zend_register_resource(ren, le_sdl_renderer));
}

PHP_FUNCTION(sdl_get_num_render_drivers) {
    int num = SDL_GetNumRenderDrivers();
    RETURN_LONG(num);
}

PHP_FUNCTION(sdl_get_render_driver) {
    zend_long index;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) == FAILURE) {
        RETURN_THROWS();
    }

    const char *name = SDL_GetRenderDriver((int)index);
    if (!name) {
        RETURN_FALSE;
    }
    RETURN_STRING(name);
}

PHP_FUNCTION(sdl_set_render_draw_color) {
    zval *ren_res;
    SDL_Renderer *ren;
    zend_long r, g, b, a;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "rllll", &ren_res, &r, &g, &b, &a) == FAILURE) {
        RETURN_THROWS();
    }

    ren = (SDL_Renderer *)zend_fetch_resource(Z_RES_P(ren_res), "SDL_Renderer", le_sdl_renderer);
    if (!ren) {
        RETURN_FALSE;
    }

    SDL_SetRenderDrawColor(ren, (Uint8)r, (Uint8)g, (Uint8)b, (Uint8)a);
    RETURN_TRUE;
}

PHP_FUNCTION(sdl_render_clear) {
    zval *ren_res;
    SDL_Renderer *ren;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &ren_res) == FAILURE) {
        RETURN_THROWS();
    }

    ren = (SDL_Renderer *)zend_fetch_resource(Z_RES_P(ren_res), "SDL_Renderer", le_sdl_renderer);
    if (!ren) {
        RETURN_FALSE;
    }

    SDL_RenderClear(ren);
    RETURN_TRUE;
}

PHP_FUNCTION(sdl_render_fill_rect) {
    zval *ren_res;
    SDL_Renderer *ren;
    zval *rect_arr;
    HashTable *rect_ht;
    zval *data;
    zend_long x, y, w, h;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ra", &ren_res, &rect_arr) == FAILURE) {
        RETURN_THROWS();
    }

    ren = (SDL_Renderer *)zend_fetch_resource(Z_RES_P(ren_res), "SDL_Renderer", le_sdl_renderer);
    if (!ren) {
        RETURN_FALSE;
    }

    rect_ht = Z_ARRVAL_P(rect_arr);
    if (((data = zend_hash_str_find(rect_ht, "x", 1)) != NULL && (x = zval_get_long(data), true)) &&
        ((data = zend_hash_str_find(rect_ht, "y", 1)) != NULL && (y = zval_get_long(data), true)) &&
        ((data = zend_hash_str_find(rect_ht, "w", 1)) != NULL && (w = zval_get_long(data), true)) &&
        ((data = zend_hash_str_find(rect_ht, "h", 1)) != NULL && (h = zval_get_long(data), true))) {

        SDL_FRect rect = {(float)x, (float)y, (float)w, (float)h};
        SDL_RenderFillRect(ren, &rect);
        RETURN_TRUE;
    }
    
    zend_throw_error(NULL, "Invalid rectangle array passed to sdl_render_fill_rect. Expected ['x'=>int, 'y'=>int, 'w'=>int, 'h'=>int]");
    RETURN_THROWS();
}

PHP_FUNCTION(sdl_render_rect) {
    zval *ren_res;
    SDL_Renderer *ren;
    zval *rect_arr;
    HashTable *rect_ht;
    zval *data;
    zend_long x, y, w, h;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ra", &ren_res, &rect_arr) == FAILURE) {
        RETURN_THROWS();
    }

    ren = (SDL_Renderer *)zend_fetch_resource(Z_RES_P(ren_res), "SDL_Renderer", le_sdl_renderer);
    if (!ren) {
        RETURN_FALSE;
    }

    rect_ht = Z_ARRVAL_P(rect_arr);
    if (((data = zend_hash_str_find(rect_ht, "x", 1)) != NULL && (x = zval_get_long(data), true)) &&
        ((data = zend_hash_str_find(rect_ht, "y", 1)) != NULL && (y = zval_get_long(data), true)) &&
        ((data = zend_hash_str_find(rect_ht, "w", 1)) != NULL && (w = zval_get_long(data), true)) &&
        ((data = zend_hash_str_find(rect_ht, "h", 1)) != NULL && (h = zval_get_long(data), true))) {

        SDL_FRect rect = {(float)x, (float)y, (float)w, (float)h};
        SDL_RenderRect(ren, &rect);
        RETURN_TRUE;
    }

    zend_throw_error(NULL, "Invalid rectangle array passed to sdl_render_rect. Expected ['x'=>int, 'y'=>int, 'w'=>int, 'h'=>int]");
    RETURN_THROWS();
}

PHP_FUNCTION(sdl_render_present) {
    zval *ren_res;
    SDL_Renderer *ren;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &ren_res) == FAILURE) {
        RETURN_THROWS();
    }

    ren = (SDL_Renderer *)zend_fetch_resource(Z_RES_P(ren_res), "SDL_Renderer", le_sdl_renderer);
    if (!ren) {
        RETURN_FALSE;
    }

    SDL_RenderPresent(ren);
    RETURN_TRUE;
}

PHP_FUNCTION(sdl_delay) {
    zend_long ms;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &ms) == FAILURE) {
        RETURN_THROWS();
    }
    SDL_Delay((Uint32)ms);
}

PHP_FUNCTION(sdl_get_error) {
    const char *error = SDL_GetError();
    RETURN_STRING(error);
}

PHP_FUNCTION(sdl_create_texture_from_surface) {
    zval *ren_res, *surf_res;
    SDL_Renderer *renderer;
    SDL_Surface *surface;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "rr", &ren_res, &surf_res) == FAILURE) {
        RETURN_THROWS();
    }

    renderer = (SDL_Renderer *)zend_fetch_resource(Z_RES_P(ren_res), "SDL_Renderer", le_sdl_renderer);
    if (!renderer) {
        RETURN_FALSE;
    }

    surface = (SDL_Surface *)zend_fetch_resource(Z_RES_P(surf_res), "SDL_Surface", le_sdl_surface);
    if (!surface) {
        RETURN_FALSE;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        php_error_docref(NULL, E_WARNING, "Failed to create texture: %s", SDL_GetError());
        RETURN_FALSE;
    }

    RETURN_RES(zend_register_resource(texture, le_sdl_texture));
}

PHP_FUNCTION(sdl_render_texture) {
    zval *ren_res, *tex_res;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    zval *dst_arr = NULL;
    HashTable *dst_ht;
    zval *data;
    zend_long x, y, w, h;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "rr|a", &ren_res, &tex_res, &dst_arr) == FAILURE) {
        RETURN_THROWS();
    }

    renderer = (SDL_Renderer *)zend_fetch_resource(Z_RES_P(ren_res), "SDL_Renderer", le_sdl_renderer);
    if (!renderer) {
        RETURN_FALSE;
    }

    texture = (SDL_Texture *)zend_fetch_resource(Z_RES_P(tex_res), "SDL_Texture", le_sdl_texture);
    if (!texture) {
        RETURN_FALSE;
    }

    if (dst_arr != NULL) {
        // Mit Zielrechteck
        dst_ht = Z_ARRVAL_P(dst_arr);
        if (((data = zend_hash_str_find(dst_ht, "x", 1)) != NULL && (x = zval_get_long(data), true)) &&
            ((data = zend_hash_str_find(dst_ht, "y", 1)) != NULL && (y = zval_get_long(data), true)) &&
            ((data = zend_hash_str_find(dst_ht, "w", 1)) != NULL && (w = zval_get_long(data), true)) &&
            ((data = zend_hash_str_find(dst_ht, "h", 1)) != NULL && (h = zval_get_long(data), true))) {

            SDL_FRect dstrect = {(float)x, (float)y, (float)w, (float)h};
            SDL_RenderTexture(renderer, texture, NULL, &dstrect);
            RETURN_TRUE;
        }

        zend_throw_error(NULL, "Invalid destination rectangle. Expected ['x'=>int, 'y'=>int, 'w'=>int, 'h'=>int]");
        RETURN_THROWS();
    } else {
        // Ohne Zielrechteck - volle Größe
        SDL_RenderTexture(renderer, texture, NULL, NULL);
        RETURN_TRUE;
    }
}

PHP_FUNCTION(sdl_create_texture) {
    zval *ren_res;
    SDL_Renderer *renderer;
    zend_long format, access, width, height;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "rllll", &ren_res, &format, &access, &width, &height) == FAILURE) {
        RETURN_THROWS();
    }

    renderer = (SDL_Renderer *)zend_fetch_resource(Z_RES_P(ren_res), "SDL_Renderer", le_sdl_renderer);
    if (!renderer) {
        RETURN_FALSE;
    }

    SDL_Texture *texture = SDL_CreateTexture(renderer, (Uint32)format, (int)access, (int)width, (int)height);
    if (!texture) {
        php_error_docref(NULL, E_WARNING, "Failed to create texture: %s", SDL_GetError());
        RETURN_FALSE;
    }

    RETURN_RES(zend_register_resource(texture, le_sdl_texture));
}

PHP_FUNCTION(sdl_destroy_texture) {
    zval *tex_res;
    SDL_Texture *texture;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &tex_res) == FAILURE) {
        RETURN_THROWS();
    }

    texture = (SDL_Texture *)zend_fetch_resource(Z_RES_P(tex_res), "SDL_Texture", le_sdl_texture);
    if (!texture) {
        RETURN_FALSE;
    }

    // Delete the resource to trigger the destructor
    zend_list_close(Z_RES_P(tex_res));
    RETURN_TRUE;
}

PHP_FUNCTION(sdl_update_texture) {
    zval *tex_res, *rect_arr, *pixels_arr;
    SDL_Texture *texture;
    SDL_Rect *rect = NULL, tmp_rect;
    zend_long pitch;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ra!al", &tex_res, &rect_arr, &pixels_arr, &pitch) == FAILURE) {
        RETURN_THROWS();
    }

    texture = (SDL_Texture *)zend_fetch_resource(Z_RES_P(tex_res), "SDL_Texture", le_sdl_texture);
    if (!texture) {
        RETURN_FALSE;
    }

    // Parse rect if provided
    if (rect_arr != NULL && Z_TYPE_P(rect_arr) == IS_ARRAY) {
        HashTable *rect_ht = Z_ARRVAL_P(rect_arr);
        zval *val;

        if ((val = zend_hash_str_find(rect_ht, "x", sizeof("x")-1)) != NULL) {
            tmp_rect.x = (int)zval_get_long(val);
        }
        if ((val = zend_hash_str_find(rect_ht, "y", sizeof("y")-1)) != NULL) {
            tmp_rect.y = (int)zval_get_long(val);
        }
        if ((val = zend_hash_str_find(rect_ht, "w", sizeof("w")-1)) != NULL) {
            tmp_rect.w = (int)zval_get_long(val);
        }
        if ((val = zend_hash_str_find(rect_ht, "h", sizeof("h")-1)) != NULL) {
            tmp_rect.h = (int)zval_get_long(val);
        }
        rect = &tmp_rect;
    }

    // Convert PHP array to pixel data
    HashTable *pixels_ht = Z_ARRVAL_P(pixels_arr);
    int pixel_count = zend_hash_num_elements(pixels_ht);
    Uint32 *pixels = emalloc(pixel_count * sizeof(Uint32));

    zval *pixel_val;
    int i = 0;
    ZEND_HASH_FOREACH_VAL(pixels_ht, pixel_val) {
        pixels[i++] = (Uint32)zval_get_long(pixel_val);
    } ZEND_HASH_FOREACH_END();

    // Update texture
    if (SDL_UpdateTexture(texture, rect, pixels, (int)pitch) < 0) {
        efree(pixels);
        php_error_docref(NULL, E_WARNING, "Failed to update texture: %s", SDL_GetError());
        RETURN_FALSE;
    }

    efree(pixels);
    RETURN_TRUE;
}

PHP_FUNCTION(sdl_set_texture_blend_mode) {
    zval *tex_res;
    SDL_Texture *texture;
    zend_long blend_mode;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "rl", &tex_res, &blend_mode) == FAILURE) {
        RETURN_THROWS();
    }

    texture = (SDL_Texture *)zend_fetch_resource(Z_RES_P(tex_res), "SDL_Texture", le_sdl_texture);
    if (!texture) {
        RETURN_FALSE;
    }

    if (SDL_SetTextureBlendMode(texture, (SDL_BlendMode)blend_mode) < 0) {
        php_error_docref(NULL, E_WARNING, "Failed to set texture blend mode: %s", SDL_GetError());
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(sdl_set_texture_alpha_mod) {
    zval *tex_res;
    SDL_Texture *texture;
    zend_long alpha;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "rl", &tex_res, &alpha) == FAILURE) {
        RETURN_THROWS();
    }

    texture = (SDL_Texture *)zend_fetch_resource(Z_RES_P(tex_res), "SDL_Texture", le_sdl_texture);
    if (!texture) {
        RETURN_FALSE;
    }

    if (SDL_SetTextureAlphaMod(texture, (Uint8)alpha) < 0) {
        php_error_docref(NULL, E_WARNING, "Failed to set texture alpha mod: %s", SDL_GetError());
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(tray_setup)
{
    char *icon;
    size_t icon_len;
    zval *menu_arr = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|a", &icon, &icon_len, &menu_arr) == FAILURE) {
        RETURN_THROWS();
    }

    memset(&g_tray, 0, sizeof(g_tray));
    g_tray.icon = estrdup(icon);

    g_tray_menu = NULL;
    g_tray_menu_count = 0;

    if (menu_arr && Z_TYPE_P(menu_arr) == IS_ARRAY) {
        HashTable *ht = Z_ARRVAL_P(menu_arr);
        int count = zend_hash_num_elements(ht);
        if (count > 0) {
            g_tray_menu = ecalloc(count + 1, sizeof(struct tray_menu));
            g_tray_menu_count = count;

            int idx = 0;
            zval *val;
            ZEND_HASH_FOREACH_VAL(ht, val) {
                if (idx >= count) {
                    break;
                }
                struct tray_menu *m = &g_tray_menu[idx];
                if (Z_TYPE_P(val) == IS_STRING) {
                    m->text = estrdup(Z_STRVAL_P(val));
                } else {
                    m->text = estrdup("-");
                }
                m->disabled = 0;
                m->checked = 0;
                m->cb = php_tray_menu_cb;
                m->context = (void *)(intptr_t)idx;
                m->submenu = NULL;
                idx++;
            } ZEND_HASH_FOREACH_END();

            g_tray.menu = g_tray_menu;
        }
    }

    if (tray_init(&g_tray) != 0) {
        RETURN_FALSE;
    }

    g_tray_last_index = -1;
    g_tray_last_clicked = 0;
    RETURN_TRUE;
}

PHP_FUNCTION(tray_poll)
{
    zend_bool blocking = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "|b", &blocking) == FAILURE) {
        RETURN_THROWS();
    }

    int res = -1;// tray_loop(blocking ? 1 : 0);
    if (res < 0) {
        RETURN_FALSE;
    }

    if (g_tray_last_clicked) {
        int idx = g_tray_last_index;
        g_tray_last_clicked = 0;
        RETURN_LONG(idx);
    }

    RETURN_LONG(-1);
}

PHP_FUNCTION(tray_exit)
{
    if (zend_parse_parameters_none() == FAILURE) {
        RETURN_THROWS();
    }

//    tray_exit();
    RETURN_TRUE;
}

PHP_FUNCTION(desktop_notify)
{
    char *title, *body;
    size_t title_len, body_len;
    zval *options = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss|a", &title, &title_len, &body, &body_len, &options) == FAILURE) {
        RETURN_THROWS();
    }

#ifdef HAVE_LIBNOTIFY
    if (!notify_is_initted()) {
        if (!notify_init("PHPNative")) {
            php_error_docref(NULL, E_WARNING, "Failed to initialize libnotify");
            RETURN_FALSE;
        }
    }

    NotifyNotification *n = notify_notification_new(title, body, NULL);
    if (!n) {
        php_error_docref(NULL, E_WARNING, "Failed to create notification");
        RETURN_FALSE;
    }

    if (options && Z_TYPE_P(options) == IS_ARRAY) {
        zval *timeout = zend_hash_str_find(Z_ARRVAL_P(options), "timeout", sizeof("timeout") - 1);
        if (timeout && Z_TYPE_P(timeout) == IS_LONG) {
            notify_notification_set_timeout(n, (int) Z_LVAL_P(timeout));
        }

        zval *urgency = zend_hash_str_find(Z_ARRVAL_P(options), "urgency", sizeof("urgency") - 1);
        if (urgency && Z_TYPE_P(urgency) == IS_STRING) {
            const char *u = Z_STRVAL_P(urgency);
            if (strcmp(u, "low") == 0) {
                notify_notification_set_urgency(n, NOTIFY_URGENCY_LOW);
            } else if (strcmp(u, "critical") == 0) {
                notify_notification_set_urgency(n, NOTIFY_URGENCY_CRITICAL);
            } else {
                notify_notification_set_urgency(n, NOTIFY_URGENCY_NORMAL);
            }
        }
    }

    GError *error = NULL;
    gboolean res = notify_notification_show(n, &error);
    if (!res) {
        if (error) {
            php_error_docref(NULL, E_WARNING, "Notification error: %s", error->message);
            g_error_free(error);
        }
        g_object_unref(G_OBJECT(n));
        RETURN_FALSE;
    }

    g_object_unref(G_OBJECT(n));
    RETURN_TRUE;
#else
    php_error_docref(NULL, E_WARNING, "desktop_notify() not available (libnotify not found at build time)");
    RETURN_FALSE;
#endif
}

PHP_FUNCTION(sdl_create_box_shadow_texture) {
    zval *ren_res;
    SDL_Renderer *renderer;
    zend_long width, height, blurRadius, alpha, r, g, b;

    if (zend_parse_parameters(
            ZEND_NUM_ARGS(),
            "rlllllll",
            &ren_res,
            &width,
            &height,
            &blurRadius,
            &alpha,
            &r,
            &g,
            &b) == FAILURE) {
        RETURN_THROWS();
    }

    renderer = (SDL_Renderer *)zend_fetch_resource(Z_RES_P(ren_res), "SDL_Renderer", le_sdl_renderer);
    if (!renderer) {
        RETURN_FALSE;
    }

    if (width <= 0 || height <= 0) {
        RETURN_FALSE;
    }

    int w = (int) width;
    int h = (int) height;
    int radius = (int) blurRadius;
    if (radius < 0) {
        radius = 0;
    }
    if (alpha < 0) {
        alpha = 0;
    }
    if (alpha > 255) {
        alpha = 255;
    }

    int size = w * h;
    Uint8 *alphaMap = emalloc(size * sizeof(Uint8));
    if (!alphaMap) {
        RETURN_FALSE;
    }

    memset(alphaMap, 0, size * sizeof(Uint8));

    int margin = radius + 2;
    for (int y = margin; y < h - margin; y++) {
        int rowOffset = y * w;
        for (int x = margin; x < w - margin; x++) {
            alphaMap[rowOffset + x] = (Uint8) alpha;
        }
    }

    if (radius > 0) {
        Uint8 *temp = emalloc(size * sizeof(Uint8));
        if (!temp) {
            efree(alphaMap);
            RETURN_FALSE;
        }

        // Horizontal blur
        for (int y = 0; y < h; y++) {
            int rowOffset = y * w;
            for (int x = 0; x < w; x++) {
                int sum = 0;
                int count = 0;
                int xStart = x - radius;
                int xEnd = x + radius;
                if (xStart < 0) xStart = 0;
                if (xEnd >= w) xEnd = w - 1;
                for (int xi = xStart; xi <= xEnd; xi++) {
                    sum += alphaMap[rowOffset + xi];
                    count++;
                }
                temp[rowOffset + x] = count > 0 ? (Uint8)(sum / count) : 0;
            }
        }

        // Vertical blur
        for (int x = 0; x < w; x++) {
            for (int y = 0; y < h; y++) {
                int sum = 0;
                int count = 0;
                int yStart = y - radius;
                int yEnd = y + radius;
                if (yStart < 0) yStart = 0;
                if (yEnd >= h) yEnd = h - 1;
                for (int yi = yStart; yi <= yEnd; yi++) {
                    sum += temp[yi * w + x];
                    count++;
                }
                alphaMap[y * w + x] = count > 0 ? (Uint8)(sum / count) : 0;
            }
        }

        efree(temp);
    }

    SDL_Texture *texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STATIC,
        w,
        h);

    if (!texture) {
        efree(alphaMap);
        php_error_docref(NULL, E_WARNING, "Failed to create shadow texture: %s", SDL_GetError());
        RETURN_FALSE;
    }

    Uint32 *pixels = emalloc(size * sizeof(Uint32));
    if (!pixels) {
        efree(alphaMap);
        SDL_DestroyTexture(texture);
        RETURN_FALSE;
    }

    Uint8 red = (Uint8) r;
    Uint8 green = (Uint8) g;
    Uint8 blue = (Uint8) b;

    for (int i = 0; i < size; i++) {
        Uint8 a = alphaMap[i];
        pixels[i] = ((Uint32)a << 24) | ((Uint32)red << 16) | ((Uint32)green << 8) | (Uint32)blue;
    }

    if (SDL_UpdateTexture(texture, NULL, pixels, w * 4) < 0) {
        efree(alphaMap);
        efree(pixels);
        SDL_DestroyTexture(texture);
        php_error_docref(NULL, E_WARNING, "Failed to update shadow texture: %s", SDL_GetError());
        RETURN_FALSE;
    }

    efree(alphaMap);
    efree(pixels);

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    RETURN_RES(zend_register_resource(texture, le_sdl_texture));
}

PHP_FUNCTION(sdl_get_render_target) {
    zval *ren_res;
    SDL_Renderer *renderer;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &ren_res) == FAILURE) {
        RETURN_THROWS();
    }

    renderer = (SDL_Renderer *)zend_fetch_resource(Z_RES_P(ren_res), "SDL_Renderer", le_sdl_renderer);
    if (!renderer) {
        RETURN_FALSE;
    }

    SDL_Texture *texture = SDL_GetRenderTarget(renderer);
    if (!texture) {
        // NULL is valid - means rendering to the screen
        RETURN_NULL();
    }

    // Return the existing texture resource (don't register a new one)
    // We just return the texture pointer as a resource
    RETURN_RES(zend_register_resource(texture, le_sdl_texture));
}

PHP_FUNCTION(sdl_set_render_target) {
    zval *ren_res;
    zval *tex_res = NULL;
    SDL_Renderer *renderer;
    SDL_Texture *texture = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r|r!", &ren_res, &tex_res) == FAILURE) {
        RETURN_THROWS();
    }

    renderer = (SDL_Renderer *)zend_fetch_resource(Z_RES_P(ren_res), "SDL_Renderer", le_sdl_renderer);
    if (!renderer) {
        RETURN_FALSE;
    }

    if (tex_res != NULL && Z_TYPE_P(tex_res) == IS_RESOURCE) {
        texture = (SDL_Texture *)zend_fetch_resource(Z_RES_P(tex_res), "SDL_Texture", le_sdl_texture);
        if (!texture) {
            RETURN_FALSE;
        }
    }

    if (SDL_SetRenderTarget(renderer, texture) < 0) {
        php_error_docref(NULL, E_WARNING, "Failed to set render target: %s", SDL_GetError());
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(sdl_rounded_box)
{
    zval *ren_res;
    SDL_Renderer *ren;
    zend_long x1, y1, x2, y2, rad;
    zend_long r, g, b, a;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "rlllllllll", &ren_res, &x1, &y1, &x2, &y2, &rad, &r, &g, &b, &a) == FAILURE) {
        RETURN_THROWS();
    }

    ren = (SDL_Renderer *)zend_fetch_resource(Z_RES_P(ren_res), "SDL_Renderer", le_sdl_renderer);
    if (!ren) {
        RETURN_FALSE;
    }

    // Zeichne eine Box mit gleichen Radien an allen Ecken (Wrapper um die erweiterte Variante)
    SDL_SetRenderDrawColor(ren, (Uint8)r, (Uint8)g, (Uint8)b, (Uint8)a);

    int halfw = ((Sint16)x2 - (Sint16)x1) / 2;
    int halfh = ((Sint16)y2 - (Sint16)y1) / 2;
    int rad_tl = (int)rad;
    int rad_tr = (int)rad;
    int rad_br = (int)rad;
    int rad_bl = (int)rad;

    if (rad_tl > halfw) rad_tl = halfw; if (rad_tl > halfh) rad_tl = halfh;
    if (rad_tr > halfw) rad_tr = halfw; if (rad_tr > halfh) rad_tr = halfh;
    if (rad_br > halfw) rad_br = halfw; if (rad_br > halfh) rad_br = halfh;
    if (rad_bl > halfw) rad_bl = halfw; if (rad_bl > halfh) rad_bl = halfh;

    SDL_FRect topRect = { x1 + rad_tl, y1, x2 - x1 - rad_tl - rad_tr, rad_tl > rad_tr ? rad_tl : rad_tr };
    if (topRect.w > 0 && topRect.h > 0) SDL_RenderFillRect(ren, &topRect);

    int maxBottomRad = rad_bl > rad_br ? rad_bl : rad_br;
    SDL_FRect bottomRect = { x1 + rad_bl, y2 - maxBottomRad, x2 - x1 - rad_bl - rad_br, maxBottomRad };
    if (bottomRect.w > 0 && bottomRect.h > 0) SDL_RenderFillRect(ren, &bottomRect);

    SDL_FRect leftRect = { x1, y1 + rad_tl, rad_tl > rad_bl ? rad_tl : rad_bl, y2 - y1 - rad_tl - rad_bl };
    if (leftRect.w > 0 && leftRect.h > 0) SDL_RenderFillRect(ren, &leftRect);

    int maxRightRad = rad_tr > rad_br ? rad_tr : rad_br;
    SDL_FRect rightRect = { x2 - maxRightRad, y1 + rad_tr, maxRightRad, y2 - y1 - rad_tr - rad_br };
    if (rightRect.w > 0 && rightRect.h > 0) SDL_RenderFillRect(ren, &rightRect);

    int maxLeftRad = rad_tl > rad_bl ? rad_tl : rad_bl;
    maxRightRad = rad_tr > rad_br ? rad_tr : rad_br;
    SDL_FRect centerRect = { x1 + maxLeftRad, y1, x2 - x1 - maxLeftRad - maxRightRad, y2 - y1 };
    if (centerRect.w > 0 && centerRect.h > 0) SDL_RenderFillRect(ren, &centerRect);

    if (rad_tl > 0) filled_quarter_circle(ren, x1 + rad_tl, y1 + rad_tl, rad_tl, 0);
    if (rad_tr > 0) filled_quarter_circle(ren, x2 - rad_tr - 1, y1 + rad_tr, rad_tr, 1);
    if (rad_br > 0) filled_quarter_circle(ren, x2 - rad_br - 1, y2 - rad_br - 1, rad_br, 2);
    if (rad_bl > 0) filled_quarter_circle(ren, x1 + rad_bl, y2 - rad_bl - 1, rad_bl, 3);

    RETURN_TRUE;
}

#include <math.h>

PHP_FUNCTION(sdl_rounded_box_ex)
{
    zval *ren_res;
    SDL_Renderer *ren;
    zend_long x1, y1, x2, y2;
    zend_long rad_tl, rad_tr, rad_br, rad_bl;
    zend_long r, g, b, a;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "rllllllllllll", &ren_res, &x1, &y1, &x2, &y2, &rad_tl, &rad_tr, &rad_br, &rad_bl, &r, &g, &b, &a) == FAILURE) {
        RETURN_THROWS();
    }

    ren = (SDL_Renderer *)zend_fetch_resource(Z_RES_P(ren_res), "SDL_Renderer", le_sdl_renderer);
    if (!ren) RETURN_FALSE;

    SDL_SetRenderDrawColor(ren, r, g, b, a);

    int halfw = ((Sint16)x2-(Sint16)x1) / 2;
    int halfh = ((Sint16)y2-(Sint16)y1) / 2;
    if (rad_tl > halfw) rad_tl = halfw; if (rad_tl > halfh) rad_tl = halfh;
    if (rad_tr > halfw) rad_tr = halfw; if (rad_tr > halfh) rad_tr = halfh;
    if (rad_br > halfw) rad_br = halfw; if (rad_br > halfh) rad_br = halfh;
    if (rad_bl > halfw) rad_bl = halfw; if (rad_bl > halfh) rad_bl = halfh;

    // Einfachere Strategie: Zeichne Rechtecke die sich überlappen dürfen,
    // dann die Kreise darüber

    // 1) Horizontales Rechteck oben (von linker Ecke bis rechter Ecke)
    SDL_FRect topRect = { x1 + rad_tl, y1, x2 - x1 - rad_tl - rad_tr, rad_tl > rad_tr ? rad_tl : rad_tr };
    if (topRect.w > 0 && topRect.h > 0) SDL_RenderFillRect(ren, &topRect);

    // 2) Horizontales Rechteck unten (von linker Ecke bis rechter Ecke)
    int maxBottomRad = rad_bl > rad_br ? rad_bl : rad_br;
    SDL_FRect bottomRect = { x1 + rad_bl, y2 - maxBottomRad, x2 - x1 - rad_bl - rad_br, maxBottomRad };
    if (bottomRect.w > 0 && bottomRect.h > 0) SDL_RenderFillRect(ren, &bottomRect);

    // 3) Vertikales Rechteck links (volle Höhe zwischen Ecken)
    SDL_FRect leftRect = { x1, y1 + rad_tl, rad_tl > rad_bl ? rad_tl : rad_bl, y2 - y1 - rad_tl - rad_bl };
    if (leftRect.w > 0 && leftRect.h > 0) SDL_RenderFillRect(ren, &leftRect);

    // 4) Vertikales Rechteck rechts (volle Höhe zwischen Ecken)
    int maxRightRad = rad_tr > rad_br ? rad_tr : rad_br;
    SDL_FRect rightRect = { x2 - maxRightRad, y1 + rad_tr, maxRightRad, y2 - y1 - rad_tr - rad_br };
    if (rightRect.w > 0 && rightRect.h > 0) SDL_RenderFillRect(ren, &rightRect);

    // 5) Zentrales Rechteck (füllt die Mitte)
    int maxLeftRad = rad_tl > rad_bl ? rad_tl : rad_bl;
    maxRightRad = rad_tr > rad_br ? rad_tr : rad_br;
    SDL_FRect centerRect = { x1 + maxLeftRad, y1, x2 - x1 - maxLeftRad - maxRightRad, y2 - y1 };
    if (centerRect.w > 0 && centerRect.h > 0) SDL_RenderFillRect(ren, &centerRect);

    // 6) vier gefüllte Viertel-Kreise in den Ecken (darüber zeichnen)
    if (rad_tl > 0) filled_quarter_circle(ren, x1 + rad_tl, y1 + rad_tl, rad_tl, 0);
    if (rad_tr > 0) filled_quarter_circle(ren, x2 - rad_tr - 1, y1 + rad_tr, rad_tr, 1);
    if (rad_br > 0) filled_quarter_circle(ren, x2 - rad_br - 1, y2 - rad_br - 1, rad_br, 2);
    if (rad_bl > 0) filled_quarter_circle(ren, x1 + rad_bl, y2 - rad_bl - 1, rad_bl, 3); 

    RETURN_TRUE;
}

PHP_FUNCTION(sdl_set_render_clip_rect) {
    zval *ren_res;
    SDL_Renderer *ren;
    zval *rect_arr = NULL;
    HashTable *rect_ht;
    zval *data;
    zend_long x, y, w, h;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r|a!", &ren_res, &rect_arr) == FAILURE) {
        RETURN_THROWS();
    }

    ren = (SDL_Renderer *)zend_fetch_resource(Z_RES_P(ren_res), "SDL_Renderer", le_sdl_renderer);
    if (!ren) {
        RETURN_FALSE;
    }

    if (rect_arr == NULL || Z_TYPE_P(rect_arr) == IS_NULL) {
        // Disable clipping
        SDL_SetRenderClipRect(ren, NULL);
        RETURN_TRUE;
    }

    rect_ht = Z_ARRVAL_P(rect_arr);
    if (((data = zend_hash_str_find(rect_ht, "x", 1)) != NULL && (x = zval_get_long(data), true)) &&
        ((data = zend_hash_str_find(rect_ht, "y", 1)) != NULL && (y = zval_get_long(data), true)) &&
        ((data = zend_hash_str_find(rect_ht, "w", 1)) != NULL && (w = zval_get_long(data), true)) &&
        ((data = zend_hash_str_find(rect_ht, "h", 1)) != NULL && (h = zval_get_long(data), true))) {

        SDL_Rect rect = {(int)x, (int)y, (int)w, (int)h};
        SDL_SetRenderClipRect(ren, &rect);
        RETURN_TRUE;
    }

    zend_throw_error(NULL, "Invalid rectangle array passed to sdl_set_render_clip_rect. Expected ['x'=>int, 'y'=>int, 'w'=>int, 'h'=>int] or null");
    RETURN_THROWS();
}

PHP_FUNCTION(sdl_set_render_scale) {
    zval *ren_res;
    SDL_Renderer *ren;
    double scale_x, scale_y;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "rdd", &ren_res, &scale_x, &scale_y) == FAILURE) {
        RETURN_THROWS();
    }

    ren = (SDL_Renderer *)zend_fetch_resource(Z_RES_P(ren_res), "SDL_Renderer", le_sdl_renderer);
    if (!ren) {
        RETURN_FALSE;
    }

    if (SDL_SetRenderScale(ren, (float)scale_x, (float)scale_y) < 0) {
        php_error_docref(NULL, E_WARNING, "Failed to set render scale: %s", SDL_GetError());
        RETURN_FALSE;
    }

    RETURN_TRUE;
}

PHP_FUNCTION(sdl_get_window_size) {
    zval *win_res;
    SDL_Window *win;
    int w, h;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &win_res) == FAILURE) {
        RETURN_THROWS();
    }

    win = (SDL_Window *)zend_fetch_resource(Z_RES_P(win_res), "SDL_Window", le_sdl_window);
    if (!win) {
        RETURN_FALSE;
    }

    SDL_GetWindowSize(win, &w, &h);

    array_init(return_value);
    add_index_long(return_value, 0, w);
    add_index_long(return_value, 1, h);
}

PHP_FUNCTION(sdl_get_window_size_in_pixels) {
    zval *win_res;
    SDL_Window *win;
    int w, h;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &win_res) == FAILURE) {
        RETURN_THROWS();
    }

    win = (SDL_Window *)zend_fetch_resource(Z_RES_P(win_res), "SDL_Window", le_sdl_window);
    if (!win) {
        RETURN_FALSE;
    }

    if (SDL_GetWindowSizeInPixels(win, &w, &h) < 0) {
        php_error_docref(NULL, E_WARNING, "Failed to get window size in pixels: %s", SDL_GetError());
        RETURN_FALSE;
    }

    array_init(return_value);
    add_index_long(return_value, 0, w);
    add_index_long(return_value, 1, h);
}

PHP_FUNCTION(sdl_get_window_pixel_density) {
    zval *win_res;
    SDL_Window *win;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &win_res) == FAILURE) {
        RETURN_THROWS();
    }

    win = (SDL_Window *)zend_fetch_resource(Z_RES_P(win_res), "SDL_Window", le_sdl_window);
    if (!win) {
        RETURN_FALSE;
    }

    float density = SDL_GetWindowPixelDensity(win);
    RETURN_DOUBLE(density);
}

PHP_FUNCTION(sdl_get_renderer_output_size) {
    zval *ren_res;
    SDL_Renderer *ren;
    int w, h;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &ren_res) == FAILURE) {
        RETURN_THROWS();
    }

    ren = (SDL_Renderer *)zend_fetch_resource(Z_RES_P(ren_res), "SDL_Renderer", le_sdl_renderer);
    if (!ren) {
        RETURN_FALSE;
    }

    if (SDL_GetRenderOutputSize(ren, &w, &h) < 0) {
        php_error_docref(NULL, E_WARNING, "Failed to get renderer output size: %s", SDL_GetError());
        RETURN_FALSE;
    }

    array_init(return_value);
    add_index_long(return_value, 0, w);
    add_index_long(return_value, 1, h);
}

PHP_FUNCTION(sdl_get_window_display_scale) {
    zval *win_res;
    SDL_Window *win;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &win_res) == FAILURE) {
        RETURN_THROWS();
    }

    win = (SDL_Window *)zend_fetch_resource(Z_RES_P(win_res), "SDL_Window", le_sdl_window);
    if (!win) {
        RETURN_FALSE;
    }

    float scale = SDL_GetWindowDisplayScale(win);
    RETURN_DOUBLE(scale);
}

PHP_FUNCTION(sdl_get_display_content_scale) {
    zval *win_res;
    SDL_Window *win;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &win_res) == FAILURE) {
        RETURN_THROWS();
    }

    win = (SDL_Window *)zend_fetch_resource(Z_RES_P(win_res), "SDL_Window", le_sdl_window);
    if (!win) {
        RETURN_FALSE;
    }

    SDL_DisplayID display = SDL_GetDisplayForWindow(win);
    if (!display) {
        RETURN_FALSE;
    }

    float scale = SDL_GetDisplayContentScale(display);
    RETURN_DOUBLE(scale);
}

PHP_FUNCTION(sdl_get_current_video_driver) {
    const char *drv = SDL_GetCurrentVideoDriver();
    if (!drv) {
        RETURN_FALSE;
    }
    RETURN_STRING(drv);
}

PHP_FUNCTION(sdl_get_num_video_drivers) {
    int num = SDL_GetNumVideoDrivers();
    RETURN_LONG(num);
}

PHP_FUNCTION(sdl_get_video_driver) {
    zend_long index;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &index) == FAILURE) {
        RETURN_THROWS();
    }

    const char *driver = SDL_GetVideoDriver((int)index);
    if (!driver) {
        RETURN_FALSE;
    }
    RETURN_STRING(driver);
}

PHP_FUNCTION(sdl_start_text_input) {
    zval *win_res;
    SDL_Window *win;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &win_res) == FAILURE) {
        RETURN_THROWS();
    }

    win = (SDL_Window *)zend_fetch_resource(Z_RES_P(win_res), "SDL_Window", le_sdl_window);
    if (!win) {
        RETURN_FALSE;
    }

    SDL_StartTextInput(win);
    RETURN_TRUE;
}

PHP_FUNCTION(sdl_stop_text_input) {
    zval *win_res;
    SDL_Window *win;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &win_res) == FAILURE) {
        RETURN_THROWS();
    }

    win = (SDL_Window *)zend_fetch_resource(Z_RES_P(win_res), "SDL_Window", le_sdl_window);
    if (!win) {
        RETURN_FALSE;
    }

    SDL_StopTextInput(win);
    RETURN_TRUE;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_init, 0, 0, 1)
    ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_quit, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_create_window, 0, 0, 3)
    ZEND_ARG_INFO(0, title)
    ZEND_ARG_INFO(0, w)
    ZEND_ARG_INFO(0, h)
    ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_destroy_window, 0, 0, 1)
    ZEND_ARG_INFO(0, window)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_destroy_renderer, 0, 0, 1)
    ZEND_ARG_INFO(0, renderer)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_get_window_id, 0, 0, 1)
    ZEND_ARG_INFO(0, window)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_create_renderer, 0, 0, 1)
    ZEND_ARG_INFO(0, window)
    ZEND_ARG_INFO(0, renderer_name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_get_num_render_drivers, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_get_render_driver, 0, 0, 1)
    ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_set_render_draw_color, 0, 0, 5)
    ZEND_ARG_INFO(0, renderer)
    ZEND_ARG_INFO(0, r)
    ZEND_ARG_INFO(0, g)
    ZEND_ARG_INFO(0, b)
    ZEND_ARG_INFO(0, a)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_render_clear, 0, 0, 1)
    ZEND_ARG_INFO(0, renderer)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_render_fill_rect, 0, 0, 2)
    ZEND_ARG_INFO(0, renderer)
    ZEND_ARG_INFO(0, rect)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_render_rect, 0, 0, 2)
    ZEND_ARG_INFO(0, renderer)
    ZEND_ARG_INFO(0, rect)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_render_present, 0, 0, 1)
    ZEND_ARG_INFO(0, renderer)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_delay, 0, 0, 1)
    ZEND_ARG_INFO(0, ms)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_get_error, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_create_texture_from_surface, 0, 0, 2)
    ZEND_ARG_INFO(0, renderer)
    ZEND_ARG_INFO(0, surface)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_render_texture, 0, 0, 2)
    ZEND_ARG_INFO(0, renderer)
    ZEND_ARG_INFO(0, texture)
    ZEND_ARG_INFO(0, dstrect)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_create_texture, 0, 0, 5)
    ZEND_ARG_INFO(0, renderer)
    ZEND_ARG_INFO(0, format)
    ZEND_ARG_INFO(0, access)
    ZEND_ARG_INFO(0, width)
    ZEND_ARG_INFO(0, height)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_destroy_texture, 0, 0, 1)
    ZEND_ARG_INFO(0, texture)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_update_texture, 0, 0, 4)
    ZEND_ARG_INFO(0, texture)
    ZEND_ARG_INFO(0, rect)
    ZEND_ARG_ARRAY_INFO(0, pixels, 0)
    ZEND_ARG_INFO(0, pitch)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_set_texture_blend_mode, 0, 0, 2)
    ZEND_ARG_INFO(0, texture)
    ZEND_ARG_INFO(0, blend_mode)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_set_texture_alpha_mod, 0, 0, 2)
    ZEND_ARG_INFO(0, texture)
    ZEND_ARG_INFO(0, alpha)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tray_setup, 0, 0, 1)
    ZEND_ARG_INFO(0, icon)
    ZEND_ARG_ARRAY_INFO(0, menuItems, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tray_poll, 0, 0, 0)
    ZEND_ARG_INFO(0, blocking)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tray_exit, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_desktop_notify, 0, 0, 2)
    ZEND_ARG_INFO(0, title)
    ZEND_ARG_INFO(0, body)
    ZEND_ARG_ARRAY_INFO(0, options, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_create_box_shadow_texture, 0, 0, 8)
    ZEND_ARG_INFO(0, renderer)
    ZEND_ARG_INFO(0, width)
    ZEND_ARG_INFO(0, height)
    ZEND_ARG_INFO(0, blurRadius)
    ZEND_ARG_INFO(0, alpha)
    ZEND_ARG_INFO(0, r)
    ZEND_ARG_INFO(0, g)
    ZEND_ARG_INFO(0, b)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_get_render_target, 0, 0, 1)
    ZEND_ARG_INFO(0, renderer)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_set_render_target, 0, 0, 1)
    ZEND_ARG_INFO(0, renderer)
    ZEND_ARG_INFO(0, texture)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_rounded_box, 0, 0, 10)
    ZEND_ARG_INFO(0, renderer)
    ZEND_ARG_INFO(0, x1)
    ZEND_ARG_INFO(0, y1)
    ZEND_ARG_INFO(0, x2)
    ZEND_ARG_INFO(0, y2)
    ZEND_ARG_INFO(0, radius)
    ZEND_ARG_INFO(0, r)
    ZEND_ARG_INFO(0, g)
    ZEND_ARG_INFO(0, b)
    ZEND_ARG_INFO(0, a)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_rounded_box_ex, 0, 0, 13)
    ZEND_ARG_INFO(0, renderer)
    ZEND_ARG_INFO(0, x1)
    ZEND_ARG_INFO(0, y1)
    ZEND_ARG_INFO(0, x2)
    ZEND_ARG_INFO(0, y2)
    ZEND_ARG_INFO(0, rad_tl)
    ZEND_ARG_INFO(0, rad_tr)
    ZEND_ARG_INFO(0, rad_br)
    ZEND_ARG_INFO(0, rad_bl)
    ZEND_ARG_INFO(0, r)
    ZEND_ARG_INFO(0, g)
    ZEND_ARG_INFO(0, b)
    ZEND_ARG_INFO(0, a)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_set_render_clip_rect, 0, 0, 1)
    ZEND_ARG_INFO(0, renderer)
    ZEND_ARG_INFO(0, rect)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_set_render_scale, 0, 0, 3)
    ZEND_ARG_INFO(0, renderer)
    ZEND_ARG_INFO(0, scaleX)
    ZEND_ARG_INFO(0, scaleY)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_get_window_size, 0, 0, 1)
    ZEND_ARG_INFO(0, window)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_get_window_size_in_pixels, 0, 0, 1)
    ZEND_ARG_INFO(0, window)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_get_window_pixel_density, 0, 0, 1)
    ZEND_ARG_INFO(0, window)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_get_window_display_scale, 0, 0, 1)
    ZEND_ARG_INFO(0, window)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_get_display_content_scale, 0, 0, 1)
    ZEND_ARG_INFO(0, window)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_get_current_video_driver, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_get_num_video_drivers, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_get_video_driver, 0, 0, 1)
    ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_get_renderer_output_size, 0, 0, 1)
    ZEND_ARG_INFO(0, renderer)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_start_text_input, 0, 0, 1)
    ZEND_ARG_INFO(0, window)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_stop_text_input, 0, 0, 1)
    ZEND_ARG_INFO(0, window)
ZEND_END_ARG_INFO()

const zend_function_entry sdl3_functions[] = {
    // SDL3 Core
    PHP_FE(sdl_init, arginfo_sdl_init)
    PHP_FE(sdl_quit, arginfo_sdl_quit)
    PHP_FE(sdl_create_window, arginfo_sdl_create_window)
    PHP_FE(sdl_destroy_window, arginfo_sdl_destroy_window)
    PHP_FE(sdl_destroy_renderer, arginfo_sdl_destroy_renderer)
    PHP_FE(sdl_get_window_id, arginfo_sdl_get_window_id)
    PHP_FE(sdl_create_renderer, arginfo_sdl_create_renderer)
    PHP_FE(sdl_get_num_render_drivers, arginfo_sdl_get_num_render_drivers)
    PHP_FE(sdl_get_render_driver, arginfo_sdl_get_render_driver)
    PHP_FE(sdl_set_render_draw_color, arginfo_sdl_set_render_draw_color)
    PHP_FE(sdl_render_clear, arginfo_sdl_render_clear)
    PHP_FE(sdl_render_fill_rect, arginfo_sdl_render_fill_rect)
    PHP_FE(sdl_render_rect, arginfo_sdl_render_rect)
    PHP_FE(sdl_render_present, arginfo_sdl_render_present)
    PHP_FE(sdl_set_render_scale, arginfo_sdl_set_render_scale)
    PHP_FE(sdl_delay, arginfo_sdl_delay)
    PHP_FE(sdl_get_error, arginfo_sdl_get_error)
    PHP_FE(sdl_create_texture_from_surface, arginfo_sdl_create_texture_from_surface)
    PHP_FE(sdl_render_texture, arginfo_sdl_render_texture)
   PHP_FE(sdl_create_texture, arginfo_sdl_create_texture)
    PHP_FE(sdl_destroy_texture, arginfo_sdl_destroy_texture)
    PHP_FE(sdl_update_texture, arginfo_sdl_update_texture)
    PHP_FE(sdl_set_texture_blend_mode, arginfo_sdl_set_texture_blend_mode)
    PHP_FE(sdl_set_texture_alpha_mod, arginfo_sdl_set_texture_alpha_mod)
    // Tray API
    PHP_FE(tray_setup, arginfo_tray_setup)
    PHP_FE(tray_poll, arginfo_tray_poll)
    PHP_FE(tray_exit, arginfo_tray_exit)
    // Desktop notifications
    PHP_FE(desktop_notify, arginfo_desktop_notify)
    PHP_FE(sdl_create_box_shadow_texture, arginfo_sdl_create_box_shadow_texture)
    PHP_FE(sdl_get_render_target, arginfo_sdl_get_render_target)
    PHP_FE(sdl_set_render_target, arginfo_sdl_set_render_target)
    PHP_FE(sdl_rounded_box, arginfo_sdl_rounded_box)
    PHP_FE(sdl_rounded_box_ex, arginfo_sdl_rounded_box_ex)
    PHP_FE(sdl_set_render_clip_rect, arginfo_sdl_set_render_clip_rect)
    PHP_FE(sdl_get_window_size, arginfo_sdl_get_window_size)
    PHP_FE(sdl_get_window_size_in_pixels, arginfo_sdl_get_window_size_in_pixels)
    PHP_FE(sdl_get_window_pixel_density, arginfo_sdl_get_window_pixel_density)
    PHP_FE(sdl_get_window_display_scale, arginfo_sdl_get_window_display_scale)
    PHP_FE(sdl_get_display_content_scale, arginfo_sdl_get_display_content_scale)
    PHP_FE(sdl_get_current_video_driver, arginfo_sdl_get_current_video_driver)
    PHP_FE(sdl_get_num_video_drivers, arginfo_sdl_get_num_video_drivers)
    PHP_FE(sdl_get_video_driver, arginfo_sdl_get_video_driver)
    PHP_FE(sdl_get_renderer_output_size, arginfo_sdl_get_renderer_output_size)
    PHP_FE(sdl_start_text_input, arginfo_sdl_start_text_input)
    PHP_FE(sdl_stop_text_input, arginfo_sdl_stop_text_input)

    // SDL3_image
    PHP_FE(img_load, arginfo_img_load)
    PHP_FE(img_load_texture, arginfo_img_load_texture)

    // SDL3_ttf
    PHP_FE(ttf_init, arginfo_ttf_init)
    PHP_FE(ttf_open_font, arginfo_ttf_open_font)
    PHP_FE(ttf_close_font, arginfo_ttf_close_font)
    PHP_FE(ttf_render_text_solid, arginfo_ttf_render_text_solid)
    PHP_FE(ttf_render_text_blended, arginfo_ttf_render_text_blended)
    PHP_FE(ttf_render_text_shaded, arginfo_ttf_render_text_shaded)
    PHP_FE(ttf_size_text, arginfo_ttf_size_text)

    // SDL3 Events
    PHP_FE(sdl_poll_event, arginfo_sdl_poll_event)
    PHP_FE(sdl_wait_event, arginfo_sdl_wait_event)
    PHP_FE(sdl_wait_event_timeout, arginfo_sdl_wait_event_timeout)
    PHP_FE(sdl_get_mod_state, arginfo_sdl_get_mod_state)
    PHP_FE(sdl_get_clipboard_text, arginfo_sdl_get_clipboard_text)
    PHP_FE(sdl_set_clipboard_text, arginfo_sdl_set_clipboard_text)

    PHP_FE_END
};

zend_module_entry sdl3_module_entry = {
    STANDARD_MODULE_HEADER,
    "sdl3",
    sdl3_functions,
    PHP_MINIT(sdl3),
    NULL, // MSHUTDOWN
    NULL, // RINIT
    NULL, // RSHUTDOWN
    NULL, // MINFO
    PHP_SDL3_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_SDL3
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(sdl3)
#endif
