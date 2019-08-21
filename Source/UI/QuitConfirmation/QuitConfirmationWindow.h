#pragma once
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Window.h>
#include "../BaseWindow.h"

class QuitConfirmationWindow : public BaseWindow
{
    URHO3D_OBJECT(QuitConfirmationWindow, BaseWindow);

public:
    /// Construct.
    QuitConfirmationWindow(Context* context);

    virtual ~QuitConfirmationWindow();

    virtual void Init();

protected:

    virtual void Create();

private:

    void SubscribeToEvents();

    SharedPtr<Button> _yesButton;
    SharedPtr<Button> _noButton;
    SharedPtr<Window> _baseWindow;

    Button* CreateButton(const String& text, int width, IntVector2 position);
};