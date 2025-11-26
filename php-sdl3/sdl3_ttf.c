#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_sdl3.h"
#include "sdl3_ttf.h"
#include "sdl3_image.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

// Resource handles
int le_ttf_font;

// Destructor für Font
void ttf_font_dtor(zend_resource *rsrc) {
    TTF_Font *font = (TTF_Font *)rsrc->ptr;
    if (font) {
        TTF_CloseFont(font);
    }
}

// Registriert die Resource-Typen
void sdl3_ttf_register_resources(int module_number) {
    le_ttf_font = zend_register_list_destructors_ex(ttf_font_dtor, NULL, "TTF_Font", module_number);
}

PHP_FUNCTION(ttf_init) {
    if (!TTF_Init()) {
        php_error_docref(NULL, E_WARNING, "TTF_Init failed: %s", SDL_GetError());
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_FUNCTION(ttf_open_font) {
    char *file;
    size_t file_len;
    zend_long ptsize;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "sl", &file, &file_len, &ptsize) == FAILURE) {
        RETURN_THROWS();
    }

    TTF_Font *font = TTF_OpenFont(file, (float)ptsize);
    if (!font) {
        php_error_docref(NULL, E_WARNING, "Failed to open font: %s", SDL_GetError());
        RETURN_FALSE;
    }

    RETURN_RES(zend_register_resource(font, le_ttf_font));
}

PHP_FUNCTION(ttf_close_font) {
    zval *font_res;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &font_res) == FAILURE) {
        RETURN_THROWS();
    }

    // Resource wird automatisch durch den Destructor freigegeben
    zend_list_close(Z_RES_P(font_res));
    RETURN_TRUE;
}

PHP_FUNCTION(ttf_render_text_solid) {
    zval *font_res;
    TTF_Font *font;
    char *text;
    size_t text_len;
    zend_long r, g, b;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "rslll", &font_res, &text, &text_len, &r, &g, &b) == FAILURE) {
        RETURN_THROWS();
    }

    font = (TTF_Font *)zend_fetch_resource(Z_RES_P(font_res), "TTF_Font", le_ttf_font);
    if (!font) {
        RETURN_FALSE;
    }

    SDL_Color color = {(Uint8)r, (Uint8)g, (Uint8)b, 255};
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, text_len, color);
    if (!surface) {
        php_error_docref(NULL, E_WARNING, "Failed to render text: %s", SDL_GetError());
        RETURN_FALSE;
    }

    RETURN_RES(zend_register_resource(surface, le_sdl_surface));
}

PHP_FUNCTION(ttf_render_text_blended) {
    zval *font_res;
    TTF_Font *font;
    char *text;
    size_t text_len;
    zend_long r, g, b;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "rslll", &font_res, &text, &text_len, &r, &g, &b) == FAILURE) {
        RETURN_THROWS();
    }

    font = (TTF_Font *)zend_fetch_resource(Z_RES_P(font_res), "TTF_Font", le_ttf_font);
    if (!font) {
        RETURN_FALSE;
    }

    SDL_Color color = {(Uint8)r, (Uint8)g, (Uint8)b, 255};
    SDL_Surface *surface = TTF_RenderText_Blended(font, text, text_len, color);
    if (!surface) {
        php_error_docref(NULL, E_WARNING, "Failed to render text: %s", SDL_GetError());
        RETURN_FALSE;
    }

    RETURN_RES(zend_register_resource(surface, le_sdl_surface));
}

PHP_FUNCTION(ttf_render_text_shaded) {
    zval *font_res;
    TTF_Font *font;
    char *text;
    size_t text_len;
    zend_long fg_r, fg_g, fg_b, bg_r, bg_g, bg_b;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "rsllllll", &font_res, &text, &text_len,
                              &fg_r, &fg_g, &fg_b, &bg_r, &bg_g, &bg_b) == FAILURE) {
        RETURN_THROWS();
    }

    font = (TTF_Font *)zend_fetch_resource(Z_RES_P(font_res), "TTF_Font", le_ttf_font);
    if (!font) {
        RETURN_FALSE;
    }

    SDL_Color fg_color = {(Uint8)fg_r, (Uint8)fg_g, (Uint8)fg_b, 255};
    SDL_Color bg_color = {(Uint8)bg_r, (Uint8)bg_g, (Uint8)bg_b, 255};
    SDL_Surface *surface = TTF_RenderText_Shaded(font, text, text_len, fg_color, bg_color);
    if (!surface) {
        php_error_docref(NULL, E_WARNING, "Failed to render text: %s", SDL_GetError());
        RETURN_FALSE;
    }

    RETURN_RES(zend_register_resource(surface, le_sdl_surface));
}

PHP_FUNCTION(ttf_size_text) {
    zval *font_res;
    TTF_Font *font;
    char *text;
    size_t text_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "rs", &font_res, &text, &text_len) == FAILURE) {
        RETURN_THROWS();
    }

    font = (TTF_Font *)zend_fetch_resource(Z_RES_P(font_res), "TTF_Font", le_ttf_font);
    if (!font) {
        RETURN_FALSE;
    }

    int w, h;
    if (!TTF_GetStringSize(font, text, text_len, &w, &h)) {
        php_error_docref(NULL, E_WARNING, "Failed to get text size: %s", SDL_GetError());
        RETURN_FALSE;
    }

    array_init(return_value);
    add_assoc_long(return_value, "w", w);
    add_assoc_long(return_value, "h", h);
}
