#include "Chunk.h"
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/OctreeQuery.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/UI/Text3D.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/Resource/JSONFile.h>
#include <Urho3D/IO/FileSystem.h>
#include "../../Global.h"
#include "VoxelEvents.h"
#include "VoxelWorld.h"
#include "ChunkGenerator.h"

using namespace VoxelEvents;

void SaveToFile(const WorkItem* item, unsigned threadIndex)
{
    Chunk* chunk = reinterpret_cast<Chunk*>(item->aux_);
    JSONFile file(chunk->context_);
    JSONValue& root = file.GetRoot();
    for (int x = 0; x < SIZE_X; ++x) {
        for (int y = 0; y < SIZE_Y; y++) {
            for (int z = 0; z < SIZE_Z; z++) {
                root.Set(String(x) + "_" + String(y) + "_" + String(z), chunk->data[x][y][z]);
            }
        }
    }
    if(!chunk->GetSubsystem<FileSystem>()->DirExists("World")) {
        chunk->GetSubsystem<FileSystem>()->CreateDir("World");
    }
    Vector3 position = chunk->position_;
    file.SaveFile("World/chunk_" + String(position.x_) + "_" + String(position.y_) + "_" + String(position.z_));
}

void GenerateMesh(const WorkItem* item, unsigned threadIndex)
{
    Chunk* chunk = reinterpret_cast<Chunk*>(item->aux_);
    Vector3 position = chunk->position_;

    JSONFile file(chunk->context_);
    JSONValue& root = file.GetRoot();
    String filename = "World/chunk_" + String(position.x_) + "_" + String(position.y_) + "_" + String(position.z_);
    if(chunk->GetSubsystem<FileSystem>()->FileExists(filename)) {
        file.LoadFile(filename);
        for (int x = 0; x < SIZE_X; ++x) {
            for (int y = 0; y < SIZE_Y; y++) {
                for (int z = 0; z < SIZE_Z; z++) {
                    chunk->data[x][y][z] = static_cast<BlockType>(root[String(x) + "_" + String(y) + "_" + String(z)].GetInt());
                }
            }
        }
        URHO3D_LOGINFO("Loaded chunk from file");
    } else {

        auto chunkGenerator = chunk->GetSubsystem<ChunkGenerator>();
        // Terrain
        for (int x = 0; x < SIZE_X; ++x) {
            for (int z = 0; z < SIZE_Z; z++) {
                Vector3 blockPosition = position + Vector3(x, 0, z);
                int surfaceHeight = chunkGenerator->GetTerrainHeight(blockPosition);
                for (int y = 0; y < SIZE_Y; y++) {
                    blockPosition.y_ = position.y_ + y;
                    chunk->data[x][y][z] = chunkGenerator->GetBlockType(blockPosition, surfaceHeight);
                }
            }
        }

        // Caves
        for (int x = 0; x < SIZE_X; ++x) {
            for (int y = 0; y < SIZE_Y; y++) {
                for (int z = 0; z < SIZE_Z; z++) {
                    Vector3 blockPosition = position + Vector3(x, y, z);
                    chunk->data[x][y][z] = chunkGenerator->GetCaveBlockType(blockPosition, chunk->data[x][y][z]);
                }
            }
        }
    }
}

Chunk::Chunk(Context* context):
Object(context)
{
}

Chunk::~Chunk()
{
    RemoveNode();
}

void Chunk::RegisterObject(Context* context)
{
    context->RegisterFactory<Chunk>();
}

void Chunk::Init(Scene* scene, const Vector3& position)
{
    scene_ = scene;
    position_ = position;

    auto cache = GetSubsystem<ResourceCache>();
    node_ = scene_->CreateChild("Chunk " + position_.ToString(), LOCAL);
    node_->SetScale(1.0f);
    node_->SetWorldPosition(position_);

    triggerNode_ = node_->CreateChild("ChunkTrigger");
    auto triggerBody = triggerNode_->CreateComponent<RigidBody>();
    triggerBody->SetTrigger(true);
    triggerBody->SetCollisionLayerAndMask(COLLISION_MASK_CHUNK , COLLISION_MASK_PLAYER);
    auto *triggerShape = triggerNode_->CreateComponent<CollisionShape>();
    triggerNode_->SetScale(VoxelWorld::visibleDistance);
    Vector3 offset = Vector3(SIZE_X / 2, SIZE_Y / 2, SIZE_Z / 2) / triggerNode_->GetScale();
    triggerShape->SetBox(Vector3(SIZE_X, SIZE_Y, SIZE_Z), Vector3(SIZE_X / 2, SIZE_Y / 2, SIZE_Z / 2) - offset);

    SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(Chunk, HandlePostRenderUpdate));
    SubscribeToEvent(E_WORKITEMCOMPLETED, URHO3D_HANDLER(Chunk, HandleWorkItemFinished));

    SubscribeToEvent("#chunk_visible_distance", [&](StringHash eventType, VariantMap& eventData) {
        if (triggerNode_) {
            triggerNode_->SetScale(VoxelWorld::visibleDistance);
        }
    });
}

void Chunk::Generate()
{
    WorkQueue *workQueue = GetSubsystem<WorkQueue>();
    generateWorkItem_ = workQueue->GetFreeItem();
    generateWorkItem_->priority_ = M_MAX_UNSIGNED;
    generateWorkItem_->workFunction_ = GenerateMesh;
    generateWorkItem_->aux_ = this;
    generateWorkItem_->sendEvent_ = true;

    generateWorkItem_->start_ = nullptr;
    generateWorkItem_->end_ = nullptr;
    workQueue->AddWorkItem(generateWorkItem_);
}

void Chunk::UpdateGeometry()
{
    geometry_->Commit();
    geometry_->SetCastShadows(true);

    auto cache = GetSubsystem<ResourceCache>();
    geometry_->SetMaterial(cache->GetResource<Material>("Materials/Voxel.xml"));

    auto *shape = node_->GetComponent<CollisionShape>();
    if (geometry_->GetNumVertices(0) > 0) {
        shape->SetCustomTriangleMesh(geometry_);
    } else {
        shape->SetEnabled(false);
    }
}

void Chunk::GenerateGeometry()
{
    geometry_->SetDynamic(false);
    geometry_->SetNumGeometries(1);
    geometry_->BeginGeometry(0, TRIANGLE_LIST);
    geometry_->SetViewMask(VIEW_MASK_CHUNK);

    auto cache = GetSubsystem<ResourceCache>();
    for (int x = 0; x < SIZE_X; x++) {
        for (int y = 0; y < SIZE_Y; y++) {
            for (int z = 0; z < SIZE_Z; z++) {
                if (data[x][y][z] > 0) {
                    int blockId = data[x][y][z];
                    Vector3 position(x, y, z);
                    if (!BlockHaveNeighbor(BlockSide::TOP, x, y, z)) {
                        // tri1
                        geometry_->DefineVertex(position + Vector3(0.0f, 1.0, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::TOP, static_cast<BlockType>(blockId), Vector2(0.0, 0.0)));
                        geometry_->DefineNormal(Vector3::UP);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));

                        geometry_->DefineVertex(position + Vector3(0.0f, 1.0, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::TOP, static_cast<BlockType>(blockId), Vector2(0.0, 1.0)));
                        geometry_->DefineNormal(Vector3::UP);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 1.0, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::TOP, static_cast<BlockType>(blockId), Vector2(1.0, 0.0)));
                        geometry_->DefineNormal(Vector3::UP);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));

                        // tri2
                        geometry_->DefineVertex(position + Vector3(0.0f, 1.0, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::TOP, static_cast<BlockType>(blockId), Vector2(0.0, 1.0)));
                        geometry_->DefineNormal(Vector3::UP);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 1.0, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::TOP, static_cast<BlockType>(blockId), Vector2(1.0, 1.0)));
                        geometry_->DefineNormal(Vector3::UP);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 1.0, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::TOP, static_cast<BlockType>(blockId), Vector2(1.0, 0.0)));
                        geometry_->DefineNormal(Vector3::UP);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));
                    }
                    if (!BlockHaveNeighbor(BlockSide::BOTTOM, x, y, z)) {
                        // tri1
                        geometry_->DefineVertex(position + Vector3(0.0f, 0.0f, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BOTTOM, static_cast<BlockType>(blockId), Vector2(0.0, 1.0)));
                        geometry_->DefineNormal(Vector3::DOWN);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f, 0.0f, 1.0f));

                        geometry_->DefineVertex(position + Vector3(0.0f, 0.0f, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BOTTOM, static_cast<BlockType>(blockId), Vector2(0.0, 0.0)));
                        geometry_->DefineNormal(Vector3::DOWN);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f, 0.0f, 1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 0.0f, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BOTTOM, static_cast<BlockType>(blockId), Vector2(1.0, 0.0)));
                        geometry_->DefineNormal(Vector3::DOWN);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f, 0.0f, 1.0f));

                        // tri2
                        geometry_->DefineVertex(position + Vector3(1.0, 0.0f, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BOTTOM, static_cast<BlockType>(blockId), Vector2(1.0, 1.0)));
                        geometry_->DefineNormal(Vector3::DOWN);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f, 0.0f, 1.0f));

                        geometry_->DefineVertex(position + Vector3(0.0f, 0.0f, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BOTTOM, static_cast<BlockType>(blockId), Vector2(0.0, 1.0)));
                        geometry_->DefineNormal(Vector3::DOWN);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f, 0.0f, 1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 0.0f, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BOTTOM, static_cast<BlockType>(blockId), Vector2(1.0, 0.0)));
                        geometry_->DefineNormal(Vector3::DOWN);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f, 0.0f, 1.0f));
                    }
                    if (!BlockHaveNeighbor(BlockSide::LEFT, x, y, z)) {
                        // tri1
                        geometry_->DefineVertex(position + Vector3(0.0f, 0.0f, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::LEFT, static_cast<BlockType>(blockId), Vector2(0.0, 1.0)));
                        geometry_->DefineNormal(Vector3::LEFT);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(0.0f, 1.0, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::LEFT, static_cast<BlockType>(blockId), Vector2(0.0, 0.0)));
                        geometry_->DefineNormal(Vector3::LEFT);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(0.0f, 0.0f, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::LEFT, static_cast<BlockType>(blockId), Vector2(1.0, 1.0)));
                        geometry_->DefineNormal(Vector3::LEFT);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

//                        // tri2
                        geometry_->DefineVertex(position + Vector3(0.0f, 1.0, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::LEFT, static_cast<BlockType>(blockId), Vector2(0.0, 0.0)));
                        geometry_->DefineNormal(Vector3::LEFT);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(0.0f, 1.0, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::LEFT, static_cast<BlockType>(blockId), Vector2(1.0, 0.0)));
                        geometry_->DefineNormal(Vector3::LEFT);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(0.0f, 0.0f, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::LEFT, static_cast<BlockType>(blockId), Vector2(1.0, 1.0)));
                        geometry_->DefineNormal(Vector3::LEFT);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));
                    }
                    if (!BlockHaveNeighbor(BlockSide::RIGHT, x, y, z)) {
                        // tri1
                        geometry_->DefineVertex(position + Vector3(1.0, 0.0f, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::RIGHT, static_cast<BlockType>(blockId), Vector2(0.0, 1.0)));
                        geometry_->DefineNormal(Vector3::RIGHT);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 1.0, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::RIGHT, static_cast<BlockType>(blockId), Vector2(0.0, 0.0)));
                        geometry_->DefineNormal(Vector3::RIGHT);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 0.0f, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::RIGHT, static_cast<BlockType>(blockId), Vector2(1.0, 1.0)));
                        geometry_->DefineNormal(Vector3::RIGHT);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));

                        // tri2
                        geometry_->DefineVertex(position + Vector3(1.0, 1.0, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::RIGHT, static_cast<BlockType>(blockId), Vector2(0.0, 0.0)));
                        geometry_->DefineNormal(Vector3::RIGHT);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 1.0, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::RIGHT, static_cast<BlockType>(blockId), Vector2(1.0, 0.0)));
                        geometry_->DefineNormal(Vector3::RIGHT);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 0.0f, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::RIGHT, static_cast<BlockType>(blockId), Vector2(1.0, 1.0)));
                        geometry_->DefineNormal(Vector3::RIGHT);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));
                    }
                    if (!BlockHaveNeighbor(BlockSide::FRONT, x, y, z)) {
                        // tri1
                        geometry_->DefineVertex(position + Vector3(0.0f, 0.0f, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::FRONT, static_cast<BlockType>(blockId), Vector2(0.0, 1.0)));
                        geometry_->DefineNormal(Vector3::FORWARD);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(0.0f, 1.0, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::FRONT, static_cast<BlockType>(blockId), Vector2(0.0, 0.0)));
                        geometry_->DefineNormal(Vector3::FORWARD);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 0.0f, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::FRONT, static_cast<BlockType>(blockId), Vector2(1.0, 1.0)));
                        geometry_->DefineNormal(Vector3::FORWARD);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        // tri2
                        geometry_->DefineVertex(position + Vector3(0.0f, 1.0, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::FRONT, static_cast<BlockType>(blockId), Vector2(0.0, 0.0)));
                        geometry_->DefineNormal(Vector3::FORWARD);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 1.0, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::FRONT, static_cast<BlockType>(blockId), Vector2(1.0, 0.0)));
                        geometry_->DefineNormal(Vector3::FORWARD);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 0.0f, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::FRONT, static_cast<BlockType>(blockId), Vector2(1.0, 1.0)));
                        geometry_->DefineNormal(Vector3::FORWARD);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));
                    }
                    if (!BlockHaveNeighbor(BlockSide::BACK, x, y, z)) {
                        // tri1
                        geometry_->DefineVertex(position + Vector3(1.0, 0.0f, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BACK, static_cast<BlockType>(blockId), Vector2(0.0, 1.0)));
                        geometry_->DefineNormal(Vector3::BACK);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 1.0, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BACK, static_cast<BlockType>(blockId), Vector2(0.0, 0.0)));
                        geometry_->DefineNormal(Vector3::BACK);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(0.0f, 0.0f, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BACK, static_cast<BlockType>(blockId), Vector2(1.0, 1.0)));
                        geometry_->DefineNormal(Vector3::BACK);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        // tri2
                        geometry_->DefineVertex(position + Vector3(1.0, 1.0, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BACK, static_cast<BlockType>(blockId), Vector2(0.0, 0.0)));
                        geometry_->DefineNormal(Vector3::BACK);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(0.0f, 1.0, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BACK, static_cast<BlockType>(blockId), Vector2(1.0, 0.0)));
                        geometry_->DefineNormal(Vector3::BACK);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(0.0f, 0.0f, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BACK, static_cast<BlockType>(blockId), Vector2(1.0, 1.0)));
                        geometry_->DefineNormal(Vector3::BACK);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));
                    }
                }
            }
        }
    }
}

void Chunk::HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
{
    if (!node_) {
        return;
    }
//    scene_->GetComponent<PhysicsWorld>()->DrawDebugGeometry(true);
//    scene_->GetComponent<PhysicsWorld>()->SetDebugRenderer(scene_->GetComponent<DebugRenderer>());
//    node_->GetComponent<StaticModel>()->DrawDebugGeometry(node_->GetScene()->GetComponent<DebugRenderer>(), true);
}

IntVector3 Chunk::GetChunkBlock(Vector3 position)
{
    IntVector3 blockPosition;
    blockPosition.x_ = Floor(position.x_ - node_->GetPosition().x_);
    blockPosition.y_ = Floor(position.y_ - node_->GetPosition().y_);
    blockPosition.z_ = Floor(position.z_ - node_->GetPosition().z_);
    return blockPosition;
}

bool Chunk::IsBlockInsideChunk(IntVector3 position)
{
    if (position.x_ < 0 || position.x_ >= SIZE_X) {
        return false;
    }
    if (position.y_ < 0 || position.y_ >= SIZE_Y) {
        return false;
    }
    if (position.z_ < 0 || position.z_ >= SIZE_Z) {
        return false;
    }

    return true;
}

void Chunk::HandleHit(StringHash eventType, VariantMap& eventData)
{
    Vector3 position = eventData["Position"].GetVector3();
    auto blockPosition = GetChunkBlock(position);
    if (!IsBlockInsideChunk(blockPosition)) {
        auto neighborChunk = GetSubsystem<VoxelWorld>()->GetChunkByPosition(position);
        if (neighborChunk && neighborChunk->GetNode()) {
            neighborChunk->GetNode()->SendEvent("ChunkHit", eventData);
        }
        return;
    }
    if (data[blockPosition.x_][blockPosition.y_][blockPosition.z_]) {
//        int blockId = data[blockPosition.x_][blockPosition.y_][blockPosition.z_];
        data[blockPosition.x_][blockPosition.y_][blockPosition.z_] = BlockType::AIR;
        URHO3D_LOGINFO("Removing block " + blockPosition.ToString());
        GenerateGeometry();
        UpdateGeometry();
    }
    Save();
}

void Chunk::HandleAdd(StringHash eventType, VariantMap& eventData)
{
    Vector3 position = eventData["Position"].GetVector3();
    auto blockPosition = GetChunkBlock(position);
    if (!IsBlockInsideChunk(blockPosition)) {
        auto neighborChunk = GetSubsystem<VoxelWorld>()->GetChunkByPosition(position);
        if (neighborChunk) {
            neighborChunk->GetNode()->SendEvent("ChunkAdd", eventData);
        } else {
            URHO3D_LOGERROR("Neighbor chunk doesn't exist at position " + position.ToString());
        }
        return;
    }
    if (data[blockPosition.x_][blockPosition.y_][blockPosition.z_] == 0) {
        data[blockPosition.x_][blockPosition.y_][blockPosition.z_] = BlockType::DIRT;
        URHO3D_LOGINFOF("Controller %d added block", eventData["ControllerId"].GetInt());
//        URHO3D_LOGINFOF("Chunk add world pos: %s; chunk pos: %d %d %d", pos.ToString().CString(), x, y, z);
        GenerateGeometry();
        UpdateGeometry();
    }

    Save();
}

const Vector3& Chunk::GetPosition()
{
    return position_;
}

void Chunk::HandleWorkItemFinished(StringHash eventType, VariantMap& eventData)
{
    using namespace WorkItemCompleted;
    WorkItem* workItem = reinterpret_cast<WorkItem*>(eventData[P_ITEM].GetPtr());
    if (workItem->aux_ != this) {
        return;
    }
    if (workItem->workFunction_ == GenerateMesh) {
//        URHO3D_LOGINFO("Chunk background preparing finished " + position_.ToString());
        CreateNode();
    } else if (workItem->workFunction_ == SaveToFile) {
        URHO3D_LOGINFO("Background chunk saving done " + position_.ToString());
    }
}

void Chunk::HandlePlayerEntered(StringHash eventType, VariantMap& eventData)
{
    using namespace ChunkEntered;
    VariantMap& data = GetEventDataMap();
    data[P_POSITION] = position_;
    SendEvent(E_CHUNK_ENTERED, data);
//    URHO3D_LOGINFO("Chunk " + position_.ToString() + " player entered");

    using namespace NodeCollisionStart;

    // Get the other colliding body, make sure it is moving (has nonzero mass)
    auto* otherNode = static_cast<Node*>(eventData[P_OTHERNODE].GetPtr());
//    URHO3D_LOGINFO("Name: " + otherNode->GetName() + " ID : " + String(otherNode->GetID()));
    visitors_++;

//    for (int i = 0; i < 6; i++) {
////        if (!visibleNeighbors_[i]) {
//            if (neighbors_[i]) {
//                neighbors_[i]->SetVisible(visibleNeighbors_[i]);
//            }
////        }
//    }
}

void Chunk::HandlePlayerExited(StringHash eventType, VariantMap& eventData)
{
    using namespace ChunkExited;
    VariantMap& data = GetEventDataMap();
    data[P_POSITION] = position_;
    SendEvent(E_CHUNK_EXITED, data);
    visitors_--;
}

Vector2 Chunk::GetTextureCoord(BlockSide side, BlockType blockType, Vector2 position)
{
    int textureCount = static_cast<int>(BlockType::NONE) - 1;
    Vector2 quadSize(1.0f / 6, 1.0f / textureCount);
    float typeOffset = (static_cast<int>(blockType) - 1) * quadSize.y_;
    switch (side) {
        case BlockSide::TOP: {
            return Vector2(quadSize.x_ * 0.0f + quadSize.x_ * position.x_, quadSize.y_ * position.y_ + typeOffset);
        }
        case BlockSide::BOTTOM: {
            return Vector2(quadSize.x_ * 1.0f + quadSize.x_ * position.x_, quadSize.y_ * position.y_ + typeOffset);
        }
        case BlockSide::LEFT: {
            return Vector2(quadSize.x_ * 2.0f + quadSize.x_ * position.x_, quadSize.y_ * position.y_ + typeOffset);
        }
        case BlockSide::RIGHT: {
            return Vector2(quadSize.x_ * 3.0f + quadSize.x_ * position.x_, quadSize.y_ * position.y_ + typeOffset);
        }
        case BlockSide::FRONT: {
            return Vector2(quadSize.x_ * 4.0f + quadSize.x_ * position.x_, quadSize.y_ * position.y_ + typeOffset);
        }
        case BlockSide::BACK: {
            return Vector2(quadSize.x_ * 5.0f + quadSize.x_ * position.x_, quadSize.y_ * position.y_ + typeOffset);
        }
    }
    return position;
}

void Chunk::Save()
{
    WorkQueue *workQueue = GetSubsystem<WorkQueue>();
    auto workItem = workQueue->GetFreeItem();
    workItem->priority_ = M_MAX_UNSIGNED;
    workItem->workFunction_ = SaveToFile;
    workItem->aux_ = this;
    // send E_WORKITEMCOMPLETED event after finishing WorkItem
    workItem->sendEvent_ = true;

    workItem->start_ = nullptr;
    workItem->end_ = nullptr;
    workQueue->AddWorkItem(workItem);
}

void Chunk::CreateNode()
{
    geometry_ = new CustomGeometry(context_);
    node_->AddComponent(geometry_, scene_->GetFreeComponentID(LOCAL), LOCAL);

    auto *body = node_->CreateComponent<RigidBody>(LOCAL);
    body->SetMass(0);
    body->SetCollisionLayerAndMask(COLLISION_MASK_GROUND, COLLISION_MASK_PLAYER | COLLISION_MASK_OBSTACLES);
    node_->CreateComponent<CollisionShape>(LOCAL);

    GenerateGeometry();
    UpdateGeometry();


    SubscribeToEvent(node_, "ChunkHit", URHO3D_HANDLER(Chunk, HandleHit));
    SubscribeToEvent(node_, "ChunkAdd", URHO3D_HANDLER(Chunk, HandleAdd));
    SubscribeToEvent(triggerNode_, E_NODECOLLISIONSTART, URHO3D_HANDLER(Chunk, HandlePlayerEntered));
    SubscribeToEvent(triggerNode_, E_NODECOLLISIONEND, URHO3D_HANDLER(Chunk, HandlePlayerExited));

    auto cache = GetSubsystem<ResourceCache>();
    label_ = node_->CreateChild("Label", LOCAL);
    auto text3D = label_->CreateComponent<Text3D>();
    text3D->SetFont(cache->GetResource<Font>(APPLICATION_FONT), 30);
    text3D->SetColor(Color::GRAY);
    text3D->SetViewMask(VIEW_MASK_GUI);
    text3D->SetAlignment(HA_CENTER, VA_BOTTOM);
    text3D->SetFaceCameraMode(FaceCameraMode::FC_LOOKAT_Y);
    text3D->SetText(position_.ToString());
    text3D->SetFontSize(32);
    label_->SetPosition(Vector3(SIZE_X / 2, SIZE_Y, SIZE_Z / 2));
    scene_->AddChild(node_);

    using namespace ChunkGenerated;
    VariantMap data = GetEventDataMap();
    data[P_POSITION] = position_;
    SendEvent(E_CHUNK_GENERATED);
}

void Chunk::RemoveNode()
{
    if (node_) {
        node_->Remove();
    }
    if (label_) {
        label_->Remove();
    }
    if (triggerNode_) {
        triggerNode_->Remove();
    }

    using namespace ChunkRemoved;
    VariantMap data = GetEventDataMap();
    data[P_POSITION] = position_;
    SendEvent(E_CHUNK_REMOVED);
}

bool Chunk::BlockHaveNeighbor(BlockSide side, int x, int y, int z)
{
    switch(side) {
        case BlockSide::TOP:
            if (y < SIZE_Y - 1) {
                if (data[x][y + 1][z] > 0) {
                    return true;
                }
            }
            return false;
        case BlockSide::BOTTOM:
            if (y > 0) {
                if (data[x][y - 1][z] > 0) {
                    return true;
                }
            }
            return false;
        case BlockSide::LEFT:
            if (x > 0) {
                if (data[x - 1][y][z] > 0) {
                    return true;
                }
            }
            return false;
        case BlockSide::RIGHT:
            if (x < SIZE_X - 1) {
                if (data[x + 1][y][z] > 0) {
                    return true;
                }
            }
            return false;
        case BlockSide::FRONT:
            if (z > 0) {
                if (data[x][y][z - 1] > 0) {
                    return true;
                }
            }
            return false;
        case BlockSide::BACK:
            if (z < SIZE_Z - 1) {
                if (data[x][y][z + 1] > 0) {
                    return true;
                }
            }
            return false;
    }
}

void Chunk::MarkForDeletion(bool value)
{
    shouldDelete_ = value;
}

bool Chunk::IsMarkedForDeletion()
{
    return shouldDelete_;
}

void Chunk::MarkActive(bool value)
{
    isActive_ = value;
}

bool Chunk::IsActive()
{
    return isActive_;
}

void Chunk::SetActive()
{
    if (node_) {
        auto shape = node_->GetComponent<CollisionShape>();
        if (shape) {
            shape->SetEnabled(isActive_);
        }
        auto body = node_->GetComponent<RigidBody>();
        if (body) {
            body->SetEnabled(isActive_);
        }
    }
}