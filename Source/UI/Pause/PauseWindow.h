#pragma once

#include <Urho3D/Urho3DAll.h>
#include "../BaseWindow.h"

class PauseWindow : public BaseWindow
{
    URHO3D_OBJECT(PauseWindow, BaseWindow);

public:
    /// Construct.
    PauseWindow(Context* context);

    virtual ~PauseWindow();

    virtual void Init();

protected:

    virtual void Create();

private:

    void SubscribeToEvents();

    void HandleUpdate(StringHash eventType, VariantMap& eventData);

    bool _active;
};