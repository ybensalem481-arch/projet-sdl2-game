#ifndef PUZZLE_H
#define PUZZLE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>

#define WINDOW_W      960
#define WINDOW_H      600
#define SAMPLE_RATE   44100
#define CHUNK_SIZE    2048
#define PUZZLE_COUNT  5
#define GRID_SIZE     3
#define PIECE_SIZE    80
#define OPTION_PIECES 3
#define TIME_LIMIT_MS 5000

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef CLAMP_VAL
#define CLAMP_VAL(x, lo, hi) ((x) < (lo) ? (lo) : ((x) > (hi) ? (hi) : (x)))
#endif

#define color_gold()  ((SDL_Color){212, 160,  23, 255})
#define color_red()   ((SDL_Color){180,  20,  20, 255})
#define color_grey()  ((SDL_Color){ 90,  90,  90, 255})
#define color_white() ((SDL_Color){230, 230, 230, 255})

typedef struct {
    SDL_Window   *window;
    SDL_Renderer *renderer;
    TTF_Font     *font_large;
    TTF_Font     *font_medium;
    TTF_Font     *font_small;
    Mix_Chunk    *sfx_hover;
    Mix_Chunk    *sfx_click;
    int           volume;
    int           running;
    Uint32        frame_tick;
} Game;

typedef struct {
    SDL_Rect  rect;
    char      label[64];
    int       hovered;
    SDL_Color color_normal;
    SDL_Color color_hover;
    SDL_Color color_text;
} Button;

typedef struct {
    int       px;
    int       py;
    int       active;
    SDL_Rect  rect;
    SDL_Color color;
} PuzzlePiece;

typedef struct {
    SDL_Rect  rect;
    SDL_Color color;
    int       is_correct;
    int       dragging;
    int       drag_offset_x;
    int       drag_offset_y;
} OptionPiece;

typedef struct {
    int          puzzle_seeded;
    int          current_puzzle;
    PuzzlePiece  grid[GRID_SIZE][GRID_SIZE];
    OptionPiece  options[OPTION_PIECES];
    int          missing_px;
    int          missing_py;
    int          puzzle_solved;
    int          puzzle_failed;
    int          puzzle_timed_out;
    int          game_over;
    Button       btn_back;
    int          hover_played;
    Uint32       start_time;
    float        timer_progress;
    float        result_scale;
    int          result_fade;
    SDL_Texture *textures[PUZZLE_COUNT];
    int          option_src_char[OPTION_PIECES];
    int          option_src_x[OPTION_PIECES];
    int          option_src_y[OPTION_PIECES];
    int          consecutive_correct;
    int          consecutive_wrong;
    char         bonus_msg[64];
    char         puzzle_name[32];
} PuzzleState;

void puzzle_init(Game *g, PuzzleState *ps);
void puzzle_free(Game *g, PuzzleState *ps);
void puzzle_handle_event(Game *g, PuzzleState *ps, SDL_Event *e);
void puzzle_draw(Game *g, PuzzleState *ps);

#endif
