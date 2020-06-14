#pragma once

#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Window.h>
#include "../BaseWindow.h"

class PauseWindow : public BaseWindow
{
    URHO3D_OBJECT(PauseWindow, BaseWindow);

public:
    PauseWindow(Context* context);

    virtual ~PauseWindow();

    virtual void Init() override;

protected:

    virtual void Create() override;

private:

    void SubscribeToEvents();

    SharedPtr<Button> continueButton_;
    SharedPtr<Button> mainMenuButton_;
    SharedPtr<Button> settingsButton_;
    SharedPtr<Button> exitButton_;
    SharedPtr<Window> baseWindow_;

    Button* CreateButton(const String& text, int width, IntVector2 position);
};
