#include <stdio.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include <stdlib.h>
#include <float.h>

#define USE_VSYNC 1

#include "includes/gep.h"
#include "gep_internal.h"
#define UseGlDriver 0
static int DbgOpenGlDriver = -1;
static SDL_AudioDeviceID AudioDev;
static SDL_Renderer *Renderer;
static SDL_Texture* ScreenTex;
static SDL_Window *Window;
static u64 BeginFrame;
static b32 Fullscreen; 

static int WinW = ScreenPxW*5;
static int WinH = ScreenPxH*5;
static int ActualW;
static int ActualH;

void collect_vsync_info(SDL_Window *Win, b32 *VsyncEnabled, f64 *VsyncHz) {
    SDL_SetRenderDrawColor(Renderer, g->Lcdc.r,g->Lcdc.g,g->Lcdc.b, 255);
    SDL_RenderClear(Renderer);
    SDL_RenderPresent(Renderer);
    int Runs = 10;
    u64 AccTime = 0;
    u64 PrevTime = SDL_GetPerformanceCounter();
    pfori(Runs) {
        SDL_RenderClear(Renderer);
        SDL_RenderPresent(Renderer);
        u64 NowTime = SDL_GetPerformanceCounter();
        AccTime += NowTime-PrevTime;
        PrevTime = NowTime;
    }
    f64 AvgMs = (((f64)AccTime*1000.0)/(f64)Runs)/(f64)SDL_GetPerformanceFrequency();
    SDL_LogInfo(0, "PresentTime: %.2fms", AvgMs);
    if(AvgMs < 1.0) {
        VsyncEnabled = 0;
        SDL_LogInfo(0, "Guessing that vsync is disabled");
    } else {
        *VsyncEnabled = 1;
        int DispIndex = SDL_GetWindowDisplayIndex(Win);
        SDL_DisplayMode DispMode = {0};
        if(SDL_GetCurrentDisplayMode(0, &DispMode)) {
            SDL_LogError(0, "Failed to get display mode: %s", SDL_GetError());
            exit(1);
        }
        *VsyncHz = DispMode.refresh_rate;
        SDL_LogInfo(0, "Guessing that vsync enabled at %.2fhz", *VsyncHz);
    }
}

static b32 VsyncEnabled;
static f64 VsyncHz;

static b32 LeftFullscreenFix;
static b32 IsFullscreen;

b32 set_fullscreen(SDL_Window *Win, b32 Borderless, b32 Enable);
#include "gep.c"
b32 set_fullscreen(SDL_Window *Win, b32 Borderless, b32 Enable) {
    int DispIndex = SDL_GetWindowDisplayIndex(Win);
    SDL_DisplayMode DispMode = {0};
    if(SDL_GetDesktopDisplayMode(0, &DispMode)) {
        SDL_LogError(0, "Failed to get display mode: %s", SDL_GetError());
        return 0;
    }
    DispMode.refresh_rate = 60;
    DispMode.driverdata = 0;
    
    if(SDL_SetWindowDisplayMode(Win, &DispMode)) {
        SDL_LogError(0, "Failed to set window display mode: %s", SDL_GetError());
        return 0;
    }
    
    u32 Flag = 0;
    if(Enable) {
        Flag = SDL_WINDOW_FULLSCREEN;
        if(Borderless) {
            Flag = SDL_WINDOW_FULLSCREEN_DESKTOP;
        }
    }
    
    if(SDL_SetWindowFullscreen(Win, Flag)) {
        SDL_LogError(0, "Failed to set window fullscreen: %s", SDL_GetError());
        return 0;
    }
    
    if(Enable) {
        if(!Borderless) {
            f64 TmpHz;
            collect_vsync_info(Win, &VsyncEnabled, &TmpHz);
        }
        ScreenScale = min(DispMode.w/ScreenPxW, DispMode.h/ScreenPxH);
        ActualW = DispMode.w;
        ActualH = DispMode.h;
        VsyncHz = (f64)DispMode.refresh_rate;
        if(SDL_ShowCursor(SDL_DISABLE) < 0) {
            SDL_LogError(0, "Failed to hide mouse cursor: %s", SDL_GetError());
        }
        IsFullscreen = 1;
    } else {
        ScreenScale = min(WinW/ScreenPxW, WinH/ScreenPxH);
        LeftFullscreenFix = 1;
        ActualW = WinW;
        ActualH = WinH;
        if(SDL_GetWindowDisplayMode(Win, &DispMode)) {
            SDL_LogError(0, "Failed to set window display mode: %s", SDL_GetError());
            return 0;
        }
        VsyncHz = (f64)DispMode.refresh_rate;
        if(SDL_ShowCursor(SDL_ENABLE) < 0) {
            SDL_LogError(0, "Failed to resotre mouse cursor: %s", SDL_GetError());
        }
        f64 TmpHz;
        collect_vsync_info(Win, &VsyncEnabled, &TmpHz);
        IsFullscreen = 0;
    }
    
    return 1;
}

int main(int argc, char** argv) {
    g = &GepState;
    gi = &GepStateInt;
    switch_to_settings = gep_switch_to_settings;
    
    if(argc == 3 && strcmp(argv[1], "--screencapdir")==0) {
        ScreenCaptureDirectory = argv[2];
    }
    
    SDL_LogSetPriority(0, SDL_LOG_PRIORITY_VERBOSE);
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)) {
        SDL_LogError(0, "Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }
    
    void *GameDLL = SDL_LoadObject("game.dll"); // "Leaks" intentionally, loaded for full application lifetime
    if(!GameDLL) {
        SDL_LogError(0, "Failed to open game.dll: %s", SDL_GetError());
        return 1;
    }
    
    update = (update_function)SDL_LoadFunction(GameDLL, "update");
    if(!update) {
        SDL_LogError(0, "Failed to find \"update()\" symbol in game.dll: %s", SDL_GetError());
        return 1;
    }
    
    const char **OrgConfPathPointer = SDL_LoadFunction(GameDLL, "ORG_CONF_PATH");
    if(!OrgConfPathPointer) {
        SDL_LogError(0, "Failed to find \"ORG_CONF_PATH\" symbol in game.dll: %s", SDL_GetError());
        return 1;
    }
    const char *OrgConfPath = *OrgConfPathPointer;
    
    const char **GameNameConfPathPointer = SDL_LoadFunction(GameDLL, "GAMENAME_CONF_PATH");
    if(!GameNameConfPathPointer) {
        SDL_LogError(0, "Failed to find \"GAMENAME_CONF_PATH\" symbol in game.dll: %s", SDL_GetError());
        return 1;
    }
    const char *GameNameConfPath = *GameNameConfPathPointer;
    
    const char **GameNamePointer = SDL_LoadFunction(GameDLL, "GAMENAME");
    if(!GameNamePointer) {
        SDL_LogError(0, "Failed to find \"GAMENAME\" symbol in game.dll: %s", SDL_GetError());
        return 1;
    }
    const char *GameName = *GameNamePointer;
    // Intentional "leak", liftime of application
    PrefPath = SDL_GetPrefPath(OrgConfPath, GameNameConfPath);
    if(!PrefPath) {
        SDL_LogError(0, "Failed to get PrefPath");
    }
    
    // set default values here
    prefs *Prefs = &(prefs){.Version=1, .Fullscreen=0, .Volume=50};
    if(!settings_read(Prefs)) {
        SDL_LogWarn(0, "Failed to read settings, this is normal on first start or when the config file was deleted. Just keep going with defaults, and create a default config file");
        if(!settings_write(Prefs)) {
            SDL_LogError(0, "Failed to write default settings, something is wrong with the system configuration or storage device.");
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                                     "Failed to write default settings file",
                                     "Failed to write default settings file. Something is wrong with the system configuration or storage device. Exiting",
                                     Window);
            return 1;
            
        }
    }
    settings_early_apply(Prefs);
    
    SDL_DisableScreenSaver();
    
    // init window and renderer
    SDL_LogInfo(0, "Renderers:");
    int NumDrivers = SDL_GetNumRenderDrivers();
    pfori32(NumDrivers) {
        SDL_RendererInfo Info;
        if(SDL_GetRenderDriverInfo(i, &Info)) {
            SDL_LogError(0, "Failed to query render driver info: %s", SDL_GetError());
            return 1;
        }
        
#if UseGlDriver
        if(strcmp("opengl", Info.name) == 0) DbgOpenGlDriver = i;
#endif
        SDL_LogInfo(0, "%s", Info.name);
    }
    
    ActualW = WinW;
    ActualH = WinH;
    Window = SDL_CreateWindow(GameName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, ActualW, ActualH, 0);
    if(!Window) {
        SDL_LogError(0, "Failed to create window: %s", SDL_GetError());
        return 1;
    }
    
    int FlagVsync = 0;
#if USE_VSYNC
    FlagVsync = SDL_RENDERER_PRESENTVSYNC;
#endif
    Renderer = SDL_CreateRenderer(Window,
                                  DbgOpenGlDriver,
                                  SDL_RENDERER_ACCELERATED|FlagVsync);
    if(!Renderer) {
        SDL_LogError(0, "Failed to create renderer: %s", SDL_GetError());
        return 1;
    }
    
    SDL_RendererInfo Info = {0};
    if(SDL_GetRendererInfo(Renderer, &Info)) {
        SDL_LogError(0, "Failed to query renderer info: %s", SDL_GetError());
        return 1;
    }
    SDL_LogInfo(0, "Renderer: %s", Info.name);
    collect_vsync_info(Window, &VsyncEnabled, &VsyncHz);
    
    ScreenTex = SDL_CreateTexture(Renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 
                                  ScreenPxW, ScreenPxH);
    if(!ScreenTex) {
        SDL_LogError(0, "Failed to create screen texture: %s", SDL_GetError());
        return 1;
    }
    
    // init audio
    SDL_AudioSpec Have;
    SDL_AudioSpec Want = {.freq=AudioSampleRate, .format=AUDIO_S8, .channels=AudioChannels, .samples=512};
    
    AudioDev = SDL_OpenAudioDevice(NULL, 0, &Want, &Have, 0);
    if(!AudioDev) {
        SDL_LogError(0, "Failed to open audio device: %s", SDL_GetError());
        return 1;
    }
    SDL_PauseAudioDevice(AudioDev, 0);
    
    settings_late_apply(Prefs);
    
    f64 UpdateRun = 0;
    b32 ToggleSettings = 0;
    int Running = 1;
    while(Running) {
        BeginFrame = SDL_GetPerformanceCounter();
        if(VsyncEnabled) {
            // calculate number of update timesteps (calls to the update() function) in one render frame (from one present() to the next)
            UpdateRun += 60.0/VsyncHz; 
        }
        
        while(UpdateRun >= 0.0) {
            if(ToggleSettings) {
                ToggleSettings = 0;
                toggle_settings();
            }
            
            gepinput LastFrameInput = g->Input;
            devinput LastFrameDevInput = g->DevInput;
            input_begin_frame_reset();
            SDL_Event ev;
            while (SDL_PollEvent(&ev)) {
                switch(ev.type) {
                    case SDL_QUIT: {
                        Running = 0;
                    } break;
                    case SDL_WINDOWEVENT: {
                        SDL_WindowEvent *e = (SDL_WindowEvent *)&ev;
                        switch(e->event) {
                            case SDL_WINDOWEVENT_CLOSE: {
                                if(e->windowID == SDL_GetWindowID(TileMapDevWindow)) {
                                    TileMapDevMode = 0;
                                } else if(e->windowID == SDL_GetWindowID(WindowMapDevWindow)) {
                                    WindowMapDevMode = 0;
                                } else if(e->windowID == SDL_GetWindowID(TileSpriteMemDevWindow)) {
                                    TileSpriteMemDevMode = 0;
                                }
                            } break;
                        }
                    } break;
                    case SDL_MOUSEMOTION: {
                        SDL_MouseMotionEvent *e = (SDL_MouseMotionEvent *)&ev;
                        devinput *d = &g->DevInput;
                        if(e->windowID == SDL_GetWindowID(TileMapDevWindow)) {
                            d = &TileMapDevInput;
                        } else if(e->windowID == SDL_GetWindowID(WindowMapDevWindow)) {
                            d = &WindowMapDevInput;
                        } else if(e->windowID == SDL_GetWindowID(TileSpriteMemDevWindow)) {
                            d = &TileSpriteDevInput;
                        }
                        f32 ex = (f32)e->x;
                        f32 ey = (f32)e->y;
                        d->mx = ex;
                        d->my = ey;
                    } break;
                    case SDL_MOUSEBUTTONDOWN: {
                        SDL_MouseButtonEvent *e = (SDL_MouseButtonEvent *)&ev;
                        devinput *d = &g->DevInput;
                        if(e->windowID == SDL_GetWindowID(TileMapDevWindow)) {
                            d = &TileMapDevInput;
                        } else if(e->windowID == SDL_GetWindowID(WindowMapDevWindow)) {
                            d = &WindowMapDevInput;
                        } else if(e->windowID == SDL_GetWindowID(TileSpriteMemDevWindow)) {
                            d = &TileSpriteDevInput;
                        }
                        
                        switch(e->button) {
                            case SDL_BUTTON_LEFT: {
                                d->ml = e->clicks;
                            } break;
                            case SDL_BUTTON_MIDDLE: {
                                d->mm = e->clicks;
                            } break;
                            case SDL_BUTTON_RIGHT: {
                                d->mr = e->clicks;
                            } break;
                        }
                    } break;
                    case SDL_MOUSEBUTTONUP: {
                        SDL_MouseButtonEvent *e = (SDL_MouseButtonEvent *)&ev;
                        devinput *d = &g->DevInput;
                        if(e->windowID == SDL_GetWindowID(TileMapDevWindow)) {
                            d = &TileMapDevInput;
                        } else if(e->windowID == SDL_GetWindowID(WindowMapDevWindow)) {
                            d = &WindowMapDevInput;
                        } else if(e->windowID == SDL_GetWindowID(TileSpriteMemDevWindow)) {
                            d = &TileSpriteDevInput;
                        }
                        
                        switch(e->button) {
                            case SDL_BUTTON_LEFT: {
                                d->ml = 0;
                            } break;
                            case SDL_BUTTON_MIDDLE: {
                                d->mm = 0;
                            } break;
                            case SDL_BUTTON_RIGHT: {
                                d->mr = 0;
                            } break;
                        }
                    } break;
                    case SDL_MOUSEWHEEL: {
                        SDL_MouseWheelEvent *e = (SDL_MouseWheelEvent *)&ev;
                        devinput *d = &g->DevInput;
                        if(e->windowID == SDL_GetWindowID(TileMapDevWindow)) {
                            d = &TileMapDevInput;
                        } else if(e->windowID == SDL_GetWindowID(WindowMapDevWindow)) {
                            d = &WindowMapDevInput;
                        } else if(e->windowID == SDL_GetWindowID(TileSpriteMemDevWindow)) {
                            d = &TileSpriteDevInput;
                        }
                        
                        d->mwd += (f32)e->y;
                    } break;
                    case SDL_KEYDOWN: {
                        SDL_KeyboardEvent *e = (SDL_KeyboardEvent *)&ev;
                        switch(e->keysym.scancode) {
                            case SDL_SCANCODE_W: {
                                g->Input.Up = 1;
                                //g->InDown.Up = 1;
                            } break;
                            case SDL_SCANCODE_A: {
                                g->Input.Left = 1;
                                //g->InDown.Left = 1;
                            } break;
                            case SDL_SCANCODE_S: {
                                g->Input.Down = 1;
                                //g->InDown.Down = 1;
                            } break;
                            case SDL_SCANCODE_D: {
                                g->Input.Right = 1;
                                //g->InDown.Right = 1;
                            } break;
                            
                            case SDL_SCANCODE_Q: {
                                g->Input.L1 = 1;
                                //g->InDown.L1 = 1;
                            } break;
                            case SDL_SCANCODE_E: {
                                g->Input.R1 = 1;
                                //g->InDown.R1 = 1;
                            } break;
                            case SDL_SCANCODE_1: {
                                g->Input.L2 = 1;
                                //g->InDown.L2 = 1;
                            } break;
                            case SDL_SCANCODE_3: {
                                g->Input.R2 = 1;
                                //g->InDown.R2 = 1;
                            } break;
                            
                            case SDL_SCANCODE_UP: {
                                g->Input.a = 1;
                                //g->InDown.a = 1;
                            } break;
                            case SDL_SCANCODE_DOWN: {
                                g->Input.b = 1;
                                //g->InDown.b = 1;
                            } break;
                            case SDL_SCANCODE_LEFT: {
                                g->Input.x = 1;
                                //g->InDown.x = 1;
                            } break;
                            case SDL_SCANCODE_RIGHT: {
                                g->Input.y = 1;
                                //g->InDown.y = 1;
                            } break;
                            
                            case SDL_SCANCODE_RETURN: {
                                g->Input.Menu = 1;
                                //g->InDown.Menu = 1;
                            } break;
                            case SDL_SCANCODE_BACKSPACE: {
                                g->Input.Option = 1;
                                //g->InDown.Option = 1;
                            } break;
                            
                            case SDL_SCANCODE_ESCAPE: {
                                ToggleSettings = 1;
                            } break;
                            case SDL_SCANCODE_F4: {
                                TakeScreenshot = 1;
                            } break;
                            case SDL_SCANCODE_F5: {
                                if(TakeScreenRecording) TakeScreenRecording = 0;
                                else TakeScreenRecording = 1;
                            } break;
                            case SDL_SCANCODE_F9: {
                                TileMapDevMode = !TileMapDevMode;
                            } break;
                            case SDL_SCANCODE_F10: {
                                WindowMapDevMode = !WindowMapDevMode;
                            } break;
                            case SDL_SCANCODE_F11: {
                                TileSpriteMemDevMode = !TileSpriteMemDevMode;
                            } break;
                            
                            case SDL_SCANCODE_LCTRL: {
                                devinput *d = &g->DevInput;
                                if(e->windowID == SDL_GetWindowID(TileMapDevWindow)) {
                                    d = &TileMapDevInput;
                                } else if(e->windowID == SDL_GetWindowID(WindowMapDevWindow)) {
                                    d = &WindowMapDevInput;
                                } else if(e->windowID == SDL_GetWindowID(TileSpriteMemDevWindow)) {
                                    d = &TileSpriteDevInput;
                                }
                                d->ctrl = 1;
                            } break;
                        }
                        
                    } break;
                    case SDL_KEYUP: {
                        SDL_KeyboardEvent *e = (SDL_KeyboardEvent *)&ev;
                        switch(e->keysym.scancode) {
                            case SDL_SCANCODE_W: {
                                g->Input.Up = 0;
                                //g->InUp.Up = 1;
                            } break;
                            case SDL_SCANCODE_A: {
                                g->Input.Left = 0;
                                //g->InUp.Left = 1;
                            } break;
                            case SDL_SCANCODE_S: {
                                g->Input.Down = 0;
                                //g->InUp.Down = 1;
                            } break;
                            case SDL_SCANCODE_D: {
                                g->Input.Right = 0;
                                //g->InUp.Right = 1;
                            } break;
                            
                            case SDL_SCANCODE_Q: {
                                g->Input.L1 = 0;
                                //g->InUp.L1 = 1;
                            } break;
                            case SDL_SCANCODE_E: {
                                g->Input.R1 = 0;
                                //g->InUp.R1 = 1;
                            } break;
                            case SDL_SCANCODE_1: {
                                g->Input.L2 = 0;
                                //g->InUp.L2 = 1;
                            } break;
                            case SDL_SCANCODE_3: {
                                g->Input.R2 = 0;
                                //g->InUp.R2 = 1;
                            } break;
                            
                            case SDL_SCANCODE_UP: {
                                g->Input.a = 0;
                                //g->InUp.a = 1;
                            } break;
                            case SDL_SCANCODE_DOWN: {
                                g->Input.b = 0;
                                //g->InUp.b = 1;
                            } break;
                            case SDL_SCANCODE_LEFT: {
                                g->Input.x = 0;
                                //g->InUp.x = 1;
                            } break;
                            case SDL_SCANCODE_RIGHT: {
                                g->Input.y = 0;
                                //g->InUp.y = 1;
                            } break;
                            
                            case SDL_SCANCODE_RETURN: {
                                g->Input.Menu = 0;
                                //g->InUp.Menu = 1;
                            } break;
                            case SDL_SCANCODE_BACKSPACE: {
                                g->Input.Option = 0;
                                //g->InUp.Option = 1;
                            } break;
                            case SDL_SCANCODE_LCTRL: {
                                devinput *d = &g->DevInput;
                                if(e->windowID == SDL_GetWindowID(TileMapDevWindow)) {
                                    d = &TileMapDevInput;
                                } else if(e->windowID == SDL_GetWindowID(WindowMapDevWindow)) {
                                    d = &WindowMapDevInput;
                                } else if(e->windowID == SDL_GetWindowID(TileSpriteMemDevWindow)) {
                                    d = &TileSpriteDevInput;
                                }
                                d->ctrl = 0;
                            } break;
                        }
                        
                    } break;
                }
            }
            if(LeftFullscreenFix) {
                LeftFullscreenFix = 0;
                SDL_SetWindowSize(Window, WinW, WinH);
                SDL_SetWindowPosition(Window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
            }
            
            input_calc_delta(g, &LastFrameInput, &LastFrameDevInput);
            
            // game update
            BeforeGameUpdate = SDL_GetPerformanceCounter();
            update(g);
            AfterGameUpdate = SDL_GetPerformanceCounter();
            UpdateRun -= 1.0; // subtract one whole timestep for one call to update()
            
            // do gep stuff
            gep_update();
        }
        
        gep_present();
        if (!VsyncEnabled) {
            // No Vsync, spinlock to at least get timing right
            f64 PerfFreq = (f64)SDL_GetPerformanceFrequency();
            f64 FrameTime = 0;
            while(FrameTime <= 16.66) {
                u64 NowTime = SDL_GetPerformanceCounter();
                FrameTime = (f64)(NowTime-BeginFrame)*1000.0/PerfFreq;
            }
        }
    }
    
    SDL_Quit();
    return 0;
}
