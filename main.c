#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include "enigme.h"

int main(int argc, char* argv[]) {
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    TTF_Font* font = NULL;
    Enigme enigme_courante;
    int niveau = 0;  // 0: jeu, 1: énigme
    int continuer = 1;
    int score = 100;
    int vie = 3;
    Uint32 temps_precedent = 0;
    int enigme_active = 1;
    int temps_affichage_message = 0;
    
    // Variables pour les sons
    Mix_Chunk* son_succes = NULL;
    Mix_Chunk* son_echec = NULL;
    
    // Initialisation SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("Erreur SDL: %s\n", SDL_GetError());
        return 1;
    }
    
    // Initialisation SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Erreur Mixer: %s\n", Mix_GetError());
    } else {
        // Charger les sons
        son_succes = Mix_LoadWAV("assets/success.wav");
        son_echec = Mix_LoadWAV("assets/fail.wav");
        
        if (!son_succes) printf("Son succès non chargé\n");
        if (!son_echec) printf("Son échec non chargé\n");
    }
    
    // Initialisation SDL_ttf
    if (TTF_Init() < 0) {
        printf("Erreur TTF: %s\n", TTF_GetError());
        if (son_succes) Mix_FreeChunk(son_succes);
        if (son_echec) Mix_FreeChunk(son_echec);
        Mix_CloseAudio();
        SDL_Quit();
        return 1;
    }
    
    // Initialisation SDL_image
    if (!(IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG) & (IMG_INIT_JPG | IMG_INIT_PNG))) {
        printf("Erreur IMG: %s\n", IMG_GetError());
    }
    
    // Création de la fenêtre
    window = SDL_CreateWindow("Jeu d'Enigme - Projet 1A", 
                              SDL_WINDOWPOS_CENTERED, 
                              SDL_WINDOWPOS_CENTERED,
                              SCREEN_WIDTH, SCREEN_HEIGHT, 
                              SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Erreur fenetre: %s\n", SDL_GetError());
        TTF_Quit();
        if (son_succes) Mix_FreeChunk(son_succes);
        if (son_echec) Mix_FreeChunk(son_echec);
        Mix_CloseAudio();
        SDL_Quit();
        return 1;
    }
    
    // Création du renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Erreur renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        if (son_succes) Mix_FreeChunk(son_succes);
        if (son_echec) Mix_FreeChunk(son_echec);
        Mix_CloseAudio();
        SDL_Quit();
        return 1;
    }
    
    // Chargement de la police
    font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 24);
    if (!font) {
        font = TTF_OpenFont("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf", 24);
    }
    if (!font) {
        font = TTF_OpenFont("/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf", 24);
    }
    if (!font) {
        printf("ATTENTION: Police non trouvée\n");
    }
    
    // Génération d'une première énigme
    enigme_courante = generer_enigme("enigme.txt", renderer, font);
    
    // Charger le background
    enigme_courante.background_texture = charger_background(renderer, "assets/background_menu.jpg");
    if (enigme_courante.background_texture) {
        enigme_courante.rect_background = (SDL_Rect){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        printf("Background charge avec succes!\n");
    } else {
        printf("Background non charge, utilisation du fond par defaut\n");
    }
    
    printf("\n════════════════════════════════════\n");
    printf("        JEU D ENIGME v2.0          \n");
    printf("════════════════════════════════════\n");
    printf("Appuyez sur ESPACE pour declencher une enigme\n");
    printf("Cliquez sur une reponse avec la souris\n");
    printf("Score initial: %d | Vies: %d\n\n", score, vie);
    
    temps_precedent = SDL_GetTicks();
    
    // Boucle principale
    while (continuer) {
        Uint32 temps_actuel = SDL_GetTicks();
        int delta_temps = (temps_actuel - temps_precedent) / 1000;
        if (delta_temps > 0) {
            temps_precedent = temps_actuel;
            if (niveau == 1 && enigme_active) {
                mettre_a_jour_temps(&enigme_courante, delta_temps);
            }
        }
        
        // Gestion des événements
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                continuer = 0;
            }
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    continuer = 0;
                }
                else if (event.key.keysym.sym == SDLK_SPACE && niveau == 0) {
                    // Déclencher une énigme
                    liberer_enigme(&enigme_courante);
                    enigme_courante = generer_enigme("enigme.txt", renderer, font);
                    // Recharger le background
                    enigme_courante.background_texture = charger_background(renderer, "assets/background_menu.jpg");
                    if (enigme_courante.background_texture) {
                        enigme_courante.rect_background = (SDL_Rect){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
                    }
                    niveau = 1;
                    enigme_active = 1;
                }
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN && niveau == 1 && enigme_active) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                int resultat = verifier_reponse(&enigme_courante, x, y);
                
                if (resultat == 1) {
                    printf("\n BONNE REPONSE! +10 points\n");
                    jouer_son(son_succes);
                    score += 10;
                    enigme_active = 0;
                    temps_affichage_message = SDL_GetTicks();
                }
                else if (resultat == 0) {
                    printf("\n MAUVAISE REPONSE! -10 points, -1 vie\n");
                    jouer_son(son_echec);
                    score -= 10;
                    vie--;
                    enigme_active = 0;
                    temps_affichage_message = SDL_GetTicks();
                    
                    if (score <= 0 || vie <= 0) {
                        printf("\n════════════════════════════════════\n");
                        printf("           GAME OVER !              \n");
                        printf("════════════════════════════════════\n");
                        printf("Score final: %d\n", score);
                        continuer = 0;
                    }
                }
            }
        }
        
        // Retour au jeu après affichage du feedback
        if (niveau == 1 && !enigme_active && temps_affichage_message > 0) {
            if (SDL_GetTicks() - temps_affichage_message > 2500) {
                niveau = 0;
                temps_affichage_message = 0;
                if (enigme_courante.etat == -1 && score > 0 && vie > 0) {
                    enigme_courante.etat = 0;
                    enigme_courante.temps_restant = 30;
                }
            }
        }
        
        // Vérifier le temps écoulé
        if (niveau == 1 && enigme_active && enigme_courante.etat == -1) {
            printf("\n TEMPS ECOULE! -5 points\n");
            jouer_son(son_echec);
            score -= 5;
            enigme_active = 0;
            temps_affichage_message = SDL_GetTicks();
            if (score <= 0) {
                printf("\n════════════════════════════════════\n");
                printf("           GAME OVER !              \n");
                printf("════════════════════════════════════\n");
                continuer = 0;
            }
        }
        
        // Rendu
        if (niveau == 0) {
            // Écran du jeu principal
            SDL_SetRenderDrawColor(renderer, 25, 25, 50, 255);
            SDL_RenderClear(renderer);
            
            if (font) {
                // Titre
                SDL_Color color_titre = {255, 215, 0, 255};
                SDL_Surface* surf_titre = TTF_RenderText_Blended(font, "=== JEU D ENIGME ===", color_titre);
                if (surf_titre) {
                    SDL_Texture* tex_titre = SDL_CreateTextureFromSurface(renderer, surf_titre);
                    SDL_Rect rect_titre = {SCREEN_WIDTH/2 - surf_titre->w/2, 50, surf_titre->w, surf_titre->h};
                    SDL_RenderCopy(renderer, tex_titre, NULL, &rect_titre);
                    SDL_FreeSurface(surf_titre);
                    SDL_DestroyTexture(tex_titre);
                }
                
                // Cadre pour les statistiques
                SDL_SetRenderDrawColor(renderer, 100, 100, 150, 255);
                SDL_Rect stats_cadre = {50, 120, 300, 120};
                SDL_RenderDrawRect(renderer, &stats_cadre);
                
                // Score
                char score_text[50];
                sprintf(score_text, " SCORE: %d", score);
                SDL_Color color_blanc = {255, 255, 255, 255};
                SDL_Surface* surf_score = TTF_RenderText_Blended(font, score_text, color_blanc);
                if (surf_score) {
                    SDL_Texture* tex_score = SDL_CreateTextureFromSurface(renderer, surf_score);
                    SDL_Rect rect_score = {70, 140, surf_score->w, surf_score->h};
                    SDL_RenderCopy(renderer, tex_score, NULL, &rect_score);
                    SDL_FreeSurface(surf_score);
                    SDL_DestroyTexture(tex_score);
                }
                
                // Vies
                char vie_text[50];
                sprintf(vie_text, " VIES: %d", vie);
                SDL_Surface* surf_vie = TTF_RenderText_Blended(font, vie_text, color_blanc);
                if (surf_vie) {
                    SDL_Texture* tex_vie = SDL_CreateTextureFromSurface(renderer, surf_vie);
                    SDL_Rect rect_vie = {70, 180, surf_vie->w, surf_vie->h};
                    SDL_RenderCopy(renderer, tex_vie, NULL, &rect_vie);
                    SDL_FreeSurface(surf_vie);
                    SDL_DestroyTexture(tex_vie);
                }
                
                // Instruction
                SDL_Color color_jaune = {255, 255, 100, 255};
                SDL_Surface* surf_space = TTF_RenderText_Blended(font, "Appuyez sur ESPACE pour une enigme!", color_jaune);
                if (surf_space) {
                    SDL_Texture* tex_space = SDL_CreateTextureFromSurface(renderer, surf_space);
                    SDL_Rect rect_space = {SCREEN_WIDTH/2 - surf_space->w/2, 400, surf_space->w, surf_space->h};
                    SDL_RenderCopy(renderer, tex_space, NULL, &rect_space);
                    SDL_FreeSurface(surf_space);
                    SDL_DestroyTexture(tex_space);
                }
                
                // Instructions supplémentaires
                SDL_Surface* surf_quit = TTF_RenderText_Blended(font, "ECHAP pour quitter", color_blanc);
                if (surf_quit) {
                    SDL_Texture* tex_quit = SDL_CreateTextureFromSurface(renderer, surf_quit);
                    SDL_Rect rect_quit = {SCREEN_WIDTH - 200, SCREEN_HEIGHT - 40, surf_quit->w, surf_quit->h};
                    SDL_RenderCopy(renderer, tex_quit, NULL, &rect_quit);
                    SDL_FreeSurface(surf_quit);
                    SDL_DestroyTexture(tex_quit);
                }
            }
        }
        else if (niveau == 1) {
            // Affichage de l'énigme
            if (enigme_active) {
                afficher_enigme(renderer, &enigme_courante, font);
            }
            else if (temps_affichage_message > 0) {
                afficher_enigme(renderer, &enigme_courante, font);
            }
        }
        
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
    
    // Nettoyage
    printf("\n════════════════════════════════════\n");
    printf("           FIN DU JEU               \n");
    printf("════════════════════════════════════\n");
    printf("Score final: %d\n", score);
    printf("Merci d avoir joue !\n");
    
    liberer_enigme(&enigme_courante);
    if (font) TTF_CloseFont(font);
    if (son_succes) Mix_FreeChunk(son_succes);
    if (son_echec) Mix_FreeChunk(son_echec);
    Mix_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    
    return 0;
}
