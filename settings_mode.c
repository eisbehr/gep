static gep_state_int GepStateInt;
static gep_state_int GepStateIntSettings;
static gep_state_int *gi;
static update_function game_update_function;
static b32 InSettings;

static u16 *GameCharLookupTable;
static u16 *GameTextBoxBorderLookupTable;

u16 SettingsCharLookup[128] = {
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

static u16 SettingsTextBoxBorderLookup[] = {
    [TextBoxBorderTopLeft] = 8,
    [TextBoxBorderTop] = 9,
    [TextBoxBorderTopRight] = 10,
    [TextBoxBorderMiddleLeft] = 40,
    [TextBoxBorderMiddleRight] = 42,
    [TextBoxBorderBottomLeft] = 72,
    [TextBoxBorderBottom] = 73,
    [TextBoxBorderBottomRight] = 74,
};

void settings_update(gep_state *);

typedef struct {
    b32 Initialized;
    b32 NormalInitialized;
    b32 CountdownInitialized;
    b32 VolInitialized;
    
    // Normal mode
    int CurPos;
    
    // Fullscreen countdown mode
    int CountDownFrames;
    b32 Choice;
    int ChoiceXOff;
    int ChoiceX;
    int ChoiceY;
    
    // Set Volume mode
    int ValBaseX;
    int ValBaseY;
    int InitialVolume;
    int HoldDownTimer;
#define HoldDownTimerVal 30
    
    // Settings
    b32 Fullscreen;
} settings_data;
static settings_data SetData;

void gep_switch_to_settings(void) {
    if(!InSettings) {
        InSettings = 1;
        g = &GepStateSettings;
        gi = &GepStateIntSettings;
        GepStateSettings.Audc.MasterVol = GepState.Audc.MasterVol;
        SetData.Fullscreen = IsFullscreen;
        game_update_function = update;
        update = settings_update;
        
        GameCharLookupTable = GlobalCharLookupTable;
        GlobalCharLookupTable = SettingsCharLookup;
        
        GameTextBoxBorderLookupTable = GlobalTextBoxBorderLookupTable;
        GlobalTextBoxBorderLookupTable = SettingsTextBoxBorderLookup;
    }
}

void gep_return_from_settings(void) {
    if(InSettings) {
        InSettings = 0;
        g = &GepState;
        gi = &GepStateInt;
        GepState.Audc.MasterVol = GepStateSettings.Audc.MasterVol;
        update = game_update_function;
        GlobalTextBoxBorderLookupTable = GameTextBoxBorderLookupTable;
        GlobalCharLookupTable = GameCharLookupTable;
        settings_write(&(prefs){.Fullscreen=SetData.Fullscreen, .Volume=GepStateSettings.Audc.MasterVol});
        // TODO say something if writing settings failed?
    }
}

void toggle_settings(void) {
    if(InSettings) {
        gep_return_from_settings();
    } else {
        gep_switch_to_settings();
    }
}

#define FirstSettingOffset 3
enum {SetFullscreen, SetVolume, NumSettings};

void update_settings_arrow(int Pos) {
    int y = (Pos+4)*TileSpriteDim;
    g->SpriteTable[0] = (sprite_attr) {
        .x=TileSpriteDim, .y=y,
        .TileNumber=1,
    };
}

void play_input_sound(void) {
    g->AudCh1 = (aud_ch1){
        .Reset = 1,
        .Enabled = 1,
        .Hz = 400,
        .SoundLen = 0,
        .UseSoundLen = 1,
        .DutyChoice = 2,
        .Vol = 7,
        .VolSweeps = 7,
        .FreqSweeps = 2,
        .FreqSweepUp = 1,
        .FreqSweepTime = 3,
    };
}

void play_change_settings_sound(void) {
    // Save/Confirm Sound
    g->AudCh1 = (aud_ch1) {
        .Reset = 1,
        .Enabled = 1,
        .Hz = 262,
        .VolSweeps = 7,
        .VolSweepUp = 0,
        .Vol = 7,
        .DutyChoice = 2,
        .FreqSweeps = 5,
        .FreqSweepUp = 1,
        .FreqSweepTime = 7,
    };
}

void set_fullscreen_tiles(void) {
    set_tile_string(15,FirstSettingOffset+SetFullscreen, SetData.Fullscreen ? "YES" : "NO ");
}

void set_volume_tiles(void) {
    char Buffer[4];
    snprintf(Buffer, ArraySize(Buffer), "%.3i", g->Audc.MasterVol);
    set_tile_string(SetData.ValBaseX,SetData.ValBaseY, Buffer);
}

void set_settings_tiles(void) {
    set_tile_text_box(0,0, ScreenW, ScreenH);
    set_tile_string(1,1, "SETTINGS");
    set_tile_string(1,FirstSettingOffset+SetFullscreen, "FULLSCREEN");
    set_fullscreen_tiles();
    SetData.ValBaseX = 15;
    SetData.ValBaseY = FirstSettingOffset+SetVolume;
    set_tile_string(1,SetData.ValBaseY, "VOLUME");
    set_volume_tiles();
}

typedef enum {
    set_mode_normal,
    set_mode_fullscreen_countdown,
    set_mode_volume,
} set_mode;
static set_mode SetMode;

#include "atlas_include/settings_atlas.h"
void update_set_mode_normal(void) {
    if(!SetData.NormalInitialized) {
        SetData.NormalInitialized = 1;
        //g->Audc.MasterVol = 100;
        
        set_settings_tiles();
        SetData.CurPos = 0;
    }
    
    if(g->InDown.Up) {
        SetData.CurPos--;
        if(SetData.CurPos >= 0) play_input_sound();
    }
    if(g->InDown.Down) {
        SetData.CurPos++;
        if(SetData.CurPos < NumSettings) play_input_sound();
    }
    
    Clip(0, SetData.CurPos, NumSettings-1);
    
    if(g->InDown.a) {
        switch(SetData.CurPos) {
            case SetFullscreen: {
                SetData.Fullscreen = !SetData.Fullscreen;
                set_fullscreen_tiles();
                play_change_settings_sound();
                
                // TODO VSYNC COPY copy pasted, clean up
                //if (0) {
                if(VsyncHz == 60.0) {
                    SDL_LogInfo(0, "Trying to toggle windowed fullscreen");
                    set_fullscreen(Window, 1, SetData.Fullscreen);
                } else {
                    SDL_LogInfo(0, "Trying to toggle exclusive fullscreen");
                    set_fullscreen(Window, 0, SetData.Fullscreen);
                    if(SetData.Fullscreen) {
                        SetMode = set_mode_fullscreen_countdown;
                    }
                }
                
            } break;
            case SetVolume: {
                SetMode = set_mode_volume;
                play_change_settings_sound();
            } break;
        }
    }
    
    //SDL_LogDebug(0, "CurPos: %i", SetData.CurPos);
    update_settings_arrow(SetData.CurPos);
}

void switch_volume_to_normal_mode(b32 SettingConfirmed) {
    set_settings_tiles();
    SetMode = set_mode_normal;
    if(SettingConfirmed) play_change_settings_sound();
    else g->Audc.MasterVol = SetData.InitialVolume;
    SetData.VolInitialized = 0;
    g->SpriteTable[1] = (sprite_attr) {0};
}

void update_set_mode_volume(void) {
    if(!SetData.VolInitialized) {
        SetData.VolInitialized = 1;
        SetData.InitialVolume = g->Audc.MasterVol;
        int x1 = (SetData.ValBaseX)*TileSpriteDim;
        int x2 = (SetData.ValBaseX+4)*TileSpriteDim;
        int y = (SetData.ValBaseY+1)*TileSpriteDim;
        g->SpriteTable[0] = (sprite_attr) {
            .x=x1, .y=y,
            .TileNumber=1,
        };
        g->SpriteTable[1] = (sprite_attr) {
            .x=x2, .y=y,
            .TileNumber=1,
            .XFlip=1,
        };
    }
    b32 DecVol = 0;
    b32 IncVol = 0;
    
    if(g->InDown.b) {
        switch_volume_to_normal_mode(0);
    }
    else if(g->InDown.a) {
        switch_volume_to_normal_mode(1);
    }
    
    if(g->InDown.Left) {
        DecVol = 1;
    } else if(g->InDown.Right) {
        IncVol = 1;
    }
    
    if(DecVol) {
        g->Audc.MasterVol--;
        if(g->Audc.MasterVol >= 0) play_input_sound();
    } else if(IncVol) {
        g->Audc.MasterVol++;
        if(g->Audc.MasterVol <= AudMaxMasterVol) play_input_sound();
    }
    
    Clip(0, g->Audc.MasterVol, AudMaxMasterVol);
    
    set_volume_tiles();
}

void switch_countdown_to_normal_mode(void) {
    set_settings_tiles();
    SetMode = set_mode_normal;
    play_change_settings_sound();
    SetData.CountdownInitialized = 0;
}

void update_set_mode_fullscreen_countdown(void) {
    const char Str[] = "KEEP FULLSCREEN? 9";
    int StrLen = ArraySize(Str)-1;;
    int XString = ((ScreenW-StrLen)/2);
    int YString = ScreenH/2-1;
    if(!SetData.CountdownInitialized) {
        SetData.CountdownInitialized = 1;
        
        SetData.CountDownFrames = 9*60;
        set_tile_text_box(XString-1, YString-1, XString+StrLen+1, YString+3);
        set_tile_string(XString, YString, Str);
        set_tile_string(XString, YString+1, "NO");
        set_tile_string(XString+StrLen-3, YString+1, "YES");
        
        SetData.ChoiceY = YString+2;
        SetData.ChoiceX = XString;
        SetData.ChoiceXOff = StrLen-3;
    }
    char Buffer[64];
    snprintf(Buffer, ArraySize(Buffer), "KEEP FULLSCREEN? %i", SetData.CountDownFrames/60);
    set_tile_string(XString, YString, Buffer);
    
    b32 Confirm = 0;
    if(g->InDown.a) {
        Confirm = 1;
    }
    
    if(g->InDown.Left) {
        SetData.Choice--;
        if(SetData.Choice >= 0) play_input_sound();
    }
    if(g->InDown.Right) {
        SetData.Choice++;
        if(SetData.Choice <= 1) play_input_sound();
    }
    
    Clip(0, SetData.Choice, 1);
    
    if(((SetData.Choice == 0) && Confirm) || SetData.CountDownFrames <= 0) {
        SetData.Fullscreen = 0;
        SDL_LogInfo(0, "Exiting fullscreen after time ran out, or cancel");
        set_fullscreen(Window, 0, SetData.Fullscreen);
        switch_countdown_to_normal_mode();
    }
    if((SetData.Choice == 1) && Confirm) {
        SDL_LogInfo(0, "Chose to keep fullscreen");
        switch_countdown_to_normal_mode();
    }
    SetData.CountDownFrames--;
    
    int y = (SetData.ChoiceY)*TileSpriteDim;
    int x = TileSpriteDim*(SetData.ChoiceX+(SetData.ChoiceXOff*SetData.Choice));
    g->SpriteTable[0] = (sprite_attr) {
        .x=x, .y=y,
        .TileNumber=1,
    };
}

void settings_update(gep_state *GS) {
    if(!SetData.Initialized) {
        SetData.Initialized = 1;
        
        g->Lcdc.TileDisplay = 1;
        g->Lcdc.SpriteDisplay = 1;
        
        map_set_rect16(g->BGMap, 0, 0, BGMapW, BGMapH, 3);
        
        memcpy(g->TileSpriteMemory, SettingsAtlas, TileSpriteMemSize);
        StaticAssert(ArraySize(SettingsAtlas) == TileSpriteMemSize);
        
        g->Palettes[0] = PalDefault;
        g->Lcdc.r = DefaultBorderColor[0];
        g->Lcdc.g = DefaultBorderColor[1];
        g->Lcdc.b = DefaultBorderColor[2];
    }
    
    switch(SetMode) {
        case set_mode_normal: {
            update_set_mode_normal();
        } break;
        case set_mode_fullscreen_countdown: {
            update_set_mode_fullscreen_countdown();
        } break;
        case set_mode_volume: {
            update_set_mode_volume();
        } break;
    }
}
