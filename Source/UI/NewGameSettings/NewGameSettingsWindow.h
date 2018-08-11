#pragma once

#include <Urho3D/Urho3DAll.h>
#include "../BaseWindow.h"
#include "../NuklearUI.h"

class NewGameSettingsWindow : public BaseWindow
{
    URHO3D_OBJECT(NewGameSettingsWindow, BaseWindow);

public:
    /// Construct.
    NewGameSettingsWindow(Context* context);

    virtual ~NewGameSettingsWindow();

    virtual void Init();

protected:

    virtual void Create();

private:

    void SubscribeToEvents();

    void HandleUpdate(StringHash eventType, VariantMap& eventData) override;

    bool _active;
};