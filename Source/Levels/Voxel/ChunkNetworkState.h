#pragma once

#include <Urho3D/Scene/Component.h>
#include <Urho3D/Container/Str.h>

using namespace Urho3D;

class ChunkNetworkState : public Component
{
URHO3D_OBJECT(ChunkNetworkState, Component);

public:
    explicit ChunkNetworkState(Context* context);
    ~ChunkNetworkState();
    static void RegisterObject(Context* context);

    void MarkChanged();
    void SetLatestChangeID(int value);
    int GetLatestChangeID() const;

protected:
    void OnNodeSet(Node* node) override;
private:
    void OnGeometryChanged();

    int changeID_{0};
};
