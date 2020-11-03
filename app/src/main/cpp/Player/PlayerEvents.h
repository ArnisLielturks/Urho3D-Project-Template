#pragma once
#include <Urho3D/Core/Object.h>

namespace PlayerEvents {
    URHO3D_EVENT(E_SET_PLAYER_CAMERA_TARGET, SetPlayerCameraTarget) {
        URHO3D_PARAM(P_ID, ID); // int - player controller id
        URHO3D_PARAM(P_NODE, Node); // pointer to the target node
        URHO3D_PARAM(P_DISTANCE, Distance); // float - set camera distance to target
    }

    URHO3D_EVENT(E_PLAYER_SCORE_ADD, PlayerScoreAdd) {
        URHO3D_PARAM(P_SCORE, Score); // int - score to add
    }

    // Let the app know that player scores have been updated
    URHO3D_EVENT(E_PLAYER_SCORES_UPDATED, PlayerScoresUpdated)
    {
    }
}