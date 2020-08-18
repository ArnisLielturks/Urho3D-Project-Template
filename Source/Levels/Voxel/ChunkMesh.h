#ifdef VOXEL_SUPPORT
#pragma once
#include <queue>
#include <vector>
#include <Urho3D/Graphics/CustomGeometry.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Core/WorkQueue.h>
#include <Urho3D/Graphics/VertexBuffer.h>
#include <Urho3D/Graphics/IndexBuffer.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Graphics/Geometry.h>
#include "VoxelDefs.h"

using namespace Urho3D;

struct MeshVertex {
    MeshVertex() {}
    MeshVertex(Vector3 p, Vector3 n, Color c, Vector2 u): position_(p), normal_(n), color_(c), uv_(u) {}
    Vector3 position_;
    Vector3 normal_;
    Color color_;
    Vector2 uv_;
};

class ChunkMesh : public Object {
URHO3D_OBJECT(ChunkMesh, Object);
    ChunkMesh(Context* context);
    virtual ~ChunkMesh();

    static void RegisterObject(Context* context);
public:
    SharedPtr<VertexBuffer> GetVertexBuffer(Context* context);
    SharedPtr<IndexBuffer> GetIndexBuffer(Context* context);

    void AddVertex(const MeshVertex& vertexData);
    void AddIndice(short index);

    unsigned GetVertexCount();

    void Clear();

    void WriteToVertexBuffer();
    void WriteToIndexBuffer();

    SharedPtr<Geometry> GetGeometry();
private:
    SharedPtr<VertexBuffer> vb_;
    SharedPtr<IndexBuffer> ib_;

    Vector<MeshVertex> vertices_;
    Vector<short> indices_;

    SharedPtr<Geometry> geometry_;
};
#endif
