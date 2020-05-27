#pragma once
#include <Urho3D/Core/Object.h>

namespace SceneManagerEvents {
    // ACK event to mark loading step as valid
    URHO3D_EVENT(E_ACK_LOADING_STEP, AckLoadingStep)
    {
        URHO3D_PARAM(P_EVENT, Event); // string - event to call to start loading process
    }

    // Loading step progress update
    URHO3D_EVENT(E_LOADING_STEP_PROGRESS, LoadingStepProgress)
    {
        URHO3D_PARAM(P_EVENT, Event); // string - event to call to start loading process
        URHO3D_PARAM(P_PROGRESS, Progress); // float - 0.0 - 1.0 to indicate the loading step progress
    }

    // Load step loading finished event
    URHO3D_EVENT(E_LOADING_STEP_FINISHED, LoadingStepFinished)
    {
        URHO3D_PARAM(P_EVENT, Event); // string - event to call to start loading process
    }

    // Load step loading finished event
    URHO3D_EVENT(E_LOADING_STEP_CRITICAL_FAIL, LoadingStepCriticalFail)
    {
        URHO3D_PARAM(P_EVENT, Event); // string - event to call to start loading process
        URHO3D_PARAM(P_DESCRIPTION, Description); // string - problem description
    }

    // Load step loading finished event
    URHO3D_EVENT(E_LOADING_STEP_TIMED_OUT, LoadingStepTimedOut)
    {
        URHO3D_PARAM(P_EVENT, Event); // string - event to call to start loading process
    }

    // Register new loading step in the loading screen
    URHO3D_EVENT(E_REGISTER_LOADING_STEP, RegisterLoadingStep)
    {
        URHO3D_PARAM(P_EVENT, Event); // string - event to call to start loading process
        URHO3D_PARAM(P_NAME, Name); // string - name of the loading step
        URHO3D_PARAM(P_REMOVE_ON_FINISH, RemoveOnFinish); // bool - automatically remove this loading step when finished
        URHO3D_PARAM(P_DEPENDS_ON, DependsOn); // String array - other loading step events that should be run before this
    }

    // Called when new loading step is about to start
    URHO3D_EVENT(E_LOADING_STATUS_UPDATE, LoadingStatusUpdate)
    {
        URHO3D_PARAM(P_NAME, Name); // string - loading step name
    }
}