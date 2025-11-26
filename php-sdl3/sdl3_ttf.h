#ifndef PHP_SDL3_TTF_H
#define PHP_SDL3_TTF_H

#include "php.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

// Resource handle für TTF_Font
extern int le_ttf_font;

// Destructor
void ttf_font_dtor(zend_resource *rsrc);

// PHP Funktionen für SDL3_ttf
PHP_FUNCTION(ttf_init);
PHP_FUNCTION(ttf_open_font);
PHP_FUNCTION(ttf_close_font);
PHP_FUNCTION(ttf_render_text_solid);
PHP_FUNCTION(ttf_render_text_blended);
PHP_FUNCTION(ttf_render_text_shaded);
PHP_FUNCTION(ttf_size_text);

// Argument Info
ZEND_BEGIN_ARG_INFO_EX(arginfo_ttf_init, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ttf_open_font, 0, 0, 2)
    ZEND_ARG_INFO(0, file)
    ZEND_ARG_INFO(0, ptsize)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ttf_close_font, 0, 0, 1)
    ZEND_ARG_INFO(0, font)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ttf_render_text_solid, 0, 0, 5)
    ZEND_ARG_INFO(0, font)
    ZEND_ARG_INFO(0, text)
    ZEND_ARG_INFO(0, r)
    ZEND_ARG_INFO(0, g)
    ZEND_ARG_INFO(0, b)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ttf_render_text_blended, 0, 0, 5)
    ZEND_ARG_INFO(0, font)
    ZEND_ARG_INFO(0, text)
    ZEND_ARG_INFO(0, r)
    ZEND_ARG_INFO(0, g)
    ZEND_ARG_INFO(0, b)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ttf_render_text_shaded, 0, 0, 8)
    ZEND_ARG_INFO(0, font)
    ZEND_ARG_INFO(0, text)
    ZEND_ARG_INFO(0, fg_r)
    ZEND_ARG_INFO(0, fg_g)
    ZEND_ARG_INFO(0, fg_b)
    ZEND_ARG_INFO(0, bg_r)
    ZEND_ARG_INFO(0, bg_g)
    ZEND_ARG_INFO(0, bg_b)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ttf_size_text, 0, 0, 2)
    ZEND_ARG_INFO(0, font)
    ZEND_ARG_INFO(0, text)
ZEND_END_ARG_INFO()

// Funktion zum Registrieren der Resource-Typen
void sdl3_ttf_register_resources(int module_number);

#endif
