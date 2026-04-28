#ifndef ENIGME_H
#define ENIGME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_QUESTIONS 100
#define MAX_TEXT 512
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

typedef struct {
    char question[MAX_TEXT];
    char reponse1[MAX_TEXT];
    char reponse2[MAX_TEXT];
    char reponse3[MAX_TEXT];
    int bonne_reponse;
    int etat;               // -1: echec, 0: non résolu, 1: succès
    int temps_restant;      // temps en secondes
    int num_question;
    
    // Champs pour l'animation et feedback
    int feedback;           // 0: pas de feedback, 1: feedback actif
    Uint32 feedback_time;   // Temps d'affichage du feedback
    int reponse_choisie;    // Réponse sélectionnée (1,2,3)
    
    // Textures pour l'affichage
    SDL_Texture* texture_question;
    SDL_Texture* texture_rep1;
    SDL_Texture* texture_rep2;
    SDL_Texture* texture_rep3;
    SDL_Texture* background_texture;  // Texture pour le background
    
    // Positions à l'écran
    SDL_Rect rect_question;
    SDL_Rect rect_rep1;
    SDL_Rect rect_rep2;
    SDL_Rect rect_rep3;
    SDL_Rect rect_background;  // Rectangle pour le background
} Enigme;

// Prototypes des fonctions
Enigme generer_enigme(const char* nom_fichier, SDL_Renderer* renderer, TTF_Font* font);
void afficher_enigme(SDL_Renderer* renderer, Enigme* e, TTF_Font* font);
int verifier_reponse(Enigme* e, int x, int y);
void liberer_enigme(Enigme* e);
void mettre_a_jour_temps(Enigme* e, int delta_temps);
void afficher_feedback(SDL_Renderer* renderer, Enigme* e, TTF_Font* font);
SDL_Texture* charger_background(SDL_Renderer* renderer, const char* chemin);
void jouer_son(Mix_Chunk* son);

#endif
