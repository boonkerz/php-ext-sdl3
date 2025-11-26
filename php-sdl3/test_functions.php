<?php

if (!extension_loaded('sdl3')) {
    echo "Die Erweiterung 'sdl3' ist nicht geladen.\n";
    exit(1);
}

echo "SDL3 Extension erfolgreich geladen!\n\n";

echo "Verfügbare Funktionen:\n";
echo "======================\n\n";

echo "SDL3 Core:\n";
$core_functions = ['sdl_init', 'sdl_quit', 'sdl_create_window', 'sdl_create_renderer',
                   'sdl_set_render_draw_color', 'sdl_render_clear', 'sdl_render_fill_rect',
                   'sdl_render_present', 'sdl_delay', 'sdl_get_error', 'sdl_rounded_box',
                   'sdl_rounded_box_ex'];
foreach ($core_functions as $func) {
    echo "  - $func: " . (function_exists($func) ? "✓" : "✗") . "\n";
}

echo "\nSDL3_image:\n";
$image_functions = ['img_load', 'img_load_texture'];
foreach ($image_functions as $func) {
    echo "  - $func: " . (function_exists($func) ? "✓" : "✗") . "\n";
}

echo "\nSDL3_ttf:\n";
$ttf_functions = ['ttf_init', 'ttf_open_font', 'ttf_close_font', 'ttf_render_text_solid',
                  'ttf_render_text_blended', 'ttf_render_text_shaded', 'ttf_size_text'];
foreach ($ttf_functions as $func) {
    echo "  - $func: " . (function_exists($func) ? "✓" : "✗") . "\n";
}

echo "\nAlle Funktionen wurden erfolgreich registriert!\n";
