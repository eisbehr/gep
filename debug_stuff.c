
        static u8 _DbgPalNeutral[] = {
    255,255,255,
    192,192,192,
    105,106,106,
    7,9,9,
};

void do_debug_stuff(void) {
    if(TileMapDbgMode) {
        enum { TileMapDbgZoom = 3 };
        if(!TileMapDbgWindow) {
            int TileMapDbgWindowW = BGMapPxW*TileMapDbgZoom;
            int TileMapDbgWindowH = BGMapPxH*TileMapDbgZoom;
            TileMapDbgWindow = SDL_CreateWindow("GameEngine pocket TileMapDbg", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, TileMapDbgWindowW, TileMapDbgWindowH, 0);
            if(!TileMapDbgWindow) {
                SDL_LogError(0, "Failed to create window: %s", SDL_GetError());
                exit(1);
            }
            
            TileMapDbgRenderer = SDL_CreateRenderer(TileMapDbgWindow,
                                                    DbgOpenGlDriver,
                                                    SDL_RENDERER_ACCELERATED);
            if(!TileMapDbgRenderer) {
                SDL_LogError(0, "Failed to create renderer: %s", SDL_GetError());
                exit(1);
            }
            
            SDL_RendererInfo TileMapDbgInfo = {0};
            if(SDL_GetRendererInfo(TileMapDbgRenderer, &TileMapDbgInfo)) {
                SDL_LogError(0, "Failed to query renderer info: %s", SDL_GetError());
                exit(1);
            }
            SDL_LogInfo(0, "TileMapDbg Renderer: %s", TileMapDbgInfo.name);
            
            TileMapDbgTexture = SDL_CreateTexture(TileMapDbgRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 
                                                  TileMapDbgWindowW/TileMapDbgZoom, TileMapDbgWindowH/TileMapDbgZoom);
            if(!TileMapDbgTexture ) {
                SDL_LogError(0, "Failed to create screen texture: %s", SDL_GetError());
                exit(1);
            }
        }
        
        void *Pixels;
        int Pitch;
        if(SDL_LockTexture(TileMapDbgTexture, 0, &Pixels, &Pitch)) {
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
        
        SDL_UnlockTexture(TileMapDbgTexture);
        
        if(SDL_RenderCopy(TileMapDbgRenderer, TileMapDbgTexture, 0, 0)) {
            SDL_LogError(0, "Failed to copy screen texture: %s", SDL_GetError());
            exit(1);
        }
        
        if(SDL_SetRenderDrawColor(TileMapDbgRenderer, 255,255,255,255)) {
            SDL_LogError(0, "Failed to set dev mode render draw color: %s", SDL_GetError());
            exit(1);
        }
        
        SDL_Rect ClipRect = {0, 0, BGMapPxW*TileMapDbgZoom, BGMapPxH*TileMapDbgZoom};
        if(SDL_RenderSetClipRect(TileMapDbgRenderer, &ClipRect)) {
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
            
            ViewRect2.x *= TileMapDbgZoom;
            ViewRect2.y *= TileMapDbgZoom;
            ViewRect2.w *= TileMapDbgZoom;
            ViewRect2.h *= TileMapDbgZoom;
            
            if(SDL_RenderDrawRect(TileMapDbgRenderer, &ViewRect2)) {
                SDL_LogError(0, "Failed to draw view rect: %s", SDL_GetError());
                exit(1);
            }
        }
        
        if(DrawRect3) {
            SDL_Rect ViewRect3 = {
                ViewRect.x, ViewRect.y-BGMapPxH, ScreenPxW, ScreenPxH };
            
            ViewRect3.x *= TileMapDbgZoom;
            ViewRect3.y *= TileMapDbgZoom;
            ViewRect3.w *= TileMapDbgZoom;
            ViewRect3.h *= TileMapDbgZoom;
            
            if(SDL_RenderDrawRect(TileMapDbgRenderer, &ViewRect3)) {
                SDL_LogError(0, "Failed to draw view rect: %s", SDL_GetError());
                exit(1);
            }
        }
        
        if(DrawRect4) {
            SDL_Rect ViewRect4 = {
                -(BGMapPxW-ViewRect.x), ViewRect.y-BGMapPxH, ScreenPxW, ScreenPxH };
            
            ViewRect4.x *= TileMapDbgZoom;
            ViewRect4.y *= TileMapDbgZoom;
            ViewRect4.w *= TileMapDbgZoom;
            ViewRect4.h *= TileMapDbgZoom;
            
            if(SDL_RenderDrawRect(TileMapDbgRenderer, &ViewRect4)) {
                SDL_LogError(0, "Failed to draw view rect: %s", SDL_GetError());
                exit(1);
            }
        }
        
        ViewRect.x *= TileMapDbgZoom;
        ViewRect.y *= TileMapDbgZoom;
        ViewRect.w *= TileMapDbgZoom;
        ViewRect.h *= TileMapDbgZoom;
        if(SDL_RenderDrawRect(TileMapDbgRenderer, &ViewRect)) {
            SDL_LogError(0, "Failed to draw view rect: %s", SDL_GetError());
            exit(1);
        }
        
        if(SDL_RenderSetClipRect(TileMapDbgRenderer, 0)) {
            SDL_LogError(0, "Failed to set dev mode clip rect: %s", SDL_GetError());
            exit(1);
        }
        SDL_RenderPresent(TileMapDbgRenderer);
    } else {
        if(TileMapDbgWindow) {
            SDL_DestroyRenderer(TileMapDbgRenderer);
            TileMapDbgRenderer = 0;
            SDL_DestroyTexture(TileMapDbgTexture);
            TileMapDbgTexture = 0;
            SDL_DestroyWindow(TileMapDbgWindow);
            TileMapDbgWindow = 0;
        }
    }
    
    if(WindowMapDbgMode) {
        enum { WindowMapDbgZoom = 3 };
        if(!WindowMapDbgWindow) {
            int WindowMapDbgWindowW = BGMapPxW*WindowMapDbgZoom;
            int WindowMapDbgWindowH = BGMapPxH*WindowMapDbgZoom;
            WindowMapDbgWindow = SDL_CreateWindow("GameEngine pocket WindowMapDbg", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WindowMapDbgWindowW, WindowMapDbgWindowH, 0);
            if(!WindowMapDbgWindow) {
                SDL_LogError(0, "Failed to create window: %s", SDL_GetError());
                exit(1);
            }
            
            WindowMapDbgRenderer = SDL_CreateRenderer(WindowMapDbgWindow,
                                                      DbgOpenGlDriver,
                                                      SDL_RENDERER_ACCELERATED);
            if(!WindowMapDbgRenderer) {
                SDL_LogError(0, "Failed to create renderer: %s", SDL_GetError());
                exit(1);
            }
            
            SDL_RendererInfo WindowMapDbgInfo = {0};
            if(SDL_GetRendererInfo(WindowMapDbgRenderer, &WindowMapDbgInfo)) {
                SDL_LogError(0, "Failed to query renderer info: %s", SDL_GetError());
                exit(1);
            }
            SDL_LogInfo(0, "WindowMapDbg Renderer: %s", WindowMapDbgInfo.name);
            
            WindowMapDbgTexture = SDL_CreateTexture(WindowMapDbgRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 
                                                    WindowMapDbgWindowW/WindowMapDbgZoom, WindowMapDbgWindowH/WindowMapDbgZoom);
            if(!WindowMapDbgTexture ) {
                SDL_LogError(0, "Failed to create screen texture: %s", SDL_GetError());
                exit(1);
            }
        }
        
        void *Pixels;
        int Pitch;
        if(SDL_LockTexture(WindowMapDbgTexture, 0, &Pixels, &Pitch)) {
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
        
        SDL_UnlockTexture(WindowMapDbgTexture);
        
        if(SDL_RenderCopy(WindowMapDbgRenderer, WindowMapDbgTexture, 0, 0)) {
            SDL_LogError(0, "Failed to copy screen texture: %s", SDL_GetError());
            exit(1);
        }
        
        if(SDL_SetRenderDrawColor(WindowMapDbgRenderer, 255,255,255,255)) {
            SDL_LogError(0, "Failed to set dev mode render draw color: %s", SDL_GetError());
            exit(1);
        }
        SDL_RenderPresent(WindowMapDbgRenderer);
    } else {
        if(WindowMapDbgWindow) {
            SDL_DestroyRenderer(WindowMapDbgRenderer);
            WindowMapDbgRenderer = 0;
            SDL_DestroyTexture(WindowMapDbgTexture);
            WindowMapDbgTexture = 0;
            SDL_DestroyWindow(WindowMapDbgWindow);
            WindowMapDbgWindow = 0;
        }
    }
    
    if(TileSpriteMemDbgMode) {
        enum { TileSpriteMemDbgZoom = 5 };
        int TileSpriteWPx = TileSpriteW*TileSpriteDim;
        int TileSpriteHPx = TileSpriteH*TileSpriteDim;
        int TileSpriteMemDbgWindowW = TileSpriteWPx*TileSpriteMemDbgZoom;
        int TileSpriteMemDbgWindowH = TileSpriteHPx*TileSpriteMemDbgZoom;
        
        const char WindowTitle[] = "GameEngine pocket TileSpriteMemDbg";
        if(!TileSpriteMemDbgWindow) {
            TileSpriteMemDbgWindow = SDL_CreateWindow(WindowTitle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, TileSpriteMemDbgWindowW, TileSpriteMemDbgWindowH, 0);
            if(!TileSpriteMemDbgWindow) {
                SDL_LogError(0, "Failed to create window: %s", SDL_GetError());
                exit(1);
            }
            
            TileSpriteMemDbgRenderer = SDL_CreateRenderer(TileSpriteMemDbgWindow,
                                                          DbgOpenGlDriver,
                                                          SDL_RENDERER_ACCELERATED);
            if(!TileSpriteMemDbgRenderer) {
                SDL_LogError(0, "Failed to create renderer: %s", SDL_GetError());
                exit(1);
            }
            
            SDL_RendererInfo TileSpriteMemDbgInfo = {0};
            if(SDL_GetRendererInfo(TileSpriteMemDbgRenderer, &TileSpriteMemDbgInfo)) {
                SDL_LogError(0, "Failed to query renderer info: %s", SDL_GetError());
                exit(1);
            }
            SDL_LogInfo(0, "TileSpriteMemDbg Renderer: %s", TileSpriteMemDbgInfo.name);
            
            TileSpriteMemDbgTexture = SDL_CreateTexture(TileSpriteMemDbgRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 
                                                        TileSpriteMemDbgWindowW/TileSpriteMemDbgZoom, TileSpriteMemDbgWindowH/TileSpriteMemDbgZoom);
            if(!TileSpriteMemDbgTexture ) {
                SDL_LogError(0, "Failed to create screen texture: %s", SDL_GetError());
                exit(1);
            }
        }
        
        void *Pixels;
        int Pitch;
        if(SDL_LockTexture(TileSpriteMemDbgTexture, 0, &Pixels, &Pitch)) {
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
                Pix[1] = _DbgPalNeutral[c*3+ShadeB]; // Blue
                Pix[2] = _DbgPalNeutral[c*3+ShadeG]; // Green
                Pix[3] = _DbgPalNeutral[c*3+ShadeR]; // Red
            }
        }
        
        SDL_UnlockTexture(TileSpriteMemDbgTexture);
        
        if(SDL_RenderCopy(TileSpriteMemDbgRenderer, TileSpriteMemDbgTexture, 0, 0)) {
            SDL_LogError(0, "Failed to copy screen texture: %s", SDL_GetError());
            exit(1);
        }
        
        for(int x=1; x<TileSpriteW; x++) {
            if(SDL_RenderDrawLine(TileSpriteMemDbgRenderer, x*TileSpriteDim*TileSpriteMemDbgZoom,0,
                                  x*TileSpriteDim*TileSpriteMemDbgZoom, TileSpriteHPx*TileSpriteMemDbgZoom)) {
                SDL_LogError(0, "Failed to draw TileSpriteMem dbg line: %s", SDL_GetError());
                exit(1);
            }
        }
        
        for(int y=1; y<TileSpriteH; y++){
            if(SDL_RenderDrawLine(TileSpriteMemDbgRenderer, 0,y*TileSpriteDim*TileSpriteMemDbgZoom,
                                  TileSpriteWPx*TileSpriteMemDbgZoom, y*TileSpriteDim*TileSpriteMemDbgZoom)) {
                SDL_LogError(0, "Failed to draw TileSpriteMem dbg line: %s", SDL_GetError());
                exit(1);
            }
        }
        
        if(TileSpriteDevInput.ctrl | 1) { 
            int TileX = ((int)(TileSpriteDevInput.mx/TileSpriteMemDbgZoom))/TileSpriteDim;
            int TileY = ((int)(TileSpriteDevInput.my/TileSpriteMemDbgZoom))/TileSpriteDim;
            int TileIndex = TileY*TileSpriteW+TileX;
            char WindowTileTitle[ArrayLength(WindowTitle)+20];
            snprintf(WindowTileTitle, ArrayLength(WindowTileTitle), "%s; Tile # %3i", WindowTitle, TileIndex);
            SDL_SetWindowTitle(TileSpriteMemDbgWindow,
                               WindowTileTitle);
        } else {
            SDL_SetWindowTitle(TileSpriteMemDbgWindow,
                               WindowTitle);
        }
        
        SDL_RenderPresent(TileSpriteMemDbgRenderer);
    } else {
        if(TileSpriteMemDbgWindow) {
            SDL_DestroyRenderer(TileSpriteMemDbgRenderer);
            TileSpriteMemDbgRenderer = 0;
            SDL_DestroyTexture(TileSpriteMemDbgTexture);
            TileSpriteMemDbgTexture = 0;
            SDL_DestroyWindow(TileSpriteMemDbgWindow);
            TileSpriteMemDbgWindow = 0;
        }
    }
}
