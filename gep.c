#include "config_file.c"

// Audio Internals
typedef struct {
    int FreqSweepTimer;
    int VolSweepTimer;
    int DutyStep; // position in the square wave
    int FreqTimer; // timer to advance square wave sample
    int SoundLenTimer; // timer which disables channel once it reaches zero
} aud_ch1_int;

typedef struct {
    int VolSweepTimer;
    int DutyStep; // position in the square wave
    int FreqTimer; // timer to advance square wave sample
    int SoundLenTimer; // timer which disables channel once it reaches zero
} aud_ch2_int;

typedef struct {
    int WaveStep; // position in the wave
    int FreqTimer; // timer to advance wave sample
    int SoundLenTimer; // timer which disables channel once it reaches zero
} aud_ch3_int;

typedef struct {
    int VolSweepTimer;
    int NoiseStep; // position in the square wave
    int FreqTimer; // timer to advance square wave sample
    int SoundLenTimer; // timer which disables channel once it reaches zero
} aud_ch4_int;

typedef struct {
    aud_ch1_int AudCh1Int;
    aud_ch2_int AudCh2Int;
    aud_ch3_int AudCh3Int;
    aud_ch4_int AudCh4Int;
} gep_state_int;

#include "settings_mode.c"

static u8 ScreenBuffer[ScreenPxW*ScreenPxH*4];

#define ScreenSurfByteDepth 3
static int ScreenScale = 5;

static sprite_attr SpritesOnLine[MaxSprites];
static int NumSpritesOnLine;

#define AudioSampleRate 48000
#define FrameAudio (AudioSampleRate/60)
#define AudioChannels 2
enum { DesiredAudioBufferSize = 4096 };
static i8 AudioBuffer[DesiredAudioBufferSize];

static const i8 SweepDir[] = {-1, 1};
static const i8 WaveDuty[4][8] = {
    {31,-31,-31,-31,-31,-31,-31,-31},
    {31, 31,-31,-31,-31,-31,-31,-31},
    {31, 31, 31, 31, -31,-31,-31,-31},
    {31, 31, 31, 31, 31, 31,-31,-31},
};
static const i8 WaveVolShift[4] = {
    [0] = 4,
    [1] = 0,
    [2] = 1,
    [3] = 2,
};

static const i8 Noise7[16] = {30,25,11,-25,9,-21,-30,24,13,-13,-15,-3,20,-25,10,-31};
static const i8 Noise15[4096] = {
    31,30,31,25,31,11,31,-24,30,15,25,0,10,31,-31,30,
    30,25,25,11,11,-24,-25,14,9,-6,-21,10,-31,-30,31,24,
    31,13,31,-12,30,-8,24,-16,12,0,-11,30,-30,24,24,13,
    13,-12,-13,-9,-15,-23,-3,-19,21,-11,-31,-25,31,10,31,-30,
    30,27,25,7,11,15,-26,-1,11,25,-25,10,10,-30,-31,26,
    29,1,19,27,-9,7,-21,13,-31,-12,31,-11,30,-26,24,3,
    13,23,-13,-17,-13,-7,-14,12,-13,-11,-10,-27,-29,4,17,5,
    -6,3,3,21,21,-30,-31,24,30,14,25,-6,10,11,-31,-25,
    31,9,31,-20,30,-24,25,15,8,0,-19,30,-11,25,-25,9,
    9,-20,-21,-25,-30,9,24,-23,12,-18,-10,-13,-31,-15,31,-2,
    30,19,24,-8,14,-16,-4,-1,7,24,13,15,-15,-2,-3,18,
    20,-15,-26,-1,11,26,-25,0,10,29,-31,18,30,-14,25,-4,
    9,7,-22,14,-29,-6,21,8,-30,-18,19,-15,-9,-3,-18,17,
    -13,-5,-14,5,-13,2,-11,16,-30,-3,24,21,15,-28,-3,22,
    22,-23,-22,-17,-23,-5,-18,2,-5,18,5,-13,0,-11,31,-30,
    30,24,25,13,11,-12,-25,-9,8,-22,-19,-21,-10,-31,-31,25,
    30,11,25,-24,10,15,-30,-1,26,25,0,11,30,-25,24,10,
    13,-30,-13,26,-14,0,-4,28,6,20,10,-25,-30,13,24,-12,
    12,-8,-11,-17,-27,-7,5,13,1,-14,25,-12,10,-11,-31,-27,
    28,5,21,3,-30,23,19,-19,-11,-11,-26,-30,11,27,-24,6,
    15,9,-2,-21,27,-31,7,30,12,24,-10,15,-28,-1,22,25,
    -23,11,-17,-27,-7,6,13,11,-14,-26,-12,2,-8,16,-17,-3,
    -6,21,3,-31,20,25,-25,8,9,-17,-21,-7,-31,13,25,-15,
    9,-2,-21,18,-31,-14,30,-7,27,13,6,-12,11,-10,-25,-20,
    8,-24,-18,14,-15,-7,-3,9,20,-21,-24,-29,15,18,-3,-15,
    21,-3,-30,22,26,-21,2,-30,18,26,-13,2,-14,19,-5,-9,
    2,-24,19,-16,-8,-4,-20,23,-8,-17,-19,-2,-10,19,-20,-10,
    -28,-28,7,20,15,-28,0,7,28,14,20,-5,-24,1,14,24,
    -4,12,4,-12,4,-25,4,13,5,-15,2,-7,16,9,-4,-24,
    23,-16,-16,-3,-4,22,23,-20,-19,-27,-9,6,-21,11,-31,-24,
    31,12,31,-10,30,-28,24,23,13,-16,-12,-1,-12,24,-24,12,
    12,-10,-11,-29,-27,16,5,-2,3,18,21,-12,-31,-11,31,-25,
    30,9,25,-20,10,-24,-30,14,24,-6,12,11,-12,-25,-24,8,
    15,-18,-1,-13,25,-15,9,-1,-21,24,-31,13,30,-15,24,-2,
    12,19,-11,-9,-26,-23,11,-18,-24,-13,12,-14,-8,-5,-17,0,
    -8,29,15,17,-3,-6,21,10,-31,-29,25,18,11,-14,-25,-5,
    13,1,-14,26,-12,0,-11,28,-27,20,6,-26,9,2,-23,19,
    -19,-8,-10,-20,-20,-8,-25,-16,13,-3,-13,20,-10,-27,-31,4,
    30,5,24,3,15,23,-2,-17,16,-7,-2,14,27,-4,5,4,
    2,4,19,7,-11,12,-31,-8,25,-16,10,0,-31,30,25,25,
    8,11,-18,-25,-13,9,-14,-22,-5,-21,1,-31,24,31,14,28,
    -6,20,11,-28,-24,7,12,15,-12,-1,-24,24,14,13,-5,-13,
    0,-16,28,0,20,28,-27,21,6,-31,9,28,-23,23,-18,-16,
    -12,-4,-12,23,-24,-17,12,-6,-10,10,-21,-31,-26,29,3,16,
    23,-3,-17,22,-2,-22,16,-28,-1,20,26,-24,1,12,24,-11,
    12,-25,-11,8,-26,-19,2,-10,17,-29,-6,18,11,-13,-25,-10,
    13,-28,-13,22,-14,-23,-4,-19,4,-12,6,-25,11,13,-24,-14,
    14,-4,-7,6,12,8,-9,-16,-18,-1,-13,25,-10,9,-31,-21,
    28,-30,21,27,-31,7,29,12,18,-10,-12,-29,-9,21,-22,-29,
    -20,17,-27,-7,5,14,1,-8,25,15,10,-3,-28,20,22,-26,
    -21,3,-29,20,16,-25,-1,9,26,-22,0,-23,29,-19,16,-9,
    -2,-23,27,-22,7,-23,12,-20,-9,-8,-22,-17,-28,-6,23,8,
    -19,-18,-14,-16,-5,0,5,30,2,27,18,5,-14,0,-6,31,
    3,30,20,27,-26,4,3,6,22,8,-21,-17,-26,-5,3,2,
    21,16,-30,-1,24,26,15,1,-3,26,20,0,-25,30,8,26,
    -17,0,-6,29,10,17,-29,-6,17,11,-7,-25,9,13,-20,-14,
    -25,-4,8,4,-20,4,-9,5,-19,1,-15,27,-6,5,8,2,
    -20,19,-8,-9,-20,-18,-8,-13,-17,-10,-2,-29,18,21,-14,-31,
    -4,31,4,30,5,27,3,5,23,1,-18,24,-4,14,4,-5,
    4,5,4,1,7,25,13,9,-15,-22,-3,-21,21,-31,-31,30,
    30,26,25,1,11,27,-25,6,10,9,-31,-21,31,-30,31,27,
    28,7,21,15,-30,0,19,28,-9,21,-21,-30,-30,27,24,4,
    13,5,-14,2,-13,16,-10,-3,-31,20,31,-26,28,3,22,23,
    -23,-16,-17,-3,-6,22,3,-21,20,-26,-25,0,10,30,-31,24,
    30,13,25,-12,10,-8,-31,-17,28,-6,21,11,-31,-24,25,12,
    11,-10,-25,-29,8,17,-18,-5,-13,1,-11,25,-30,10,24,-30,
    12,27,-10,6,-28,8,22,-18,-20,-12,-25,-11,13,-25,-13,8,
    -14,-19,-5,-11,1,-28,25,7,11,12,-26,-11,11,-26,-25,2,
    9,17,-21,-5,-30,1,19,24,-10,13,-31,-14,28,-4,20,7,
    -28,15,7,-3,13,20,-13,-24,-13,14,-13,-7,-15,12,-4,-11,
    23,-27,-17,6,-6,10,10,-29,-28,21,23,-28,-16,23,-3,-20,
    21,-8,-31,-19,31,-9,31,-22,29,-20,19,-24,-11,15,-25,-2,
    8,19,-19,-9,-10,-23,-20,-22,-24,-20,12,-27,-9,4,-21,4,
    -31,5,31,0,29,29,17,19,-7,-9,13,-19,-14,-14,-5,-5,
    5,0,2,31,19,29,-11,16,-26,0,3,31,21,30,-31,26,
    29,2,19,17,-9,-5,-22,5,-29,3,21,20,-30,-26,24,0,
    14,29,-7,18,14,-16,-7,0,9,30,-21,27,-29,7,17,12,
    -6,-11,3,-31,22,25,-22,8,-20,-17,-25,-5,9,2,-23,16,
    -22,-2,-24,16,-16,-1,-4,26,23,3,-19,20,-11,-26,-25,11,
    10,-24,-31,14,29,-6,18,11,-16,-25,0,13,31,-13,30,-13,
    24,-14,12,-4,-11,6,-28,8,7,-18,13,-14,-13,-12,-10,-9,
    -29,-23,16,-18,-2,-13,16,-10,0,-31,30,31,25,28,11,21,
    -24,-29,15,17,-3,-5,21,0,-31,30,25,26,8,1,-18,26,
    -13,1,-13,25,-15,10,-1,-31,24,28,13,22,-12,-23,-8,-19,
    -19,-11,-10,-27,-20,1,-24,27,14,4,-5,4,0,4,31,7,
    29,12,17,-10,-6,-29,10,21,-30,-30,27,27,4,7,5,14,
    2,-5,19,5,-10,0,-21,31,-26,31,0,28,29,20,19,-25,
    -8,9,-19,-22,-11,-21,-26,-26,0,3,30,22,24,-21,14,-30,
    -5,26,2,0,17,31,-5,28,2,23,17,-18,-4,-16,4,-1,
    7,29,14,17,-5,-6,1,10,25,-28,9,20,-20,-27,-24,5,
    12,0,-12,28,-24,20,12,-26,-10,2,-31,16,28,-2,22,19,
    -20,-8,-28,-19,7,-9,15,-24,-2,-16,19,-1,-12,26,-24,3,
    12,23,-11,-17,-25,-7,10,13,-29,-13,21,-14,-29,-4,16,4,
    -4,5,23,1,-19,24,-14,14,-7,-4,12,6,-12,11,-25,-25,
    13,9,-12,-21,-9,-31,-22,28,-23,21,-18,-31,-12,30,-11,27,
    -26,6,3,9,22,-21,-20,-29,-24,18,12,-13,-11,-15,-30,-3,
    26,21,0,-28,30,22,26,-21,1,-30,24,26,14,2,-6,16,
    10,-4,-30,23,19,-16,-11,-1,-26,29,11,19,-27,-9,5,-22,
    1,-22,27,-28,4,20,6,-28,9,7,-23,13,-20,-12,-8,-12,
    -17,-24,-7,15,13,-3,-13,17,-15,-5,-2,0,27,28,5,22,
    0,-24,29,-16,16,-3,-2,21,27,-30,4,24,6,12,9,-9,
    -21,-18,-31,-12,29,-11,17,-26,-5,3,1,21,26,-30,2,24,
    18,12,-14,-10,-5,-20,0,-26,29,11,16,-27,-3,5,21,1,
    -30,24,19,14,-11,-7,-27,9,4,-20,7,-26,15,11,-3,-26,
    20,2,-26,18,2,-13,18,-15,-12,-2,-9,27,-23,6,-17,9,
    -8,-24,15,-17,0,-2,28,18,23,-13,-20,-14,-8,-8,-17,15,
    -8,-1,15,29,-1,17,26,-5,1,2,25,16,9,-1,-24,24,
    -16,13,-3,-16,20,0,-27,28,4,22,6,-24,9,-17,-23,-2,
    -18,19,-4,-12,4,-25,7,13,15,-15,-1,-6,24,10,12,-29,
    -8,16,-16,-3,-1,21,29,-30,16,24,-1,13,25,-14,10,-7,
    -31,12,28,-10,23,-28,-17,23,-6,-20,11,-9,-28,-18,7,-12,
    15,-10,-2,-20,18,-25,-14,10,-7,-28,12,22,-10,-20,-28,-25,
    20,9,-25,-20,8,-27,-18,4,-15,5,-4,1,23,26,-18,2,
    -15,18,-3,-12,22,-9,-20,-21,-25,-29,9,18,-23,-15,-19,-2,
    -14,16,-5,0,2,30,19,27,-11,4,-26,7,2,15,19,-1,
    -10,26,-29,3,18,23,-15,-17,-1,-2,26,18,3,-13,20,-15,
    -24,-1,14,26,-7,0,14,28,-7,23,14,-18,-6,-4,8,6,
    -17,11,-3,-25,17,13,-4,-16,6,-1,8,29,-19,17,-9,-5,
    -23,5,-23,3,-23,20,-22,-25,-23,10,-17,-29,-7,18,13,-13,
    -13,-10,-10,-29,-29,16,17,-2,-5,19,5,-11,0,-31,31,25,
    31,8,31,-18,30,-12,25,-8,9,-16,-21,-1,-31,25,25,8,
    8,-18,-19,-13,-11,-15,-30,-6,26,11,0,-24,31,14,31,-5,
    30,1,24,27,15,7,-3,14,20,-7,-24,14,14,-5,-4,0,
    6,28,11,23,-27,-18,5,-12,0,-10,31,-20,30,-27,25,5,
    8,3,-20,22,-8,-23,-19,-22,-9,-21,-21,-26,-29,3,18,20,
    -15,-27,-1,1,26,26,0,2,30,16,24,-1,14,25,-4,10,
    4,-31,4,31,5,29,3,17,23,-6,-17,8,-3,-18,17,-4,
    -5,4,5,7,1,13,25,-14,9,-7,-21,12,-31,-10,31,-31,
    30,29,25,19,11,-8,-25,-17,13,-6,-13,10,-11,-31,-30,28,
    27,21,7,-28,15,22,-3,-20,21,-25,-31,10,30,-29,24,17,
    13,-4,-13,6,-11,8,-31,-19,25,-10,11,-28,-26,22,3,-22,
    23,-21,-19,-29,-9,18,-21,-12,-31,-8,25,-19,10,-10,-31,-29,
    25,17,11,-4,-25,6,13,9,-15,-21,-6,-31,11,28,-27,23,
    5,-16,3,-2,20,27,-24,4,12,6,-12,8,-25,-19,13,-10,
    -13,-29,-10,16,-28,-3,23,21,-18,-28,-15,20,-1,-24,25,14,
    8,-5,-19,0,-12,29,-24,17,12,-4,-11,6,-31,8,25,-18,
    9,-12,-21,-9,-26,-22,2,-23,17,-19,-7,-10,14,-21,-4,-26,
    7,2,12,19,-11,-10,-26,-29,11,17,-24,-5,15,1,-3,26,
    17,0,-7,30,12,27,-9,5,-22,2,-22,17,-28,-7,20,14,
    -25,-6,13,8,-14,-19,-12,-11,-9,-27,-18,4,-12,5,-10,1,
    -21,26,-26,1,0,24,28,12,22,-9,-23,-22,-19,-23,-10,-18,
    -31,-4,30,7,27,15,5,0,1,30,26,27,2,4,17,4,
    -6,6,3,11,21,-26,-30,0,27,30,6,25,10,10,-30,-28,
    26,23,1,-16,27,-1,4,26,7,0,12,31,-11,29,-25,18,
    9,-14,-21,-5,-26,1,2,24,19,12,-11,-10,-27,-20,4,-24,
    5,14,0,-5,28,5,20,0,-25,29,13,16,-15,-3,-3,20,
    17,-27,-7,6,14,11,-8,-26,15,2,0,19,30,-9,26,-21,
    3,-30,23,26,-19,2,-10,18,-29,-12,18,-8,-12,-17,-9,-2,
    -23,18,-22,-14,-24,-7,-16,14,-2,-6,27,3,6,20,11,-28,
    -27,7,6,15,8,-2,-16,27,-1,7,26,13,0,-15,31,-3,
    30,22,24,-22,14,-20,-5,-25,2,8,18,-16,-15,-4,-3,23,
    17,-17,-7,-6,14,3,-5,21,5,-30,0,19,30,-10,25,-31,
    9,29,-20,18,-24,-14,15,-7,-2,12,18,-11,-12,-26,-9,11,
    -22,-25,-21,9,-30,-23,26,-17,1,-6,24,10,15,-29,-2,16,
    19,-3,-9,22,-18,-20,-15,-25,-1,13,26,-13,0,-13,28,-15,
    20,-1,-27,25,4,8,6,-20,8,-9,-19,-18,-14,-13,-5,-10,
    5,-30,2,19,17,-10,-5,-31,5,31,3,29,23,17,-16,-7,
    -1,14,29,-4,17,4,-6,5,3,1,21,25,-30,8,24,-17,
    12,-6,-11,10,-31,-31,25,29,11,19,-24,-9,15,-22,-2,-21,
    19,-31,-11,30,-25,27,9,7,-20,14,-26,-6,11,8,-26,-19,
    11,-10,-25,-29,13,17,-12,-5,-9,0,-19,28,-14,21,-7,-30,
    13,26,-15,3,-2,22,18,-23,-13,-18,-13,-4,-14,6,-13,8,
    -11,-19,-30,-11,27,-26,5,3,3,22,22,-20,-21,-27,-29,6,
    18,10,-16,-31,0,25,31,10,31,-29,30,17,25,-4,11,7,
    -27,14,1,-6,25,10,10,-29,-28,16,23,-2,-17,19,-2,-12,
    16,-24,0,12,31,-8,30,-19,24,-10,13,-28,-14,22,-4,-23,
    7,-20,12,-9,-10,-18,-20,-13,-25,-14,13,-7,-13,12,-16,-11,
    0,-27,30,4,25,6,10,9,-29,-21,21,-30,-28,27,20,4,
    -26,5,2,0,19,28,-10,22,-31,-20,29,-24,16,15,-1,-1,
    24,29,12,16,-9,-3,-23,20,-22,-26,-23,0,-17,30,-7,26,
    14,3,-6,21,10,-30,-29,19,18,-8,-15,-17,-6,-2,11,18,
    -26,-12,0,-8,31,-17,30,-5,25,1,9,27,-22,6,-23,9,
    -20,-23,-8,-18,-16,-4,-4,7,23,13,-19,-15,-14,-2,-5,27,
    0,6,30,11,24,-27,15,5,0,2,30,16,27,-1,4,25,
    7,10,12,-29,-11,21,-26,-29,3,17,20,-5,-27,2,1,18,
    25,-15,8,-1,-16,24,-1,13,26,-14,0,-7,28,12,20,-9,
    -24,-22,14,-23,-6,-19,8,-12,-18,-24,-4,15,7,-3,14,17,
    -7,-6,14,10,-5,-28,5,22,3,-21,23,-26,-19,0,-9,30,
    -23,27,-17,7,-6,12,10,-9,-28,-18,22,-12,-23,-8,-22,-19,
    -21,-9,-31,-21,25,-29,11,17,-27,-5,5,1,1,26,25,3,
    8,20,-19,-27,-9,5,-21,1,-31,27,31,4,28,5,20,3,
    -25,23,13,-19,-15,-11,-2,-27,27,4,7,6,14,8,-5,-16,
    5,-2,3,27,21,5,-31,0,28,30,23,25,-19,11,-10,-28,
    -29,7,17,15,-6,-1,3,29,22,17,-21,-7,-31,14,25,-5,
    9,1,-22,26,-28,1,20,24,-27,13,6,-15,8,-4,-19,23,
    -11,-17,-25,-2,10,19,-28,-9,20,-22,-24,-20,15,-27,-3,4,
    22,4,-24,6,-17,10,-3,-30,17,19,-4,-12,7,-25,13,13,
    -12,-14,-9,-5,-23,0,-20,29,-8,16,-19,0,-11,31,-30,29,
    24,19,13,-8,-13,-17,-10,-7,-29,12,21,-10,-30,-28,26,20,
    1,-26,27,2,4,18,4,-16,6,-1,11,29,-25,17,10,-4,
    -31,6,25,9,9,-20,-22,-25,-20,9,-27,-23,4,-17,5,-8,
    0,15,31,-2,29,16,18,-2,-14,19,-12,-10,-12,-29,-24,16,
    15,-2,-1,18,29,-15,16,-1,0,25,30,9,26,-23,0,-18,
    29,-13,16,-13,0,-15,30,-6,24,8,12,-18,-8,-13,-17,-15,
    -2,-3,18,17,-15,-8,-2,15,27,-1,5,26,2,0,18,31,
    -15,28,-1,23,25,-17,11,-5,-28,0,7,29,13,18,-15,-12,
    -3,-9,17,-23,-5,-18,1,-5,24,5,15,0,-3,28,17,20,
    -7,-26,13,11,-15,-26,-3,2,21,16,-29,-1,18,26,-12,1,
    -11,25,-27,10,6,-30,8,26,-18,3,-12,23,-9,-18,-21,-13,
    -30,-14,19,-7,-9,13,-24,-14,-16,-4,-1,4,24,7,15,12,
    -3,-11,17,-27,-5,6,1,11,26,-26,3,0,23,28,-17,22,
    -5,-22,1,-29,24,21,14,-31,-6,28,8,20,-18,-24,-12,15,
    -11,-2,-27,18,4,-14,6,-6,11,3,-26,21,11,-28,-27,22,
    6,-22,9,-22,-23,-28,-17,23,-5,-20,1,-9,24,-18,15,-15,
    -1,-3,29,20,18,-25,-13,9,-13,-22,-15,-21,-2,-31,16,28,
    -1,22,25,-20,11,-27,-27,4,6,5,8,2,-17,19,-2,-9,
    16,-18,0,-16,31,0,29,28,18,21,-13,-29,-14,21,-7,-29,
    13,16,-15,0,-3,30,17,24,-7,14,13,-5,-13,5,-16,2,
    -1,16,29,-3,16,22,0,-23,28,-22,22,-23,-21,-18,-29,-15,
    18,-1,-12,25,-9,9,-21,-21,-31,-30,29,24,16,13,-2,-13,
    18,-10,-15,-31,-3,31,21,30,-28,26,23,2,-16,17,-1,-8,
    26,15,3,-3,21,20,-29,-25,18,10,-13,-31,-15,25,-2,10,
    19,-31,-9,30,-22,27,-20,7,-24,12,14,-9,-4,-23,6,-20,
    9,-9,-24,-18,-16,-12,-1,-12,29,-24,18,12,-14,-11,-5,-30,
    0,26,29,3,19,20,-9,-27,-21,1,-29,27,16,4,-1,5,
    24,1,15,24,-2,12,16,-11,-3,-26,20,11,-26,-27,2,6,
    17,8,-6,-18,3,-5,23,5,-19,0,-15,30,-6,27,8,6,
    -18,11,-14,-25,-12,13,-8,-13,-17,-15,-7,-3,12,17,-11,-6,
    -26,10,11,-30,-26,26,3,1,23,26,-17,2,-5,18,0,-13,
    30,-10,27,-31,6,29,9,18,-20,-12,-25,-8,13,-19,-13,-11,
    -15,-27,-6,4,11,4,-26,7,11,15,-27,-1,1,25,26,10,
    2,-29,16,16,-2,-2,19,27,-10,4,-31,7,28,15,23,0,
    -18,31,-13,28,-13,23,-14,-17,-4,-7,4,12,7,-9,13,-19,
    -13,-14,-15,-5,-6,0,10,28,-28,23,20,-16,-26,0,0,28,
    31,20,28,-25,21,9,-31,-20,28,-27,21,5,-31,3,28,20,
    23,-26,-19,3,-9,20,-23,-24,-17,15,-5,-4,0,23,28,-18,
    22,-15,-22,-2,-28,16,22,-1,-22,25,-28,8,20,-17,-27,-6,
    5,8,1,-20,25,-8,11,-19,-26,-11,3,-26,21,2,-28,18,
    22,-13,-22,-14,-28,-7,22,14,-24,-6,-16,8,-2,-18,27,-4,
    7,4,13,4,-15,7,-7,14,9,-8,-22,15,-22,0,-28,28,
    22,22,-21,-22,-30,-23,24,-17,14,-6,-6,10,3,-31,21,31,
    -28,28,23,22,-16,-22,0,-24,28,-16,22,-3,-22,21,-28,-31,
    20,30,-25,25,9,8,-20,-19,-25,-10,9,-31,-22,28,-20,21,
    -24,-31,15,30,-3,24,21,12,-28,-9,22,-21,-23,-30,-18,24,
    -15,14,-2,-4,18,6,-16,10,-1,-28,29,22,19,-21,-8,-31,
    -19,25,-9,11,-22,-26,-21,3,-30,20,26,-25,2,9,18,-21,
    -15,-30,-2,19,16,-9,-3,-22,17,-28,-4,20,4,-25,5,13,
    0,-15,28,-6,20,8,-28,-18,7,-15,15,-4,-2,23,18,-17,
    -13,-6,-14,3,-13,22,-10,-23,-31,-19,30,-10,26,-28,3,23,
    23,-17,-16,-5,-4,2,23,19,-18,-11,-16,-26,0,11,31,-25,
    30,10,25,-30,10,27,-30,6,27,9,6,-20,11,-26,-24,11,
    12,-24,-11,14,-26,-7,2,13,16,-13,0,-14,30,-12,24,-11,
    12,-26,-11,2,-26,16,2,-2,18,18,-12,-14,-12,-12,-24,-9,
    14,-22,-7,-21,13,-31,-15,31,-1,30,25,24,11,14,-24,-7,
    14,13,-8,-13,15,-16,-1,0,24,30,12,26,-9,0,-22,28,
    -21,21,-29,-31,17,30,-7,25,13,9,-12,-22,-9,-21,-22,-31,
    -23,29,-17,16,-6,-2,11,27,-26,5,0,3,28,22,23,-21,
    -19,-30,-9,24,-21,15,-30,0,26,28,0,21,30,-29,25,18,
    8,-14,-19,-5,-14,1,-6,25,3,10,20,-28,-27,20,5,-25,
    3,8,20,-16,-27,-3,5,22,1,-24,24,-16,14,-3,-6,20,
    3,-28,20,7,-25,12,8,-9,-16,-23,-1,-19,25,-14,8,-7,
    -16,12,-2,-11,27,-30,6,24,9,12,-20,-8,-25,-16,8,-3,
    -19,20,-14,-27,-7,1,14,26,-7,3,14,22,-7,-20,14,-26,
    -5,11,2,-26,16,11,-2,-28,18,7,-14,12,-6,-8,3,-18,
    22,-4,-23,4,-23,6,-23,10,-23,-29,-22,18,-20,-13,-25,-13,
    13,-13,-13,-15,-15,-3,-3,20,20,-27,-25,6,10,10,-31,-31};

void input_begin_frame_reset(void) {
    DevInput.mwd = 0;
    g->InUp = (gepinput){0};
    g->InDown = (gepinput){0};
}

#include "developer_tools.c"

void gep_update(void) {
    // Clip input struct values
    Clip(-1, g->AudCh1.VolSweeps, 7);
    Clip(0, g->AudCh1.Vol, 15);
    Clip(0, g->AudCh1.DutyChoice, 3);
    Clip(0, g->AudCh1.SoundLen, 63);
    Clip(-1, g->AudCh1.FreqSweeps, 7);
    Clip(0, g->AudCh1.FreqSweepTime, 7);
    
    Clip(-1, g->AudCh2.VolSweeps, 7);
    Clip(0, g->AudCh2.Vol, 15);
    Clip(0, g->AudCh2.DutyChoice, 3);
    Clip(0, g->AudCh2.SoundLen, 63);
    
    Clip(0, g->AudCh3.VolShift, 3);
    Clip(0, g->AudCh3.SoundLen, 255);
    pfori(ArrayLength(g->AudCh3.Wave)) {
        Clip(-7, g->AudCh3.Wave[i], 7);
    }
    
    Clip(0, g->AudCh4.SoundLen, 63);
    
    Clip(0, g->Audc.MasterVol, AudMaxMasterVol);
    
    enum {TimerDefault = -1};
    if(g->AudCh1.Reset) {
        g->AudCh1.Reset = 0;
        gi->AudCh1Int.DutyStep = 0;
        gi->AudCh1Int.FreqTimer = TimerDefault;
        gi->AudCh1Int.FreqSweepTimer = TimerDefault;
        gi->AudCh1Int.VolSweepTimer = TimerDefault;
        gi->AudCh1Int.SoundLenTimer = TimerDefault;
    }
    if(g->AudCh2.Reset) {
        g->AudCh2.Reset = 0;
        gi->AudCh2Int.DutyStep = 0;
        gi->AudCh2Int.FreqTimer = TimerDefault;
        gi->AudCh2Int.SoundLenTimer = TimerDefault;
        gi->AudCh2Int.VolSweepTimer = TimerDefault;
    }
    if(g->AudCh3.Reset) {
        g->AudCh3.Reset = 0;
        gi->AudCh3Int.WaveStep = 0;
        gi->AudCh3Int.FreqTimer = TimerDefault;
        gi->AudCh3Int.SoundLenTimer = TimerDefault;
    }
    if(g->AudCh4.Reset) {
        g->AudCh4.Reset = 0;
        gi->AudCh4Int.VolSweepTimer = TimerDefault;
        gi->AudCh4Int.FreqTimer = TimerDefault;
        gi->AudCh4Int.SoundLenTimer = TimerDefault;
    }
    
    BeforeAudio = SDL_GetPerformanceCounter();
    u32 QueueSize = SDL_GetQueuedAudioSize(AudioDev);
    if(QueueSize<DesiredAudioBufferSize) {
        u32 AddQueueSize = DesiredAudioBufferSize-QueueSize;
        for(u32 i=0; i<AddQueueSize; i+=2) {
            enum { FreqSweepBaseLenHz = 128 };
            enum { VolSweepBaseLenHz = 64 };
            enum { SoundLenBaseRateHz = 256 };
            
            i8 Ch1 = 0;
            if(g->AudCh1.Enabled && g->AudCh1.Hz) {
                if(g->AudCh1.UseSoundLen && gi->AudCh1Int.SoundLenTimer == TimerDefault) {
                    gi->AudCh1Int.SoundLenTimer = (AudioSampleRate/SoundLenBaseRateHz)*(64-g->AudCh1.SoundLen);
                } else if(g->AudCh1.UseSoundLen && gi->AudCh1Int.SoundLenTimer == (TimerDefault+1)) {
                    g->AudCh1.Enabled = 0;
                }
                
                if(g->AudCh1.FreqSweeps >= 0 && g->AudCh1.FreqSweepTime > 0 &&gi->AudCh1Int.FreqSweepTimer < 0) {
                    g->AudCh1.Hz = g->AudCh1.Hz + ((g->AudCh1.Hz >> g->AudCh1.FreqSweeps)*SweepDir[g->AudCh1.FreqSweepUp]);
                    gi->AudCh1Int.FreqSweepTimer = AudioSampleRate/(FreqSweepBaseLenHz/g->AudCh1.FreqSweepTime);
                    g->AudCh1.FreqSweeps--;
                }
                
                if(gi->AudCh1Int.FreqTimer < 0) {
                    int WaveLen = AudioSampleRate/g->AudCh1.Hz;
                    gi->AudCh1Int.FreqTimer = WaveLen/8;
                    gi->AudCh1Int.DutyStep = (gi->AudCh1Int.DutyStep+1)%8;
                }
                
                if(g->AudCh1.VolSweeps >= 0 && gi->AudCh1Int.VolSweepTimer < 0) {
                    gi->AudCh1Int.VolSweepTimer = (AudioSampleRate/VolSweepBaseLenHz)*g->AudCh1.VolSweeps;
                    g->AudCh1.VolSweeps--;
                    g->AudCh1.Vol += SweepDir[g->AudCh1.VolSweepUp];
                }
                
                Ch1 = (i8)(WaveDuty[g->AudCh1.DutyChoice][gi->AudCh1Int.DutyStep%8]*g->AudCh1.Vol/15);
                
                
                // this will never wrap
                gi->AudCh1Int.FreqTimer--;
                // these might wrap so we need to stop decrementing
                if(g->AudCh1.UseSoundLen) gi->AudCh1Int.SoundLenTimer--; 
                if(gi->AudCh1Int.VolSweepTimer > -1) gi->AudCh1Int.VolSweepTimer--; 
                if(gi->AudCh1Int.FreqSweepTimer > -1) gi->AudCh1Int.FreqSweepTimer--; 
            }
            
            i8 Ch2 = 0;
            if(g->AudCh2.Enabled && g->AudCh2.Hz) {
                if(g->AudCh2.UseSoundLen && gi->AudCh2Int.SoundLenTimer == TimerDefault) {
                    gi->AudCh2Int.SoundLenTimer = (AudioSampleRate/SoundLenBaseRateHz)*(64-g->AudCh2.SoundLen);
                } else if(g->AudCh2.UseSoundLen && gi->AudCh2Int.SoundLenTimer == (TimerDefault+1)) {
                    g->AudCh2.Enabled = 0;
                }
                
                if(gi->AudCh2Int.FreqTimer < 0) {
                    int WaveLen = AudioSampleRate/g->AudCh2.Hz;
                    gi->AudCh2Int.FreqTimer = WaveLen/8;
                    gi->AudCh2Int.DutyStep = (gi->AudCh2Int.DutyStep+1)%8;
                }
                
                if(g->AudCh2.VolSweeps >= 0 && gi->AudCh2Int.VolSweepTimer < 0) {
                    gi->AudCh2Int.VolSweepTimer = (AudioSampleRate/VolSweepBaseLenHz)*g->AudCh2.VolSweeps;
                    g->AudCh2.VolSweeps--;
                    g->AudCh2.Vol += SweepDir[g->AudCh2.VolSweepUp];
                }
                
                Ch2 = (i8)(WaveDuty[g->AudCh2.DutyChoice][gi->AudCh2Int.DutyStep%8]*g->AudCh2.Vol/15);
                
                // this will never wrap
                gi->AudCh2Int.FreqTimer--;
                // these  might wrap, so stop decrementing
                if(g->AudCh2.UseSoundLen) gi->AudCh2Int.SoundLenTimer--; 
                if(gi->AudCh2Int.VolSweepTimer > -1) gi->AudCh2Int.VolSweepTimer--;
            }
            
            i8 Ch3 = 0;
            if(g->AudCh3.Enabled && g->AudCh3.Hz) {
                if(g->AudCh3.UseSoundLen && gi->AudCh3Int.SoundLenTimer == TimerDefault) {
                    gi->AudCh3Int.SoundLenTimer = (AudioSampleRate/SoundLenBaseRateHz)*(256-g->AudCh3.SoundLen);
                } else if(g->AudCh3.UseSoundLen && gi->AudCh3Int.SoundLenTimer == (TimerDefault+1)) {
                    g->AudCh3.Enabled = 0;
                }
                
                if(gi->AudCh3Int.FreqTimer < 0) {
                    int WaveLen = AudioSampleRate/(g->AudCh3.Hz*2); 
                    gi->AudCh3Int.FreqTimer = WaveLen/8;
                    gi->AudCh3Int.WaveStep = (gi->AudCh3Int.WaveStep+1)%32;
                }
                
                if(g->AudCh3.VolShift != 0) {
                    Ch3 = g->AudCh3.Wave[gi->AudCh3Int.WaveStep];
                    Ch3 >>= WaveVolShift[g->AudCh3.VolShift];
                    Ch3 *= 4;
                }
                
                // this will never wrap
                gi->AudCh3Int.FreqTimer--;
                // this might wrap, so we need to stop decrementing
                if(g->AudCh3.UseSoundLen) gi->AudCh3Int.SoundLenTimer--; 
            }
            
            i8 Ch4 = 0;
            if(g->AudCh4.Enabled) {
                if(g->AudCh4.UseSoundLen && gi->AudCh4Int.SoundLenTimer == TimerDefault) {
                    gi->AudCh4Int.SoundLenTimer = (AudioSampleRate/SoundLenBaseRateHz)*(64-g->AudCh4.SoundLen);
                } else if(g->AudCh4.UseSoundLen && gi->AudCh4Int.SoundLenTimer == (TimerDefault+1)) {
                    g->AudCh4.Enabled = 0;
                }
                
                enum { NoiseShiftClockBaseHz = 524288 };
                if(gi->AudCh4Int.FreqTimer < 0) {
                    int Div = 1;
                    if(g->AudCh4.ShiftClockDivider != 0) Div = g->AudCh4.ShiftClockDivider;
                    
                    gi->AudCh4Int.FreqTimer = AudioSampleRate/((NoiseShiftClockBaseHz/Div)>>(g->AudCh4.ShiftClockFreq+1));
                    if(g->AudCh4.BitWidth) {
                        gi->AudCh4Int.NoiseStep = (gi->AudCh4Int.NoiseStep+1)%ArrayLength(Noise7);
                    } else {
                        gi->AudCh4Int.NoiseStep = (gi->AudCh4Int.NoiseStep+1)%ArrayLength(Noise15);
                    }
                }
                
                if(g->AudCh4.VolSweeps >= 0 && gi->AudCh4Int.VolSweepTimer < 0) {
                    gi->AudCh4Int.VolSweepTimer = (AudioSampleRate/VolSweepBaseLenHz)*g->AudCh4.VolSweeps;
                    g->AudCh4.VolSweeps--;
                    g->AudCh4.Vol += SweepDir[g->AudCh4.VolSweepUp];
                }
                
                if(g->AudCh4.BitWidth) { 
                    Ch4 = (i8)(Noise7[gi->AudCh4Int.NoiseStep]*g->AudCh4.Vol/15);
                } else {
                    Ch4 = (i8)(Noise15[gi->AudCh4Int.NoiseStep]*g->AudCh4.Vol/15);
                }
                
                // this will never wrap
                gi->AudCh4Int.FreqTimer--;
                // these might wrap, so stop decrementing
                if(g->AudCh4.UseSoundLen) gi->AudCh4Int.SoundLenTimer--; 
                if(gi->AudCh4Int.VolSweepTimer > -1) gi->AudCh4Int.VolSweepTimer--;
            }
            
            i32 ResL = 0;
            i32 ResR = 0;
            if(g->Audc.Ch1Out == AudMiddle) {
                ResL += Ch1;
                ResR += Ch1;
            } else if(g->Audc.Ch1Out == AudLeft) {
                ResL += Ch1;
            } else if(g->Audc.Ch1Out == AudRight) {
                ResR += Ch1;
            }
            if(g->Audc.Ch2Out == AudMiddle) {
                ResL += Ch2;
                ResR += Ch2;
            } else if(g->Audc.Ch2Out == AudLeft) {
                ResL += Ch2;
            } else if(g->Audc.Ch2Out == AudRight) {
                ResR += Ch2;
            }
            if(g->Audc.Ch3Out == AudMiddle) {
                ResL += Ch3;
                ResR += Ch3;
            } else if(g->Audc.Ch3Out == AudLeft) {
                ResL += Ch3;
            } else if(g->Audc.Ch3Out == AudRight) {
                ResR += Ch3;
            }
            if(g->Audc.Ch4Out == AudMiddle) {
                ResL += Ch4;
                ResR += Ch4;
            } else if(g->Audc.Ch4Out == AudLeft) {
                ResL += Ch4;
            } else if(g->Audc.Ch4Out == AudRight) {
                ResR += Ch4;
            }
            
            AudioBuffer[i] = (i8)(ResL*g->Audc.MasterVol/AudMaxMasterVol);
            AudioBuffer[i+1] = (i8)(ResR*g->Audc.MasterVol/AudMaxMasterVol);
        }
        
        if(SDL_QueueAudio(AudioDev, AudioBuffer, AddQueueSize)) {
            SDL_LogError(0, "Failed to queue audio on device: %s", SDL_GetError());
            exit(1);
        }
    }
    AfterAudio = SDL_GetPerformanceCounter();
    
    // draw BGTiles + Window + Sprites 
    BeforeDraw = SDL_GetPerformanceCounter();
    pfory32(ScreenPxH) {
        g->Lcdp.LY = y;
        if(g->Lcdp.LY == g->Lcdp.LYC && g->LYi) g->LYi();
        int Y = (g->Lcdp.SCY%BGMapPxH)+y;
        if (Y < 0) Y-=TileSpriteDim-1;
        int TileY = Y/TileSpriteDim;
        int TileLine = Y%TileSpriteDim;
        if(TileY < 0) {
            TileY += BGMapH;
            TileLine = (TileSpriteDim-1)+TileLine;
        }
        
        int WrappedY = TileY%BGMapH;
        
        int WinY = y-g->Lcdp.WY;
        int WinTileY = WinY/TileSpriteDim;
        int WinTileLine = WinY%TileSpriteDim;
        
        // collect sprites on line
        NumSpritesOnLine = 0;
        pfori(ArrayLength(g->SpriteTable)) {
            sprite_attr* Sprite = g->SpriteTable+i;
            if(Sprite->y <= 0 || Sprite->y >= ScreenPxH+TileSpriteDim) continue;
            if(y < Sprite->y && y >= Sprite->y-TileSpriteDim) {
                SpritesOnLine[NumSpritesOnLine++] = *Sprite;
            }
        }
        
        pforx32(ScreenPxW) {
            i8 PalIndex = 0;
            g->Lcdp.LX = x;
            if(g->Lcdp.LX == g->Lcdp.LXC && g->LXi) g->LXi();
            u8 SpritePx = 0;
            pfori(NumSpritesOnLine) {
                sprite_attr *s = SpritesOnLine+i;
                if(x < s->x && x >= s->x-TileSpriteDim) {
                    int Line = y-(s->y-TileSpriteDim);
                    if(s->YFlip) Line = (TileSpriteDim-1)-Line;
                    int Px = x-(s->x-TileSpriteDim);
                    if(s->XFlip) Px = (TileSpriteDim-1)-Px;
                    SpritePx = g->TileSpriteMemory[(s->TileNumber*TileSpriteSize)+Line*TileSpriteDim+Px];
                    if(SpritePx != 0) {
                        PalIndex = s->Palette;
                        break;
                    }
                }
            }
            
            u8 IndexPixel = 0;
            // TODO handle sprite_attr.Priority
            if(SpritePx && g->Lcdc.SpriteDisplay) {
                IndexPixel = SpritePx;
            } else {
                if(g->Lcdc.WindowDisplay && x >= g->Lcdp.WX && y >= g->Lcdp.WY) {
                    int WinX = x-g->Lcdp.WX;
                    int WinTileX = WinX/TileSpriteDim;
                    int WinTilePix = WinX%TileSpriteDim;
                    bg_attr Attr = g->WinAttrMap[WinTileY*BGMapW+WinTileX];
                    PalIndex = Attr.Palette;
                    int Line = WinTileLine;
                    if(Attr.YFlip) Line = (TileSpriteDim-1)-WinTileLine; 
                    int Pix = WinTilePix;
                    if(Attr.XFlip) Pix = (TileSpriteDim-1)-WinTilePix; 
                    IndexPixel = g->TileSpriteMemory[(TileSpriteSize*g->WinMap[WinTileY*BGMapW+WinTileX])+(Line*TileSpriteDim+Pix)];
                } else if(g->Lcdc.TileDisplay) {
                    int X = (g->Lcdp.SCX%BGMapPxW)+x;
                    if(X < 0) X-=TileSpriteDim-1;
                    int TileX = X/TileSpriteDim;
                    int TilePix = X%TileSpriteDim;
                    if(TileX < 0) {
                        TileX += BGMapW;
                        TilePix = (TileSpriteDim-1)+TilePix;
                    }
                    
                    int WrappedX = TileX%BGMapW;
                    bg_attr Attr = g->BGAttrMap[WrappedY*BGMapW+WrappedX];
                    PalIndex = Attr.Palette;
                    int Line = TileLine;
                    if(Attr.YFlip) Line = (TileSpriteDim-1)-TileLine; 
                    int Pix = TilePix;
                    if(Attr.XFlip) Pix = (TileSpriteDim-1)-TilePix; 
                    IndexPixel = g->TileSpriteMemory[(TileSpriteSize*g->BGMap[WrappedY*BGMapW+WrappedX])+(Line*TileSpriteDim+Pix)];
                } else {
                    IndexPixel = 0;
                }
            }
            u8 *ScreenPix = ScreenBuffer+((ScreenPxW*4)*y+(x*4));
            ScreenPix[0] = 255;
            ScreenPix[1] = g->Palettes[PalIndex].E[IndexPixel*3+ShadeB];
            ScreenPix[2] = g->Palettes[PalIndex].E[IndexPixel*3+ShadeG];
            ScreenPix[3] = g->Palettes[PalIndex].E[IndexPixel*3+ShadeR];
        }
    }
    AfterDraw = SDL_GetPerformanceCounter();
    BeforeLock = SDL_GetPerformanceCounter();
    SDL_SetRenderDrawColor(Renderer, 127,127,127, 255);
    SDL_RenderClear(Renderer);
    
    void *Pixels;
    int Pitch;
    if(SDL_LockTexture(ScreenTex, 0, &Pixels, &Pitch)) {
        SDL_LogError(0, "Failed to lock screen texture: %s", SDL_GetError());
        exit(1);
    }
    Assert(Pitch == (ScreenPxW*4));
    memcpy(Pixels, ScreenBuffer, ScreenPxW*ScreenPxH*4);
    
    SDL_UnlockTexture(ScreenTex);
    AfterLock = SDL_GetPerformanceCounter();
    
    update_developer_tools();
}

void gep_present(void) {
    SDL_SetRenderDrawColor(Renderer, g->Lcdc.r, g->Lcdc.g, g->Lcdc.b, 255);
    
    if(SDL_RenderClear(Renderer)) {
        SDL_LogError(0, "Failed to clear rendertarget. Oh well.: %s", SDL_GetError());
    }
    int ScaledW = ScreenPxW*ScreenScale;
    int ScaledH = ScreenPxH*ScreenScale;
    
    int MinX = (ActualW-ScaledW)/2;
    int MinY = (ActualH-ScaledH)/2;
    SDL_Rect DstRect = {.x=MinX, .y=MinY, .w=ScaledW, .h=ScaledH};
    if(SDL_RenderCopy(Renderer, ScreenTex, 0, &DstRect)) {
        SDL_LogError(0, "Failed to copy screen texture: %s", SDL_GetError());
        exit(1);
    }
    
    u64 BeforePresent = SDL_GetPerformanceCounter();
    SDL_RenderPresent(Renderer);
    
    u64 EndFrame = SDL_GetPerformanceCounter();
    f64 PerfFreq = (f64)SDL_GetPerformanceFrequency();
    u64 LockTime = AfterLock-BeforeLock;
    //SDL_LogInfo(0, "Game Update: %.4fms, Frame: %.2fms, Frame Present: %.2fms, Present Only: %.2fms, Audio: %.2fms, Draw: %.2fms, Lock: %.2fms", (f64)(AfterGameUpdate-BeforeGameUpdate)*1000.0/PerfFreq, (f64)(BeforePresent-BeginFrame)*1000.0/PerfFreq, (f64)(EndFrame-BeginFrame)*1000.0/PerfFreq,(f64)(EndFrame-BeforePresent)*1000.0/PerfFreq,(f64)(AfterAudio-BeforeAudio)*1000.0/PerfFreq,(f64)(AfterDraw-BeforeDraw)*1000.0/PerfFreq,(f64)(LockTime)*1000.0/PerfFreq);
}
