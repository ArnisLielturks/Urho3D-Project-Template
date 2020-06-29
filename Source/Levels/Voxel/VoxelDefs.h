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
    AIR,
    STONE,
    DIRT,
    SAND,
    COAL,
    LIGHT,
    NONE
};

struct VoxelBlock {
    BlockType type;
    Color color;
};