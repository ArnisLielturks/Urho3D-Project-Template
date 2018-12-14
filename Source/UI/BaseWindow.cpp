#include <Urho3D/Urho3DAll.h>
#include "BaseWindow.h"

/// Construct.
BaseWindow::BaseWindow(Context* context):
    Object(context)
{
    Init();
}

BaseWindow::~BaseWindow()
{
    Dispose();
}
