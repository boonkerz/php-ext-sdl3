<?php

if (!extension_loaded('sdl3')) {
    echo "Die Erweiterung 'sdl3' ist nicht geladen.\n";
    exit(1);
}

// SDL_INIT_VIDEO
const SDL_INIT_VIDEO = 0x00000020;

if (!sdl_init(SDL_INIT_VIDEO)) {
    echo 'Fehler bei der Initialisierung von SDL: ' . sdl_get_error() . "\n";
    exit(1);
}

$window = sdl_create_window('PHP-SDL3 EX Rechteck', 1024, 1024);
if (!$window) {
    echo 'Fehler beim Erstellen des Fensters: ' . sdl_get_error() . "\n";
    sdl_quit();
    exit(1);
}

$renderer = sdl_create_renderer($window);
if (!$renderer) {
    echo 'Fehler beim Erstellen des Renderers: ' . sdl_get_error() . "\n";
    sdl_quit();
    exit(1);
}

// Hintergrund dunkelgrau machen
sdl_set_render_draw_color($renderer, 50, 50, 50, 255);
sdl_render_clear($renderer);

// Ein grünes, abgerundetes Rechteck mit verschiedenen Radien zeichnen
// Radien: Oben-Links, Oben-Rechts, Unten-Rechts, Unten-Links
$rad_tl = 40;
$rad_tr = 10;
$rad_br = 40;
$rad_bl = 10;
sdl_rounded_box_ex($renderer, 100, 100, 200, 200, $rad_tl, $rad_tr, $rad_br, $rad_bl, 30, 200, 70, 255);

// Alles auf dem Bildschirm anzeigen
sdl_render_present($renderer);

// 4 Sekunden warten
sdl_delay(4000);

sdl_quit();

echo "Fenster wurde für 4 Sekunden angezeigt.\n";

