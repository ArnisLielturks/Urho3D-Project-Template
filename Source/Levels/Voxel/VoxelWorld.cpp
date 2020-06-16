#include <Urho3D/IO/Log.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/DebugHud.h>
#include "VoxelWorld.h"
#include "../../SceneManager.h"
#include "VoxelEvents.h"

using namespace VoxelEvents;

VoxelWorld::VoxelWorld(Context* context):
    Object(context)
{
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(VoxelWorld, HandleUpdate));
    scene_ = GetSubsystem<SceneManager>()->GetActiveScene();

    SubscribeToEvent(E_CHUNK_ENTERED, URHO3D_HANDLER(VoxelWorld, HandleChunkEntered));
    SubscribeToEvent(E_CHUNK_EXITED, URHO3D_HANDLER(VoxelWorld, HandleChunkExited));
}

void VoxelWorld::RegisterObject(Context* context)
{
    context->RegisterFactory<VoxelWorld>();
}

void VoxelWorld::AddObserver(SharedPtr<Node> observer)
{
    observers_.Push(observer);
    URHO3D_LOGINFO("Adding observer to voxel world!");
    Vector3 position = GetNodeToChunkPosition(observer);
    LoadChunk(position);
}

void VoxelWorld::RemoveObserver(SharedPtr<Node> observer)
{
    auto it = observers_.Find(observer);
    if (it != observers_.End()) {
        observers_.Erase(it);
        URHO3D_LOGINFO("Removing observer from voxel world!");
    }
}

void VoxelWorld::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    if (!pendingChunks_.Empty() && updateTimer_.GetMSec(false) > 100) {
        updateTimer_.Reset();
        CreateChunk(pendingChunks_.Front());
        pendingChunks_.PopFront();
        if (GetSubsystem<DebugHud>()) {
            GetSubsystem<DebugHud>()->SetAppStats("Chunks Loaded", chunks_.Size());
        }
    }

    if (!removeBlocks_.Empty() && updateTimer_.GetMSec(false) > 100) {
        updateTimer_.Reset();
        auto chunk = GetChunkByPosition(removeBlocks_.Front());
        if (chunk) {
            VariantMap& data = GetEventDataMap();
            data["Position"] = removeBlocks_.Front();
            data["ControllerId"] = -1;
            if (chunk->GetNode()) {
                chunk->GetNode()->SendEvent("ChunkHit", data);
                removeBlocks_.PopFront();
            }
        }
    }
}

void VoxelWorld::CreateChunk(const Vector3& position)
{
    chunks_.Push(SharedPtr<Chunk>(new Chunk(context_)));
    chunks_.Back()->Init(1, scene_, position);
}

Vector3 VoxelWorld::GetNodeToChunkPosition(Node* node)
{
    Vector3 position = node->GetWorldPosition();
    return GetWorldToChunkPosition(position);
}

bool VoxelWorld::IsChunkLoaded(Vector3 position)
{
    for (auto it = chunks_.Begin(); it != chunks_.End(); ++it) {
        Vector3 chunkPosition = (*it)->GetPosition();
        if (IsEqualPositions(position, chunkPosition)) {
            return true;
        }
    }

    return false;
}

void VoxelWorld::HandleChunkEntered(StringHash eventType, VariantMap& eventData)
{
    using namespace ChunkEntered;
    Vector3 position = eventData[P_POSITION].GetVector3();
    LoadChunk(position);
}

void VoxelWorld::HandleChunkExited(StringHash eventType, VariantMap& eventData)
{

}

void VoxelWorld::LoadChunk(Vector3 position)
{
    position.y_ = -SIZE_Y - 2;
    if (!IsChunkLoaded(position) && !IsChunkPending(position)) {
        CreateChunk(position);
        URHO3D_LOGINFO("Loading chunk" + position.ToString());
    }
    Vector<Vector3> positions;
    // Same level
    positions.Push(Vector3(position + Vector3::LEFT * SIZE_X));
    positions.Push(Vector3(position + Vector3::RIGHT * SIZE_X));
    positions.Push(Vector3(position + Vector3::FORWARD * SIZE_Z));
    positions.Push(Vector3(position + Vector3::BACK * SIZE_Z));

    positions.Push(Vector3(position + Vector3::BACK * SIZE_Z + Vector3::LEFT * SIZE_X));
    positions.Push(Vector3(position + Vector3::BACK * SIZE_Z + Vector3::RIGHT * SIZE_X));
    positions.Push(Vector3(position + Vector3::FORWARD * SIZE_Z + Vector3::LEFT * SIZE_X));
    positions.Push(Vector3(position + Vector3::FORWARD * SIZE_Z + Vector3::RIGHT * SIZE_X));

    // Up
//    positions.Push(Vector3(position + Vector3::UP * SIZE_Y + Vector3::LEFT * SIZE_X));
//    positions.Push(Vector3(position + Vector3::UP * SIZE_Y + Vector3::RIGHT * SIZE_X));
//    positions.Push(Vector3(position + Vector3::UP * SIZE_Y + Vector3::FORWARD * SIZE_Z));
//    positions.Push(Vector3(position + Vector3::UP * SIZE_Y + Vector3::BACK * SIZE_Z));
//    positions.Push(Vector3(position + Vector3::UP * SIZE_Y));
//
//    positions.Push(Vector3(position + Vector3::UP * SIZE_Y + Vector3::BACK * SIZE_Z + Vector3::LEFT * SIZE_X));
//    positions.Push(Vector3(position + Vector3::UP * SIZE_Y + Vector3::BACK * SIZE_Z + Vector3::RIGHT * SIZE_X));
//    positions.Push(Vector3(position + Vector3::UP * SIZE_Y + Vector3::FORWARD * SIZE_Z + Vector3::LEFT * SIZE_X));
//    positions.Push(Vector3(position + Vector3::UP * SIZE_Y + Vector3::FORWARD * SIZE_Z + Vector3::RIGHT * SIZE_X));
//
//    // Down
//    positions.Push(Vector3(position + Vector3::DOWN * SIZE_Y + Vector3::LEFT * SIZE_X));
//    positions.Push(Vector3(position + Vector3::DOWN * SIZE_Y + Vector3::RIGHT * SIZE_X));
//    positions.Push(Vector3(position + Vector3::DOWN * SIZE_Y + Vector3::FORWARD * SIZE_Z));
//    positions.Push(Vector3(position + Vector3::DOWN * SIZE_Y + Vector3::BACK * SIZE_Z));
//    positions.Push(Vector3(position + Vector3::DOWN * SIZE_Y));
//
//    positions.Push(Vector3(position + Vector3::DOWN * SIZE_Y + Vector3::BACK * SIZE_Z + Vector3::LEFT * SIZE_X));
//    positions.Push(Vector3(position + Vector3::DOWN * SIZE_Y + Vector3::BACK * SIZE_Z + Vector3::RIGHT * SIZE_X));
//    positions.Push(Vector3(position + Vector3::DOWN * SIZE_Y + Vector3::FORWARD * SIZE_Z + Vector3::LEFT * SIZE_X));
//    positions.Push(Vector3(position + Vector3::DOWN * SIZE_Y + Vector3::FORWARD * SIZE_Z + Vector3::RIGHT * SIZE_X));

    for (auto it = positions.Begin(); it != positions.End(); ++it) {
        if(!IsChunkLoaded((*it)) && !IsChunkPending((*it))) {
            pendingChunks_.Push((*it));
            URHO3D_LOGINFO("Loading neighbor chunk " + (*it).ToString());
        }
    }
}

bool VoxelWorld::IsChunkPending(Vector3 position)
{
    for (auto it = pendingChunks_.Begin(); it != pendingChunks_.End(); ++it) {
        if (IsEqualPositions((*it), position)) {
            return true;
        }
    }

    return false;
}

bool VoxelWorld::IsEqualPositions(Vector3 a, Vector3 b)
{
    return Floor(a.x_) == Floor(b.x_) && Floor(a.y_) == Floor(b.y_) && Floor(a.z_) == Floor(b.z_);
}

SharedPtr<Chunk> VoxelWorld::GetChunkByPosition(const Vector3& position)
{
    for (auto it = chunks_.Begin(); it != chunks_.End(); ++it) {
        if ((*it)->IsPointInsideChunk(position)) {
            return (*it);
        }
    }

    return nullptr;
}

Vector3 VoxelWorld::GetWorldToChunkPosition(Vector3& position)
{
    return Vector3(
            Floor(position.x_ / SIZE_X) * SIZE_X,
//        Floor(position.y_ / SIZE_Y) * SIZE_Y,
            -SIZE_Y - 2,
            Floor(position.z_ / SIZE_Z) * SIZE_Z
    );
}

void VoxelWorld::RemoveBlockAtPosition(const Vector3& position)
{
    removeBlocks_.Push(position);
}