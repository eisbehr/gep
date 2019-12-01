static char *PrefPath;
typedef struct {
    i64 Version;
    b32 Fullscreen;
    i64 Volume;
} prefs;

static const char* SettingsKeys[] = {
    "Fullscreen",
    "Volume",
};

typedef enum {
    skey_fullscreen,
    skey_volume,
} settings_key_enum;

#define sdefault_fullscreen 0 
#define sdefault_volume 50 

enum { TempBufLen = 1024 };
static char TempBuffer[TempBufLen];

/*
settings.conf file defintion
first line:
Version <integer>\n
kv-pair:
<Key>: <Value>\n
Keys are case sensitive. Any whitespaces before the colon are part of the key.
The colon plus one single space separate they key from the value. Any further spaces are part of the value. Every line ends in a \n
No empty lines allowed.

Value types: 
boolean - true/false
integer - 64 bit signed integer
*/

b32 settings_write(prefs *Prefs) {
    int PathLen = snprintf(TempBuffer, TempBufLen, "%ssettings.conf", PrefPath);
    if(PathLen >= TempBufLen) {
        SDL_LogError(0, "Settings path did not fit in static buffer of size %i", TempBufLen);
        return 0;
    }
    SDL_RWops *File = SDL_RWFromFile(TempBuffer, "w");
    if(!File) {
        SDL_LogError(0, "Failed to open config file (%s) for writing. Error: %s", TempBuffer, SDL_GetError());
        return 0;
    }
    
    const char VersionLine[] = "Version 1\n";
    if(SDL_RWwrite(File, VersionLine, ArraySize(VersionLine)-1, 1) != 1) {
        SDL_LogError(0, "Failed to write to config file (%s). Error: %s", TempBuffer, SDL_GetError());
        goto err;
    }
    
    int ConfLen = snprintf(TempBuffer, TempBufLen, "Fullscreen: %s\n", Prefs->Fullscreen ? "true" : "false");
    if(ConfLen >= TempBufLen) {
        SDL_LogError(0, "Settings did not fit in static buffer of size %i", TempBufLen);
        goto err;
    }
    if(SDL_RWwrite(File, TempBuffer, ConfLen, 1) != 1) {
        SDL_LogError(0, "Failed to write to config file. Error: %s", SDL_GetError());
        goto err;
    }
    
    ConfLen = snprintf(TempBuffer, TempBufLen, "Volume: %lli\n", Prefs->Volume);
    if(ConfLen >= TempBufLen) {
        SDL_LogError(0, "Settings did not fit in static buffer of size %i", TempBufLen);
        goto err;
    }
    if(SDL_RWwrite(File, TempBuffer, ConfLen, 1) != 1) {
        SDL_LogError(0, "Failed to write to config file. Error: %s", SDL_GetError());
        goto err;
    }
    
    // TODO implement >write to temp file, delete original, rename<, SDL2 alone does not have the facilities for this.
    SDL_RWclose(File);
    return 1;
    
    err:
    SDL_RWclose(File);
    return 0;
}

typedef struct {
    char *Data;
    i64 Size;
    i64 Offset;
} conf_ctx;

b32 settings_is_version(conf_ctx *Ctx, i64 Version) {
    i64 LineLen = snprintf(TempBuffer, TempBufLen, "Version %lli\n", Version);
    if(LineLen >= TempBufLen) {
        SDL_LogError(0, "Exceeded snprintf temp buffer size");
        return 0;
    }
    if(strncmp(TempBuffer, Ctx->Data, LineLen) != 0) return 0;
    Ctx->Offset+=LineLen;
    return 1;
}

void settings_parse_eat_whitespace(conf_ctx *Ctx) {
    while(isspace(Ctx->Data[Ctx->Offset])) {
        Ctx->Offset++;
    }
}

i64 settings_parse_peek(conf_ctx *Ctx, char C) {
    i64 Offset = Ctx->Offset;
    while(Ctx->Data[Offset] != '\n' && Offset < Ctx->Size) {
        if(Ctx->Data[Offset] == C) {
            return Offset;
        }
        Offset++;
    }
    if(C == '\n' && Ctx->Data[Offset] == '\n') return Offset;
    return -1;
}

settings_key_enum settings_parse_key(conf_ctx *Ctx) {
    settings_parse_eat_whitespace(Ctx);
    i64 ColonOff = settings_parse_peek(Ctx, ':');
    if(ColonOff == -1) return -1;
    pfori(ArrayLength(SettingsKeys)) {
        if(strncmp(Ctx->Data+Ctx->Offset, SettingsKeys[i], ColonOff-Ctx->Offset) == 0) {
            Ctx->Offset = ColonOff+2; // 2 for colon and space
            return i;
        }
    }
    return -1;
}

b32 settings_parse_bool(conf_ctx *Ctx, b32 *Err) {
    *Err = 0;
    const char TrueStr[] = "true\n";
    const char FalseStr[] = "false\n";
    if(strncmp(Ctx->Data+Ctx->Offset, TrueStr, ArraySize(TrueStr)-1) == 0) {
        Ctx->Offset+=ArraySize(TrueStr)-1;
        return 1;
    }
    if(strncmp(Ctx->Data+Ctx->Offset, FalseStr, ArraySize(FalseStr)-1) == 0) {
        Ctx->Offset+=ArraySize(FalseStr)-1;
        return 0;
    }
    *Err = 1;
    return -1;
}

i64 settings_parse_i64(conf_ctx *Ctx, b32 *Err) {
    *Err = 0;
    i64 LineEndOff = settings_parse_peek(Ctx, '\n');
    char *EndPtr = 0;
    i64 Val = strtoll(Ctx->Data+Ctx->Offset, &EndPtr, 10);
    if(EndPtr != Ctx->Data+LineEndOff) { 
        *Err = 1;
        return -1;
    }
    Ctx->Offset = (EndPtr-Ctx->Data)+1;
    return Val;
}

b32 settings_read(prefs *Prefs) {
    int PathLen = snprintf(TempBuffer, TempBufLen, "%ssettings.conf", PrefPath);
    if(PathLen >= TempBufLen) {
        SDL_LogError(0, "Settings path did not fit in static buffer of size %i", TempBufLen);
        return 0;
    }
    SDL_RWops *File = SDL_RWFromFile(TempBuffer, "r");
    if(!File) {
        SDL_LogError(0, "Failed to open config file (%s) for writing. Error: %s", TempBuffer, SDL_GetError());
        return 0;
    }
    
    i64 FileSize = SDL_RWsize(File);
    if(FileSize == -1) {
        SDL_LogError(0, "Failed to get config file size. Error: %s", SDL_GetError());
        goto err;
    }
    char *Conf = malloc(FileSize);
    if(!Conf) {
        SDL_LogError(0, "Failed to malloc() %li bytes", FileSize);
        exit(1);
    }
    if(SDL_RWread(File,Conf,FileSize,1) != 1) {
        SDL_LogError(0, "Failed to read config file. Error: %s", SDL_GetError());
        goto err;
    }
    SDL_RWclose(File);
    conf_ctx *Ctx = &(conf_ctx){.Data=Conf, .Size=FileSize};
    
    if(settings_is_version(Ctx, 1)) {
        Prefs->Version = 1;
        settings_key_enum KeyEnum = -1;
        while((KeyEnum = settings_parse_key(Ctx)) != -1) {
            switch(KeyEnum) {
                case skey_fullscreen: {
                    b32 Err = 0;
                    Prefs->Fullscreen = settings_parse_bool(Ctx, &Err);
                    if(Err) Prefs->Fullscreen = sdefault_fullscreen;
                } break;
                case skey_volume: {
                    b32 Err = 0;
                    Prefs->Volume = settings_parse_i64(Ctx, &Err);
                    if(Err) Prefs->Volume = sdefault_volume;
                } break;
            }
        }
    } else {
        SDL_LogError(0, "Unsupported config file version");
        Prefs->Fullscreen = sdefault_fullscreen;
        Prefs->Volume = sdefault_volume;
        goto err;
    }
    
    
    return 1;
    err:
    SDL_RWclose(File);
    return 0;
}

// apply as soon as posible, before window init and all that
void settings_early_apply(prefs *Prefs) {
    g->Audc.MasterVol = (int)Prefs->Volume;
}

// Apply after window and renderer init
void settings_late_apply(prefs *Prefs) {
    // TODO VSYNC COPY copy pasted, clean up
    if(VsyncHz == 60.0) {
        SDL_LogInfo(0, "Trying to toggle windowed fullscreen");
        set_fullscreen(Window, 1, Prefs->Fullscreen);
    } else {
        SDL_LogInfo(0, "Trying to toggle exclusive fullscreen");
        set_fullscreen(Window, 0, Prefs->Fullscreen);
    }
}
