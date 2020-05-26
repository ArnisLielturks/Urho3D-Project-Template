#pragma once
#include <Urho3D/Core/Object.h>

namespace ControllerEvents {
    // Sent when ui joystick option is changed via settings
    URHO3D_EVENT(E_UI_JOYSTICK_TOGGLE, UIJoystickToggle) {
        URHO3D_PARAM(P_ENABLED, Enabled); // bool
    }
}