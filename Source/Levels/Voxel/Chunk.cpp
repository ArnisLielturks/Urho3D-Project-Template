#include "Chunk.h"
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
#include "../../Global.h"
#include "VoxelEvents.h"
#include "VoxelWorld.h"
#include "ChunkGenerator.h"
//#include "../../PolyVox/CubicSurfaceExtractor.h"
//#include "../../PolyVox/MarchingCubesSurfaceExtractor.h"
//#include "../../PolyVox/Mesh.h"
//#include "../../PolyVox/RawVolume.h"

using namespace VoxelEvents;

bool Chunk::visibilityUpdate_ = true;

//Use the PolyVox namespace
//using namespace PolyVox;

//void createSphereInVolume(RawVolume<uint8_t>& volData, float fRadius)
//{
//    //This vector hold the position of the center of the volume
//    Vector3DFloat v3dVolCenter(volData.getWidth() / 2, volData.getHeight() / 2, volData.getDepth() / 2);
//
//    //This three-level for loop iterates over every voxel in the volume
//    for (int z = 0; z < volData.getDepth(); z++)
//    {
//        for (int y = 0; y < volData.getHeight(); y++)
//        {
//            for (int x = 0; x < volData.getWidth(); x++)
//            {
//                //Store our current position as a vector...
//                Vector3DFloat v3dCurrentPos(x, y, z);
//                //And compute how far the current position is from the center of the volume
//                float fDistToCenter = (v3dCurrentPos - v3dVolCenter).length();
//
//                uint8_t uVoxelValue = 0;
//
//                //If the current voxel is less than 'radius' units from the center then we make it solid.
//                if (fDistToCenter <= fRadius)
//                {
//                    //Our new voxel value
//                    uVoxelValue = 255;
//                }
//
//
//
//                //Wrte the voxel value into the volume
//                volData.setVoxel(x, y, z, uVoxelValue);
//            }
//        }
//    }
//}


void SaveToFile(const WorkItem* item, unsigned threadIndex)
{
    Chunk* chunk = reinterpret_cast<Chunk*>(item->aux_);
    JSONFile file(chunk->context_);
    JSONValue& root = file.GetRoot();
    for (int x = 0; x < SIZE_X; ++x) {
        for (int y = 0; y < SIZE_Y; y++) {
            for (int z = 0; z < SIZE_Z; z++) {
                root.Set(String(x) + "_" + String(y) + "_" + String(z), chunk->data[x][y][z]);
            }
        }
    }
    if(!chunk->GetSubsystem<FileSystem>()->DirExists("World")) {
        chunk->GetSubsystem<FileSystem>()->CreateDir("World");
    }
    Vector3 position = chunk->position_;
    file.SaveFile("World/chunk_" + String(position.x_) + "_" + String(position.y_) + "_" + String(position.z_));
}

void GenerateMesh(const WorkItem* item, unsigned threadIndex)
{

    Chunk* chunk = reinterpret_cast<Chunk*>(item->aux_);
    Vector3 position = chunk->position_;

    JSONFile file(chunk->context_);
    JSONValue& root = file.GetRoot();
    String filename = "World/chunk_" + String(position.x_) + "_" + String(position.y_) + "_" + String(position.z_);
    if(chunk->GetSubsystem<FileSystem>()->FileExists(filename)) {
        file.LoadFile(filename);
        for (int x = 0; x < SIZE_X; ++x) {
            for (int y = 0; y < SIZE_Y; y++) {
                for (int z = 0; z < SIZE_Z; z++) {
                    String key = String(x) + "_" + String(y) + "_" + String(z);
                    if (root.Contains(key)) {
                        chunk->data[x][y][z] = static_cast<BlockType>(root[key].GetInt());
                    }
                }
            }
        }
        URHO3D_LOGINFO("Loaded chunk from file");
    } else {

        auto chunkGenerator = chunk->GetSubsystem<ChunkGenerator>();
        // Terrain
        for (int x = 0; x < SIZE_X; ++x) {
            for (int z = 0; z < SIZE_Z; z++) {
                Vector3 blockPosition = position + Vector3(x, 0, z);
                int surfaceHeight = chunkGenerator->GetTerrainHeight(blockPosition);
                for (int y = 0; y < SIZE_Y; y++) {
                    blockPosition.y_ = position.y_ + y;
                    chunk->data[x][y][z] = chunkGenerator->GetBlockType(blockPosition, surfaceHeight);
                }
            }
        }

        // Caves
        for (int x = 0; x < SIZE_X; ++x) {
            for (int y = 0; y < SIZE_Y; y++) {
                for (int z = 0; z < SIZE_Z; z++) {
                    Vector3 blockPosition = position + Vector3(x, y, z);
                    chunk->data[x][y][z] = chunkGenerator->GetCaveBlockType(blockPosition, chunk->data[x][y][z]);
                }
            }
        }
    }
}

Chunk::Chunk(Context* context):
Object(context)
{
}

Chunk::~Chunk()
{
    if (generateWorkItem_) {
        WorkQueue *workQueue = GetSubsystem<WorkQueue>();
        if (workQueue) {
            workQueue->RemoveWorkItem(generateWorkItem_);
        }
    }
    if (saveWorkItem_) {
        WorkQueue *workQueue = GetSubsystem<WorkQueue>();
        if (workQueue) {
            workQueue->RemoveWorkItem(saveWorkItem_);
        }
    }
    RemoveNode();
}

void Chunk::RegisterObject(Context* context)
{
    context->RegisterFactory<Chunk>();
}

void Chunk::Init(Scene* scene, const Vector3& position)
{
    scene_ = scene;
    position_ = position;

    auto cache = GetSubsystem<ResourceCache>();
    node_ = scene_->CreateChild("Chunk " + position_.ToString(), LOCAL);
    node_->SetScale(1.0f);
    node_->SetWorldPosition(position_);

    triggerNode_ = node_->CreateChild("ChunkTrigger");
    auto triggerBody = triggerNode_->CreateComponent<RigidBody>();
    triggerBody->SetTrigger(true);
    triggerBody->SetCollisionLayerAndMask(COLLISION_MASK_CHUNK , COLLISION_MASK_CHUNK_LOADER);
    auto *triggerShape = triggerNode_->CreateComponent<CollisionShape>();
    triggerNode_->SetWorldScale(VoxelWorld::visibleDistance);

    SubscribeToEvent(triggerNode_, E_NODECOLLISIONSTART, URHO3D_HANDLER(Chunk, HandlePlayerEntered));
    SubscribeToEvent(triggerNode_, E_NODECOLLISIONEND, URHO3D_HANDLER(Chunk, HandlePlayerExited));

//    Vector3 offset = Vector3(SIZE_X / 2, SIZE_Y / 2, SIZE_Z / 2) * triggerNode_->GetScale();
    triggerShape->SetBox(Vector3(SIZE_X, SIZE_Y, SIZE_Z), Vector3(SIZE_X / 2, SIZE_Y / 2, SIZE_Z / 2));

    SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(Chunk, HandlePostRenderUpdate));
    SubscribeToEvent(E_WORKITEMCOMPLETED, URHO3D_HANDLER(Chunk, HandleWorkItemFinished));

    SubscribeToEvent("#chunk_visible_distance", [&](StringHash eventType, VariantMap& eventData) {
        if (triggerNode_) {
            triggerNode_->SetWorldScale(VoxelWorld::visibleDistance);
        }
    });
//    SetVisibility(false);
}

void Chunk::Generate()
{
    WorkQueue *workQueue = GetSubsystem<WorkQueue>();
    generateWorkItem_ = workQueue->GetFreeItem();
    generateWorkItem_->priority_ = M_MAX_UNSIGNED;
    generateWorkItem_->workFunction_ = GenerateMesh;
    generateWorkItem_->aux_ = this;
    generateWorkItem_->sendEvent_ = true;

    generateWorkItem_->start_ = nullptr;
    generateWorkItem_->end_ = nullptr;
    workQueue->AddWorkItem(generateWorkItem_);
}

void Chunk::UpdateGeometry()
{
    geometry_->Commit();
    geometry_->SetCastShadows(true);

    auto cache = GetSubsystem<ResourceCache>();
    geometry_->SetMaterial(cache->GetResource<Material>("Materials/Voxel.xml"));

    auto *shape = node_->GetComponent<CollisionShape>();
    if (geometry_->GetNumVertices(0) > 0) {
        shape->SetCustomTriangleMesh(geometry_);
    } else {
        shape->SetEnabled(false);
    }
//    UpdateVisibility();
}

void Chunk::GenerateGeometry()
{
//    // Create an empty volume and then place a sphere in it
//    HashMap<Vector3, int> materials;
//    RawVolume<uint8_t> volData(PolyVox::Region(Vector3DInt32(0, 0, 0), Vector3DInt32(SIZE_X, SIZE_Y, SIZE_Z)));
//    createSphereInVolume(volData, 3);
//
//    // Extract the surface for the specified region of the volume. Uncomment the line for the kind of surface extraction you want to see.
//    Region region;
//    region.setLowerX(0);
//    region.setLowerY(0);
//    region.setLowerZ(0);
//    region.setUpperX(SIZE_X);
//    region.setUpperY(SIZE_Y);
//    region.setUpperZ(SIZE_Z);
//    auto mesh = extractCubicMesh(&volData, region);
//    //auto mesh = extractMarchingCubesMesh(&volData, volData.getEnclosingRegion());
//
//    // The surface extractor outputs the mesh in an efficient compressed format which is not directly suitable for rendering. The easiest approach is to
//    // decode this on the CPU as shown below, though more advanced applications can upload the compressed mesh to the GPU and decompress in shader code.
//    auto decodedMesh = decodeMesh(mesh);
//
//
//    vb_->SetShadowed(true);
//    vb_->SetSize(8, MASK_POSITION | MASK_NORMAL | MASK_TEXCOORD1, false);
//    vb_->SetDataRange(decodedMesh.getRawVertexData(), 0, decodedMesh.getNoOfVertices());

//    if (vb.GetVertexCount() != vertices_.Size() || vb.GetElementMask() != elementMask_)
//        vb.SetSize(vertices_.Size(), elementMask_, false);

//    unsigned char* dest = (unsigned char*) vb_->Lock(0, decodedMesh.getNoOfVertices(), true);

//    if (dest) {
//        for (auto i = 0; i < decodedMesh.getNoOfVertices(); ++i) {
////            if (elementMask_ & MASK_POSITION) {
//                auto vertex = decodedMesh.getVertex(i);
//                *((Vector3*)dest) = Vector3(vertex.position.getX(), vertex.position.getY(), vertex.position.getZ());
//                dest += sizeof(Vector3);
////            }
////            if (elementMask_ & MASK_NORMAL) {
//                *((Vector3*)dest) = Vector3(vertex.normal.getX(), vertex.normal.getY(), vertex.normal.getZ());
//                dest += sizeof(Vector3);
////            }
////            if (elementMask_ & MASK_COLOR) {
////                *((unsigned*)dest) = vertices_[i].color_.ToUInt();
////                dest += sizeof(unsigned);
////            }
////            if (elementMask_ & MASK_TEXCOORD1) {
//            float pos = i * 2.0 / decodedMesh.getNoOfVertices();
//                *((Vector2*)dest) = Vector2(pos, pos);
//                dest += sizeof(Vector2);
////            }
////            if (elementMask_ & MASK_TEXCOORD2) {
////                *((Vector2*)dest) = vertices_[i].texCoord2_;
////                dest += sizeof(Vector2);
////            }
////            if (elementMask_ & MASK_CUBETEXCOORD1) {
////                *((Vector3*)dest) = vertices_[i].cubeTexCoord1_;
////                dest += sizeof(Vector3);
////            }
////            if (elementMask_ & MASK_CUBETEXCOORD2) {
////                *((Vector3*)dest) = vertices_[i].cubeTexCoord2_;
////                dest += sizeof(Vector3);
////            }
////            if (elementMask_ & MASK_TANGENT) {
////                *((Vector4*)dest) = vertices_[i].tangent_;
////                dest += sizeof(Vector4);
////            }
//        }
//    } else {
//        URHO3D_LOGERROR("Failed to lock vertex buffer");
//    }
//    vb_->Unlock();
//
//
////    auto vertexData = decodedMesh.getRawVertexData();
////    auto vertex0 = decodedMesh.getVertex(0);
////    for (int i = 0; i < decodedMesh.getNoOfVertices(); i++) {
////        auto vertex = decodedMesh.getVertex(i);
////    }
//
//    Urho3D::Vector<unsigned short> indices(decodedMesh.getNoOfIndices());
//
//    for (int i = 0; i < decodedMesh.getNoOfIndices(); i++) {
//        auto indexData = decodedMesh.getIndex(i);
//        indices[i] = static_cast<unsigned short>(indexData);
//    }
//
//    ib_->SetShadowed(true);
//    ib_->SetSize(decodedMesh.getNoOfIndices(), false);
//    ib_->SetData(indices.Buffer());
////    ib_->SetDataRange(decodedMesh.getRawIndexData(), 0, decodedMesh.getNoOfIndices());
//
//    URHO3D_LOGINFOF("VB Size %d => %d", decodedMesh.getNoOfVertices(), vb_->GetVertexCount());
//    URHO3D_LOGINFOF("IB Size %d => %d", decodedMesh.getNoOfIndices(), ib_->GetIndexCount());
//
//    chunkGeometry_->SetVertexBuffer(0, vb_);
//    chunkGeometry_->SetIndexBuffer(ib_);
//    chunkGeometry_->SetDrawRange(TRIANGLE_LIST, 0, decodedMesh.getNoOfIndices());
//
//    chunkModel_ = new Model(context_);
//    chunkModel_->SetNumGeometries(1);
//    chunkModel_->SetGeometry(0, 0, chunkGeometry_);
//    auto staticModel = node_->CreateComponent<StaticModel>();
//    staticModel->SetModel(chunkModel_);
//    auto cache = GetSubsystem<ResourceCache>();
//    staticModel->SetMaterial(cache->GetResource<Material>("Materials/Voxel.xml"));
//    staticModel->SetCastShadows(true);
//
//    return;
    geometry_->SetDynamic(false);
    geometry_->SetNumGeometries(1);
    geometry_->BeginGeometry(0, TRIANGLE_LIST);
    geometry_->SetViewMask(VIEW_MASK_CHUNK);
    geometry_->SetOccluder(true);
    geometry_->SetOccludee(true);

//    int vertices = 0;
//    for (int x = 0; x < SIZE_X; x++) {
//        for (int y = 0; y < SIZE_Y; y++) {
//            for (int z = 0; z < SIZE_Z; z++) {
//                if (data[x][y][z] > 0) {
//                    int blockId = data[x][y][z];
//                    Vector3 position(x, y, z);
//                    if (!BlockHaveNeighbor(BlockSide::TOP, x, y, z)) {
//                        vertices += 4;
//                    }
//                    if (!BlockHaveNeighbor(BlockSide::BOTTOM, x, y, z)) {
//                        vertices += 4;
//                    }
//                    if (!BlockHaveNeighbor(BlockSide::LEFT, x, y, z)) {
//                        vertices += 4;
//                    }
//                    if (!BlockHaveNeighbor(BlockSide::RIGHT, x, y, z)) {
//                        vertices += 4;
//                    }
//                    if (!BlockHaveNeighbor(BlockSide::FRONT, x, y, z)) {
//                        vertices += 4;
//                    }
//                    if (!BlockHaveNeighbor(BlockSide::BACK, x, y, z)) {
//                        vertices += 4;
//                    }
//                }
//            }
//        }
//    }
//    vb_->SetShadowed(true);
//    vb_->SetSize(8, MASK_POSITION | MASK_NORMAL | MASK_COLOR | MASK_TEXCOORD1 | MASK_TANGENT, false);
//    unsigned char* dest = (unsigned char*) vb_->Lock(0, vertices, true);
//    for (int x = 0; x < SIZE_X; x++) {
//        for (int y = 0; y < SIZE_Y; y++) {
//            for (int z = 0; z < SIZE_Z; z++) {
//                if (data[x][y][z] > 0) {
//                    int blockId = data[x][y][z];
//                    Vector3 position(x, y, z);
//                    if (!BlockHaveNeighbor(BlockSide::TOP, x, y, z)) {
//                        *((Vector3*)dest) = Vector3(x, y, z);
//                        dest += sizeof(Vector3);
//                        *((Vector3*)dest) = Vector3::UP;
//                        dest += sizeof(Vector3);
//                        *((unsigned*)dest) = 244 * 233 * 100;
//                        dest += sizeof(unsigned);
//                    }
//                    if (!BlockHaveNeighbor(BlockSide::BOTTOM, x, y, z)) {
//                        vertices += 4;
//                    }
//                    if (!BlockHaveNeighbor(BlockSide::LEFT, x, y, z)) {
//                        vertices += 4;
//                    }
//                    if (!BlockHaveNeighbor(BlockSide::RIGHT, x, y, z)) {
//                        vertices += 4;
//                    }
//                    if (!BlockHaveNeighbor(BlockSide::FRONT, x, y, z)) {
//                        vertices += 4;
//                    }
//                    if (!BlockHaveNeighbor(BlockSide::BACK, x, y, z)) {
//                        vertices += 4;
//                    }
//                }
//            }
//        }
//    }
//    vb_->Unlock();
    for (int i = 0; i < 6; i++) {
        sideTransparent_[i] = true;
    }
    for (int x = 0; x < SIZE_X; x++) {
        for (int y = 0; y < SIZE_Y; y++) {
            for (int z = 0; z < SIZE_Z; z++) {
                if (data[x][y][z] > 0) {

                    if (x == 0) {
                        sideTransparent_[static_cast<int>(BlockSide::LEFT)] = false;
                    }
                    if (x == SIZE_X - 1) {
                        sideTransparent_[static_cast<int>(BlockSide::RIGHT)] = false;
                    }
                    if (y == 0) {
                        sideTransparent_[static_cast<int>(BlockSide::BOTTOM)] = false;
                    }
                    if (y == SIZE_Y - 1) {
                        sideTransparent_[static_cast<int>(BlockSide::TOP)] = false;
                    }
                    if (z == 0) {
                        sideTransparent_[static_cast<int>(BlockSide::BACK)] = false;
                    }
                    if (z == SIZE_Z - 1) {
                        sideTransparent_[static_cast<int>(BlockSide::FRONT)] = false;
                    }
                    int blockId = data[x][y][z];
                    Vector3 position(x, y, z);
                    if (!BlockHaveNeighbor(BlockSide::TOP, x, y, z)) {
                        // tri1
                        geometry_->DefineVertex(position + Vector3(0.0f, 1.0, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::TOP, static_cast<BlockType>(blockId), Vector2(0.0, 0.0)));
                        geometry_->DefineNormal(Vector3::UP);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));

                        geometry_->DefineVertex(position + Vector3(0.0f, 1.0, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::TOP, static_cast<BlockType>(blockId), Vector2(0.0, 1.0)));
                        geometry_->DefineNormal(Vector3::UP);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 1.0, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::TOP, static_cast<BlockType>(blockId), Vector2(1.0, 0.0)));
                        geometry_->DefineNormal(Vector3::UP);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));

                        // tri2
                        geometry_->DefineVertex(position + Vector3(0.0f, 1.0, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::TOP, static_cast<BlockType>(blockId), Vector2(0.0, 1.0)));
                        geometry_->DefineNormal(Vector3::UP);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 1.0, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::TOP, static_cast<BlockType>(blockId), Vector2(1.0, 1.0)));
                        geometry_->DefineNormal(Vector3::UP);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 1.0, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::TOP, static_cast<BlockType>(blockId), Vector2(1.0, 0.0)));
                        geometry_->DefineNormal(Vector3::UP);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));
                    }
                    if (!BlockHaveNeighbor(BlockSide::BOTTOM, x, y, z)) {
                        // tri1
                        geometry_->DefineVertex(position + Vector3(0.0f, 0.0f, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BOTTOM, static_cast<BlockType>(blockId), Vector2(0.0, 1.0)));
                        geometry_->DefineNormal(Vector3::DOWN);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f, 0.0f, 1.0f));

                        geometry_->DefineVertex(position + Vector3(0.0f, 0.0f, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BOTTOM, static_cast<BlockType>(blockId), Vector2(0.0, 0.0)));
                        geometry_->DefineNormal(Vector3::DOWN);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f, 0.0f, 1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 0.0f, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BOTTOM, static_cast<BlockType>(blockId), Vector2(1.0, 0.0)));
                        geometry_->DefineNormal(Vector3::DOWN);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f, 0.0f, 1.0f));

                        // tri2
                        geometry_->DefineVertex(position + Vector3(1.0, 0.0f, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BOTTOM, static_cast<BlockType>(blockId), Vector2(1.0, 1.0)));
                        geometry_->DefineNormal(Vector3::DOWN);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f, 0.0f, 1.0f));

                        geometry_->DefineVertex(position + Vector3(0.0f, 0.0f, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BOTTOM, static_cast<BlockType>(blockId), Vector2(0.0, 1.0)));
                        geometry_->DefineNormal(Vector3::DOWN);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f, 0.0f, 1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 0.0f, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BOTTOM, static_cast<BlockType>(blockId), Vector2(1.0, 0.0)));
                        geometry_->DefineNormal(Vector3::DOWN);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f, 0.0f, 1.0f));
                    }
                    if (!BlockHaveNeighbor(BlockSide::LEFT, x, y, z)) {
                        // tri1
                        geometry_->DefineVertex(position + Vector3(0.0f, 0.0f, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::LEFT, static_cast<BlockType>(blockId), Vector2(0.0, 1.0)));
                        geometry_->DefineNormal(Vector3::LEFT);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(0.0f, 1.0, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::LEFT, static_cast<BlockType>(blockId), Vector2(0.0, 0.0)));
                        geometry_->DefineNormal(Vector3::LEFT);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(0.0f, 0.0f, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::LEFT, static_cast<BlockType>(blockId), Vector2(1.0, 1.0)));
                        geometry_->DefineNormal(Vector3::LEFT);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

//                        // tri2
                        geometry_->DefineVertex(position + Vector3(0.0f, 1.0, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::LEFT, static_cast<BlockType>(blockId), Vector2(0.0, 0.0)));
                        geometry_->DefineNormal(Vector3::LEFT);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(0.0f, 1.0, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::LEFT, static_cast<BlockType>(blockId), Vector2(1.0, 0.0)));
                        geometry_->DefineNormal(Vector3::LEFT);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(0.0f, 0.0f, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::LEFT, static_cast<BlockType>(blockId), Vector2(1.0, 1.0)));
                        geometry_->DefineNormal(Vector3::LEFT);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));
                    }
                    if (!BlockHaveNeighbor(BlockSide::RIGHT, x, y, z)) {
                        // tri1
                        geometry_->DefineVertex(position + Vector3(1.0, 0.0f, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::RIGHT, static_cast<BlockType>(blockId), Vector2(0.0, 1.0)));
                        geometry_->DefineNormal(Vector3::RIGHT);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 1.0, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::RIGHT, static_cast<BlockType>(blockId), Vector2(0.0, 0.0)));
                        geometry_->DefineNormal(Vector3::RIGHT);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 0.0f, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::RIGHT, static_cast<BlockType>(blockId), Vector2(1.0, 1.0)));
                        geometry_->DefineNormal(Vector3::RIGHT);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));

                        // tri2
                        geometry_->DefineVertex(position + Vector3(1.0, 1.0, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::RIGHT, static_cast<BlockType>(blockId), Vector2(0.0, 0.0)));
                        geometry_->DefineNormal(Vector3::RIGHT);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 1.0, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::RIGHT, static_cast<BlockType>(blockId), Vector2(1.0, 0.0)));
                        geometry_->DefineNormal(Vector3::RIGHT);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 0.0f, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::RIGHT, static_cast<BlockType>(blockId), Vector2(1.0, 1.0)));
                        geometry_->DefineNormal(Vector3::RIGHT);
                        geometry_->DefineTangent(Vector4(0.0f, 0.0f, -1.0f, -1.0f));
                    }
                    if (!BlockHaveNeighbor(BlockSide::FRONT, x, y, z)) {
                        // tri1
                        geometry_->DefineVertex(position + Vector3(0.0f, 0.0f, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::FRONT, static_cast<BlockType>(blockId), Vector2(0.0, 1.0)));
                        geometry_->DefineNormal(Vector3::FORWARD);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(0.0f, 1.0, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::FRONT, static_cast<BlockType>(blockId), Vector2(0.0, 0.0)));
                        geometry_->DefineNormal(Vector3::FORWARD);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 0.0f, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::FRONT, static_cast<BlockType>(blockId), Vector2(1.0, 1.0)));
                        geometry_->DefineNormal(Vector3::FORWARD);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        // tri2
                        geometry_->DefineVertex(position + Vector3(0.0f, 1.0, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::FRONT, static_cast<BlockType>(blockId), Vector2(0.0, 0.0)));
                        geometry_->DefineNormal(Vector3::FORWARD);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 1.0, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::FRONT, static_cast<BlockType>(blockId), Vector2(1.0, 0.0)));
                        geometry_->DefineNormal(Vector3::FORWARD);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 0.0f, 0.0f));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::FRONT, static_cast<BlockType>(blockId), Vector2(1.0, 1.0)));
                        geometry_->DefineNormal(Vector3::FORWARD);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));
                    }
                    if (!BlockHaveNeighbor(BlockSide::BACK, x, y, z)) {
                        // tri1
                        geometry_->DefineVertex(position + Vector3(1.0, 0.0f, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BACK, static_cast<BlockType>(blockId), Vector2(0.0, 1.0)));
                        geometry_->DefineNormal(Vector3::BACK);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(1.0, 1.0, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BACK, static_cast<BlockType>(blockId), Vector2(0.0, 0.0)));
                        geometry_->DefineNormal(Vector3::BACK);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(0.0f, 0.0f, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BACK, static_cast<BlockType>(blockId), Vector2(1.0, 1.0)));
                        geometry_->DefineNormal(Vector3::BACK);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        // tri2
                        geometry_->DefineVertex(position + Vector3(1.0, 1.0, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BACK, static_cast<BlockType>(blockId), Vector2(0.0, 0.0)));
                        geometry_->DefineNormal(Vector3::BACK);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(0.0f, 1.0, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BACK, static_cast<BlockType>(blockId), Vector2(1.0, 0.0)));
                        geometry_->DefineNormal(Vector3::BACK);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));

                        geometry_->DefineVertex(position + Vector3(0.0f, 0.0f, 1.0));
                        geometry_->DefineTexCoord(GetTextureCoord(BlockSide::BACK, static_cast<BlockType>(blockId), Vector2(1.0, 1.0)));
                        geometry_->DefineNormal(Vector3::BACK);
                        geometry_->DefineTangent(Vector4(1.0f, 0.0f,  0.0f,  1.0f));
                    }
                }
            }
        }
    }

//    for (int i = 0; i < 6; i++) {
//        if (sideTransparent_[i]) {
//            auto neighbor = GetNeighbor(static_cast<BlockSide>(i));
//            if (neighbor) {
//                neighbor->UpdateVisibility();
//            }
//        }
//    }
}

void Chunk::HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
{
    if (!node_) {
        return;
    }
//    scene_->GetComponent<PhysicsWorld>()->DrawDebugGeometry(true);
//    scene_->GetComponent<PhysicsWorld>()->SetDebugRenderer(scene_->GetComponent<DebugRenderer>());
//    node_->GetComponent<StaticModel>()->DrawDebugGeometry(node_->GetScene()->GetComponent<DebugRenderer>(), true);
}

IntVector3 Chunk::GetChunkBlock(Vector3 position)
{
    IntVector3 blockPosition;
    blockPosition.x_ = Floor(position.x_ - node_->GetPosition().x_);
    blockPosition.y_ = Floor(position.y_ - node_->GetPosition().y_);
    blockPosition.z_ = Floor(position.z_ - node_->GetPosition().z_);
    return blockPosition;
}

bool Chunk::IsBlockInsideChunk(IntVector3 position)
{
    if (position.x_ < 0 || position.x_ >= SIZE_X) {
        return false;
    }
    if (position.y_ < 0 || position.y_ >= SIZE_Y) {
        return false;
    }
    if (position.z_ < 0 || position.z_ >= SIZE_Z) {
        return false;
    }

    return true;
}

void Chunk::HandleHit(StringHash eventType, VariantMap& eventData)
{
    Vector3 position = eventData["Position"].GetVector3();
    auto blockPosition = GetChunkBlock(position);
    if (!IsBlockInsideChunk(blockPosition)) {
        auto neighborChunk = GetSubsystem<VoxelWorld>()->GetChunkByPosition(position);
        if (neighborChunk && neighborChunk->GetNode()) {
            neighborChunk->GetNode()->SendEvent("ChunkHit", eventData);
        }
        return;
    }
    if (data[blockPosition.x_][blockPosition.y_][blockPosition.z_]) {
//        int blockId = data[blockPosition.x_][blockPosition.y_][blockPosition.z_];
        data[blockPosition.x_][blockPosition.y_][blockPosition.z_] = BlockType::AIR;
        URHO3D_LOGINFO("Removing block " + blockPosition.ToString());
        GenerateGeometry();
        UpdateGeometry();
    }
    Save();
}

void Chunk::HandleAdd(StringHash eventType, VariantMap& eventData)
{
    Vector3 position = eventData["Position"].GetVector3();
    auto blockPosition = GetChunkBlock(position);
    if (!IsBlockInsideChunk(blockPosition)) {
        auto neighborChunk = GetSubsystem<VoxelWorld>()->GetChunkByPosition(position);
        if (neighborChunk && neighborChunk->GetNode()) {
            neighborChunk->GetNode()->SendEvent("ChunkAdd", eventData);
        } else {
            URHO3D_LOGERROR("Neighbor chunk doesn't exist at position " + position.ToString());
        }
        return;
    }
    if (data[blockPosition.x_][blockPosition.y_][blockPosition.z_] == 0) {
        data[blockPosition.x_][blockPosition.y_][blockPosition.z_] = BlockType::DIRT;
        URHO3D_LOGINFOF("Controller %d added block", eventData["ControllerId"].GetInt());
//        URHO3D_LOGINFOF("Chunk add world pos: %s; chunk pos: %d %d %d", pos.ToString().CString(), x, y, z);
        GenerateGeometry();
        UpdateGeometry();
    }

    Save();
}

const Vector3& Chunk::GetPosition()
{
    return position_;
}

void Chunk::HandleWorkItemFinished(StringHash eventType, VariantMap& eventData)
{
    using namespace WorkItemCompleted;
    WorkItem* workItem = reinterpret_cast<WorkItem*>(eventData[P_ITEM].GetPtr());
    if (workItem->aux_ != this) {
        return;
    }
    if (workItem->workFunction_ == GenerateMesh) {
//        URHO3D_LOGINFO("Chunk background preparing finished " + position_.ToString());
        CreateNode();
        generateWorkItem_.Reset();
    } else if (workItem->workFunction_ == SaveToFile) {
        URHO3D_LOGINFO("Background chunk saving done " + position_.ToString());
        saveWorkItem_.Reset();
    }
}

void Chunk::HandlePlayerEntered(StringHash eventType, VariantMap& eventData)
{
    using namespace ChunkEntered;
    VariantMap& data = GetEventDataMap();
    data[P_POSITION] = position_;
    SendEvent(E_CHUNK_ENTERED, data);
//    URHO3D_LOGINFO("Chunk " + position_.ToString() + " player entered");

    using namespace NodeCollisionStart;

    // Get the other colliding body, make sure it is moving (has nonzero mass)
    auto* otherNode = static_cast<Node*>(eventData[P_OTHERNODE].GetPtr());
//    URHO3D_LOGINFO("Name: " + otherNode->GetName() + " ID : " + String(otherNode->GetID()));
    visitors_++;

//    for (int i = 0; i < 6; i++) {
////        if (!visibleNeighbors_[i]) {
//            if (neighbors_[i]) {
//                neighbors_[i]->SetVisible(visibleNeighbors_[i]);
//            }
////        }
//    }
}

void Chunk::HandlePlayerExited(StringHash eventType, VariantMap& eventData)
{
    using namespace ChunkExited;
    VariantMap& data = GetEventDataMap();
    data[P_POSITION] = position_;
    SendEvent(E_CHUNK_EXITED, data);
    visitors_--;
}

Vector2 Chunk::GetTextureCoord(BlockSide side, BlockType blockType, Vector2 position)
{
    int textureCount = static_cast<int>(BlockType::NONE) - 1;
    Vector2 quadSize(1.0f / 6, 1.0f / textureCount);
    float typeOffset = (static_cast<int>(blockType) - 1) * quadSize.y_;
    switch (side) {
        case BlockSide::TOP: {
            return Vector2(quadSize.x_ * 0.0f + quadSize.x_ * position.x_, quadSize.y_ * position.y_ + typeOffset);
        }
        case BlockSide::BOTTOM: {
            return Vector2(quadSize.x_ * 1.0f + quadSize.x_ * position.x_, quadSize.y_ * position.y_ + typeOffset);
        }
        case BlockSide::LEFT: {
            return Vector2(quadSize.x_ * 2.0f + quadSize.x_ * position.x_, quadSize.y_ * position.y_ + typeOffset);
        }
        case BlockSide::RIGHT: {
            return Vector2(quadSize.x_ * 3.0f + quadSize.x_ * position.x_, quadSize.y_ * position.y_ + typeOffset);
        }
        case BlockSide::FRONT: {
            return Vector2(quadSize.x_ * 4.0f + quadSize.x_ * position.x_, quadSize.y_ * position.y_ + typeOffset);
        }
        case BlockSide::BACK: {
            return Vector2(quadSize.x_ * 5.0f + quadSize.x_ * position.x_, quadSize.y_ * position.y_ + typeOffset);
        }
    }
    return position;
}

void Chunk::Save()
{
    WorkQueue *workQueue = GetSubsystem<WorkQueue>();
    saveWorkItem_ = workQueue->GetFreeItem();
    saveWorkItem_->priority_ = M_MAX_UNSIGNED;
    saveWorkItem_->workFunction_ = SaveToFile;
    saveWorkItem_->aux_ = this;
    // send E_WORKITEMCOMPLETED event after finishing WorkItem
    saveWorkItem_->sendEvent_ = true;

    saveWorkItem_->start_ = nullptr;
    saveWorkItem_->end_ = nullptr;
    workQueue->AddWorkItem(saveWorkItem_);

}

void Chunk::CreateNode()
{
    chunkGeometry_ = new Geometry(context_);
    vb_ = new VertexBuffer(context_);
    ib_ = new IndexBuffer(context_);

    geometry_ = new CustomGeometry(context_);
    node_->AddComponent(geometry_, scene_->GetFreeComponentID(LOCAL), LOCAL);


    auto *body = node_->CreateComponent<RigidBody>(LOCAL);
    body->SetMass(0);
    body->SetCollisionLayerAndMask(COLLISION_MASK_GROUND, COLLISION_MASK_PLAYER | COLLISION_MASK_OBSTACLES);
    node_->CreateComponent<CollisionShape>(LOCAL);

    GenerateGeometry();
    UpdateGeometry();

    SubscribeToEvent(node_, "ChunkHit", URHO3D_HANDLER(Chunk, HandleHit));
    SubscribeToEvent(node_, "ChunkAdd", URHO3D_HANDLER(Chunk, HandleAdd));

    auto cache = GetSubsystem<ResourceCache>();
    label_ = node_->CreateChild("Label", LOCAL);
    auto text3D = label_->CreateComponent<Text3D>();
    text3D->SetFont(cache->GetResource<Font>(APPLICATION_FONT), 30);
    text3D->SetColor(Color::GRAY);
    text3D->SetViewMask(VIEW_MASK_GUI);
    text3D->SetAlignment(HA_CENTER, VA_BOTTOM);
    text3D->SetFaceCameraMode(FaceCameraMode::FC_LOOKAT_Y);
    int count = 0;
    for (int i = 0; i < 6; i++) {
        if (sideTransparent_[i]) {
            count++;
        }
    }
    text3D->SetText(position_.ToString() + " Sides transparent:" + String(count));
    text3D->SetFontSize(32);
    label_->SetPosition(Vector3(SIZE_X / 2, SIZE_Y, SIZE_Z / 2));
    scene_->AddChild(node_);

    MarkActive(false);
    SetActive();

    using namespace ChunkGenerated;
    VariantMap data = GetEventDataMap();
    data[P_POSITION] = position_;
    SendEvent(E_CHUNK_GENERATED);
}

void Chunk::RemoveNode()
{
    if (node_) {
        node_->Remove();
    }
    if (label_) {
        label_->Remove();
    }
    if (triggerNode_) {
        triggerNode_->Remove();
    }

    using namespace ChunkRemoved;
    VariantMap data = GetEventDataMap();
    data[P_POSITION] = position_;
    SendEvent(E_CHUNK_REMOVED);
}

bool Chunk::BlockHaveNeighbor(BlockSide side, int x, int y, int z)
{
    switch(side) {
        case BlockSide::TOP:
            if (y < SIZE_Y - 1) {
                if (data[x][y + 1][z] > 0) {
                    return true;
                }
            }
            return false;
        case BlockSide::BOTTOM:
            if (y > 0) {
                if (data[x][y - 1][z] > 0) {
                    return true;
                }
            }
            return false;
        case BlockSide::LEFT:
            if (x > 0) {
                if (data[x - 1][y][z] > 0) {
                    return true;
                }
            }
            return false;
        case BlockSide::RIGHT:
            if (x < SIZE_X - 1) {
                if (data[x + 1][y][z] > 0) {
                    return true;
                }
            }
            return false;
        case BlockSide::FRONT:
            if (z > 0) {
                if (data[x][y][z - 1] > 0) {
                    return true;
                }
            }
            return false;
        case BlockSide::BACK:
            if (z < SIZE_Z - 1) {
                if (data[x][y][z + 1] > 0) {
                    return true;
                }
            }
            return false;
    }
}

void Chunk::MarkForDeletion(bool value)
{
    shouldDelete_ = value;
}

bool Chunk::IsMarkedForDeletion()
{
    return shouldDelete_;
}

void Chunk::MarkActive(bool value)
{
    isActive_ = value;
}

bool Chunk::IsActive()
{
    return isActive_;
}

void Chunk::SetActive()
{
    if (node_) {
        auto shape = node_->GetComponent<CollisionShape>();
        if (shape) {
            shape->SetEnabled(isActive_);
        }
        auto body = node_->GetComponent<RigidBody>();
        if (body) {
            body->SetEnabled(isActive_);
        }
    }
}

void Chunk::UpdateVisibility()
{
    return;
    int visibleSides = 0;
    auto topChunk = GetSubsystem<VoxelWorld>()->GetChunkByPosition(position_ + Vector3::UP * SIZE_Y);
    if (topChunk && topChunk->IsSideTransparent(BlockSide::BOTTOM)) {
        visibleSides++;
    }
    auto bottomChunk = GetSubsystem<VoxelWorld>()->GetChunkByPosition(position_ + Vector3::DOWN * SIZE_Y);
    if (bottomChunk && bottomChunk->IsSideTransparent(BlockSide::TOP)) {
        visibleSides++;
    }
    auto leftChunk = GetSubsystem<VoxelWorld>()->GetChunkByPosition(position_ + Vector3::LEFT * SIZE_X);
    if (leftChunk && leftChunk->IsSideTransparent(BlockSide::RIGHT)) {
        visibleSides++;
    }
    auto rightChunk = GetSubsystem<VoxelWorld>()->GetChunkByPosition(position_ + Vector3::RIGHT * SIZE_X);
    if (rightChunk && rightChunk->IsSideTransparent(BlockSide::LEFT)) {
        visibleSides++;
    }
    auto frontChunk = GetSubsystem<VoxelWorld>()->GetChunkByPosition(position_ + Vector3::FORWARD * SIZE_Z);
    if (frontChunk && frontChunk->IsSideTransparent(BlockSide::BACK)) {
        visibleSides++;
    }
    auto backChunk = GetSubsystem<VoxelWorld>()->GetChunkByPosition(position_ + Vector3::BACK * SIZE_Z);
    if (backChunk && backChunk->IsSideTransparent(BlockSide::FRONT)) {
        visibleSides++;
    }
    if (label_) {
        label_->GetComponent<Text3D>()->SetText("Visible neighbor sides " + String(visibleSides));
    }
    URHO3D_LOGINFO("Chunkk update visibility " + position_.ToString() + " visible sides=" + String(visibleSides));
    visible_ = visibleSides > 0;
    if (!visibilityUpdate_) {
        visible_ = true;
    }
    if (geometry_) {
        geometry_->SetEnabled(visible_);
    }
}

bool Chunk::IsSideTransparent(BlockSide side)
{
    return sideTransparent_[side];
}

bool Chunk::IsVisible()
{
    return visible_;
}

SharedPtr<Chunk> Chunk::GetNeighbor(BlockSide side)
{
    switch(side) {
        case BlockSide::TOP:
            return GetSubsystem<VoxelWorld>()->GetChunkByPosition(position_ + Vector3::UP * SIZE_Y * 0.5);
        case BlockSide::BOTTOM:
            return GetSubsystem<VoxelWorld>()->GetChunkByPosition(position_ + Vector3::DOWN * SIZE_Y * 0.5);
        case BlockSide::LEFT:
            return GetSubsystem<VoxelWorld>()->GetChunkByPosition(position_ + Vector3::LEFT * SIZE_X * 0.5);
        case BlockSide::RIGHT:
            return GetSubsystem<VoxelWorld>()->GetChunkByPosition(position_ + Vector3::RIGHT * SIZE_X * 0.5);
        case BlockSide::FRONT:
            return GetSubsystem<VoxelWorld>()->GetChunkByPosition(position_ + Vector3::FORWARD * SIZE_Z * 0.5);
        case BlockSide::BACK:
            return GetSubsystem<VoxelWorld>()->GetChunkByPosition(position_ + Vector3::BACK * SIZE_Z * 0.5);
    }
}
//
//void Chunk::SetVisibility(bool value)
//{
//    visible_ = value;
//    if (geometry_) {
//        geometry_->SetEnabled(visible_);
//    }
//}