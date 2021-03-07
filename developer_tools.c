
        static u8 _DevPalNeutral[] = {
    255,255,255,
    192,192,192,
    105,106,106,
    7,9,9,
};

void update_developer_tools(void) {
    if(TileMapDevMode) {
        enum { TileMapDevZoom = 3 };
        if(!TileMapDevWindow) {
            int TileMapDevWindowW = BGMapPxW*TileMapDevZoom;
            int TileMapDevWindowH = BGMapPxH*TileMapDevZoom;
            TileMapDevWindow = SDL_CreateWindow("GameEngine pocket TileMapDev", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, TileMapDevWindowW, TileMapDevWindowH, 0);
            if(!TileMapDevWindow) {
                SDL_LogError(0, "Failed to create window: %s", SDL_GetError());
                exit(1);
            }
            
            TileMapDevRenderer = SDL_CreateRenderer(TileMapDevWindow,
                                                    DbgOpenGlDriver,
                                                    SDL_RENDERER_ACCELERATED);
            if(!TileMapDevRenderer) {
                SDL_LogError(0, "Failed to create renderer: %s", SDL_GetError());
                exit(1);
            }
            
            SDL_RendererInfo TileMapDevInfo = {0};
            if(SDL_GetRendererInfo(TileMapDevRenderer, &TileMapDevInfo)) {
                SDL_LogError(0, "Failed to query renderer info: %s", SDL_GetError());
                exit(1);
            }
            SDL_LogInfo(0, "TileMapDev Renderer: %s", TileMapDevInfo.name);
            
            TileMapDevTexture = SDL_CreateTexture(TileMapDevRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 
                                                  TileMapDevWindowW/TileMapDevZoom, TileMapDevWindowH/TileMapDevZoom);
            if(!TileMapDevTexture ) {
                SDL_LogError(0, "Failed to create screen texture: %s", SDL_GetError());
                exit(1);
            }
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
            SDL_DestroyRenderer(TileMapDevRenderer);
            TileMapDevRenderer = 0;
            SDL_DestroyTexture(TileMapDevTexture);
            TileMapDevTexture = 0;
            SDL_DestroyWindow(TileMapDevWindow);
            TileMapDevWindow = 0;
        }
    }
    
    if(WindowMapDevMode) {
        enum { WindowMapDevZoom = 3 };
        if(!WindowMapDevWindow) {
            int WindowMapDevWindowW = BGMapPxW*WindowMapDevZoom;
            int WindowMapDevWindowH = BGMapPxH*WindowMapDevZoom;
            WindowMapDevWindow = SDL_CreateWindow("GameEngine pocket WindowMapDev", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WindowMapDevWindowW, WindowMapDevWindowH, 0);
            if(!WindowMapDevWindow) {
                SDL_LogError(0, "Failed to create window: %s", SDL_GetError());
                exit(1);
            }
            
            WindowMapDevRenderer = SDL_CreateRenderer(WindowMapDevWindow,
                                                      DbgOpenGlDriver,
                                                      SDL_RENDERER_ACCELERATED);
            if(!WindowMapDevRenderer) {
                SDL_LogError(0, "Failed to create renderer: %s", SDL_GetError());
                exit(1);
            }
            
            SDL_RendererInfo WindowMapDevInfo = {0};
            if(SDL_GetRendererInfo(WindowMapDevRenderer, &WindowMapDevInfo)) {
                SDL_LogError(0, "Failed to query renderer info: %s", SDL_GetError());
                exit(1);
            }
            SDL_LogInfo(0, "WindowMapDev Renderer: %s", WindowMapDevInfo.name);
            
            WindowMapDevTexture = SDL_CreateTexture(WindowMapDevRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 
                                                    WindowMapDevWindowW/WindowMapDevZoom, WindowMapDevWindowH/WindowMapDevZoom);
            if(!WindowMapDevTexture ) {
                SDL_LogError(0, "Failed to create screen texture: %s", SDL_GetError());
                exit(1);
            }
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
            SDL_DestroyRenderer(WindowMapDevRenderer);
            WindowMapDevRenderer = 0;
            SDL_DestroyTexture(WindowMapDevTexture);
            WindowMapDevTexture = 0;
            SDL_DestroyWindow(WindowMapDevWindow);
            WindowMapDevWindow = 0;
        }
    }
    
    if(TileSpriteMemDevMode) {
        enum { TileSpriteMemDevZoom = 5 };
        int TileSpriteWPx = TileSpriteW*TileSpriteDim;
        int TileSpriteHPx = TileSpriteH*TileSpriteDim;
        int TileSpriteMemDevWindowW = TileSpriteWPx*TileSpriteMemDevZoom;
        int TileSpriteMemDevWindowH = TileSpriteHPx*TileSpriteMemDevZoom;
        
        const char WindowTitle[] = "GameEngine pocket TileSpriteMemDev";
        if(!TileSpriteMemDevWindow) {
            TileSpriteMemDevWindow = SDL_CreateWindow(WindowTitle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, TileSpriteMemDevWindowW, TileSpriteMemDevWindowH, 0);
            if(!TileSpriteMemDevWindow) {
                SDL_LogError(0, "Failed to create window: %s", SDL_GetError());
                exit(1);
            }
            
            TileSpriteMemDevRenderer = SDL_CreateRenderer(TileSpriteMemDevWindow,
                                                          DbgOpenGlDriver,
                                                          SDL_RENDERER_ACCELERATED);
            if(!TileSpriteMemDevRenderer) {
                SDL_LogError(0, "Failed to create renderer: %s", SDL_GetError());
                exit(1);
            }
            
            SDL_RendererInfo TileSpriteMemDevInfo = {0};
            if(SDL_GetRendererInfo(TileSpriteMemDevRenderer, &TileSpriteMemDevInfo)) {
                SDL_LogError(0, "Failed to query renderer info: %s", SDL_GetError());
                exit(1);
            }
            SDL_LogInfo(0, "TileSpriteMemDev Renderer: %s", TileSpriteMemDevInfo.name);
            
            TileSpriteMemDevTexture = SDL_CreateTexture(TileSpriteMemDevRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 
                                                        TileSpriteMemDevWindowW/TileSpriteMemDevZoom, TileSpriteMemDevWindowH/TileSpriteMemDevZoom);
            if(!TileSpriteMemDevTexture ) {
                SDL_LogError(0, "Failed to create screen texture: %s", SDL_GetError());
                exit(1);
            }
        }
        
        void *Pixels;
        int Pitch;
        if(SDL_LockTexture(TileSpriteMemDevTexture, 0, &Pixels, &Pitch)) {
            SDL_LogError(0, "Failed to lock screen texture: %s", SDL_GetError());
            exit(1);
        }
        
        pfory32(TileSpriteHPx) {
            u8 *Line = ((u8 *)Pixels)+(Pitch*y);
            pforx32(TileSpriteWPx) {
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
                                  x*TileSpriteDim*TileSpriteMemDevZoom, TileSpriteHPx*TileSpriteMemDevZoom)) {
                SDL_LogError(0, "Failed to draw TileSpriteMem dbg line: %s", SDL_GetError());
                exit(1);
            }
        }
        
        for(int y=1; y<TileSpriteH; y++){
            if(SDL_RenderDrawLine(TileSpriteMemDevRenderer, 0,y*TileSpriteDim*TileSpriteMemDevZoom,
                                  TileSpriteWPx*TileSpriteMemDevZoom, y*TileSpriteDim*TileSpriteMemDevZoom)) {
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
}
