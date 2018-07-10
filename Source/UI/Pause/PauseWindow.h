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

    void HandleClose(StringHash eventType, VariantMap& eventData);
    void HandleExit(StringHash eventType, VariantMap& eventData);
    Button* CreateButton(String name, IntVector2 position, IntVector2 size, HorizontalAlignment hAlign, VerticalAlignment vAlign);

    SharedPtr<Button> _closeButton;
    SharedPtr<Button> _exitButton;
};