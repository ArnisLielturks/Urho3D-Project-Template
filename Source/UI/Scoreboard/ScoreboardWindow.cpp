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
    _baseWindow->Remove();
}

void ScoreboardWindow::Init()
{
    Create();

    SubscribeToEvents();
}

void ScoreboardWindow::Create()
{
    _baseWindow = GetSubsystem<UI>()->GetRoot()->CreateChild<Window>();
    _baseWindow->SetStyleAuto();
    _baseWindow->SetAlignment(HA_CENTER, VA_CENTER);
    _baseWindow->SetSize(300, 300);
    _baseWindow->BringToFront();

}

void ScoreboardWindow::SubscribeToEvents()
{
}
