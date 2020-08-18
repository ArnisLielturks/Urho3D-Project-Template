#ifdef VOXEL_SUPPORT
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/OctreeQuery.h>
#include <Urho3D/Graphics/Geometry.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/UI/Text3D.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/Resource/JSONFile.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Container/Vector.h>
#include <Urho3D/Core/Profiler.h>
#include <Urho3D/Engine/DebugHud.h>
#include <Urho3D/Audio/AudioDefs.h>
#include <Urho3D/Network/Network.h>
#include "ChunkMesh.h"
#include "../../Global.h"

ChunkMesh::ChunkMesh(Context* context):
        Object(context)
{
    vb_ = new VertexBuffer(context);
    ib_ = new IndexBuffer(context);
    geometry_ = new Geometry(context_);
}

ChunkMesh::~ChunkMesh()
{
}

void ChunkMesh::RegisterObject(Context* context)
{
    context->RegisterFactory<ChunkMesh>();
}

void ChunkMesh::WriteToVertexBuffer()
{
    unsigned elementMask = MASK_POSITION | MASK_NORMAL | MASK_COLOR | MASK_TEXCOORD1;
    vb_->SetSize(vertices_.Size(), elementMask, false);
    vb_->SetShadowed(true);

    if (!vertices_.Empty()) {
        unsigned char *dest = (unsigned char *) vb_->Lock(0, vertices_.Size(), true);

        if (dest) {
            for (auto i = 0; i < vertices_.Size(); ++i) {
                if (elementMask & MASK_POSITION) {
                    *((Vector3 *) dest) = vertices_[i].position_;
                    dest += sizeof(Vector3);
                }
                if (elementMask & MASK_NORMAL) {
                    *((Vector3 *) dest) = vertices_[i].normal_;
                    dest += sizeof(Vector3);
                }
                if (elementMask & MASK_COLOR) {
                    *((unsigned *) dest) = vertices_[i].color_.ToUInt();
                    dest += sizeof(unsigned);
                }
                if (elementMask & MASK_TEXCOORD1) {
                    *((Vector2 *) dest) = vertices_[i].uv_;
                    dest += sizeof(Vector2);
                }
//            if (elementMask & MASK_TEXCOORD2) {
//                *((Vector2*)dest) = vertices_[i].texCoord2_;
//                dest += sizeof(Vector2);
//            }
//            if (elementMask & MASK_CUBETEXCOORD1) {
//                *((Vector3*)dest) = vertices_[i].cubeTexCoord1_;
//                dest += sizeof(Vector3);
//            }
//            if (elementMask & MASK_CUBETEXCOORD2) {
//                *((Vector3*)dest) = vertices_[i].cubeTexCoord2_;
//                dest += sizeof(Vector3);
//            }
//            if (elementMask & MASK_TANGENT) {
//                *((Vector4*)dest) = vertices_[i].tangent_;
//                dest += sizeof(Vector4);
//            }
            }
        } else {
            URHO3D_LOGERROR("Failed to lock vertex buffer");
        }
        vb_->Unlock();
    }
}

void ChunkMesh::AddVertex(const MeshVertex& vertexData)
{
    vertices_.Push(vertexData);
}

void ChunkMesh::AddIndice(short index)
{
    indices_.Push(index);
}

void ChunkMesh::WriteToIndexBuffer()
{
    ib_->SetShadowed(true);
    ib_->SetSize(indices_.Size(), false);
    if (!indices_.Empty()) {
        ib_->SetData(indices_.Buffer());
    }
}

SharedPtr<VertexBuffer> ChunkMesh::GetVertexBuffer(Context* context) {
    WriteToVertexBuffer();

    return vb_;
}

SharedPtr<IndexBuffer> ChunkMesh::GetIndexBuffer(Context* context) {
    WriteToIndexBuffer();
    return ib_;
}

unsigned ChunkMesh::GetVertexCount()
{
    return vertices_.Size();
}

void ChunkMesh::Clear()
{
    indices_.Clear();
    vertices_.Clear();
}

SharedPtr<Geometry> ChunkMesh::GetGeometry()
{
    WriteToVertexBuffer();
    WriteToIndexBuffer();
    geometry_->SetVertexBuffer(0, vb_);
    geometry_->SetIndexBuffer(ib_);
    geometry_->SetDrawRange(TRIANGLE_LIST, 0, indices_.Size(), 0, vertices_.Size());

    return geometry_;
}
#endif
