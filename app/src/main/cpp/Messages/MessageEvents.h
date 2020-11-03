#pragma once
#include <Urho3D/Core/Object.h>

namespace MessageEvents {
    URHO3D_EVENT(E_NEW_ACHIEVEMENT, NewAchievement)
    {
        URHO3D_PARAM(P_MESSAGE, Message); // string - achievement title
        URHO3D_PARAM(P_IMAGE, Image); // string - Texture to use for achievement
    }

    // Achievement has been unlocked
    URHO3D_EVENT(E_ACHIEVEMENT_UNLOCKED, AchievementUnlocked)
    {
        URHO3D_PARAM(P_MESSAGE, Message); // string - achievement title
    }

    // Register new achievement
    URHO3D_EVENT(E_ADD_ACHIEVEMENT, AddAchievement)
    {
        URHO3D_PARAM(P_EVENT, Event); // string - achievement event
        URHO3D_PARAM(P_MESSAGE, Message); // string - achievement event
        URHO3D_PARAM(P_IMAGE, Image); // string - achievement event
        URHO3D_PARAM(P_THRESHOLD, Threshold); // string - achievement event
        URHO3D_PARAM(P_PARAMETER_NAME, ParameterName); // string - achievement event
        URHO3D_PARAM(P_PARAMETER_VALUE, ParameterValue); // string - achievement event
    }
}