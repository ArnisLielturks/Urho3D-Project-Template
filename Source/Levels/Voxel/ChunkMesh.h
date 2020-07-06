#pragma once
#include <Urho3D/Graphics/CustomGeometry.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Core/WorkQueue.h>
#include <Urho3D/Graphics/VertexBuffer.h>
#include <Urho3D/Graphics/IndexBuffer.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include "VoxelDefs.h"
#include <queue>

using namespace Urho3D;

class ChunkMesh : public Object {
URHO3D_OBJECT(ChunkMesh, Object);
    ChunkMesh(Context* context);
    virtual ~ChunkMesh();

    static void RegisterObject(Context* context);
public:


private:


};
