#include <stdint.h>

#define DLLEXPORT  __declspec( dllexport )

typedef float r32;
typedef double r64;

typedef float f32;
typedef double f64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int32_t b32;

typedef uintptr_t uptr;
typedef intptr_t ptr;

#define Assert(Expression) if(!(Expression)) {int *x = 0; *x = 0;}
#define StaticAssert(Expression) static_assert(Expression, "Static assert failed");

#define ArrayLength(Arr) (sizeof(Arr)/sizeof(Arr[0]))
#define ArraySize(Arr) (sizeof(Arr))

#define pfor(counter, max) for(i64 (counter)=0; (counter)<(max); (counter)++)
#define pfori(max) for(i64 i=0; i<(max); i++)
#define pforj(max) for(i64 j=0; j<(max); j++)
#define pfork(max) for(i64 k=0; k<(max); k++)
#define pforx(max) for(i64 x=0; x<(max); x++)
#define pfory(max) for(i64 y=0; y<(max); y++)
#define pforz(max) for(i64 z=0; z<(max); z++)

#define pfor(counter, max) for(i64 (counter)=0; (counter)<(max); (counter)++)
#define pfori32(max) for(i32 i=0; i<(max); i++)
#define pforj32(max) for(i32 j=0; j<(max); j++)
#define pfork32(max) for(i32 k=0; k<(max); k++)
#define pforx32(max) for(i32 x=0; x<(max); x++)
#define pfory32(max) for(i32 y=0; y<(max); y++)
#define pforz32(max) for(i32 z=0; z<(max); z++)

#define pforu(counter, max) for(u64 (counter)=0; (counter)<(max); (counter)++)
#define pforiu(max) for(u64 i=0; i<(max); i++)
#define pforju(max) for(u64 j=0; j<(max); j++)
#define pforku(max) for(u64 k=0; k<(max); k++)
#define pforxu(max) for(u64 x=0; x<(max); x++)
#define pforyu(max) for(u64 y=0; y<(max); y++)
#define pforzu(max) for(u64 z=0; z<(max); z++)

#define Kilobytes(size) ((ptr)size*1024)
#define Megabytes(size) Kilobytes((ptr)size*1024)
#define Gigabytes(size) Megabytes((ptr)size*1024)

#define MsToNs(Ms) ((Ms)*1000000)
#define SecToMs(S) ((S)*1000)

#define Million 1000000
#define Billion 1000000000

#define PI 3.1415926535897f

#define Min(a, b) ((a) < (b) ? (a) : (b))
#define Max(a, b) ((a) > (b) ? (a) : (b))

#define Clip(From, Value, To) if((Value) > (To)) {(Value) = (To); } else if((Value) < (From)) {(Value) = (From); }

typedef struct {
    f32 mx, my;
    b32 ml, mm, mr;
    f32 mwd;
    b32 ctrl;
} devinput;

static devinput DevInput;

typedef struct {
    b32 a, b, x, y;
    b32 Up, Down, Left, Right;
    b32 L1, L2, R1, R2;
    b32 Menu, Option;
} gepinput;

typedef struct {
    b32 TileDisplay; // Display tile layer
    b32 WindowDisplay; // Display window layer
    b32 SpriteDisplay; // display sprite layer;
    u8 r,g,b; // Border color around the screen
} lcd_ctrl;

static u8 DefaultBorderColor[3] = { 8,40,8 };

typedef struct {
    int SCX, SCY;
    int LX, LY;
    int LXC, LYC;
    int WX, WY;
} lcdp;

typedef struct {
    i8 Palette;
    i8 XFlip;
    i8 YFlip;
} bg_attr;

// IMPORTANT! Needs to be power of two dim
#define TileSpriteDim 8

#define TileSpriteSize (TileSpriteDim*TileSpriteDim)
#define NumMemTileSprites 512
#define TileSpriteW 32
#define TileSpriteH (NumMemTileSprites/TileSpriteW) // 16

#define TileSpriteMemSize (64*NumMemTileSprites)

#define ScreenW 32
#define ScreenH 18

#define ScreenPxW (ScreenW*TileSpriteDim)
#define ScreenPxH (ScreenH*TileSpriteDim)

// IMPORTANT! Need to be power of two width and height
#define BGMapW 64
#define BGMapH 32

#define BGMapPxW (BGMapW*8)
#define BGMapPxH (BGMapH*8)

#define ShadeR 0
#define ShadeG 1
#define ShadeB 2
#define Shade0 0
#define Shade1 3
#define Shade2 6
#define Shade3 9

void map_set_attr(bg_attr *Map, int x, int y, bg_attr Val) {
    if(x < 0) x+=BGMapW;
    if(y < 0) y+=BGMapH;
    x = x%BGMapW;
    y = y%BGMapH;
    Map[y*BGMapW+x] = Val;
}

void map_set16(u16 *Map, int x, int y, u16 Val) {
    if(x < 0) x+=BGMapW;
    if(y < 0) y+=BGMapH;
    x = x%BGMapW;
    y = y%BGMapH;
    Map[y*BGMapW+x] = Val;
}

void map_set8(u8 *Map, int x, int y, u8 Val) {
    if(x < 0) x+=BGMapW;
    if(y < 0) y+=BGMapH;
    x = x%BGMapW;
    y = y%BGMapH;
    Map[y*BGMapW+x] = Val;
}

bg_attr map_get_attr(bg_attr *Map, int x, int y) {
    if(x < 0) x+=BGMapW;
    if(y < 0) y+=BGMapH;
    x = x%BGMapW;
    y= y%BGMapH;
    return Map[(y*BGMapW)+x];
}

u16 map_get16(u16 *Map, int x, int y) {
    if(x < 0) x+=BGMapW;
    if(y < 0) y+=BGMapH;
    x = x%BGMapW;
    y= y%BGMapH;
    return Map[(y*BGMapW)+x];
}

u8 map_get8(u8 *Map, int x, int y) {
    if(x < 0) x+=BGMapW;
    if(y < 0) y+=BGMapH;
    x = x%BGMapW;
    y= y%BGMapH;
    return Map[(y*BGMapW)+x];
}

b32 map_set_rect_attr(bg_attr *Map, int MinX, int MinY, int MaxX, int MaxY, bg_attr Val) {
    if(MinX < 0) return 0;
    if(MinY < 0) return 0;
    if(MinX > MaxX) return 0;
    if(MinY > MaxY) return 0;
    
    int Width = MaxX-MinX;
    int Height = MaxY-MinY;
    pfory32(Height) {
        pforx32(Width) {
            map_set_attr(Map, MinX+x, MinY+y, Val);
        }
    }
    return 1;
}

b32 map_set_rect16(u16 *Map, int MinX, int MinY, int MaxX, int MaxY, u16 Val) {
    if(MinX < 0) return 0;
    if(MinY < 0) return 0;
    if(MinX > MaxX) return 0;
    if(MinY > MaxY) return 0;
    
    int Width = MaxX-MinX;
    int Height = MaxY-MinY;
    pfory32(Height) {
        pforx32(Width) {
            map_set16(Map, MinX+x, MinY+y, Val);
        }
    }
    return 1;
}

b32 map_set_rect8(u8 *Map, int MinX, int MinY, int MaxX, int MaxY, u8 Val) {
    if(MinX < 0) return 0;
    if(MinY < 0) return 0;
    if(MinX > MaxX) return 0;
    if(MinY > MaxY) return 0;
    
    int Width = MaxX-MinX;
    int Height = MaxY-MinY;
    pfory32(Height) {
        pforx32(Width) {
            map_set8(Map, MinX+x, MinY+y, Val);
        }
    }
    return 1;
}

typedef struct {
    int x, y;
    int TileNumber;
    int Priority;
    b32 XFlip;
    b32 YFlip;
    i8 Palette;
} sprite_attr;

#define MaxSprites 40

typedef void (*LX_interrupt_function)(void);
typedef void (*LY_interrupt_function)(void);

typedef struct {
    int Reset;
    int Enabled;
    int Hz;
    int VolSweeps; // 0-7
    int VolSweepUp;
    int Vol; // 0-15
    int DutyChoice; // 0-3; 2 = default tone
    
    int SoundLen; // 0-63 0=longest, 63=shortest
    b32 UseSoundLen;
    
    int FreqSweeps; // 0-7
    int FreqSweepUp;
    int FreqSweepTime; // n:0-7 (n/128Hz)
} aud_ch1;

typedef struct {
    int Reset;
    int Enabled;
    int Hz;
    int VolSweeps; // 0-7
    int VolSweepUp;
    int Vol; // 0-15
    int DutyChoice; // 0-3; 2 = default tone
    int SoundLen; // 0-63 0=longest, 63=shortest
    b32 UseSoundLen;
} aud_ch2;

typedef struct {
    int Reset;
    int Enabled;
    int Hz;
    int VolShift; // 0-3
    int SoundLen; // 0-255 0=longest, 255=shortest
    b32 UseSoundLen;
    i8 Wave[32]; // -7..7
} aud_ch3;

typedef struct {
    int Reset;
    int Enabled;
    int VolSweeps; // 0-7
    int VolSweepUp;
    int Vol; // 0-15
    int SoundLen; // 0-63 0=longest, 63=shortest
    b32 UseSoundLen;
    
    int BitWidth; // 0=15bit noise, 1=7bit noise
    int ShiftClockFreq; // 0-7, higher is faster
    int ShiftClockDivider; // 0-7, A divider on the shift clock frequency, higher is slower
} aud_ch4;

typedef enum {
    AudMiddle,
    AudLeft,
    AudRight,
} aud_pan;

#define AudMaxMasterVol 100
typedef struct {
    int MasterVol; // 0-AudMaxMasterVol; 0: mute, AudMaxMasterVol: full volume
    aud_pan Ch1Out;
    aud_pan Ch2Out;
    aud_pan Ch3Out;
    aud_pan Ch4Out;
} audc;

#define NumPalettes 16
typedef union {
    u8 E[12];
    struct {
        u8 r0,g0,b0;
        u8 r1,g1,b1;
        u8 r2,g2,b2;
        u8 r3,g3,b3;
    };
} palette;

static palette PalDefault = {
    .E = {
        155, 188, 15, // Light
        139, 172, 15,
        48, 98, 48, 
        15, 56, 15, // Dark
    },
};

typedef struct {
    gepinput Input;
    gepinput InDown;
    gepinput InUp;
    u8 TileSpriteMemory[TileSpriteMemSize];
    u16 BGMap[BGMapW*BGMapH];
    bg_attr BGAttrMap[BGMapW*BGMapH];
    u16 WinMap[BGMapW*BGMapH];
    bg_attr WinAttrMap[BGMapW*BGMapH];
    sprite_attr SpriteTable[MaxSprites];
    palette Palettes[NumPalettes];
    aud_ch1 AudCh1;
    aud_ch2 AudCh2;
    aud_ch3 AudCh3;
    aud_ch4 AudCh4;
    audc Audc;
    lcdp Lcdp;
    lcd_ctrl Lcdc;
    LX_interrupt_function LXi;
    LX_interrupt_function LYi;
} gep_state;

static gep_state *g;

static u16 *GlobalCharLookupTable;
static char TmpStrBuffer[ScreenW*(ScreenH+1)];
void set_tile_string(int x, int y, const char *Str) {
    int XOrig = x;
    while(*Str) {
        if(*Str == '\n') {
            y++;
            x = XOrig;
        }
        map_set16(g->BGMap, x, y, GlobalCharLookupTable[*Str]);
        x++;
        Str++;
    }
}

// Text box border 8,9,10,11
typedef enum {
    TextBoxBorderTopLeft,
    TextBoxBorderTop,
    TextBoxBorderTopRight,
    TextBoxBorderMiddleLeft,
    TextBoxBorderMiddleRight,
    TextBoxBorderBottomLeft,
    TextBoxBorderBottom,
    TextBoxBorderBottomRight,
} text_box_border;

static u16 *GlobalTextBoxBorderLookupTable;
b32 set_tile_text_box(int XMin, int YMin, int XMax, int YMax) {
    XMax--;
    YMax--;
    if(XMin < 0) return 0;
    if(YMin < 0) return 0;
    if(XMin > XMax) return 0;
    if(YMin > YMax) return 0;
    
    int Width = XMax-XMin;
    
    int Height = YMax-YMin;
    
    // Corners
    map_set16(g->BGMap, XMin, YMin, GlobalTextBoxBorderLookupTable[TextBoxBorderTopLeft]);
    map_set16(g->BGMap, XMax, YMin, GlobalTextBoxBorderLookupTable[TextBoxBorderTopRight]);
    map_set16(g->BGMap, XMin, YMax, GlobalTextBoxBorderLookupTable[TextBoxBorderBottomLeft]);
    map_set16(g->BGMap, XMax, YMax, GlobalTextBoxBorderLookupTable[TextBoxBorderBottomRight]);
    
    // Top and Bottom
    for(int i=1; i<Width; i++) {
        map_set16(g->BGMap, XMin+i, YMin, GlobalTextBoxBorderLookupTable[TextBoxBorderTop]);
        map_set16(g->BGMap, XMin+i, YMax, GlobalTextBoxBorderLookupTable[TextBoxBorderBottom]);
    }
    
    // Sides
    for(int i=1; i<Height; i++) {
        map_set16(g->BGMap, XMin, YMin+i, GlobalTextBoxBorderLookupTable[TextBoxBorderMiddleLeft]);
        map_set16(g->BGMap, XMax, YMin+i, GlobalTextBoxBorderLookupTable[TextBoxBorderMiddleRight]);
    }
    
    // FillCenter
    map_set_rect16(g->BGMap, XMin+1, YMin+1, XMax, YMax, 0);
    
    return 1;
}
