#pragma once
#include <Urho3D/Graphics/CustomGeometry.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Core/WorkQueue.h>

const int SIZE_X = 8;
const int SIZE_Y = 8;
const int SIZE_Z = 8;

using namespace Urho3D;

enum BlockSide {
    TOP,
    BOTTOM,
    LEFT,
    RIGHT,
    FRONT,
    BACK
};

class Chunk : public Object {
    URHO3D_OBJECT(Chunk, Object);
    Chunk(Context* context);
    virtual ~Chunk();

    static void RegisterObject(Context* context);
public:
    int data[SIZE_X][SIZE_Y][SIZE_Z];
    void Init(int seed, Scene* scene, const Vector3& position);
    const Vector3& GetPosition();
    const BoundingBox& GetBoundingBox();
    Node* GetNode() { return node_; }
    friend void GenerateMesh(const WorkItem* item, unsigned threadIndex);
    bool IsPointInsideChunk(Vector3 pointPosition);
private:
    void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData);
    void HandleMeshLoaded(StringHash eventType, VariantMap& eventData);
    void HandleHit(StringHash eventType, VariantMap& eventData);
    void HandleAdd(StringHash eventType, VariantMap& eventData);
    void HandlePlayerEntered(StringHash eventType, VariantMap& eventData);
    void HandlePlayerExited(StringHash eventType, VariantMap& eventData);
    void UpdateGeometry();
    Vector2 GetTextureCoord(BlockSide side, int blockType, Vector2 position);
    IntVector3 GetChunkBlock(Vector3 position);
    bool IsBlockInsideChunk(IntVector3 position);

    CustomGeometry* geometry_;
    Node* node_{nullptr};
    Node* triggerNode_{nullptr};
    bool used_{true};
    int seed_;
    Scene* scene_;
    Vector3 position_;
    SharedPtr<WorkItem> workItem_;
};
