#include "puzzle.h"
#include <string.h>

int main(void)
{
    Game        g;
    PuzzleState ps;
    SDL_Event   e;
    const char *font_paths[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSansBold.ttf",
        "/usr/share/fonts/TTF/DejaVuSans-Bold.ttf",
        NULL
    };
    int i;

    memset(&g,  0, sizeof(g));
    memset(&ps, 0, sizeof(ps));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
        return 1;
    if (TTF_Init() < 0)
        return 1;

    g.window = SDL_CreateWindow("Lot 6 - Enigme Puzzle",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H, SDL_WINDOW_SHOWN);
    if (!g.window) return 1;

    g.renderer = SDL_CreateRenderer(g.window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!g.renderer) return 1;

    SDL_SetRenderDrawBlendMode(g.renderer, SDL_BLENDMODE_BLEND);

    for (i = 0; font_paths[i] && !g.font_large; i++) {
        g.font_large  = TTF_OpenFont(font_paths[i], 32);
        g.font_medium = TTF_OpenFont(font_paths[i], 20);
        g.font_small  = TTF_OpenFont(font_paths[i], 14);
    }

    g.volume     = 80;
    g.running    = 1;
    g.frame_tick = 0;

    puzzle_init(&g, &ps);

    while (g.running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                g.running = 0;
            puzzle_handle_event(&g, &ps, &e);
        }
        g.frame_tick = SDL_GetTicks();
        SDL_SetRenderDrawColor(g.renderer, 0, 0, 0, 255);
        SDL_RenderClear(g.renderer);
        puzzle_draw(&g, &ps);
        SDL_RenderPresent(g.renderer);
        SDL_Delay(16);
    }

    puzzle_free(&g, &ps);
    if (g.font_large)  TTF_CloseFont(g.font_large);
    if (g.font_medium) TTF_CloseFont(g.font_medium);
    if (g.font_small)  TTF_CloseFont(g.font_small);
    SDL_DestroyRenderer(g.renderer);
    SDL_DestroyWindow(g.window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
