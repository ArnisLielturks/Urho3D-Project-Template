#pragma once

#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Window.h>
#include <Urho3D/UI/Text.h>
#include "../BaseWindow.h"

class PopupMessageWindow : public BaseWindow
{
    URHO3D_OBJECT(PopupMessageWindow, BaseWindow);

public:
    PopupMessageWindow(Context* context);

    virtual ~PopupMessageWindow();

    virtual void Init() override;

protected:

    virtual void Create() override;

private:

    void SubscribeToEvents();

    SharedPtr<Button> okButton_;
    SharedPtr<Window> baseWindow_;

    Button* CreateButton(const String& text, int width, IntVector2 position);
    Text* CreateLabel(const String& text, int fontSize);
};
