#ifdef VOXEL_SUPPORT
#pragma once
#include <Urho3D/Core/Object.h>
#include <queue>
#include "VoxelDefs.h"
#include "VoxelEvents.h"
#include "Chunk.h"

using namespace Urho3D;

struct TreeNode {
    TreeNode(short x, short y, short z, int height, int width, Chunk* ch) : x_(x), y_(y), z_(z), height_(height), width_(width), chunk_(ch) {}
    short x_;
    short y_;
    short z_;
    Chunk* chunk_;
    int height_;
    int width_;
};

class TreeGenerator : public Object {
URHO3D_OBJECT(TreeGenerator, Object);
    TreeGenerator(Context* context);
    virtual ~TreeGenerator();

public:
    static void RegisterObject(Context* context);
    void AddTreeNode(int x, int y, int z, int height, int width, Chunk* chunk);
    void Process();

private:
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleChunkGenerated(StringHash eventType, VariantMap& eventData);

    std::queue<TreeNode> treeBfsQueue_;
    std::queue<TreeNode> failedTreeBfsQueue_;
    Mutex mutex_;
};
#endif
