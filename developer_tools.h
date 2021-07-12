#pragma warning(push)
#pragma warning(disable:4459)
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "libs/stb_image_write.h"
#pragma warning(pop)

static b32 TileMapDevMode;
static SDL_Window *TileMapDevWindow;
static SDL_Renderer *TileMapDevRenderer;
static SDL_Texture *TileMapDevTexture;
static devinput TileMapDevInput;

static b32 WindowMapDevMode;
static SDL_Window *WindowMapDevWindow;
static SDL_Renderer *WindowMapDevRenderer;
static SDL_Texture *WindowMapDevTexture;
static devinput WindowMapDevInput;

static b32 TileSpriteMemDevMode;
static SDL_Window *TileSpriteMemDevWindow;
static SDL_Renderer *TileSpriteMemDevRenderer;
static SDL_Texture *TileSpriteMemDevTexture;
static devinput TileSpriteDevInput;

static b32 TakeScreenshot;
static b32 TakeScreenRecording;

static const char *ScreenCaptureDirectory;