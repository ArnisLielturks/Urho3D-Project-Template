#include <Urho3D/Urho3DAll.h>
#include "SettingsWindow.h"
#include "../../MyEvents.h"
#include "../../Input/ControllerInput.h"
#include "../../Audio/AudioManagerDefs.h"
#include "../../UI/NuklearUI.h"

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
    //for (int i = 0; i < _resoulutionVector.Size(); i++) {
    //    delete _supportedResolutions[i];
    //}
    delete _supportedResolutions;
}

void SettingsWindow::Init()
{
    Create();

    SubscribeToEvents();
}

void SettingsWindow::Create()
{
}

void SettingsWindow::SubscribeToEvents()
{
    SubscribeToEvent(MyEvents::E_INPUT_MAPPING_FINISHED, URHO3D_HANDLER(SettingsWindow, HandleControlsUpdated));
	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(SettingsWindow, HandleUpdate));
}

void SettingsWindow::SaveVideoSettings()
{
    String resolution = _resoulutionVector.At(_graphicsSettingsNew.activeResolution);
    StringVector dimensions = resolution.Split('x', false);
    if (dimensions.Size() == 2) {
        int width = ToInt(dimensions[0]);
        _graphicsSettingsNew.width = width;

        int height = ToInt(dimensions[1]);
        _graphicsSettingsNew.height = height;
    }

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
	renderer->SetTextureQuality(_graphicsSettings.textureQuality);
	renderer->SetShadowQuality((ShadowQuality)_graphicsSettings.shadowQuality);
	renderer->SetDrawShadows(_graphicsSettings.shadows);

	//SendEvent(MyEvents::E_SAVE_CONFIG);
	//VariantMap data = GetEventDataMap();
	//data["Title"] = "Settings saved!";
	//data["Message"] = "You must restart the game for\nthe changes to take effect";
	//SendEvent("ShowAlertMessage", data);
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
	_graphicsSettings.multisample = GetGlobalVar("Multisample").GetInt() - 1;

	String activeResolution = String(_graphicsSettings.width) + "x" + String(_graphicsSettings.height);
    auto graphics = GetSubsystem<Graphics>();

	URHO3D_LOGINFO("Active resolution " + activeResolution);
	PODVector<IntVector3> resolutions = graphics->GetResolutions(0);
	for (auto it = resolutions.Begin(); it != resolutions.End(); ++it) {
	    String resolution = String((*it).x_) + "x" + String((*it).y_);
	    if (_resoulutionVector.Find(resolution) == _resoulutionVector.End()) {
            _resoulutionVector.Push(resolution);

            if (resolution == activeResolution) {
                _graphicsSettings.activeResolution = _resoulutionVector.Size() - 1;
            }
	    }
	}

    _supportedResolutions = new char*[_resoulutionVector.Size()];
    for (int i = 0; i < _resoulutionVector.Size(); i++) {

        _supportedResolutions[i] = new char[_resoulutionVector.At(i).Length()];
        sprintf(_supportedResolutions[i], "%s", _resoulutionVector.At(i).CString());
    }

    _graphicsSettingsNew = _graphicsSettings;
}

void SettingsWindow::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	DrawWindow();
}

void SettingsWindow::DrawWindow()
{
	auto nuklear = GetSubsystem<NuklearUI>();
	auto ctx = nuklear->GetNkContext();

    nk_style_default(ctx);

    if (nk_begin(ctx, "Settings", nk_rect(10, 10, 500, 500), NK_WINDOW_BORDER | NK_WINDOW_CLOSABLE | NK_WINDOW_MOVABLE)) {
		/* menubar */
		enum menu_states {MENU_DEFAULT, MENU_WINDOWS};
		static nk_size mprog = 60;
		static int mslider = 10;
		static int mcheck = nk_true;
		nk_menubar_begin(ctx);

		int sections = 3;
		const float singleButtonWidth = 1.0f / (float) sections;
		/* menu #1 */
		nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 3);

		nk_layout_row_push(ctx, singleButtonWidth);
		if (nk_button_label(ctx, "Controls")) {
			_openedView = SettingsViewType::CONTROLS_VIEW;
        }
		nk_button_set_behavior(nuklear->GetNkContext(), NK_BUTTON_DEFAULT);
		
		nk_layout_row_push(ctx, singleButtonWidth);
		if (nk_button_label(ctx, "Audio")) {
            _openedView = SettingsViewType::AUDIO_VIEW;
        }
		nk_button_set_behavior(nuklear->GetNkContext(), NK_BUTTON_DEFAULT);

		nk_layout_row_push(ctx, singleButtonWidth);
		if (nk_button_label(ctx, "Video")) {
            _openedView = SettingsViewType::VIDEO_VIEW;
        }
		nk_button_set_behavior(nuklear->GetNkContext(), NK_BUTTON_DEFAULT);
		nk_menubar_end(ctx);

		if (_openedView == SettingsViewType::AUDIO_VIEW) {
			DrawAudioSettings();
		}
		else if (_openedView == SettingsViewType::VIDEO_VIEW) {
			DrawVideoSettings();
		}
		else if (_openedView == SettingsViewType::CONTROLS_VIEW) {
			DrawControlsSettings();
		}
    }

    nk_end(nuklear->GetNkContext());

    if (nk_window_is_hidden(nuklear->GetNkContext(), "Settings")) {

		VariantMap data = GetEventDataMap();
		data["Name"] = "SettingsWindow";
		SendEvent(MyEvents::E_CLOSE_WINDOW, data);
    }
}

void SettingsWindow::DrawControlsSettings()
{
    ControllerInput* controllerInput = GetSubsystem<ControllerInput>();
    HashMap<int, String> controlNames = controllerInput->GetControlNames();
    auto nuklear = GetSubsystem<NuklearUI>();

    for (auto it = controlNames.Begin(); it != controlNames.End(); ++it) {
        nk_layout_row_begin(nuklear->GetNkContext(), NK_DYNAMIC, 30, 2);
        {
            nk_layout_row_push(nuklear->GetNkContext(), 0.5f);
            nk_label(nuklear->GetNkContext(), (*it).second_.CString(), NK_TEXT_LEFT);

            nk_layout_row_push(nuklear->GetNkContext(), 0.5f);
            if (nk_button_label(nuklear->GetNkContext(), controllerInput->GetActionKeyName((*it).first_).CString()) && !controllerInput->IsMappingInProgress()) {
                using namespace MyEvents::StartInputMapping;
                VariantMap data = GetEventDataMap();
                data[P_CONTROL_ACTION] = (*it).second_;
                SendEvent(MyEvents::E_START_INPUT_MAPPING, data);

				Input* input = GetSubsystem<Input>();
				if (input->IsMouseVisible()) {
					input->SetMouseVisible(false);
				}
            }
            nk_button_set_behavior(nuklear->GetNkContext(), NK_BUTTON_DEFAULT);
        }
        nk_layout_row_end(nuklear->GetNkContext());
    }
}

void SettingsWindow::DrawVideoSettings()
{
	auto nuklear = GetSubsystem<NuklearUI>();
	nk_layout_row_begin(nuklear->GetNkContext(), NK_DYNAMIC, 30, 1);
	{
		nk_layout_row_push(nuklear->GetNkContext(), 1.0f);
		// nk_label(nuklear->GetNkContext(), "Stereo", NK_TEXT_LEFT);
		// nk_layout_row_push(nuklear->GetNkContext(), 0.5f);
		nk_checkbox_label(nuklear->GetNkContext(), "FullScreen", &_graphicsSettingsNew.fullscreen);
	}
    nk_layout_row_end(nuklear->GetNkContext());
    // ---------------------
	nk_layout_row_begin(nuklear->GetNkContext(), NK_DYNAMIC, 30, 1);
	{
		nk_layout_row_push(nuklear->GetNkContext(), 1.0f);
		// nk_label(nuklear->GetNkContext(), "Stereo", NK_TEXT_LEFT);
		// nk_layout_row_push(nuklear->GetNkContext(), 0.5f);
		nk_checkbox_label(nuklear->GetNkContext(), "VSync", &_graphicsSettingsNew.vsync);
	}
    nk_layout_row_end(nuklear->GetNkContext());
    // ---------------------
	nk_layout_row_begin(nuklear->GetNkContext(), NK_DYNAMIC, 30, 1);
	{
		nk_layout_row_push(nuklear->GetNkContext(), 1.0f);
		// nk_label(nuklear->GetNkContext(), "Stereo", NK_TEXT_LEFT);
		// nk_layout_row_push(nuklear->GetNkContext(), 0.5f);
		nk_checkbox_label(nuklear->GetNkContext(), "Triple buffer", &_graphicsSettingsNew.tripleBuffer);
	}
    nk_layout_row_end(nuklear->GetNkContext());
    // ---------------------
	nk_layout_row_begin(nuklear->GetNkContext(), NK_DYNAMIC, 30, 1);
	{
		nk_layout_row_push(nuklear->GetNkContext(), 1.0f);
		// nk_label(nuklear->GetNkContext(), "Stereo", NK_TEXT_LEFT);
		// nk_layout_row_push(nuklear->GetNkContext(), 0.5f);
		nk_checkbox_label(nuklear->GetNkContext(), "Shadows", &_graphicsSettingsNew.shadows);
	}
    nk_layout_row_end(nuklear->GetNkContext());
    // ---------------------

    static const char *shadowModes[] = { "Bad", "Low", "Medium", "High", "Ultra", "HD"};

    if (_graphicsSettingsNew.shadowQuality > NK_LEN(shadowModes)) {
        _graphicsSettingsNew.shadowQuality = NK_LEN(shadowModes) - 1;
    }
    nk_layout_row_dynamic(nuklear->GetNkContext(), 25, 2);
    nk_label(nuklear->GetNkContext(), "Shadow quality", NK_TEXT_LEFT);
    _graphicsSettingsNew.shadowQuality = nk_combo(nuklear->GetNkContext(), shadowModes, NK_LEN(shadowModes), _graphicsSettingsNew.shadowQuality, 25, nk_vec2(180, 200));

    // ---------------------

	nk_layout_row_dynamic(nuklear->GetNkContext(), 25, 2);
	nk_label(nuklear->GetNkContext(), "Resolution", NK_TEXT_LEFT);
    const char** resArray = const_cast<const char**>(_supportedResolutions);
	_graphicsSettingsNew.activeResolution = nk_combo(nuklear->GetNkContext(), resArray, _resoulutionVector.Size(), _graphicsSettingsNew.activeResolution, 25, nk_vec2(180,200));

    // ---------------------

    static const char *qualityModes[] = { "Low","Medium","High","Ultra" };
	if (_graphicsSettingsNew.textureQuality > NK_LEN(qualityModes)) {
		_graphicsSettingsNew.textureQuality = NK_LEN(qualityModes) - 1;
	}
	nk_layout_row_dynamic(nuklear->GetNkContext(), 25, 2);
	nk_label(nuklear->GetNkContext(), "Texture quality", NK_TEXT_LEFT);
	_graphicsSettingsNew.textureQuality = nk_combo(nuklear->GetNkContext(), qualityModes, NK_LEN(qualityModes), _graphicsSettingsNew.textureQuality, 25, nk_vec2(180,200));

    // ---------------------

	static const char *textureFilterModes[] = {"Nearest", "Bilinear","Trilinear","Anistropic","Nearest Anistropic", "Default", "Max"};

	nk_layout_row_dynamic(nuklear->GetNkContext(), 25, 2);
	nk_label(nuklear->GetNkContext(), "Texture filter mode", NK_TEXT_LEFT);
	_graphicsSettingsNew.textureFilterMode = nk_combo(nuklear->GetNkContext(), textureFilterModes, NK_LEN(textureFilterModes), _graphicsSettingsNew.textureFilterMode, 25, nk_vec2(180,200));

    // ---------------------

    static const char *textureAnisotropyLevel[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16" };

    nk_layout_row_dynamic(nuklear->GetNkContext(), 25, 2);
    nk_label(nuklear->GetNkContext(), "Texture anistropy level", NK_TEXT_LEFT);
    _graphicsSettingsNew.textureAnistropy = nk_combo(nuklear->GetNkContext(), textureAnisotropyLevel, NK_LEN(textureAnisotropyLevel), _graphicsSettingsNew.textureAnistropy, 25, nk_vec2(180, 200));

    // ---------------------

    static const char *multisampleLevel[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16" };

    nk_layout_row_dynamic(nuklear->GetNkContext(), 25, 2);
    nk_label(nuklear->GetNkContext(), "Multisample", NK_TEXT_LEFT);
    _graphicsSettingsNew.multisample = nk_combo(nuklear->GetNkContext(), multisampleLevel, NK_LEN(multisampleLevel), _graphicsSettingsNew.multisample, 25, nk_vec2(180, 200));


    nk_layout_row_dynamic(nuklear->GetNkContext(), 25, 2);
    if (nk_button_label(nuklear->GetNkContext(), "Save")) {
        SaveVideoSettings();
    }
    nk_button_set_behavior(nuklear->GetNkContext(), NK_BUTTON_DEFAULT);
}

void SettingsWindow::DrawAudioSettings()
{
    auto nuklear = GetSubsystem<NuklearUI>();

	/* custom widget pixel width */
	nk_layout_row_begin(nuklear->GetNkContext(), NK_DYNAMIC, 30, 2);
	{
		nk_layout_row_push(nuklear->GetNkContext(), 0.5f);
		nk_label(nuklear->GetNkContext(), "Master volume", NK_TEXT_LEFT);
		nk_layout_row_push(nuklear->GetNkContext(), 0.5f);
		nk_slider_float(nuklear->GetNkContext(), 0, &_audioSettingsNew.masterVolume, 1.0f, 0.05f);
	}
    nk_layout_row_end(nuklear->GetNkContext());

	nk_layout_row_begin(nuklear->GetNkContext(), NK_DYNAMIC, 30, 2);
	{
		nk_layout_row_push(nuklear->GetNkContext(), 0.5f);
		nk_label(nuklear->GetNkContext(), "Effects volume", NK_TEXT_LEFT);
		nk_layout_row_push(nuklear->GetNkContext(), 0.5f);
		nk_slider_float(nuklear->GetNkContext(), 0, &_audioSettingsNew.effectsVolume, 1.0f, 0.05f);
	}
    nk_layout_row_end(nuklear->GetNkContext());

	nk_layout_row_begin(nuklear->GetNkContext(), NK_DYNAMIC, 30, 2);
	{
		nk_layout_row_push(nuklear->GetNkContext(), 0.5f);
		nk_label(nuklear->GetNkContext(), "Music volume", NK_TEXT_LEFT);
		nk_layout_row_push(nuklear->GetNkContext(), 0.5f);
		nk_slider_float(nuklear->GetNkContext(), 0, &_audioSettingsNew.musicVolume, 1.0f, 0.05f);
	}
    nk_layout_row_end(nuklear->GetNkContext());

	nk_layout_row_begin(nuklear->GetNkContext(), NK_DYNAMIC, 30, 2);
	{
		nk_layout_row_push(nuklear->GetNkContext(), 0.5f);
		nk_label(nuklear->GetNkContext(), "Ambient volume", NK_TEXT_LEFT);
		nk_layout_row_push(nuklear->GetNkContext(), 0.5f);
		nk_slider_float(nuklear->GetNkContext(), 0, &_audioSettingsNew.ambientVolume, 1.0f, 0.05f);
	}
    nk_layout_row_end(nuklear->GetNkContext());

	nk_layout_row_begin(nuklear->GetNkContext(), NK_DYNAMIC, 30, 2);
	{
		nk_layout_row_push(nuklear->GetNkContext(), 0.5f);
		nk_label(nuklear->GetNkContext(), "Voice volume", NK_TEXT_LEFT);
		nk_layout_row_push(nuklear->GetNkContext(), 0.5f);
		nk_slider_float(nuklear->GetNkContext(), 0, &_audioSettingsNew.voiceVolume, 1.0f, 0.05f);
	}
    nk_layout_row_end(nuklear->GetNkContext());

	nk_layout_row_begin(nuklear->GetNkContext(), NK_DYNAMIC, 30, 1);
	{
		nk_layout_row_push(nuklear->GetNkContext(), 1.0f);
		// nk_label(nuklear->GetNkContext(), "Stereo", NK_TEXT_LEFT);
		// nk_layout_row_push(nuklear->GetNkContext(), 0.5f);
		nk_checkbox_label(nuklear->GetNkContext(), "Stereo", &_audioSettingsNew.stereo);
	}
    nk_layout_row_end(nuklear->GetNkContext());

	nk_layout_row_begin(nuklear->GetNkContext(), NK_DYNAMIC, 30, 1);
	{
		nk_layout_row_push(nuklear->GetNkContext(), 1.0f);
		// nk_label(nuklear->GetNkContext(), "Stereo", NK_TEXT_LEFT);
		// nk_layout_row_push(nuklear->GetNkContext(), 0.5f);
		nk_checkbox_label(nuklear->GetNkContext(), "Interpolation", &_audioSettingsNew.soundInterpolation);
	}
    nk_layout_row_end(nuklear->GetNkContext());

	ApplyAudioSettings();
}

void SettingsWindow::ApplyAudioSettings()
{
	Audio* audio = GetSubsystem<Audio>();

	if (_audioSettingsNew.masterVolume != _audioSettings.masterVolume) {
		_audioSettings.masterVolume = _audioSettingsNew.masterVolume;
		audio->SetMasterGain(SOUND_MASTER, _audioSettings.masterVolume);
		SetGlobalVar("SoundMasterVolume", _audioSettings.masterVolume);
		URHO3D_LOGINFO("Applying master volume");
	}

	if (_audioSettingsNew.effectsVolume != _audioSettings.effectsVolume) {
		_audioSettings.effectsVolume = _audioSettingsNew.effectsVolume;
		audio->SetMasterGain(SOUND_EFFECT, _audioSettings.effectsVolume);
		SetGlobalVar("SoundEffectsVolume", _audioSettings.effectsVolume);
		URHO3D_LOGINFO("Applying effects volume");
	}

	if (_audioSettingsNew.ambientVolume != _audioSettings.ambientVolume) {
		_audioSettings.ambientVolume = _audioSettingsNew.ambientVolume;
		audio->SetMasterGain(SOUND_AMBIENT, _audioSettings.ambientVolume);
		SetGlobalVar("SoundAmbientVolume", _audioSettings.ambientVolume);
		URHO3D_LOGINFO("Applying ambient volume");
	}
	if (_audioSettingsNew.voiceVolume != _audioSettings.voiceVolume) {
		_audioSettings.voiceVolume = _audioSettingsNew.voiceVolume;
		audio->SetMasterGain(SOUND_VOICE, _audioSettings.voiceVolume);
		SetGlobalVar("SoundVoiceVolume", _audioSettings.voiceVolume);
		URHO3D_LOGINFO("Applying voice volume");
	}
	if (_audioSettingsNew.musicVolume != _audioSettings.musicVolume) {
		_audioSettings.musicVolume = _audioSettingsNew.musicVolume;
		audio->SetMasterGain(SOUND_MUSIC, _audioSettings.musicVolume);
		SetGlobalVar("SoundMusicVolume", _audioSettings.musicVolume);
		URHO3D_LOGINFO("Applying music volume");
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

		URHO3D_LOGINFO("Applying audio other");
	}

	SetGlobalVar("Sound", _audioSettings.enabled);
}