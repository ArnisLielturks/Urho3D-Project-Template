#pragma once

#include <Urho3D/Urho3DAll.h>
#include "../BaseWindow.h"

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

    SharedPtr<Button> _newGameButton;
    SharedPtr<Button> _exitWindow;
    SharedPtr<Window> _baseWindow;

    Button* CreateButton(const String& text, int width, IntVector2 position);
};