
        static u8 _DevPalNeutral[] = {
    255,255,255,
    192,192,192,
    105,106,106,
    7,9,9,
};

void create_devtool_window(const char *WindowTitle, int Width, int Height, int Zoom, SDL_Window **WindowOut, SDL_Renderer **RendererOut, SDL_Texture **TextureOut) {
    *WindowOut = SDL_CreateWindow(WindowTitle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Width*Zoom, Height*Zoom, 0);
    if(!*WindowOut) {
        SDL_LogError(0, "Failed to create window '%s': %s", WindowTitle, SDL_GetError());
        exit(1);
    }
    
    *RendererOut = SDL_CreateRenderer(*WindowOut,
                                      DbgOpenGlDriver,
                                      SDL_RENDERER_ACCELERATED);
    if(!*RendererOut) {
        SDL_LogError(0, "Failed to create renderer for window '%s': %s", WindowTitle, SDL_GetError());
        exit(1);
    }
    
    SDL_RendererInfo TileMapDevInfo = {0};
    if(SDL_GetRendererInfo(*RendererOut, &TileMapDevInfo)) {
        SDL_LogError(0, "Failed to query renderer info for window '%s': %s", WindowTitle, SDL_GetError());
        exit(1);
    }
    SDL_LogInfo(0, "TileMapDev Renderer: %s", TileMapDevInfo.name);
    
    *TextureOut = SDL_CreateTexture(*RendererOut, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 
                                    Width, Height);
    if(!*TextureOut) {
        SDL_LogError(0, "Failed to create screen texture or window '%s': %s", WindowTitle, SDL_GetError());
        exit(1);
    }
}

void destroy_devtool_window(SDL_Window **DevWindow, SDL_Renderer **DevRenderer, SDL_Texture **DevTexture) {
    SDL_DestroyRenderer(*DevRenderer);
    *DevRenderer = 0;
    SDL_DestroyTexture(*DevTexture);
    *DevTexture = 0;
    SDL_DestroyWindow(*DevWindow);
    *DevWindow = 0;
}

void update_developer_tools(void) {
    if(TileMapDevMode) {
        enum { TileMapDevZoom = 3 };
        if(!TileMapDevWindow) {
            create_devtool_window("GameEngine pocket TileMapDev", BGMapPxW, BGMapPxH, TileMapDevZoom, &TileMapDevWindow, &TileMapDevRenderer, &TileMapDevTexture);
        }
        
        void *Pixels;
        int Pitch;
        if(SDL_LockTexture(TileMapDevTexture, 0, &Pixels, &Pitch)) {
            SDL_LogError(0, "Failed to lock screen texture: %s", SDL_GetError());
            exit(1);
        }
        
        pfory32(BGMapPxH) {
            u8 *Line = ((u8 *)Pixels)+(Pitch*y);
            pforx32(BGMapPxW) {
                int TileX = x/TileSpriteDim;
                int TileY = y/TileSpriteDim;
                int InTileX = x%TileSpriteDim;
                int InTileY = y%TileSpriteDim;
                int TileId = g->BGMap[BGMapW*TileY+TileX];
                u8 c = g->TileSpriteMemory[(TileId*TileSpriteSize)+InTileY*TileSpriteDim+InTileX];
                bg_attr Attr = g->BGAttrMap[BGMapW*TileY+TileX];
                i8 PalIndex = Attr.Palette;
                u8 *Pix = Line+x*4;
                Pix[0] = 255; // Alpha
                Pix[1] = g->Palettes[PalIndex].E[c*3+ShadeB]; // Blue
                Pix[2] = g->Palettes[PalIndex].E[c*3+ShadeG]; // Green
                Pix[3] = g->Palettes[PalIndex].E[c*3+ShadeR]; // Red
            }
        }
        
        SDL_UnlockTexture(TileMapDevTexture);
        
        if(SDL_RenderCopy(TileMapDevRenderer, TileMapDevTexture, 0, 0)) {
            SDL_LogError(0, "Failed to copy screen texture: %s", SDL_GetError());
            exit(1);
        }
        
        if(SDL_SetRenderDrawColor(TileMapDevRenderer, 255,255,255,255)) {
            SDL_LogError(0, "Failed to set dev mode render draw color: %s", SDL_GetError());
            exit(1);
        }
        
        SDL_Rect ClipRect = {0, 0, BGMapPxW*TileMapDevZoom, BGMapPxH*TileMapDevZoom};
        if(SDL_RenderSetClipRect(TileMapDevRenderer, &ClipRect)) {
            SDL_LogError(0, "Failed to set dev mode clip rect: %s", SDL_GetError());
            exit(1);
        }
        
        SDL_Rect ViewRect = {(g->Lcdp.SCX%BGMapPxW), (g->Lcdp.SCY%BGMapPxH), ScreenPxW, ScreenPxH };
        if(ViewRect.x < 0) ViewRect.x += BGMapPxW;
        if(ViewRect.y < 0) ViewRect.y += BGMapPxH;
        b32 DrawRect2 = 0;
        b32 DrawRect3 = 0;
        b32 DrawRect4 = 0;
        if(ViewRect.x+ViewRect.w > BGMapPxW) {
            DrawRect2 = 1;
        }
        if(ViewRect.y+ViewRect.h > BGMapPxH) {
            DrawRect3 = 1;
        }
        if(DrawRect2 && DrawRect3) DrawRect4 = 1;
        
        if(DrawRect2) {
            SDL_Rect ViewRect2 = {
                -(BGMapPxW-ViewRect.x), ViewRect.y, ScreenPxW, ScreenPxH };
            
            ViewRect2.x *= TileMapDevZoom;
            ViewRect2.y *= TileMapDevZoom;
            ViewRect2.w *= TileMapDevZoom;
            ViewRect2.h *= TileMapDevZoom;
            
            if(SDL_RenderDrawRect(TileMapDevRenderer, &ViewRect2)) {
                SDL_LogError(0, "Failed to draw view rect: %s", SDL_GetError());
                exit(1);
            }
        }
        
        if(DrawRect3) {
            SDL_Rect ViewRect3 = {
                ViewRect.x, ViewRect.y-BGMapPxH, ScreenPxW, ScreenPxH };
            
            ViewRect3.x *= TileMapDevZoom;
            ViewRect3.y *= TileMapDevZoom;
            ViewRect3.w *= TileMapDevZoom;
            ViewRect3.h *= TileMapDevZoom;
            
            if(SDL_RenderDrawRect(TileMapDevRenderer, &ViewRect3)) {
                SDL_LogError(0, "Failed to draw view rect: %s", SDL_GetError());
                exit(1);
            }
        }
        
        if(DrawRect4) {
            SDL_Rect ViewRect4 = {
                -(BGMapPxW-ViewRect.x), ViewRect.y-BGMapPxH, ScreenPxW, ScreenPxH };
            
            ViewRect4.x *= TileMapDevZoom;
            ViewRect4.y *= TileMapDevZoom;
            ViewRect4.w *= TileMapDevZoom;
            ViewRect4.h *= TileMapDevZoom;
            
            if(SDL_RenderDrawRect(TileMapDevRenderer, &ViewRect4)) {
                SDL_LogError(0, "Failed to draw view rect: %s", SDL_GetError());
                exit(1);
            }
        }
        
        ViewRect.x *= TileMapDevZoom;
        ViewRect.y *= TileMapDevZoom;
        ViewRect.w *= TileMapDevZoom;
        ViewRect.h *= TileMapDevZoom;
        if(SDL_RenderDrawRect(TileMapDevRenderer, &ViewRect)) {
            SDL_LogError(0, "Failed to draw view rect: %s", SDL_GetError());
            exit(1);
        }
        
        if(SDL_RenderSetClipRect(TileMapDevRenderer, 0)) {
            SDL_LogError(0, "Failed to set dev mode clip rect: %s", SDL_GetError());
            exit(1);
        }
        SDL_RenderPresent(TileMapDevRenderer);
    } else {
        if(TileMapDevWindow) {
            destroy_devtool_window(&TileMapDevWindow, &TileMapDevRenderer, &TileMapDevTexture);
        }
    }
    
    if(WindowMapDevMode) {
        enum { WindowMapDevZoom = 3 };
        if(!WindowMapDevWindow) {
            create_devtool_window("GameEngine pocket WindowMapDev", BGMapPxW, BGMapPxH, WindowMapDevZoom, &WindowMapDevWindow, &WindowMapDevRenderer, &WindowMapDevTexture);
        }
        
        void *Pixels;
        int Pitch;
        if(SDL_LockTexture(WindowMapDevTexture, 0, &Pixels, &Pitch)) {
            SDL_LogError(0, "Failed to lock screen texture: %s", SDL_GetError());
            exit(1);
        }
        
        pfory32(BGMapPxH) {
            u8 *Line = ((u8 *)Pixels)+(Pitch*y);
            pforx32(BGMapPxW) {
                int TileX = x/TileSpriteDim;
                int TileY = y/TileSpriteDim;
                int InTileX = x%TileSpriteDim;
                int InTileY = y%TileSpriteDim;
                int TileId = g->WinMap[BGMapW*TileY+TileX];
                u8 c = g->TileSpriteMemory[(TileId*TileSpriteSize)+InTileY*TileSpriteDim+InTileX];
                bg_attr Attr = g->BGAttrMap[BGMapW*TileY+TileX];
                i8 PalIndex = Attr.Palette;
                u8 *Pix = Line+x*4;
                Pix[0] = 255; // Alpha
                Pix[1] = g->Palettes[PalIndex].E[c*3+ShadeB]; // Blue
                Pix[2] = g->Palettes[PalIndex].E[c*3+ShadeG]; // Green
                Pix[3] = g->Palettes[PalIndex].E[c*3+ShadeR]; // Red
            }
        }
        
        SDL_UnlockTexture(WindowMapDevTexture);
        
        if(SDL_RenderCopy(WindowMapDevRenderer, WindowMapDevTexture, 0, 0)) {
            SDL_LogError(0, "Failed to copy screen texture: %s", SDL_GetError());
            exit(1);
        }
        
        if(SDL_SetRenderDrawColor(WindowMapDevRenderer, 255,255,255,255)) {
            SDL_LogError(0, "Failed to set dev mode render draw color: %s", SDL_GetError());
            exit(1);
        }
        SDL_RenderPresent(WindowMapDevRenderer);
    } else {
        if(WindowMapDevWindow) {
            destroy_devtool_window(&WindowMapDevWindow, &WindowMapDevRenderer, &WindowMapDevTexture);
        }
    }
    
    if(TileSpriteMemDevMode) {
        enum { TileSpriteMemDevZoom = 5 };
        const char WindowTitle[] = "GameEngine pocket TileSpriteMemDev";
        if(!TileSpriteMemDevWindow) {
            create_devtool_window("GameEngine pocket TileSpriteMemDev", TileSpritePxW, TileSpritePxH, TileSpriteMemDevZoom, &TileSpriteMemDevWindow, &TileSpriteMemDevRenderer, &TileSpriteMemDevTexture);
        }
        void *Pixels;
        int Pitch;
        if(SDL_LockTexture(TileSpriteMemDevTexture, 0, &Pixels, &Pitch)) {
            SDL_LogError(0, "Failed to lock screen texture: %s", SDL_GetError());
            exit(1);
        }
        
        pfory32(TileSpritePxH) {
            u8 *Line = ((u8 *)Pixels)+(Pitch*y);
            pforx32(TileSpritePxW) {
                int TileX = x/TileSpriteDim;
                int TileY = y/TileSpriteDim;
                int InTileX = x%TileSpriteDim;
                int InTileY = y%TileSpriteDim;
                int TileId = TileY*TileSpriteW+TileX;
                u8 c = g->TileSpriteMemory[(TileId*TileSpriteSize)+(InTileY*TileSpriteDim+InTileX)];
                u8 *Pix = Line+x*4;
                Pix[0] = 255; // Alpha
                Pix[1] = _DevPalNeutral[c*3+ShadeB]; // Blue
                Pix[2] = _DevPalNeutral[c*3+ShadeG]; // Green
                Pix[3] = _DevPalNeutral[c*3+ShadeR]; // Red
            }
        }
        
        SDL_UnlockTexture(TileSpriteMemDevTexture);
        
        if(SDL_RenderCopy(TileSpriteMemDevRenderer, TileSpriteMemDevTexture, 0, 0)) {
            SDL_LogError(0, "Failed to copy screen texture: %s", SDL_GetError());
            exit(1);
        }
        
        for(int x=1; x<TileSpriteW; x++) {
            if(SDL_RenderDrawLine(TileSpriteMemDevRenderer, x*TileSpriteDim*TileSpriteMemDevZoom,0,
                                  x*TileSpriteDim*TileSpriteMemDevZoom, TileSpritePxH*TileSpriteMemDevZoom)) {
                SDL_LogError(0, "Failed to draw TileSpriteMem dbg line: %s", SDL_GetError());
                exit(1);
            }
        }
        
        for(int y=1; y<TileSpriteH; y++){
            if(SDL_RenderDrawLine(TileSpriteMemDevRenderer, 0,y*TileSpriteDim*TileSpriteMemDevZoom,
                                  TileSpritePxW*TileSpriteMemDevZoom, y*TileSpriteDim*TileSpriteMemDevZoom)) {
                SDL_LogError(0, "Failed to draw TileSpriteMem dbg line: %s", SDL_GetError());
                exit(1);
            }
        }
        
        if(TileSpriteDevInput.ctrl | 1) { 
            int TileX = ((int)(TileSpriteDevInput.mx/TileSpriteMemDevZoom))/TileSpriteDim;
            int TileY = ((int)(TileSpriteDevInput.my/TileSpriteMemDevZoom))/TileSpriteDim;
            int TileIndex = TileY*TileSpriteW+TileX;
            char WindowTileTitle[ArrayLength(WindowTitle)+20];
            snprintf(WindowTileTitle, ArrayLength(WindowTileTitle), "%s; Tile # %3i", WindowTitle, TileIndex);
            SDL_SetWindowTitle(TileSpriteMemDevWindow,
                               WindowTileTitle);
        } else {
            SDL_SetWindowTitle(TileSpriteMemDevWindow,
                               WindowTitle);
        }
        
        SDL_RenderPresent(TileSpriteMemDevRenderer);
    } else {
        if(TileSpriteMemDevWindow) {
            SDL_DestroyRenderer(TileSpriteMemDevRenderer);
            TileSpriteMemDevRenderer = 0;
            SDL_DestroyTexture(TileSpriteMemDevTexture);
            TileSpriteMemDevTexture = 0;
            SDL_DestroyWindow(TileSpriteMemDevWindow);
            TileSpriteMemDevWindow = 0;
        }
    }
    
    if(TakeScreenshot || TakeScreenRecording) {
        TakeScreenshot = 0;
        enum {NameBufferSize=1024};
        char NameBuffer[NameBufferSize];
        if(TakeScreenRecording) {
            snprintf(NameBuffer, NameBufferSize, "%s/screenshot-%.8i.png", ScreenCaptureDirectory ? ScreenCaptureDirectory : "", TakeScreenRecording);
            TakeScreenRecording++;
        } else {
            snprintf(NameBuffer, NameBufferSize, "%s/screenshot.png", ScreenCaptureDirectory ? ScreenCaptureDirectory : "");
        }
        enum{NumPx=ScreenPxW*ScreenPxH};
        u8 *PxSwapped = malloc(NumPx*4);
        if(!PxSwapped) {
            SDL_LogError(0, "Failed to alloc screenshot memory, oh well, carry on.");
        } else {
            pfori(NumPx) {
                u32 SrcPx = ((u32 *)ScreenBuffer)[i];
                u8 b0 = (u8)((SrcPx) >> 24);
                u8 b1 = (u8)((SrcPx) >> 16);
                u8 b2 = (u8)((SrcPx) >> 8);
                u8 b3 = (u8)((SrcPx) >> 0);
                ((u32 *)PxSwapped)[i] = b3 << 24 | b2 << 16 | b1 << 8 | b0;
            }
            if(!stbi_write_png(NameBuffer, ScreenPxW, ScreenPxH, 4, PxSwapped, 0)) {
                SDL_LogError(0, "Failed to save screenshot, oh well, carry on.");
            }
            free(PxSwapped);
        }
    }
}
