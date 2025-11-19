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

// Font laden
$font = ttf_open_font('/usr/share/fonts/truetype/ubuntu/Ubuntu[wdth,wght].ttf', 24);
if (!$font) {
    echo 'Fehler beim Laden der Schriftart: ' . sdl_get_error() . "\n";
    sdl_quit();
    exit(1);
}

// Hauptfenster erstellen
$main_window = sdl_create_window('Hauptfenster', 640, 480);
if (!$main_window) {
    echo 'Fehler beim Erstellen des Hauptfensters: ' . sdl_get_error() . "\n";
    sdl_quit();
    exit(1);
}

$main_renderer = sdl_create_renderer($main_window);
if (!$main_renderer) {
    echo 'Fehler beim Erstellen des Hauptrenderers: ' . sdl_get_error() . "\n";
    sdl_quit();
    exit(1);
}

// Window IDs speichern
$main_window_id = sdl_get_window_id($main_window);

// Text-Texturen erstellen
function create_text_texture($renderer, $font, $text) {
    $surface = ttf_render_text_solid($font, $text, 255, 255, 255);
    if (!$surface) return false;

    $texture = sdl_create_texture_from_surface($renderer, $surface);
    return $texture;
}

// Hauptfenster Text-Texture
$main_text_texture = create_text_texture($main_renderer, $font, 'Fenster öffnen');
$main_text_size = ttf_size_text($font, 'Fenster öffnen');

// Sekundäres Fenster (initial NULL)
$second_window = null;
$second_window_id = null;
$second_renderer = null;
$second_text_texture = null;
$second_text_size = null;

// Hilfsfunktion: Prüft ob Punkt in Rechteck ist
function point_in_rect($x, $y, $rect) {
    return $x >= $rect['x'] &&
           $x <= $rect['x'] + $rect['w'] &&
           $y >= $rect['y'] &&
           $y <= $rect['y'] + $rect['h'];
}

// Zeichnet einen Button
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

// Button-Eigenschaften Hauptfenster
$main_button = [
    'x' => 200,
    'y' => 200,
    'w' => 240,
    'h' => 80,
    'hovered' => false
];

// Button-Eigenschaften zweites Fenster
$second_button = [
    'x' => 100,
    'y' => 150,
    'w' => 200,
    'h' => 80,
    'hovered' => false
];

echo "Multi-Window Beispiel läuft!\n";
echo "Hauptfenster: Klicke auf 'Fenster öffnen' um ein zweites Fenster zu öffnen.\n";
echo "Zweites Fenster: Klicke auf 'Schließen' um es zu schließen.\n";

// Hauptschleife
$main_running = true;
$second_running = false;

while ($main_running) {
    // Events verarbeiten
    while ($event = sdl_poll_event()) {
        // Quit-Event schließt alle Fenster
        if ($event['type'] === SDL_EVENT_QUIT) {
            $main_running = false;
            $second_running = false;
        }

        // Window Close Event - prüfe welches Fenster
        if ($event['type'] === SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
            if (isset($event['window_id'])) {
                if ($event['window_id'] === $main_window_id) {
                    echo "Hauptfenster schließen-Anfrage\n";
                    $main_running = false;
                    $second_running = false;
                } elseif ($second_running && $event['window_id'] === $second_window_id) {
                    echo "Zweites Fenster schließen-Anfrage\n";
                    $second_running = false;
                    $second_window = null;
                    $second_window_id = null;
                    $second_renderer = null;
                    $second_text_texture = null;
                    $second_button['hovered'] = false;
                }
            }
        }

        // Maus-Bewegung
        if ($event['type'] === SDL_EVENT_MOUSE_MOTION) {
            if (isset($event['window_id'])) {
                // Hauptfenster
                if ($event['window_id'] === $main_window_id) {
                    $main_button['hovered'] = point_in_rect($event['x'], $event['y'], $main_button);
                }
                // Zweites Fenster
                elseif ($second_running && $event['window_id'] === $second_window_id) {
                    $second_button['hovered'] = point_in_rect($event['x'], $event['y'], $second_button);
                }
            }
        }

        // Maus-Klick
        if ($event['type'] === SDL_EVENT_MOUSE_BUTTON_DOWN) {
            if ($event['button'] === SDL_BUTTON_LEFT && isset($event['window_id'])) {
                // Hauptfenster-Button: Öffne zweites Fenster
                if ($event['window_id'] === $main_window_id &&
                    point_in_rect($event['x'], $event['y'], $main_button)) {
                    if (!$second_running) {
                        echo "Öffne zweites Fenster...\n";

                        $second_window = sdl_create_window('Zweites Fenster', 400, 300);
                        if ($second_window) {
                            $second_window_id = sdl_get_window_id($second_window);
                            $second_renderer = sdl_create_renderer($second_window);
                            if ($second_renderer) {
                                $second_text_texture = create_text_texture($second_renderer, $font, 'Schließen');
                                $second_text_size = ttf_size_text($font, 'Schließen');
                                $second_running = true;
                            }
                        }
                    }
                }

                // Zweites Fenster-Button: Schließe zweites Fenster
                if ($second_running && $event['window_id'] === $second_window_id &&
                    point_in_rect($event['x'], $event['y'], $second_button)) {
                    echo "Schließe zweites Fenster...\n";
                    $second_running = false;
                    $second_window = null;
                    $second_window_id = null;
                    $second_renderer = null;
                    $second_text_texture = null;
                    $second_button['hovered'] = false;
                }
            }
        }
    }

    // Hauptfenster rendern
    sdl_set_render_draw_color($main_renderer, 45, 45, 48, 255);
    sdl_render_clear($main_renderer);
    draw_button($main_renderer, $main_button, $main_text_texture, $main_text_size);

    // Info-Text (weißes Rechteck als Platzhalter)
    sdl_set_render_draw_color($main_renderer, 80, 80, 80, 255);
    sdl_render_fill_rect($main_renderer, ['x' => 50, 'y' => 50, 'w' => 540, 'h' => 100]);

    sdl_render_present($main_renderer);

    // Zweites Fenster rendern (wenn offen)
    if ($second_running && $second_renderer) {
        sdl_set_render_draw_color($second_renderer, 60, 60, 65, 255);
        sdl_render_clear($second_renderer);
        draw_button($second_renderer, $second_button, $second_text_texture, $second_text_size);

        // Info-Text
        sdl_set_render_draw_color($second_renderer, 100, 100, 100, 255);
        sdl_render_fill_rect($second_renderer, ['x' => 50, 'y' => 50, 'w' => 300, 'h' => 60]);

        sdl_render_present($second_renderer);
    }

    // CPU schonen
    sdl_delay(16);  // ~60 FPS
}

echo "Programm beendet.\n";
sdl_quit();
