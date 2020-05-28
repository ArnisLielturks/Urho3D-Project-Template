#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Serializable.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/UI/Text3D.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include "PlayerState.h"
#include "PlayerEvents.h"
#include "../../Global.h"

PlayerState::PlayerState(Context* context) :
        Component(context)
{
}

PlayerState::~PlayerState()
{
    VariantMap players = GetGlobalVar("Players").GetVariantMap();
    players.Erase(String(GetPlayerID()));
    SetGlobalVar("Players", players);
}

void PlayerState::RegisterObject(Context* context)
{
    context->RegisterFactory<PlayerState>();
    URHO3D_ACCESSOR_ATTRIBUTE("Score", GetScore, SetScore, int, 0, AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE("Player ID", GetPlayerID, SetPlayerID, int, -1, AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE("Name", GetPlayerName, SetPlayerName, String, String::EMPTY, AM_DEFAULT);
}

void PlayerState::OnNodeSet(Node* node)
{
    SubscribeToEvent(node, PlayerEvents::E_PLAYER_SCORE_ADD, URHO3D_HANDLER(PlayerState, HandlePlayerScoreAdd));
    SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(PlayerState, HandlePostUpdate));

    if (node) {
        auto *cache = GetSubsystem<ResourceCache>();
        _label = node->GetParent()->CreateChild("Label", LOCAL);

        auto text3D = _label->CreateComponent<Text3D>();
        text3D->SetFont(cache->GetResource<Font>(APPLICATION_FONT), 30);
        text3D->SetColor(Color::GRAY);
        text3D->SetAlignment(HA_CENTER, VA_BOTTOM);
        text3D->SetFaceCameraMode(FaceCameraMode::FC_LOOKAT_Y);
//    text3D->SetViewMask(~(1 << _controllerId));

//    if (!SHOW_LABELS) {
//        _label->SetEnabled(false);
//    }
    }
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

    VariantMap notificationData;
    if (value < 0) {
        notificationData["Status"] = "Error";
        notificationData["Message"] = _name + " lost " + String(-value) + " points";
    } else {
        notificationData["Message"] = _name + " got " + String(value) + " points";
    }
    SendEvent("ShowNotification", notificationData);

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
        playerData["Name"] = GetPlayerName();
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

void PlayerState::SetPlayerName(const String& name)
{
    _name = name;
    OnScoreChanged();
    MarkNetworkUpdate();

    if (_label) {
        _label->GetComponent<Text3D>()->SetText(name);
    }
}

const String& PlayerState::GetPlayerName() const
{
    return _name;
}

void PlayerState::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{
    if (_label) {
        _label->SetPosition(node_->GetPosition() + Vector3::UP * 0.2);
    }
}

void PlayerState::HideLabel()
{
    if (_label) {
        _label->SetEnabled(false);
    }
}
