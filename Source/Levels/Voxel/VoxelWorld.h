#pragma once
#include <Urho3D/Core/Object.h>
#include <Urho3D/Container/List.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Core/Timer.h>

#include "Chunk.h"

class VoxelWorld : public Object {
    URHO3D_OBJECT(VoxelWorld, Object);
    VoxelWorld(Context* context);

    static void RegisterObject(Context* context);

    void AddObserver(SharedPtr<Node> observer);
    void RemoveObserver(SharedPtr<Node> observer);
    SharedPtr<Chunk> GetChunkByPosition(const Vector3& position);
    void RemoveBlockAtPosition(const Vector3& position);
private:
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleChunkEntered(StringHash eventType, VariantMap& eventData);
    void HandleChunkExited(StringHash eventType, VariantMap& eventData);
    void LoadChunk(Vector3 position, bool loadImmediately = false);
    void UnloadChunks();
    Vector3 GetNodeToChunkPosition(Node* node);
    bool IsChunkLoaded(Vector3 position);
    bool IsChunkPending(Vector3 position);
    bool IsEqualPositions(Vector3 a, Vector3 b);
    Vector3 GetWorldToChunkPosition(Vector3& position);
    void CreateChunk(const Vector3& position);

    List<SharedPtr<Chunk>> chunks_;
    List<WeakPtr<Node>> observers_;
    List<Vector3> pendingChunks_;
    Timer updateTimer_;
    Timer cleanupTimer_;
    Scene* scene_;
    List<Vector3> removeBlocks_;
};
