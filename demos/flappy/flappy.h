#include "gep.h"
#include <time.h>

DLLEXPORT const char *ORG_CONF_PATH = "gep";
DLLEXPORT const char *GAMENAME_CONF_PATH = "demo_flappy";
DLLEXPORT const char *GAMENAME = "gep flappy demo";

static u8 ColMap[BGMapW*BGMapH];

typedef enum { 
    mode_pre, 
    mode_game, 
    mode_death, 
    mode_score
} modes;

typedef enum {
    death_mode_hit,
    death_mode_flip_up,
    death_mode_down,
} death_mode_steps;

typedef struct {
    b32 Initialized;
    modes Mode;
    b32 ModeInitialized;
    b32 LeaveMode;
    // Pre Mode
    
    // Game Mode
    int NextPipeX;
    int NextPipeChunkX;
    
    int BGNoteIndex;
    int BGNoteFrames;
    
    // Death Mode
    death_mode_steps DeathMode;
    b32 DeathModeInitial;
    int DeathModeCountdown;
} game_state;
static game_state Game;

typedef struct {
    b32 Visible;
    b32 FlipY;
    u8 SpriteIndexTopLeft;
    u8 SpriteIndexTopRight;
    u8 SpriteIndexBottomLeft;
    u8 SpriteIndexBottomRight;
    
    b32 Flap;
    int AnimationFrames;
    b32 FlapAnim;
    int x,y;
} player;
static player P;

#define PipeW 4
u16 PipeTileIndicesTopRim[PipeW] = {4, 5, 6, 7,};
u16 PipeTileIndicesShaft[PipeW] = {36, 37, 38, 39};


typedef enum {
    note_pause,
    note_c,
    note_d,
    note_e,
    note_f,
    note_g,
    note_a,
    note_b,
} enum_notes;

static int Notes[] = {
    [note_pause] = 0,
    [note_c] = 262,
    [note_d] = 294,
    [note_e] = 330,
    [note_f] = 349,
    [note_g] = 392,
    [note_a] = 440,
    [note_b] = 494,
};

static int BGMusic[] = {
    note_d, note_f, note_g, note_pause, note_d, note_f, note_a, note_g, note_pause,  note_d, note_f, note_g, note_pause, note_f, note_d, note_pause, note_pause, note_pause,
};

i8 TmpWave[] = {
    0,   0,  1,  1,  2,  3,  3,  4, 
    4,   4,  3,  3,  2,  1,  1,  0, 
    0,  -1, -2, -2, -3, -4, -4, -5, 
    -5, -5, -4, -4, -3, -2, -2, -1};

u16 CharLookup[128] = {
    ['A'] = 96,
    ['B'] = 97,
    ['C'] = 98,
    ['D'] = 99,
    ['E'] = 100,
    ['F'] = 101,
    ['G'] = 102,
    ['H'] = 103,
    ['I'] = 104,
    ['J'] = 105,
    ['K'] = 106,
    ['L'] = 107,
    ['M'] = 108,
    ['N'] = 109,
    ['O'] = 110,
    ['P'] = 111,
    ['Q'] = 112,
    ['R'] = 113,
    ['S'] = 114,
    ['T'] = 115,
    ['U'] = 116,
    ['V'] = 117,
    ['W'] = 118,
    ['X'] = 119,
    ['Y'] = 120,
    ['Z'] = 121,
    
    ['0'] = 128,
    ['1'] = 129,
    ['2'] = 130,
    ['3'] = 131,
    ['4'] = 132,
    ['5'] = 133,
    ['6'] = 134,
    ['7'] = 135,
    ['8'] = 136,
    ['9'] = 137,
    
    ['!'] = 122,
    ['?'] = 123,
};

static u16 TextBoxBorderLookup[] = {
    [TextBoxBorderTopLeft] = 8,
    [TextBoxBorderTop] = 9,
    [TextBoxBorderTopRight] = 10,
    [TextBoxBorderMiddleLeft] = 40,
    [TextBoxBorderMiddleRight] = 42,
    [TextBoxBorderBottomLeft] = 72,
    [TextBoxBorderBottom] = 73,
    [TextBoxBorderBottomRight] = 74,
};
