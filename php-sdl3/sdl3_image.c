#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_sdl3.h"
#include "sdl3_image.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

// Resource handles
int le_sdl_surface;

// Destructor für Surface
void sdl_surface_dtor(zend_resource *rsrc) {
    SDL_Surface *surface = (SDL_Surface *)rsrc->ptr;
    if (surface) {
        SDL_DestroySurface(surface);
    }
}

// Registriert die Resource-Typen
void sdl3_image_register_resources(int module_number) {
    le_sdl_surface = zend_register_list_destructors_ex(sdl_surface_dtor, NULL, "SDL_Surface", module_number);
}

PHP_FUNCTION(img_load) {
    char *file;
    size_t file_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &file, &file_len) == FAILURE) {
        RETURN_THROWS();
    }

    SDL_Surface *surface = IMG_Load(file);
    if (!surface) {
        php_error_docref(NULL, E_WARNING, "Failed to load image: %s", SDL_GetError());
        RETURN_FALSE;
    }

    RETURN_RES(zend_register_resource(surface, le_sdl_surface));
}

PHP_FUNCTION(img_load_texture) {
    zval *ren_res;
    SDL_Renderer *renderer;
    char *file;
    size_t file_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "rs", &ren_res, &file, &file_len) == FAILURE) {
        RETURN_THROWS();
    }

    renderer = (SDL_Renderer *)zend_fetch_resource(Z_RES_P(ren_res), "SDL_Renderer", le_sdl_renderer);
    if (!renderer) {
        RETURN_FALSE;
    }

    SDL_Texture *texture = IMG_LoadTexture(renderer, file);
    if (!texture) {
        php_error_docref(NULL, E_WARNING, "Failed to load texture: %s", SDL_GetError());
        RETURN_FALSE;
    }

    RETURN_RES(zend_register_resource(texture, le_sdl_texture));
}
