#include "flappy.h"
#include "atlas_include/atlas.h"
#include <stdlib.h>
#include <stdio.h>

void mode_switch(modes m) {
    Game.Mode = m;
    Game.LeaveMode = 1;
    Game.ModeInitialized = 0;
}

void clear_bg(void) {
    map_set_rect16(g->BGMap, 0, 0, BGMapW, BGMapH, 0);
    map_set_rect8(ColMap, 0, 0, BGMapW, BGMapH, 0);
    map_set_rect_attr(g->BGAttrMap, 0, 0, BGMapW, BGMapH, (bg_attr){0});
};

void bg_clear_rect(int MinX, int MinY, int MaxX, int MaxY) {
    map_set_rect16(g->BGMap, MinX, MinY, MaxX, MaxY, 0);
    map_set_rect8(ColMap, MinX, MinY, MaxX, MaxY, 0);
    map_set_rect_attr(g->BGAttrMap, MinX, MinY, MaxX, MaxY, (bg_attr){0});
}

void bg_set(int x, int y, int SpriteTileIndex, b32 Collision, bg_attr Attr) {
    map_set16(g->BGMap, x, y, (u16)SpriteTileIndex);
    map_set8(ColMap, x, y, (u8)Collision);
    map_set_attr(g->BGAttrMap, x, y, Attr);
}

void place_pipe_tiles(int x, int Top, int Gap) {
    x = x%BGMapW;
    int LenTop = Top;
    int y = 0;
    pforj32(LenTop-1) {
        pfori32(4) {
            bg_set(x+i, y+j, PipeTileIndicesShaft[i], 1, (bg_attr){0});
        }
    }
    y+=LenTop-1;
    pfori32(4) {
        bg_set(x+i, y, PipeTileIndicesTopRim[i], 1, (bg_attr){0});
    }
    
    int LenBottom = ScreenH-(Top+Gap);
    y = ScreenH-LenBottom;
    pfori32(4) {
        bg_set(x+i, y, PipeTileIndicesTopRim[i], 1, (bg_attr){0});
    }
    y++;
    pforj32(LenBottom-1) {
        pfori32(4) {
            bg_set(x+i, y+j, PipeTileIndicesShaft[i], 1, (bg_attr){0});
        }
    }
}

int random_between(int From, int To) {
    Assert(From < To);
    int Range = To-From;
    return rand()%Range+From;
}

void player_start_flap_sound(void) {
    g->AudCh4 = (aud_ch4) {
        .Reset = 1,
        .Enabled = 1,
        .VolSweeps = 7,
        .Vol = 7,
        .SoundLen = 0,
        .UseSoundLen = 1,
        .BitWidth = 0,
        .ShiftClockFreq = 3,
        .ShiftClockDivider = 4,
    };
}

void update_player_sprite_data(player *Player) {
    int x = Player->x;
    int y = -1;
    if(Player->Visible) {
        y = Player->y;
    }
    
    int FlipY = Player->FlipY;
    int TileTopLeft;
    int TileTopRight;
    int TileBottomLeft;
    int TileBottomRight;
    if(FlipY) {
        TileTopLeft = 66;
        TileTopRight = 67;
        TileBottomLeft = 34;
        TileBottomRight = 35;
    } else {
        TileTopLeft = 34;
        TileTopRight = 35;
        TileBottomLeft = 66;
        TileBottomRight = 67;
    }
    if(!Player->FlapAnim) {
        TileTopLeft -= 2;
        TileTopRight -= 2;
        TileBottomLeft -= 2;
        TileBottomRight -= 2;
    }
    g->SpriteTable[Player->SpriteIndexTopLeft] = (sprite_attr)
    {.x = x-TileSpriteDim, .y=y-TileSpriteDim, .TileNumber=TileTopLeft, .XFlip=0, .YFlip=FlipY};
    g->SpriteTable[Player->SpriteIndexTopRight] = (sprite_attr)
    {.x = x, .y=y-TileSpriteDim, .TileNumber=TileTopRight, .XFlip=0, .YFlip=FlipY};
    g->SpriteTable[Player->SpriteIndexBottomLeft] = (sprite_attr)
    {.x = x-TileSpriteDim, .y=y, .TileNumber=TileBottomLeft, .XFlip=0, .YFlip=FlipY};
    g->SpriteTable[Player->SpriteIndexBottomRight] = (sprite_attr)
    {.x = x, .y=y, .TileNumber=TileBottomRight, .XFlip=0, .YFlip=FlipY};
}

void run_mode_pre(void) {
    if(!Game.ModeInitialized) {
        Game.ModeInitialized = 1;
        
        memcpy(g->TileSpriteMemory, Atlas, ArraySize(Atlas));
        memcpy(g->AudCh3.Wave, TmpWave, ArrayLength(TmpWave));
        Game.BGNoteIndex = 0;
        
        g->Lcdc.TileDisplay = 1;
        g->Lcdc.WindowDisplay = 0;
        g->Lcdc.SpriteDisplay = 1;
        
        P = (player){0};
        P.Visible = 1;
        P.SpriteIndexTopLeft = 0;
        P.SpriteIndexTopRight = 1;
        P.SpriteIndexBottomLeft = 2;
        P.SpriteIndexBottomRight = 3;
        
        P.x = 50;
        P.y = 50;
        
        g->Lcdp.SCX = 0;
        g->Lcdp.SCY = 0;
        
        clear_bg();
    }
    if(g->Input.Up) mode_switch(mode_game);
    // update sprites
    update_player_sprite_data(&P);
    
    if(Game.LeaveMode) {
        Game.LeaveMode = 0;
    }
}
void run_mode_game(void) {
    if(!Game.ModeInitialized) {
        // Init Game
        Game.ModeInitialized = 1;
        
        // set random seed
        time_t T;
        srand((u32) time(&T));
        
        Game.NextPipeX = ScreenW;
        Game.NextPipeChunkX = ScreenW;
    }
    
    enum { PlayerAnimationFrames = 15 };
    if(g->Input.Up) {
        P.y-=2;
        if(P.Flap == 0) {
            P.AnimationFrames = PlayerAnimationFrames;
            P.FlapAnim = 1;
            player_start_flap_sound();
        } else {
            P.AnimationFrames--;
            if(P.AnimationFrames == 0) {
                //P.FlapAnim = !P.FlapAnim;
                if(P.FlapAnim) P.FlapAnim = 0;
                else {
                    P.FlapAnim = 1;
                    player_start_flap_sound();
                }
                P.AnimationFrames = PlayerAnimationFrames;
            }
        }
        P.Flap = 1;
    } else {
        P.Flap = 0;
        P.AnimationFrames = 0;
        P.FlapAnim = 0;
    }
    
    // Update game logic
    g->Lcdp.SCX++;
    P.y += 1;
    
    // player collision
    // This is a bad collision test, but good enough for a demo
    int PlayerTileX = ((g->Lcdp.SCX+P.x)/TileSpriteDim)-1;
    int PlayerTileY = ((g->Lcdp.SCY+P.y)/TileSpriteDim)-1;
    if(map_get8(ColMap, PlayerTileX, PlayerTileY) || P.y < 1*TileSpriteDim || P.y > (ScreenH+1)*TileSpriteDim) {
        mode_switch(mode_death);
    }
    
    // Set background pipes
    enum { PipeMinGap = 4, PipeMaxGap = 6, PipeMinFromEdge = 3, PipeSetChunk = (BGMapW-ScreenW)/2, ChunkMargin = ScreenPxW+PipeW*TileSpriteDim};
    // Update the background if the view advances far enough. The chunk that is updated is PipeW(*TileSpriteDim) away from the edge of the screen, in case a pipe is drawn overlapping the previous chunk
    if(g->Lcdp.SCX+ChunkMargin >= Game.NextPipeChunkX*TileSpriteDim) {
        int OldPipeChunkX = Game.NextPipeChunkX;
        Game.NextPipeChunkX += PipeSetChunk;
        bg_clear_rect(OldPipeChunkX, 0, Game.NextPipeChunkX, ScreenH);
        // Set next pipe if it fits inside this current chunk. Offset pipe width to make sure the pipe that is entirely on already cleared background.
        while((Game.NextPipeX >= OldPipeChunkX-PipeW) &&
              (Game.NextPipeX < Game.NextPipeChunkX-PipeW)) {
            // get random pipe arguments
            int Top = random_between(PipeMinFromEdge, ScreenH-PipeMinFromEdge-PipeMinGap);
            int Gap = random_between(PipeMinGap, PipeMaxGap);
            place_pipe_tiles(Game.NextPipeX, Top, Gap);
            // choose next pipe position
            int RandomMin = Game.NextPipeX+PipeW+4;
            Game.NextPipeX = random_between(RandomMin, RandomMin+10);
        }
    }
    
    // update bg music
    enum { BGNextNoteFrames = 25 };
    if(Game.BGNoteFrames == 0) {
        Game.BGNoteFrames = BGNextNoteFrames;
        int Hz = Notes[BGMusic[Game.BGNoteIndex]];
        Game.BGNoteIndex = (Game.BGNoteIndex+1) % ArrayLength(BGMusic);
        
        g->AudCh2 = (aud_ch2) {
            .Reset = 1,
            .Enabled = 1,
            .Hz = Hz,
            .VolSweeps = 7,
            .Vol = 7,
            .DutyChoice = 2,
        };
        
        g->AudCh3.Reset = 1;
        g->AudCh3.Enabled = 1;
        g->AudCh3.Hz = Hz;
        g->AudCh3.VolShift = 1;
    } else {
        Game.BGNoteFrames--;
    }
    
    // update sprites
    update_player_sprite_data(&P);
    if(Game.LeaveMode) {
        Game.LeaveMode = 0;
        g->AudCh2.Enabled = 0;
        g->AudCh3.Enabled = 0;
    }
}

enum { FlipModeCountdown = 15};
void run_mode_death(void) {
    if(!Game.ModeInitialized) {
        Game.ModeInitialized = 1;
        Game.DeathMode = death_mode_hit;
        Game.DeathModeInitial = 1;
    }
    
    switch(Game.DeathMode) {
        case death_mode_hit: {
            if(Game.DeathModeInitial) {
                g->AudCh1 = (aud_ch1) {
                    .Reset = 1,
                    .Enabled = 1,
                    .Hz = Notes[note_c],
                    .Vol = 7,
                    .DutyChoice = 2,
                    .SoundLen = 0,
                    .UseSoundLen = 1,
                    .FreqSweeps = 7,
                    .FreqSweepUp = 0,
                    .FreqSweepTime = 4,
                };
                Game.DeathModeCountdown = 15;
            }
            Game.DeathModeInitial = 0;
            if(Game.DeathModeCountdown <= 0) {
                P.FlipY = 1;
                Game.DeathMode = death_mode_flip_up;
                Game.DeathModeInitial = 1;
            }
        } break;
        case death_mode_flip_up: {
            if(Game.DeathModeInitial) {
                Game.DeathModeInitial = 0;
                Game.DeathModeCountdown = FlipModeCountdown;
            }
            int Delta = (Game.DeathModeCountdown)/4;
            Delta = max(Delta, 2);
            if((Game.DeathModeCountdown > 5) || 
               Game.DeathModeCountdown%2) P.y-=Delta;
            if(Game.DeathModeCountdown <= 0) {
                Game.DeathMode = death_mode_down;
                Game.DeathModeInitial = 1;
            }
        } break;
        case death_mode_down: {
            if(Game.DeathModeInitial) {
                Game.DeathModeInitial = 0;
            }
            P.y+=3;
            if(P.y > ScreenPxH+(TileSpriteDim*2)) {
                mode_switch(mode_score);
            }
        } break;
    }
    Game.DeathModeCountdown--;
    
    // update sprites
    update_player_sprite_data(&P);
    if(Game.LeaveMode) {
        Game.LeaveMode = 0;
    }
}

void run_mode_score(void) {
    if(!Game.ModeInitialized) {
        Game.ModeInitialized = 1;
        
        int StrLen = snprintf(TmpStrBuffer, ArrayLength(TmpStrBuffer), "SCORE %i", g->Lcdp.SCX);
        const char *PlayAgainStr = "PRESS UP TO PLAY AGAIN!";
        int StrLen2 = (int)strlen(PlayAgainStr);
        
        int TileXOff = g->Lcdp.SCX / TileSpriteDim;
        
        int XScore = ((ScreenW-StrLen)/2)+TileXOff;
        int YScore = ScreenH/2-1;
        
        int XPlayAgain = ((ScreenW-StrLen2)/2)+TileXOff;
        int YPlayAgain = ScreenH/2;
        
        int XMin = min(XScore, XPlayAgain)-1;
        int XMax = XMin+2+max(StrLen, StrLen2);
        int YMin = YScore-1;
        int YMax = YPlayAgain+2;
        set_tile_text_box(XMin, YMin, XMax, YMax);
        set_tile_string(XScore, YScore, TmpStrBuffer);
        set_tile_string(XPlayAgain, YPlayAgain, PlayAgainStr);
        map_set_rect_attr(g->BGAttrMap, XMin, YMin, XMax, YMax, (bg_attr){0});
    }
    
    if(g->InUp.Up) {
        mode_switch(mode_pre);
    }
    
    if(Game.LeaveMode) {
        Game.LeaveMode = 0;
    }
}

DLLEXPORT void update(gep_state *GS) {
    if(!Game.Initialized) {
        Game.Initialized = 1;
        g = GS;
        GlobalCharLookupTable = CharLookup;
        GlobalTextBoxBorderLookupTable = TextBoxBorderLookup;
        
        g->Palettes[0] = PalDefault;
        g->Lcdc.r = DefaultBorderColor[0];
        g->Lcdc.g = DefaultBorderColor[1];
        g->Lcdc.b = DefaultBorderColor[2];
    }
    
    switch(Game.Mode) {
        case mode_pre: {
            run_mode_pre();
        } break;
        case mode_game: {
            run_mode_game();
        } break;
        default:
        case mode_death: {
            run_mode_death();
        } break;
        case mode_score: {
            run_mode_score();
        } break;
    }
}
