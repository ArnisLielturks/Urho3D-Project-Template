#include <Urho3D/IO/Log.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/DebugHud.h>
#include "VoxelWorld.h"
#include "../../SceneManager.h"
#include "VoxelEvents.h"
#include "ChunkGenerator.h"

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
    if (!pendingChunks_.Empty() && updateTimer_.GetMSec(false) > 1) {
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

    if (cleanupTimer_.GetMSec(false) > 50) {
        cleanupTimer_.Reset();
        UpdateChunks();
    }
}

void VoxelWorld::CreateChunk(const Vector3& position)
{
    String id = GetChunkIdentificator(position);
    chunks_[id] = SharedPtr<Chunk>(new Chunk(context_));
    ChunkType type;
    if (position.y_ == -SIZE_Y) {
        type = ChunkType::TERRAIN;
    } else if (position.y_ > -SIZE_Y) {
        type = ChunkType::SKY;
    } else {
        type = ChunkType::GROUND;
    }
    chunks_[id]->Init(1, scene_, position, type);
}

Vector3 VoxelWorld::GetNodeToChunkPosition(Node* node)
{
    Vector3 position = node->GetWorldPosition();
    return GetWorldToChunkPosition(position);
}

bool VoxelWorld::IsChunkLoaded(const Vector3& position)
{
    String id = GetChunkIdentificator(position);
    if (chunks_.Contains(id) && chunks_[id]) {
        return true;
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

void VoxelWorld::LoadChunk(const Vector3& position, bool loadImmediately)
{
//    position.y_ = -SIZE_Y;
    if (!IsChunkLoaded(position) && !IsChunkPending(position)) {
        if (loadImmediately) {
            CreateChunk(position);
        } else {
            pendingChunks_.Push(position);
        }
//        URHO3D_LOGINFO("Loading chunk" + position.ToString());
    }
    Vector<Vector3> positions;


    //Terrain blocks only
//    for (int x = -5; x < 5; x++) {
//        for (int z = -5; z < 5; z++) {
//            Vector3 terrain = Vector3(position + Vector3::FORWARD * SIZE_Z * z + Vector3::LEFT * SIZE_X * x);
//            terrain.y_ = GetSubsystem<ChunkGenerator>()->GetTerrainHeight(terrain);
//            positions.Push(terrain);
////            positions.Push(terrain + Vector3::UP * SIZE_Y);
//            positions.Push(terrain + Vector3::DOWN * SIZE_Y);
//        }
//    }
//    pq2ws

    // Same
    positions.Push(Vector3(position + Vector3::LEFT * SIZE_X));
    positions.Push(Vector3(position + Vector3::RIGHT * SIZE_X));
    positions.Push(Vector3(position + Vector3::FORWARD * SIZE_Z));
    positions.Push(Vector3(position + Vector3::BACK * SIZE_Z));

    positions.Push(Vector3(position + Vector3::BACK * SIZE_Z + Vector3::LEFT * SIZE_X));
    positions.Push(Vector3(position + Vector3::BACK * SIZE_Z + Vector3::RIGHT * SIZE_X));
    positions.Push(Vector3(position + Vector3::FORWARD * SIZE_Z + Vector3::LEFT * SIZE_X));
    positions.Push(Vector3(position + Vector3::FORWARD * SIZE_Z + Vector3::RIGHT * SIZE_X));

    // Up
    positions.Push(Vector3(position + Vector3::UP * SIZE_Y + Vector3::LEFT * SIZE_X));
    positions.Push(Vector3(position + Vector3::UP * SIZE_Y + Vector3::RIGHT * SIZE_X));
    positions.Push(Vector3(position + Vector3::UP * SIZE_Y + Vector3::FORWARD * SIZE_Z));
    positions.Push(Vector3(position + Vector3::UP * SIZE_Y + Vector3::BACK * SIZE_Z));
    positions.Push(Vector3(position + Vector3::UP * SIZE_Y));

    positions.Push(Vector3(position + Vector3::UP * SIZE_Y + Vector3::BACK * SIZE_Z + Vector3::LEFT * SIZE_X));
    positions.Push(Vector3(position + Vector3::UP * SIZE_Y + Vector3::BACK * SIZE_Z + Vector3::RIGHT * SIZE_X));
    positions.Push(Vector3(position + Vector3::UP * SIZE_Y + Vector3::FORWARD * SIZE_Z + Vector3::LEFT * SIZE_X));
    positions.Push(Vector3(position + Vector3::UP * SIZE_Y + Vector3::FORWARD * SIZE_Z + Vector3::RIGHT * SIZE_X));

    // Down
    positions.Push(Vector3(position + Vector3::DOWN * SIZE_Y + Vector3::LEFT * SIZE_X));
    positions.Push(Vector3(position + Vector3::DOWN * SIZE_Y + Vector3::RIGHT * SIZE_X));
    positions.Push(Vector3(position + Vector3::DOWN * SIZE_Y + Vector3::FORWARD * SIZE_Z));
    positions.Push(Vector3(position + Vector3::DOWN * SIZE_Y + Vector3::BACK * SIZE_Z));
    positions.Push(Vector3(position + Vector3::DOWN * SIZE_Y));

    positions.Push(Vector3(position + Vector3::DOWN * SIZE_Y + Vector3::BACK * SIZE_Z + Vector3::LEFT * SIZE_X));
    positions.Push(Vector3(position + Vector3::DOWN * SIZE_Y + Vector3::BACK * SIZE_Z + Vector3::RIGHT * SIZE_X));
    positions.Push(Vector3(position + Vector3::DOWN * SIZE_Y + Vector3::FORWARD * SIZE_Z + Vector3::LEFT * SIZE_X));
    positions.Push(Vector3(position + Vector3::DOWN * SIZE_Y + Vector3::FORWARD * SIZE_Z + Vector3::RIGHT * SIZE_X));

    for (auto it = positions.Begin(); it != positions.End(); ++it) {
        if(!IsChunkLoaded((*it)) && !IsChunkPending((*it))) {
//            URHO3D_LOGINFO("Loading neighbor chunk " + (*it).ToString());
            if (loadImmediately) {
                CreateChunk((*it));
            } else {
                pendingChunks_.Push((*it));
            }
        }
    }
}

bool VoxelWorld::IsChunkPending(const Vector3& position)
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
    if (IsChunkLoaded(position)) {
        return chunks_[GetChunkIdentificator(position)];
    }

    return nullptr;
//    for (auto it = chunks_.Begin(); it != chunks_.End(); ++it) {
//        if ((*it)->IsPointInsideChunk(position)) {
//            return (*it);
//        }
//    }
//
//    return nullptr;
}

Vector3 VoxelWorld::GetWorldToChunkPosition(const Vector3& position)
{
    return Vector3(
            Floor(position.x_ / SIZE_X) * SIZE_X,
        Floor(position.y_ / SIZE_Y) * SIZE_Y,
            Floor(position.z_ / SIZE_Z) * SIZE_Z
    );
}

void VoxelWorld::RemoveBlockAtPosition(const Vector3& position)
{
    removeBlocks_.Push(position);
}


void VoxelWorld::UpdateChunks()
{
    int counter = 0;
    for (auto chIt = chunks_.Begin(); chIt != chunks_.End(); ++chIt) {
        if ((*chIt).second_ && (*chIt).second_->IsActive()) {
            counter++;
        }
    }
    if (GetSubsystem<DebugHud>()) {
        GetSubsystem<DebugHud>()->SetAppStats("Active chunks", counter);
    }

    float activeDistance = SIZE_X * 2;
    float visibleDistance = SIZE_X * 5;
    float maxActiveDistanceSquared = activeDistance * activeDistance;
    float maxVisibleDistanceSquared = visibleDistance * visibleDistance;
    for (auto chIt = chunks_.Begin(); chIt != chunks_.End(); ++chIt) {
        if ((*chIt).second_) {
            (*chIt).second_->MarkForDeletion(true);
            (*chIt).second_->MarkActive(false);
        }
    }
    for (auto chIt = chunks_.Begin(); chIt != chunks_.End(); ++chIt) {
        for (auto obIt = observers_.Begin(); obIt != observers_.End(); ++obIt) {
            if (!(*chIt).second_) {
                return;
            }
            Vector3 chunkPosition = (*chIt).second_->GetPosition();
            float distance = ((*obIt)->GetWorldPosition() - (*chIt).second_->GetPosition()).LengthSquared();
            if (distance < maxVisibleDistanceSquared) {
                (*chIt).second_->MarkForDeletion(false);
            }
            if (distance < maxActiveDistanceSquared) {
                (*chIt).second_->MarkActive(true);
            }
        }
    }

    for (auto chIt = chunks_.Begin(); chIt != chunks_.End(); ++chIt) {
        if ((*chIt).second_) {
            if ((*chIt).second_->IsMarkedForDeletion()) {
                chunks_.Erase(chIt);
                return;
            }
            (*chIt).second_->SetActive();
        }
    }
}

String VoxelWorld::GetChunkIdentificator(const Vector3& position)
{

    return String(position.x_ / SIZE_X) + "_" +  String(position.y_ / SIZE_Y) + "_" + String(position.z_ / SIZE_Z);
}
