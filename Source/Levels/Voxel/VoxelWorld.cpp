#include <Urho3D/IO/Log.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/DebugHud.h>
#include <Urho3D/Math/Ray.h>
#include <Urho3D/Container/Vector.h>
#include <Urho3D/Graphics/Octree.h>
#include "VoxelWorld.h"
#include "../../SceneManager.h"
#include "VoxelEvents.h"
#include "../../Console/ConsoleHandlerEvents.h"
#include "ChunkGenerator.h"
#include "../../Global.h"

using namespace VoxelEvents;
using namespace ConsoleHandlerEvents;

int VoxelWorld::visibleDistance = 1;
int VoxelWorld::activeDistance = 1;

VoxelWorld::VoxelWorld(Context* context):
    Object(context)
{
    scene_ = GetSubsystem<SceneManager>()->GetActiveScene();

    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(VoxelWorld, HandleUpdate));
    SubscribeToEvent(E_CHUNK_ENTERED, URHO3D_HANDLER(VoxelWorld, HandleChunkEntered));
    SubscribeToEvent(E_CHUNK_EXITED, URHO3D_HANDLER(VoxelWorld, HandleChunkExited));
    SubscribeToEvent(E_CHUNK_GENERATED, URHO3D_HANDLER(VoxelWorld, HandleChunkGenerated));

    SendEvent(
            E_CONSOLE_COMMAND_ADD,
            ConsoleCommandAdd::P_NAME, "chunk_visible_distance",
            ConsoleCommandAdd::P_EVENT, "#chunk_visible_distance",
            ConsoleCommandAdd::P_DESCRIPTION, "How far away the generated chunks are visible",
            ConsoleCommandAdd::P_OVERWRITE, true
    );
    SubscribeToEvent("#chunk_visible_distance", [&](StringHash eventType, VariantMap& eventData) {
        StringVector params = eventData["Parameters"].GetStringVector();
        if (params.Size() != 2) {
            URHO3D_LOGERROR("radius parameter is required!");
            return;
        }
        int value = ToInt(params[1]);
        VoxelWorld::visibleDistance = value;
        URHO3D_LOGINFOF("Changing chunk visibility radius to %d", value);
    });

    SendEvent(
            E_CONSOLE_COMMAND_ADD,
            ConsoleCommandAdd::P_NAME, "chunk_active_distance",
            ConsoleCommandAdd::P_EVENT, "#chunk_active_distance",
            ConsoleCommandAdd::P_DESCRIPTION, "How far aways the chunks are active",
            ConsoleCommandAdd::P_OVERWRITE, true
    );
    SubscribeToEvent("#chunk_active_distance", [&](StringHash eventType, VariantMap& eventData) {
        StringVector params = eventData["Parameters"].GetStringVector();
        if (params.Size() != 2) {
            URHO3D_LOGERROR("radius parameter is required!");
            return;
        }
        int value = ToInt(params[1]);
        VoxelWorld::activeDistance = value;
        URHO3D_LOGINFOF("Changing chunk active radius to %d", value);
    });

    SendEvent(
            E_CONSOLE_COMMAND_ADD,
            ConsoleCommandAdd::P_NAME, "chunk_load_per_frame",
            ConsoleCommandAdd::P_EVENT, "#chunk_load_per_frame",
            ConsoleCommandAdd::P_DESCRIPTION, "How many chunks to attempt to load in a single frame",
            ConsoleCommandAdd::P_OVERWRITE, true
    );
    SubscribeToEvent("#chunk_load_per_frame", [&](StringHash eventType, VariantMap& eventData) {
        StringVector params = eventData["Parameters"].GetStringVector();
        if (params.Size() != 2) {
            URHO3D_LOGERROR("amount parameter is required!");
            return;
        }
        int value = ToInt(params[1]);
        loadChunksPerFrame_ = value;
        URHO3D_LOGINFOF("Chunks loaded per frame change to %d", value);
    });

//    SendEvent(
//            E_CONSOLE_COMMAND_ADD,
//            ConsoleCommandAdd::P_NAME, "chunk_visibility_update",
//            ConsoleCommandAdd::P_EVENT, "#chunk_visibility_update",
//            ConsoleCommandAdd::P_DESCRIPTION, "Hide chunks when they are not visible",
//            ConsoleCommandAdd::P_OVERWRITE, true
//    );
//    SubscribeToEvent("#chunk_visibility_update", [&](StringHash eventType, VariantMap& eventData) {
//        StringVector params = eventData["Parameters"].GetStringVector();
//        if (params.Size() != 2) {
//            URHO3D_LOGERROR("visiblity parameter is required!");
//            return;
//        }
//        int value = ToBool(params[1]);
//        Chunk::visibilityUpdate_ = value;
//        URHO3D_LOGINFOF("Chunk visiblitity updated to %d", value);
//    });
}

void VoxelWorld::RegisterObject(Context* context)
{
    context->RegisterFactory<VoxelWorld>();
}

void VoxelWorld::AddObserver(SharedPtr<Node> observer)
{
    observers_.Push(observer);
    URHO3D_LOGINFO("Adding observer to voxel world!");
    LoadChunk(observer->GetWorldPosition());
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
    int loadedChunkCounter = 0;
    while (!pendingChunks_.Empty()) {
        loadedChunkCounter++;
        String id = GetChunkIdentificator(pendingChunks_.Front()->GetPosition());
        chunks_[id] = pendingChunks_.Front();
        chunks_[id]->Generate();
        chunks_[id]->RenderNeighbors();
        pendingChunks_.PopFront();
        if (loadedChunkCounter > loadChunksPerFrame_) {
            break;
        }
    }

    if (!removeBlocks_.Empty()) {
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

    if (updateTimer_.GetMSec(false) > 10) {
        updateTimer_.Reset();
        UpdateChunks();
    }
}

void VoxelWorld::CreateChunk(const Vector3& position)
{
    String id = GetChunkIdentificator(position);
    pendingChunks_.Push(new Chunk(context_));
    pendingChunks_.Back()->Init(scene_, position);
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

void VoxelWorld::HandleChunkGenerated(StringHash eventType, VariantMap& eventData)
{
}

void VoxelWorld::LoadChunk(const Vector3& position)
{
    Vector3 fixedChunkPosition = GetWorldToChunkPosition(position);
    if (!IsChunkLoaded(fixedChunkPosition) && !IsChunkPending(fixedChunkPosition)) {
        CreateChunk(fixedChunkPosition);
    }
    Vector<Vector3> positions;


    //Terrain blocks only
//    for (int x = -5; x < 5; x++) {
//        for (int z = -5; z < 5; z++) {
//            Vector3 terrain = Vector3(fixedChunkPosition + Vector3::FORWARD * SIZE_Z * z + Vector3::LEFT * SIZE_X * x);
//            terrain.y_ = GetSubsystem<ChunkGenerator>()->GetTerrainHeight(terrain);
//            positions.Push(terrain);
////            positions.Push(terrain + Vector3::UP * SIZE_Y);
//            positions.Push(terrain + Vector3::DOWN * SIZE_Y);
//        }
//    }

    // Same
    positions.Push(Vector3(fixedChunkPosition + Vector3::LEFT * SIZE_X));
    positions.Push(Vector3(fixedChunkPosition + Vector3::RIGHT * SIZE_X));
    positions.Push(Vector3(fixedChunkPosition + Vector3::FORWARD * SIZE_Z));
    positions.Push(Vector3(fixedChunkPosition + Vector3::BACK * SIZE_Z));

    positions.Push(Vector3(fixedChunkPosition + Vector3::BACK * SIZE_Z + Vector3::LEFT * SIZE_X));
    positions.Push(Vector3(fixedChunkPosition + Vector3::BACK * SIZE_Z + Vector3::RIGHT * SIZE_X));
    positions.Push(Vector3(fixedChunkPosition + Vector3::FORWARD * SIZE_Z + Vector3::LEFT * SIZE_X));
    positions.Push(Vector3(fixedChunkPosition + Vector3::FORWARD * SIZE_Z + Vector3::RIGHT * SIZE_X));

    // Up
    positions.Push(Vector3(fixedChunkPosition + Vector3::UP * SIZE_Y + Vector3::LEFT * SIZE_X));
    positions.Push(Vector3(fixedChunkPosition + Vector3::UP * SIZE_Y + Vector3::RIGHT * SIZE_X));
    positions.Push(Vector3(fixedChunkPosition + Vector3::UP * SIZE_Y + Vector3::FORWARD * SIZE_Z));
    positions.Push(Vector3(fixedChunkPosition + Vector3::UP * SIZE_Y + Vector3::BACK * SIZE_Z));
    positions.Push(Vector3(fixedChunkPosition + Vector3::UP * SIZE_Y));

    positions.Push(Vector3(fixedChunkPosition + Vector3::UP * SIZE_Y + Vector3::BACK * SIZE_Z + Vector3::LEFT * SIZE_X));
    positions.Push(Vector3(fixedChunkPosition + Vector3::UP * SIZE_Y + Vector3::BACK * SIZE_Z + Vector3::RIGHT * SIZE_X));
    positions.Push(Vector3(fixedChunkPosition + Vector3::UP * SIZE_Y + Vector3::FORWARD * SIZE_Z + Vector3::LEFT * SIZE_X));
    positions.Push(Vector3(fixedChunkPosition + Vector3::UP * SIZE_Y + Vector3::FORWARD * SIZE_Z + Vector3::RIGHT * SIZE_X));

    // Down
    positions.Push(Vector3(fixedChunkPosition + Vector3::DOWN * SIZE_Y + Vector3::LEFT * SIZE_X));
    positions.Push(Vector3(fixedChunkPosition + Vector3::DOWN * SIZE_Y + Vector3::RIGHT * SIZE_X));
    positions.Push(Vector3(fixedChunkPosition + Vector3::DOWN * SIZE_Y + Vector3::FORWARD * SIZE_Z));
    positions.Push(Vector3(fixedChunkPosition + Vector3::DOWN * SIZE_Y + Vector3::BACK * SIZE_Z));
    positions.Push(Vector3(fixedChunkPosition + Vector3::DOWN * SIZE_Y));

    positions.Push(Vector3(fixedChunkPosition + Vector3::DOWN * SIZE_Y + Vector3::BACK * SIZE_Z + Vector3::LEFT * SIZE_X));
    positions.Push(Vector3(fixedChunkPosition + Vector3::DOWN * SIZE_Y + Vector3::BACK * SIZE_Z + Vector3::RIGHT * SIZE_X));
    positions.Push(Vector3(fixedChunkPosition + Vector3::DOWN * SIZE_Y + Vector3::FORWARD * SIZE_Z + Vector3::LEFT * SIZE_X));
    positions.Push(Vector3(fixedChunkPosition + Vector3::DOWN * SIZE_Y + Vector3::FORWARD * SIZE_Z + Vector3::RIGHT * SIZE_X));

    for (auto it = positions.Begin(); it != positions.End(); ++it) {
        if(!IsChunkLoaded((*it)) && !IsChunkPending((*it))) {
            CreateChunk((*it));
        }
    }
}

bool VoxelWorld::IsChunkPending(const Vector3& position)
{
    for (auto it = pendingChunks_.Begin(); it != pendingChunks_.End(); ++it) {
        if (IsEqualPositions((*it)->GetPosition(), position)) {
            return true;
        }
    }

    return false;
}

bool VoxelWorld::IsEqualPositions(Vector3 a, Vector3 b)
{
    return Floor(a.x_) == Floor(b.x_) && Floor(a.y_) == Floor(b.y_) && Floor(a.z_) == Floor(b.z_);
}

Chunk* VoxelWorld::GetChunkByPosition(const Vector3& position)
{
    Vector3 fixedPositon = GetWorldToChunkPosition(position);
    String id = GetChunkIdentificator(fixedPositon);
    if (chunks_.Contains(id) && chunks_[id]) {
        return chunks_[id].Get();
    }

    return nullptr;
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
    if (chunks_.Empty()) {
        URHO3D_LOGWARNING("All chunks were destroyed, regenerating chunks around players!");
        // In case player moved to quickly trough the world and the chunks
        // were not loading as fast
        for (auto it = observers_.Begin(); it != observers_.End(); ++it) {
            LoadChunk((*it)->GetPosition());
        }
    }

    if (GetSubsystem<DebugHud>()) {
        GetSubsystem<DebugHud>()->SetAppStats("Chunks Loaded", chunks_.Size());
    }
    if (GetSubsystem<DebugHud>()) {
        GetSubsystem<DebugHud>()->SetAppStats("PendingChuml", chunks_.Size());
    }

    int counter = 0;
    for (auto chIt = chunks_.Begin(); chIt != chunks_.End(); ++chIt) {
        if ((*chIt).second_ && (*chIt).second_->IsActive()) {
            counter++;
        }
    }
    if (GetSubsystem<DebugHud>()) {
        GetSubsystem<DebugHud>()->SetAppStats("Active chunks", counter);
    }

    float activeBlockDistance = SIZE_X * ( activeDistance + 2);
    float visibleBlockDistance = SIZE_X * ( visibleDistance + 2);

    for (auto chIt = chunks_.Begin(); chIt != chunks_.End(); ++chIt) {
        for (auto obIt = observers_.Begin(); obIt != observers_.End(); ++obIt) {
            if (!(*chIt).second_) {
                continue;
            }
//            if ((*chIt).second_->IsVisible()) {
//                visibleChunks++;
//            }
            const Vector3& chunkPosition = (*chIt).second_->GetPosition();
            auto distance = (*obIt)->GetWorldPosition() - chunkPosition;
            distance.x_ = Abs(distance.x_);
            distance.y_ = Abs(distance.y_);
            distance.z_ = Abs(distance.z_);

            if (distance.x_ <= visibleBlockDistance && distance.y_ <= visibleBlockDistance && distance.z_ <= visibleBlockDistance) {
                (*chIt).second_->MarkForDeletion(false);
            } else {
                (*chIt).second_->MarkForDeletion(true);
            }
            if (distance.x_ <= activeBlockDistance && distance.y_ <= activeBlockDistance && distance.z_ <= activeBlockDistance) {
                (*chIt).second_->MarkActive(true);
            } else {
                (*chIt).second_->MarkActive(false);
            }
        }
    }

//    if (GetSubsystem<DebugHud>()) {
//        GetSubsystem<DebugHud>()->SetAppStats("Visible chunks", visibleChunks);
//    }

    for (auto it = pendingChunks_.Begin(); it != pendingChunks_.End(); ++it) {
        bool visible = false;
        for (auto obIt = observers_.Begin(); obIt != observers_.End(); ++obIt) {
            const Vector3& chunkPosition = (*it)->GetPosition();
            auto distance = (*obIt)->GetWorldPosition() - chunkPosition;
            distance.x_ = Abs(distance.x_);
            distance.y_ = Abs(distance.y_);
            distance.z_ = Abs(distance.z_);

            if (distance.x_ <= visibleBlockDistance && distance.y_ <= visibleBlockDistance && distance.z_ <= visibleBlockDistance) {
                visible = true;
                break;
            }
        }
        if (!visible) {
            delete (*it);
            it = pendingChunks_.Erase(it);
            if (it == pendingChunks_.End()) {
                break;
            }
        }
    }

    for (auto chIt = chunks_.Begin(); chIt != chunks_.End(); ++chIt) {
        if ((*chIt).second_) {
            if ((*chIt).second_->IsMarkedForDeletion()) {
                chIt = chunks_.Erase(chIt);
            }
            if (chunks_.End() == chIt) {
                break;
            }
            (*chIt).second_->SetActive();
//            (*chIt).second_->UpdateVisibility();
        }
    }
}

String VoxelWorld::GetChunkIdentificator(const Vector3& position)
{

    return String(position.x_ / SIZE_X) + "_" +  String(position.y_ / SIZE_Y) + "_" + String(position.z_ / SIZE_Z);
}

VoxelBlock* VoxelWorld::GetBlockAt(Vector3 position)
{
    Vector3 chunkPosition = GetWorldToChunkPosition(position);
    String id = GetChunkIdentificator(chunkPosition);
    if (chunks_.Contains(id) && chunks_[id]) {
        Vector3 blockPosition = position - chunkPosition;
        return chunks_[id]->GetBlockAt(IntVector3(blockPosition.x_, blockPosition.y_, blockPosition.z_));
    }
    return nullptr;
}