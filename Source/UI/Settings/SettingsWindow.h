#pragma once

#include <Urho3D/Urho3DAll.h>
#include "../BaseWindow.h"

enum SettingsButtonType {
    CLOSE,
    SAVE,
    CONTROLS,
    AUDIO,
    VIDEO
};

enum SettingsViewType {
    CONTROLS_VIEW,
    AUDIO_VIEW,
    VIDEO_VIEW
};

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

    HashMap<int, SharedPtr<Button>> _buttons;

    SettingsViewType _openedView;

    Vector<SharedPtr<UIElement>> _activeSettingElements;
};