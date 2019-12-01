static b32 TileMapDbgMode;
static SDL_Window *TileMapDbgWindow;
static SDL_Renderer *TileMapDbgRenderer;
static SDL_Texture *TileMapDbgTexture;
static devinput TileMapDevInput;

static b32 WindowMapDbgMode;
static SDL_Window *WindowMapDbgWindow;
static SDL_Renderer *WindowMapDbgRenderer;
static SDL_Texture *WindowMapDbgTexture;
static devinput WindowMapDevInput;

static b32 TileSpriteMemDbgMode;
static SDL_Window *TileSpriteMemDbgWindow;
static SDL_Renderer *TileSpriteMemDbgRenderer;
static SDL_Texture *TileSpriteMemDbgTexture;
static devinput TileSpriteDevInput;

typedef void (*update_function)(gep_state *);
static update_function update;

static gep_state GepState;
static gep_state GepStateSettings;

typedef void (*switch_to_settings_function)(void);
static switch_to_settings_function switch_to_settings;

// Debug Timer stuff
static u64 BeforeLock;
static u64 AfterLock;
static u64 BeforePalApply;
static u64 AfterPalApply;
static u64 BeforeAudio;
static u64 AfterAudio;
static u64 BeforeDraw;
static u64 AfterDraw;
