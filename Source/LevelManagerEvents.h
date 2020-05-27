#pragma once
#include <Urho3D/Core/Object.h>

namespace LevelManagerEvents {
    // Start new level
    URHO3D_EVENT(E_SET_LEVEL, SetLevel)
    {
        URHO3D_PARAM(P_NAME, Name); // string - level object name
        URHO3D_PARAM(P_MESSAGE, Message); // string - pop-up message content, can be empty
    }

    // Level changing started
    URHO3D_EVENT(E_LEVEL_CHANGING_STARTED, LevelChangingStarted)
    {
        URHO3D_PARAM(P_FROM, From); // string
        URHO3D_PARAM(P_TO, To); // string
    }

    // When the new level is actually created, before the fade effect goes away
    URHO3D_EVENT(E_LEVEL_CHANGING_IN_PROGRESS, LevelChangingInProgress)
    {
        URHO3D_PARAM(P_FROM, From); // string
        URHO3D_PARAM(P_TO, To); // string
    }

    // Level changing finished
    URHO3D_EVENT(E_LEVEL_CHANGING_FINISHED, LevelChangingFinished)
    {
        URHO3D_PARAM(P_FROM, From); // string - previous level
        URHO3D_PARAM(P_TO, To); // string - new level
    }

    // Called before the level is destroyed
    URHO3D_EVENT(E_LEVEL_BEFORE_DESTROY, LevelBeforeDestroy)
    {
    }
}