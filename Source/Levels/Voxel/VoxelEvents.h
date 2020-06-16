#pragma once
#include <Urho3D/Core/Object.h>

namespace VoxelEvents {
    URHO3D_EVENT(E_CHUNK_ENTERED, ChunkEntered) {
        URHO3D_PARAM(P_POSITION, Position); // Vector3 chunk position that was entered
    }

    URHO3D_EVENT(E_CHUNK_EXITED, ChunkExited) {
        URHO3D_PARAM(P_POSITION, Position); // Vector3 chunk position that was entered
    }
}