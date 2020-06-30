#pragma once
#include <Urho3D/Graphics/CustomGeometry.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Core/WorkQueue.h>
#include <Urho3D/Graphics/VertexBuffer.h>
#include <Urho3D/Graphics/IndexBuffer.h>
#include <Urho3D/Graphics/Model.h>
#include "VoxelDefs.h"
#include <queue>

const int SIZE_X = 16;
const int SIZE_Y = 16;
const int SIZE_Z = 16;

using namespace Urho3D;

class Chunk;

struct ChunkVertex {
    Vector3 position;
    Vector3 normal;
    Vector2 uvCoords;
    Vector4 tangent;
    unsigned char light;
};

struct LightRemovalNode {
    LightRemovalNode(short x, short y, short z, short val, Chunk* ch) : x_(x), y_(y), z_(z), value_(val), chunk_(ch) {}
    short x_;
    short y_;
    short z_;
    short value_;
    Chunk* chunk_; //pointer to the chunk that owns it!
};

struct LightNode {
    LightNode(short x, short y, short z, Chunk* ch) : x_(x), y_(y), z_(z), chunk_(ch) {}
    short x_;
    short y_;
    short z_;
    Chunk* chunk_; //pointer to the chunk that owns it!
};

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
    friend void GenerateVertices(const WorkItem* item, unsigned threadIndex);
    friend void GenerateMesh(const WorkItem* item, unsigned threadIndex);
    friend void SaveToFile(const WorkItem* item, unsigned threadIndex);
    void Save();
    void MarkForDeletion(bool value);
    bool IsMarkedForDeletion();
    void MarkActive(bool value);
    bool IsActive();
    void SetActive();
    bool IsVisible();
    static bool visibilityUpdate_;
    BlockType GetBlockValue(int x, int y, int z);
    void Render(bool neighborLoaded = false);
    void RenderNeighbors();
    VoxelBlock* GetBlockAt(IntVector3 position);
    int GetSunlight(int x, int y, int z);
    void SetSunlight(int x, int y, int z, int value);
    int GetTorchlight(int x, int y, int z);
    void SetTorchlight(int x, int y, int z, int value);
    void AddLightNode(int x, int y, int z);
    void AddLightRemovalNode(int x, int y, int z, int level);
    unsigned char GetLightValue(int x, int y, int z);
    void SetSunlight(int value);
private:
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
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
    VoxelBlock* GetNeighborBlockByLocalPosition(IntVector3 position);
    void CreateNode();
    void RemoveNode();
    bool BlockHaveNeighbor(BlockSide side, int x, int y, int z);
    unsigned char NeighborLightValue(BlockSide side, int x, int y, int z);
    Chunk* GetNeighbor(BlockSide side);
    void ProcessQueues();

    SharedPtr<CustomGeometry> geometry_;
    SharedPtr<Node> node_;
    SharedPtr<Node> triggerNode_;
    SharedPtr<Node> label_;
    Scene* scene_;
    Vector3 position_;
    SharedPtr<WorkItem> generateWorkItem_;
    SharedPtr<WorkItem> generateGeometryWorkItem_;
    SharedPtr<WorkItem> saveWorkItem_;
    int visitors_{0};
    VoxelBlock data_[SIZE_X][SIZE_Y][SIZE_Z];
    unsigned char lightMap_[SIZE_X][SIZE_Y][SIZE_Z];
    bool shouldDelete_;
    bool isActive_;
    bool visible_;
    int loadedNeighbors_{0};
    Vector<ChunkVertex> vertices_;
    Vector<short> indices_;
    Vector<IntVector3> neighborLights_;
    std::queue<LightNode> lightBfsQueue_;
    std::queue<LightRemovalNode> lightRemovalBfsQueue_;
    Mutex mutex_;
};
