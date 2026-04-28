#define _USE_MATH_DEFINES
#include "puzzle.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

Mix_Chunk *make_beep_chunk(int freq, int duration_ms, float vol_factor)
{
    int        samples, data_bytes, wav_size, i;
    float      dur, attack, release, t, env, s;
    Uint8     *wav;
    Sint16    *pcm;
    SDL_RWops *rw;
    Mix_Chunk *chunk;
    Uint32     val32;
    Uint16     val16;

    samples    = SAMPLE_RATE * duration_ms / 1000;
    data_bytes = samples * (int)sizeof(Sint16);
    wav_size   = 44 + data_bytes;
    wav        = (Uint8 *)malloc(wav_size);
    if (!wav) return NULL;

    dur     = (float)duration_ms / 1000.0f;
    attack  = 0.01f;
    release = SDL_min((float)duration_ms / 1000.0f * 0.5f, 0.08f);
    pcm     = (Sint16 *)(wav + 44);

    for (i = 0; i < samples; i++) {
        t = (float)i / SAMPLE_RATE;
        if (t < attack)
            env = t / attack;
        else if (t > dur - release)
            env = (dur - t) / release;
        else
            env = 1.0f;
        s      = sinf(2.0f * (float)M_PI * freq * t);
        pcm[i] = (Sint16)(s * env * 20000.0f * vol_factor);
    }

    wav[0]='R'; wav[1]='I'; wav[2]='F'; wav[3]='F';
    val32 = (Uint32)(wav_size - 8);
    wav[4]=(Uint8)(val32&0xFF); wav[5]=(Uint8)((val32>>8)&0xFF);
    wav[6]=(Uint8)((val32>>16)&0xFF); wav[7]=(Uint8)((val32>>24)&0xFF);
    wav[8]='W'; wav[9]='A'; wav[10]='V'; wav[11]='E';
    wav[12]='f'; wav[13]='m'; wav[14]='t'; wav[15]=' ';
    wav[16]=16; wav[17]=0; wav[18]=0; wav[19]=0;
    wav[20]=1; wav[21]=0;
    wav[22]=1; wav[23]=0;
    val32 = SAMPLE_RATE;
    wav[24]=(Uint8)(val32&0xFF); wav[25]=(Uint8)((val32>>8)&0xFF);
    wav[26]=(Uint8)((val32>>16)&0xFF); wav[27]=(Uint8)((val32>>24)&0xFF);
    val32 = SAMPLE_RATE * (Uint32)sizeof(Sint16);
    wav[28]=(Uint8)(val32&0xFF); wav[29]=(Uint8)((val32>>8)&0xFF);
    wav[30]=(Uint8)((val32>>16)&0xFF); wav[31]=(Uint8)((val32>>24)&0xFF);
    val16 = (Uint16)sizeof(Sint16);
    wav[32]=(Uint8)(val16&0xFF); wav[33]=(Uint8)((val16>>8)&0xFF);
    wav[34]=16; wav[35]=0;
    wav[36]='d'; wav[37]='a'; wav[38]='t'; wav[39]='a';
    val32 = (Uint32)data_bytes;
    wav[40]=(Uint8)(val32&0xFF); wav[41]=(Uint8)((val32>>8)&0xFF);
    wav[42]=(Uint8)((val32>>16)&0xFF); wav[43]=(Uint8)((val32>>24)&0xFF);

    rw    = SDL_RWFromConstMem(wav, wav_size);
    chunk = NULL;
    if (rw)
        chunk = Mix_LoadWAV_RW(rw, 1);
    free(wav);
    return chunk;
}

void draw_text(SDL_Renderer *r, TTF_Font *font, const char *text,
               SDL_Color color, int x, int y, int centered, int shadow)
{
    SDL_Surface *surf;
    SDL_Texture *tex;
    SDL_Rect     dst;
    SDL_Color    shcol;

    if (!font || !text) return;

    if (shadow) {
        shcol.r = 0; shcol.g = 0; shcol.b = 0; shcol.a = 180;
        surf = TTF_RenderUTF8_Blended(font, text, shcol);
        if (surf) {
            tex = SDL_CreateTextureFromSurface(r, surf);
            if (tex) {
                dst.w = surf->w; dst.h = surf->h;
                dst.x = centered ? x - surf->w / 2 + 2 : x + 2;
                dst.y = y + 2;
                SDL_RenderCopy(r, tex, NULL, &dst);
                SDL_DestroyTexture(tex);
            }
            SDL_FreeSurface(surf);
        }
    }

    surf = TTF_RenderUTF8_Blended(font, text, color);
    if (!surf) return;
    tex = SDL_CreateTextureFromSurface(r, surf);
    if (tex) {
        dst.w = surf->w; dst.h = surf->h;
        dst.x = centered ? x - surf->w / 2 : x;
        dst.y = y;
        SDL_RenderCopy(r, tex, NULL, &dst);
        SDL_DestroyTexture(tex);
    }
    SDL_FreeSurface(surf);
}

void draw_text_rotozoom(SDL_Renderer *r, TTF_Font *font, const char *text,
                        SDL_Color color, int cx, int cy, double angle, float scale)
{
    SDL_Surface *surf;
    SDL_Texture *tex;
    SDL_Rect     dst;
    SDL_Color    shcol;
    int          w, h;

    if (!font || !text) return;

    /* shadow */
    shcol.r = 0; shcol.g = 0; shcol.b = 0; shcol.a = 180;
    surf = TTF_RenderUTF8_Blended(font, text, shcol);
    if (surf) {
        tex = SDL_CreateTextureFromSurface(r, surf);
        if (tex) {
            w = (int)(surf->w * scale);
            h = (int)(surf->h * scale);
            dst.x = cx - w / 2 + 3;
            dst.y = cy - h / 2 + 3;
            dst.w = w;
            dst.h = h;
            SDL_RenderCopyEx(r, tex, NULL, &dst, angle, NULL, SDL_FLIP_NONE);
            SDL_DestroyTexture(tex);
        }
        SDL_FreeSurface(surf);
    }

    /* main text */
    surf = TTF_RenderUTF8_Blended(font, text, color);
    if (!surf) return;
    tex = SDL_CreateTextureFromSurface(r, surf);
    if (tex) {
        w = (int)(surf->w * scale);
        h = (int)(surf->h * scale);
        dst.x = cx - w / 2;
        dst.y = cy - h / 2;
        dst.w = w;
        dst.h = h;
        SDL_RenderCopyEx(r, tex, NULL, &dst, angle, NULL, SDL_FLIP_NONE);
        SDL_DestroyTexture(tex);
    }
    SDL_FreeSurface(surf);
}

void draw_background(SDL_Renderer *r, Uint32 tick)
{
    int   i, cx, cy, dy, dx, rad, sx, sy, bright;
    float phase, flk;
    Uint8 a;

    SDL_SetRenderDrawColor(r, 4, 4, 4, 255);
    SDL_RenderClear(r);

    srand(42);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    for (i = 0; i < 80; i++) {
        sx     = rand() % WINDOW_W;
        sy     = rand() % WINDOW_H;
        bright = 100 + rand() % 155;
        flk    = sinf((tick * 0.002f) + i * 0.7f) * 0.5f + 0.5f;
        a      = (Uint8)(bright * flk);
        SDL_SetRenderDrawColor(r, 255, 255, 255, a);
        SDL_RenderDrawPoint(r, sx, sy);
        if (i % 5 == 0)
            SDL_RenderDrawPoint(r, sx + 1, sy);
    }
    srand((unsigned)SDL_GetTicks());

    rad = 120;
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 180, 20, 20, 30);
    for (i = 0; i < 3; i++) {
        phase = tick * 0.001f + i * 2.094f;
        cx    = WINDOW_W / 2 + (int)(cosf(phase) * 200);
        cy    = WINDOW_H / 2 + (int)(sinf(phase) * 100);
        for (dy = -rad; dy <= rad; dy++) {
            dx = (int)sqrt((double)(rad * rad - dy * dy));
            SDL_RenderDrawLine(r, cx - dx, cy + dy, cx + dx, cy + dy);
        }
    }
}

void generate_puzzle(PuzzleState *ps)
{
    const char *names[PUZZLE_COUNT];
    SDL_Color   pcolors[PUZZLE_COUNT];
    SDL_Color   base_color, wrong_base;
    int         grid_start_x, grid_start_y;
    int         option_y, spacing, option_start_x;
    int         correct_index, color_var;
    int         i, x, y;

    names[0] = "THOR";
    names[1] = "HULK";
    names[2] = "IRON MAN";
    names[3] = "CAPTAIN AMERICA";
    names[4] = "THANOS";

    pcolors[0].r=100; pcolors[0].g=150; pcolors[0].b=220; pcolors[0].a=255;
    pcolors[1].r=80;  pcolors[1].g=180; pcolors[1].b=80;  pcolors[1].a=255;
    pcolors[2].r=200; pcolors[2].g=50;  pcolors[2].b=50;  pcolors[2].a=255;
    pcolors[3].r=50;  pcolors[3].g=100; pcolors[3].b=200; pcolors[3].a=255;
    pcolors[4].r=150; pcolors[4].g=100; pcolors[4].b=200; pcolors[4].a=255;

    if (!ps->puzzle_seeded) {
        srand((unsigned int)time(NULL));
        ps->puzzle_seeded = 1;
    }

    ps->current_puzzle = rand() % PUZZLE_COUNT;
    strncpy(ps->puzzle_name, names[ps->current_puzzle], 31);
    ps->puzzle_name[31] = '\0';

    base_color   = pcolors[ps->current_puzzle];
    grid_start_x = WINDOW_W / 2 - (GRID_SIZE * PIECE_SIZE) / 2;
    grid_start_y = 180;

    for (y = 0; y < GRID_SIZE; y++) {
        for (x = 0; x < GRID_SIZE; x++) {
            ps->grid[y][x].px      = x;
            ps->grid[y][x].py      = y;
            ps->grid[y][x].active  = 1;
            ps->grid[y][x].rect.x  = grid_start_x + x * PIECE_SIZE;
            ps->grid[y][x].rect.y  = grid_start_y + y * PIECE_SIZE;
            ps->grid[y][x].rect.w  = PIECE_SIZE;
            ps->grid[y][x].rect.h  = PIECE_SIZE;
            color_var              = (x + y) * 15;
            ps->grid[y][x].color.r = (Uint8)CLAMP_VAL(base_color.r + color_var, 0, 255);
            ps->grid[y][x].color.g = (Uint8)CLAMP_VAL(base_color.g + color_var, 0, 255);
            ps->grid[y][x].color.b = (Uint8)CLAMP_VAL(base_color.b + color_var, 0, 255);
            ps->grid[y][x].color.a = 255;
        }
    }

    do {
        ps->missing_px = rand() % GRID_SIZE;
        ps->missing_py = rand() % GRID_SIZE;
    } while (ps->missing_px == 1 && ps->missing_py == 1);

    ps->grid[ps->missing_py][ps->missing_px].active = 0;

    option_y       = grid_start_y + GRID_SIZE * PIECE_SIZE + 60;
    spacing        = 120;
    option_start_x = WINDOW_W / 2 - (OPTION_PIECES * spacing) / 2 + 40;
    correct_index  = rand() % OPTION_PIECES;

    for (i = 0; i < OPTION_PIECES; i++) {
        ps->options[i].rect.x   = option_start_x + i * spacing;
        ps->options[i].rect.y   = option_y;
        ps->options[i].rect.w   = PIECE_SIZE;
        ps->options[i].rect.h   = PIECE_SIZE;
        ps->options[i].dragging = 0;

        if (i == correct_index) {
            ps->options[i].color      = ps->grid[ps->missing_py][ps->missing_px].color;
            ps->options[i].is_correct = 1;
            ps->option_src_char[i]    = ps->current_puzzle;
            ps->option_src_x[i]       = ps->missing_px;
            ps->option_src_y[i]       = ps->missing_py;
        } else {
            wrong_base = pcolors[(ps->current_puzzle + i + 1) % PUZZLE_COUNT];
            ps->options[i].color.r    = wrong_base.r;
            ps->options[i].color.g    = wrong_base.g;
            ps->options[i].color.b    = wrong_base.b;
            ps->options[i].color.a    = 255;
            ps->options[i].is_correct = 0;
            ps->option_src_char[i]    = (ps->current_puzzle + i + 1) % PUZZLE_COUNT;
            ps->option_src_x[i]       = ps->missing_px;
            ps->option_src_y[i]       = ps->missing_py;
        }
    }
}

void puzzle_init(Game *g, PuzzleState *ps)
{
    const char  *img_paths[PUZZLE_COUNT];
    SDL_Surface *surf;
    SDL_Color    cn, ch, ct;
    int          i;

    if (Mix_OpenAudio(SAMPLE_RATE, AUDIO_S16SYS, 1, CHUNK_SIZE) == 0) {
        Mix_AllocateChannels(8);
        g->sfx_hover = make_beep_chunk(880,  60, 0.4f);
        g->sfx_click = make_beep_chunk(1200, 80, 0.5f);
        Mix_VolumeMusic((int)(MIX_MAX_VOLUME * g->volume / 100.0f));
        Mix_Volume(-1, (int)(MIX_MAX_VOLUME * g->volume / 100.0f));
    }

    IMG_Init(IMG_INIT_PNG);

    img_paths[0] = "images/thor.png";
    img_paths[1] = "images/hulk.png";
    img_paths[2] = "images/ironman.png";
    img_paths[3] = "images/capamerica.png";
    img_paths[4] = "images/thanos.png";

    for (i = 0; i < PUZZLE_COUNT; i++) {
        ps->textures[i] = NULL;
        surf = IMG_Load(img_paths[i]);
        if (surf) {
            ps->textures[i] = SDL_CreateTextureFromSurface(g->renderer, surf);
            SDL_FreeSurface(surf);
        }
    }

    cn.r=28;  cn.g=28;  cn.b=32;  cn.a=220;
    ch.r=180; ch.g=20;  ch.b=20;  ch.a=240;
    ct.r=230; ct.g=230; ct.b=230; ct.a=255;

    ps->btn_back.rect.x      = 60;
    ps->btn_back.rect.y      = 30;
    ps->btn_back.rect.w      = 100;
    ps->btn_back.rect.h      = 34;
    ps->btn_back.hovered     = 0;
    ps->btn_back.color_normal = cn;
    ps->btn_back.color_hover  = ch;
    ps->btn_back.color_text   = ct;
    strncpy(ps->btn_back.label, "back", 63);
    ps->btn_back.label[63] = '\0';

    ps->puzzle_seeded       = 0;
    ps->puzzle_solved       = 0;
    ps->puzzle_failed       = 0;
    ps->puzzle_timed_out    = 0;
    ps->game_over           = 0;
    ps->consecutive_correct = 0;
    ps->consecutive_wrong   = 0;
    ps->bonus_msg[0]        = '\0';
    ps->hover_played        = 0;
    ps->result_scale        = 1.0f;
    ps->result_fade         = 0;

    generate_puzzle(ps);
    ps->start_time = SDL_GetTicks();
}

void puzzle_free(Game *g, PuzzleState *ps)
{
    int i;
    if (g->sfx_hover) { Mix_FreeChunk(g->sfx_hover); g->sfx_hover = NULL; }
    if (g->sfx_click) { Mix_FreeChunk(g->sfx_click); g->sfx_click = NULL; }
    for (i = 0; i < PUZZLE_COUNT; i++) {
        if (ps->textures[i]) {
            SDL_DestroyTexture(ps->textures[i]);
            ps->textures[i] = NULL;
        }
    }
    IMG_Quit();
    Mix_CloseAudio();
}

void puzzle_handle_event(Game *g, PuzzleState *ps, SDL_Event *e)
{
    int      mx, my, i, was_hovered;
    SDL_Rect missing_rect;

    if (e->type == SDL_KEYDOWN) {
        if (e->key.keysym.sym == SDLK_ESCAPE)
            g->running = 0;
        if (e->key.keysym.sym == SDLK_SPACE &&
            (ps->puzzle_solved || ps->puzzle_failed) && !ps->game_over) {
            generate_puzzle(ps);
            ps->puzzle_solved    = 0;
            ps->puzzle_failed    = 0;
            ps->puzzle_timed_out = 0;
            ps->result_fade      = 0;
            ps->start_time       = SDL_GetTicks();
        }
    }

    if (ps->puzzle_solved || ps->puzzle_failed) {
        if (e->type == SDL_MOUSEMOTION) {
            ps->hover_played = 0;
            mx = e->motion.x; my = e->motion.y;
            was_hovered = ps->btn_back.hovered;
            ps->btn_back.hovered =
                (mx >= ps->btn_back.rect.x &&
                 mx <= ps->btn_back.rect.x + ps->btn_back.rect.w &&
                 my >= ps->btn_back.rect.y &&
                 my <= ps->btn_back.rect.y + ps->btn_back.rect.h);
            if (ps->btn_back.hovered && !was_hovered && !ps->hover_played) {
                if (g->sfx_hover) Mix_PlayChannel(-1, g->sfx_hover, 0);
                ps->hover_played = 1;
            }
        }
        if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT) {
            mx = e->button.x; my = e->button.y;
            if (mx >= ps->btn_back.rect.x &&
                mx <= ps->btn_back.rect.x + ps->btn_back.rect.w &&
                my >= ps->btn_back.rect.y &&
                my <= ps->btn_back.rect.y + ps->btn_back.rect.h)
                g->running = 0;
        }
        return;
    }

    if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT) {
        mx = e->button.x; my = e->button.y;
        if (mx >= ps->btn_back.rect.x &&
            mx <= ps->btn_back.rect.x + ps->btn_back.rect.w &&
            my >= ps->btn_back.rect.y &&
            my <= ps->btn_back.rect.y + ps->btn_back.rect.h) {
            g->running = 0;
            return;
        }
        for (i = 0; i < OPTION_PIECES; i++) {
            if (mx >= ps->options[i].rect.x &&
                mx <= ps->options[i].rect.x + ps->options[i].rect.w &&
                my >= ps->options[i].rect.y &&
                my <= ps->options[i].rect.y + ps->options[i].rect.h) {
                ps->options[i].dragging      = 1;
                ps->options[i].drag_offset_x = mx - ps->options[i].rect.x;
                ps->options[i].drag_offset_y = my - ps->options[i].rect.y;
                if (g->sfx_click) Mix_PlayChannel(-1, g->sfx_click, 0);
                break;
            }
        }
    }

    if (e->type == SDL_MOUSEBUTTONUP && e->button.button == SDL_BUTTON_LEFT) {
        mx = e->button.x; my = e->button.y;
        for (i = 0; i < OPTION_PIECES; i++) {
            if (ps->options[i].dragging) {
                missing_rect = ps->grid[ps->missing_py][ps->missing_px].rect;
                if (mx >= missing_rect.x &&
                    mx <= missing_rect.x + missing_rect.w &&
                    my >= missing_rect.y &&
                    my <= missing_rect.y + missing_rect.h) {
                    if (ps->options[i].is_correct) {
                        ps->grid[ps->missing_py][ps->missing_px].active = 1;
                        ps->puzzle_solved  = 1;
                        ps->result_fade    = 255;
                        if (g->sfx_click) Mix_PlayChannel(-1, g->sfx_click, 0);
                        ps->consecutive_wrong = 0;
                        ps->consecutive_correct++;
                        if (ps->consecutive_correct == 3) {
                            strncpy(ps->bonus_msg, "You won an extra life", 63);
                            ps->bonus_msg[63]       = '\0';
                            ps->consecutive_correct = 0;
                        } else {
                            ps->bonus_msg[0] = '\0';
                        }
                    } else {
                        ps->puzzle_failed = 1;
                        ps->result_fade   = 255;
                        ps->consecutive_correct = 0;
                        ps->consecutive_wrong++;
                        if (ps->consecutive_wrong == 3) {
                            strncpy(ps->bonus_msg, "You lose", 63);
                            ps->bonus_msg[63] = '\0';
                            ps->game_over     = 1;
                        } else {
                            ps->bonus_msg[0] = '\0';
                        }
                    }
                }
                ps->options[i].dragging = 0;
            }
        }
    }

    if (e->type == SDL_MOUSEMOTION) {
        ps->hover_played = 0;
        mx = e->motion.x; my = e->motion.y;
        was_hovered = ps->btn_back.hovered;
        ps->btn_back.hovered =
            (mx >= ps->btn_back.rect.x &&
             mx <= ps->btn_back.rect.x + ps->btn_back.rect.w &&
             my >= ps->btn_back.rect.y &&
             my <= ps->btn_back.rect.y + ps->btn_back.rect.h);
        if (ps->btn_back.hovered && !was_hovered && !ps->hover_played) {
            if (g->sfx_hover) Mix_PlayChannel(-1, g->sfx_hover, 0);
            ps->hover_played = 1;
        }
        for (i = 0; i < OPTION_PIECES; i++) {
            if (ps->options[i].dragging) {
                ps->options[i].rect.x = mx - ps->options[i].drag_offset_x;
                ps->options[i].rect.y = my - ps->options[i].drag_offset_y;
            }
        }
    }
}

void puzzle_draw(Game *g, PuzzleState *ps)
{
    SDL_Renderer *r = g->renderer;
    SDL_Color     gold  = color_gold();
    SDL_Color     white = color_white();
    SDL_Color     red   = color_red();
    SDL_Color     green, timer_color, border_color, msg_color, hint_color, esc_color, bg, brd;
    SDL_Rect      panel, inner, title_line, empty, overlay, r_piece, shadow_rect, src_rect, dst;
    SDL_Surface  *surf;
    SDL_Texture  *tex;
    const char   *msg;
    char          lbl[8];
    int           x, y, i, ww, angle, arc_angle, iw, ih;
    int           timer_cx, timer_cy, timer_radius;
    int           half, h, apex_x, apex_y, bl_x, bl_y, br_x, br_y;
    int           bar_y, bar_left, bar_right, notch_top, notch_bot, t;
    Uint32        elapsed;

    green.r=80;  green.g=220; green.b=80;  green.a=255;
    hint_color.r=150; hint_color.g=150; hint_color.b=150; hint_color.a=200;
    esc_color.r=100;  esc_color.g=100;  esc_color.b=100;  esc_color.a=180;

    draw_background(r, g->frame_tick);

    panel.x=100; panel.y=60; panel.w=760; panel.h=500;
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 18, 18, 22, 210);
    SDL_RenderFillRect(r, &panel);
    SDL_SetRenderDrawColor(r, gold.r, gold.g, gold.b, 200);
    SDL_RenderDrawRect(r, &panel);
    inner.x=panel.x+1; inner.y=panel.y+1; inner.w=panel.w-2; inner.h=panel.h-2;
    SDL_SetRenderDrawColor(r, 40, 40, 50, 120);
    SDL_RenderDrawRect(r, &inner);

    SDL_SetRenderDrawColor(r, gold.r, gold.g, gold.b, 255);
    half     = 28 / 2;
    h        = (int)(28 * 0.866f);
    apex_x   = WINDOW_W / 2;
    apex_y   = 80 - h * 2 / 3;
    bl_x     = WINDOW_W / 2 - half;
    bl_y     = 80 + h / 3;
    br_x     = WINDOW_W / 2 + half;
    br_y     = 80 + h / 3;
    for (t = -1; t <= 1; t++) {
        SDL_RenderDrawLine(r, apex_x+t, apex_y, bl_x+t, bl_y);
        SDL_RenderDrawLine(r, apex_x+t, apex_y, br_x+t, br_y);
        SDL_RenderDrawLine(r, bl_x+t,   bl_y,   br_x+t, br_y);
    }
    bar_y     = 80 - h / 8;
    bar_left  = WINDOW_W / 2 - half * 2 / 5;
    bar_right = WINDOW_W / 2 + half * 2 / 5;
    for (t = -1; t <= 1; t++)
        SDL_RenderDrawLine(r, bar_left, bar_y+t, bar_right, bar_y+t);
    notch_top = 80 - h * 2 / 5;
    notch_bot = 80 + h / 6;
    SDL_RenderDrawLine(r, WINDOW_W/2, notch_top, WINDOW_W/2, notch_bot);

    draw_text(r, g->font_large,  "PUZZLE",          gold, WINDOW_W/2, 90,  1, 1);
    draw_text(r, g->font_medium, ps->puzzle_name,   gold, WINDOW_W/2, 130, 1, 0);

    title_line.x=WINDOW_W/2-150; title_line.y=155; title_line.w=300; title_line.h=2;
    SDL_SetRenderDrawColor(r, gold.r, gold.g, gold.b, gold.a);
    SDL_RenderFillRect(r, &title_line);

    if (!ps->puzzle_solved && !ps->puzzle_failed) {
        elapsed            = SDL_GetTicks() - ps->start_time;
        ps->timer_progress = (float)elapsed / (float)TIME_LIMIT_MS;
        if (ps->timer_progress >= 1.0f) {
            ps->puzzle_failed    = 1;
            ps->puzzle_timed_out = 1;
            ps->game_over        = 1;
            ps->result_fade      = 255;
        }
    }

    timer_cx     = WINDOW_W - 150;
    timer_cy     = 120;
    timer_radius = 30;

    SDL_SetRenderDrawColor(r, 40, 40, 45, 255);
    for (ww = 0; ww < timer_radius; ww++) {
        for (angle = 0; angle < 360; angle++) {
            x = timer_cx + (int)(ww * cos(angle * M_PI / 180.0));
            y = timer_cy + (int)(ww * sin(angle * M_PI / 180.0));
            SDL_RenderDrawPoint(r, x, y);
        }
    }

    timer_color = (ps->timer_progress < 0.7f) ? green : red;
    SDL_SetRenderDrawColor(r, timer_color.r, timer_color.g, timer_color.b, timer_color.a);
    arc_angle = (int)(360 * ps->timer_progress);
    for (angle = 0; angle < arc_angle; angle++) {
        for (ww = timer_radius - 8; ww < timer_radius; ww++) {
            x = timer_cx + (int)(ww * cos((angle - 90) * M_PI / 180.0));
            y = timer_cy + (int)(ww * sin((angle - 90) * M_PI / 180.0));
            SDL_RenderDrawPoint(r, x, y);
        }
    }

    SDL_SetRenderDrawColor(r, 28, 28, 32, 255);
    for (ww = 0; ww < timer_radius - 10; ww++) {
        for (angle = 0; angle < 360; angle++) {
            x = timer_cx + (int)(ww * cos(angle * M_PI / 180.0));
            y = timer_cy + (int)(ww * sin(angle * M_PI / 180.0));
            SDL_RenderDrawPoint(r, x, y);
        }
    }

    for (y = 0; y < GRID_SIZE; y++) {
        for (x = 0; x < GRID_SIZE; x++) {
            if (ps->grid[y][x].active) {
                if (ps->textures[ps->current_puzzle]) {
                    SDL_QueryTexture(ps->textures[ps->current_puzzle], NULL, NULL, &iw, &ih);
                    src_rect.x = iw / GRID_SIZE * ps->grid[y][x].px;
                    src_rect.y = ih / GRID_SIZE * ps->grid[y][x].py;
                    src_rect.w = iw / GRID_SIZE;
                    src_rect.h = ih / GRID_SIZE;
                    SDL_RenderCopy(r, ps->textures[ps->current_puzzle], &src_rect, &ps->grid[y][x].rect);
                } else {
                    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
                    SDL_SetRenderDrawColor(r, ps->grid[y][x].color.r, ps->grid[y][x].color.g,
                                          ps->grid[y][x].color.b, ps->grid[y][x].color.a);
                    SDL_RenderFillRect(r, &ps->grid[y][x].rect);
                }
                SDL_SetRenderDrawColor(r, 20, 20, 25, 255);
                SDL_RenderDrawRect(r, &ps->grid[y][x].rect);
            } else {
                empty = ps->grid[y][x].rect;
                SDL_SetRenderDrawColor(r, 60, 60, 70, 180);
                SDL_RenderFillRect(r, &empty);
                SDL_SetRenderDrawColor(r, 120, 120, 130, 255);
                for (i = 0; i < empty.w; i += 10) {
                    SDL_RenderDrawPoint(r, empty.x + i, empty.y);
                    SDL_RenderDrawPoint(r, empty.x + i, empty.y + empty.h);
                }
                for (i = 0; i < empty.h; i += 10) {
                    SDL_RenderDrawPoint(r, empty.x, empty.y + i);
                    SDL_RenderDrawPoint(r, empty.x + empty.w, empty.y + i);
                }
                draw_text(r, g->font_small, "?", gold,
                          empty.x + PIECE_SIZE / 2, empty.y + PIECE_SIZE / 2, 1, 0);
            }
        }
    }

    draw_text(r, g->font_small, "Glissez la bonne piece dans l'espace vide",
              hint_color, WINDOW_W / 2, 440, 1, 0);

    for (i = 0; i < OPTION_PIECES; i++) {
        r_piece = ps->options[i].rect;
        if (ps->options[i].dragging) {
            shadow_rect   = r_piece;
            shadow_rect.x += 4;
            shadow_rect.y += 4;
            SDL_SetRenderDrawColor(r, 0, 0, 0, 100);
            SDL_RenderFillRect(r, &shadow_rect);
        }
        if (ps->textures[ps->option_src_char[i]]) {
            SDL_QueryTexture(ps->textures[ps->option_src_char[i]], NULL, NULL, &iw, &ih);
            src_rect.x = iw / GRID_SIZE * ps->option_src_x[i];
            src_rect.y = ih / GRID_SIZE * ps->option_src_y[i];
            src_rect.w = iw / GRID_SIZE;
            src_rect.h = ih / GRID_SIZE;
            SDL_RenderCopy(r, ps->textures[ps->option_src_char[i]], &src_rect, &r_piece);
        } else {
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(r, ps->options[i].color.r, ps->options[i].color.g,
                                   ps->options[i].color.b, ps->options[i].color.a);
            SDL_RenderFillRect(r, &r_piece);
        }
        if (ps->options[i].dragging) {
            border_color = color_gold();
        } else {
            border_color.r=40; border_color.g=40; border_color.b=45; border_color.a=255;
        }
        SDL_SetRenderDrawColor(r, border_color.r, border_color.g, border_color.b, border_color.a);
        SDL_RenderDrawRect(r, &r_piece);
        snprintf(lbl, sizeof(lbl), "%c", 'A' + i);
        draw_text(r, g->font_small, lbl, white, r_piece.x + 8, r_piece.y + 4, 0, 0);
    }

    if (ps->puzzle_solved || ps->puzzle_failed) {
        if (ps->result_fade > 10) ps->result_fade -= 10;
        ps->result_scale = 1.0f + 0.2f * sinf(
            (float)(SDL_GetTicks() % 1000) / 1000.0f * 2.0f * (float)M_PI);

        if (ps->puzzle_solved) {
            msg_color = green;
            msg       = "Success";
        } else if (ps->puzzle_timed_out) {
            msg_color = red;
            msg       = "Time perdu";
        } else {
            msg_color = red;
            msg       = "Fail";
        }

        SDL_SetRenderDrawColor(r, 0, 0, 0, 150);
        overlay.x=0; overlay.y=0; overlay.w=WINDOW_W; overlay.h=WINDOW_H;
        SDL_RenderFillRect(r, &overlay);

        {
            double roto_angle = (double)(SDL_GetTicks() % 2000) / 2000.0 * 360.0;
            draw_text_rotozoom(r, g->font_large, msg, msg_color,
                               WINDOW_W / 2, WINDOW_H / 2 - 50,
                               roto_angle, ps->result_scale);
        }

        if (ps->bonus_msg[0] != '\0')
            draw_text(r, g->font_medium, ps->bonus_msg, gold, WINDOW_W/2, WINDOW_H/2 + 5, 1, 1);

        if (!ps->game_over)
            draw_text(r, g->font_medium, "Appuyez sur ESPACE pour continuer",
                      white, WINDOW_W/2, WINDOW_H/2 + 40, 1, 0);
        else
            draw_text(r, g->font_medium, "Partie terminee - ESC pour quitter",
                      white, WINDOW_W/2, WINDOW_H/2 + 40, 1, 0);
    }

    bg  = ps->btn_back.hovered ? ps->btn_back.color_hover : ps->btn_back.color_normal;
    brd = ps->btn_back.hovered ? color_gold() : color_grey();
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderFillRect(r, &ps->btn_back.rect);
    SDL_SetRenderDrawColor(r, brd.r, brd.g, brd.b, 255);
    SDL_RenderDrawRect(r, &ps->btn_back.rect);
    if (g->font_small && ps->btn_back.label[0]) {
        surf = TTF_RenderUTF8_Blended(g->font_small, ps->btn_back.label, ps->btn_back.color_text);
        if (surf) {
            tex = SDL_CreateTextureFromSurface(r, surf);
            if (tex) {
                dst.w = surf->w; dst.h = surf->h;
                dst.x = ps->btn_back.rect.x + (ps->btn_back.rect.w - dst.w) / 2;
                dst.y = ps->btn_back.rect.y + (ps->btn_back.rect.h - dst.h) / 2;
                SDL_RenderCopy(r, tex, NULL, &dst);
                SDL_DestroyTexture(tex);
            }
            SDL_FreeSurface(surf);
        }
    }

    draw_text(r, g->font_small, "ESC: Quitter   ESPACE: Nouveau puzzle",
              esc_color, WINDOW_W / 2, WINDOW_H - 24, 1, 0);
}
