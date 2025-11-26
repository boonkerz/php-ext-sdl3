#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_sdl3.h"
#include "sdl3_events.h"
#include <SDL3/SDL.h>

// Registriert Event-Konstanten
void sdl3_events_register_constants(int module_number) {
    // Event Types
    REGISTER_LONG_CONSTANT("SDL_EVENT_QUIT", SDL_EVENT_QUIT, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_EVENT_WINDOW_CLOSE_REQUESTED", SDL_EVENT_WINDOW_CLOSE_REQUESTED, CONST_CS | CONST_PERSISTENT);

    // Window Events
    REGISTER_LONG_CONSTANT("SDL_EVENT_WINDOW_SHOWN", SDL_EVENT_WINDOW_SHOWN, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_EVENT_WINDOW_HIDDEN", SDL_EVENT_WINDOW_HIDDEN, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_EVENT_WINDOW_EXPOSED", SDL_EVENT_WINDOW_EXPOSED, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_EVENT_WINDOW_MOVED", SDL_EVENT_WINDOW_MOVED, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_EVENT_WINDOW_RESIZED", SDL_EVENT_WINDOW_RESIZED, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_EVENT_WINDOW_MINIMIZED", SDL_EVENT_WINDOW_MINIMIZED, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_EVENT_WINDOW_MAXIMIZED", SDL_EVENT_WINDOW_MAXIMIZED, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_EVENT_WINDOW_RESTORED", SDL_EVENT_WINDOW_RESTORED, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_EVENT_WINDOW_MOUSE_ENTER", SDL_EVENT_WINDOW_MOUSE_ENTER, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_EVENT_WINDOW_MOUSE_LEAVE", SDL_EVENT_WINDOW_MOUSE_LEAVE, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_EVENT_WINDOW_FOCUS_GAINED", SDL_EVENT_WINDOW_FOCUS_GAINED, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_EVENT_WINDOW_FOCUS_LOST", SDL_EVENT_WINDOW_FOCUS_LOST, CONST_CS | CONST_PERSISTENT);

    // Keyboard Events
    REGISTER_LONG_CONSTANT("SDL_EVENT_KEY_DOWN", SDL_EVENT_KEY_DOWN, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_EVENT_KEY_UP", SDL_EVENT_KEY_UP, CONST_CS | CONST_PERSISTENT);

    // Mouse Events
    REGISTER_LONG_CONSTANT("SDL_EVENT_MOUSE_MOTION", SDL_EVENT_MOUSE_MOTION, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_EVENT_MOUSE_BUTTON_DOWN", SDL_EVENT_MOUSE_BUTTON_DOWN, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_EVENT_MOUSE_BUTTON_UP", SDL_EVENT_MOUSE_BUTTON_UP, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_EVENT_MOUSE_WHEEL", SDL_EVENT_MOUSE_WHEEL, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_EVENT_TEXT_INPUT", SDL_EVENT_TEXT_INPUT, CONST_CS | CONST_PERSISTENT);

    // Mouse Buttons
    REGISTER_LONG_CONSTANT("SDL_BUTTON_LEFT", SDL_BUTTON_LEFT, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_BUTTON_MIDDLE", SDL_BUTTON_MIDDLE, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_BUTTON_RIGHT", SDL_BUTTON_RIGHT, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_BUTTON_X1", SDL_BUTTON_X1, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDL_BUTTON_X2", SDL_BUTTON_X2, CONST_CS | CONST_PERSISTENT);

    // Common Keycodes
    REGISTER_LONG_CONSTANT("SDLK_RETURN", SDLK_RETURN, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDLK_ESCAPE", SDLK_ESCAPE, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDLK_BACKSPACE", SDLK_BACKSPACE, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDLK_TAB", SDLK_TAB, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDLK_DELETE", SDLK_DELETE, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDLK_LEFT", SDLK_LEFT, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDLK_RIGHT", SDLK_RIGHT, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDLK_UP", SDLK_UP, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDLK_DOWN", SDLK_DOWN, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDLK_HOME", SDLK_HOME, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDLK_END", SDLK_END, CONST_CS | CONST_PERSISTENT);

    // Letter keys for shortcuts
    REGISTER_LONG_CONSTANT("SDLK_A", SDLK_A, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDLK_C", SDLK_C, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDLK_V", SDLK_V, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("SDLK_X", SDLK_X, CONST_CS | CONST_PERSISTENT);

    // Key modifiers
    REGISTER_LONG_CONSTANT("KMOD_NONE", SDL_KMOD_NONE, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("KMOD_LSHIFT", SDL_KMOD_LSHIFT, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("KMOD_RSHIFT", SDL_KMOD_RSHIFT, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("KMOD_LCTRL", SDL_KMOD_LCTRL, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("KMOD_RCTRL", SDL_KMOD_RCTRL, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("KMOD_LALT", SDL_KMOD_LALT, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("KMOD_RALT", SDL_KMOD_RALT, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("KMOD_LGUI", SDL_KMOD_LGUI, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("KMOD_RGUI", SDL_KMOD_RGUI, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("KMOD_CTRL", SDL_KMOD_CTRL, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("KMOD_SHIFT", SDL_KMOD_SHIFT, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("KMOD_ALT", SDL_KMOD_ALT, CONST_CS | CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("KMOD_GUI", SDL_KMOD_GUI, CONST_CS | CONST_PERSISTENT);
}

PHP_FUNCTION(sdl_poll_event) {
    SDL_Event event;

    if (!SDL_PollEvent(&event)) {
        RETURN_FALSE;
    }

    // Event als Array zurückgeben
    array_init(return_value);
    add_assoc_long(return_value, "type", event.type);
    add_assoc_long(return_value, "timestamp", event.common.timestamp);

    switch (event.type) {
        case SDL_EVENT_QUIT:
            // Keine zusätzlichen Daten
            break;

        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        case SDL_EVENT_WINDOW_SHOWN:
        case SDL_EVENT_WINDOW_HIDDEN:
        case SDL_EVENT_WINDOW_EXPOSED:
        case SDL_EVENT_WINDOW_MOVED:
        case SDL_EVENT_WINDOW_RESIZED:
        case SDL_EVENT_WINDOW_MINIMIZED:
        case SDL_EVENT_WINDOW_MAXIMIZED:
        case SDL_EVENT_WINDOW_RESTORED:
        case SDL_EVENT_WINDOW_MOUSE_ENTER:
        case SDL_EVENT_WINDOW_MOUSE_LEAVE:
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
        case SDL_EVENT_WINDOW_FOCUS_LOST:
            add_assoc_long(return_value, "window_id", event.window.windowID);
            if (event.type == SDL_EVENT_WINDOW_MOVED || event.type == SDL_EVENT_WINDOW_RESIZED) {
                add_assoc_long(return_value, "data1", event.window.data1);
                add_assoc_long(return_value, "data2", event.window.data2);
            }
            break;

        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            add_assoc_long(return_value, "window_id", event.key.windowID);
            add_assoc_long(return_value, "scancode", event.key.scancode);
            add_assoc_long(return_value, "keycode", event.key.key);
            add_assoc_long(return_value, "mod", event.key.mod);
            add_assoc_bool(return_value, "repeat", event.key.repeat);
            add_assoc_bool(return_value, "down", event.key.down);
            break;

        case SDL_EVENT_MOUSE_MOTION:
            add_assoc_long(return_value, "window_id", event.motion.windowID);
            add_assoc_long(return_value, "which", event.motion.which);
            add_assoc_double(return_value, "x", event.motion.x);
            add_assoc_double(return_value, "y", event.motion.y);
            add_assoc_double(return_value, "xrel", event.motion.xrel);
            add_assoc_double(return_value, "yrel", event.motion.yrel);
            add_assoc_long(return_value, "state", event.motion.state);
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
            add_assoc_long(return_value, "window_id", event.button.windowID);
            add_assoc_long(return_value, "which", event.button.which);
            add_assoc_long(return_value, "button", event.button.button);
            add_assoc_bool(return_value, "down", event.button.down);
            add_assoc_long(return_value, "clicks", event.button.clicks);
            add_assoc_double(return_value, "x", event.button.x);
            add_assoc_double(return_value, "y", event.button.y);
            break;

        case SDL_EVENT_MOUSE_WHEEL:
            add_assoc_long(return_value, "window_id", event.wheel.windowID);
            add_assoc_long(return_value, "which", event.wheel.which);
            add_assoc_double(return_value, "x", event.wheel.x);
            add_assoc_double(return_value, "y", event.wheel.y);
            add_assoc_long(return_value, "direction", event.wheel.direction);
            add_assoc_double(return_value, "mouse_x", event.wheel.mouse_x);
            add_assoc_double(return_value, "mouse_y", event.wheel.mouse_y);
            break;

        case SDL_EVENT_TEXT_INPUT:
            add_assoc_long(return_value, "window_id", event.text.windowID);
            add_assoc_string(return_value, "text", event.text.text);
            break;
    }
}

PHP_FUNCTION(sdl_wait_event) {
    SDL_Event event;

    if (!SDL_WaitEvent(&event)) {
        php_error_docref(NULL, E_WARNING, "SDL_WaitEvent failed: %s", SDL_GetError());
        RETURN_FALSE;
    }

    // Event als Array zurückgeben (gleiche Logik wie poll_event)
    array_init(return_value);
    add_assoc_long(return_value, "type", event.type);
    add_assoc_long(return_value, "timestamp", event.common.timestamp);
}

PHP_FUNCTION(sdl_wait_event_timeout) {
    zend_long timeout_ms;
    SDL_Event event;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &timeout_ms) == FAILURE) {
        RETURN_THROWS();
    }

    if (!SDL_WaitEventTimeout(&event, (int)timeout_ms)) {
        RETURN_FALSE;  // Timeout oder Fehler
    }

    // Event als Array zurückgeben
    array_init(return_value);
    add_assoc_long(return_value, "type", event.type);
    add_assoc_long(return_value, "timestamp", event.common.timestamp);
}

PHP_FUNCTION(sdl_get_mod_state) {
    SDL_Keymod mod = SDL_GetModState();
    RETURN_LONG((zend_long)mod);
}

PHP_FUNCTION(sdl_get_clipboard_text) {
    char *text = SDL_GetClipboardText();

    if (text == NULL) {
        RETURN_EMPTY_STRING();
    }

    RETVAL_STRING(text);
    SDL_free(text);
}

PHP_FUNCTION(sdl_set_clipboard_text) {
    char *text;
    size_t text_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &text, &text_len) == FAILURE) {
        RETURN_THROWS();
    }

    if (SDL_SetClipboardText(text) < 0) {
        php_error_docref(NULL, E_WARNING, "SDL_SetClipboardText failed: %s", SDL_GetError());
        RETURN_FALSE;
    }

    RETURN_TRUE;
}
