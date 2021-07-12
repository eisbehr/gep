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

#define Assert(Expression) if(!(Expression)) {int *_x = 0; *_x = 0;}
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

int abs32(int Val) {
    return Val<0 ? -Val : Val;
}

#define Clip(From, Value, To) if((Value) > (To)) {(Value) = (To); } else if((Value) < (From)) {(Value) = (From); }

typedef struct {
    f32 mx, my;
    b32 ml, mm, mr;
    f32 mwd;
    b32 ctrl;
} devinput;

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
    int SCX, SCY; // Scene X and Y Offset
    int LX, LY; // X and Y pixel coordinate of screen currently being drawn
    int LXC, LYC; // X column and Y row to call respective interrupts on
    int WX, WY; // Window X and Y
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

#define TileSpritePxW (TileSpriteW*TileSpriteDim)
#define TileSpritePxH (TileSpriteH*TileSpriteDim)

#define TileSpriteMemSize (64*NumMemTileSprites)

#define ScreenW 32
#define ScreenH 18

#define ScreenPxW (ScreenW*TileSpriteDim)
#define ScreenPxH (ScreenH*TileSpriteDim)

// IMPORTANT! Need to be power of two width and height
#define BGMapW 64
#define BGMapH 32
#define WinMapW BGMapW
#define WinMapH BGMapH

#define BGMapPxW (BGMapW*TileSpriteDim)
#define BGMapPxH (BGMapH*TileSpriteDim)
#define WinMapPxW (WinMapW*TileSpriteDim)
#define WinMapPxH (WinMapH*TileSpriteDim)

#define ShadeR 0
#define ShadeG 1
#define ShadeB 2
#define Shade0 0
#define Shade1 3
#define Shade2 6
#define Shade3 9

void map_set_attr(bg_attr *Map, int x, int y, bg_attr Val) {
    x = x%BGMapW;
    y = y%BGMapH;
    if(x < 0) x+=BGMapW;
    if(y < 0) y+=BGMapH;
    Map[y*BGMapW+x] = Val;
}

void map_set16(u16 *Map, int x, int y, u16 Val) {
    x = x%BGMapW;
    y = y%BGMapH;
    if(x < 0) x+=BGMapW;
    if(y < 0) y+=BGMapH;
    Map[y*BGMapW+x] = Val;
}

void map_set16c(u16 *Map, int MapW, int MapH, int x, int y, u16 Val) {
    x = x%MapW;
    y = y%MapH;
    if(x < 0) x+=MapW;
    if(y < 0) y+=MapH;
    Map[y*MapW+x] = Val;
}

void map_set8(u8 *Map, int x, int y, u8 Val) {
    x = x%BGMapW;
    y = y%BGMapH;
    if(x < 0) x+=BGMapW;
    if(y < 0) y+=BGMapH;
    Map[y*BGMapW+x] = Val;
}

void map_set8c(u8 *Map, int MapW, int MapH, int x, int y, u8 Val) {
    x = x%MapW;
    y = y%MapH;
    if(x < 0) x+=MapW;
    if(y < 0) y+=MapH;
    Map[y*MapW+x] = Val;
}

bg_attr map_get_attr(bg_attr *Map, int x, int y) {
    x = x%BGMapW;
    y = y%BGMapH;
    if(x < 0) x+=BGMapW;
    if(y < 0) y+=BGMapH;
    return Map[(y*BGMapW)+x];
}

u16 map_get16(const u16 *Map, int x, int y) {
    x = x%BGMapW;
    y = y%BGMapH;
    if(x < 0) x+=BGMapW;
    if(y < 0) y+=BGMapH;
    return Map[(y*BGMapW)+x];
}

u16 map_get16c(const u16 *Map, int MapW, int MapH, int x, int y) {
    x = x%MapW;
    y = y%MapH;
    if(x < 0) x+=MapW;
    if(y < 0) y+=MapH;
    return Map[(y*MapW)+x];
}

u8 map_get8(const u8 *Map, int x, int y) {
    x = x%BGMapW;
    y = y%BGMapH;
    if(x < 0) x+=BGMapW;
    if(y < 0) y+=BGMapH;
    return Map[(y*BGMapW)+x];
}

u8 map_get8c(const u8 *Map, int MapW, int MapH, int x, int y) {
    x = x%MapW;
    y = y%MapH;
    if(x < 0) x+=MapW;
    if(y < 0) y+=MapH;
    return Map[(y*MapW)+x];
}

b32 map_set_rect_attr(bg_attr *Map, int MinX, int MinY, int MaxX, int MaxY, bg_attr Val) {
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

b32 map_copy_rect16(u16 *Dst, int DX, int DY, const u16 *Src, int SWidth, int SHeight, int SMinX, int SMinY, int SMaxX, int SMaxY) {
    if(SMinX > SMaxX || SMinY > SMaxY) return 0;
    
    int Width = SMaxX-SMinX;
    int Height = SMaxY-SMinY;
    pfory32(Height) {
        pforx32(Width) {
            u16 Val = map_get16c(Src, SWidth, SHeight, SMinX+x, SMinY+y);
            map_set16(Dst, DX+x, DY+y, Val);
        }
    }
    return 1;
}

b32 map_copy_rect16c(u16 *Dst, int DstW, int DstH, int DX, int DY, const u16 *Src, int SWidth, int SHeight, int SMinX, int SMinY, int SMaxX, int SMaxY) {
    if(SMinX > SMaxX || SMinY > SMaxY) return 0;
    
    int Width = SMaxX-SMinX;
    int Height = SMaxY-SMinY;
    pfory32(Height) {
        pforx32(Width) {
            u16 Val = map_get16c(Src, SWidth, SHeight, SMinX+x, SMinY+y);
            map_set16c(Dst, DstW, DstH, DX+x, DY+y, Val);
        }
    }
    return 1;
}

b32 map_set_rect8(u8 *Map, int MinX, int MinY, int MaxX, int MaxY, u8 Val) {
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

b32 map_copy_rect8(u8 *Dst, int DX, int DY, const u8 *Src, int SWidth, int SHeight, int SMinX, int SMinY, int SMaxX, int SMaxY) {
    if(SMinX > SMaxX || SMinY > SMaxY) return 0;
    
    int Width = SMaxX-SMinX;
    int Height = SMaxY-SMinY;
    pfory32(Height) {
        pforx32(Width) {
            u8 Val = map_get8c(Src, SWidth, SHeight, SMinX+x, SMinY+y);
            map_set8(Dst, DX+x, DY+y, Val);
        }
    }
    return 1;
}

b32 map_copy_rect8c(u8 *Dst, int DstW, int DstH, int DX, int DY, const u8 *Src, int SWidth, int SHeight, int SMinX, int SMinY, int SMaxX, int SMaxY) {
    if(SMinX > SMaxX || SMinY > SMaxY) return 0;
    
    int Width = SMaxX-SMinX;
    int Height = SMaxY-SMinY;
    pfory32(Height) {
        pforx32(Width) {
            u8 Val = map_get8c(Src, SWidth, SHeight, SMinX+x, SMinY+y);
            map_set8c(Dst, DstW, DstH, DX+x, DY+y, Val);
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

#define MaxSprites 256

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

#define HexColorR(Hex) ((u8)(((u32)(Hex))>>16))
#define HexColorG(Hex) ((u8)(((u32)(Hex))>>8))
#define HexColorB(Hex) ((u8)(((u32)(Hex))>>0))

#define PaletteHexColorRGB(Palette, ColorIndex, Hex) { \
(Palette).E[(ColorIndex)*3+0]=HexColorR(Hex); \
(Palette).E[(ColorIndex)*3+1]=HexColorG(Hex); \
(Palette).E[(ColorIndex)*3+2]=HexColorB(Hex);}

#define PaletteHexRGB(Palette, Hex0, Hex1, Hex2, Hex3) \
PaletteHexColorRGB(Palette, 0, Hex0); \
PaletteHexColorRGB(Palette, 1, Hex1); \
PaletteHexColorRGB(Palette, 2, Hex2); \
PaletteHexColorRGB(Palette, 3, Hex3);

static palette PalDefault = {
    .E = {
        0xe0, 0xf8, 0xd0, // Light
        0x88, 0xc0, 0x70,
        0x34, 0x68, 0x56, 
        0x08, 0x18, 0x20, // Dark
    },
};

typedef struct {
    gepinput Input;
    gepinput InDown;
    gepinput InUp;
    devinput DevInput;
    devinput DevDown;
    devinput DevUp;
    u8 TileSpriteMemory[TileSpriteMemSize];
    u16 BGMap[BGMapW*BGMapH];
    bg_attr BGAttrMap[BGMapW*BGMapH];
    u16 WinMap[WinMapW*WinMapH];
    bg_attr WinAttrMap[BGMapW*BGMapH];
    sprite_attr SpriteTable[MaxSprites];
    int NumSprites;
    palette Palettes[NumPalettes];
    aud_ch1 AudCh1;
    aud_ch2 AudCh2;
    aud_ch3 AudCh3;
    aud_ch4 AudCh4;
    audc Audc;
    lcdp Lcdp;
    lcd_ctrl Lcdc;
    int ScreenScale;
    LX_interrupt_function LXi;
    LY_interrupt_function LYi;
} gep_state;

static gep_state *g;

void sprite_reset(void) {
    memset(g->SpriteTable, 0, ArraySize(g->SpriteTable));
    g->NumSprites = 0;
}

sprite_attr *sprite_push(sprite_attr *s) {
    sprite_attr *Res = 0;
    if(g->NumSprites < MaxSprites) {
        Res = &g->SpriteTable[g->NumSprites++];
        *Res = *s;
    }
    return Res;
}

static u16 _DefaultCharLookup[128] = {
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

static u16 *GlobalCharLookupTable = _DefaultCharLookup;
static char TmpStrBuffer[ScreenW*(ScreenH+1)];
void set_tile_string(u16 *Map, int x, int y, const char *Str) {
    int XOrig = x;
    while(*Str) {
        if(*Str == '\n') {
            y++;
            x = XOrig;
        }
        map_set16(Map, x, y, GlobalCharLookupTable[*Str]);
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

static u16 _DefaultTextBoxBorderLookup[] = {
    [TextBoxBorderTopLeft] = 8,
    [TextBoxBorderTop] = 9,
    [TextBoxBorderTopRight] = 10,
    [TextBoxBorderMiddleLeft] = 40,
    [TextBoxBorderMiddleRight] = 42,
    [TextBoxBorderBottomLeft] = 72,
    [TextBoxBorderBottom] = 73,
    [TextBoxBorderBottomRight] = 74,
};

static u16 *GlobalTextBoxBorderLookupTable = _DefaultTextBoxBorderLookup;
b32 set_tile_text_box(u16 *Map, int XMin, int YMin, int XMax, int YMax) {
    XMax--;
    YMax--;
    if(XMin < 0) return 0;
    if(YMin < 0) return 0;
    if(XMin > XMax) return 0;
    if(YMin > YMax) return 0;
    
    int Width = XMax-XMin;
    int Height = YMax-YMin;
    
    // Corners
    map_set16(Map, XMin, YMin, GlobalTextBoxBorderLookupTable[TextBoxBorderTopLeft]);
    map_set16(Map, XMax, YMin, GlobalTextBoxBorderLookupTable[TextBoxBorderTopRight]);
    map_set16(Map, XMin, YMax, GlobalTextBoxBorderLookupTable[TextBoxBorderBottomLeft]);
    map_set16(Map, XMax, YMax, GlobalTextBoxBorderLookupTable[TextBoxBorderBottomRight]);
    
    // Top and Bottom
    for(int i=1; i<Width; i++) {
        map_set16(Map, XMin+i, YMin, GlobalTextBoxBorderLookupTable[TextBoxBorderTop]);
        map_set16(Map, XMin+i, YMax, GlobalTextBoxBorderLookupTable[TextBoxBorderBottom]);
    }
    
    // Sides
    for(int i=1; i<Height; i++) {
        map_set16(Map, XMin, YMin+i, GlobalTextBoxBorderLookupTable[TextBoxBorderMiddleLeft]);
        map_set16(Map, XMax, YMin+i, GlobalTextBoxBorderLookupTable[TextBoxBorderMiddleRight]);
    }
    
    // FillCenter
    map_set_rect16(Map, XMin+1, YMin+1, XMax, YMax, 0);
    
    return 1;
}
