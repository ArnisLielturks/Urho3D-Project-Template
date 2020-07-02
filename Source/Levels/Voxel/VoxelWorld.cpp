#include <Urho3D/IO/Log.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/DebugHud.h>
#include <Urho3D/Math/Ray.h>
#include <Urho3D/Container/Vector.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/IO/FileSystem.h>
#include "VoxelWorld.h"
#include "../../SceneManager.h"
#include "VoxelEvents.h"
#include "../../Console/ConsoleHandlerEvents.h"
#include "ChunkGenerator.h"
#include "../../Global.h"
#include "LightManager.h"
#include "TreeGenerator.h"

using namespace VoxelEvents;
using namespace ConsoleHandlerEvents;

int VoxelWorld::visibleDistance = 1;
int VoxelWorld::activeDistance = 1;


void UpdateChunkState(const WorkItem* item, unsigned threadIndex)
{
    Timer loadTime;
    VoxelWorld* world = reinterpret_cast<VoxelWorld*>(item->aux_);
    MutexLock lock(world->mutex_);
    if (world->GetSubsystem<LightManager>()) {
        world->GetSubsystem<LightManager>()->Process();
    }
    if (world->GetSubsystem<TreeGenerator>()) {
        world->GetSubsystem<TreeGenerator>()->Process();
    }
    if (world->GetSubsystem<DebugHud>()) {
        world->GetSubsystem<DebugHud>()->SetAppStats("Chunks Loaded", world->chunks_.Size());
    }

    int counter = 0;
    for (auto chIt = world->chunks_.Begin(); chIt != world->chunks_.End(); ++chIt) {
        if ((*chIt).second_ && (*chIt).second_->IsActive()) {
            counter++;
        }
    }
    if (world->GetSubsystem<DebugHud>()) {
        world->GetSubsystem<DebugHud>()->SetAppStats("Active chunks", counter);
    }

    float activeBlockDistance = SIZE_X * ( world->activeDistance + 2);
    float visibleBlockDistance = SIZE_X * ( world->visibleDistance + 2);

    for (auto chIt = world->chunks_.Begin(); chIt != world->chunks_.End(); ++chIt) {
        for (auto obIt = world->observers_.Begin(); obIt != world->observers_.End(); ++obIt) {
            if (!(*chIt).second_) {
                continue;
            }
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

    for (auto it = world->chunks_.Begin(); it != world->chunks_.End(); ++it) {
        if (!(*it).second_) {
            continue;
        }

        if (world->reloadAllChunks_) {
            (*it).second_->MarkForGeometryCalculation();
        }
        // Initialize new chunks
        if (!(*it).second_->IsLoaded()) {
            (*it).second_->Load();
            for (int i = 0; i < 6; i++) {
                auto neighbor = (*it).second_->GetNeighbor(static_cast<BlockSide>(i));
                if (neighbor) {
                    neighbor->MarkForGeometryCalculation();
                }
            }
        }

        if (!(*it).second_->IsGeometryCalculated()) {
            (*it).second_->CalculateGeometry();
//            URHO3D_LOGINFO("CalculateGeometry " + (*it).second_->GetPosition().ToString());
        }
    }

    world->reloadAllChunks_ = false;
//    URHO3D_LOGINFO("Chunks updated in " + String(loadTime.GetMSec(false)) + "ms");
}

VoxelWorld::VoxelWorld(Context* context):
    Object(context)
{
}

void VoxelWorld::Init()
{
    scene_ = GetSubsystem<SceneManager>()->GetActiveScene();

    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(VoxelWorld, HandleUpdate));
    SubscribeToEvent(E_CHUNK_ENTERED, URHO3D_HANDLER(VoxelWorld, HandleChunkEntered));
    SubscribeToEvent(E_CHUNK_EXITED, URHO3D_HANDLER(VoxelWorld, HandleChunkExited));
    SubscribeToEvent(E_CHUNK_GENERATED, URHO3D_HANDLER(VoxelWorld, HandleChunkGenerated));
    SubscribeToEvent(E_WORKITEMCOMPLETED, URHO3D_HANDLER(VoxelWorld, HandleWorkItemFinished));

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

    SendEvent(
            E_CONSOLE_COMMAND_ADD,
            ConsoleCommandAdd::P_NAME, "world_reset",
            ConsoleCommandAdd::P_EVENT, "#world_reset",
            ConsoleCommandAdd::P_DESCRIPTION, "Remove saved chunks",
            ConsoleCommandAdd::P_OVERWRITE, true
    );
    SubscribeToEvent("#world_reset", [&](StringHash eventType, VariantMap& eventData) {
        StringVector params = eventData["Parameters"].GetStringVector();
        if (params.Size() != 1) {
            URHO3D_LOGERROR("Thi command doesn't have any arguments!");
            return;
        }
        if(GetSubsystem<FileSystem>()->DirExists("World")) {
            Vector<String> files;
            GetSubsystem<FileSystem>()->ScanDir(files, "World", "", SCAN_FILES, false);
            for (auto it = files.Begin(); it != files.End(); ++it) {
                URHO3D_LOGINFO("Deleting file " + (*it));
                GetSubsystem<FileSystem>()->Delete("World/" + (*it));
            }
        }
    });

    SendEvent(
            E_CONSOLE_COMMAND_ADD,
            ConsoleCommandAdd::P_NAME, "sunlight",
            ConsoleCommandAdd::P_EVENT, "#sunlight",
            ConsoleCommandAdd::P_DESCRIPTION, "Set sunlight level [0-15]",
            ConsoleCommandAdd::P_OVERWRITE, true
    );
    SubscribeToEvent("#sunlight", [&](StringHash eventType, VariantMap& eventData) {
        StringVector params = eventData["Parameters"].GetStringVector();
        if (params.Size() != 2) {
            URHO3D_LOGERROR("Thi command requires exactly 1 argument!");
            return;
        }
        int level = ToInt(params[1]);
        Chunk::sunlightLevel = level;
        reloadAllChunks_ = true;
    });
}

void VoxelWorld::RegisterObject(Context* context)
{
    context->RegisterFactory<VoxelWorld>();
}

void VoxelWorld::AddObserver(SharedPtr<Node> observer)
{
    observers_.Push(observer);
    URHO3D_LOGINFO("Adding observer to voxel world!");
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
//    if (sunlightTimer_.GetMSec(false) > 2000) {
//        Chunk::sunlightLevel++;
//        if (Chunk::sunlightLevel > 15) {
//            Chunk::sunlightLevel = 0;
//        }
//        reloadAllChunks_ = true;
//        sunlightTimer_.Reset();
//    }
    int loadedChunkCounter = 0;
//    while (!pendingChunks_.Empty()) {
//        loadedChunkCounter++;
//        String id = GetChunkIdentificator(pendingChunks_.Front()->GetPosition());
//        chunks_[id] = pendingChunks_.Front();
//        chunks_[id]->Generate();
//        pendingChunks_.PopFront();
//        if (loadedChunkCounter > loadChunksPerFrame_) {
//            break;
//        }
//    }

    for (auto it = observers_.Begin(); it != observers_.End(); ++it) {
        Vector3 fixedPosition = GetWorldToChunkPosition((*it)->GetPosition());
        LoadChunk(fixedPosition);
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

//    if (updateTimer_.GetMSec(false) > 10) {
//        updateTimer_.Reset();
    UpdateChunks();
//    }
}

void VoxelWorld::CreateChunk(const Vector3& position)
{
    String id = GetChunkIdentificator(position);
    chunks_[id] = new Chunk(context_);
    chunks_[id]->Init(scene_, position);
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
//    LoadChunk(position);
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
    if (!IsChunkLoaded(fixedChunkPosition)) {
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
        if(!IsChunkLoaded((*it))) {
            CreateChunk((*it));
        }
    }
}

bool VoxelWorld::IsEqualPositions(Vector3 a, Vector3 b)
{
    return Floor(a.x_) == Floor(b.x_) && Floor(a.y_) == Floor(b.y_) && Floor(a.z_) == Floor(b.z_);
}

Chunk* VoxelWorld::GetChunkByPosition(const Vector3& position)
{
    Vector3 fixedPositon = GetWorldToChunkPosition(position);
    String id = GetChunkIdentificator(fixedPositon);
    if (chunks_.Find(id) != chunks_.End() && chunks_[id]) {
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

    int deleteCount = 0;
    for (auto it = chunks_.Begin(); it != chunks_.End(); ++it) {
        if ((*it).second_) {
            if ((*it).second_->IsMarkedForDeletion()) {
                deleteCount++;
            }
        }
    }

    if (deleteCount > 0) {
        MutexLock lock(mutex_);
        for (auto it = chunks_.Begin(); it != chunks_.End(); ++it) {
            if ((*it).second_) {
                if ((*it).second_->IsMarkedForDeletion()) {
                    it = chunks_.Erase(it);
                }
                if (chunks_.End() == it) {
                    break;
                }
                (*it).second_->SetActive();
            }
        }
    }

    if (!updateWorkItem_) {
        WorkQueue *workQueue = GetSubsystem<WorkQueue>();
        updateWorkItem_ = workQueue->GetFreeItem();
        updateWorkItem_->priority_ = M_MAX_INT;
        updateWorkItem_->workFunction_ = UpdateChunkState;
        updateWorkItem_->aux_ = this;
        updateWorkItem_->sendEvent_ = true;
        updateWorkItem_->start_ = nullptr;
        updateWorkItem_->end_ = nullptr;
        workQueue->AddWorkItem(updateWorkItem_);
    }

    int renderedChunkCount = 0;
    int renderedChunkLimit = 4;
    for (auto it = chunks_.Begin(); it != chunks_.End(); ++it) {
        if ((*it).second_->ShouldRender()) {
            bool rendered = (*it).second_->Render();
//            URHO3D_LOGINFO("Rendering chunk " + (*it).second_->GetPosition().ToString());
            if (rendered) {
                renderedChunkCount++;
            }
            if (renderedChunkCount > renderedChunkLimit) {
                break;
            }
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

bool VoxelWorld::IsChunkValid(Chunk* chunk)
{
    for (auto it = chunks_.Begin(); it != chunks_.End(); ++it) {
        if ((*it).second_.Get() == chunk) {
            return true;
        }
    }

    return false;
}

void VoxelWorld::HandleWorkItemFinished(StringHash eventType, VariantMap& eventData) {
    using namespace WorkItemCompleted;
    WorkItem *workItem = reinterpret_cast<WorkItem *>(eventData[P_ITEM].GetPtr());
    if (workItem->aux_ != this) {
        return;
    }
//    if (workItem->workFunction_ == GenerateVoxelData) {
//        CreateNode();
//        generateWorkItem_.Reset();
//        MarkDirty();
//    } else if (workItem->workFunction_ == GenerateVertices) {
////        GetSubsystem<VoxelWorld>()->AddChunkToRenderQueue(this);
//        Render();
//        generateGeometryWorkItem_.Reset();
//    } else
    if (workItem->workFunction_ == UpdateChunkState) {
        updateWorkItem_.Reset();
    }
}

const String VoxelWorld::GetBlockName(BlockType type)
{
    switch (type) {
        case BT_AIR:
            return "BT_AIR";
        case BT_STONE:
            return "BT_STONE";
        case BT_DIRT:
            return "BT_DIRT";
        case BT_SAND:
            return "BT_SAND";
        case BT_COAL:
            return "BT_COAL";
        case BT_TORCH:
            return "BT_TORCH";
        case BT_WOOD:
            return "BT_WOOD";
        case BT_TREE_LEAVES:
            return "BT_TREE_LEAVES";
        case BT_WATER:
            return "BT_WATER";
        default:
            return "BT_NONE";
    }
}