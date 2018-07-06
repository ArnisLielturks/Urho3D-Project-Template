#pragma once

#include <Urho3D/Urho3DAll.h>

/// User defined event
namespace MyEvents
{
    // Start new level
    URHO3D_EVENT(E_SET_LEVEL, SetLevel)
    {
        URHO3D_PARAM(P_NAME, Name);
        URHO3D_PARAM(P_MESSAGE, Message);
    }


    // Open UI Window
    URHO3D_EVENT(E_OPEN_WINDOW, OpenWindow)
    {
        URHO3D_PARAM(P_NAME, Name);
    }
    // Close UI Window
    URHO3D_EVENT(E_CLOSE_WINDOW, CloseWindow)
    {
        URHO3D_PARAM(P_NAME, Name);
    }
    // Close all active UI Windows
    URHO3D_EVENT(E_CLOSE_ALL_WINDOWS, CloseAllWindows)
    {
        URHO3D_PARAM(P_NAME, Name);
    }


    // Save configuration JSON file
    URHO3D_EVENT(E_SAVE_CONFIG, SaveConfig)
    {
    }
    URHO3D_EVENT(E_ADD_CONFIG, AddConfig)
    {
        URHO3D_PARAM(P_NAME, Name);
    }
    URHO3D_EVENT(E_LOAD_CONFIG, LoadConfig)
    {
        URHO3D_PARAM(P_FILEPATH, Filepath);
        URHO3D_PARAM(P_PREFIX, Prefix);
    }

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