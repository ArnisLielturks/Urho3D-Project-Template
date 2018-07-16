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

struct GraphicsSettings {
	int width;
	int height;
	bool fullscreen;
	bool vsync;
	bool tripleBuffer;
	bool shadows;
	bool lowQualityShadows;
	int textureQuality;
	int textureAnistropy;
	int textureFilterMode;
	int multisample;
};

struct AudioSettings{
	bool enabled;
	bool stereo;
	bool soundInterpolation;
	int soundBuffer;
	int mixRate;
	float masterVolume;
	float effectsVolume;
	float ambientVolume;
	float voiceVolume;
	float musicVolume;
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

	void HandleGraphicsSettingsChange(StringHash eventType, VariantMap& eventData);
	void HandleGraphicsSettingsToggle(StringHash eventType, VariantMap& eventData);

	void HandleAudioSettingsToggle(StringHash eventType, VariantMap& eventData);
	void HandleAudioSettingsSlider(StringHash eventType, VariantMap& eventData);

    void ClearView();

	void InitGraphicsSettings();
	void InitAudioSettings();


    HashMap<int, SharedPtr<Button>> _buttons;

    SettingsViewType _openedView;

    Vector<SharedPtr<UIElement>> _activeSettingElements;

	GraphicsSettings _graphicsSettings;
	AudioSettings _audioSettings;

	HashMap<int, String> _textureQualityMapping;
	HashMap<int, String> _textureFilterModesMapping;
};