#pragma once

#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Window.h>
#include "../BaseWindow.h"

class PauseWindow : public BaseWindow
{
    URHO3D_OBJECT(PauseWindow, BaseWindow);

public:
    /// Construct.
    PauseWindow(Context* context);

    virtual ~PauseWindow();

    virtual void Init() override;

protected:

    virtual void Create() override;

private:

    void SubscribeToEvents();

    SharedPtr<Button> _continueButton;
    SharedPtr<Button> _mainMenuButton;
    SharedPtr<Button> _settingsButton;
    SharedPtr<Button> _exitButton;
    SharedPtr<Window> _baseWindow;

    Button* CreateButton(const String& text, int width, IntVector2 position);
};
