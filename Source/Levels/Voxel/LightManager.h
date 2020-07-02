#pragma once
#include <Urho3D/Core/Object.h>
#include <queue>
#include "VoxelDefs.h"
#include "VoxelEvents.h"
#include "Chunk.h"

using namespace Urho3D;

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

class LightManager : public Object {
URHO3D_OBJECT(LightManager, Object);
    LightManager(Context* context);
    virtual ~LightManager();

public:
    static void RegisterObject(Context* context);
    void AddLightNode(int x, int y, int z, Chunk* chunk);
    void AddLightRemovalNode(int x, int y, int z, int level, Chunk* chunk);

//    void AddFailedLightNode(int x, int y, int z, Vector3 position);
//    void AddFailedLightRemovalNode(int x, int y, int z, int level, Vector3 position);
    void Process();

private:
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleEvents(StringHash eventType, VariantMap& eventData);

    std::queue<LightNode> lightBfsQueue_;
    std::queue<LightRemovalNode> lightRemovalBfsQueue_;

    std::queue<LightNode> failedLightBfsQueue_;
    std::queue<LightRemovalNode> failedLightRemovalBfsQueue_;

    Mutex mutex_;
};
