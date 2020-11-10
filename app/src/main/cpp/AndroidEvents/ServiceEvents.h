#pragma once
#include <Urho3D/Core/Object.h>

namespace ServiceEvents {
    URHO3D_EVENT(E_SERVICE_MESSAGE, ServiceMessage)
    {
        URHO3D_PARAM(P_COMMAND, Command); // int
        URHO3D_PARAM(P_STATUS, Status);   // int
        URHO3D_PARAM(P_MESSAGE, Message); // String
    }
}