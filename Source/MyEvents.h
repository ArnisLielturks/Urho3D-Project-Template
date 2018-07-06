#pragma once

#include <Urho3D/Urho3DAll.h>

/// User defined event
namespace MyEvents
{
    // Start new level
    static const StringHash E_SET_LEVEL = StringHash("SetLevel");

    // Open UI Window
    static const StringHash E_OPEN_WINDOW = StringHash("OpenWindow");
    // Close UI Window
    static const StringHash E_CLOSE_WINDOW = StringHash("CloseWindow");
    // Close all active UI Windows
    static const StringHash E_CLOSE_ALL_WINDOWS = StringHash("CloseAllWindows");


    // Save configuration JSON file
    static const StringHash E_SAVE_CONFIG = StringHash("SaveConfig");
    static const StringHash E_ADD_CONFIG = StringHash("AddConfig");

    // Control bits we define
    static const unsigned CTRL_FORWARD = 1;
    static const unsigned CTRL_BACK = 2;
    static const unsigned CTRL_LEFT = 4;
    static const unsigned CTRL_RIGHT = 8;
    static const unsigned CTRL_JUMP = 16;
    static const unsigned CTRL_ACTION = 32;

    static const unsigned COLLISION_TERRAIN_LEVEL = 1;
    static const unsigned COLLISION_PLAYER_LEVEL = 2;
    static const unsigned COLLISION_STATIC_OBJECTS = 4;
    static const unsigned COLLISION_DYNAMIC_OBJECTS = 8;
    static const unsigned COLLISION_PICKABLES = 16;
    static const unsigned COLLISION_ITEM_PICKER = 32;
}