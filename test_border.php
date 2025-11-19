<?php

// Teste die neue sdl_render_rect Funktion

if (!sdl_init(SDL_INIT_VIDEO)) {
    die("SDL Init failed: " . sdl_get_error());
}

$window = sdl_create_window("Test Border", 640, 480, SDL_WINDOW_RESIZABLE);
if (!$window) {
    die("Window creation failed: " . sdl_get_error());
}

$renderer = sdl_create_renderer($window);
if (!$renderer) {
    die("Renderer creation failed: " . sdl_get_error());
}

$running = true;
while ($running) {
    // Events verarbeiten
    while (sdl_poll_event($event)) {
        if ($event['type'] === SDL_EVENT_QUIT ||
            $event['type'] === SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
            $running = false;
        }
    }

    // Hintergrund weiß
    sdl_set_render_draw_color($renderer, 255, 255, 255, 255);
    sdl_render_clear($renderer);

    // Rahmen zeichnen - Blau
    sdl_set_render_draw_color($renderer, 0, 0, 255, 255);
    sdl_render_rect($renderer, ['x' => 50, 'y' => 50, 'w' => 200, 'h' => 150]);

    // Zweiter Rahmen - Rot
    sdl_set_render_draw_color($renderer, 255, 0, 0, 255);
    sdl_render_rect($renderer, ['x' => 100, 'y' => 100, 'w' => 300, 'h' => 200]);

    // Gefülltes Rechteck zum Vergleich - Grün
    sdl_set_render_draw_color($renderer, 0, 255, 0, 255);
    sdl_render_fill_rect($renderer, ['x' => 300, 'y' => 250, 'w' => 150, 'h' => 100]);

    // Rahmen um das gefüllte Rechteck - Schwarz
    sdl_set_render_draw_color($renderer, 0, 0, 0, 255);
    sdl_render_rect($renderer, ['x' => 300, 'y' => 250, 'w' => 150, 'h' => 100]);

    sdl_render_present($renderer);
    sdl_delay(16); // ~60 FPS
}

sdl_destroy_renderer($renderer);
sdl_destroy_window($window);
sdl_quit();
