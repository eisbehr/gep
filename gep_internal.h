#include "developer_tools.h"

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
static u64 BeforeGameUpdate;
static u64 AfterGameUpdate;
