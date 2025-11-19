<?php

if (!extension_loaded('sdl3')) {
    echo "Die Erweiterung 'sdl3' ist nicht geladen.\n";
    exit(1);
}

// SDL_INIT_VIDEO
const SDL_INIT_VIDEO = 0x00000020;

if (!sdl_init(SDL_INIT_VIDEO)) {
    echo "Fehler bei der Initialisierung von SDL: " . sdl_get_error() . "\n";
    exit(1);
}

$window = sdl_create_window("PHP-SDL3 Fenster", 640, 480);
if (!$window) {
    echo "Fehler beim Erstellen des Fensters: " . sdl_get_error() . "\n";
    sdl_quit();
    exit(1);
}

$renderer = sdl_create_renderer($window);
if (!$renderer) {
    echo "Fehler beim Erstellen des Renderers: " . sdl_get_error() . "\n";
    // Das Fenster wird durch den Resource Destructor zerstört
    sdl_quit();
    exit(1);
}

// Hintergrund blau machen
sdl_set_render_draw_color($renderer, 0, 0, 255, 255);
sdl_render_clear($renderer);

// Ein rotes Rechteck zeichnen
$rect = ['x' => 220, 'y' => 140, 'w' => 200, 'h' => 200];
sdl_set_render_draw_color($renderer, 255, 0, 0, 255);
sdl_render_fill_rect($renderer, $rect);

// Alles auf dem Bildschirm anzeigen
sdl_render_present($renderer);

// 3 Sekunden warten
sdl_delay(3000);

// SDL herunterfahren
// Die Ressourcen für Fenster und Renderer werden automatisch freigegeben
sdl_quit();

echo "Fenster wurde für 3 Sekunden angezeigt.\n";

?>