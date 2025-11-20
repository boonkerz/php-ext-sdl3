#ifndef PHP_SDL3_HELPER
#define PHP_SDL3_HELPER

#include <SDL3/SDL.h>
#include "math.h"

// Zeichnet einen gefüllten Viertel-Kreis mit Anti-Aliasing (filled quarter circle).
// quadrant: 0 = top-left, 1 = top-right, 2 = bottom-right, 3 = bottom-left
static void filled_quarter_circle(SDL_Renderer *renderer, int cx, int cy, int r, int quadrant) {
    if (r <= 0) return;

    // Get current draw color
    Uint8 r_col, g_col, b_col, a_col;
    SDL_GetRenderDrawColor(renderer, &r_col, &g_col, &b_col, &a_col);

    // Use scanline filling with antialiasing on the edge
    float radius = (float)r;

    // Iterate through each row of the quadrant
    for (int y = 0; y < r; y++) {
        // Calculate distance from center
        float dy = (float)y + 0.5f;
        float x_exact = sqrtf(fmaxf(0.0f, radius * radius - dy * dy));
        int x_max = (int)floorf(x_exact);
        float frac = x_exact - (float)x_max;

        // Draw the fully opaque pixels for this scanline
        for (int x = 0; x <= x_max; x++) {
            int px, py;
            switch (quadrant) {
                case 0: // top-left: draw from center going left and up
                    px = cx - x;
                    py = cy - y;
                    break;
                case 1: // top-right: draw from center going right and up
                    px = cx + x;
                    py = cy - y;
                    break;
                case 2: // bottom-right: draw from center going right and down
                    px = cx + x;
                    py = cy + y;
                    break;
                case 3: // bottom-left: draw from center going left and down
                    px = cx - x;
                    py = cy + y;
                    break;
            }
            SDL_SetRenderDrawColor(renderer, r_col, g_col, b_col, a_col);
            SDL_RenderPoint(renderer, px, py);
        }

        // Draw the antialiased edge pixel
        if (frac > 0.01f && x_max + 1 < r) {
            int px, py;
            switch (quadrant) {
                case 0: // top-left
                    px = cx - (x_max + 1);
                    py = cy - y;
                    break;
                case 1: // top-right
                    px = cx + (x_max + 1);
                    py = cy - y;
                    break;
                case 2: // bottom-right
                    px = cx + (x_max + 1);
                    py = cy + y;
                    break;
                case 3: // bottom-left
                    px = cx - (x_max + 1);
                    py = cy + y;
                    break;
            }
            Uint8 edge_alpha = (Uint8)(frac * a_col);
            SDL_SetRenderDrawColor(renderer, r_col, g_col, b_col, edge_alpha);
            SDL_RenderPoint(renderer, px, py);
        }
    }

    // Restore original draw color
    SDL_SetRenderDrawColor(renderer, r_col, g_col, b_col, a_col);
}

#endif
