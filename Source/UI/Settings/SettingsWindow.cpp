#include <Urho3D/Urho3DAll.h>
#include "SettingsWindow.h"
#include "../../MyEvents.h"
#include "../../Input/ControllerInput.h"
#include "../../Audio/AudioManagerDefs.h"
#include "../../Global.h"
#include "../../Messages/Achievements.h"

/**
 * Settings view has horizontal layout, and each element takes up this much horizontal space
 */
static const int COLUMN_WIDTH = 250;

/// Construct.
SettingsWindow::SettingsWindow(Context* context) :
        BaseWindow(context)
{
    Init();
}

SettingsWindow::~SettingsWindow()
{
    _baseWindow->Remove();
}

void SettingsWindow::Init()
{
    Create();

    SubscribeToEvents();
}

void SettingsWindow::Create()
{
    auto* localization = GetSubsystem<Localization>();

    _baseWindow = CreateOverlay()->CreateChild<Window>();
    _baseWindow->SetStyleAuto();
    _baseWindow->SetAlignment(HA_CENTER, VA_CENTER);
    _baseWindow->SetSize(400, 500);
    _baseWindow->BringToFront();

    // Create Window 'titlebar' container
    _titleBar =_baseWindow->CreateChild<UIElement>();
    _titleBar->SetFixedSize(_baseWindow->GetWidth(), 24);
    _titleBar->SetVerticalAlignment(VA_TOP);
    _titleBar->SetLayoutMode(LM_HORIZONTAL);
    _titleBar->SetLayoutBorder(IntRect(4, 4, 4, 4));

    auto* cache = GetSubsystem<ResourceCache>();
    auto* font = cache->GetResource<Font>(APPLICATION_FONT);

    // Create the Window title Text
    auto* windowTitle = new Text(context_);
    windowTitle->SetName("WindowTitle");
    windowTitle->SetText(localization->Get("SETTINGS"));
    windowTitle->SetFont(font, 14);


    // Create the Window's close button
    auto* buttonClose = new Button(context_);
    buttonClose->SetName("CloseButton");
    buttonClose->SetHorizontalAlignment(HA_RIGHT);

    // Add the controls to the title bar
    _titleBar->AddChild(windowTitle);
    _titleBar->AddChild(buttonClose);

    // Apply styles
    windowTitle->SetStyleAuto();
    buttonClose->SetStyle("CloseButton");

    SubscribeToEvent(buttonClose, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "SettingsWindow";
        SendEvent(MyEvents::E_CLOSE_WINDOW, data);
    });

    _tabView = _baseWindow->CreateChild<UIElement>();
    _tabView->SetAlignment(HA_LEFT, VA_TOP);
    _tabView->SetPosition(0, 40);
    _tabView->SetWidth(_baseWindow->GetWidth());
    _tabView->SetHeight(_baseWindow->GetHeight());

    _tabs[CONTROLS] = CreateTabButton(localization->Get("CONTROLS"));
    SubscribeToEvent(_tabs[CONTROLS], E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        ChangeTab(CONTROLS);
    });
    _tabs[CONTROLLERS] = CreateTabButton(localization->Get("CONTROLLERS"));
    SubscribeToEvent(_tabs[CONTROLLERS], E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        ChangeTab(CONTROLLERS);
    });
    _tabs[AUDIO] = CreateTabButton(localization->Get("AUDIO"));
    SubscribeToEvent(_tabs[AUDIO], E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        ChangeTab(AUDIO);
    });
    _tabs[VIDEO] = CreateTabButton(localization->Get("VIDEO"));
    SubscribeToEvent(_tabs[VIDEO], E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        ChangeTab(VIDEO);
    });
    _tabs[GAME] = CreateTabButton(localization->Get("GAME"));
    SubscribeToEvent(_tabs[GAME], E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        ChangeTab(GAME);
    });

    ChangeTab(CONTROLS);
}

void SettingsWindow::SubscribeToEvents()
{
    SubscribeToEvent(MyEvents::E_INPUT_MAPPING_FINISHED, URHO3D_HANDLER(SettingsWindow, HandleControlsUpdated));
    SubscribeToEvent(MyEvents::E_STOP_INPUT_MAPPING, URHO3D_HANDLER(SettingsWindow, HandleControlsUpdated));
}

void SettingsWindow::ChangeTab(SettingTabs tab)
{
    _activeTab = tab;
    _tabView->RemoveAllChildren();
    _tabElementCount = 0;

    switch (_activeTab) {
        case CONTROLS:
            CreateControlsTab();
            break;
        case CONTROLLERS:
            CreateControllersTab();
            break;
        case AUDIO:
            CreateAudioTab();
            break;
        case VIDEO:
            CreateVideoTab();
            break;
        case GAME:
            CreateGameTab();
            break;
    }
}

void SettingsWindow::CreateControlsTab()
{
    auto controllerInput = GetSubsystem<ControllerInput>();
    auto names = controllerInput->GetControlNames();

    // Loop trough all of the controls
    for (auto it = names.Begin(); it != names.End(); ++it) {
        CreateSingleLine();
        CreateLabel((*it).second_);
        Button* button = CreateButton(controllerInput->GetActionKeyName((*it).first_));
        button->SetVar("Action", (*it).first_);

        // Detect button press events
        SubscribeToEvent(button, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
            // Start mapping input
            using namespace Released;
            Button* button = static_cast<Button*>(eventData[P_ELEMENT].GetPtr());
            int action = button->GetVar("Action").GetInt();

            using namespace MyEvents::StartInputMapping;
            VariantMap& data = GetEventDataMap();
            data[P_CONTROL_ACTION] = action;
            SendEvent(MyEvents::E_START_INPUT_MAPPING, data);

            auto buttonLabel = button->GetChildStaticCast<Text>("Label", false);
            buttonLabel->SetText("...");

            Input* input = GetSubsystem<Input>();
            if (input->IsMouseVisible()) {
                input->SetMouseVisible(false);
            }
        });
    }
}

void SettingsWindow::CreateControllersTab()
{
    auto* localization = GetSubsystem<Localization>();

    auto controllerInput = GetSubsystem<ControllerInput>();
    {
        CreateSingleLine();
        CreateLabel(localization->Get("MOUSE"));

        // Invert X
        CreateSingleLine();
        auto mouseInvertX = CreateCheckbox(localization->Get("INVERT_X_AXIS"));
        mouseInvertX->SetChecked(controllerInput->GetInvertX(ControllerType::MOUSE));
        URHO3D_LOGINFO("Set mouse invert x " + String(mouseInvertX->IsChecked()));
        SubscribeToEvent(mouseInvertX, E_TOGGLED, [&](StringHash eventType, VariantMap &eventData) {
            using namespace Toggled;
            bool enabled = eventData[P_STATE].GetBool();

            URHO3D_LOGINFO("Set mouse invert x " + String(enabled));

            auto controllerInput = GetSubsystem<ControllerInput>();
            controllerInput->SetInvertX(enabled, ControllerType::MOUSE);
            GetSubsystem<ConfigManager>()->Set("mouse", "InvertX", enabled);

            GetSubsystem<ConfigManager>()->Save(true);
        });

        // Invert Y
        CreateSingleLine();
        auto mouseInvertY = CreateCheckbox(localization->Get("INVERT_Y_AXIS"));
        mouseInvertY->SetChecked(controllerInput->GetInvertY(ControllerType::MOUSE));
        SubscribeToEvent(mouseInvertY, E_TOGGLED, [&](StringHash eventType, VariantMap &eventData) {
            using namespace Toggled;
            bool enabled = eventData[P_STATE].GetBool();

            auto controllerInput = GetSubsystem<ControllerInput>();
            controllerInput->SetInvertY(enabled, ControllerType::MOUSE);
            GetSubsystem<ConfigManager>()->Set("mouse", "InvertY", enabled);

            GetSubsystem<ConfigManager>()->Save(true);
        });

        // Sensitivity
        CreateSingleLine();
        auto slider = CreateSlider(localization->Get("SENSITIVITY"));
        slider->SetRange(10);
        slider->SetValue(controllerInput->GetSensitivityX(ControllerType::MOUSE));
        // Detect button press events
        SubscribeToEvent(slider, E_SLIDERCHANGED, [&](StringHash eventType, VariantMap &eventData) {

            using namespace SliderChanged;
            float newValue = eventData[P_VALUE].GetFloat();
            auto controllerInput = GetSubsystem<ControllerInput>();
            controllerInput->SetSensitivityX(newValue, ControllerType::MOUSE);
            controllerInput->SetSensitivityY(newValue, ControllerType::MOUSE);
            GetSubsystem<ConfigManager>()->Set("mouse", "Sensitivity", newValue);

            GetSubsystem<ConfigManager>()->Save(true);
        });
    }

    // Joystick
    {
        CreateSingleLine();
        CreateSingleLine();
        CreateLabel(localization->Get("JOYSTICK"));

        // Invert X
        CreateSingleLine();
        auto joystickInvertX = CreateCheckbox(localization->Get("INVERT_X_AXIS"));
        joystickInvertX->SetChecked(controllerInput->GetInvertX(ControllerType::JOYSTICK));
        SubscribeToEvent(joystickInvertX, E_TOGGLED, [&](StringHash eventType, VariantMap &eventData) {
            using namespace Toggled;
            bool enabled = eventData[P_STATE].GetBool();

            auto controllerInput = GetSubsystem<ControllerInput>();
            controllerInput->SetInvertX(enabled, ControllerType::JOYSTICK);
            GetSubsystem<ConfigManager>()->Set("joystick", "InvertX", enabled);

            GetSubsystem<ConfigManager>()->Save(true);
        });

        // Invert Y
        CreateSingleLine();
        auto joystickInvertY = CreateCheckbox(localization->Get("INVERT_Y_AXIS"));
        joystickInvertY->SetChecked(controllerInput->GetInvertY(ControllerType::JOYSTICK));
        SubscribeToEvent(joystickInvertY, E_TOGGLED, [&](StringHash eventType, VariantMap &eventData) {
            using namespace Toggled;
            bool enabled = eventData[P_STATE].GetBool();

            auto controllerInput = GetSubsystem<ControllerInput>();
            controllerInput->SetInvertY(enabled, ControllerType::JOYSTICK);
            GetSubsystem<ConfigManager>()->Set("joystick", "InvertY", enabled);

            GetSubsystem<ConfigManager>()->Save(true);
        });

        // Sensitivity X
        CreateSingleLine();
        auto sensitivityX = CreateSlider(localization->Get("SENSITIVITY_X_AXIS"));
        sensitivityX->SetRange(10);
        sensitivityX->SetValue(controllerInput->GetSensitivityX(ControllerType::JOYSTICK) * 0.2f);
        // Detect button press events
        SubscribeToEvent(sensitivityX, E_SLIDERCHANGED, [&](StringHash eventType, VariantMap &eventData) {

            using namespace SliderChanged;
            float newValue = eventData[P_VALUE].GetFloat() * 5.0f;
            auto controllerInput = GetSubsystem<ControllerInput>();
            controllerInput->SetSensitivityX(newValue, ControllerType::JOYSTICK);
            GetSubsystem<ConfigManager>()->Set("joystick", "SensitivityX", newValue);

            GetSubsystem<ConfigManager>()->Save(true);
        });

        // Sensitivity Y
        CreateSingleLine();
        auto sensitivityY = CreateSlider(localization->Get("SENSITIVITY_Y_AXIS"));
        sensitivityY->SetRange(10);
        sensitivityY->SetValue(controllerInput->GetSensitivityY(ControllerType::JOYSTICK) * 0.2f);
        // Detect button press events
        SubscribeToEvent(sensitivityY, E_SLIDERCHANGED, [&](StringHash eventType, VariantMap &eventData) {

            using namespace SliderChanged;
            float newValue = eventData[P_VALUE].GetFloat() * 5.0f;
            auto controllerInput = GetSubsystem<ControllerInput>();
            controllerInput->SetSensitivityY(newValue, ControllerType::JOYSTICK);
            GetSubsystem<ConfigManager>()->Set("joystick", "SensitivityY", newValue);

            GetSubsystem<ConfigManager>()->Save(true);
        });

        // Multiple controller support
        CreateSingleLine();
        auto multipleControllers = CreateCheckbox(localization->Get("MULTIPLE_CONTROLLER_SUPPORT"));
        multipleControllers->SetChecked(controllerInput->GetMultipleControllerSupport());
        SubscribeToEvent(multipleControllers, E_TOGGLED, [&](StringHash eventType, VariantMap &eventData) {
            using namespace Toggled;
            bool enabled = eventData[P_STATE].GetBool();

            auto controllerInput = GetSubsystem<ControllerInput>();
            controllerInput->SetMultipleControllerSupport(enabled);
            GetSubsystem<ConfigManager>()->Set("joystick", "MultipleControllers", enabled);

            GetSubsystem<ConfigManager>()->Save(true);
        });

        // Joystick as first controller
        CreateSingleLine();
        auto joystickAsFirstController = CreateCheckbox(localization->Get("JOYSTICK_AS_FIRST_CONTROLLER"));
        joystickAsFirstController->SetChecked(controllerInput->GetJoystickAsFirstController());
        SubscribeToEvent(joystickAsFirstController, E_TOGGLED, [&](StringHash eventType, VariantMap &eventData) {
            using namespace Toggled;
            bool enabled = eventData[P_STATE].GetBool();

            auto controllerInput = GetSubsystem<ControllerInput>();
            controllerInput->SetJoystickAsFirstController(enabled);
            GetSubsystem<ConfigManager>()->Set("joystick", "JoystickAsFirstController", enabled);

            GetSubsystem<ConfigManager>()->Save(true);
        });
    }
}

void SettingsWindow::CreateAudioTab()
{
    auto* localization = GetSubsystem<Localization>();

    String volumeControls[] = {
            SOUND_MASTER,
            SOUND_EFFECT,
            SOUND_AMBIENT,
            SOUND_VOICE,
            SOUND_MUSIC
    };
    for (int i = 0; i < 5; i++) {
        // Master volume slider
        CreateSingleLine();
        auto slider = CreateSlider(localization->Get(volumeControls[i].ToUpper() + "_VOLUME"));
        slider->SetValue(GetSubsystem<Audio>()->GetMasterGain(volumeControls[i]));
        slider->SetVar("SoundType", volumeControls[i]);

        // Detect button press events
        SubscribeToEvent(slider, E_SLIDERCHANGED, [&](StringHash eventType, VariantMap &eventData) {

            using namespace SliderChanged;
            float newVolume = eventData[P_VALUE].GetFloat();
            Slider* slider = static_cast<Slider*>(eventData[P_ELEMENT].GetPtr());
            String soundType = slider->GetVar("SoundType").GetString();

            GetSubsystem<Audio>()->SetMasterGain(soundType, newVolume);

            SetGlobalVar(soundType, newVolume);
            GetSubsystem<ConfigManager>()->Set("audio", soundType, newVolume);

            GetSubsystem<ConfigManager>()->Save(true);

            URHO3D_LOGINFO("Volume changed " + soundType + " => " + String(newVolume));
        });
    }
}
void SettingsWindow::CreateVideoTab()
{
    auto* localization = GetSubsystem<Localization>();
    InitGraphicsSettings();

    // UI Scale
    CreateSingleLine();
    auto scaleSlider = CreateSlider(localization->Get("UI"));
    scaleSlider->SetRange(0.5);
    scaleSlider->SetValue(GetSubsystem<UI>()->GetScale() - 1.0f);
    // Detect button press events
    SubscribeToEvent(scaleSlider, E_SLIDERCHANGED, [&](StringHash eventType, VariantMap &eventData) {

        using namespace SliderChanged;
        float newValue = eventData[P_VALUE].GetFloat();
        GetSubsystem<UI>()->SetScale(newValue + 1.0f);

    });

    // Gamma
    CreateSingleLine();
    auto gammaSlider = CreateSlider(localization->Get("GAMMA"));
    gammaSlider->SetRange(GAMMA_MAX_VALUE);
    gammaSlider->SetValue(GetSubsystem<ConfigManager>()->GetFloat("engine", "Gamma", 1.0f));
    // Detect button press events
    SubscribeToEvent(gammaSlider, E_SLIDERCHANGED, [&](StringHash eventType, VariantMap &eventData) {

        using namespace SliderChanged;
        float newValue = eventData[P_VALUE].GetFloat();
        VariantMap data = GetEventDataMap();
        StringVector command;
        command.Push("gamma");
        command.Push(String(newValue));
        data["Parameters"] = command;
        SendEvent("gamma", data);

    });

    // FOV
    CreateSingleLine();
    auto fovSlider = CreateSlider(localization->Get("FIELD_OF_VIEW"));
    fovSlider->SetRange(100);
    fovSlider->SetValue(GetGlobalVar("CameraFov").GetFloat() - 60.0f);
    // Detect button press events
    SubscribeToEvent(fovSlider, E_SLIDERCHANGED, [&](StringHash eventType, VariantMap &eventData) {

        using namespace SliderChanged;
        float newValue = eventData[P_VALUE].GetFloat() + 60.0f;
        VariantMap data = GetEventDataMap();
        StringVector command;
        command.Push("fov");
        command.Push(String(newValue));
        data["Parameters"] = command;
        SendEvent("FovChange", data);
        
    });

    // Fullscreen
    CreateSingleLine();
    auto fullscreenToggle = CreateCheckbox(localization->Get("FULLSCREEN"));
    fullscreenToggle->SetChecked(_graphicsSettings.fullscreen);
    SubscribeToEvent(fullscreenToggle, E_TOGGLED, [&](StringHash eventType, VariantMap &eventData) {
        using namespace Toggled;
        bool enabled = eventData[P_STATE].GetBool();

        _graphicsSettingsNew.fullscreen = enabled;
    });

    // Fullscreen
    CreateSingleLine();
    auto frameLimiterToggle = CreateCheckbox(localization->Get("FRAME_LIMITER"));
    frameLimiterToggle->SetChecked(_graphicsSettings.frameLimiter);
    SubscribeToEvent(frameLimiterToggle, E_TOGGLED, [&](StringHash eventType, VariantMap &eventData) {
        using namespace Toggled;
        bool enabled = eventData[P_STATE].GetBool();

        _graphicsSettingsNew.frameLimiter = enabled;
    });

    // Shadows
    CreateSingleLine();
    auto shadowToggle = CreateCheckbox(localization->Get("ENABLE_SHADOWS"));
    shadowToggle->SetChecked(_graphicsSettings.shadows);
    SubscribeToEvent(shadowToggle, E_TOGGLED, [&](StringHash eventType, VariantMap &eventData) {
        using namespace Toggled;
        bool enabled = eventData[P_STATE].GetBool();

        _graphicsSettingsNew.shadows = enabled;
    });

    // VSync
    CreateSingleLine();
    auto vsyncToggle = CreateCheckbox(localization->Get("VERTICAL_SYNC"));
    vsyncToggle->SetChecked(_graphicsSettings.vsync);
    SubscribeToEvent(vsyncToggle, E_TOGGLED, [&](StringHash eventType, VariantMap &eventData) {
        using namespace Toggled;
        bool enabled = eventData[P_STATE].GetBool();

        _graphicsSettingsNew.vsync = enabled;
    });

    // Triple buffering
    CreateSingleLine();
    auto trippleBuffToggle = CreateCheckbox(localization->Get("TRIPLE_BUFFERING"));
    trippleBuffToggle->SetChecked(_graphicsSettings.tripleBuffer);
    SubscribeToEvent(trippleBuffToggle, E_TOGGLED, [&](StringHash eventType, VariantMap &eventData) {
        using namespace Toggled;
        bool enabled = eventData[P_STATE].GetBool();

        _graphicsSettingsNew.tripleBuffer = enabled;
    });

    // Resolution
    CreateSingleLine();
    auto resolutionMenu = CreateMenu(localization->Get("RESOLUTION"), _availableResolutionNames);
    resolutionMenu->SetSelection(_graphicsSettings.activeResolution);
    SubscribeToEvent(resolutionMenu, E_ITEMSELECTED, [&](StringHash eventType, VariantMap &eventData) {
        using namespace ItemSelected;
        int selection = eventData[P_SELECTION].GetInt();

        StringVector dimensions = _availableResolutionNames.At(selection).Split('x', false);
        if (dimensions.Size() == 2) {
            int width = ToInt(dimensions[0]);
            _graphicsSettingsNew.width = width;

            int height = ToInt(dimensions[1]);
            _graphicsSettingsNew.height = height;

            _graphicsSettingsNew.activeResolution = selection;
        }
    });

    // Shadow quality
    CreateSingleLine();
    Vector<String> shadowQualityLevels;
    shadowQualityLevels.Push("SHADOWQUALITY_SIMPLE_16BIT");
    shadowQualityLevels.Push("SHADOWQUALITY_SIMPLE_24BIT");
    shadowQualityLevels.Push("SHADOWQUALITY_PCF_16BIT");
    shadowQualityLevels.Push("SHADOWQUALITY_PCF_24BIT");
    shadowQualityLevels.Push("SHADOWQUALITY_VSM");
    shadowQualityLevels.Push("SHADOWQUALITY_BLUR_VSM");

    auto shadowQualityMenu = CreateMenu(localization->Get("SHADOW_QUALITY"), shadowQualityLevels);
    shadowQualityMenu->SetSelection(_graphicsSettings.shadowQuality);
    SubscribeToEvent(shadowQualityMenu, E_ITEMSELECTED, [&](StringHash eventType, VariantMap &eventData) {
        using namespace ItemSelected;
        int selection = eventData[P_SELECTION].GetInt();

        _graphicsSettingsNew.shadowQuality = selection;
    });


    // Texture filter mode
    CreateSingleLine();
    Vector<String> textureFilterModes;
    textureFilterModes.Push("FILTER_NEAREST");
    textureFilterModes.Push("FILTER_BILINEAR");
    textureFilterModes.Push("FILTER_ANISOTROPIC");
    textureFilterModes.Push("FILTER_NEAREST_ANISOTROPIC");
    textureFilterModes.Push("FILTER_DEFAULT");
    textureFilterModes.Push("MAX_FILTERMODES");

    auto textureFilterModeMenu = CreateMenu(localization->Get("TEXTURE_FILTER_MODE"), textureFilterModes);
    textureFilterModeMenu->SetSelection(_graphicsSettings.textureFilterMode);
    SubscribeToEvent(textureFilterModeMenu, E_ITEMSELECTED, [&](StringHash eventType, VariantMap &eventData) {
        using namespace ItemSelected;
        int selection = eventData[P_SELECTION].GetInt();

        _graphicsSettingsNew.textureFilterMode = selection;
    });


    // Texture anistrophy
    CreateSingleLine();
    Vector<String> levels;
    levels.Push("1");
    levels.Push("2");
    levels.Push("3");
    levels.Push("4");
    levels.Push("5");
    levels.Push("6");
    levels.Push("7");
    levels.Push("8");
    levels.Push("9");
    levels.Push("10");
    levels.Push("11");
    levels.Push("12");
    levels.Push("13");
    levels.Push("14");
    levels.Push("15");
    levels.Push("16");

    auto textureAnisotropyLevelMenu = CreateMenu(localization->Get("TEXTURE_ANISTROPY_LEVEL"), levels);
    textureAnisotropyLevelMenu->SetSelection(_graphicsSettings.textureAnistropy);
    SubscribeToEvent(textureAnisotropyLevelMenu, E_ITEMSELECTED, [&](StringHash eventType, VariantMap &eventData) {
        using namespace ItemSelected;
        int selection = eventData[P_SELECTION].GetInt();

        _graphicsSettingsNew.textureAnistropy = selection;
    });

    // Texture quality
    CreateSingleLine();
    auto textureQualityMenu = CreateMenu(localization->Get("TEXTURE_QUALITY"), levels);
    textureQualityMenu->SetSelection(_graphicsSettings.textureQuality);
    SubscribeToEvent(textureQualityMenu, E_ITEMSELECTED, [&](StringHash eventType, VariantMap &eventData) {
        using namespace ItemSelected;
        int selection = eventData[P_SELECTION].GetInt();

        _graphicsSettingsNew.textureQuality = selection;
    });

    // Multisample
    CreateSingleLine();
    auto multisampleMenu = CreateMenu(localization->Get("MULTISAMPLE"), levels);
    multisampleMenu->SetSelection(_graphicsSettings.multisample);
    SubscribeToEvent(multisampleMenu, E_ITEMSELECTED, [&](StringHash eventType, VariantMap &eventData) {
        using namespace ItemSelected;
        int selection = eventData[P_SELECTION].GetInt();

        _graphicsSettingsNew.multisample = selection;
    });

    CreateSingleLine();
    auto applyVideoSettings = CreateButton(localization->Get("APPLY"));

    // Detect button press events
    SubscribeToEvent(applyVideoSettings, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        SaveVideoSettings();
    });
}

void SettingsWindow::CreateGameTab()
{
    auto* localization = GetSubsystem<Localization>();

    CreateSingleLine();
    Vector<String> languages;
    for (unsigned int i = 0; i < localization->GetNumLanguages(); i++) {
        languages.Push(localization->GetLanguage(i));
    }

    auto languageMenu = CreateMenu(localization->Get("LANGUAGE"), languages);
    languageMenu->SetSelection(languages.IndexOf(GetGlobalVar("Language").GetString()));
    SubscribeToEvent(languageMenu, E_ITEMSELECTED, [&](StringHash eventType, VariantMap &eventData) {
        using namespace ItemSelected;
        int selection = eventData[P_SELECTION].GetInt();

        auto* localization = GetSubsystem<Localization>();
        Vector<String> languages;
        for (unsigned int i = 0; i < localization->GetNumLanguages(); i++) {
            languages.Push(localization->GetLanguage(i));
        }

        GetSubsystem<ConfigManager>()->Set("engine", "Language", languages.At(selection));
        GetSubsystem<ConfigManager>()->Save(true);

        //TODO - apply settings directly, reload view or just show the pop-up message?
    });

    // Load mods
    CreateSingleLine();
    auto loadMods = CreateCheckbox(localization->Get("LOAD_MODS"));
    loadMods->SetChecked(GetSubsystem<ConfigManager>()->GetBool("game", "LoadMods", true));
    SubscribeToEvent(loadMods, E_TOGGLED, [&](StringHash eventType, VariantMap &eventData) {
        using namespace Toggled;
        bool enabled = eventData[P_STATE].GetBool();
        GetSubsystem<ConfigManager>()->Set("game", "LoadMods", enabled);
        GetSubsystem<ConfigManager>()->Save(true);
    });

    // Load mods
    CreateSingleLine();
    auto developerConsole = CreateCheckbox(localization->Get("DEVELOPER_CONSOLE"));
    developerConsole->SetChecked(GetSubsystem<ConfigManager>()->GetBool("game", "DeveloperConsole", true));
    SubscribeToEvent(developerConsole, E_TOGGLED, [&](StringHash eventType, VariantMap &eventData) {
        using namespace Toggled;
        bool enabled = eventData[P_STATE].GetBool();
        GetSubsystem<ConfigManager>()->Set("game", "DeveloperConsole", enabled);
        GetSubsystem<ConfigManager>()->Save(true);
    });

    CreateSingleLine();
    auto clearAchievementsButton = CreateButton(localization->Get("CLEAR_ACHIEVEMENTS"));
    SubscribeToEvent(clearAchievementsButton, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        GetSubsystem<Achievements>()->ClearAchievementsProgress();
    });
}

void SettingsWindow::SaveVideoSettings()
{

    _graphicsSettings = _graphicsSettingsNew;

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
            _graphicsSettings.monitor,
            0
    );

    auto* renderer = GetSubsystem<Renderer>();
    renderer->SetTextureFilterMode(TextureFilterMode(_graphicsSettings.textureFilterMode));
    renderer->SetTextureAnisotropy(_graphicsSettings.textureAnistropy);
    renderer->SetTextureQuality(static_cast<MaterialQuality>(_graphicsSettings.textureQuality));
    renderer->SetMaterialQuality(static_cast<MaterialQuality>(_graphicsSettings.textureQuality));
    renderer->SetShadowQuality((ShadowQuality)_graphicsSettings.shadowQuality);
    renderer->SetDrawShadows(_graphicsSettings.shadows);

    GetSubsystem<Engine>()->SetMaxFps((_graphicsSettings.frameLimiter) ? 60 : 0);

    GetSubsystem<ConfigManager>()->Set("engine", "WindowWidth", _graphicsSettingsNew.width);
    GetSubsystem<ConfigManager>()->Set("engine", "WindowHeight", _graphicsSettingsNew.height);
    GetSubsystem<ConfigManager>()->Set("engine", "VSync", (bool)_graphicsSettingsNew.vsync);
    GetSubsystem<ConfigManager>()->Set("engine", "Fullscreen", (bool)_graphicsSettingsNew.fullscreen);
    GetSubsystem<ConfigManager>()->Set("engine", "FrameLimiter", (bool)_graphicsSettingsNew.frameLimiter);
    GetSubsystem<ConfigManager>()->Set("engine", "TripleBuffer", (bool)_graphicsSettingsNew.tripleBuffer);
    GetSubsystem<ConfigManager>()->Set("engine", "Shadows", (bool)_graphicsSettingsNew.shadows);
    GetSubsystem<ConfigManager>()->Set("engine", "Monitor", _graphicsSettingsNew.monitor);

    GetSubsystem<ConfigManager>()->Set("engine", "ShadowQuality", _graphicsSettingsNew.shadowQuality);
    GetSubsystem<ConfigManager>()->Set("engine", "TextureQuality", _graphicsSettingsNew.textureQuality);
    GetSubsystem<ConfigManager>()->Set("engine", "TextureAnisotropy", _graphicsSettingsNew.textureAnistropy);
    GetSubsystem<ConfigManager>()->Set("engine", "TextureFilterMode", _graphicsSettingsNew.textureFilterMode);
    GetSubsystem<ConfigManager>()->Set("engine", "MultiSample", _graphicsSettingsNew.multisample);
    GetSubsystem<ConfigManager>()->Save(true);

    SendEvent(MyEvents::E_VIDEO_SETTINGS_CHANGED);
}

void SettingsWindow::HandleControlsUpdated(StringHash eventType, VariantMap& eventData)
{
    Input* input = GetSubsystem<Input>();
    if (!input->IsMouseVisible()) {
        input->SetMouseVisible(true);
    }

    ChangeTab(CONTROLS);
}

void SettingsWindow::InitGraphicsSettings()
{
    _graphicsSettings.width = GetGlobalVar("WindowWidth").GetInt();
    _graphicsSettings.height = GetGlobalVar("WindowHeight").GetInt();
    _graphicsSettings.fullscreen = GetGlobalVar("Fullscreen").GetBool() ? 1 : 0;
    _graphicsSettings.frameLimiter = GetGlobalVar("FrameLimiter").GetBool() ? 1 : 0;
    _graphicsSettings.monitor = GetGlobalVar("Monitor").GetInt();
    _graphicsSettings.vsync = GetGlobalVar("VSync").GetBool() ? 1 : 0;
    _graphicsSettings.tripleBuffer = GetGlobalVar("TripleBuffer").GetBool() ? 1 : 0;
    _graphicsSettings.shadows = GetGlobalVar("Shadows").GetBool() ? 1 : 0;
    _graphicsSettings.shadowQuality = GetGlobalVar("ShadowQuality").GetInt();
    _graphicsSettings.textureQuality = GetGlobalVar("TextureQuality").GetInt();
    _graphicsSettings.textureAnistropy = GetGlobalVar("TextureAnisotropy").GetInt();
    _graphicsSettings.textureFilterMode = GetGlobalVar("TextureFilterMode").GetInt();
    _graphicsSettings.multisample = Max(GetGlobalVar("MultiSample").GetInt(), 0);

    String activeResolution = String(_graphicsSettings.width) + " x " + String(_graphicsSettings.height);
    auto graphics = GetSubsystem<Graphics>();

    auto resolutions = graphics->GetResolutions(0);
    for (auto it = resolutions.Begin(); it != resolutions.End(); ++it) {
        if ((*it).x_ < 500 || (*it).y_ < 500) {
            continue;
        }
        String name = String((*it).x_) + " x " + String((*it).y_);
        if (!_availableResolutionNames.Contains(name)) {
            _availableResolutionNames.Push(name);
            if (activeResolution == name) {
                _graphicsSettings.activeResolution = _availableResolutionNames.Size() - 1;
            }
        }
    }

    _graphicsSettingsNew = _graphicsSettings;
}

Button* SettingsWindow::CreateTabButton(const String& text)
{
    const int width = 120;
    const int border = 4;

    auto* cache = GetSubsystem<ResourceCache>();
    auto* font = cache->GetResource<Font>(APPLICATION_FONT);

    auto* button = _baseWindow->CreateChild<Button>();
    button->SetStyleAuto();
    button->SetFixedWidth(width);
    button->SetFixedHeight(30);
    button->SetPosition(IntVector2(_tabs.Size() * (width + border) + border, 24));
    button->SetAlignment(HA_LEFT, VA_TOP);

    auto* buttonText = button->CreateChild<Text>();
    buttonText->SetFont(font, 12);
    buttonText->SetAlignment(HA_CENTER, VA_CENTER);
    buttonText->SetText(text);

    _baseWindow->SetWidth((_tabs.Size() + 1) * (width + border) + border);
    _titleBar->SetFixedSize(_baseWindow->GetWidth(), 24);

    return button;
}

Button* SettingsWindow::CreateButton(const String& text)
{
    auto* cache = GetSubsystem<ResourceCache>();
    auto* font = cache->GetResource<Font>(APPLICATION_FONT);

    auto* button = _activeLine->CreateChild<Button>();
    button->SetStyleAuto();
    button->SetFixedWidth(COLUMN_WIDTH);
    button->SetFixedHeight(30);

    auto* buttonText = button->CreateChild<Text>("Label");
    buttonText->SetFont(font, 12);
    buttonText->SetAlignment(HA_CENTER, VA_CENTER);
    buttonText->SetText(text);

    return button;
}

CheckBox* SettingsWindow::CreateCheckbox(const String& label)
{
    if (!_activeLine) {
        URHO3D_LOGERROR("Call `CreateSingleLine` first before making any elements!");
        return nullptr;
    }

    auto *cache = GetSubsystem<ResourceCache>();
    auto* font = cache->GetResource<Font>(APPLICATION_FONT);

    SharedPtr<Text> text(new Text(context_));
    _activeLine->AddChild(text);
    text->SetText(label);
    text->SetStyleAuto();
    text->SetFixedWidth(COLUMN_WIDTH);
    text->SetFont(font, 12);

    SharedPtr<CheckBox> box(new CheckBox(context_));
    _activeLine->AddChild(box);
    box->SetStyleAuto();

    return box;
}


Text* SettingsWindow::CreateLabel(const String& text)
{
    if (!_activeLine) {
        URHO3D_LOGERROR("Call `CreateSingleLine` first before making any elements!");
        return nullptr;
    }

    auto *cache = GetSubsystem<ResourceCache>();
    auto* font = cache->GetResource<Font>(APPLICATION_FONT);

    // Create log element to view latest logs from the system
    auto *label = _activeLine->CreateChild<Text>();
    label->SetFont(font, 12);
    label->SetPosition(10, 30 + _tabElementCount * 30);
    label->SetText(text);
    label->SetFixedWidth(COLUMN_WIDTH);

    return label;
}

Slider* SettingsWindow::CreateSlider(const String& text)
{
    if (!_activeLine) {
        URHO3D_LOGERROR("Call `CreateSingleLine` first before making any elements!");
        return nullptr;
    }

    auto* cache = GetSubsystem<ResourceCache>();
    auto* font = cache->GetResource<Font>(APPLICATION_FONT);

    // Create text and slider below it
    auto* sliderText = _activeLine->CreateChild<Text>();
    sliderText->SetPosition(0, 0);
    sliderText->SetWidth(50);
    sliderText->SetFont(font, 12);
    sliderText->SetText(text);
    sliderText->SetFixedWidth(COLUMN_WIDTH);

    auto* slider = _activeLine->CreateChild<Slider>();
    slider->SetStyleAuto();
    slider->SetPosition(0, 0);
    slider->SetSize(300, 30);
    slider->SetFixedWidth(COLUMN_WIDTH);
    // Use 0-1 range for controlling sound/music master volume
    slider->SetRange(1.0f);
    slider->SetRepeatRate(0.2);

    return slider;
}

DropDownList* SettingsWindow::CreateMenu(const String& label, Vector<String>& items)
{
    if (!_activeLine) {
        URHO3D_LOGERROR("Call `CreateSingleLine` first before making any elements!");
        return nullptr;
    }

    auto* cache = GetSubsystem<ResourceCache>();
    auto* font = cache->GetResource<Font>(APPLICATION_FONT);

    SharedPtr<Text> text(new Text(context_));
    _activeLine->AddChild(text);
    text->SetText(label);
    text->SetStyleAuto();
    text->SetFixedWidth(COLUMN_WIDTH);
    text->SetFont(font, 12);

    SharedPtr<DropDownList> list(new DropDownList(context_));
    _activeLine->AddChild(list);
    list->SetStyleAuto();
    list->SetFixedWidth(COLUMN_WIDTH);

    for (auto it = items.Begin(); it != items.End(); ++it)
    {
        SharedPtr<Text> item(new Text(context_));
        list->AddItem(item);
        item->SetText((*it));
        item->SetStyleAuto();
        item->SetFixedWidth(COLUMN_WIDTH);
        item->SetFont(font, 12);
    }

    return list;
}

UIElement* SettingsWindow::CreateSingleLine()
{
    SharedPtr<UIElement> container(new UIElement(context_));
    container->SetAlignment(HA_LEFT, VA_TOP);
    container->SetLayout(LM_HORIZONTAL, 20);
    container->SetPosition(10, 30 + _tabElementCount * 30);
    container->SetWidth(_tabView->GetWidth());
    _tabView->AddChild(container);

    _activeLine = container;

    _tabElementCount++;

    return _activeLine;
}
