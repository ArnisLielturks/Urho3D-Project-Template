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

    void CreateGraphicsSettingsView();
    void CreateAudioSettingsView();
    void CreatePlayerSettingsView();

    void HandleClose(StringHash eventType, VariantMap& eventData);
    void HandleSave(StringHash eventType, VariantMap& eventData);
    
    void ShowVideoSettings(StringHash eventType, VariantMap& eventData);
    void ShowAudioSettings(StringHash eventType, VariantMap& eventData);
    void ShowPlayerSettings(StringHash eventType, VariantMap& eventData);

    void ClearView();

    SharedPtr<UIElement> CreateMenu(const String& label, StringVector options, int selected, IntVector2 position/*, EventHandler* handler*/);
    Button* CreateButton(String name, IntVector2 position, IntVector2 size, HorizontalAlignment hAlign, VerticalAlignment vAlign);
    SharedPtr<UIElement> CreateCheckbox(const String& label, bool isActive, IntVector2 position/*, EventHandler* handler*/);
	SharedPtr<UIElement> CreateSlider(const String& text, IntVector2 position, float value);

    SharedPtr<Button> _closeButton;
    SharedPtr<Button> _saveButton;
    SharedPtr<Button> _playerTabButton;
    SharedPtr<Button> _audioTabButton;
    SharedPtr<Button> _graphicsTabButton;

    Vector<SharedPtr<UIElement>> _activeSettingElements;
};