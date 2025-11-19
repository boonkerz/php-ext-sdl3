#ifndef PHP_SDL3_IMAGE_H
#define PHP_SDL3_IMAGE_H

#include "php.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

// Resource handle für SDL_Surface
extern int le_sdl_surface;

// Destructor
void sdl_surface_dtor(zend_resource *rsrc);

// PHP Funktionen für SDL3_image
PHP_FUNCTION(img_load);
PHP_FUNCTION(img_load_texture);

// Argument Info
ZEND_BEGIN_ARG_INFO_EX(arginfo_img_load, 0, 0, 1)
    ZEND_ARG_INFO(0, file)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_img_load_texture, 0, 0, 2)
    ZEND_ARG_INFO(0, renderer)
    ZEND_ARG_INFO(0, file)
ZEND_END_ARG_INFO()

// Funktion zum Registrieren der Resource-Typen
void sdl3_image_register_resources(int module_number);

#endif
