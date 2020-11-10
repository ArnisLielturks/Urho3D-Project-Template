#pragma once
#include <Urho3D/Core/Object.h>

namespace ControllerEvents {
    // Sent when ui joystick option is changed via settings
    URHO3D_EVENT(E_UI_JOYSTICK_TOGGLE, UIJoystickToggle) {
        URHO3D_PARAM(P_ENABLED, Enabled); // bool
    }

    // New controller/joystick added
    URHO3D_EVENT(E_CONTROLLER_ADDED, ControllerAdded)
    {
        URHO3D_PARAM(P_INDEX, Index); // string or int - controller id
    }

    URHO3D_EVENT(E_MAPPED_CONTROL_PRESSED, MappedControlPressed)
    {
        URHO3D_PARAM(P_ACTION, Action); // int
        URHO3D_PARAM(P_CONTROLLER, Controller); // int
    }

    URHO3D_EVENT(E_MAPPED_CONTROL_RELEASED, MappedControlReleased)
    {
        URHO3D_PARAM(P_ACTION, Action); // int
        URHO3D_PARAM(P_CONTROLLER, Controller); // int
    }

    // controller/joystick removed
    URHO3D_EVENT(E_CONTROLLER_REMOVED, ControllerRemoved)
    {
        URHO3D_PARAM(P_INDEX, Index); // string or int - controller id
    }

    // When mapping was finished
    URHO3D_EVENT(E_INPUT_MAPPING_FINISHED, InputMappingFinished)
    {
        URHO3D_PARAM(P_CONTROLLER, Controller); // string - keyboard, mouse, joystick - which controller was used to do the mapping
        URHO3D_PARAM(P_CONTROL_ACTION, ControlAction); // int - action ID
        URHO3D_PARAM(P_ACTION_NAME, ActionName); // string - action name
        URHO3D_PARAM(P_KEY, Key); // int - key ID, relative to P_CONTROLLEr
        URHO3D_PARAM(P_KEY_NAME, KeyName); // string - key name
    }

    // Start mapping key to specific action
    URHO3D_EVENT(E_START_INPUT_MAPPING, StartInputMapping)
    {
        URHO3D_PARAM(P_CONTROL_ACTION, ControlAction); // string or int - action name or ID
    }

    // Start mapping key to specific action
    URHO3D_EVENT(E_STOP_INPUT_MAPPING, StopInputMapping)
    {
        URHO3D_PARAM(P_CONTROL_ACTION, ControlAction); // int - action ID
    }
}