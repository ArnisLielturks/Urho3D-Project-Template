#include <Urho3D/Urho3DAll.h>
#include "BaseWindow.h"

/// Construct.
BaseWindow::BaseWindow(Context* context):
    Object(context),
    _active(true)
{
    SubscribeToBaseEvents();
    Init();
}

BaseWindow::~BaseWindow()
{
    Dispose();
}

void BaseWindow::Init()
{
}

void BaseWindow::SetActive(bool active)
{
    if (active) {
        SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(BaseWindow, HandleUpdate));
    }
    else {
        UnsubscribeFromEvent(E_UPDATE);
    }
    _active = active;
}

bool BaseWindow::IsActive()
{
    return _active;
}
