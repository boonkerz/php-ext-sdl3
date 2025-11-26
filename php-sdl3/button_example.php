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

// Fenster erstellen
$window = sdl_create_window('SDL3 Button Beispiel', 640, 480);
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
function draw_button($renderer, $button) {
    // Button-Hintergrund (grün wenn hovered, blau sonst)
    if ($button['hovered']) {
        sdl_set_render_draw_color($renderer, 50, 150, 50, 255);
    } else {
        sdl_set_render_draw_color($renderer, 70, 130, 180, 255);
    }

    // Abgerundeter Button
    sdl_rounded_box($renderer,
        $button['x'], $button['y'],
        $button['x'] + $button['w'], $button['y'] + $button['h'],
        15,
        $button['hovered'] ? 50 : 70,
        $button['hovered'] ? 150 : 130,
        $button['hovered'] ? 50 : 180,
        255
    );

    // Button-Rahmen (dunkler)
    sdl_set_render_draw_color($renderer, 40, 80, 120, 255);
    sdl_render_fill_rect($renderer, [
        'x' => $button['x'],
        'y' => $button['y'],
        'w' => $button['w'],
        'h' => 3
    ]);
    sdl_render_fill_rect($renderer, [
        'x' => $button['x'],
        'y' => $button['y'] + $button['h'] - 3,
        'w' => $button['w'],
        'h' => 3
    ]);
}

echo "Button-Beispiel läuft!\n";
echo "Bewege die Maus über den blauen Button.\n";
echo "Klicke auf den Button um das Fenster zu schließen.\n";

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

    // Hintergrund zeichnen (hellgrau)
    sdl_set_render_draw_color($renderer, 220, 220, 220, 255);
    sdl_render_clear($renderer);

    // Button zeichnen
    draw_button($renderer, $button);

    // Text-Bereich für "X Schließen" simulieren (schwarzes Rechteck in der Mitte)
    $text_x = $button['x'] + ($button['w'] - 100) / 2;
    $text_y = $button['y'] + ($button['h'] - 20) / 2;
    sdl_set_render_draw_color($renderer, 255, 255, 255, 255);
    sdl_render_fill_rect($renderer, [
        'x' => $text_x,
        'y' => $text_y,
        'w' => 100,
        'h' => 20
    ]);

    // Anzeigen
    sdl_render_present($renderer);

    // Kurze Pause um CPU zu schonen
    sdl_delay(16);  // ~60 FPS
}

echo "Fenster geschlossen.\n";
sdl_quit();
