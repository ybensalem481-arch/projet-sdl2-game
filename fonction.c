#include "fonction.h"
#include <stdio.h>
#include <stdlib.h>

/* ── generic helper ───────────────────────────────────────────────────── */

SDL_Texture* loadTexture(const char* path, SDL_Renderer* renderer)
{
    SDL_Surface* surf = IMG_Load(path);
    if (!surf) { printf("IMG_Load error (%s): %s\n", path, IMG_GetError()); return NULL; }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    return tex;
}

/* helper: render a TTF surface at (x,y) then free it */
static void renderText(SDL_Surface* surf, SDL_Renderer* renderer, int x, int y)
{
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_Rect dst = { x, y, surf->w, surf->h };
    SDL_RenderCopy(renderer, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
    SDL_FreeSurface(surf);
}

/* ── animation 1 (NUMM_FRAMES = 20) ──────────────────────────────────── */

int init_animation(struct Animation* anim, SDL_Renderer* renderer)
{
    char filename[64];
    for (int i = 0; i < NUMM_FRAMES; ++i) {
        snprintf(filename, sizeof(filename), "animation/background%d.png", i);
        anim->anim[i] = loadTexture(filename, renderer);
        if (!anim->anim[i]) { printf("Error loading frame %d\n", i); return 0; }
    }
    return 1;
}

void afficher_animation(struct Animation anim, SDL_Renderer* renderer)
{
    static int i = 0;
    int w, h;
    SDL_QueryTexture(anim.anim[i], NULL, NULL, &w, &h);
    SDL_Rect dst = { anim.pos.x, anim.pos.y, w, h };
    SDL_RenderCopy(renderer, anim.anim[i], NULL, &dst);
    if (++i == NUMM_FRAMES) i = 0;
}

void liberer_animation(struct Animation* anim)
{
    for (int i = 0; i < NUMM_FRAMES; ++i) SDL_DestroyTexture(anim->anim[i]);
}

/* ── animation 2 (NUMMM_FRAMES = 9) ──────────────────────────────────── */

int init_animation2(struct Animation* anim, SDL_Renderer* renderer)
{
    char filename[64];
    for (int i = 0; i < NUMMM_FRAMES; ++i) {
        snprintf(filename, sizeof(filename), "animation2/background%d.png", i);
        anim->anim[i] = loadTexture(filename, renderer);
        if (!anim->anim[i]) { printf("Error loading frame %d\n", i); return 0; }
    }
    return 1;
}

void afficher_animation2(struct Animation anim, SDL_Renderer* renderer)
{
    static int i = 0;
    int w, h;
    SDL_QueryTexture(anim.anim[i], NULL, NULL, &w, &h);
    SDL_Rect dst = { anim.pos.x, anim.pos.y, w, h };
    SDL_RenderCopy(renderer, anim.anim[i], NULL, &dst);
    if (++i == NUMMM_FRAMES) i = 0;
}

void liberer_animation2(struct Animation* anim)
{
    for (int i = 0; i < NUMMM_FRAMES; ++i) SDL_DestroyTexture(anim->anim[i]);
}

/* ── minimap ──────────────────────────────────────────────────────────── */

void init_minimap(Minimap* m, SDL_Renderer* renderer)
{
    m->mini_image   = loadTexture("mini.jpg",   renderer);
    m->player_image = loadTexture("persop.png", renderer);

    m->mini_position.x = 680; m->mini_position.y = 0;
    m->mini_position.w = 369; m->mini_position.h = 123;

    m->player_position.x = 250; m->player_position.y = 10;
    m->minimap_player_position.x = m->mini_position.x + m->player_position.x;
    m->minimap_player_position.y = m->mini_position.y + m->player_position.y;
}

void MAJMinimap(SDL_Rect posJoueur, Minimap* m, SDL_Rect camera, int redimensionnement)
{
    int absX = posJoueur.x + camera.x;
    int absY = posJoueur.y + camera.y;
    m->minimap_player_position.x = (absX * redimensionnement / 100) + m->mini_position.x;
    m->minimap_player_position.y = (absY * redimensionnement / 100) + m->mini_position.y;
}

void afficher_minimap(Minimap m, SDL_Renderer* renderer)
{
    /* query sizes so the rects are correct */
    int w, h;

    SDL_QueryTexture(m.mini_image, NULL, NULL, &w, &h);
    SDL_Rect miniDst = m.mini_position;   /* already has x,y,w,h set */
    SDL_RenderCopy(renderer, m.mini_image, NULL, &miniDst);

    SDL_QueryTexture(m.player_image, NULL, NULL, &w, &h);
    SDL_Rect playerDst = { m.minimap_player_position.x, m.minimap_player_position.y, w, h };
    SDL_RenderCopy(renderer, m.player_image, NULL, &playerDst);
}

void liberer_minimap(Minimap* m)
{
    SDL_DestroyTexture(m->mini_image);
    SDL_DestroyTexture(m->player_image);
}

/* ── pixel-perfect collision (still uses SDL_Surface mask) ───────────── */

SDL_Color GetPixel(SDL_Surface* pSurface, int x, int y)
{
    SDL_Color color = {0,0,0,255};
    Uint32 col = 0;
    char* p = (char*)pSurface->pixels;
    p += pSurface->pitch * y;
    p += pSurface->format->BytesPerPixel * x;
    memcpy(&col, p, pSurface->format->BytesPerPixel);
    SDL_GetRGB(col, pSurface->format, &color.r, &color.g, &color.b);
    return color;
}

int collisionPP(SDL_Rect player_pos, SDL_Surface* background)
{
    int posX[8] = {
        player_pos.x, player_pos.x + player_pos.w/2, player_pos.x + player_pos.w,
        player_pos.x, player_pos.x,
        player_pos.x + player_pos.w/2, player_pos.x + player_pos.w, player_pos.x + player_pos.w
    };
    int posY[8] = {
        player_pos.y, player_pos.y, player_pos.y,
        player_pos.y + player_pos.h/2, player_pos.y + player_pos.h,
        player_pos.y + player_pos.h,   player_pos.y + player_pos.h, player_pos.y + player_pos.h/2
    };
    for (int i = 0; i < 8; i++) {
        SDL_Color c = GetPixel(background, posX[i], posY[i]);
        if (c.r == 255 && c.g == 255 && c.b == 255) return 1;
        if (c.r == 0   && c.g == 0   && c.b == 0  ) return 2;
    }
    return 0;
}

int CollisionBB(SDL_Rect character, SDL_Rect platform)
{
    if (character.x + character.w < platform.x) return 0;
    if (character.x > platform.x + platform.w)  return 0;
    if (character.y + character.h < platform.y)  return 0;
    if (character.y > platform.y + platform.h)   return 0;
    return 1;
}

/* ── HUD ──────────────────────────────────────────────────────────────── */

void affichertemps(int temps, SDL_Renderer* renderer)
{
    TTF_Font* font = TTF_OpenFont("04B_08__.TTF", 24);
    if (!font) return;
    SDL_Color color = {0, 0, 0, 255};
    char buf[32];
    sprintf(buf, "%02d:%02d:%02d", temps/3600, (temps%3600)/60, temps%60);
    SDL_Surface* surf = TTF_RenderText_Solid(font, buf, color);
    renderText(surf, renderer, 0, 0);
    TTF_CloseFont(font);
}

void affichertempsen(Uint32 startTime, SDL_Renderer* renderer)
{
    TTF_Font* font = TTF_OpenFont("04B_08__.TTF", 23);
    if (!font) return;
    SDL_Color color = {0, 0, 0, 255};
    int temps = 30000 - (int)(SDL_GetTicks() - startTime);
    if (temps < 0) temps = 0;
    char buf[32];
    sprintf(buf, "%02d:%02d", 0, temps / 1000);
    SDL_Surface* surf = TTF_RenderText_Solid(font, buf, color);
    renderText(surf, renderer, 20, 20);
    TTF_CloseFont(font);
}

/* ── platform ─────────────────────────────────────────────────────────── */

void initializeplatform(PLATFORM* pt, SDL_Renderer* renderer)
{
    pt->image = loadTexture("platform.png", renderer);
    SDL_QueryTexture(pt->image, NULL, NULL, &pt->image_w, &pt->image_h);

    pt->screen_pos.x = 800;
    pt->screen_pos.y = 200;
    pt->screen_pos.w = pt->image_w;
    pt->screen_pos.h = pt->image_h;
}

void mves(SDL_Rect* pos, int* tempsp, int* tempsa, int* v)
{
    *tempsa = SDL_GetTicks();
    if (*tempsa - *tempsp > 20) {
        if (pos->x <= 1200 && *v == 1) { pos->x += 15; if (pos->x >= 1200) *v = 2; }
        if (*v == 2)                    { pos->x -= 15; if (pos->x <= 800)  *v = 1; }
        *tempsp = *tempsa;
    }
}
