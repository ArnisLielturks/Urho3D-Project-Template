#pragma once
#include <Urho3D/Graphics/CustomGeometry.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Core/WorkQueue.h>
#include "VoxelDefs.h"

const int SIZE_X = 8;
const int SIZE_Y = 8;
const int SIZE_Z = 8;

using namespace Urho3D;

class Chunk : public Object {
    URHO3D_OBJECT(Chunk, Object);
    Chunk(Context* context);
    virtual ~Chunk();

    static void RegisterObject(Context* context);
public:
    void Init(Scene* scene, const Vector3& position);
    void Generate();
    const Vector3& GetPosition();
    const BoundingBox& GetBoundingBox();
    Node* GetNode() { return node_; }
    friend void GenerateMesh(const WorkItem* item, unsigned threadIndex);
    friend void SaveToFile(const WorkItem* item, unsigned threadIndex);
    void Save();
    void MarkForDeletion(bool value);
    bool IsMarkedForDeletion();
    void MarkActive(bool value);
    bool IsActive();
    void SetActive();
private:
    void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData);
    void HandleWorkItemFinished(StringHash eventType, VariantMap& eventData);
    void HandleHit(StringHash eventType, VariantMap& eventData);
    void HandleAdd(StringHash eventType, VariantMap& eventData);
    void HandlePlayerEntered(StringHash eventType, VariantMap& eventData);
    void HandlePlayerExited(StringHash eventType, VariantMap& eventData);
    void GenerateGeometry();
    void UpdateGeometry();
    Vector2 GetTextureCoord(BlockSide side, BlockType blockType, Vector2 position);
    IntVector3 GetChunkBlock(Vector3 position);
    bool IsBlockInsideChunk(IntVector3 position);
    void CreateNode();
    void RemoveNode();
    bool BlockHaveNeighbor(BlockSide side, int x, int y, int z);

    SharedPtr<CustomGeometry> geometry_;
    SharedPtr<Node> node_;
    SharedPtr<Node> triggerNode_;
    SharedPtr<Node> label_;
    Scene* scene_;
    Vector3 position_;
    SharedPtr<WorkItem> generateWorkItem_;
    int visitors_{0};
    BlockType data[SIZE_X][SIZE_Y][SIZE_Z];
    bool shouldDelete_;
    bool isActive_;
};
