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

    void SetChunkData(const PODVector<unsigned char>& data);
    const PODVector<unsigned char>& GetChunkData() const;

    void SetChunkPosition(const Vector3& position);
    const Vector3& GetChunkPosition() const;

    void SetChunkPartIndex(int index);
    const int GetChunkPartIndex() const;

protected:
    void OnNodeSet(Node* node) override;
private:
    void OnGeometryChanged();

    int changeID_{0};
    Vector3 chunkPosition_;
    int chunkPartIndex_;
    PODVector<unsigned char> chunkData_;
};
