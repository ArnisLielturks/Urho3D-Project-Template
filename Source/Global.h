#include <Urho3D/Container/Str.h>

// Control bits we define
static const unsigned CTRL_FORWARD = 1 << 0; //1
static const unsigned CTRL_BACK = 1 << 2; //2
static const unsigned CTRL_LEFT = 1 << 3; //4
static const unsigned CTRL_RIGHT = 1 << 4; //8
static const unsigned CTRL_JUMP = 1 << 5; //16
static const unsigned CTRL_ACTION = 1 << 6; //32
static const unsigned CTRL_SPRINT = 1 << 7; //64
static const unsigned CTRL_UP = 1 << 8; //128
static const unsigned CTRL_SCREENSHOT = 1 << 9; //256

static const unsigned COLLISION_MASK_PLAYER = 1 << 0; //1
static const unsigned COLLISION_MASK_CHECKPOINT = 1 << 1; //2
static const unsigned COLLISION_MASK_OBSTACLES = 1 << 2; //4
static const unsigned COLLISION_MASK_GROUND = 1 << 3; //8

static const String APPLICATION_FONT = "Fonts/Muli-Regular.ttf";

static const float GAMMA_MAX_VALUE = 2.0f;

static const String DOCUMENTS_DIR = "ProjectTemplate";

/**
 * Android ad events
 */
static const int ANDROID_AD_INITIALIZED = 10;

static const int ANDROID_AD_LOAD_INTERSTITIAL = 20;
static const int ANDROID_AD_SHOW_INTERSTITIAL = 21;
static const int ANDROID_AD_INTERSTITIAL_LOADED = 22;
static const int ANDROID_AD_SHOW_INTERSTITIAL_OPENED = 23;
static const int ANDROID_AD_INTERSTITIAL_NOT_LOADED = 24;
static const int ANDROID_AD_INTERSTITIAL_CLOSED = 25;

static const int ANDROID_AD_LOAD_REWARDED = 30;
static const int ANDROID_AD_REWARDED_SHOW = 31;
static const int ANDROID_AD_REWARDED_LOADED = 32;
static const int ANDROID_AD_REWARDED_FAILED_TO_LOAD = 33;
static const int ANDROID_AD_REWARDED_OPENED = 34;
static const int ANDROID_AD_REWARDED_CLOSED = 35;
static const int ANDROID_AD_REWARDED_EARNED = 36;
static const int ANDROID_AD_REWARDED_FAILED_TO_SHOW = 37;
