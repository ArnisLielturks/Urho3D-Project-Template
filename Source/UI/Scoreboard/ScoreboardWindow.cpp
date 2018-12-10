#include <Urho3D/Urho3DAll.h>
#include "ScoreboardWindow.h"
#include "../../MyEvents.h"

/// Construct.
ScoreboardWindow::ScoreboardWindow(Context* context) :
    BaseWindow(context)
{
    Init();
}

ScoreboardWindow::~ScoreboardWindow()
{
}

void ScoreboardWindow::Init()
{
    Create();

    SubscribeToEvents();
}

void ScoreboardWindow::Create()
{
}

void ScoreboardWindow::SubscribeToEvents()
{
}
