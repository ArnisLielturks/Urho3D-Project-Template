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
}

void SettingsWindow::CreateControllerSettingsView()
{
    ControllerInput* controllerInput = GetSubsystem<ControllerInput>();
    // HashMap<int, String> controlNames = controllerInput->GetControlNames();

    // ClearView();

    // _openedView = SettingsViewType::CONTROLS_VIEW;

    // // SharedPtr<ListView> list(_base->CreateChild<ListView>());
    // list->SetSelectOnClickEnd(true);
    // list->SetHighlightMode(HM_ALWAYS);
    // list->SetMinHeight(350);
    // list->SetWidth(360);
    // list->SetPosition(IntVector2(20, 60));
    // list->SetStyleAuto();
    // _activeSettingElements.Push(list);

    // int index = 0;
    // for (auto it = controlNames.Begin(); it != controlNames.End(); ++it) {
    //     // SharedPtr<UIElement> singleItem(
    //     //     CreateControlsElement(
    //     //         (*it).second_, 
    //     //         IntVector2(20, 60 + index * 30), 
    //     //         controllerInput->GetActionKeyName((*it).first_), 
    //     //         (*it).second_,
    //     //         URHO3D_HANDLER(SettingsWindow, HandleChangeControls)
    //     //     )
    //     // );
    //     // list->AddItem(singleItem);
    //     // _activeSettingElements.Push(singleItem);
    //     // index++;
    // }
}

void SettingsWindow::SubscribeToEvents()
{
    // SubscribeToEvent(_buttons[SettingsButtonType::CLOSE], E_RELEASED, URHO3D_HANDLER(SettingsWindow, HandleClose));
    // SubscribeToEvent(_buttons[SettingsButtonType::VIDEO], E_RELEASED, URHO3D_HANDLER(SettingsWindow, ShowVideoSettings));
    // SubscribeToEvent(_buttons[SettingsButtonType::CONTROLS], E_RELEASED, URHO3D_HANDLER(SettingsWindow, ShowControllerSettings));
    // SubscribeToEvent(_buttons[SettingsButtonType::AUDIO], E_RELEASED, URHO3D_HANDLER(SettingsWindow, ShowAudioSettings));
    // SubscribeToEvent(_buttons[SettingsButtonType::SAVE], E_RELEASED, URHO3D_HANDLER(SettingsWindow, HandleSave));

    SubscribeToEvent(MyEvents::E_INPUT_MAPPING_FINISHED, URHO3D_HANDLER(SettingsWindow, HandleControlsUpdated));

	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(SettingsWindow, HandleUpdate));
}

void SettingsWindow::HandleClose(StringHash eventType, VariantMap& eventData)
{
	{
		using namespace AudioDefs;
		using namespace MyEvents::PlaySound;
		VariantMap data = GetEventDataMap();
		data[P_INDEX] = SOUND_EFFECTS::BUTTON_CLICK;
		data[P_TYPE] = SOUND_EFFECT;
		SendEvent(MyEvents::E_PLAY_SOUND, data);
	}
	{
		VariantMap data = GetEventDataMap();
		data["Name"] = "SettingsWindow";
		SendEvent(MyEvents::E_CLOSE_WINDOW, data);
	}
}

void SettingsWindow::HandleSave(StringHash eventType, VariantMap& eventData)
{
	{
		using namespace AudioDefs;
		using namespace MyEvents::PlaySound;
		VariantMap data = GetEventDataMap();
		data[P_INDEX] = SOUND_EFFECTS::BUTTON_CLICK;
		data[P_TYPE] = SOUND_EFFECT;
		SendEvent(MyEvents::E_PLAY_SOUND, data);
	}
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
        // Text* text = dynamic_cast<Text*>(_base->GetChild((*it).second_, true));
        // if (text) {
        //     text->SetText(controllerInput->GetActionKeyName((*it).first_));
        // }
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
	_graphicsSettings.lowQualityShadows = GetGlobalVar("LowQualityShadows").GetBool() ? 1 : 0;
	_graphicsSettings.textureQuality = GetGlobalVar("TextureQuality").GetInt();
	_graphicsSettings.textureAnistropy = GetGlobalVar("TextureAnisotropy").GetInt();
	_graphicsSettings.textureFilterMode = GetGlobalVar("TextureFilterMode").GetInt();
	_graphicsSettings.multisample = GetGlobalVar("Multisample").GetInt();

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

    if (nk_begin(ctx, "Settings", nk_rect(50, 50, 400, 400), NK_WINDOW_BORDER | NK_WINDOW_CLOSABLE | NK_WINDOW_MOVABLE)) {
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
	bool _showGUI = true;
    auto nuklear = GetSubsystem<NuklearUI>();
    enum {EASY, HARD};
    static int op = EASY;
    static float value = 0.6f;
    static int i =  20;
    static size_t progress = 1;
    enum options {A,B,C};
    static int checkbox;
    static int option;
	/* fixed widget pixel width */
        // nk_layout_row_static(nuklear->GetNkContext(), 30, 80, 1);
        // if (nk_button_label(nuklear->GetNkContext(), "button")) {
        //     /* event handling */
        // }

        /* fixed widget window ratio width */
        nk_layout_row_dynamic(nuklear->GetNkContext(), 30, 2);
        if (nk_option_label(nuklear->GetNkContext(), "easy", op == EASY)) op = EASY;
        if (nk_option_label(nuklear->GetNkContext(), "hard", op == HARD)) op = HARD;

        nk_layout_row_begin(nuklear->GetNkContext(), NK_DYNAMIC, 150, 1);
        {
            nk_layout_row_push(nuklear->GetNkContext(), 1.0f);
            nk_label_wrap(nuklear->GetNkContext(), "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged.");
        }
        nk_layout_row_end(nuklear->GetNkContext());

        progress += 1;
        if (progress >= 1000) {
            progress = 0;
        }
}

void SettingsWindow::DrawVideoSettings()
{
	auto nuklear = GetSubsystem<NuklearUI>();
	nk_layout_row_begin(nuklear->GetNkContext(), NK_DYNAMIC, 30, 2);
	{
		nk_layout_row_push(nuklear->GetNkContext(), 1.0f);
		// nk_label(nuklear->GetNkContext(), "Stereo", NK_TEXT_LEFT);
		// nk_layout_row_push(nuklear->GetNkContext(), 0.5f);
		nk_checkbox_label(nuklear->GetNkContext(), "FullScreen", &_graphicsSettingsNew.fullscreen);
	}

	nk_layout_row_begin(nuklear->GetNkContext(), NK_DYNAMIC, 30, 2);
	{
		nk_layout_row_push(nuklear->GetNkContext(), 1.0f);
		// nk_label(nuklear->GetNkContext(), "Stereo", NK_TEXT_LEFT);
		// nk_layout_row_push(nuklear->GetNkContext(), 0.5f);
		nk_checkbox_label(nuklear->GetNkContext(), "VSync", &_graphicsSettingsNew.vsync);
	}

	nk_layout_row_begin(nuklear->GetNkContext(), NK_DYNAMIC, 30, 2);
	{
		nk_layout_row_push(nuklear->GetNkContext(), 1.0f);
		// nk_label(nuklear->GetNkContext(), "Stereo", NK_TEXT_LEFT);
		// nk_layout_row_push(nuklear->GetNkContext(), 0.5f);
		nk_checkbox_label(nuklear->GetNkContext(), "Triple buffer", &_graphicsSettingsNew.tripleBuffer);
	}

	nk_layout_row_begin(nuklear->GetNkContext(), NK_DYNAMIC, 30, 2);
	{
		nk_layout_row_push(nuklear->GetNkContext(), 1.0f);
		// nk_label(nuklear->GetNkContext(), "Stereo", NK_TEXT_LEFT);
		// nk_layout_row_push(nuklear->GetNkContext(), 0.5f);
		nk_checkbox_label(nuklear->GetNkContext(), "Shadows", &_graphicsSettingsNew.shadows);
	}
	
	static const char *resolutions[] = {"1920 x 1080","1280 x 720","800 x 600","600 x 480"};

	nk_layout_row_dynamic(nuklear->GetNkContext(), 25, 2);
	nk_label(nuklear->GetNkContext(), "Resolution", NK_TEXT_LEFT);
	_graphicsSettingsNew.textureFilterMode = nk_combo(nuklear->GetNkContext(), resolutions, NK_LEN(resolutions), _graphicsSettingsNew.textureFilterMode, 25, nk_vec2(180,200));


	static const char *qualityModes[] = {"Low","Medium","High","Ultra"};

	if (_graphicsSettingsNew.textureQuality > NK_LEN(qualityModes)) {
		_graphicsSettingsNew.textureQuality = NK_LEN(qualityModes) - 1;
	}
	nk_layout_row_dynamic(nuklear->GetNkContext(), 25, 2);
	nk_label(nuklear->GetNkContext(), "Texture quality", NK_TEXT_LEFT);
	_graphicsSettingsNew.textureQuality = nk_combo(nuklear->GetNkContext(), qualityModes, NK_LEN(qualityModes), _graphicsSettingsNew.textureQuality, 25, nk_vec2(180,200));



	static const char *textureAnisotropyLevel[] = {"Bilinear","Trilinear","Anistropic","Nearest Anistropic", "Default", "Max"};

	nk_layout_row_dynamic(nuklear->GetNkContext(), 25, 2);
	nk_label(nuklear->GetNkContext(), "Texture filter mode", NK_TEXT_LEFT);
	_graphicsSettingsNew.textureFilterMode = nk_combo(nuklear->GetNkContext(), textureAnisotropyLevel, NK_LEN(textureAnisotropyLevel), _graphicsSettingsNew.textureFilterMode, 25, nk_vec2(180,200));

    // String activeResolution = String(_graphicsSettings.width) + "x" + String(_graphicsSettings.height);

    // URHO3D_LOGINFO("Active resolution " + activeResolution);
    // int activeIndex = 0;
    // StringVector supportedResolutions;
    // PODVector<IntVector3> resolutions = graphics->GetResolutions(0);
    // for (auto it = resolutions.Begin(); it != resolutions.End(); ++it) {
    //     String resolution = String((*it).x_) + "x" + String((*it).y_);
    //     if (resolution == activeResolution) {
    //         activeIndex = supportedResolutions.Size() - 1;
    //     }
    //     if (supportedResolutions.Find(resolution) == supportedResolutions.End()) {
    //         supportedResolutions.Push(resolution);
    //     }
    // }
    // // _activeSettingElements.Push(CreateMenu(_base, "Resolution", supportedResolutions, activeIndex, IntVector2(20, 60), URHO3D_HANDLER(SettingsWindow, HandleGraphicsSettingsChange)));
    // // _activeSettingElements.Push(CreateCheckbox(_base, "Fullscreen", _graphicsSettings.fullscreen, IntVector2(20, 90), URHO3D_HANDLER(SettingsWindow, HandleGraphicsSettingsToggle)));
    // // _activeSettingElements.Push(CreateCheckbox(_base, "Vertical sync", _graphicsSettings.vsync, IntVector2(20, 120), URHO3D_HANDLER(SettingsWindow, HandleGraphicsSettingsToggle)));
    // // _activeSettingElements.Push(CreateCheckbox(_base, "Triple buffer", _graphicsSettings.tripleBuffer, IntVector2(20, 150), URHO3D_HANDLER(SettingsWindow, HandleGraphicsSettingsToggle)));

    // // _activeSettingElements.Push(CreateCheckbox(_base, "Enable shadows", _graphicsSettings.shadows, IntVector2(20, 200), URHO3D_HANDLER(SettingsWindow, HandleGraphicsSettingsToggle)));
    // // _activeSettingElements.Push(CreateCheckbox(_base, "Low quality shadows", _graphicsSettings.lowQualityShadows, IntVector2(20, 230), URHO3D_HANDLER(SettingsWindow, HandleGraphicsSettingsToggle)));

	// int activeItem = 0;
    // StringVector textureAnisotropySettings;
	// for (int i = 1; i <= 16; i++) {
	// 	textureAnisotropySettings.Push(String(i));
	// 	if (i == _graphicsSettings.textureAnistropy) {
	// 		activeItem = i - 1;
	// 	}
	// }
    // // _activeSettingElements.Push(CreateMenu(_base, "Texture anisotropy level", textureAnisotropySettings, activeItem, IntVector2(20, 290), URHO3D_HANDLER(SettingsWindow, HandleGraphicsSettingsChange)));

    // StringVector textureFilterModes;
	// int index = 0;
	// for (auto it = _textureFilterModesMapping.Begin(); it != _textureFilterModesMapping.End(); ++it) {
	// 	textureFilterModes.Push((*it).second_);
	// 	if ((*it).first_ == _graphicsSettings.textureFilterMode) {
	// 		activeItem = index;
	// 	}
	// 	index++;
	// }
    // // _activeSettingElements.Push(CreateMenu(_base, "Texture filer mode", textureFilterModes, activeItem, IntVector2(20, 320), URHO3D_HANDLER(SettingsWindow, HandleGraphicsSettingsChange)));

	// StringVector multisampleLevel;
	// for (int i = 1; i <= 16; i++) {
	// 	multisampleLevel.Push(String(i));
	// 	if (i == _graphicsSettings.multisample) {
	// 		activeItem = i - 1;
	// 	}
	// }
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

	nk_layout_row_begin(nuklear->GetNkContext(), NK_DYNAMIC, 30, 2);
	{
		nk_layout_row_push(nuklear->GetNkContext(), 0.5f);
		nk_label(nuklear->GetNkContext(), "Effects volume", NK_TEXT_LEFT);
		nk_layout_row_push(nuklear->GetNkContext(), 0.5f);
		nk_slider_float(nuklear->GetNkContext(), 0, &_audioSettingsNew.effectsVolume, 1.0f, 0.05f);
	}

	nk_layout_row_begin(nuklear->GetNkContext(), NK_DYNAMIC, 30, 2);
	{
		nk_layout_row_push(nuklear->GetNkContext(), 0.5f);
		nk_label(nuklear->GetNkContext(), "Music volume", NK_TEXT_LEFT);
		nk_layout_row_push(nuklear->GetNkContext(), 0.5f);
		nk_slider_float(nuklear->GetNkContext(), 0, &_audioSettingsNew.musicVolume, 1.0f, 0.05f);
	}

	nk_layout_row_begin(nuklear->GetNkContext(), NK_DYNAMIC, 30, 2);
	{
		nk_layout_row_push(nuklear->GetNkContext(), 0.5f);
		nk_label(nuklear->GetNkContext(), "Ambient volume", NK_TEXT_LEFT);
		nk_layout_row_push(nuklear->GetNkContext(), 0.5f);
		nk_slider_float(nuklear->GetNkContext(), 0, &_audioSettingsNew.ambientVolume, 1.0f, 0.05f);
	}

	nk_layout_row_begin(nuklear->GetNkContext(), NK_DYNAMIC, 30, 2);
	{
		nk_layout_row_push(nuklear->GetNkContext(), 0.5f);
		nk_label(nuklear->GetNkContext(), "Voice volume", NK_TEXT_LEFT);
		nk_layout_row_push(nuklear->GetNkContext(), 0.5f);
		nk_slider_float(nuklear->GetNkContext(), 0, &_audioSettingsNew.voiceVolume, 1.0f, 0.05f);
	}

	nk_layout_row_begin(nuklear->GetNkContext(), NK_DYNAMIC, 30, 2);
	{
		nk_layout_row_push(nuklear->GetNkContext(), 1.0f);
		// nk_label(nuklear->GetNkContext(), "Stereo", NK_TEXT_LEFT);
		// nk_layout_row_push(nuklear->GetNkContext(), 0.5f);
		nk_checkbox_label(nuklear->GetNkContext(), "Stereo", &_audioSettingsNew.stereo);
	}

	nk_layout_row_begin(nuklear->GetNkContext(), NK_DYNAMIC, 30, 2);
	{
		nk_layout_row_push(nuklear->GetNkContext(), 1.0f);
		// nk_label(nuklear->GetNkContext(), "Stereo", NK_TEXT_LEFT);
		// nk_layout_row_push(nuklear->GetNkContext(), 0.5f);
		nk_checkbox_label(nuklear->GetNkContext(), "Interpolation", &_audioSettingsNew.soundInterpolation);
	}

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