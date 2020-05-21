#pragma once

#include <Urho3D/Scene/Component.h>

using namespace Urho3D;

class PlayerState : public Component
{
URHO3D_OBJECT(PlayerState, Component);

public:
    explicit PlayerState(Context* context);
    static void RegisterObject(Context* context);
    void AddScore(int value);
    void SetScore(int value);
    int GetScore() const;

    void SetPlayerID(int id);
    int GetPlayerID() const;

protected:
    void OnNodeSet(Node* node) override;
private:
    void HandlePlayerScoreAdd(StringHash eventType, VariantMap& eventData);
    void OnScoreChanged();
    int _score{0};
    int _playerId{-1};
};
