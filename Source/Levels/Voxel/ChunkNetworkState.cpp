#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Serializable.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/UI/Text3D.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include "ChunkNetworkState.h"
#include "../../Global.h"
#include "Chunk.h"
#include "VoxelEvents.h"

using namespace VoxelEvents;

ChunkNetworkState::ChunkNetworkState(Context* context) :
        Component(context)
{
    chunkData_.Resize(SIZE_X * SIZE_Y * SIZE_Z);
}

ChunkNetworkState::~ChunkNetworkState()
{
}

void ChunkNetworkState::RegisterObject(Context* context)
{
    context->RegisterFactory<ChunkNetworkState>();
    URHO3D_ACCESSOR_ATTRIBUTE("LatestChangeID", GetLatestChangeID, SetLatestChangeID, int, 0, AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE("ChunkData", GetChunkData, SetChunkData, PODVector<unsigned char>, Variant::emptyBuffer, AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE("ChunkPosition", GetChunkPosition, SetChunkPosition, Vector3, Vector3::ZERO, AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE("ChunkPartIndex", GetChunkPartIndex, SetChunkPartIndex, int, 0, AM_DEFAULT);
}

void ChunkNetworkState::OnNodeSet(Node* node)
{
//    SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(ChunkNetworkState, HandlePostUpdate));
}

int ChunkNetworkState::GetLatestChangeID() const
{
    return changeID_;
}

void ChunkNetworkState::SetLatestChangeID(int value)
{
    changeID_ = value;
}

void ChunkNetworkState::OnGeometryChanged()
{
    MarkNetworkUpdate();
}

void ChunkNetworkState::MarkChanged()
{
    changeID_++;
    int index = 0;
    Chunk* chunk = reinterpret_cast<Chunk*>(node_->GetVar("Chunk").GetPtr());
    if (chunk) {
        for (int x = 0; x < SIZE_X; x++) {
            for (int y = 0; y < SIZE_Y; y++) {
                for (int z = 0; z < SIZE_Z; z++) {
                    chunkData_[index] = chunk->GetBlockAt(IntVector3(x, y, z))->type;
                    index++;
                }
            }
        }
        URHO3D_LOGINFOF("Chunk data set %d", index);
    }
    MarkNetworkUpdate();
}

void ChunkNetworkState::SetChunkData(const PODVector<unsigned char>& data)
{
    chunkData_ = data;

    using namespace ChunkReceived;
    VariantMap& eventData = GetEventDataMap();
    eventData[P_POSITION] = chunkPosition_;
    eventData[P_PART_INDEX] = chunkPartIndex_;
    SendEvent(E_CHUNK_RECEIVED, eventData);
}

const PODVector<unsigned char>& ChunkNetworkState::GetChunkData() const
{
    return chunkData_;
}

void ChunkNetworkState::SetChunkPosition(const Vector3& position)
{
    chunkPosition_ = position;
}

const Vector3& ChunkNetworkState::GetChunkPosition() const
{
    return chunkPosition_;
}

void ChunkNetworkState::SetChunkPartIndex(int index)
{
    chunkPartIndex_ = index;
}

const int ChunkNetworkState::GetChunkPartIndex() const
{
    return chunkPartIndex_;
}