#include <Urho3D/Core/Context.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/IO/Log.h>
#include "LightManager.h"
#include "VoxelWorld.h"

using namespace VoxelEvents;

LightManager::LightManager(Context* context):
        Object(context)
{
//    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(LightManager, HandleUpdate));
    SubscribeToEvent(E_CHUNK_GENERATED, URHO3D_HANDLER(LightManager, HandleEvents));
    SubscribeToEvent(E_BLOCK_ADDED, URHO3D_HANDLER(LightManager, HandleEvents));
    SubscribeToEvent(E_BLOCK_REMOVED, URHO3D_HANDLER(LightManager, HandleEvents));
}

LightManager::~LightManager()
{
}

void LightManager::RegisterObject(Context* context)
{
    context->RegisterFactory<LightManager>();
}

void LightManager::AddLightNode(int x, int y, int z, Chunk* chunk)
{
    lightBfsQueue_.emplace(x, y, z, chunk);
    chunk->MarkForGeometryCalculation();
}

void LightManager::AddLightRemovalNode(int x, int y, int z, int level, Chunk* chunk)
{
    lightRemovalBfsQueue_.emplace(x, y, z, level, chunk);
    chunk->MarkForGeometryCalculation();
}

//void LightManager::AddFailedLightNode(int x, int y, int z, Vector3 position)
//{
//    failedLightBfsQueue_.emplace(x, y, z, position);
//}
//
//void LightManager::AddFailedLightRemovalNode(int x, int y, int z, int level, Vector3 position)
//{
//    failedLightRemovalBfsQueue_.emplace(x, y, z, level, position);
//}

void LightManager::Process()
{
    MutexLock lock(mutex_);
    while(!lightRemovalBfsQueue_.empty()) {
        // Get a reference to the front node
        LightRemovalNode &node = lightRemovalBfsQueue_.front();
        int lightLevel = (int)node.value_;
        Chunk* chunk = node.chunk_;
        // Pop the front node off the queue.
        lightRemovalBfsQueue_.pop();
        if (!GetSubsystem<VoxelWorld>()->IsChunkValid(chunk)) {
            continue;
        }
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
                auto neighborLightLevel = chunk->GetTorchlight(dX, dY, dZ);
                if (neighborLightLevel != 0 && neighborLightLevel < lightLevel) {
                    chunk->SetTorchlight(dX, dY, dZ, 0);
                    AddLightRemovalNode(dX, dY, dZ, neighborLightLevel, chunk);
                } else if (neighborLightLevel >= lightLevel) {
                    AddLightNode(dX, dY, dZ, chunk);
                }
            } else {
                auto neighbor = chunk->GetNeighbor(static_cast<BlockSide>(i));
                if (neighbor) {
                    auto neighborLightLevel = neighbor->GetTorchlight(dX, dY, dZ);
                    if (neighborLightLevel != 0 && neighborLightLevel < lightLevel) {
                        neighbor->SetTorchlight(dX, dY, dZ, 0);
                        AddLightRemovalNode(dX, dY, dZ, neighborLightLevel, neighbor);
                    } else if (neighborLightLevel >= lightLevel) {
                        AddLightNode(SIZE_X - 1, node.y_, node.z_, neighbor);
                    }
                } else {
                    failedLightRemovalBfsQueue_.emplace(node);
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
        if (!GetSubsystem<VoxelWorld>()->IsChunkValid(chunk)) {
            continue;
        }
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
                BlockType type = chunk->GetBlockAt(IntVector3(dX, dY, dZ))->type;
                int blockLightLevel = chunk->GetTorchlight(dX, dY, dZ);
                if ((type == BlockType::BT_AIR || type == BlockType::BT_WATER) && blockLightLevel + 2 <= lightLevel) {
                    if (type == BlockType::BT_WATER) {
                        // Light in water will fade out a bit quicker
                        chunk->SetTorchlight(dX, dY, dZ, lightLevel - 2);
                    } else {
                        chunk->SetTorchlight(dX, dY, dZ, lightLevel - 1);
                    }
                    lightBfsQueue_.emplace(dX, dY, dZ, chunk);
                }
            } else {
                auto neighbor = chunk->GetNeighbor(static_cast<BlockSide>(i));
                if (neighbor) {
                    BlockType type = neighbor->GetBlockAt(IntVector3(dX, dY, dZ))->type;
                    int blockLightLevel = neighbor->GetTorchlight(dX, dY, dZ);
                    if ((type == BlockType::BT_AIR || type == BlockType::BT_WATER) && blockLightLevel + 2 <= lightLevel) {
                        if (type == BlockType::BT_WATER) {
                            // Light in water will fade out a bit quicker
                            neighbor->SetTorchlight(dX, dY, dZ, lightLevel - 2);
                        } else {
                            neighbor->SetTorchlight(dX, dY, dZ, lightLevel - 1);
                        }
                        AddLightNode(dX, dY, dZ, neighbor);
                    }
                } else {
                    failedLightBfsQueue_.emplace(node);
                }
            }
        }
    }
}

void LightManager::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    if (GetSubsystem<VoxelWorld>()) {
        Process();
    }
}

void LightManager::HandleEvents(StringHash eventType, VariantMap& eventData)
{
    MutexLock lock(mutex_);
    while(!failedLightBfsQueue_.empty()) {
        lightBfsQueue_.emplace(failedLightBfsQueue_.front());
        failedLightBfsQueue_.pop();
    }

    while(!failedLightRemovalBfsQueue_.empty()) {
        lightRemovalBfsQueue_.emplace(failedLightRemovalBfsQueue_.front());
        failedLightRemovalBfsQueue_.pop();
    }
}