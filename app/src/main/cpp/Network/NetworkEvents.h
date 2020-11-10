#pragma once
#include <Urho3D/Core/Object.h>

namespace NetworkEvents
{
    URHO3D_EVENT(E_REMOTE_CLIENT_ID, RemoteClientId)
    {
        URHO3D_PARAM(P_NODE_ID, NodeID);
        URHO3D_PARAM(P_PLAYER_ID, PlayerID);
    }
}
