#pragma once

#include <Urho3D/Core/Object.h>

using namespace Urho3D;

class Maze : public Object {
URHO3D_OBJECT(Maze, Object);

public:
    Maze(Context* context);
    virtual ~Maze();

    static void RegisterObject(Context* context);


};
