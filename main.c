#include <stdio.h>
#include <stdlib.h>
#include "fonction.h"

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);

    /* ── window + renderer (same size as original) ── */
    SDL_Window* window = SDL_CreateWindow(
        "Game",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1500, 500,
        SDL_WINDOW_SHOWN
    );
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    /* ── load assets ── */
    /* background and mask stay as SDL_Surface for pixel-perfect collision */
    SDL_Surface* masque     = IMG_Load("backmask.jpg");
    /* background and player rendered as textures */
    SDL_Texture* background = loadTexture("back.jpg",  renderer);
    SDL_Texture* player_tex = loadTexture("perso.png", renderer);
    int player_tex_w, player_tex_h;
    SDL_QueryTexture(player_tex, NULL, NULL, &player_tex_w, &player_tex_h);

    SDL_Rect player_pos = {600, 300, 29, 87};

    Minimap minimap;
    init_minimap(&minimap, renderer);

    PLATFORM pt;
    int tempsp = 0, tempsa = 0, v = 1;
    initializeplatform(&pt, renderer);

    struct Animation myAnimation, myAnimation2;
    if (!init_animation(&myAnimation, renderer) || !init_animation2(&myAnimation2, renderer)) {
        printf("Failed to initialize animations.\n");
        SDL_Quit(); return 1;
    }
    myAnimation.pos.x  = 50;  myAnimation.pos.y  = 50;
    myAnimation2.pos.x = 930; myAnimation2.pos.y = 25;

    Uint32 start_time = SDL_GetTicks();
    int running = 1, test = 0;
    Uint32 anim_display_start     = 0;
    Uint32 platform_collision_start = 0;
    int    platform_collision_flag  = 0;
    const Uint32 animation_duration = 3000;

    /* ── main loop ── */
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = 0;
                break;

            case SDL_KEYUP:
                switch (event.key.keysym.sym)
                {
                case SDLK_UP:
                    player_pos.y -= 10;
                    if (collisionPP(player_pos, masque) == 1 || CollisionBB(player_pos, pt.screen_pos))
                        player_pos.y += 10;
                    else if (collisionPP(player_pos, masque) == 2)
                        { player_pos.y += 10; test = 1; anim_display_start = SDL_GetTicks(); }
                    break;

                case SDLK_DOWN:
                    player_pos.y += 10;
                    if (collisionPP(player_pos, masque) == 1 || CollisionBB(player_pos, pt.screen_pos))
                        player_pos.y -= 10;
                    else if (collisionPP(player_pos, masque) == 2)
                        { player_pos.y -= 10; test = 1; anim_display_start = SDL_GetTicks(); }
                    break;

                case SDLK_LEFT:
                    player_pos.x -= 10;
                    if (collisionPP(player_pos, masque) == 1 || CollisionBB(player_pos, pt.screen_pos))
                        player_pos.x += 10;
                    else if (collisionPP(player_pos, masque) == 2)
                        { player_pos.x += 10; test = 1; anim_display_start = SDL_GetTicks(); }
                    break;

                case SDLK_RIGHT:
                    player_pos.x += 10;
                    if (collisionPP(player_pos, masque) == 1 || CollisionBB(player_pos, pt.screen_pos))
                        player_pos.x -= 10;
                    else if (collisionPP(player_pos, masque) == 2)
                        { player_pos.x -= 10; test = 1; anim_display_start = SDL_GetTicks(); }
                    break;

                }
                break;
            }
        }

        /* ── platform movement ── */
        mves(&pt.screen_pos, &tempsp, &tempsa, &v);

        /* ── platform push-out ── */
        if (CollisionBB(player_pos, pt.screen_pos)) {
            if (player_pos.x < pt.screen_pos.x)
                player_pos.x = pt.screen_pos.x - player_pos.w;
            else
                player_pos.x = pt.screen_pos.x + pt.screen_pos.w;

            if (!platform_collision_flag) {
                platform_collision_start = SDL_GetTicks();
                platform_collision_flag  = 1;
            }
        } else {
            platform_collision_flag = 0;
        }

        Uint32 current_time  = SDL_GetTicks();
        Uint32 elapsed_time  = current_time - start_time;

        /* ── render ── */
        SDL_RenderClear(renderer);

        /* background */
        SDL_RenderCopy(renderer, background, NULL, NULL);

        /* player */
        SDL_RenderCopy(renderer, player_tex, NULL, &player_pos);

        /* platform */
        SDL_RenderCopy(renderer, pt.image, NULL, &pt.screen_pos);

        /* minimap */
        MAJMinimap(player_pos, &minimap, player_pos, 11);
        afficher_minimap(minimap, renderer);

        /* HUD timers */
        affichertempsen(elapsed_time / 1000, renderer);  /* note: original passes elapsed/1000 */
        affichertemps  (elapsed_time / 1000, renderer);

        /* animations */
        if (platform_collision_flag &&
            (SDL_GetTicks() - platform_collision_start < 3000)) {
            afficher_animation2(myAnimation2, renderer);
            afficher_animation (myAnimation,  renderer);
        } else {
            platform_collision_flag = 0;
        }

        if (test == 1 &&
            (current_time - anim_display_start < animation_duration)) {
            afficher_animation2(myAnimation2, renderer);
            afficher_animation (myAnimation,  renderer);
        } else {
            test = 0;
        }

        SDL_RenderPresent(renderer);
    }

    /* ── cleanup ── */
    SDL_FreeSurface(masque);
    SDL_DestroyTexture(background);
    SDL_DestroyTexture(player_tex);
    liberer_minimap(&minimap);
    liberer_animation(&myAnimation);
    liberer_animation2(&myAnimation2);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
}
