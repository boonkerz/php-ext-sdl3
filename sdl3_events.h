#ifndef PHP_SDL3_EVENTS_H
#define PHP_SDL3_EVENTS_H

#include "php.h"
#include <SDL3/SDL.h>

// PHP Funktionen für SDL3 Events
PHP_FUNCTION(sdl_poll_event);
PHP_FUNCTION(sdl_wait_event);
PHP_FUNCTION(sdl_wait_event_timeout);
PHP_FUNCTION(sdl_get_mod_state);
PHP_FUNCTION(sdl_get_clipboard_text);
PHP_FUNCTION(sdl_set_clipboard_text);

// Argument Info
ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_poll_event, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_wait_event, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_wait_event_timeout, 0, 0, 1)
    ZEND_ARG_INFO(0, timeout_ms)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_get_mod_state, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_get_clipboard_text, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sdl_set_clipboard_text, 0, 0, 1)
    ZEND_ARG_INFO(0, text)
ZEND_END_ARG_INFO()

// Funktion zum Registrieren von Event-Konstanten
void sdl3_events_register_constants(int module_number);

#endif
