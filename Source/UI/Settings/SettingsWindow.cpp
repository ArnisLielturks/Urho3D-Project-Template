#include <Urho3D/Urho3DAll.h>
#include "SettingsWindow.h"
#include "../../MyEvents.h"
#include "../../Input/ControllerInput.h"

/// Construct.
SettingsWindow::SettingsWindow(Context* context) :
    BaseWindow(context, IntVector2(400, 430))
{
	_textureQualityMapping[0] = "Low";
	_textureQualityMapping[1] = "Medium";
	_textureQualityMapping[2] = "High";
	_textureQualityMapping[15] = "Max";

	// FILTER_NEAREST = 0,
	// FILTER_BILINEAR = 1,
	// FILTER_TRILINEAR = 2,
	// FILTER_ANISOTROPIC = 3,
	// FILTER_NEAREST_ANISOTROPIC = 4,
	// FILTER_DEFAULT = 5,
	// MAX_FILTERMODES
	_textureFilterModesMapping[0] = "Nearest";
	_textureFilterModesMapping[1] = "Bilinear";
	_textureFilterModesMapping[2] = "Trilinear";
	_textureFilterModesMapping[3] = "Anistropic";
	_textureFilterModesMapping[4] = "Nearest Anistropic";
	_textureFilterModesMapping[5] = "Default";
	_textureFilterModesMapping[6] = "Max";
    Init();
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
    UI* ui = GetSubsystem<UI>();

    int margin = 10;
    int width = 110;

    _buttons[SettingsButtonType::CLOSE] = CreateButton(_base, "X", IntVector2(-5, 5), IntVector2(20, 20), HA_RIGHT, VA_TOP);

    int index = 0;
    _buttons[SettingsButtonType::CONTROLS] = CreateButton(_base, "Controls", IntVector2(margin + index * margin + index * width, 5), IntVector2(width, 20), HA_LEFT, VA_TOP);
    index++;
    _buttons[SettingsButtonType::AUDIO] = CreateButton(_base, "Audio", IntVector2(margin + index * margin + index * width, 5), IntVector2(width, 20), HA_LEFT, VA_TOP);
    index++;
    _buttons[SettingsButtonType::VIDEO] = CreateButton(_base, "Video", IntVector2(margin + index * margin + index * width, 5), IntVector2(width, 20), HA_LEFT, VA_TOP);
    index++;
    _buttons[SettingsButtonType::SAVE] = CreateButton(_base, "Save", IntVector2(-20, -20), IntVector2(width, 20), HA_RIGHT, VA_BOTTOM);

    _buttons[SettingsButtonType::SAVE]->SetVisible(false);
    CreateControllerSettingsView();
}

void SettingsWindow::CreateGraphicsSettingsView()
{
	InitGraphicsSettings();
    ClearView();
    _openedView = SettingsViewType::VIDEO_VIEW;
    auto* graphics = GetSubsystem<Graphics>();

    String activeResolution = String(_graphicsSettings.width) + "x" + String(_graphicsSettings.height);

    URHO3D_LOGINFO("Active resolution " + activeResolution);
    int activeIndex = 0;
    StringVector supportedResolutions;
    PODVector<IntVector3> resolutions = graphics->GetResolutions(0);
    for (auto it = resolutions.Begin(); it != resolutions.End(); ++it) {
        String resolution = String((*it).x_) + "x" + String((*it).y_);
        if (resolution == activeResolution) {
            activeIndex = supportedResolutions.Size() - 1;
        }
        if (supportedResolutions.Find(resolution) == supportedResolutions.End()) {
            supportedResolutions.Push(resolution);
        }
    }
    _activeSettingElements.Push(CreateMenu(_base, "Resolution", supportedResolutions, activeIndex, IntVector2(20, 60), URHO3D_HANDLER(SettingsWindow, HandleGraphicsSettingsChange)));
    _activeSettingElements.Push(CreateCheckbox(_base, "Fullscreen", _graphicsSettings.fullscreen, IntVector2(20, 90), URHO3D_HANDLER(SettingsWindow, HandleGraphicsSettingsToggle)));
    _activeSettingElements.Push(CreateCheckbox(_base, "Vertical sync", _graphicsSettings.vsync, IntVector2(20, 120), URHO3D_HANDLER(SettingsWindow, HandleGraphicsSettingsToggle)));
    _activeSettingElements.Push(CreateCheckbox(_base, "Triple buffer", _graphicsSettings.tripleBuffer, IntVector2(20, 150), URHO3D_HANDLER(SettingsWindow, HandleGraphicsSettingsToggle)));

    _activeSettingElements.Push(CreateCheckbox(_base, "Enable shadows", _graphicsSettings.shadows, IntVector2(20, 200), URHO3D_HANDLER(SettingsWindow, HandleGraphicsSettingsToggle)));
    _activeSettingElements.Push(CreateCheckbox(_base, "Low quality shadows", _graphicsSettings.lowQualityShadows, IntVector2(20, 230), URHO3D_HANDLER(SettingsWindow, HandleGraphicsSettingsToggle)));

    StringVector supportedQualitySettings;
    supportedQualitySettings.Push(_textureQualityMapping[0]); //0
    supportedQualitySettings.Push(_textureQualityMapping[1]); // 1
    supportedQualitySettings.Push(_textureQualityMapping[2]); // 2
    supportedQualitySettings.Push(_textureQualityMapping[15]); // 15
    int activeTextureQualityIndex = _graphicsSettings.textureQuality;
    if (activeTextureQualityIndex > 2) {
        activeTextureQualityIndex = 3;
    }
    _activeSettingElements.Push(CreateMenu(_base, "Texture quality", supportedQualitySettings, activeTextureQualityIndex, IntVector2(20, 260), URHO3D_HANDLER(SettingsWindow, HandleGraphicsSettingsChange)));

	int activeItem = 0;
    StringVector textureAnisotropySettings;
	for (int i = 1; i <= 16; i++) {
		textureAnisotropySettings.Push(String(i));
		if (i == _graphicsSettings.textureAnistropy) {
			activeItem = i - 1;
		}
	}
    _activeSettingElements.Push(CreateMenu(_base, "Texture anisotropy level", textureAnisotropySettings, activeItem, IntVector2(20, 290), URHO3D_HANDLER(SettingsWindow, HandleGraphicsSettingsChange)));

    StringVector textureFilterModes;
	int index = 0;
	for (auto it = _textureFilterModesMapping.Begin(); it != _textureFilterModesMapping.End(); ++it) {
		textureFilterModes.Push((*it).second_);
		if ((*it).first_ == _graphicsSettings.textureFilterMode) {
			activeItem = index;
		}
		index++;
	}
    _activeSettingElements.Push(CreateMenu(_base, "Texture filer mode", textureFilterModes, activeItem, IntVector2(20, 320), URHO3D_HANDLER(SettingsWindow, HandleGraphicsSettingsChange)));

	StringVector multisampleLevel;
	for (int i = 1; i <= 16; i++) {
		multisampleLevel.Push(String(i));
		if (i == _graphicsSettings.multisample) {
			activeItem = i - 1;
		}
	}
	_activeSettingElements.Push(CreateMenu(_base, "Multisample", multisampleLevel, activeItem, IntVector2(20, 350), URHO3D_HANDLER(SettingsWindow, HandleGraphicsSettingsChange)));
}

void SettingsWindow::CreateAudioSettingsView()
{
	InitAudioSettings();
    ClearView();
    _openedView = SettingsViewType::AUDIO_VIEW;
    //_activeSettingElements.Push(CreateCheckbox(_base, "Enable audio", _audioSettings.enabled, IntVector2(20, 60), URHO3D_HANDLER(SettingsWindow, HandleAudioSettingsToggle)));
    _activeSettingElements.Push(CreateCheckbox(_base, "Stereo", _audioSettings.stereo, IntVector2(20, 60), URHO3D_HANDLER(SettingsWindow, HandleAudioSettingsToggle)));
    _activeSettingElements.Push(CreateCheckbox(_base, "Sound interpolation", _audioSettings.soundInterpolation, IntVector2(20, 90), URHO3D_HANDLER(SettingsWindow, HandleAudioSettingsToggle)));
	_activeSettingElements.Push(CreateSlider(_base, "Master volume", IntVector2(20, 120), _audioSettings.masterVolume, URHO3D_HANDLER(SettingsWindow, HandleAudioSettingsSlider)));
	_activeSettingElements.Push(CreateSlider(_base, "Effects volume", IntVector2(20, 150), _audioSettings.effectsVolume, URHO3D_HANDLER(SettingsWindow, HandleAudioSettingsSlider)));
	_activeSettingElements.Push(CreateSlider(_base, "Ambient volume", IntVector2(20, 180), _audioSettings.ambientVolume, URHO3D_HANDLER(SettingsWindow, HandleAudioSettingsSlider)));
	_activeSettingElements.Push(CreateSlider(_base, "Voice volume", IntVector2(20, 210), _audioSettings.voiceVolume, URHO3D_HANDLER(SettingsWindow, HandleAudioSettingsSlider)));
	_activeSettingElements.Push(CreateSlider(_base, "Music volume", IntVector2(20, 240), _audioSettings.musicVolume, URHO3D_HANDLER(SettingsWindow, HandleAudioSettingsSlider)));
}

void SettingsWindow::CreateControllerSettingsView()
{
    ControllerInput* controllerInput = GetSubsystem<ControllerInput>();
    HashMap<int, String> controlNames = controllerInput->GetControlNames();

    ClearView();

    _openedView = SettingsViewType::CONTROLS_VIEW;

    SharedPtr<ListView> list(_base->CreateChild<ListView>());
    list->SetSelectOnClickEnd(true);
    list->SetHighlightMode(HM_ALWAYS);
    list->SetMinHeight(320);
    list->SetWidth(360);
    list->SetPosition(IntVector2(20, 60));
    list->SetStyleAuto();
    _activeSettingElements.Push(list);

    int index = 0;
    for (auto it = controlNames.Begin(); it != controlNames.End(); ++it) {
        SharedPtr<UIElement> singleItem(
            CreateControlsElement(
                (*it).second_, 
                IntVector2(20, 60 + index * 30), 
                controllerInput->GetActionKeyName((*it).first_), 
                (*it).second_,
                URHO3D_HANDLER(SettingsWindow, HandleChangeControls)
            )
        );
        list->AddItem(singleItem);
        _activeSettingElements.Push(singleItem);
        index++;
    }
}

void SettingsWindow::SubscribeToEvents()
{
    SubscribeToEvent(_buttons[SettingsButtonType::CLOSE], E_RELEASED, URHO3D_HANDLER(SettingsWindow, HandleClose));
    SubscribeToEvent(_buttons[SettingsButtonType::VIDEO], E_RELEASED, URHO3D_HANDLER(SettingsWindow, ShowVideoSettings));
    SubscribeToEvent(_buttons[SettingsButtonType::CONTROLS], E_RELEASED, URHO3D_HANDLER(SettingsWindow, ShowControllerSettings));
    SubscribeToEvent(_buttons[SettingsButtonType::AUDIO], E_RELEASED, URHO3D_HANDLER(SettingsWindow, ShowAudioSettings));
    SubscribeToEvent(_buttons[SettingsButtonType::SAVE], E_RELEASED, URHO3D_HANDLER(SettingsWindow, HandleSave));

    SubscribeToEvent(MyEvents::E_INPUT_MAPPING_FINISHED, URHO3D_HANDLER(SettingsWindow, HandleControlsUpdated));
}

void SettingsWindow::HandleClose(StringHash eventType, VariantMap& eventData)
{
    VariantMap data = GetEventDataMap();
    data["Name"] = "SettingsWindow";
    SendEvent(MyEvents::E_CLOSE_WINDOW, data);
}

void SettingsWindow::HandleSave(StringHash eventType, VariantMap& eventData)
{
	if (_openedView == SettingsViewType::VIDEO_VIEW) {
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
		renderer->SetShadowQuality(SHADOWQUALITY_SIMPLE_16BIT);
		renderer->SetDrawShadows(_graphicsSettings.shadows);
	}

	SendEvent(MyEvents::E_SAVE_CONFIG);
	VariantMap data = GetEventDataMap();
	data["Title"] = "Settings saved!";
	data["Message"] = "You must restart the game for\nthe changes to take effect";
	SendEvent("ShowAlertMessage", data);
}

void SettingsWindow::ShowVideoSettings(StringHash eventType, VariantMap& eventData)
{
    _buttons[SettingsButtonType::SAVE]->SetVisible(true);
    CreateGraphicsSettingsView();
}

void SettingsWindow::ShowAudioSettings(StringHash eventType, VariantMap& eventData)
{
    _buttons[SettingsButtonType::SAVE]->SetVisible(true);
    CreateAudioSettingsView();
}

void SettingsWindow::ShowControllerSettings(StringHash eventType, VariantMap& eventData)
{
    _buttons[SettingsButtonType::SAVE]->SetVisible(false);
    CreateControllerSettingsView();
}

void SettingsWindow::ClearView()
{
    for (auto it = _activeSettingElements.Begin(); it != _activeSettingElements.End(); ++it) {
        (*it)->Remove();
    }
    _activeSettingElements.Clear();
}

void SettingsWindow::HandleChangeControls(StringHash eventType, VariantMap& eventData)
{
    auto* input = GetSubsystem<Input>();
    using namespace Released;
    Button* button = static_cast<Button*>(eventData[P_ELEMENT].GetPtr());
    String actionName = button->GetVar("ActionName").GetString();

    Text* text = dynamic_cast<Text*>(button->GetChild(actionName, true));
    if (text) {
        text->SetText("Press any key...");
    }

    if (!actionName.Empty()) {
        using namespace MyEvents::StartInputMapping;
		VariantMap data = GetEventDataMap();
		data[P_CONTROL_ACTION] = actionName;
		SendEvent(MyEvents::E_START_INPUT_MAPPING, data);

        Input* input = GetSubsystem<Input>();
        if (input->IsMouseVisible()) {
            input->SetMouseVisible(false);
        }
    }
}

void SettingsWindow::HandleControlsUpdated(StringHash eventType, VariantMap& eventData)
{
    ControllerInput* controllerInput = GetSubsystem<ControllerInput>();
    HashMap<int, String> controlNames = controllerInput->GetControlNames();
    for (auto it = controlNames.Begin(); it != controlNames.End(); ++it) {
        Text* text = dynamic_cast<Text*>(_base->GetChild((*it).second_, true));
        if (text) {
            text->SetText(controllerInput->GetActionKeyName((*it).first_));
        }
    }

    Input* input = GetSubsystem<Input>();
    if (!input->IsMouseVisible()) {
        input->SetMouseVisible(true);
    }
}

void SettingsWindow::HandleGraphicsSettingsChange(StringHash eventType, VariantMap& eventData)
{
	using namespace ItemSelected;
	DropDownList* dropdown = static_cast<DropDownList*>(eventData[P_ELEMENT].GetPtr());
	int selected = eventData[P_SELECTION].GetInt();
	Text* text = static_cast<Text*>(dropdown->GetSelectedItem());
	if (dropdown->GetName() == "Resolution") {
		String resolution = text->GetText();
		StringVector dimensions = resolution.Split('x', false);
		if (dimensions.Size() == 2) {
			int width = ToInt(dimensions[0]);
			_graphicsSettings.width = width;

			int height = ToInt(dimensions[1]);
			_graphicsSettings.height = height;
		}
		URHO3D_LOGINFOF("New resolution %d x %d", _graphicsSettings.width, _graphicsSettings.height);
	}
	if (dropdown->GetName() == "Texture quality") {
		URHO3D_LOGINFO("Texture quality: " + text->GetText());
		_graphicsSettings.textureQuality = ToInt(text->GetText());
	}
	if (dropdown->GetName() == "Texture anisotropy level") {
		URHO3D_LOGINFO("Texture anisotropy level: " + text->GetText());
		_graphicsSettings.textureAnistropy = ToInt(text->GetText());
	}
	if (dropdown->GetName() == "Texture filer mode") {
		URHO3D_LOGINFO("Texture filer mode: " + text->GetText());
		for (auto it = _textureFilterModesMapping.Begin(); it != _textureFilterModesMapping.End(); ++it) {
			if ((*it).second_ == text->GetText()) {
				_graphicsSettings.textureFilterMode = (*it).first_;
			}
		}
		URHO3D_LOGINFO("Texture filer mode: " + String(_graphicsSettings.textureFilterMode));
	}
	if (dropdown->GetName() == "Multisample") {
		URHO3D_LOGINFO("Multisample: " + text->GetText());
		_graphicsSettings.multisample = ToInt(text->GetText());
	}
}

void SettingsWindow::HandleGraphicsSettingsToggle(StringHash eventType, VariantMap& eventData)
{
	using namespace Toggled;
	CheckBox* checkbox = static_cast<CheckBox*>(eventData[P_ELEMENT].GetPtr());
	bool isChecked = eventData[P_STATE].GetBool();
	if (checkbox->GetName() == "Fullscreen") {
		_graphicsSettings.fullscreen = isChecked;
	}
	if (checkbox->GetName() == "Vertical sync") {
		_graphicsSettings.vsync = isChecked;
	}
	if (checkbox->GetName() == "Triple buffer") {
		_graphicsSettings.tripleBuffer = isChecked;
	}
	if (checkbox->GetName() == "Enable shadows") {
		_graphicsSettings.shadows = isChecked;
	}
	if (checkbox->GetName() == "Low quality shadows") {
		_graphicsSettings.lowQualityShadows = isChecked;
	}
}

void SettingsWindow::HandleAudioSettingsToggle(StringHash eventType, VariantMap& eventData)
{
	using namespace Toggled;
	CheckBox* checkbox = static_cast<CheckBox*>(eventData[P_ELEMENT].GetPtr());
	bool isChecked = eventData[P_STATE].GetBool();
	if (checkbox->GetName() == "Enable audio") {
		_audioSettings.enabled = isChecked;
	}
	if (checkbox->GetName() == "Stereo") {
		_audioSettings.stereo = isChecked;
	}
	if (checkbox->GetName() == "Sound interpolation") {
		_audioSettings.soundInterpolation = isChecked;
	}
	Audio* audio = GetSubsystem<Audio>();
	audio->SetMode(_audioSettings.soundBuffer, _audioSettings.mixRate, _audioSettings.stereo, _audioSettings.soundInterpolation);
}

void SettingsWindow::HandleAudioSettingsSlider(StringHash eventType, VariantMap& eventData)
{
	using namespace SliderChanged;
	Slider* slider = static_cast<Slider*>(eventData[P_ELEMENT].GetPtr());
	float value = eventData[P_VALUE].GetFloat();
	Audio* audio = GetSubsystem<Audio>();
	if (slider->GetName() == "Master volume") {
		audio->SetMasterGain(SOUND_MASTER, value);
	}
	if (slider->GetName() == "Effects volume") {
		audio->SetMasterGain(SOUND_EFFECT, value);
	}
	if (slider->GetName() == "Ambient volume") {
		audio->SetMasterGain(SOUND_AMBIENT, value);
	}
	if (slider->GetName() == "Voice volume") {
		audio->SetMasterGain(SOUND_VOICE, value);
	}
	if (slider->GetName() == "Music volume") {
		audio->SetMasterGain(SOUND_MUSIC, value);
	}
}

void SettingsWindow::InitAudioSettings()
{
	_audioSettings.enabled = GetGlobalVar("Sound").GetBool();
	_audioSettings.stereo = GetGlobalVar("SoundStereo").GetBool();
	_audioSettings.soundInterpolation = GetGlobalVar("SoundInterpolation").GetBool();
	_audioSettings.mixRate = GetGlobalVar("SoundMixRate").GetFloat();
	_audioSettings.soundBuffer = GetGlobalVar("SoundBuffer").GetFloat();

	URHO3D_LOGINFOF("Audio %f", GetGlobalVar("SoundMasterVolume").GetFloat());
	_audioSettings.masterVolume = GetGlobalVar("SoundMasterVolume").GetFloat();
	_audioSettings.effectsVolume = GetGlobalVar("SoundEffectsVolume").GetFloat();
	_audioSettings.ambientVolume = GetGlobalVar("SoundAmbientVolume").GetFloat();
	_audioSettings.voiceVolume = GetGlobalVar("SoundVoiceVolume").GetFloat();
	_audioSettings.musicVolume = GetGlobalVar("SoundMusicVolume").GetFloat();
}

void SettingsWindow::InitGraphicsSettings()
{
	_graphicsSettings.width = GetGlobalVar("WindowWidth").GetInt();
	_graphicsSettings.height = GetGlobalVar("WindowHeight").GetInt();
	_graphicsSettings.fullscreen = GetGlobalVar("Fullscreen").GetBool();
	_graphicsSettings.vsync = GetGlobalVar("VSync").GetBool();
	_graphicsSettings.tripleBuffer = GetGlobalVar("TripleBuffer").GetBool();
	_graphicsSettings.shadows = GetGlobalVar("Shadows").GetBool();
	_graphicsSettings.lowQualityShadows = GetGlobalVar("LowQualityShadows").GetBool();
	_graphicsSettings.textureQuality = GetGlobalVar("TextureQuality").GetInt();
	_graphicsSettings.textureAnistropy = GetGlobalVar("TextureAnisotropy").GetInt();
	_graphicsSettings.textureFilterMode = GetGlobalVar("TextureFilterMode").GetInt();
	_graphicsSettings.multisample = GetGlobalVar("Multisample").GetInt();
}