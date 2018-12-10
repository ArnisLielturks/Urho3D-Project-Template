#include <Urho3D/Urho3DAll.h>
#include "QuitConfirmationWindow.h"
#include "../../MyEvents.h"
#include "../../Audio/AudioManagerDefs.h"

/// Construct.
QuitConfirmationWindow::QuitConfirmationWindow(Context* context) :
    BaseWindow(context),
    _active(true)
{
    Init();
}

QuitConfirmationWindow::~QuitConfirmationWindow()
{
}

void QuitConfirmationWindow::Init()
{
    Create();

    SubscribeToEvents();
}

void QuitConfirmationWindow::Create()
{
    Input* input = GetSubsystem<Input>();
    if (!input->IsMouseVisible()) {
        input->SetMouseVisible(true);
    }
}

void QuitConfirmationWindow::SubscribeToEvents()
{
}