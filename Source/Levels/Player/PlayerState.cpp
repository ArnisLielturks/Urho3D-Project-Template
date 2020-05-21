#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Serializable.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/IO/Log.h>
#include "PlayerState.h"
#include "PlayerEvents.h"

PlayerState::PlayerState(Context* context) :
        Component(context)
{
    URHO3D_ACCESSOR_ATTRIBUTE("Score", GetScore, SetScore, int, 0, AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE("Player ID", GetPlayerID, SetPlayerID, int, -1, AM_DEFAULT);
}

void PlayerState::RegisterObject(Context* context)
{
    context->RegisterFactory<PlayerState>();
}

void PlayerState::OnNodeSet(Node* node)
{
    SubscribeToEvent(node, PlayerEvents::E_PLAYER_SCORE_ADD, URHO3D_HANDLER(PlayerState, HandlePlayerScoreAdd));
}

int PlayerState::GetScore() const
{
    return _score;
}

void PlayerState::SetScore(int value)
{
    _score = value;
    if (_score < 0) {
        _score = 0;
    }
    OnScoreChanged();
}

void PlayerState::AddScore(int value)
{
    _score += value;
    if (_score < 0) {
        _score = 0;
    }
    OnScoreChanged();
}

void PlayerState::HandlePlayerScoreAdd(StringHash eventType, VariantMap& eventData)
{
    using namespace PlayerEvents::PlayerScoreAdd;
    int amount = eventData[P_SCORE].GetInt();
    URHO3D_LOGINFOF("Adding score to player %d", amount);
    AddScore(amount);
}

void PlayerState::OnScoreChanged()
{
    URHO3D_LOGINFOF("Player %d Score %d", _playerId, _score);
    if (_playerId >= 0) {
        VariantMap players = GetGlobalVar("Players").GetVariantMap();
        VariantMap playerData = players[String(GetPlayerID())].GetVariantMap();
        playerData["Score"] = _score;
        playerData["ID"] = GetPlayerID();
        players[String(GetPlayerID())] = playerData;
        SetGlobalVar("Players", players);
        SendEvent(PlayerEvents::E_PLAYER_SCORES_UPDATED);
    }
    MarkNetworkUpdate();
}

void PlayerState::SetPlayerID(int id)
{
    _playerId = id;
}

int PlayerState::GetPlayerID() const
{
    return _playerId;
}
