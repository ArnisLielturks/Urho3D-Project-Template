#include <Urho3D/Urho3DAll.h>

// Control bits we define
static const unsigned CTRL_FORWARD = 1 << 0;
static const unsigned CTRL_BACK = 1 << 2;
static const unsigned CTRL_LEFT = 1 << 3;
static const unsigned CTRL_RIGHT = 1 << 4;
static const unsigned CTRL_JUMP = 1 << 5;
static const unsigned CTRL_ACTION = 1 << 6;
static const unsigned CTRL_SPRINT = 1 << 7;
static const unsigned CTRL_UP = 1 << 8;

static String APPLICATION_FONT = "Fonts/Muli-Regular.ttf";