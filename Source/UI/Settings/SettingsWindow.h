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
    void CreateControllerSettingsView();

    void HandleClose(StringHash eventType, VariantMap& eventData);
    void HandleSave(StringHash eventType, VariantMap& eventData);
    
    void ShowVideoSettings(StringHash eventType, VariantMap& eventData);
    void ShowAudioSettings(StringHash eventType, VariantMap& eventData);
    void ShowControllerSettings(StringHash eventType, VariantMap& eventData);

    void HandleChangeControls(StringHash eventType, VariantMap& eventData);
    void HandleControlsUpdated(StringHash eventType, VariantMap& eventData);

    void ClearView();

    SharedPtr<Button> _closeButton;
    SharedPtr<Button> _saveButton;
    SharedPtr<Button> _controlsTabButton;
    SharedPtr<Button> _audioTabButton;
    SharedPtr<Button> _graphicsTabButton;

    Vector<SharedPtr<UIElement>> _activeSettingElements;
};