#ifdef VOXEL_SUPPORT
#pragma once
#include <Urho3D/Core/Object.h>

namespace VoxelEvents {
    URHO3D_EVENT(E_CHUNK_GENERATED, ChunkGenerated) {
        URHO3D_PARAM(P_POSITION, Position); // Vector3 - chunk position
    }

    URHO3D_EVENT(E_CHUNK_REMOVED, ChunkRemoved) {
        URHO3D_PARAM(P_POSITION, Position); // Vector3 - chunk position
    }

    URHO3D_EVENT(E_BLOCK_ADDED, BlockAdded) {
        URHO3D_PARAM(P_POSITION, Position); // Vector3 - block position
    }

    URHO3D_EVENT(E_BLOCK_REMOVED, BlockRemoved) {
        URHO3D_PARAM(P_POSITION, Position); // Vector3 - block position
    }

    URHO3D_EVENT(E_CHUNK_HIT, ChunkHit) {
        URHO3D_PARAM(P_POSITION, Position); // Vector3 - block position
        URHO3D_PARAM(P_DIRECTION, Direction); // Vector3 - block position
        URHO3D_PARAM(P_ORIGIN, Direction); // Vector3 - block position
        URHO3D_PARAM(P_CONTROLLER_ID, ControllerID); // int
        URHO3D_PARAM(P_ACTION_ID, ActionID); // int
    }

    URHO3D_EVENT(E_CHUNK_ADD, ChunkAdd) {
        URHO3D_PARAM(P_POSITION, Position); // Vector3 - block position
        URHO3D_PARAM(P_CONTROLLER_ID, ControllerID); // int
        URHO3D_PARAM(P_ACTION_ID, ActionID); // int
        URHO3D_PARAM(P_ITEM_ID, ItemID); // int
    }

    URHO3D_EVENT(E_CHUNK_RECEIVED, ChunkReceived) {
        URHO3D_PARAM(P_POSITION, Position);
        URHO3D_PARAM(P_DATA, Data);
    }
}
#endif
