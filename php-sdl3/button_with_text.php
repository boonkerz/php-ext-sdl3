<?php

if (!extension_loaded('sdl3')) {
    echo "Die Erweiterung 'sdl3' ist nicht geladen.\n";
    exit(1);
}

// SDL_INIT_VIDEO
const SDL_INIT_VIDEO = 0x00000020;

// Initialisierung
if (!sdl_init(SDL_INIT_VIDEO)) {
    echo 'Fehler bei der Initialisierung von SDL: ' . sdl_get_error() . "\n";
    exit(1);
}

// TTF initialisieren
if (!ttf_init()) {
    echo 'Fehler bei der Initialisierung von TTF: ' . sdl_get_error() . "\n";
    sdl_quit();
    exit(1);
}

// Fenster erstellen
$window = sdl_create_window('SDL3 Button mit Text', 640, 480);
if (!$window) {
    echo 'Fehler beim Erstellen des Fensters: ' . sdl_get_error() . "\n";
    sdl_quit();
    exit(1);
}

// Renderer erstellen
$renderer = sdl_create_renderer($window);
if (!$renderer) {
    echo 'Fehler beim Erstellen des Renderers: ' . sdl_get_error() . "\n";
    sdl_quit();
    exit(1);
}

// Font laden
$font = ttf_open_font('/usr/share/fonts/truetype/ubuntu/Ubuntu[wdth,wght].ttf', 24);
if (!$font) {
    echo 'Fehler beim Laden der Schriftart: ' . sdl_get_error() . "\n";
    sdl_quit();
    exit(1);
}

// Text-Surface erstellen
$text_surface = ttf_render_text_solid($font, 'Schließen', 255, 255, 255);
if (!$text_surface) {
    echo 'Fehler beim Rendern des Textes: ' . sdl_get_error() . "\n";
    sdl_quit();
    exit(1);
}

// Text-Texture erstellen
$text_texture = sdl_create_texture_from_surface($renderer, $text_surface);
if (!$text_texture) {
    echo 'Fehler beim Erstellen der Text-Texture: ' . sdl_get_error() . "\n";
    sdl_quit();
    exit(1);
}

// Textgröße ermitteln
$text_size = ttf_size_text($font, 'Schließen');

// Button-Eigenschaften
$button = [
    'x' => 220,
    'y' => 200,
    'w' => 200,
    'h' => 80,
    'hovered' => false
];

// Hilfsfunktion: Prüft ob Punkt in Rechteck ist
function point_in_rect($x, $y, $rect) {
    return $x >= $rect['x'] &&
           $x <= $rect['x'] + $rect['w'] &&
           $y >= $rect['y'] &&
           $y <= $rect['y'] + $rect['h'];
}

// Zeichnet den Button
function draw_button($renderer, $button, $text_texture, $text_size) {
    // Button-Hintergrund (grün wenn hovered, blau sonst)
    if ($button['hovered']) {
        sdl_rounded_box($renderer,
            $button['x'], $button['y'],
            $button['x'] + $button['w'], $button['y'] + $button['h'],
            15,
            60, 180, 60, 255
        );
    } else {
        sdl_rounded_box($renderer,
            $button['x'], $button['y'],
            $button['x'] + $button['w'], $button['y'] + $button['h'],
            15,
            70, 130, 180, 255
        );
    }

    // Text zentriert auf Button rendern
    $text_x = $button['x'] + ($button['w'] - $text_size['w']) / 2;
    $text_y = $button['y'] + ($button['h'] - $text_size['h']) / 2;

    sdl_render_texture($renderer, $text_texture, [
        'x' => (int)$text_x,
        'y' => (int)$text_y,
        'w' => $text_size['w'],
        'h' => $text_size['h']
    ]);
}

echo "Button-Beispiel mit Text läuft!\n";
echo "Bewege die Maus über den Button.\n";
echo "Klicke auf den 'Schließen'-Button um das Fenster zu schließen.\n";

// Hauptschleife
$running = true;
while ($running) {
    // Events verarbeiten
    while ($event = sdl_poll_event()) {
        // Quit-Event
        if ($event['type'] === SDL_EVENT_QUIT) {
            $running = false;
        }

        // Window Close Button
        if ($event['type'] === SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
            $running = false;
        }

        // Maus-Bewegung
        if ($event['type'] === SDL_EVENT_MOUSE_MOTION) {
            $button['hovered'] = point_in_rect($event['x'], $event['y'], $button);
        }

        // Maus-Klick
        if ($event['type'] === SDL_EVENT_MOUSE_BUTTON_DOWN) {
            if ($event['button'] === SDL_BUTTON_LEFT) {
                // Prüfe ob auf Button geklickt wurde
                if (point_in_rect($event['x'], $event['y'], $button)) {
                    echo "Button wurde geklickt! Fenster wird geschlossen...\n";
                    $running = false;
                }
            }
        }
    }

    // Hintergrund zeichnen (dunkelgrau)
    sdl_set_render_draw_color($renderer, 45, 45, 48, 255);
    sdl_render_clear($renderer);

    // Button zeichnen
    draw_button($renderer, $button, $text_texture, $text_size);

    // Anzeigen
    sdl_render_present($renderer);

    // Kurze Pause um CPU zu schonen
    sdl_delay(16);  // ~60 FPS
}

echo "Fenster geschlossen.\n";
sdl_quit();
