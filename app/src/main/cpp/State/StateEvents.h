#pragma once
#include <Urho3D/Core/Object.h>

namespace StateEvents {
    URHO3D_EVENT(E_SET_STATE_PARAMETER, SetStateParameter) {
        URHO3D_PARAM(P_NAME, Name); // string - parameter name
        URHO3D_PARAM(P_VALUE, Value); // variant - parameter value
        URHO3D_PARAM(P_SAVE, Save); // bool - should we save state when this parameter is set
    }

    URHO3D_EVENT(E_INCREMENT_STATE_PARAMETER, IncrementStateParameter) {
        URHO3D_PARAM(P_NAME, Name); // string - parameter name
        URHO3D_PARAM(P_AMOUNT, Amount); // int - by how much we must increment the value
        URHO3D_PARAM(P_SAVE, Save); // bool - should we save state when this parameter is set
    }
}