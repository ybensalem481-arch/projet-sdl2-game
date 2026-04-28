#ifndef FONCTION_H_INCLUDED
#define FONCTION_H_INCLUDED

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <string.h>

#define NUMM_FRAMES  20
#define NUMMM_FRAMES 9

struct Animation {
    SDL_Texture* anim[NUMM_FRAMES];
    SDL_Rect pos;
};

typedef struct {
    SDL_Texture* mini_image;
    SDL_Rect mini_position;
    SDL_Texture* player_image;
    SDL_Rect player_position;
    SDL_Rect minimap_player_position;
} Minimap;

typedef struct PLATFORM PLATFORM;
struct PLATFORM {
    SDL_Texture* image;
    int image_w, image_h;        /* store original size since textures don't carry it */
    SDL_Rect world_pos;
    SDL_Rect screen_pos;
};

/* animation */
int  init_animation (struct Animation* anim, SDL_Renderer* renderer);
void afficher_animation (struct Animation anim, SDL_Renderer* renderer);
void liberer_animation (struct Animation* anim);

int  init_animation2 (struct Animation* anim, SDL_Renderer* renderer);
void afficher_animation2(struct Animation anim, SDL_Renderer* renderer);
void liberer_animation2 (struct Animation* anim);

/* minimap */
void init_minimap    (Minimap* m, SDL_Renderer* renderer);
void MAJMinimap      (SDL_Rect posJoueur, Minimap* m, SDL_Rect camera, int redimensionnement);
void afficher_minimap(Minimap m, SDL_Renderer* renderer);
void liberer_minimap (Minimap* m);

/* pixel / collision */
SDL_Color GetPixel   (SDL_Surface* pSurface, int x, int y);
int  collisionPP     (SDL_Rect player_pos, SDL_Surface* background);
int  CollisionBB     (SDL_Rect character, SDL_Rect platform);

/* HUD */
void affichertemps  (int temps,   SDL_Renderer* renderer);
void affichertempsen(Uint32 startTime, SDL_Renderer* renderer);


/* platform */
void initializeplatform(PLATFORM* pt, SDL_Renderer* renderer);
void mves(SDL_Rect* pos, int* tempsp, int* tempsa, int* v);

/* helper used internally and in main */
SDL_Texture* loadTexture(const char* path, SDL_Renderer* renderer);

#endif
