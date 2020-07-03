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

ChunkNetworkState::ChunkNetworkState(Context* context) :
        Component(context)
{
}

ChunkNetworkState::~ChunkNetworkState()
{
}

void ChunkNetworkState::RegisterObject(Context* context)
{
    context->RegisterFactory<ChunkNetworkState>();
    URHO3D_ACCESSOR_ATTRIBUTE("LatestChangeID", GetLatestChangeID, SetLatestChangeID, int, 0, AM_DEFAULT);
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
    MarkNetworkUpdate();
}
