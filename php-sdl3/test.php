<?php

if (!extension_loaded('sdl3')) {
    echo "Die Erweiterung 'sdl3' ist nicht geladen.\n";
    // Optional: Versuch, sie dynamisch zu laden (funktioniert möglicherweise nicht in allen Umgebungen)
    if (function_exists('dl')) {
        if (!@dl('sdl3.so')) {
            echo "Konnte die Erweiterung nicht dynamisch laden.\n";
            exit(1);
        }
    } else {
        exit(1);
    }
}

echo "Die Erweiterung 'sdl3' ist geladen.\n";

if (function_exists('confirm_sdl3_compiled')) {
    echo "Funktion 'confirm_sdl3_compiled()' existiert.\n";
    $result = confirm_sdl3_compiled();
    echo "Ergebnis: ";
    var_dump($result);
} else {
    echo "FEHLER: Funktion 'confirm_sdl3_compiled()' existiert nicht!\n";
}

?>