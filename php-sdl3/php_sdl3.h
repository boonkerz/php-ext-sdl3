#ifndef PHP_SDL3_H
#define PHP_SDL3_H

extern zend_module_entry sdl3_module_entry;
#define phpext_sdl3_ptr &sdl3_module_entry

#define PHP_SDL3_VERSION "0.1.0"

// Resource handles exportieren, damit sie in anderen Modulen verfügbar sind
extern int le_sdl_window;
extern int le_sdl_renderer;
extern int le_sdl_texture;
extern int le_sdl_surface;

#endif
