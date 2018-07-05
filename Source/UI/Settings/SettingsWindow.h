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
    void ChangeVideoSettings(StringHash eventType, VariantMap& eventData);
    SharedPtr<DropDownList> CreateMenu(const String& label, const char** items, IntVector2 position/*, EventHandler* handler*/);

    Button* CreateButton(String name, IntVector2 position, IntVector2 size, HorizontalAlignment hAlign, VerticalAlignment vAlign);

    SharedPtr<Button> _closeButton;
    SharedPtr<Button> _playerTabButton;
    SharedPtr<Button> _audioTabButton;
    SharedPtr<Button> _graphicsTabButton;
};