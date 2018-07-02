#pragma once

#include <Urho3D/Urho3DAll.h>

/// User defined event
namespace MyEvents
{
    static const StringHash E_SET_LEVEL = StringHash("SetLevel");
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