#include <Urho3D/Core/Context.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/IO/Log.h>
#include "TreeGenerator.h"
#include "VoxelWorld.h"

using namespace VoxelEvents;

TreeGenerator::TreeGenerator(Context* context):
        Object(context)
{
//    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(TreeGenerator, HandleUpdate));
    SubscribeToEvent(E_CHUNK_GENERATED, URHO3D_HANDLER(TreeGenerator, HandleChunkGenerated));
}

TreeGenerator::~TreeGenerator()
{
}

void TreeGenerator::RegisterObject(Context* context)
{
    context->RegisterFactory<TreeGenerator>();
}

void TreeGenerator::AddTreeNode(int x, int y, int z, int height, int width, Chunk *chunk)
{
    treeBfsQueue_.emplace(x, y, z, height, width, chunk);
    chunk->MarkForGeometryCalculation();
//    URHO3D_LOGINFOF("AddTreeNode %d %d %d => %d", x, y, z, height);
}

void TreeGenerator::Process()
{
    MutexLock lock(mutex_);

    while(!treeBfsQueue_.empty()) {
        // Get a reference to the front node.
        TreeNode &node = treeBfsQueue_.front();
        Chunk* chunk = node.chunk_;
        int height = node.height_;
        int width = node.width_;
        // Pop the front node off the queue. We no longer need the node reference
        treeBfsQueue_.pop();
        if (!GetSubsystem<VoxelWorld>()->IsChunkValid(chunk)) {
            continue;
        }

        for (int i = 0; i < 6; i++) {
            BlockSide side = static_cast<BlockSide>(i);
            int dX = node.x_;
            int dY = node.y_;
            int dZ = node.z_;
            bool insideChunk = true;

            if (side == BlockSide::BOTTOM) {
                continue;
            }

            if (height < 5 && side != BlockSide::TOP) {
                continue;
            }

            if (side == BlockSide::TOP) {
                height++;
            }
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

            int dWidth = width;
            if (side == BlockSide::FRONT || side == BlockSide::BACK || side == BlockSide::LEFT || side == BlockSide::RIGHT) {
                dWidth++;
                if (width > 2) {
                    continue;
                }
            }
            if (insideChunk) {
                BlockType type = chunk->GetBlockAt(IntVector3(dX, dY, dZ))->type;
                if (type == BT_AIR) {
                    chunk->SetVoxel(dX, dY, dZ, height > 5 ? BT_TREE_LEAVES : BT_WOOD);
                    if (height < 10) {
                        AddTreeNode(dX, dY, dZ, height, dWidth, chunk);
                    }
                }
            } else {
                auto neighbor = chunk->GetNeighbor(static_cast<BlockSide>(i));
                if (neighbor) {
                    BlockType type = neighbor->GetBlockAt(IntVector3(dX, dY, dZ))->type;
                    if (type == BT_AIR) {
                        neighbor->SetVoxel(dX, dY, dZ, height > 5 ? BT_TREE_LEAVES : BT_WOOD);
                        if (height < 10) {
                            AddTreeNode(dX, dY, dZ, height, dWidth, neighbor);
                        }
                    }
                } else {
                    failedTreeBfsQueue_.emplace(node);
                }
            }
        }
    }
}

void TreeGenerator::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    if (GetSubsystem<VoxelWorld>()) {
        Process();
    }
}

void TreeGenerator::HandleChunkGenerated(StringHash eventType, VariantMap& eventData)
{
    MutexLock lock(mutex_);
    while(!failedTreeBfsQueue_.empty()) {
        treeBfsQueue_.emplace(failedTreeBfsQueue_.front());
        failedTreeBfsQueue_.pop();
    }
}