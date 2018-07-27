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
    VIDEO_VIEW,
    MOUSE_VIEW,
    JOYSTICK_VIEW
};

struct GraphicsSettings {
	int width;
	int height;
	int fullscreen;
	int vsync;
	int tripleBuffer;
	int shadows;
	int shadowQuality;
	int textureQuality;
	int textureAnistropy;
	int textureFilterMode;
	int multisample;
    int activeResolution;
};

struct AudioSettings{
	bool enabled;
	int stereo;
	int soundInterpolation;
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

    void SaveVideoSettings();

	void HandleUpdate(StringHash eventType, VariantMap& eventData) override;
	void DrawWindow();
	void DrawControlsSettings();
	void DrawVideoSettings();
	void DrawAudioSettings();
    void DrawMouseSettings();
    void DrawJoystickSettings();

    void HandleControlsUpdated(StringHash eventType, VariantMap& eventData);

	void InitGraphicsSettings();
	void InitAudioSettings();
	void ApplyAudioSettings();

    HashMap<int, SharedPtr<Button>> _buttons;

    SettingsViewType _openedView;

    Vector<SharedPtr<UIElement>> _activeSettingElements;

	GraphicsSettings _graphicsSettings;
	GraphicsSettings _graphicsSettingsNew;

	AudioSettings _audioSettings;
	AudioSettings _audioSettingsNew;

	char **_supportedResolutions;
    StringVector _resoulutionVector;
};