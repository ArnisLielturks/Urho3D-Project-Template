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
#include "../../Global.h"
#include "VoxelEvents.h"
#include "VoxelWorld.h"
#include "ChunkGenerator.h"

using namespace VoxelEvents;

bool Chunk::visibilityUpdate_ = true;

void SaveToFile(const WorkItem* item, unsigned threadIndex)
{
    Chunk* chunk = reinterpret_cast<Chunk*>(item->aux_);
    JSONFile file(chunk->context_);
    JSONValue& root = file.GetRoot();
    for (int x = 0; x < SIZE_X; ++x) {
        for (int y = 0; y < SIZE_Y; y++) {
            for (int z = 0; z < SIZE_Z; z++) {
                root.Set(String(x) + "_" + String(y) + "_" + String(z), chunk->data_[x][y][z].type);
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
    MutexLock lock(chunk->mutex_);
    Vector3 position = chunk->position_;

    JSONFile file(chunk->context_);
    JSONValue& root = file.GetRoot();
    String filename = "World/chunk_" + String(position.x_) + "_" + String(position.y_) + "_" + String(position.z_);
    if(chunk->GetSubsystem<FileSystem>()->FileExists(filename)) {
        file.LoadFile(filename);
        for (int x = 0; x < SIZE_X; ++x) {
            for (int y = 0; y < SIZE_Y; y++) {
                for (int z = 0; z < SIZE_Z; z++) {
                    String key = String(x) + "_" + String(y) + "_" + String(z);
                    if (root.Contains(key)) {
                        chunk->data_[x][y][z].type = static_cast<BlockType>(root[key].GetInt());
                        if (chunk->data_[x][y][z].type == BlockType::BT_TORCH) {
                            chunk->SetTorchlight(x, y, z, 15);
                            chunk->AddLightNode(x, y, z);
                        }
                    }
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
                    chunk->data_[x][y][z].type = chunkGenerator->GetBlockType(blockPosition, surfaceHeight);
                }
            }
        }

        // Caves
        for (int x = 0; x < SIZE_X; ++x) {
            for (int y = 0; y < SIZE_Y; y++) {
                for (int z = 0; z < SIZE_Z; z++) {
                    Vector3 blockPosition = position + Vector3(x, y, z);
                    chunk->data_[x][y][z].type = chunkGenerator->GetCaveBlockType(blockPosition, chunk->data_[x][y][z].type);
                }
            }
        }
    }
}

void GenerateVertices(const WorkItem* item, unsigned threadIndex)
{
    Chunk* chunk = reinterpret_cast<Chunk*>(item->aux_);
    if (!chunk || chunk->shouldDelete_) {
        return;
    }
    MutexLock lock(chunk->mutex_);
    chunk->SetSunlight(4);
    chunk->ProcessQueues();
    chunk->vertices_.Clear();
    for (int x = 0; x < SIZE_X; x++) {
        for (int y = 0; y < SIZE_Y; y++) {
            for (int z = 0; z < SIZE_Z; z++) {
                if (chunk->data_[x][y][z].type != BlockType::BT_AIR && !chunk->shouldDelete_) {
                    int blockId = chunk->data_[x][y][z].type;
                    Vector3 position(x, y, z);
                    if (!chunk->BlockHaveNeighbor(BlockSide::TOP, x, y, z)) {
                        // tri1
                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::TOP, x, y, z);
                            vertex.position = position + Vector3(0.0f, 1.0, 0.0f);
                            vertex.normal = Vector3::UP;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::TOP, static_cast<BlockType>(blockId), Vector2(0.0, 0.0));
                            vertex.tangent = Vector4(0.0f, 0.0f, -1.0f, -1.0f);
                            chunk->vertices_.Push(vertex);
                        }

                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::TOP, x, y, z);
                            vertex.position = position + Vector3(0.0f, 1.0, 1.0);
                            vertex.normal = Vector3::UP;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::TOP, static_cast<BlockType>(blockId), Vector2(0.0, 1.0));
                            vertex.tangent = Vector4(0.0f, 0.0f, -1.0f, -1.0f);
                            chunk->vertices_.Push(vertex);
                        }

                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::TOP, x, y, z);
                            vertex.position = position + Vector3(1.0, 1.0, 0.0f);
                            vertex.normal = Vector3::UP;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::TOP, static_cast<BlockType>(blockId), Vector2(1.0, 0.0));
                            vertex.tangent = Vector4(0.0f, 0.0f, -1.0f, -1.0f);
                            chunk->vertices_.Push(vertex);
                        }

                        // tri2
                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::TOP, x, y, z);
                            vertex.position = position + Vector3(0.0f, 1.0, 1.0);
                            vertex.normal = Vector3::UP;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::TOP, static_cast<BlockType>(blockId), Vector2(0.0, 1.0));
                            vertex.tangent = Vector4(0.0f, 0.0f, -1.0f, -1.0f);
                            chunk->vertices_.Push(vertex);
                        }

                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::TOP, x, y, z);
                            vertex.position = position + Vector3(1.0, 1.0, 1.0);
                            vertex.normal = Vector3::UP;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::TOP, static_cast<BlockType>(blockId), Vector2(1.0, 1.0));
                            vertex.tangent = Vector4(0.0f, 0.0f, -1.0f, -1.0f);
                            chunk->vertices_.Push(vertex);
                        }

                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::TOP, x, y, z);
                            vertex.position = position + Vector3(1.0, 1.0, 0.0f);
                            vertex.normal = Vector3::UP;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::TOP, static_cast<BlockType>(blockId), Vector2(1.0, 0.0));
                            vertex.tangent = Vector4(0.0f, 0.0f, -1.0f, -1.0f);
                            chunk->vertices_.Push(vertex);
                        }
                    }
                    if (!chunk->BlockHaveNeighbor(BlockSide::BOTTOM, x, y, z)) {
                        // tri1
                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::BOTTOM, x, y, z);
                            vertex.position = position + Vector3(0.0f, 0.0f, 1.0);
                            vertex.normal = Vector3::DOWN;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::BOTTOM, static_cast<BlockType>(blockId), Vector2(0.0, 1.0));
                            vertex.tangent = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
                            chunk->vertices_.Push(vertex);
                        }

                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::BOTTOM, x, y, z);
                            vertex.position = position + Vector3(0.0f, 0.0f, 0.0f);
                            vertex.normal = Vector3::DOWN;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::BOTTOM, static_cast<BlockType>(blockId), Vector2(0.0, 0.0));
                            vertex.tangent = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
                            chunk->vertices_.Push(vertex);
                        }

                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::BOTTOM, x, y, z);
                            vertex.position = position + Vector3(1.0, 0.0f, 0.0f);
                            vertex.normal = Vector3::DOWN;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::BOTTOM, static_cast<BlockType>(blockId), Vector2(1.0, 0.0));
                            vertex.tangent = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
                            chunk->vertices_.Push(vertex);
                        }

                        // tri2
                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::BOTTOM, x, y, z);
                            vertex.position = position + Vector3(1.0, 0.0f, 1.0);
                            vertex.normal = Vector3::DOWN;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::BOTTOM, static_cast<BlockType>(blockId), Vector2(1.0, 1.0));
                            vertex.tangent = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
                            chunk->vertices_.Push(vertex);
                        }

                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::BOTTOM, x, y, z);
                            vertex.position = position + Vector3(0.0f, 0.0f, 1.0);
                            vertex.normal = Vector3::DOWN;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::BOTTOM, static_cast<BlockType>(blockId), Vector2(0.0, 1.0));
                            vertex.tangent = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
                            chunk->vertices_.Push(vertex);
                        }

                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::BOTTOM, x, y, z);
                            vertex.position = position + Vector3(1.0, 0.0f, 0.0f);
                            vertex.normal = Vector3::DOWN;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::BOTTOM, static_cast<BlockType>(blockId), Vector2(1.0, 0.0));
                            vertex.tangent = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
                            chunk->vertices_.Push(vertex);
                        }
                    }
                    if (!chunk->BlockHaveNeighbor(BlockSide::LEFT, x, y, z)) {
                        // tri1
                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::LEFT, x, y, z);
                            vertex.position = position + Vector3(0.0f, 0.0f, 1.0);
                            vertex.normal = Vector3::LEFT;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::LEFT, static_cast<BlockType>(blockId), Vector2(0.0, 1.0));
                            vertex.tangent = Vector4(1.0f, 0.0f,  0.0f,  1.0f);
                            chunk->vertices_.Push(vertex);
                        }

                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::LEFT, x, y, z);
                            vertex.position = position + Vector3(0.0f, 1.0, 1.0);
                            vertex.normal = Vector3::LEFT;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::LEFT, static_cast<BlockType>(blockId), Vector2(0.0, 0.0));
                            vertex.tangent = Vector4(1.0f, 0.0f,  0.0f,  1.0f);
                            chunk->vertices_.Push(vertex);
                        }

                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::LEFT, x, y, z);
                            vertex.position = position + Vector3(0.0f, 0.0f, 0.0f);
                            vertex.normal = Vector3::LEFT;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::LEFT, static_cast<BlockType>(blockId), Vector2(1.0, 1.0));
                            vertex.tangent = Vector4(1.0f, 0.0f,  0.0f,  1.0f);
                            chunk->vertices_.Push(vertex);
                        }

//                        // tri2
                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::LEFT, x, y, z);
                            vertex.position = position + Vector3(0.0f, 1.0, 1.0);
                            vertex.normal = Vector3::LEFT;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::LEFT, static_cast<BlockType>(blockId), Vector2(0.0, 0.0));
                            vertex.tangent = Vector4(1.0f, 0.0f,  0.0f,  1.0f);
                            chunk->vertices_.Push(vertex);
                        }

                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::LEFT, x, y, z);
                            vertex.position = position + Vector3(0.0f, 1.0, 0.0f);
                            vertex.normal = Vector3::LEFT;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::LEFT, static_cast<BlockType>(blockId), Vector2(1.0, 0.0));
                            vertex.tangent = Vector4(1.0f, 0.0f,  0.0f,  1.0f);
                            chunk->vertices_.Push(vertex);
                        }

                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::LEFT, x, y, z);
                            vertex.position = position + Vector3(0.0f, 0.0f, 0.0f);
                            vertex.normal = Vector3::LEFT;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::LEFT, static_cast<BlockType>(blockId), Vector2(1.0, 1.0));
                            vertex.tangent = Vector4(1.0f, 0.0f,  0.0f,  1.0f);
                            chunk->vertices_.Push(vertex);
                        }
                    }
                    if (!chunk->BlockHaveNeighbor(BlockSide::RIGHT, x, y, z)) {
                        // tri1
                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::RIGHT, x, y, z);
                            vertex.position = position + Vector3(1.0, 0.0f, 0.0f);
                            vertex.normal = Vector3::RIGHT;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::RIGHT, static_cast<BlockType>(blockId), Vector2(0.0, 1.0));
                            vertex.tangent = Vector4(0.0f, 0.0f, -1.0f, -1.0f);
                            chunk->vertices_.Push(vertex);
                        }

                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::RIGHT, x, y, z);
                            vertex.position = position + Vector3(1.0, 1.0, 0.0f);
                            vertex.normal = Vector3::RIGHT;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::RIGHT, static_cast<BlockType>(blockId), Vector2(0.0, 0.0));
                            vertex.tangent = Vector4(0.0f, 0.0f, -1.0f, -1.0f);
                            chunk->vertices_.Push(vertex);
                        }

                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::RIGHT, x, y, z);
                            vertex.position = position + Vector3(1.0, 0.0f, 1.0);
                            vertex.normal = Vector3::RIGHT;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::RIGHT, static_cast<BlockType>(blockId), Vector2(1.0, 1.0));
                            vertex.tangent = Vector4(0.0f, 0.0f, -1.0f, -1.0f);
                            chunk->vertices_.Push(vertex);
                        }

                        // tri2
                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::RIGHT, x, y, z);
                            vertex.position = position + Vector3(1.0, 1.0, 0.0f);
                            vertex.normal = Vector3::RIGHT;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::RIGHT, static_cast<BlockType>(blockId), Vector2(0.0, 0.0));
                            vertex.tangent = Vector4(0.0f, 0.0f, -1.0f, -1.0f);
                            chunk->vertices_.Push(vertex);
                        }

                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::RIGHT, x, y, z);
                            vertex.position = position + Vector3(1.0, 1.0, 1.0);
                            vertex.normal = Vector3::RIGHT;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::RIGHT, static_cast<BlockType>(blockId), Vector2(1.0, 0.0));
                            vertex.tangent = Vector4(0.0f, 0.0f, -1.0f, -1.0f);
                            chunk->vertices_.Push(vertex);
                        }

                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::RIGHT, x, y, z);
                            vertex.position = position + Vector3(1.0, 0.0f, 1.0);
                            vertex.normal = Vector3::RIGHT;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::RIGHT, static_cast<BlockType>(blockId), Vector2(1.0, 1.0));
                            vertex.tangent = Vector4(0.0f, 0.0f, -1.0f, -1.0f);
                            chunk->vertices_.Push(vertex);
                        }
                    }
                    if (!chunk->BlockHaveNeighbor(BlockSide::FRONT, x, y, z)) {
                        // tri1
                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::FRONT, x, y, z);
                            vertex.position = position + Vector3(0.0f, 0.0f, 0.0f);
                            vertex.normal = Vector3::FORWARD;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::FRONT, static_cast<BlockType>(blockId), Vector2(0.0, 1.0));
                            vertex.tangent = Vector4(1.0f, 0.0f,  0.0f,  1.0f);
                            chunk->vertices_.Push(vertex);
                        }

                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::FRONT, x, y, z);
                            vertex.position = position + Vector3(0.0f, 1.0, 0.0f);
                            vertex.normal = Vector3::FORWARD;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::FRONT, static_cast<BlockType>(blockId), Vector2(0.0, 0.0));
                            vertex.tangent = Vector4(1.0f, 0.0f,  0.0f,  1.0f);
                            chunk->vertices_.Push(vertex);
                        }

                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::FRONT, x, y, z);
                            vertex.position = position + Vector3(1.0, 0.0f, 0.0f);
                            vertex.normal = Vector3::FORWARD;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::FRONT, static_cast<BlockType>(blockId), Vector2(1.0, 1.0));
                            vertex.tangent = Vector4(1.0f, 0.0f,  0.0f,  1.0f);
                            chunk->vertices_.Push(vertex);
                        }

                        // tri2
                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::FRONT, x, y, z);
                            vertex.position = position + Vector3(0.0f, 1.0, 0.0f);
                            vertex.normal = Vector3::FORWARD;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::FRONT, static_cast<BlockType>(blockId), Vector2(0.0, 0.0));
                            vertex.tangent = Vector4(1.0f, 0.0f,  0.0f,  1.0f);
                            chunk->vertices_.Push(vertex);
                        }

                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::FRONT, x, y, z);
                            vertex.position = position + Vector3(1.0, 1.0, 0.0f);
                            vertex.normal = Vector3::FORWARD;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::FRONT, static_cast<BlockType>(blockId), Vector2(1.0, 0.0));
                            vertex.tangent = Vector4(1.0f, 0.0f,  0.0f,  1.0f);
                            chunk->vertices_.Push(vertex);
                        }

                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::FRONT, x, y, z);
                            vertex.position = position + Vector3(1.0, 0.0f, 0.0f);
                            vertex.normal = Vector3::FORWARD;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::FRONT, static_cast<BlockType>(blockId), Vector2(1.0, 1.0));
                            vertex.tangent = Vector4(1.0f, 0.0f,  0.0f,  1.0f);
                            chunk->vertices_.Push(vertex);
                        }
                    }
                    if (!chunk->BlockHaveNeighbor(BlockSide::BACK, x, y, z)) {
                        // tri1
                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::BACK, x, y, z);
                            vertex.position = position + Vector3(1.0, 0.0f, 1.0);
                            vertex.normal = Vector3::BACK;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::BACK, static_cast<BlockType>(blockId), Vector2(0.0, 1.0));
                            vertex.tangent = Vector4(1.0f, 0.0f,  0.0f,  1.0f);
                            chunk->vertices_.Push(vertex);
                        }

                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::BACK, x, y, z);
                            vertex.position = position + Vector3(1.0, 1.0, 1.0);
                            vertex.normal = Vector3::BACK;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::BACK, static_cast<BlockType>(blockId), Vector2(0.0, 0.0));
                            vertex.tangent = Vector4(1.0f, 0.0f,  0.0f,  1.0f);
                            chunk->vertices_.Push(vertex);
                        }

                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::BACK, x, y, z);
                            vertex.position = position + Vector3(0.0f, 0.0f, 1.0);
                            vertex.normal = Vector3::BACK;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::BACK, static_cast<BlockType>(blockId), Vector2(1.0, 1.0));
                            vertex.tangent = Vector4(1.0f, 0.0f,  0.0f,  1.0f);
                            chunk->vertices_.Push(vertex);
                        }

                        // tri2
                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::BACK, x, y, z);
                            vertex.position = position + Vector3(1.0, 1.0, 1.0);
                            vertex.normal = Vector3::BACK;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::BACK, static_cast<BlockType>(blockId), Vector2(0.0, 0.0));
                            vertex.tangent = Vector4(1.0f, 0.0f,  0.0f,  1.0f);
                            chunk->vertices_.Push(vertex);
                        }

                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::BACK, x, y, z);
                            vertex.position = position + Vector3(0.0f, 1.0, 1.0);
                            vertex.normal = Vector3::BACK;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::BACK, static_cast<BlockType>(blockId), Vector2(1.0, 0.0));
                            vertex.tangent = Vector4(1.0f, 0.0f,  0.0f,  1.0f);
                            chunk->vertices_.Push(vertex);
                        }

                        {
                            ChunkVertex vertex;
                            vertex.light = chunk->NeighborLightValue(BlockSide::BACK, x, y, z);
                            vertex.position = position + Vector3(0.0f, 0.0f, 1.0);
                            vertex.normal = Vector3::BACK;
                            vertex.uvCoords = chunk->GetTextureCoord(BlockSide::BACK, static_cast<BlockType>(blockId), Vector2(1.0, 1.0));
                            vertex.tangent = Vector4(1.0f, 0.0f,  0.0f,  1.0f);
                            chunk->vertices_.Push(vertex);
                        }
                    }
                }
            }
        }
    }
}

Chunk::Chunk(Context* context):
Object(context)
{
    for (int x = 0; x < SIZE_X; x++) {
        for (int y = 0; y < SIZE_Y; y++) {
            for (int z = 0; z < SIZE_Z; z++) {
                data_[x][y][z].type = BlockType::BT_AIR;
            }
        }
    }
    memset(lightMap_, 0, sizeof(lightMap_));
}

Chunk::~Chunk()
{
    if (generateWorkItem_) {
        WorkQueue *workQueue = GetSubsystem<WorkQueue>();
        if (workQueue) {
            workQueue->RemoveWorkItem(generateWorkItem_);
        }
    }
    if (saveWorkItem_) {
        WorkQueue *workQueue = GetSubsystem<WorkQueue>();
        if (workQueue) {
            workQueue->RemoveWorkItem(saveWorkItem_);
        }
    }
    if (generateGeometryWorkItem_) {
        WorkQueue *workQueue = GetSubsystem<WorkQueue>();
        if (workQueue) {
            workQueue->RemoveWorkItem(generateGeometryWorkItem_);
        }
    }
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
    triggerBody->SetCollisionLayerAndMask(COLLISION_MASK_CHUNK , COLLISION_MASK_CHUNK_LOADER);
    auto *triggerShape = triggerNode_->CreateComponent<CollisionShape>();
    triggerNode_->SetWorldScale(VoxelWorld::visibleDistance);

    SubscribeToEvent(triggerNode_, E_NODECOLLISIONSTART, URHO3D_HANDLER(Chunk, HandlePlayerEntered));
    SubscribeToEvent(triggerNode_, E_NODECOLLISIONEND, URHO3D_HANDLER(Chunk, HandlePlayerExited));

//    Vector3 offset = Vector3(SIZE_X / 2, SIZE_Y / 2, SIZE_Z / 2) * triggerNode_->GetScale();
    triggerShape->SetBox(Vector3(SIZE_X, SIZE_Y, SIZE_Z), Vector3(SIZE_X / 2, SIZE_Y / 2, SIZE_Z / 2));

    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Chunk, HandleUpdate));
    SubscribeToEvent(E_WORKITEMCOMPLETED, URHO3D_HANDLER(Chunk, HandleWorkItemFinished));

    SubscribeToEvent("#chunk_visible_distance", [&](StringHash eventType, VariantMap& eventData) {
        if (triggerNode_) {
            triggerNode_->SetWorldScale(VoxelWorld::visibleDistance);
        }
    });
//    SetVisibility(false);
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
    MutexLock lock(mutex_);
    geometry_->SetDynamic(true);
    geometry_->SetNumGeometries(1);
    geometry_->BeginGeometry(0, TRIANGLE_LIST);
    for (auto it = vertices_.Begin(); it != vertices_.End(); ++it) {
        geometry_->DefineVertex((*it).position);
        geometry_->DefineTexCoord((*it).uvCoords);
//        geometry_->DefineNormal((*it).normal);
//        geometry_->DefineTangent((*it).tangent);
        Color color;
        // Torchlight
        color.r_ = static_cast<int>((*it).light & 0xF) / 15.0f;
        // Sunlight
        color.g_ = static_cast<int>(((*it).light >> 4) & 0xF) / 15.0f;
        geometry_->DefineColor(color);
    }
    geometry_->Commit();
    geometry_->SetViewMask(VIEW_MASK_CHUNK);
    geometry_->SetOccluder(true);
    geometry_->SetOccludee(true);
    geometry_->SetCastShadows(true);

    auto cache = GetSubsystem<ResourceCache>();
    geometry_->SetMaterial(cache->GetResource<Material>("Materials/Voxel.xml"));

    auto *shape = node_->GetComponent<CollisionShape>();
    if (geometry_->GetNumVertices(0) > 0) {
        shape->SetCustomTriangleMesh(geometry_);
    } else {
        shape->SetEnabled(false);
    }
//    UpdateVisibility();
}

void Chunk::GenerateGeometry()
{
//    if (generateGeometryWorkItem_ && !generateGeometryWorkItem_->completed_) {
//        return;
//    }
    WorkQueue *workQueue = GetSubsystem<WorkQueue>();
    generateGeometryWorkItem_ = workQueue->GetFreeItem();
    generateGeometryWorkItem_->priority_ = 1;
    generateGeometryWorkItem_->workFunction_ = GenerateVertices;
    generateGeometryWorkItem_->aux_ = this;
    generateGeometryWorkItem_->sendEvent_ = true;

    generateGeometryWorkItem_->start_ = nullptr;
    generateGeometryWorkItem_->end_ = nullptr;
    workQueue->AddWorkItem(generateGeometryWorkItem_);
}

void Chunk::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    if (!node_) {
        return;
    }
//    scene_->GetComponent<PhysicsWorld>()->DrawDebugGeometry(true);
//    scene_->GetComponent<PhysicsWorld>()->SetDebugRenderer(scene_->GetComponent<DebugRenderer>());
//    node_->GetComponent<StaticModel>()->DrawDebugGeometry(node_->GetScene()->GetComponent<DebugRenderer>(), true);

    if (!lightBfsQueue_.empty() || !lightRemovalBfsQueue_.empty()) {
        GenerateGeometry();
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
    Vector3 position = eventData["Position"].GetVector3();
    auto blockPosition = GetChunkBlock(position);
    if (!IsBlockInsideChunk(blockPosition)) {
        auto neighborChunk = GetSubsystem<VoxelWorld>()->GetChunkByPosition(position);
        if (neighborChunk && neighborChunk->GetNode()) {
            neighborChunk->GetNode()->SendEvent("ChunkHit", eventData);
        }
        return;
    }
    if (data_[blockPosition.x_][blockPosition.y_][blockPosition.z_].type) {
//        int blockId = data[blockPosition.x_][blockPosition.y_][blockPosition.z_];
        if (data_[blockPosition.x_][blockPosition.y_][blockPosition.z_].type == BlockType::BT_TORCH) {
            int lightValue = GetTorchlight(blockPosition.x_, blockPosition.y_, blockPosition.z_);
            SetTorchlight(blockPosition.x_, blockPosition.y_, blockPosition.z_, 0);
            AddLightRemovalNode(blockPosition.x_, blockPosition.y_, blockPosition.z_, lightValue);
        }
        data_[blockPosition.x_][blockPosition.y_][blockPosition.z_].type = BlockType::BT_AIR;
//        URHO3D_LOGINFO("Removing block " + blockPosition.ToString());
        GenerateGeometry();
        if (blockPosition.x_ == 0) {
            auto neighbor = GetNeighbor(BlockSide::LEFT);
            if (neighbor) {
                URHO3D_LOGINFO("Rerendering neigbhbor left");
                neighbor->Render();
            }
        }
        if (blockPosition.x_ == SIZE_X - 1) {
            auto neighbor = GetNeighbor(BlockSide::RIGHT);
            if (neighbor) {
                URHO3D_LOGINFO("Rerendering neigbhbor right");
                neighbor->Render();
            }
        }
        if (blockPosition.y_ == 0) {
            auto neighbor = GetNeighbor(BlockSide::BOTTOM);
            if (neighbor) {
                URHO3D_LOGINFO("Rerendering neigbhbor bottom");
                neighbor->Render();
            }
        }
        if (blockPosition.y_ == SIZE_Y - 1) {
            auto neighbor = GetNeighbor(BlockSide::TOP);
            if (neighbor) {
                URHO3D_LOGINFO("Rerendering neigbhbor top");
                neighbor->Render();
            }
        }
        if (blockPosition.z_ == 0) {
            auto neighbor = GetNeighbor(BlockSide::FRONT);
            if (neighbor) {
                URHO3D_LOGINFO("Rerendering neigbhbor back");
                neighbor->Render();
            }
        }
        if (blockPosition.z_ == SIZE_Z - 1) {
            auto neighbor = GetNeighbor(BlockSide::BACK);
            if (neighbor) {
                URHO3D_LOGINFO("Rerendering neigbhbor front");
                neighbor->Render();
            }
        }
    }
    Save();
}

void Chunk::HandleAdd(StringHash eventType, VariantMap& eventData)
{
    Vector3 position = eventData["Position"].GetVector3();
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
        if (eventData["Action"].GetInt() == CTRL_DETECT) {
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
        data_[blockPosition.x_][blockPosition.y_][blockPosition.z_].type = BlockType::BT_TORCH;
//        URHO3D_LOGINFOF("Controller %d added block", eventData["ControllerId"].GetInt());
//        URHO3D_LOGINFOF("Chunk add world pos: %s; chunk pos: %d %d %d", pos.ToString().CString(), x, y, z);
        SetTorchlight(blockPosition.x_, blockPosition.y_, blockPosition.z_, 15);
        lightBfsQueue_.emplace(blockPosition.x_, blockPosition.y_, blockPosition.z_, this);
        GenerateGeometry();
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
        CreateNode();
        generateWorkItem_.Reset();
    } else if (workItem->workFunction_ == GenerateVertices) {
        UpdateGeometry();
        generateGeometryWorkItem_.Reset();
    } else if (workItem->workFunction_ == SaveToFile) {
//        URHO3D_LOGINFO("Background chunk saving done " + position_.ToString());
        saveWorkItem_.Reset();
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
    WorkQueue *workQueue = GetSubsystem<WorkQueue>();
    saveWorkItem_ = workQueue->GetFreeItem();
    saveWorkItem_->priority_ = M_MAX_UNSIGNED;
    saveWorkItem_->workFunction_ = SaveToFile;
    saveWorkItem_->aux_ = this;
    // send E_WORKITEMCOMPLETED event after finishing WorkItem
    saveWorkItem_->sendEvent_ = true;

    saveWorkItem_->start_ = nullptr;
    saveWorkItem_->end_ = nullptr;
    workQueue->AddWorkItem(saveWorkItem_);

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

    SubscribeToEvent(node_, "ChunkHit", URHO3D_HANDLER(Chunk, HandleHit));
    SubscribeToEvent(node_, "ChunkAdd", URHO3D_HANDLER(Chunk, HandleAdd));

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

    MarkActive(false);
    SetActive();

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

unsigned char Chunk::NeighborLightValue(BlockSide side, int x, int y, int z)
{
    switch(side) {
        case BlockSide::TOP:
            if (y == SIZE_Y - 1) {
                auto neighbor = GetNeighbor(BlockSide::TOP);
                if (neighbor) {
                    return neighbor->GetLightValue(x, 0, z);
                }
            } else if (y + 1 < SIZE_Y) {
                return GetLightValue(x, y + 1, z);
            }
        case BlockSide::BOTTOM:
            if (y == 0) {
                auto neighbor = GetNeighbor(BlockSide::BOTTOM);
                if (neighbor) {
                    return neighbor->GetLightValue(x, SIZE_Y - 1, z);
                }
            } else if (y > 0) {
                return GetLightValue(x, y - 1, z);
            }
        case BlockSide::LEFT:
            if (x == 0) {
                auto neighbor = GetNeighbor(BlockSide::LEFT);
                if (neighbor) {
                    return neighbor->GetLightValue(SIZE_X - 1, y, z);
                }
            } else if (x > 0) {
                return GetLightValue(x - 1, y, z);
            }
        case BlockSide::RIGHT:
            if (x == SIZE_X - 1) {
                auto neighbor = GetNeighbor(BlockSide::RIGHT);
                if (neighbor) {
                    return neighbor->GetLightValue(0, y, z);
                }
            } else if (x + 1 < SIZE_X) {
                return GetLightValue(x + 1, y, z);
            }
        case BlockSide::FRONT:
            if (z == 0) {
                auto neighbor = GetNeighbor(BlockSide::FRONT);
                if (neighbor) {
                    return neighbor->GetLightValue(x, y, SIZE_Z - 1);
                }
            } else if (z > 0) {
                return GetLightValue(x, y, z - 1);
            }
        case BlockSide::BACK:
            if (z == SIZE_Z - 1) {
                auto neighbor = GetNeighbor(BlockSide::BACK);
                if (neighbor) {
                    return neighbor->GetLightValue(x, y, 0);
                }
            } else if (z + 1 < SIZE_Z) {
                return GetLightValue(x, y, z + 1);
            }
    }
    return 0x0;
}

bool Chunk::BlockHaveNeighbor(BlockSide side, int x, int y, int z)
{
    switch(side) {
        case BlockSide::TOP:
            if (y == SIZE_Y - 1) {
                auto neighbor = GetNeighbor(BlockSide::TOP);
                if (neighbor && neighbor->GetBlockValue(x, 0, z) == BlockType::BT_AIR) {
                    return false;
                }
            } else if (y + 1 < SIZE_Y) {
                if (data_[x][y + 1][z].type == BlockType::BT_AIR) {
                    return false;
                }
            }
            return true;
        case BlockSide::BOTTOM:
            if (y == 0) {
                auto neighbor = GetNeighbor(BlockSide::BOTTOM);
                if (neighbor && neighbor->GetBlockValue(x, SIZE_Y - 1, z) == BlockType::BT_AIR) {
                    return false;
                }
            } else if (y > 0) {
                if (data_[x][y - 1][z].type == BlockType::BT_AIR) {
                    return false;
                }
            }
            return true;
        case BlockSide::LEFT:
            if (x == 0) {
                auto neighbor = GetNeighbor(BlockSide::LEFT);
                if (neighbor && neighbor->GetBlockValue(SIZE_X - 1, y, z) == BlockType::BT_AIR) {
                    return false;
                }
            } else if (x > 0) {
                if (data_[x - 1][y][z].type == BlockType::BT_AIR) {
                    return false;
                }
            }
            return true;
        case BlockSide::RIGHT:
            if (x == SIZE_X - 1) {
                auto neighbor = GetNeighbor(BlockSide::RIGHT);
                if (neighbor && neighbor->GetBlockValue(0, y, z) == BlockType::BT_AIR) {
                    return false;
                }
            } else if (x + 1 < SIZE_X) {
                if (data_[x + 1][y][z].type == BlockType::BT_AIR) {
                    return false;
                }
            }
            return true;
        case BlockSide::FRONT:
            if (z == 0) {
                auto neighbor = GetNeighbor(BlockSide::FRONT);
                if (neighbor && neighbor->GetBlockValue(x, y, SIZE_Z - 1) == BlockType::BT_AIR) {
                    return false;
                }
            } else if (z > 0) {
                if (data_[x][y][z - 1].type == BlockType::BT_AIR) {
                    return false;
                }
            }
            return true;
        case BlockSide::BACK:
            if (z == SIZE_Z - 1) {
                auto neighbor = GetNeighbor(BlockSide::BACK);
                if (neighbor && neighbor->GetBlockValue(x, y, 0) == BlockType::BT_AIR) {
                    return false;
                }
            } else if (z + 1 < SIZE_Z) {
                if (data_[x][y][z + 1].type == BlockType::BT_AIR) {
                    return false;
                }
            }
            return true;
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

bool Chunk::IsVisible()
{
    return visible_;
}

Chunk* Chunk::GetNeighbor(BlockSide side)
{
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

void Chunk::Render(bool neighborLoaded)
{
    if (neighborLoaded) {
        if (loadedNeighbors_ < 6) {
            loadedNeighbors_ = 0;
//            URHO3D_LOGINFO("Chunk " + position_.ToString() + " render from neighbor required");
            for (int i = 0; i < 6; i++) {
                if (GetNeighbor(static_cast<BlockSide>(i))) {
                    loadedNeighbors_++;
                }
            }
            if (geometry_) {
                GenerateGeometry();
            }
        }
    } else {
        if (geometry_) {
            GenerateGeometry();
        }
    }
}
//
//void Chunk::SetVisibility(bool value)
//{
//    visible_ = value;
//    if (geometry_) {
//        geometry_->SetEnabled(visible_);
//    }
//}

void Chunk::RenderNeighbors()
{
    for (int i = 0; i < 6; i++) {
        auto neighbor = GetNeighbor(static_cast<BlockSide>(i));
        if (neighbor) {
            neighbor->Render(true);
        }
    }
}

VoxelBlock* Chunk::GetNeighborBlockByLocalPosition(IntVector3 position)
{
    if (position.x_ < 0) {
        auto neighbor = GetNeighbor(BlockSide::LEFT);
        if (neighbor) {
            position.x_ = SIZE_X - position.x_;
            return neighbor->GetBlockAt(position);
        }
    }

    if (position.x_ >= SIZE_X) {
        auto neighbor = GetNeighbor(BlockSide::RIGHT);
        if (neighbor) {
            position.x_ = position.x_ - SIZE_X;
            return neighbor->GetBlockAt(position);
        }
    }

    if (position.y_ < 0) {
        auto neighbor = GetNeighbor(BlockSide::BOTTOM);
        if (neighbor) {
            position.y_ = SIZE_Y - position.y_;
            return neighbor->GetBlockAt(position);
        }
    }

    if (position.y_ >= SIZE_Y) {
        auto neighbor = GetNeighbor(BlockSide::TOP);
        if (neighbor) {
            position.y_ = position.y_ - SIZE_Y;
            return neighbor->GetBlockAt(position);
        }
    }

    if (position.z_ < 0) {
        auto neighbor = GetNeighbor(BlockSide::FRONT);
        if (neighbor) {
            position.z_ = SIZE_Z - position.z_;
            return neighbor->GetBlockAt(position);
        }
    }

    if (position.z_ >= SIZE_Z) {
        auto neighbor = GetNeighbor(BlockSide::BACK);
        if (neighbor) {
            position.z_ = position.z_ - SIZE_Z;
            return neighbor->GetBlockAt(position);
        }
    }

    return nullptr;
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
}

int Chunk::GetTorchlight(int x, int y, int z)
{
    return lightMap_[x][y][z] & 0xF;
}

void Chunk::SetTorchlight(int x, int y, int z, int value)
{
    lightMap_[x][y][z] = (lightMap_[x][y][z] & 0xF0) | value;
}

void Chunk::ProcessQueues()
{
    while(!lightRemovalBfsQueue_.empty()) {
        // Get a reference to the front node
        LightRemovalNode &node = lightRemovalBfsQueue_.front();
        int lightLevel = (int)node.value_;
        Chunk* chunk = node.chunk_;
        // Pop the front node off the queue.
        lightRemovalBfsQueue_.pop();
        // Extract x, y, and z from our chunk. Same as before.
        // NOTE: Don't forget chunk bounds checking! I didn't show it here.
        // Check negative X neighbor

        for (int i = 0; i < 6; i++) {
            int dX = node.x_;
            int dY = node.y_;
            int dZ = node.z_;
            bool insideChunk = true;
            switch (i) {
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
                auto neighborLightLevel = GetTorchlight(dX, dY, dZ);
                if (neighborLightLevel != 0 && neighborLightLevel < lightLevel) {
                    SetTorchlight(dX, dY, dZ, 0);
                    AddLightRemovalNode(dX, dY, dZ, neighborLightLevel);
                } else if (neighborLightLevel >= lightLevel) {
                    AddLightNode(dX, dY, dZ);
                }
            } else {
                auto neighbor = GetNeighbor(static_cast<BlockSide>(i));
                if (neighbor) {
                    auto neighborLightLevel = neighbor->GetTorchlight(dX, dY, dZ);
                    if (neighborLightLevel != 0 && neighborLightLevel < lightLevel) {
                        neighbor->SetTorchlight(dX, dY, dZ, 0);
                        neighbor->AddLightRemovalNode(dX, dY, dZ, neighborLightLevel);
                    } else if (neighborLightLevel >= lightLevel) {
                         neighbor->AddLightNode(SIZE_X - 1, node.y_, node.z_);
                    }
                }
            }
        }
    }
    while(!lightBfsQueue_.empty()) {
        // Get a reference to the front node.
        LightNode &node = lightBfsQueue_.front();
        Chunk* chunk = node.chunk_;
        // Pop the front node off the queue. We no longer need the node reference
        lightBfsQueue_.pop();
        // Grab the light level of the current node
        int lightLevel = chunk->GetTorchlight(node.x_, node.y_, node.z_);
        // NOTE: You will need to do bounds checking!
        // If you are on the edge of a chunk, then x - 1 will be -1. Instead
        // you need to look at your left neighboring chunk and check the
        // adjacent block there. When you do that, be sure to use the
        // neighbor chunk when emplacing the new node to lightBfsQueue;
        // Check negative X neighbor
        // Make sure you don't propagate light into opaque blocks like stone!

        for (int i = 0; i < 6; i++) {
            int dX = node.x_;
            int dY = node.y_;
            int dZ = node.z_;
            bool insideChunk = true;
            switch (i) {
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
                if (chunk->GetBlockAt(IntVector3(dX, dY, dZ))->type == BlockType::BT_AIR &&
                    chunk->GetTorchlight(dX, dY, dZ) + 2 <= lightLevel) {
                    chunk->SetTorchlight(dX, dY, dZ, lightLevel - 1);
                    lightBfsQueue_.emplace(dX, dY, dZ, chunk);
                }
            } else {
                auto neighbor = GetNeighbor(static_cast<BlockSide>(i));
                if (neighbor) {
                    if (neighbor->GetBlockAt(IntVector3(dX, dY, dZ))->type == BlockType::BT_AIR &&
                        neighbor->GetTorchlight(dX, dY, dZ) + 2 <= lightLevel) {
                        neighbor->SetTorchlight(dX, dY, dZ, lightLevel - 1);
                        neighbor->AddLightNode(dX, dY, dZ);
                    }
                }
            }
        }
    }
}

void Chunk::AddLightNode(int x, int y, int z)
{
    lightBfsQueue_.emplace(x, y, z, this);
}

void Chunk::AddLightRemovalNode(int x, int y, int z, int level)
{
    lightRemovalBfsQueue_.emplace(x, y, z, level, this);
}

unsigned char Chunk::GetLightValue(int x, int y, int z)
{
    return lightMap_[x][y][z];
}

void Chunk::SetSunlight(int value)
{
    for (int x = 0; x < SIZE_X; x++) {
        for (int y = 0; y < SIZE_Y; y++) {
            for (int z = 0; z < SIZE_Z; z++) {
                SetSunlight(x, y, z, value);
            }
        }
    }
}