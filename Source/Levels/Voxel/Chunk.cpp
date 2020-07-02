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
#include <Urho3D/Graphics/Geometry.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/UI/Text3D.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/Resource/JSONFile.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Container/Vector.h>
#include <Urho3D/Core/Profiler.h>
#include <Urho3D/Engine/DebugHud.h>
#include <Urho3D/Audio/AudioDefs.h>
#include "../../Global.h"
#include "VoxelEvents.h"
#include "VoxelWorld.h"
#include "ChunkGenerator.h"
#include "../../Console/ConsoleHandlerEvents.h"
#include "LightManager.h"
#include "TreeGenerator.h"
#include "../../Audio/AudioManagerDefs.h"
#include "../../Audio/AudioEvents.h"

using namespace VoxelEvents;
using namespace ConsoleHandlerEvents;

int Chunk::sunlightLevel = 1;

Chunk::Chunk(Context* context):
Object(context)
{
    for (int x = 0; x < SIZE_X; x++) {
        for (int y = 0; y < SIZE_Y; y++) {
            for (int z = 0; z < SIZE_Z; z++) {
                SetVoxel(x, y, z, BlockType::BT_AIR);
            }
        }
    }
    memset(lightMap_, 0, sizeof(lightMap_));
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

    CreateNode();

    SendEvent(
            E_CONSOLE_COMMAND_ADD,
            ConsoleCommandAdd::P_NAME, "chunk_rerender",
            ConsoleCommandAdd::P_EVENT, "#chunk_rerender",
            ConsoleCommandAdd::P_DESCRIPTION, "Rerender all chunks",
            ConsoleCommandAdd::P_OVERWRITE, true
    );
    SubscribeToEvent("#chunk_rerender", [&](StringHash eventType, VariantMap& eventData) {
        StringVector params = eventData["Parameters"].GetStringVector();
        if (params.Size() != 1) {
            URHO3D_LOGERROR("Thi command doesn't have any arguments!");
            return;
        }
        MarkForGeometryCalculation();
    });
}

void Chunk::Load()
{
    Timer loadTime;
    MutexLock lock(mutex_);

    JSONFile file(context_);
    JSONValue& root = file.GetRoot();
    String filename = "World/chunk_" + String(position_.x_) + "_" + String(position_.y_) + "_" + String(position_.z_) + ".json";
    if(GetSubsystem<FileSystem>() && GetSubsystem<FileSystem>()->FileExists(filename)) {
        file.LoadFile(filename);
        for (int x = 0; x < SIZE_X; ++x) {
            for (int y = 0; y < SIZE_Y; y++) {
                for (int z = 0; z < SIZE_Z; z++) {
                    String key = String(x) + "_" + String(y) + "_" + String(z);
                    if (root.Contains(key)) {
                        SetVoxel(x, y, z, static_cast<BlockType>(root[key].GetInt()));
                    }
                }
            }
        }
    } else {

        auto chunkGenerator = GetSubsystem<ChunkGenerator>();
        // Terrain
        for (int x = 0; x < SIZE_X; ++x) {
            for (int z = 0; z < SIZE_Z; z++) {
                Vector3 blockPosition = position_ + Vector3(x, 0, z);
                int surfaceHeight = chunkGenerator->GetTerrainHeight(blockPosition);
                for (int y = 0; y < SIZE_Y; y++) {
                    blockPosition.y_ = position_.y_ + y;
                    BlockType block = chunkGenerator->GetBlockType(blockPosition, surfaceHeight);
                    SetVoxel(x, y, z, block);
                }
            }
        }

        // Water
        const int SEA_LEVEL = 0;
        for (int x = 0; x < SIZE_X; ++x) {
            for (int z = 0; z < SIZE_Z; z++) {
                Vector3 blockPosition = position_ + Vector3(x, 0, z);
                int surfaceHeight = chunkGenerator->GetTerrainHeight(blockPosition);
                for (int y = 0; y < SIZE_Y; y++) {
                    int height = blockPosition.y_ + y;
                    if (height <= SEA_LEVEL && height > surfaceHeight) {
                        SetVoxel(x, y, z, BT_WATER);
                    }
                }
            }
        }

        // Caves
        for (int x = 0; x < SIZE_X; ++x) {
            for (int y = 0; y < SIZE_Y; y++) {
                for (int z = 0; z < SIZE_Z; z++) {
                    Vector3 blockPosition = position_ + Vector3(x, y, z);
                    BlockType block = chunkGenerator->GetCaveBlockType(blockPosition, data_[x][y][z].type);
                    SetVoxel(x, y, z, block);
                }
            }
        }

        // Trees
        for (int x = 0; x < SIZE_X; ++x) {
            for (int z = 0; z < SIZE_Z; z++) {
                Vector3 blockPosition = position_ + Vector3(x, 0, z);
                int surfaceHeight = chunkGenerator->GetTerrainHeight(blockPosition);

                for (int y = SIZE_Y - 1; y >= 0; y--) {
                    blockPosition.y_ = position_.y_ + y;
                    BlockType type = data_[x][y][z].type;
                    if (surfaceHeight >= blockPosition.y_ && type == BT_DIRT) {
                        if (GetSubsystem<ChunkGenerator>()->HaveTree(blockPosition)) {
//                            GetSubsystem<TreeGenerator>()->AddTreeNode(x, y, z, 0, 0, this);
                            SetVoxel(x, y, z, BT_WOOD);
                            break;
                        }
                    }
                }
            }
        }
    }
    CalculateLight();
//    URHO3D_LOGINFO("Chunk " + String(position_) + " loaded in " + String(loadTime.GetMSec(false)) + "ms");
//    Save();
    loaded_ = true;
}

bool Chunk::Render()
{
    if (!shouldRender_) {
        return false;
    }
    MutexLock lock(mutex_);
    auto cache = GetSubsystem<ResourceCache>();
    Node* part = parts_.At(renderIndex_);
    auto geometry = part->GetComponent<CustomGeometry>();
    geometry->Commit();
    geometry->SetOccludee(true);
    if (renderIndex_ == PART_COUNT) {
        geometry->SetMaterial(cache->GetResource<Material>("Materials/VoxelWater.xml"));
        geometry->SetOccluder(false);
    } else {
        geometry->SetMaterial(cache->GetResource<Material>("Materials/Voxel.xml"));
        geometry->SetOccluder(true);
    }

    auto *shape = part->GetComponent<CollisionShape>();
    if (geometry->GetNumVertices(0) > 0) {
        shape->SetCustomTriangleMesh(geometry);
        shape->SetEnabled(true);
    } else {
        shape->SetEnabled(false);
    }
    renderIndex_++;
    if (renderIndex_ > PART_COUNT) {
        shouldRender_ = false;
    }

//    return geometry->GetNumVertices(0) > 0;
    return true;
}

void Chunk::CalculateGeometry()
{
    if (geometryCalculated_) {
        return;
    }
    renderCounter_++;
    Timer loadTime;
    MutexLock lock(mutex_);

    SetSunlight(sunlightLevel);

    HashMap<int, Vector<ChunkVertex>> allVertices;
    for (int i = 0; i <= PART_COUNT; i++) {
        Node *node = parts_.At(i);
        auto geometry = node->GetComponent<CustomGeometry>();
        geometry->SetDynamic(false);
        geometry->SetNumGeometries(1);
        geometry->BeginGeometry(0, TRIANGLE_LIST);
    }
    for (int x = 0; x < SIZE_X; x++) {
        for (int y = 0; y < SIZE_Y; y++) {
            for (int z = 0; z < SIZE_Z; z++) {
                BlockType type = data_[x][y][z].type;
                if (type == BlockType::BT_AIR) {
                    continue;
                }
                if (!shouldDelete_) {
                    int blockId = data_[x][y][z].type;
                    Vector3 position(x, y, z);
                    int index = GetPartIndex(x, y, z);
                    if (blockId == BT_WATER) {
                        index = PART_COUNT;
                    }
                    Node* part = parts_.At(index);
                    auto geometry = part->GetComponent<CustomGeometry>();
                    if (!BlockHaveNeighbor(BlockSide::TOP, x, y, z)) {
                        // tri1
                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::TOP, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(0.0f, 1.0, 0.0f));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::UP);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::TOP, static_cast<BlockType>(blockId), Vector2(0.0, 0.0)));
                            geometry->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));
                        }

                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::TOP, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(0.0f, 1.0, 1.0));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::UP);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::TOP, static_cast<BlockType>(blockId), Vector2(0.0, 1.0)));
                            geometry->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));
                        }

                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::TOP, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(1.0, 1.0, 0.0f));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::UP);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::TOP, static_cast<BlockType>(blockId), Vector2(1.0, 0.0)));
                            geometry->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));
                        }

                        // tri2
                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::TOP, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(0.0f, 1.0, 1.0));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::UP);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::TOP, static_cast<BlockType>(blockId), Vector2(0.0, 1.0)));
                            geometry->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));
                        }

                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::TOP, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(1.0, 1.0, 1.0));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::UP);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::TOP, static_cast<BlockType>(blockId), Vector2(1.0, 1.0)));
                            geometry->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));
                        }

                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::TOP, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(1.0, 1.0, 0.0f));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::UP);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::TOP, static_cast<BlockType>(blockId), Vector2(1.0, 0.0)));
                            geometry->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));
                        }
                    }
                    if (!BlockHaveNeighbor(BlockSide::BOTTOM, x, y, z)) {
                        // tri1
                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::BOTTOM, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(0.0f, 0.0f, 1.0));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::DOWN);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::BOTTOM, static_cast<BlockType>(blockId), Vector2(0.0, 1.0)));
                            geometry->DefineTangent(Vector4(1.0f, 0.0f, 0.0f, 1.0f));
                        }

                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::BOTTOM, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(0.0f, 0.0f, 0.0f));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::DOWN);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::BOTTOM, static_cast<BlockType>(blockId), Vector2(0.0, 0.0)));
                            geometry->DefineTangent(Vector4(1.0f, 0.0f, 0.0f, 1.0f));
                        }

                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::BOTTOM, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(1.0, 0.0f, 0.0f));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::DOWN);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::BOTTOM, static_cast<BlockType>(blockId), Vector2(1.0, 0.0)));
                            geometry->DefineTangent(Vector4(1.0f, 0.0f, 0.0f, 1.0f));
                        }

                        // tri2
                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::BOTTOM, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(1.0, 0.0f, 1.0));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::DOWN);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::BOTTOM, static_cast<BlockType>(blockId), Vector2(1.0, 1.0)));
                            geometry->DefineTangent(Vector4(1.0f, 0.0f, 0.0f, 1.0f));
                        }

                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::BOTTOM, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(0.0f, 0.0f, 1.0));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::DOWN);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::BOTTOM, static_cast<BlockType>(blockId), Vector2(0.0, 1.0)));
                            geometry->DefineTangent(Vector4(1.0f, 0.0f, 0.0f, 1.0f));
                        }

                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::BOTTOM, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(1.0, 0.0f, 0.0f));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::DOWN);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::BOTTOM, static_cast<BlockType>(blockId), Vector2(1.0, 0.0)));
                            geometry->DefineTangent(Vector4(1.0f, 0.0f, 0.0f, 1.0f));
                        }
                    }
                    if (!BlockHaveNeighbor(BlockSide::LEFT, x, y, z)) {
                        // tri1
                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::LEFT, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(0.0f, 0.0f, 1.0));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::LEFT);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::LEFT, static_cast<BlockType>(blockId), Vector2(0.0, 1.0)));
                            geometry->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));
                        }

                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::LEFT, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(0.0f, 1.0, 1.0));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::LEFT);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::LEFT, static_cast<BlockType>(blockId), Vector2(0.0, 0.0)));
                            geometry->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));
                        }

                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::LEFT, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(0.0f, 0.0f, 0.0f));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::LEFT);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::LEFT, static_cast<BlockType>(blockId), Vector2(1.0, 1.0)));
                            geometry->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));
                        }

//                        // tri2
                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::LEFT, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(0.0f, 1.0, 1.0));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::LEFT);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::LEFT, static_cast<BlockType>(blockId), Vector2(0.0, 0.0)));
                            geometry->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));
                        }

                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::LEFT, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(0.0f, 1.0, 0.0f));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::LEFT);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::LEFT, static_cast<BlockType>(blockId), Vector2(1.0, 0.0)));
                            geometry->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));
                        }

                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::LEFT, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(0.0f, 0.0f, 0.0f));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::LEFT);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::LEFT, static_cast<BlockType>(blockId), Vector2(1.0, 1.0)));
                            geometry->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));
                        }
                    }
                    if (!BlockHaveNeighbor(BlockSide::RIGHT, x, y, z)) {
                        // tri1
                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::RIGHT, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(1.0, 0.0f, 0.0f));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::RIGHT);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::RIGHT, static_cast<BlockType>(blockId), Vector2(0.0, 1.0)));
                            geometry->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));
                        }

                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::RIGHT, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(1.0, 1.0, 0.0f));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::RIGHT);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::RIGHT, static_cast<BlockType>(blockId), Vector2(0.0, 0.0)));
                            geometry->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));
                        }

                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::RIGHT, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(1.0, 0.0f, 1.0));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::RIGHT);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::RIGHT, static_cast<BlockType>(blockId), Vector2(1.0, 1.0)));
                            geometry->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));
                        }

                        // tri2
                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::RIGHT, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(1.0, 1.0, 0.0f));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::RIGHT);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::RIGHT, static_cast<BlockType>(blockId), Vector2(0.0, 0.0)));
                            geometry->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));
                        }

                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::RIGHT, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(1.0, 1.0, 1.0));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::RIGHT);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::RIGHT, static_cast<BlockType>(blockId), Vector2(1.0, 0.0)));
                            geometry->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));
                        }

                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::RIGHT, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(1.0, 0.0f, 1.0));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::RIGHT);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::RIGHT, static_cast<BlockType>(blockId), Vector2(1.0, 1.0)));
                            geometry->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));
                        }
                    }
                    if (!BlockHaveNeighbor(BlockSide::FRONT, x, y, z)) {
                        // tri1
                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::FRONT, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(0.0f, 0.0f, 0.0f));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::FORWARD);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::FRONT, static_cast<BlockType>(blockId), Vector2(0.0, 1.0)));
                            geometry->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));
                        }

                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::FRONT, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(0.0f, 1.0, 0.0f));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::FORWARD);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::FRONT, static_cast<BlockType>(blockId), Vector2(0.0, 0.0)));
                            geometry->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));
                        }

                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::FRONT, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(1.0, 0.0f, 0.0f));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::FORWARD);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::FRONT, static_cast<BlockType>(blockId), Vector2(1.0, 1.0)));
                            geometry->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));
                        }

                        // tri2
                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::FRONT, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(0.0f, 1.0, 0.0f));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::FORWARD);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::FRONT, static_cast<BlockType>(blockId), Vector2(0.0, 0.0)));
                            geometry->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));
                        }

                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::FRONT, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(1.0, 1.0, 0.0f));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::FORWARD);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::FRONT, static_cast<BlockType>(blockId), Vector2(1.0, 0.0)));
                            geometry->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));
                        }

                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::FRONT, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(1.0, 0.0f, 0.0f));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::FORWARD);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::FRONT, static_cast<BlockType>(blockId), Vector2(1.0, 1.0)));
                            geometry->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));
                        }
                    }
                    if (!BlockHaveNeighbor(BlockSide::BACK, x, y, z)) {
                        // tri1
                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::BACK, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(1.0, 0.0f, 1.0));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::BACK);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::BACK, static_cast<BlockType>(blockId), Vector2(0.0, 1.0)));
                            geometry->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));
                        }

                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::BACK, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(1.0, 1.0, 1.0));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::BACK);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::BACK, static_cast<BlockType>(blockId), Vector2(0.0, 0.0)));
                            geometry->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));
                        }

                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::BACK, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(0.0f, 0.0f, 1.0));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::BACK);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::BACK, static_cast<BlockType>(blockId), Vector2(1.0, 1.0)));
                            geometry->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));
                        }

                        // tri2
                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::BACK, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(1.0, 1.0, 1.0));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::BACK);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::BACK, static_cast<BlockType>(blockId), Vector2(0.0, 0.0)));
                            geometry->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));
                        }

                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::BACK, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(0.0f, 1.0, 1.0));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::BACK);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::BACK, static_cast<BlockType>(blockId), Vector2(1.0, 0.0)));
                            geometry->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));
                        }

                        {
                            ChunkVertex vertex;
                            unsigned char light = NeighborLightValue(BlockSide::BACK, x, y, z);
                            Color color;
                            color.r_ = static_cast<int>(light & 0xF) / 15.0f;
                            color.g_ = static_cast<int>((light >> 4) & 0xF) / 15.0f;
                            geometry->DefineVertex(position + Vector3(0.0f, 0.0f, 1.0));
                            geometry->DefineColor(color);
                            geometry->DefineNormal(Vector3::BACK);
                            geometry->DefineTexCoord(GetTextureCoord(BlockSide::BACK, static_cast<BlockType>(blockId), Vector2(1.0, 1.0)));
                            geometry->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));
                        }
                    }
                }
            }
        }
    }
    geometryCalculated_ = true;
    shouldRender_ = true;
    renderIndex_ = 0;
//    URHO3D_LOGINFO("Chunk " + String(position_) + " geometry calculated in " + String(loadTime.GetMSec(false)) + "ms");
}

void Chunk::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    if (IsLoaded() && !notified_) {
        using namespace ChunkGenerated;
        VariantMap& data = GetEventDataMap();
        data[P_POSITION] = position_;
        SendEvent(E_CHUNK_GENERATED, data);
        notified_ = true;
    }
//    scene_->GetComponent<PhysicsWorld>()->DrawDebugGeometry(true);
//    scene_->GetComponent<PhysicsWorld>()->SetDebugRenderer(scene_->GetComponent<DebugRenderer>());
//    node_->GetComponent<StaticModel>()->DrawDebugGeometry(node_->GetScene()->GetComponent<DebugRenderer>(), true);
    if (saveTimer_.GetMSec(false) > 30000) {
        saveTimer_.Reset();
        Save();
    }
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
    using namespace ChunkHit;
    Vector3 position = eventData[P_POSITION].GetVector3();
    Vector3 direction = eventData[P_DIRECTION].GetVector3();
    auto blockPosition = GetChunkBlock(position);
    auto neighborPosition = GetChunkBlock(position + direction * 0.5);
    URHO3D_LOGINFO("HandleHit blockposition: " + blockPosition.ToString() + "; neighbor position: " + neighborPosition.ToString());
    if (!IsBlockInsideChunk(blockPosition)) {
        auto neighborChunk = GetSubsystem<VoxelWorld>()->GetChunkByPosition(position);
        if (neighborChunk && neighborChunk->GetNode()) {
            neighborChunk->GetNode()->SendEvent(E_CHUNK_HIT, eventData);
        }
        return;
    }
    BlockType type = data_[blockPosition.x_][blockPosition.y_][blockPosition.z_].type;
    if (type != BT_AIR) {
        int lightValue = GetTorchlight(blockPosition.x_, blockPosition.y_, blockPosition.z_);
        SetTorchlight(blockPosition.x_, blockPosition.y_, blockPosition.z_, 0);
        GetSubsystem<LightManager>()->AddLightRemovalNode(blockPosition.x_, blockPosition.y_, blockPosition.z_, lightValue, this);
        SetVoxel(blockPosition.x_, blockPosition.y_, blockPosition.z_, BlockType::BT_AIR);
        URHO3D_LOGINFO("Removing block " + blockPosition.ToString() + " Type: " + String(static_cast<int>(type)));
        BlockType neighborType = GetBlockNeighbor(BlockSide::BOTTOM, blockPosition.x_, blockPosition.y_, blockPosition.z_);
        GetSubsystem<LightManager>()->AddLightNode(blockPosition.x_, blockPosition.y_, blockPosition.z_, this);
        if (IsBlockInsideChunk(neighborPosition)) {
//            GetSubsystem<LightManager>()->AddLightNode(neighborPosition.x_, neighborPosition.y_, neighborPosition.z_, this);
            CalculateLight();
        } else {
            BlockSide side = GetNeighborDirection(neighborPosition);
            auto neighbor = GetNeighbor(side);
            if (neighbor) {
                IntVector3 normalizedPosition = GetNeighborBlockPosition(neighborPosition);
                URHO3D_LOGINFO("HandleHit blockposition: " + blockPosition.ToString() + "; normalized neighbor position: " + normalizedPosition.ToString());
//                GetSubsystem<LightManager>()->AddLightNode(normalizedPosition.x_, normalizedPosition.y_, normalizedPosition.z_, neighbor);
                neighbor->CalculateLight();
            }
        }

        MarkForGeometryCalculation();

        VariantMap& data = GetEventDataMap();
        data[BlockRemoved::P_POSITION] = blockPosition;
        SendEvent(E_BLOCK_REMOVED, data);

        using namespace AudioEvents::PlaySound;
        data[P_INDEX] = AudioDefs::PLACE_BLOCK;
        data[P_TYPE] = SOUND_EFFECT;
        SendEvent(AudioEvents::E_PLAY_SOUND, data);
    }
}

void Chunk::HandleAdd(StringHash eventType, VariantMap& eventData)
{
    using namespace ChunkAdd;
    Vector3 position = eventData[P_POSITION].GetVector3();
    auto blockPosition = GetChunkBlock(position);
    if (!IsBlockInsideChunk(blockPosition)) {
        auto neighborChunk = GetSubsystem<VoxelWorld>()->GetChunkByPosition(position);
        if (neighborChunk && neighborChunk->GetNode()) {
            neighborChunk->GetNode()->SendEvent("ChunkAdd", eventData);
        } else {
            URHO3D_LOGERROR("Neighbor chunk doesn't exist at position " + position.ToString());
        }
        return;
    }
    if (data_[blockPosition.x_][blockPosition.y_][blockPosition.z_].type == BlockType::BT_AIR) {
        if (eventData[P_ACTION_ID].GetInt() == CTRL_DETECT) {
            URHO3D_LOGINFO("Chunk selected: " + position_.ToString() + "; block: " + blockPosition.ToString()
            + " Torch light: " + String(GetTorchlight(blockPosition.x_, blockPosition.y_, blockPosition.z_)) +
            "; Sun light: " + String(GetSunlight(blockPosition.x_, blockPosition.y_, blockPosition.z_)));

            Color color;
            // Torchlight
            color.r_ = static_cast<int>(GetSunlight(blockPosition.x_, blockPosition.y_, blockPosition.z_) & 0xF) / 15.0f;
            // Sunlight
            color.g_ = static_cast<int>((GetSunlight(blockPosition.x_, blockPosition.y_, blockPosition.z_) >> 4) & 0xF) / 15.0f;
            URHO3D_LOGINFO("Color : " + color.ToString());
            return;
        }
        BlockType type = static_cast<BlockType>(eventData[P_ITEM_ID].GetInt());
        SetVoxel(blockPosition.x_, blockPosition.y_, blockPosition.z_, type);
        URHO3D_LOGINFO("Adding block " + blockPosition.ToString());
//        URHO3D_LOGINFOF("Controller %d added block", eventData["ControllerId"].GetInt());
//        URHO3D_LOGINFOF("Chunk add world pos: %s; chunk pos: %d %d %d", pos.ToString().CString(), x, y, z);
        int lightValue = 0;
        if (type == BT_TORCH) {
            lightValue = 15;
        }
        SetTorchlight(blockPosition.x_, blockPosition.y_, blockPosition.z_, lightValue);
        GetSubsystem<LightManager>()->AddLightNode(blockPosition.x_, blockPosition.y_, blockPosition.z_, this);
        MarkForGeometryCalculation();

        VariantMap& data = GetEventDataMap();
        data[BlockAdded::P_POSITION] = blockPosition;
        SendEvent(E_BLOCK_ADDED, data);

        using namespace AudioEvents::PlaySound;
        data[P_INDEX] = AudioDefs::PLACE_BLOCK;
        data[P_TYPE] = SOUND_EFFECT;
        SendEvent(AudioEvents::E_PLAY_SOUND, data);
    }
}

const Vector3& Chunk::GetPosition()
{
    return position_;
}

Vector2 Chunk::GetTextureCoord(BlockSide side, BlockType blockType, Vector2 position)
{
    int textureCount = static_cast<int>(BlockType::BT_NONE) - 1;
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
    JSONFile file(context_);
    JSONValue& root = file.GetRoot();
    for (int x = 0; x < SIZE_X; ++x) {
        for (int y = 0; y < SIZE_Y; y++) {
            for (int z = 0; z < SIZE_Z; z++) {
                root.Set(String(x) + "_" + String(y) + "_" + String(z), data_[x][y][z].type);
            }
        }
    }
    if(!GetSubsystem<FileSystem>()->DirExists("World")) {
        GetSubsystem<FileSystem>()->CreateDir("World");
    }
    Vector3 position = position_;
    file.SaveFile("World/chunk_" + String(position.x_) + "_" + String(position.y_) + "_" + String(position.z_) + ".json");
//    URHO3D_LOGINFO("Chunk saved " + chunk->position_.ToString());
}

void Chunk::CreateNode()
{
    auto cache = GetSubsystem<ResourceCache>();
    node_ = scene_->CreateChild("Chunk " + position_.ToString(), LOCAL);
    node_->SetScale(1.0f);
    node_->SetWorldPosition(position_);

    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Chunk, HandleUpdate));

    for (int i = 0; i <= PART_COUNT; i++) {
        SharedPtr<Node> part(node_->CreateChild("Part"));
        part->CreateComponent<CustomGeometry>();

        auto *body = part->CreateComponent<RigidBody>(LOCAL);
        body->SetMass(0);
        body->SetCollisionLayerAndMask(COLLISION_MASK_GROUND, COLLISION_MASK_PLAYER | COLLISION_MASK_OBSTACLES);
        part->CreateComponent<CollisionShape>(LOCAL);

        parts_.Push(part);
    }

    SubscribeToEvent(node_, E_CHUNK_HIT, URHO3D_HANDLER(Chunk, HandleHit));
    SubscribeToEvent(node_, E_CHUNK_ADD, URHO3D_HANDLER(Chunk, HandleAdd));

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

    MarkActive(false);
    SetActive();
}

void Chunk::RemoveNode()
{
    if (node_) {
        node_->Remove();
    }
    if (label_) {
        label_->Remove();
    }
}

unsigned char Chunk::NeighborLightValue(BlockSide side, int x, int y, int z)
{
    bool insideChunk = true;
    int dX = x;
    int dY = y;
    int dZ = z;
    switch (side) {
        case BlockSide::LEFT:
            if (dX - 1 < 0) {
                insideChunk = false;
                dX = SIZE_X - 1;
            } else {
                dX -= 1;
            }
            break;
        case BlockSide::RIGHT:
            if (dX + 1 >= SIZE_X) {
                insideChunk = false;
                dX = 0;
            } else {
                dX += 1;
            }
            break;
        case BlockSide::BOTTOM:
            if (dY - 1 < 0) {
                insideChunk = false;
                dY = SIZE_Y - 1;
            } else {
                dY -= 1;
            }
            break;
        case BlockSide::TOP:
            if (dY + 1 >= SIZE_Y) {
                insideChunk = false;
                dY = 0;
            } else {
                dY += 1;
            }
            break;
        case BlockSide::FRONT:
            if (dZ - 1 < 0) {
                insideChunk = false;
                dZ = SIZE_Z - 1;
            } else {
                dZ -= 1;
            }
            break;
        case BlockSide::BACK:
            if (dZ + 1 >= SIZE_Z) {
                insideChunk = false;
                dZ = 0;
            } else {
                dZ += 1;
            }
            break;
    }
    if (insideChunk) {
        return GetLightValue(dX, dY, dZ);
    } else {
        auto neighbor = GetNeighbor(side);
        if (neighbor) {
            return neighbor->GetLightValue(dX, dY, dZ);
        } else {
            // Fallback to our own block light
            return GetLightValue(x, y, z);
        }
    }
}

BlockType Chunk::GetBlockNeighbor(BlockSide side, int x, int y, int z)
{
    BlockType type = data_[x][y][z].type;
    bool insideChunk = true;
    int dX = x;
    int dY = y;
    int dZ = z;
    switch (side) {
        case BlockSide::LEFT:
            if (dX - 1 < 0) {
                insideChunk = false;
                dX = SIZE_X - 1;
            } else {
                dX -= 1;
            }
            break;
        case BlockSide::RIGHT:
            if (dX + 1 >= SIZE_X) {
                insideChunk = false;
                dX = 0;
            } else {
                dX += 1;
            }
            break;
        case BlockSide::BOTTOM:
            if (dY - 1 < 0) {
                insideChunk = false;
                dY = SIZE_Y - 1;
            } else {
                dY -= 1;
            }
            break;
        case BlockSide::TOP:
            if (dY + 1 >= SIZE_Y) {
                insideChunk = false;
                dY = 0;
            } else {
                dY += 1;
            }
            break;
        case BlockSide::FRONT:
            if (dZ - 1 < 0) {
                insideChunk = false;
                dZ = SIZE_Z - 1;
            } else {
                dZ -= 1;
            }
            break;
        case BlockSide::BACK:
            if (dZ + 1 >= SIZE_Z) {
                insideChunk = false;
                dZ = 0;
            } else {
                dZ += 1;
            }
            break;
    }

    if (insideChunk) {
        BlockType neighborType = data_[dX][dY][dZ].type;
        return neighborType;
    } else {
        auto neighbor = GetNeighbor(side);
        if (neighbor) {
            BlockType neighborType = neighbor->GetBlockValue(dX, dY, dZ);
            return neighborType;
        }
    }

    return BT_AIR;
}

bool Chunk::BlockHaveNeighbor(BlockSide side, int x, int y, int z)
{
    BlockType type = data_[x][y][z].type;
    bool insideChunk = true;
    int dX = x;
    int dY = y;
    int dZ = z;
    switch (side) {
    case BlockSide::LEFT:
        if (dX - 1 < 0) {
            insideChunk = false;
            dX = SIZE_X - 1;
        } else {
            dX -= 1;
        }
        break;
    case BlockSide::RIGHT:
        if (dX + 1 >= SIZE_X) {
            insideChunk = false;
            dX = 0;
        } else {
            dX += 1;
        }
        break;
    case BlockSide::BOTTOM:
        if (dY - 1 < 0) {
            insideChunk = false;
            dY = SIZE_Y - 1;
        } else {
            dY -= 1;
        }
        break;
    case BlockSide::TOP:
        if (dY + 1 >= SIZE_Y) {
            insideChunk = false;
            dY = 0;
        } else {
            dY += 1;
        }
        break;
    case BlockSide::FRONT:
        if (dZ - 1 < 0) {
            insideChunk = false;
            dZ = SIZE_Z - 1;
        } else {
            dZ -= 1;
        }
        break;
    case BlockSide::BACK:
        if (dZ + 1 >= SIZE_Z) {
            insideChunk = false;
            dZ = 0;
        } else {
            dZ += 1;
        }
        break;
    }

    if (insideChunk) {
        BlockType neighborType = data_[dX][dY][dZ].type;
        if (neighborType != type && (neighborType == BT_AIR || neighborType == BT_WATER)) {
            return false;
        }
    } else {
        auto neighbor = GetNeighbor(side);
        if (neighbor) {
            BlockType neighborType = neighbor->GetBlockValue(dX, dY, dZ);
            if (neighborType != type && (neighborType == BT_AIR || neighborType == BT_WATER)) {
                return false;
            }
        }
    }

    return true;
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

Chunk* Chunk::GetNeighbor(BlockSide side)
{
    if (!GetSubsystem<VoxelWorld>()) {
        return nullptr;
    }

    Chunk* neighbor = nullptr;
    switch(side) {
        case BlockSide::TOP:
            neighbor = GetSubsystem<VoxelWorld>()->GetChunkByPosition(position_ + Vector3::UP * SIZE_Y);
            break;
        case BlockSide::BOTTOM:
            neighbor = GetSubsystem<VoxelWorld>()->GetChunkByPosition(position_ + Vector3::DOWN * SIZE_Y);
            break;
        case BlockSide::LEFT:
            neighbor = GetSubsystem<VoxelWorld>()->GetChunkByPosition(position_ + Vector3::LEFT * SIZE_X);
            break;
        case BlockSide::RIGHT:
            neighbor = GetSubsystem<VoxelWorld>()->GetChunkByPosition(position_ + Vector3::RIGHT * SIZE_X);
            break;
        case BlockSide::FRONT:
            neighbor = GetSubsystem<VoxelWorld>()->GetChunkByPosition(position_ + Vector3::BACK * SIZE_Z);
            break;
        case BlockSide::BACK:
            neighbor = GetSubsystem<VoxelWorld>()->GetChunkByPosition(position_ + Vector3::FORWARD * SIZE_Z);
            break;
    }
    return neighbor;
}

BlockType Chunk::GetBlockValue(int x, int y, int z)
{
    return data_[x][y][z].type;
}

VoxelBlock* Chunk::GetBlockAt(IntVector3 position)
{
    if (IsBlockInsideChunk(position)) {
        return &data_[position.x_][position.y_][position.z_];
    }
    return nullptr;
}

int Chunk::GetSunlight(int x, int y, int z)
{
    return (lightMap_[x][y][z] >> 4) & 0xF;
}

void Chunk::SetSunlight(int x, int y, int z, int value)
{
    lightMap_[x][y][z] = (lightMap_[x][y][z] & 0xF) | (value << 4);
    MarkForGeometryCalculation();
}

int Chunk::GetTorchlight(int x, int y, int z)
{
    return lightMap_[x][y][z] & 0xF;
}

void Chunk::SetTorchlight(int x, int y, int z, int value)
{
    lightMap_[x][y][z] = (lightMap_[x][y][z] & 0xF0) | value;
    MarkForGeometryCalculation();
}

unsigned char Chunk::GetLightValue(int x, int y, int z)
{
    return lightMap_[x][y][z];
}

void Chunk::SetSunlight(int value)
{
    for (int x = 0; x < SIZE_X; x++) {
        for (int z = 0; z < SIZE_Z; z++) {
            bool added = false;
            for (int y = SIZE_Y - 1; y >= 0; y--) {
                BlockType type = data_[x][y][z].type;
//                if (added) {
//                    SetSunlight(x, y, z, 0);
//                }
//                if (type != BT_AIR && !added) {
                    SetSunlight(x, y, z, value);
                    added = true;
//                }
            }
        }
    }
}

void Chunk::CalculateLight()
{
    for (int x = 0; x < SIZE_X; x++) {
        for (int y = 0; y < SIZE_Y; y++) {
            for (int z = 0; z < SIZE_Z; z++) {
                if (data_[x][y][z].type == BT_TORCH) {
                    SetTorchlight(x, y, z);
                    GetSubsystem<LightManager>()->AddLightNode(x, y, z, this);
                }
            }
        }
    }
    MarkForGeometryCalculation();
}

void Chunk::SetVoxel(int x, int y, int z, BlockType block)
{
    Chunk::data_[x][y][z].type = block;
    MarkForGeometryCalculation();
}

void Chunk::MarkForGeometryCalculation()
{
    geometryCalculated_ = false;
}

int Chunk::GetPartIndex(int x, int y, int z)
{
    return Floor(x / (SIZE_X / (PART_COUNT - 1)));
}

BlockSide Chunk::GetNeighborDirection(const IntVector3& position)
{
    if (position_.x_ < 0) {
        return BlockSide::LEFT;
    }
    else if (position_.x_ >= SIZE_X) {
        return BlockSide::RIGHT;
    }
    else if (position_.y_ < 0) {
        return BlockSide::BOTTOM;
    }
    else if (position_.y_ >= SIZE_Y) {
        return BlockSide::TOP;
    }
    else if (position_.z_ < 0) {
        return BlockSide::FRONT;
    }
    else {
        return BlockSide::BACK;
    }
}

IntVector3 Chunk::GetNeighborBlockPosition(const IntVector3& position)
{
    IntVector3 normalizedPosition = position;
    if (normalizedPosition.x_ < 0) {
        normalizedPosition.x_ += SIZE_X;
    }
    if (normalizedPosition.x_ >= SIZE_X) {
        normalizedPosition.x_ -= SIZE_X;
    }
    if (normalizedPosition.y_ < 0) {
        normalizedPosition.y_ += SIZE_Y;
    }
    if (normalizedPosition.y_ >= SIZE_Y) {
        normalizedPosition.y_ -= SIZE_Y;
    }
    if (normalizedPosition.z_ < 0) {
        normalizedPosition.z_ += SIZE_Z;
    }
    if (normalizedPosition.z_ >= SIZE_Z) {
        normalizedPosition.z_ -= SIZE_Z;
    }
    return normalizedPosition;
}
