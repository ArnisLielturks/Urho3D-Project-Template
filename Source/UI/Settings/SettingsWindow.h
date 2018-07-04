#pragma once

#include <Urho3D/Urho3DAll.h>
#include "../BaseWindow.h"

class SettingsWindow : public BaseWindow
{
    URHO3D_OBJECT(SettingsWindow, BaseWindow);

public:
    /// Construct.
    SettingsWindow(Context* context);

    virtual ~SettingsWindow();

    virtual void Init();

protected:

    virtual void Create();

private:

    void SubscribeToEvents();

    void HandleClose(StringHash eventType, VariantMap& eventData);

    SharedPtr<Button> _closeButton;
};