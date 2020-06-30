#pragma once
#include <Urho3D/Math/Color.h>

using namespace Urho3D;

enum BlockSide {
    TOP,
    BOTTOM,
    LEFT,
    RIGHT,
    FRONT,
    BACK
};

enum BlockType {
    BT_AIR,
    BT_STONE,
    BT_DIRT,
    BT_SAND,
    BT_COAL,
    BT_TORCH,
    BT_WOOD,
    BT_WATER,
    BT_NONE
};

enum Biome {
    B_GRASS,
    B_FOREST,
    B_SEA,
    B_MOUNTAINS,
    B_DESERT,
    B_NONE,
};

struct VoxelBlock {
    BlockType type;
};