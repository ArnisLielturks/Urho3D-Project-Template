#include "Chunk.h"
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/OctreeQuery.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/UI/Text3D.h>
#include <Urho3D/UI/Font.h>
#include "../../Generator/PerlinNoise.h"
#include "../../Global.h"
#include "VoxelEvents.h"
#include "VoxelWorld.h"

using namespace VoxelEvents;

void GenerateMesh(const WorkItem* item, unsigned threadIndex)
{
    Chunk* chunk = reinterpret_cast<Chunk*>(item->aux_);
    PerlinNoise perlin(chunk->seed_);
    PerlinNoise perlin2(chunk->seed_ + 1);
    float octaves = 8;
    float frequency = 15.41f;
    // Terrain
    for (int x = 0; x < SIZE_X; ++x) {
        for (int z = 0; z < SIZE_Z; z++) {
            double dx = 0.001f + (chunk->position_.x_ + x) / frequency;
            double dz = 0.001f + (chunk->position_.z_ + z) / frequency;
            double result = perlin.octaveNoise(dx, dz, octaves) * 0.5 + 0.5;
            result *= perlin2.octaveNoise(dx, dz, octaves) * 0.5 + 0.5;
            int y = result * SIZE_Y;
            for (int i = 0; i < SIZE_Y; i++) {
                chunk->data[x][i][z] = (i == 0 || i <= y) ? 1 : 0;
            }
        }
    }

    octaves = 16;
    frequency = 33.3f;
    // Stone
    for (int x = 0; x < SIZE_X; ++x) {
        for (int y = 0; y < SIZE_Y; y++) {
            for (int z = 0; z < SIZE_Z; z++) {
                double dx = 0.001f + (chunk->position_.x_ + x) / frequency;
                double dy = 0.001f + (chunk->position_.y_ + y) / frequency;
                double dz = 0.001f + (chunk->position_.z_ + z) / frequency;
                double result = perlin.octaveNoise(dx, dy, dz, octaves);
                if (result > 0.5 && chunk->data[x][y][z] > 0) {
                    chunk->data[x][y][z] = 2;
                }
            }
        }
    }

    octaves = 16;
    frequency = 40.33f;
    // Sand
    for (int x = 0; x < SIZE_X; ++x) {
        for (int y = 0; y < SIZE_Y; y++) {
            for (int z = 0; z < SIZE_Z; z++) {
                double dx = 0.001f + (chunk->position_.x_ + x) / frequency;
                double dy = 0.001f + (chunk->position_.y_ + y) / frequency;
                double dz = 0.001f + (chunk->position_.z_ + z) / frequency;
                double result = perlin.octaveNoise(dx, dy, dz, octaves);
                if (result > 0.5 && chunk->data[x][y][z] > 0) {
                    chunk->data[x][y][z] = 3;
                }
            }
        }
    }

    octaves = 16;
    frequency = 53.67f;
    // Caves
    for (int x = 0; x < SIZE_X; ++x) {
        for (int y = 0; y < SIZE_Y; y++) {
            for (int z = 0; z < SIZE_Z; z++) {
                double dx = 0.001f + (chunk->position_.x_ + x) / frequency;
                double dy = 0.001f + (chunk->position_.y_ + y) / frequency;
                double dz = 0.001f + (chunk->position_.z_ + z) / frequency;
                double result = perlin.octaveNoise(dx, dy, dz, octaves);
                if (result > 0.0) {
                    chunk->data[x][y][z] = 0;
                }
            }
        }
    }
}

bool haveNeighborLeft(Chunk* chunk, int x, int y, int z) {
    if (x > 0) {
        if (chunk->data[x - 1][y][z] > 0) {
            return true;
        }
    }
    return false;
}

bool haveNeighborRight(Chunk* chunk, int x, int y, int z) {
    if (x < SIZE_X - 1) {
        if (chunk->data[x + 1][y][z] > 0) {
            return true;
        }
    }
    return false;
}

bool haveNeighborTop(Chunk* chunk, int x, int y, int z) {
    if (y < SIZE_Y - 1) {
        if (chunk->data[x][y + 1][z] > 0) {
            return true;
        }
    }
    return false;
}

bool haveNeighborBottom(Chunk* chunk, int x, int y, int z) {
    if (y > 0) {
        if (chunk->data[x][y - 1][z] > 0) {
            return true;
        }
    }
    return false;
}

bool haveNeighborBack(Chunk* chunk, int x, int y, int z) {
    if (z < SIZE_Z - 1) {
        if (chunk->data[x][y][z + 1] > 0) {
            return true;
        }
    }
    return false;
}

bool haveNeighborFront(Chunk* chunk, int x, int y, int z) {
    if (z > 0) {
        if (chunk->data[x][y][z - 1] > 0) {
            return true;
        }
    }
    return false;
}

Chunk::Chunk(Context* context):
Object(context)
{
}

Chunk::~Chunk()
{
    if (node_) {
        node_->Remove();
    }
}

void Chunk::RegisterObject(Context* context)
{
    context->RegisterFactory<Chunk>();
}

void Chunk::Init(int seed, Scene* scene, const Vector3& position)
{
    seed_ = seed;
    scene_ = scene;
    position_ = position;
    WorkQueue *workQueue = GetSubsystem<WorkQueue>();
    workItem_ = workQueue->GetFreeItem();
    workItem_->priority_ = M_MAX_UNSIGNED;
    workItem_->workFunction_ = GenerateMesh;
    workItem_->aux_ = this;
    // send E_WORKITEMCOMPLETED event after finishing WorkItem
    workItem_->sendEvent_ = true;

    workItem_->start_ = nullptr;
    workItem_->end_ = nullptr;
    workQueue->AddWorkItem(workItem_);

    SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(Chunk, HandlePostRenderUpdate));
    SubscribeToEvent(E_WORKITEMCOMPLETED, URHO3D_HANDLER(Chunk, HandleMeshLoaded));
}

void Chunk::UpdateGeometry()
{
    geometry_->SetDynamic(true);
    geometry_->BeginGeometry(0, TRIANGLE_LIST);
    geometry_->SetNumGeometries(1);
    geometry_->SetViewMask(VIEW_MASK_CHUNK);

    auto cache = GetSubsystem<ResourceCache>();
    for (int x = 0; x < SIZE_X; ++x) {
        for (int y = 0; y < SIZE_Y; ++y) {
            for (int z = 0; z < SIZE_Z; ++z) {
                if (data[x][y][z] > 0) {
                    int blockId = data[x][y][z];
                    Vector3 position(x, y, z);
//                    Node *node_ = scene_->CreateChild("Test");
//                    node_->SetScale(1.0f);
//                    node_->SetPosition(Vector3(x, y, z));
//                    StaticModel* object = node_->CreateComponent<StaticModel>();
//                    object->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
//                    object->SetMaterial(cache->GetResource<Material>("Materials/Box.xml"));
//                    continue;
                    if (!haveNeighborTop(this, x, y, z)) {
                        // tri1
                        geometry_->DefineVertex(position + Vector3(0.0f, 1.0, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::TOP, blockId, Vector2(0.0, 0.0)));
                        geometry_->DefineNormal(Vector3::UP);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));

                        geometry_->DefineVertex(position + Vector3(0.0f, 1.0, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::TOP, blockId, Vector2(0.0, 1.0)));
                        geometry_->DefineNormal(Vector3::UP);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 1.0, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::TOP, blockId, Vector2(1.0, 0.0)));
                        geometry_->DefineNormal(Vector3::UP);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));

                        // tri2
                        geometry_->DefineVertex(position + Vector3(0.0f, 1.0, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::TOP, blockId, Vector2(0.0, 1.0)));
                        geometry_->DefineNormal(Vector3::UP);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 1.0, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::TOP, blockId, Vector2(1.0, 1.0)));
                        geometry_->DefineNormal(Vector3::UP);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 1.0, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::TOP, blockId, Vector2(1.0, 0.0)));
                        geometry_->DefineNormal(Vector3::UP);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));
                    }
                    if (!haveNeighborBottom(this, x, y, z)) {
                        // tri1
                        geometry_->DefineVertex(position + Vector3(0.0f, 0.0f, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BOTTOM, blockId, Vector2(0.0, 1.0)));
                        geometry_->DefineNormal(Vector3::DOWN);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f, 0.0f, 1.0f));

                        geometry_->DefineVertex(position + Vector3(0.0f, 0.0f, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BOTTOM, blockId, Vector2(0.0, 0.0)));
                        geometry_->DefineNormal(Vector3::DOWN);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f, 0.0f, 1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 0.0f, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BOTTOM, blockId, Vector2(1.0, 0.0)));
                        geometry_->DefineNormal(Vector3::DOWN);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f, 0.0f, 1.0f));

                        // tri2
                        geometry_->DefineVertex(position + Vector3(1.0, 0.0f, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BOTTOM, blockId, Vector2(1.0, 1.0)));
                        geometry_->DefineNormal(Vector3::DOWN);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f, 0.0f, 1.0f));

                        geometry_->DefineVertex(position + Vector3(0.0f, 0.0f, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BOTTOM, blockId, Vector2(0.0, 1.0)));
                        geometry_->DefineNormal(Vector3::DOWN);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f, 0.0f, 1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 0.0f, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BOTTOM, blockId, Vector2(1.0, 0.0)));
                        geometry_->DefineNormal(Vector3::DOWN);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f, 0.0f, 1.0f));
                    }
                    if (!haveNeighborLeft(this, x, y, z)) {
                        // tri1
                        geometry_->DefineVertex(position + Vector3(0.0f, 0.0f, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::LEFT, blockId, Vector2(0.0, 1.0)));
                        geometry_->DefineNormal(Vector3::LEFT);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(0.0f, 1.0, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::LEFT, blockId, Vector2(0.0, 0.0)));
                        geometry_->DefineNormal(Vector3::LEFT);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(0.0f, 0.0f, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::LEFT, blockId, Vector2(1.0, 1.0)));
                        geometry_->DefineNormal(Vector3::LEFT);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

//                        // tri2
                        geometry_->DefineVertex(position + Vector3(0.0f, 1.0, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::LEFT, blockId, Vector2(0.0, 0.0)));
                        geometry_->DefineNormal(Vector3::LEFT);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(0.0f, 1.0, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::LEFT, blockId, Vector2(1.0, 0.0)));
                        geometry_->DefineNormal(Vector3::LEFT);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(0.0f, 0.0f, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::LEFT, blockId, Vector2(1.0, 1.0)));
                        geometry_->DefineNormal(Vector3::LEFT);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));
                    }
                    if (!haveNeighborRight(this, x, y, z)) {
                        // tri1
                        geometry_->DefineVertex(position + Vector3(1.0, 0.0f, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::RIGHT, blockId, Vector2(0.0, 1.0)));
                        geometry_->DefineNormal(Vector3::RIGHT);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 1.0, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::RIGHT, blockId, Vector2(0.0, 0.0)));
                        geometry_->DefineNormal(Vector3::RIGHT);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 0.0f, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::RIGHT, blockId, Vector2(1.0, 1.0)));
                        geometry_->DefineNormal(Vector3::RIGHT);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));

                        // tri2
                        geometry_->DefineVertex(position + Vector3(1.0, 1.0, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::RIGHT, blockId, Vector2(0.0, 0.0)));
                        geometry_->DefineNormal(Vector3::RIGHT);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 1.0, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::RIGHT, blockId, Vector2(1.0, 0.0)));
                        geometry_->DefineNormal(Vector3::RIGHT);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 0.0f, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::RIGHT, blockId, Vector2(1.0, 1.0)));
                        geometry_->DefineNormal(Vector3::RIGHT);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));
                    }
                    if (!haveNeighborFront(this, x, y, z)) {
                        // tri1
                        geometry_->DefineVertex(position + Vector3(0.0f, 0.0f, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::FRONT, blockId, Vector2(0.0, 1.0)));
                        geometry_->DefineNormal(Vector3::FORWARD);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(0.0f, 1.0, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::FRONT, blockId, Vector2(0.0, 0.0)));
                        geometry_->DefineNormal(Vector3::FORWARD);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 0.0f, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::FRONT, blockId, Vector2(1.0, 1.0)));
                        geometry_->DefineNormal(Vector3::FORWARD);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        // tri2
                        geometry_->DefineVertex(position + Vector3(0.0f, 1.0, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::FRONT, blockId, Vector2(0.0, 0.0)));
                        geometry_->DefineNormal(Vector3::FORWARD);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 1.0, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::FRONT, blockId, Vector2(1.0, 0.0)));
                        geometry_->DefineNormal(Vector3::FORWARD);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 0.0f, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::FRONT, blockId, Vector2(1.0, 1.0)));
                        geometry_->DefineNormal(Vector3::FORWARD);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));
                    }
                    if (!haveNeighborBack(this, x, y, z)) {
                        // tri1
                        geometry_->DefineVertex(position + Vector3(1.0, 0.0f, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BACK, blockId, Vector2(0.0, 1.0)));
                        geometry_->DefineNormal(Vector3::BACK);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 1.0, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BACK, blockId, Vector2(0.0, 0.0)));
                        geometry_->DefineNormal(Vector3::BACK);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(0.0f, 0.0f, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BACK, blockId, Vector2(1.0, 1.0)));
                        geometry_->DefineNormal(Vector3::BACK);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        // tri2
                        geometry_->DefineVertex(position + Vector3(1.0, 1.0, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BACK, blockId, Vector2(0.0, 0.0)));
                        geometry_->DefineNormal(Vector3::BACK);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(0.0f, 1.0, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BACK, blockId, Vector2(1.0, 0.0)));
                        geometry_->DefineNormal(Vector3::BACK);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(0.0f, 0.0f, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BACK, blockId, Vector2(1.0, 1.0)));
                        geometry_->DefineNormal(Vector3::BACK);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));
                    }
                }
            }
        }
    }
    geometry_->Commit();
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
    if (data[blockPosition.x_][blockPosition.y_][blockPosition.z_] > 0) {
        int blockId = data[blockPosition.x_][blockPosition.y_][blockPosition.z_];
        data[blockPosition.x_][blockPosition.y_][blockPosition.z_] = 0;
        URHO3D_LOGINFOF("Controller %d removed block %d", eventData["ControllerId"].GetInt(), blockId);
//        URHO3D_LOGINFOF("Chunk hit world pos: %s; chunk pos: %d %d %d, ID = %d", pos.ToString().CString(), x, y, z, node_->GetID());
        UpdateGeometry();
        auto *shape = node_->GetComponent<CollisionShape>();
        if (geometry_->GetNumVertices(0) > 0) {
            shape->SetCustomTriangleMesh(geometry_);
        } else {
            shape->SetEnabled(false);
        }
    }
}

void Chunk::HandleAdd(StringHash eventType, VariantMap& eventData)
{
    Vector3 position = eventData["Position"].GetVector3();
    auto blockPosition = GetChunkBlock(position);
    if (!IsBlockInsideChunk(blockPosition)) {
        auto neighborChunk = GetSubsystem<VoxelWorld>()->GetChunkByPosition(position);
        neighborChunk->GetNode()->SendEvent("ChunkAdd", eventData);
        return;
    }
    if (data[blockPosition.x_][blockPosition.y_][blockPosition.z_] == 0) {
        data[blockPosition.x_][blockPosition.y_][blockPosition.z_] = 1;
        URHO3D_LOGINFOF("Controller %d added block", eventData["ControllerId"].GetInt());
//        URHO3D_LOGINFOF("Chunk add world pos: %s; chunk pos: %d %d %d", pos.ToString().CString(), x, y, z);
        UpdateGeometry();
        auto *shape = node_->GetComponent<CollisionShape>();
        shape->SetCustomTriangleMesh(geometry_);
    }
}

const Vector3& Chunk::GetPosition()
{
    return position_;
}

void Chunk::HandleMeshLoaded(StringHash eventType, VariantMap& eventData)
{
    using namespace WorkItemCompleted;
    WorkItem* workItem = reinterpret_cast<WorkItem*>(eventData[P_ITEM].GetPtr());
    if (workItem_->aux_ == this) {
        node_ = scene_->CreateChild("Chunk");
        node_->SetScale(1.0f);
        node_->SetWorldPosition(position_);

        geometry_ = node_->GetOrCreateComponent<CustomGeometry>();
        geometry_->SetCastShadows(true);
        auto cache = GetSubsystem<ResourceCache>();
        geometry_->SetMaterial(cache->GetResource<Material>("Materials/Voxel.xml"));
        UpdateGeometry();

        Model* model = new Model(context_);
        model->SetNumGeometries(1);
        model->SetGeometry(0, 0, geometry_->GetLodGeometry(0, 0));

        auto *body = node_->CreateComponent<RigidBody>();
        body->SetMass(0);
        body->SetCollisionLayerAndMask(COLLISION_MASK_GROUND, COLLISION_MASK_PLAYER | COLLISION_MASK_OBSTACLES);
        auto *shape = node_->CreateComponent<CollisionShape>();
        if (geometry_->GetNumVertices(0) > 0) {
            shape->SetCustomTriangleMesh(geometry_);
        }

        triggerNode_ = node_->CreateChild("ChunkTrigger");
        auto triggerBody = triggerNode_->CreateComponent<RigidBody>();
        triggerBody->SetTrigger(true);
        triggerBody->SetCollisionLayerAndMask(COLLISION_MASK_CHUNK , COLLISION_MASK_PLAYER);
        auto *triggerShape = triggerNode_->CreateComponent<CollisionShape>();
        triggerShape->SetBox(Vector3(SIZE_X + 0.1f, SIZE_Y + 0.1f, SIZE_Z + 0.1f), Vector3(SIZE_X / 2 - 0.5f, SIZE_Y / 2 - 0.5f, SIZE_Z / 2 - 0.5f));

        SubscribeToEvent(node_, "ChunkHit", URHO3D_HANDLER(Chunk, HandleHit));
        SubscribeToEvent(node_, "ChunkAdd", URHO3D_HANDLER(Chunk, HandleAdd));
        SubscribeToEvent(triggerNode_, E_NODECOLLISIONSTART, URHO3D_HANDLER(Chunk, HandlePlayerEntered));
        SubscribeToEvent(triggerNode_, E_NODECOLLISIONEND, URHO3D_HANDLER(Chunk, HandlePlayerExited));

        Node* label = node_->CreateChild("Label", LOCAL);

        auto text3D = label->CreateComponent<Text3D>();
        text3D->SetFont(cache->GetResource<Font>(APPLICATION_FONT), 30);
        text3D->SetColor(Color::GRAY);
        text3D->SetViewMask(VIEW_MASK_GUI);
        text3D->SetAlignment(HA_CENTER, VA_BOTTOM);
        text3D->SetFaceCameraMode(FaceCameraMode::FC_LOOKAT_Y);
        text3D->SetText(position_.ToString());
        label->SetPosition(Vector3(SIZE_X / 2, SIZE_Y / 2, SIZE_Z / 2));
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
}

void Chunk::HandlePlayerExited(StringHash eventType, VariantMap& eventData)
{
    using namespace ChunkExited;
    VariantMap& data = GetEventDataMap();
    data[P_POSITION] = position_;
    SendEvent(E_CHUNK_EXITED, data);
}

Vector2 Chunk::GetTextureCoord(BlockSide side, int blockType, Vector2 position)
{
    int textureCount = 3;
    Vector2 quadSize(1.0f / 6, 1.0f / textureCount);
    float typeOffset = (blockType - 1) * quadSize.y_;
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

bool Chunk::IsPointInsideChunk(Vector3 pointPosition)
{
    BoundingBox box;
    box.min_ = position_;
    box.max_ = position_ + Vector3(SIZE_X, SIZE_Y, SIZE_Z);
    return box.IsInside(pointPosition);
}