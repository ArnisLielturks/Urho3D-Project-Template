#pragma once

#include <Urho3D/Scene/Component.h>
#include <Urho3D/Container/Str.h>

using namespace Urho3D;

class PlayerState : public Component
{
URHO3D_OBJECT(PlayerState, Component);

public:
    explicit PlayerState(Context* context);
    ~PlayerState();
    static void RegisterObject(Context* context);
    void AddScore(int value);
    void SetScore(int value);
    int GetScore() const;

    void SetPlayerID(int id);
    int GetPlayerID() const;

    void SetPlayerName(const String& name);
    const String& GetPlayerName() const;

protected:
    void OnNodeSet(Node* node) override;
private:
    void HandlePlayerScoreAdd(StringHash eventType, VariantMap& eventData);
    void OnScoreChanged();
    int _score{0};
    int _playerId{-1};
    String _name;
};
