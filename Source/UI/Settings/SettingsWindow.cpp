#include <Urho3D/Urho3DAll.h>
#include "SettingsWindow.h"
#include "../../MyEvents.h"
#include "../../Input/ControllerInput.h"
#include "../../Audio/AudioManagerDefs.h"

/// Construct.
SettingsWindow::SettingsWindow(Context* context) :
    BaseWindow(context),
	_openedView(SettingsViewType::CONTROLS_VIEW)
{

    Init();
	InitAudioSettings();
	InitGraphicsSettings();
}

SettingsWindow::~SettingsWindow()
{
}

void SettingsWindow::Init()
{
    Create();

    SubscribeToEvents();
}

void SettingsWindow::Create()
{
	URHO3D_LOGINFO("Settings Window created");
}

void SettingsWindow::SubscribeToEvents()
{
    SubscribeToEvent(MyEvents::E_INPUT_MAPPING_FINISHED, URHO3D_HANDLER(SettingsWindow, HandleControlsUpdated));
}

void SettingsWindow::SaveVideoSettings()
{
//    String resolution = _resoulutionVector.At(_graphicsSettingsNew.activeResolution);
//    StringVector dimensions = resolution.Split('x', false);
//    if (dimensions.Size() == 2) {
//        int width = ToInt(dimensions[0]);
//        _graphicsSettingsNew.width = width;
//
//        int height = ToInt(dimensions[1]);
//        _graphicsSettingsNew.height = height;
//    }

    _graphicsSettings = _graphicsSettingsNew;
	{
		using namespace AudioDefs;
		using namespace MyEvents::PlaySound;
		VariantMap data = GetEventDataMap();
		data[P_INDEX] = SOUND_EFFECTS::BUTTON_CLICK;
		data[P_TYPE] = SOUND_EFFECT;
		SendEvent(MyEvents::E_PLAY_SOUND, data);
	}
	Graphics* graphics = GetSubsystem<Graphics>();
	graphics->SetMode(
		_graphicsSettings.width,
		_graphicsSettings.height,
		_graphicsSettings.fullscreen,
		false,
		false,
		false,
		_graphicsSettings.vsync,
		_graphicsSettings.tripleBuffer,
		_graphicsSettings.multisample,
		0,
		0
	);
		
	auto* renderer = GetSubsystem<Renderer>();
	renderer->SetTextureFilterMode(TextureFilterMode(_graphicsSettings.textureFilterMode));
	renderer->SetTextureAnisotropy(_graphicsSettings.textureAnistropy);
	renderer->SetTextureQuality(static_cast<MaterialQuality>(_graphicsSettings.textureQuality));
	renderer->SetShadowQuality((ShadowQuality)_graphicsSettings.shadowQuality);
	renderer->SetDrawShadows(_graphicsSettings.shadows);

    GetSubsystem<ConfigManager>()->Set("engine", "WindowWidth", _graphicsSettingsNew.width);
    GetSubsystem<ConfigManager>()->Set("engine", "WindowHeight", _graphicsSettingsNew.height);
    GetSubsystem<ConfigManager>()->Set("engine", "VSync", (bool)_graphicsSettingsNew.vsync);
    GetSubsystem<ConfigManager>()->Set("engine", "Fullscreen", (bool)_graphicsSettingsNew.fullscreen);
    GetSubsystem<ConfigManager>()->Set("engine", "TripleBuffer", (bool)_graphicsSettingsNew.tripleBuffer);
    GetSubsystem<ConfigManager>()->Set("engine", "Shadows", (bool)_graphicsSettingsNew.shadows);

    if (_graphicsSettingsNew.textureQuality >= 3) {
        _graphicsSettingsNew.textureQuality = 15;
    }

    GetSubsystem<ConfigManager>()->Set("engine", "ShadowQuality", _graphicsSettingsNew.shadowQuality + 1);
    GetSubsystem<ConfigManager>()->Set("engine", "TextureQuality", _graphicsSettingsNew.textureQuality + 1);
    GetSubsystem<ConfigManager>()->Set("engine", "TextureAnisotropy", _graphicsSettingsNew.textureAnistropy + 1);
    GetSubsystem<ConfigManager>()->Set("engine", "TextureFilterMode", _graphicsSettingsNew.textureFilterMode + 1);
    GetSubsystem<ConfigManager>()->Set("engine", "MultiSample", _graphicsSettingsNew.multisample + 1);
    GetSubsystem<ConfigManager>()->Save(true);
}

void SettingsWindow::HandleControlsUpdated(StringHash eventType, VariantMap& eventData)
{
    Input* input = GetSubsystem<Input>();
    if (!input->IsMouseVisible()) {
        input->SetMouseVisible(true);
    }
}

void SettingsWindow::InitAudioSettings()
{
	_audioSettings.enabled = GetGlobalVar("Sound").GetBool();
	_audioSettings.stereo = GetGlobalVar("SoundStereo").GetBool() ? 1 : 0;
	_audioSettings.soundInterpolation = GetGlobalVar("SoundInterpolation").GetBool() ? 1 : 0;
	_audioSettings.mixRate = GetGlobalVar("SoundMixRate").GetFloat();
	_audioSettings.soundBuffer = GetGlobalVar("SoundBuffer").GetFloat();

	URHO3D_LOGINFOF("Audio %d", _audioSettings.stereo);
	_audioSettings.masterVolume = GetGlobalVar("SoundMasterVolume").GetFloat();
	_audioSettings.effectsVolume = GetGlobalVar("SoundEffectsVolume").GetFloat();
	_audioSettings.ambientVolume = GetGlobalVar("SoundAmbientVolume").GetFloat();
	_audioSettings.voiceVolume = GetGlobalVar("SoundVoiceVolume").GetFloat();
	_audioSettings.musicVolume = GetGlobalVar("SoundMusicVolume").GetFloat();

	_audioSettingsNew = _audioSettings;
}

void SettingsWindow::InitGraphicsSettings()
{
	_graphicsSettings.width = GetGlobalVar("WindowWidth").GetInt();
	_graphicsSettings.height = GetGlobalVar("WindowHeight").GetInt();
	_graphicsSettings.fullscreen = GetGlobalVar("Fullscreen").GetBool() ? 1 : 0;
	_graphicsSettings.vsync = GetGlobalVar("VSync").GetBool() ? 1 : 0;
	_graphicsSettings.tripleBuffer = GetGlobalVar("TripleBuffer").GetBool() ? 1 : 0;
	_graphicsSettings.shadows = GetGlobalVar("Shadows").GetBool() ? 1 : 0;
	_graphicsSettings.shadowQuality = GetGlobalVar("ShadowQuality").GetInt() - 1;
	_graphicsSettings.textureQuality = GetGlobalVar("TextureQuality").GetInt() - 1;
	_graphicsSettings.textureAnistropy = GetGlobalVar("TextureAnisotropy").GetInt() - 1;
	_graphicsSettings.textureFilterMode = GetGlobalVar("TextureFilterMode").GetInt() - 1 ;
	_graphicsSettings.multisample = Max(GetGlobalVar("MultiSample").GetInt() - 1, 0);

	String activeResolution = String(_graphicsSettings.width) + "x" + String(_graphicsSettings.height);
    auto graphics = GetSubsystem<Graphics>();

	URHO3D_LOGINFO("Active resolution " + activeResolution);
	PODVector<IntVector3> resolutions = graphics->GetResolutions(0);
	for (auto it = resolutions.Begin(); it != resolutions.End(); ++it) {
        if ((*it).x_ < 800 || (*it).y_ < 720) {
            continue;
        }
	    String resolution = String((*it).x_) + "x" + String((*it).y_);
//	    if (_resoulutionVector.Find(resolution) == _resoulutionVector.End()) {
//            _resoulutionVector.Push(resolution);
//
//            if (resolution == activeResolution) {
//                _graphicsSettings.activeResolution = _resoulutionVector.Size() - 1;
//            }
//	    }
	}

    _graphicsSettingsNew = _graphicsSettings;
}

void SettingsWindow::DrawControlsSettings()
{
    ControllerInput* controllerInput = GetSubsystem<ControllerInput>();
    HashMap<int, String> controlNames = controllerInput->GetControlNames();

}

void SettingsWindow::DrawVideoSettings()
{
}

void SettingsWindow::DrawAudioSettings()
{
}

void SettingsWindow::ApplyAudioSettings()
{
	Audio* audio = GetSubsystem<Audio>();
    bool shouldSave = false;
	if (_audioSettingsNew.masterVolume != _audioSettings.masterVolume) {
		_audioSettings.masterVolume = _audioSettingsNew.masterVolume;
		audio->SetMasterGain(SOUND_MASTER, _audioSettings.masterVolume);
		SetGlobalVar("SoundMasterVolume", _audioSettings.masterVolume);
        GetSubsystem<ConfigManager>()->Set("audio", "SoundMasterVolume", _audioSettings.masterVolume);
		URHO3D_LOGINFO("Applying master volume");
        shouldSave = true;
	}

	if (_audioSettingsNew.effectsVolume != _audioSettings.effectsVolume) {
		_audioSettings.effectsVolume = _audioSettingsNew.effectsVolume;
		audio->SetMasterGain(SOUND_EFFECT, _audioSettings.effectsVolume);
		SetGlobalVar("SoundEffectsVolume", _audioSettings.effectsVolume);
        GetSubsystem<ConfigManager>()->Set("audio", "SoundEffectsVolume", _audioSettings.effectsVolume);
		URHO3D_LOGINFO("Applying effects volume");
        shouldSave = true;
	}

	if (_audioSettingsNew.ambientVolume != _audioSettings.ambientVolume) {
		_audioSettings.ambientVolume = _audioSettingsNew.ambientVolume;
		audio->SetMasterGain(SOUND_AMBIENT, _audioSettings.ambientVolume);
		SetGlobalVar("SoundAmbientVolume", _audioSettings.ambientVolume);
        GetSubsystem<ConfigManager>()->Set("audio", "SoundAmbientVolume", _audioSettings.ambientVolume);
		URHO3D_LOGINFO("Applying ambient volume");
        shouldSave = true;
	}
	if (_audioSettingsNew.voiceVolume != _audioSettings.voiceVolume) {
		_audioSettings.voiceVolume = _audioSettingsNew.voiceVolume;
		audio->SetMasterGain(SOUND_VOICE, _audioSettings.voiceVolume);
		SetGlobalVar("SoundVoiceVolume", _audioSettings.voiceVolume);
        GetSubsystem<ConfigManager>()->Set("audio", "SoundVoiceVolume", _audioSettings.voiceVolume);
		URHO3D_LOGINFO("Applying voice volume");
        shouldSave = true;
	}
	if (_audioSettingsNew.musicVolume != _audioSettings.musicVolume) {
		_audioSettings.musicVolume = _audioSettingsNew.musicVolume;
		audio->SetMasterGain(SOUND_MUSIC, _audioSettings.musicVolume);
		SetGlobalVar("SoundMusicVolume", _audioSettings.musicVolume);
        GetSubsystem<ConfigManager>()->Set("audio", "SoundMusicVolume", _audioSettings.musicVolume);
		URHO3D_LOGINFO("Applying music volume");
        shouldSave = true;
	}
	if (_audioSettingsNew.soundBuffer != _audioSettings.soundBuffer ||
		_audioSettingsNew.mixRate != _audioSettings.mixRate ||
		_audioSettingsNew.stereo != _audioSettings.stereo ||
		_audioSettingsNew.soundInterpolation != _audioSettings.soundInterpolation) {

		_audioSettings.soundBuffer = _audioSettingsNew.soundBuffer;
		_audioSettings.mixRate = _audioSettingsNew.mixRate;
		_audioSettings.stereo = _audioSettingsNew.stereo;
		_audioSettings.soundInterpolation = _audioSettingsNew.soundInterpolation;

		audio->SetMode(_audioSettings.soundBuffer, _audioSettings.mixRate, _audioSettings.stereo, _audioSettings.soundInterpolation);

		SetGlobalVar("SoundStereo", _audioSettings.stereo);
		SetGlobalVar("SoundInterpolation", _audioSettings.soundInterpolation);
		SetGlobalVar("SoundMixRate", _audioSettings.mixRate);
		SetGlobalVar("SoundBuffer", _audioSettings.soundBuffer);

        GetSubsystem<ConfigManager>()->Set("audio", "SoundStereo", (bool)_audioSettings.stereo);
        GetSubsystem<ConfigManager>()->Set("audio", "SoundInterpolation", (bool)_audioSettings.soundInterpolation);
        GetSubsystem<ConfigManager>()->Set("audio", "SoundMixRate", _audioSettings.mixRate);
        GetSubsystem<ConfigManager>()->Set("audio", "SoundBuffer", _audioSettings.soundBuffer);

		URHO3D_LOGINFO("Applying audio other");
        shouldSave = true;
	}

    if (shouldSave) {
        GetSubsystem<ConfigManager>()->Save(true);
    }

	SetGlobalVar("Sound", _audioSettings.enabled);
}

void SettingsWindow::DrawMouseSettings()
{
}

void SettingsWindow::DrawJoystickSettings()
{
}

void SettingsWindow::InitMouseSettings()
{
    auto controllerInput = GetSubsystem<ControllerInput>();
    _controllerSettings.sensitivityX = controllerInput->GetSensitivityX(ControllerType::MOUSE);
    _controllerSettings.sensitivityY = controllerInput->GetSensitivityY(ControllerType::MOUSE);
    _controllerSettings.invertX = controllerInput->GetInvertX(ControllerType::MOUSE);
    _controllerSettings.invertY = controllerInput->GetInvertY(ControllerType::MOUSE);
    _controllerSettingsNew = _controllerSettings;
}

void SettingsWindow::InitJoystickSettings()
{
    auto controllerInput = GetSubsystem<ControllerInput>();
    _controllerSettings.sensitivityX = controllerInput->GetSensitivityX(ControllerType::JOYSTICK);
    _controllerSettings.sensitivityY = controllerInput->GetSensitivityY(ControllerType::JOYSTICK);
    _controllerSettings.invertX = controllerInput->GetInvertX(ControllerType::JOYSTICK);
    _controllerSettings.invertY = controllerInput->GetInvertY(ControllerType::JOYSTICK);
    _controllerSettingsNew = _controllerSettings;
}

void SettingsWindow::ApplyControllerSettings()
{
    if (_controllerSettings.invertX == _controllerSettingsNew.invertX &&
        _controllerSettings.invertY == _controllerSettingsNew.invertY &&
        _controllerSettings.sensitivityX == _controllerSettingsNew.sensitivityX &&
        _controllerSettings.sensitivityY == _controllerSettingsNew.sensitivityY) {
        return;
    }

    _controllerSettings = _controllerSettingsNew;

    URHO3D_LOGINFO("Saving controller settings " + String((int)_openedView));
    URHO3D_LOGINFO("====== " + String(_controllerSettings.sensitivityX));
    if (_openedView == SettingsViewType::MOUSE_VIEW) {
        auto controllerInput = GetSubsystem<ControllerInput>();
        controllerInput->SetSensitivityX(_controllerSettings.sensitivityX, ControllerType::MOUSE);
        controllerInput->SetSensitivityY(_controllerSettings.sensitivityX, ControllerType::MOUSE);
        controllerInput->SetInvertX(_controllerSettings.invertX, ControllerType::MOUSE);
        controllerInput->SetInvertY(_controllerSettings.invertY, ControllerType::MOUSE);


        GetSubsystem<ConfigManager>()->Set("mouse", "Sensitivity", _controllerSettings.sensitivityX);
        GetSubsystem<ConfigManager>()->Set("mouse", "InvertX", (bool)_controllerSettings.invertX);
        GetSubsystem<ConfigManager>()->Set("mouse", "InvertY", (bool)_controllerSettings.invertY);
        GetSubsystem<ConfigManager>()->Save(true);
    }
    else {
        auto controllerInput = GetSubsystem<ControllerInput>();
        controllerInput->SetSensitivityX(_controllerSettings.sensitivityX, ControllerType::JOYSTICK);
        controllerInput->SetSensitivityY(_controllerSettings.sensitivityY, ControllerType::JOYSTICK);
        controllerInput->SetInvertX(_controllerSettings.invertX, ControllerType::JOYSTICK);
        controllerInput->SetInvertY(_controllerSettings.invertY, ControllerType::JOYSTICK);

        GetSubsystem<ConfigManager>()->Set("joystick", "SensitivityX", _controllerSettings.sensitivityX);
        GetSubsystem<ConfigManager>()->Set("joystick", "SensitivityY", _controllerSettings.sensitivityY);
        GetSubsystem<ConfigManager>()->Set("joystick", "InvertX", (bool)_controllerSettings.invertX);
        GetSubsystem<ConfigManager>()->Set("joystick", "InvertY", (bool)_controllerSettings.invertY);
        GetSubsystem<ConfigManager>()->Save(true);
    }

}
