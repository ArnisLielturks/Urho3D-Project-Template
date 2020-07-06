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
#include <Urho3D/Network/NetworkEvents.h>
#include "ChunkMesh.h"
#include "../../Global.h"
#include "VoxelEvents.h"
#include "VoxelWorld.h"
#include "ChunkMeshGenerator.h"
#include "../../Console/ConsoleHandlerEvents.h"
#include "LightManager.h"
#include "TreeGenerator.h"
#include "../../Audio/AudioManagerDefs.h"
#include "../../Audio/AudioEvents.h"

using namespace VoxelEvents;
using namespace ConsoleHandlerEvents;

ChunkMesh::ChunkMesh(Context* context):
        Object(context)
{
}

ChunkMesh::~ChunkMesh()
{
}

void ChunkMesh::RegisterObject(Context* context)
{
    context->RegisterFactory<ChunkMesh>();
}
