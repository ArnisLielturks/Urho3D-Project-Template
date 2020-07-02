#pragma once
#include <Urho3D/Core/Object.h>
#include <Urho3D/Container/List.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Core/Timer.h>
#include <queue>

#include "Chunk.h"

struct ChunkNode {
    ChunkNode(Vector3 position, int distance): position_(position), distance_(distance) {}
    Vector3 position_;
    int distance_;
};
class VoxelWorld : public Object {
    URHO3D_OBJECT(VoxelWorld, Object);
    VoxelWorld(Context* context);

    static void RegisterObject(Context* context);
    friend void UpdateChunkState(const WorkItem* item, unsigned threadIndex);

    void AddObserver(SharedPtr<Node> observer);
    void RemoveObserver(SharedPtr<Node> observer);
    Chunk* GetChunkByPosition(const Vector3& position);
    void RemoveBlockAtPosition(const Vector3& position);
    static int visibleDistance;
    static int activeDistance;
    VoxelBlock* GetBlockAt(Vector3 position);
    void Init();
    bool IsChunkValid(Chunk* chunk);
    const String GetBlockName(BlockType type);
private:
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleWorkItemFinished(StringHash eventType, VariantMap& eventData);
    void LoadChunk(const Vector3& position);
    void UpdateChunks();
    Vector3 GetNodeToChunkPosition(Node* node);
    bool IsChunkLoaded(const Vector3& position);
    bool IsEqualPositions(Vector3 a, Vector3 b);
    Vector3 GetWorldToChunkPosition(const Vector3& position);
    void CreateChunk(const Vector3& position);
    String GetChunkIdentificator(const Vector3& position);
    void ProcessQueue();
    void AddChunkToQueue(Vector3 position, int distance = 0);

//    void RaycastFromObservers();

//    List<SharedPtr<Chunk>> chunks_;
    List<WeakPtr<Node>> observers_;
    Scene* scene_;
    List<Vector3> removeBlocks_;
    HashMap<String, SharedPtr<Chunk>> chunks_;
    Mutex mutex_;
    SharedPtr<WorkItem> updateWorkItem_;
    bool reloadAllChunks_{false};
    Timer sunlightTimer_;
    std::queue<ChunkNode> chunkBfsQueue_;
    Vector<Vector3> chunksToLoad_;
    Timer updateTimer_;
};
