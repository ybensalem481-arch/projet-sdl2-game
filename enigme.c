#include "enigme.h"

// Fonction pour charger le background
SDL_Texture* charger_background(SDL_Renderer* renderer, const char* chemin) {
    SDL_Surface* surface = IMG_Load(chemin);
    if (!surface) {
        printf("Erreur chargement background %s: %s\n", chemin, IMG_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

// Fonction pour jouer un son
void jouer_son(Mix_Chunk* son) {
    if (son != NULL) {
        Mix_PlayChannel(-1, son, 0);
    }
}

// Fonction pour décomposer une ligne du fichier (version robuste)
Enigme decomposer_ligne(char* ligne, SDL_Renderer* renderer, TTF_Font* font, int num_q) {
    Enigme e;
    int i = 0, j = 0, partie = 0;
    char buffer[MAX_TEXT];
    
    // Initialisation
    memset(&e, 0, sizeof(Enigme));
    e.num_question = num_q;
    e.etat = 0;
    e.temps_restant = 30;
    e.feedback = 0;
    e.feedback_time = 0;
    e.reponse_choisie = 0;
    e.background_texture = NULL;
    
    // Parcourir la ligne caractère par caractère
    while (ligne[i] != '\0' && ligne[i] != '\n') {
        if (ligne[i] == '?') {
            // Fin de la question
            buffer[j] = '\0';
            strcpy(e.question, buffer);
            j = 0;
            partie = 1;
            i++;
            continue;
        }
        else if (ligne[i] == '!') {
            // Fin d'une réponse
            buffer[j] = '\0';
            if (partie == 1) {
                strcpy(e.reponse1, buffer);
                partie = 2;
            } else if (partie == 2) {
                strcpy(e.reponse2, buffer);
                partie = 3;
            } else if (partie == 3) {
                strcpy(e.reponse3, buffer);
                partie = 4;
            }
            j = 0;
            i++;
            continue;
        }
        
        buffer[j++] = ligne[i++];
    }
    
    // La dernière partie est le numéro de la bonne réponse
    if (j > 0) {
        buffer[j] = '\0';
        e.bonne_reponse = atoi(buffer);
    }
    
    // Afficher les informations lues
    printf("\n════════════════════════════════════\n");
    printf("     Question chargee avec succes   \n");
    printf("════════════════════════════════════\n");
    printf(" Question: %s\n", e.question);
    printf("   1. %s\n", e.reponse1);
    printf("   2. %s\n", e.reponse2);
    printf("   3. %s\n", e.reponse3);
    printf(" Bonne reponse: %d\n", e.bonne_reponse);
    
    // Créer les textures pour l'affichage
    if (font != NULL && renderer != NULL) {
        SDL_Color color_blanc = {255, 255, 255, 255};
        SDL_Color color_jaune = {255, 215, 0, 255};
        
        // Question (en jaune doré)
        SDL_Surface* surface = TTF_RenderText_Blended(font, e.question, color_jaune);
        if (surface) {
            e.texture_question = SDL_CreateTextureFromSurface(renderer, surface);
            e.rect_question = (SDL_Rect){SCREEN_WIDTH/2 - surface->w/2, 50, surface->w, surface->h};
            SDL_FreeSurface(surface);
        }
        
        // Réponse 1 - centrée dans la zone
        surface = TTF_RenderText_Blended(font, e.reponse1, color_blanc);
        if (surface) {
            e.texture_rep1 = SDL_CreateTextureFromSurface(renderer, surface);
            int x_pos = 140 + (520 - surface->w) / 2;
            e.rect_rep1 = (SDL_Rect){x_pos, 205, surface->w, surface->h};
            SDL_FreeSurface(surface);
        }
        
        // Réponse 2 - centrée dans la zone
        surface = TTF_RenderText_Blended(font, e.reponse2, color_blanc);
        if (surface) {
            e.texture_rep2 = SDL_CreateTextureFromSurface(renderer, surface);
            int x_pos = 140 + (520 - surface->w) / 2;
            e.rect_rep2 = (SDL_Rect){x_pos, 285, surface->w, surface->h};
            SDL_FreeSurface(surface);
        }
        
        // Réponse 3 - centrée dans la zone
        surface = TTF_RenderText_Blended(font, e.reponse3, color_blanc);
        if (surface) {
            e.texture_rep3 = SDL_CreateTextureFromSurface(renderer, surface);
            int x_pos = 140 + (520 - surface->w) / 2;
            e.rect_rep3 = (SDL_Rect){x_pos, 365, surface->w, surface->h};
            SDL_FreeSurface(surface);
        }
    }
    
    return e;
}

// Fonction pour générer une énigme aléatoire
Enigme generer_enigme(const char* nom_fichier, SDL_Renderer* renderer, TTF_Font* font) {
    FILE* fichier;
    char lignes[MAX_QUESTIONS][MAX_TEXT];
    int nombre_questions = 0;
    int question_aleatoire;
    
    // Initialiser le générateur aléatoire
    srand(time(NULL));
    
    // Ouvrir le fichier
    fichier = fopen(nom_fichier, "r");
    if (fichier == NULL) {
        printf(" Erreur: Impossible d'ouvrir le fichier %s\n", nom_fichier);
        Enigme e_vide = {{0}};
        return e_vide;
    }
    
    // Lire toutes les lignes du fichier
    while (fgets(lignes[nombre_questions], MAX_TEXT, fichier) != NULL && 
           nombre_questions < MAX_QUESTIONS) {
        // Enlever le saut de ligne
        lignes[nombre_questions][strcspn(lignes[nombre_questions], "\n")] = 0;
        // Ignorer les lignes vides
        if (strlen(lignes[nombre_questions]) > 0) {
            nombre_questions++;
        }
    }
    fclose(fichier);
    
    if (nombre_questions == 0) {
        printf(" Erreur: Aucune question trouvee dans le fichier\n");
        Enigme e_vide = {{0}};
        return e_vide;
    }
    
    // Choisir une question aléatoire
    question_aleatoire = rand() % nombre_questions;
    printf("\n════════════════════════════════════\n");
    printf("   Chargement question %d/%d        \n", question_aleatoire + 1, nombre_questions);
    printf("════════════════════════════════════\n");
    
    // Décomposer la ligne choisie
    return decomposer_ligne(lignes[question_aleatoire], renderer, font, question_aleatoire + 1);
}

// Fonction pour afficher le feedback (message de résultat)
void afficher_feedback(SDL_Renderer* renderer, Enigme* e, TTF_Font* font) {
    if (!font || e->feedback == 0) return;
    
    // Vérifier si le feedback doit encore être affiché (2 secondes)
    if (SDL_GetTicks() - e->feedback_time > 2000) {
        e->feedback = 0;
        return;
    }
    
    SDL_Color color;
    const char* message;
    
    if (e->reponse_choisie == e->bonne_reponse) {
        color = (SDL_Color){0, 255, 0, 255};  // Vert
        message = " BRAVO ! BONNE REPONSE ! +10 points ";
    } else {
        color = (SDL_Color){255, 0, 0, 255};  // Rouge
        message = " DOMMAGE ! MAUVAISE REPONSE ! -10 points ";
    }
    
    // Fond semi-transparent pour le message
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_Rect fond_message = {SCREEN_WIDTH/2 - 300, 450, 600, 50};
    SDL_RenderFillRect(renderer, &fond_message);
    
    // Afficher le message
    SDL_Surface* surface = TTF_RenderText_Blended(font, message, color);
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_Rect rect = {SCREEN_WIDTH/2 - surface->w/2, 460, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, NULL, &rect);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    }
}

// Fonction pour afficher l'énigme
void afficher_enigme(SDL_Renderer* renderer, Enigme* e, TTF_Font* font) {
    if (e->etat != 0 && e->feedback == 0) return;
    
    // Afficher le background en premier
    if (e->background_texture) {
        SDL_RenderCopy(renderer, e->background_texture, NULL, &e->rect_background);
    } else {
        // Fallback: fond noir si pas de background
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 220);
        SDL_Rect fond = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderFillRect(renderer, &fond);
    }
    
    // Fond semi-transparent pour la zone question
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_Rect fond_question = {70, 40, 660, 80};
    SDL_RenderFillRect(renderer, &fond_question);
    
    // Cadre de la question
    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);  // Or
    SDL_Rect cadre_question = {80, 30, 640, 80};
    SDL_RenderDrawRect(renderer, &cadre_question);
    
    // Lignes décoratives du cadre
    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 100);
    for(int i = 1; i <= 3; i++) {
        SDL_Rect inner = {80 + i, 30 + i, 640 - 2*i, 80 - 2*i};
        SDL_RenderDrawRect(renderer, &inner);
    }
    
    // Zones de réponse (utilisées pour la détection des clics)
    SDL_Rect zone_rep1 = {140, 190, 520, 60};
    SDL_Rect zone_rep2 = {140, 270, 520, 60};
    SDL_Rect zone_rep3 = {140, 350, 520, 60};
    
    // Fond des réponses avec transparence
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
    SDL_RenderFillRect(renderer, &zone_rep1);
    SDL_RenderFillRect(renderer, &zone_rep2);
    SDL_RenderFillRect(renderer, &zone_rep3);
    
    // Bordures des réponses
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderDrawRect(renderer, &zone_rep1);
    SDL_RenderDrawRect(renderer, &zone_rep2);
    SDL_RenderDrawRect(renderer, &zone_rep3);
    
    // Surligner la réponse sélectionnée
    if (e->feedback == 1 && e->reponse_choisie > 0) {
        SDL_Rect rect_selected;
        if (e->reponse_choisie == 1) rect_selected = zone_rep1;
        else if (e->reponse_choisie == 2) rect_selected = zone_rep2;
        else rect_selected = zone_rep3;
        
        if (e->reponse_choisie == e->bonne_reponse) {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 150);  // Vert semi-transparent
        } else {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 150);  // Rouge semi-transparent
        }
        SDL_RenderFillRect(renderer, &rect_selected);
    }
    
    // Numéros des réponses
    if (font) {
        SDL_Color color_blanc = {255, 255, 255, 255};
        SDL_Surface* surf1 = TTF_RenderText_Blended(font, "1.", color_blanc);
        SDL_Surface* surf2 = TTF_RenderText_Blended(font, "2.", color_blanc);
        SDL_Surface* surf3 = TTF_RenderText_Blended(font, "3.", color_blanc);
        
        if (surf1) {
            SDL_Texture* tex1 = SDL_CreateTextureFromSurface(renderer, surf1);
            SDL_Rect rect1 = {120, 210, surf1->w, surf1->h};
            SDL_RenderCopy(renderer, tex1, NULL, &rect1);
            SDL_FreeSurface(surf1);
            SDL_DestroyTexture(tex1);
        }
        if (surf2) {
            SDL_Texture* tex2 = SDL_CreateTextureFromSurface(renderer, surf2);
            SDL_Rect rect2 = {120, 290, surf2->w, surf2->h};
            SDL_RenderCopy(renderer, tex2, NULL, &rect2);
            SDL_FreeSurface(surf2);
            SDL_DestroyTexture(tex2);
        }
        if (surf3) {
            SDL_Texture* tex3 = SDL_CreateTextureFromSurface(renderer, surf3);
            SDL_Rect rect3 = {120, 370, surf3->w, surf3->h};
            SDL_RenderCopy(renderer, tex3, NULL, &rect3);
            SDL_FreeSurface(surf3);
            SDL_DestroyTexture(tex3);
        }
    }
    
    // Afficher les textures des réponses
    if (e->texture_rep1) {
        SDL_RenderCopy(renderer, e->texture_rep1, NULL, &e->rect_rep1);
    }
    if (e->texture_rep2) {
        SDL_RenderCopy(renderer, e->texture_rep2, NULL, &e->rect_rep2);
    }
    if (e->texture_rep3) {
        SDL_RenderCopy(renderer, e->texture_rep3, NULL, &e->rect_rep3);
    }
    
    // Afficher la question
    if (e->texture_question) {
        SDL_RenderCopy(renderer, e->texture_question, NULL, &e->rect_question);
    }
    
    // Barre de temps visuelle
    int pourcentage = (e->temps_restant * 100) / 30;
    if (pourcentage < 0) pourcentage = 0;
    
    // Couleur de la barre selon le temps restant
    if (pourcentage > 60) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);  // Vert
    } else if (pourcentage > 30) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);  // Jaune
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // Rouge
    }
    
    SDL_Rect barre_temps = {100, 540, 600 * pourcentage / 100, 25};
    SDL_RenderFillRect(renderer, &barre_temps);
    
    // Contour de la barre
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect contour_temps = {100, 540, 600, 25};
    SDL_RenderDrawRect(renderer, &contour_temps);
    
    // Texte du temps
    if (font) {
        char timer_text[20];
        sprintf(timer_text, "Temps: %d s", e->temps_restant);
        SDL_Color color_blanc = {255, 255, 255, 255};
        SDL_Surface* surface = TTF_RenderText_Blended(font, timer_text, color_blanc);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_Rect rect = {710, 540, surface->w, surface->h};
            SDL_RenderCopy(renderer, texture, NULL, &rect);
            SDL_FreeSurface(surface);
            SDL_DestroyTexture(texture);
        }
    }
    
    // Afficher le message de feedback
    if (e->feedback == 1) {
        afficher_feedback(renderer, e, font);
    }
}

// Fonction pour vérifier la réponse
int verifier_reponse(Enigme* e, int x, int y) {
    int reponse = 0;
    
    // Définir les zones de clic (identiques aux zones d'affichage)
    SDL_Rect zone_rep1 = {140, 190, 520, 60};
    SDL_Rect zone_rep2 = {140, 270, 520, 60};
    SDL_Rect zone_rep3 = {140, 350, 520, 60};
    
    printf("\n════════════════════════════════════\n");
    printf("         DETECTION CLIC             \n");
    printf("════════════════════════════════════\n");
    printf(" Position du clic: (%d, %d)\n", x, y);
    printf(" Bonne reponse attendue: %d\n", e->bonne_reponse);
    printf(" Zone reponse 1: x=%d-%d, y=%d-%d\n", 
           zone_rep1.x, zone_rep1.x + zone_rep1.w,
           zone_rep1.y, zone_rep1.y + zone_rep1.h);
    printf(" Zone reponse 2: x=%d-%d, y=%d-%d\n",
           zone_rep2.x, zone_rep2.x + zone_rep2.w,
           zone_rep2.y, zone_rep2.y + zone_rep2.h);
    printf(" Zone reponse 3: x=%d-%d, y=%d-%d\n",
           zone_rep3.x, zone_rep3.x + zone_rep3.w,
           zone_rep3.y, zone_rep3.y + zone_rep3.h);
    
    // Vérifier réponse 1
    if (x >= zone_rep1.x && x <= zone_rep1.x + zone_rep1.w &&
        y >= zone_rep1.y && y <= zone_rep1.y + zone_rep1.h) {
        reponse = 1;
        printf("REPONSE 1 DETECTEE: %s\n", e->reponse1);
    }
    // Vérifier réponse 2
    else if (x >= zone_rep2.x && x <= zone_rep2.x + zone_rep2.w &&
             y >= zone_rep2.y && y <= zone_rep2.y + zone_rep2.h) {
        reponse = 2;
        printf(" REPONSE 2 DETECTEE: %s\n", e->reponse2);
    }
    // Vérifier réponse 3
    else if (x >= zone_rep3.x && x <= zone_rep3.x + zone_rep3.w &&
             y >= zone_rep3.y && y <= zone_rep3.y + zone_rep3.h) {
        reponse = 3;
        printf(" REPONSE 3 DETECTEE: %s\n", e->reponse3);
    }
    else {
        printf(" AUCUNE REPONSE DETECTEE\n");
        return -1;
    }
    
    e->reponse_choisie = reponse;
    e->feedback = 1;
    e->feedback_time = SDL_GetTicks();
    
    printf("\n Comparaison:\n");
    printf("   Reponse choisie: %d\n", reponse);
    printf("   Bonne reponse: %d\n", e->bonne_reponse);
    
    if (reponse == e->bonne_reponse) {
        e->etat = 1;
        printf("\n RESULTAT: BONNE REPONSE ! \n");
        return 1;
    } else {
        e->etat = -1;
        printf("\n RESULTAT: MAUVAISE REPONSE ! \n");
        return 0;
    }
}

// Fonction pour libérer les textures
void liberer_enigme(Enigme* e) {
    if (e->texture_question) {
        SDL_DestroyTexture(e->texture_question);
        e->texture_question = NULL;
    }
    if (e->texture_rep1) {
        SDL_DestroyTexture(e->texture_rep1);
        e->texture_rep1 = NULL;
    }
    if (e->texture_rep2) {
        SDL_DestroyTexture(e->texture_rep2);
        e->texture_rep2 = NULL;
    }
    if (e->texture_rep3) {
        SDL_DestroyTexture(e->texture_rep3);
        e->texture_rep3 = NULL;
    }
    if (e->background_texture) {
        SDL_DestroyTexture(e->background_texture);
        e->background_texture = NULL;
    }
}

// Mise à jour du temps
void mettre_a_jour_temps(Enigme* e, int delta_temps) {
    if (e->etat == 0 && e->feedback == 0) {
        e->temps_restant -= delta_temps;
        if (e->temps_restant <= 0) {
            e->etat = -1;
            e->temps_restant = 0;
            e->feedback = 1;
            e->feedback_time = SDL_GetTicks();
            e->reponse_choisie = 0;
            printf("\n TEMPS ECOULE ! ⏰\n");
        }
    }
}
