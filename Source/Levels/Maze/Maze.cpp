#include "Maze.h"
#include <Urho3D/Core/Context.h>
#include "MazeGenerator.h"

Maze::Maze(Context* context):
        Object(context)
{
}

Maze::~Maze()
{
}

void Maze::RegisterObject(Context* context)
{
    context->RegisterFactory<Maze>();
}
