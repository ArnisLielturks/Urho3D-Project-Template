#include <Urho3D/Input/Input.h>
#include <Urho3D/Resource/Localization.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Core/StringUtils.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Audio/AudioDefs.h>
#include <Urho3D/Audio/Audio.h>
#include "../../Input/ControllerInput.h"
#include "SettingsWindow.h"
#include "../../MyEvents.h"
#include "../../Global.h"
#include "../../Messages/Achievements.h"

namespace {
// RenderWindow modes
    enum RenderWindowMode {
        RWM_WINDOWED = 0,
        RWM_FULLSCREEN_WINDOW,
        RWM_FULLSCREEN
    };

// represents a resolution
    struct Resolution {
        int width;
        int height;
        int refreshrate;

        Resolution()
                : width(0), height(0), refreshrate(0) {}

        Resolution(int w, int h, int refresh = 0)
                : width(w), height(h), refreshrate(refresh) {}

        Resolution(const Resolution& other)
                : width(other.width), height(other.height), refreshrate(other.refreshrate) {}

        void Set(int w, int h, int r = 0) {
            width = w;
            height = h;
            refreshrate = r;
        }

        void operator=(const Resolution& rhs) {
            width = rhs.width;
            height = rhs.height;
            refreshrate = rhs.refreshrate;
        }

        bool operator==(const Resolution& rhs) {
            return (width == rhs.width) && (height == rhs.height) && (refreshrate == rhs.refreshrate);
        }

        bool operator > (const Resolution& rhs) const {
            return ((width * height) > (rhs.width * rhs.height));
        }

        bool operator < (const Resolution& rhs) const {
            return ((width * height) < (rhs.width * rhs.height));
        }

        static Resolution FromString(const String& s) {
            auto tokens = s.Split('x');
            if (tokens.Size() > 2) {
                return Resolution(ToInt(tokens[0], 0), ToInt(tokens[1], 0), ToInt(tokens[2], 0));
            } else if (tokens.Size() == 2) {
                auto rate_tokens = tokens[1].Split('@');
                if (rate_tokens.Size() > 1)
                    return Resolution(ToInt(tokens[0], 0), ToInt(rate_tokens[0]), ToInt(rate_tokens[1]));
                return Resolution(ToInt(tokens[0], 0), ToInt(tokens[1], 0));
            } else
                return Resolution(0, 0, 0);
        }

        String ToString(bool with_rate = false) const {
            if (!with_rate)
                return Urho3D::ToString("%dx%d", width, height);
            else
                return Urho3D::ToString("%dx%d@%dHz", width, height, refreshrate);
        }
    };

    typedef Vector<Resolution> ResolutionVector;

// Get number of currently present monitors
    int GetMonitorCount() {
        return SDL_GetNumVideoDisplays();
    }

// Get current desktop resolution
    Resolution GetDesktopResolution(int monitor) {
        SDL_DisplayMode mode;
        if (SDL_GetDesktopDisplayMode(monitor, &mode) == 0) {
            Resolution res;
            res.width = mode.w;
            res.height = mode.h;
            res.refreshrate = mode.refresh_rate;
            return res;
        }
        return Resolution();
    }

    struct greater {
        template<class T>
        bool operator()(T const &a, T const &b) const { return a > b; }
    };

// Get a list of supported fullscreen resolutions
    ResolutionVector GetFullscreenResolutions(int monitor, int rate) {
        ResolutionVector resolutions;
        if (monitor < 0)
            return resolutions;

        int modes = SDL_GetNumDisplayModes(monitor);

        SDL_DisplayMode mode;
        for (int i = 0; i < modes; i++) {
            if (SDL_GetDisplayMode(monitor, i, &mode) != 0)
                continue;
            if (rate == -1 || (rate != -1 && mode.refresh_rate == rate))
                resolutions.Push(Resolution(mode.w, mode.h, mode.refresh_rate));
        }
        // sort resolutions in descending order
        Sort(resolutions.Begin(), resolutions.End(), greater());
        return resolutions;
    }

    Vector<int> GetFullscreenRefreshRates(int monitor) {
        Vector<int> rates;
        if (monitor < 0)
            return rates;

        ResolutionVector modes = GetFullscreenResolutions(monitor, -1);
        for (auto mode : modes) {
            if (rates.Find(mode.refreshrate) == rates.End())
                rates.Push(mode.refreshrate);
        }
        return rates;
    }

} // namespace

/// Construct.
SettingsWindow::SettingsWindow(Context* context) :
    BaseWindow(context)
{
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
    // Load XML file containing default UI style sheet
    auto* cache = GetSubsystem<ResourceCache>();
    auto* style = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");

    // Set the loaded style as default style
    GetSubsystem<UI>()->GetRoot()->SetDefaultStyle(style);

    // Initialize Window
    InitWindow();

    // Create and add some controls to the Window
    InitControls();

    // Refresh graphics settings shown in the settings window
    RefreshVideoOptions();

    SubscribeToEvent(E_UIOPTION_CHANGED, URHO3D_HANDLER(SettingsWindow, HandleOptionChanged));
}

void SettingsWindow::SubscribeToEvents()
{
}


void SettingsWindow::InitWindow()
{
    // Create the Window and add it to the UI's root node
    window_ = CreateOverlay()->CreateChild<Window>();
    window_->BringToFront();
    window_->GetParent()->SetPriority(window_->GetParent()->GetPriority() + 1);

    // Set Window size and layout settings
    window_->SetMinHeight(GetSubsystem<Graphics>()->GetHeight() / GetSubsystem<UI>()->GetScale() * 0.7);
    window_->SetFixedWidth(GetSubsystem<Graphics>()->GetWidth() / GetSubsystem<UI>()->GetScale() * 0.8);
    window_->SetLayout(LM_VERTICAL, 6, IntRect(6, 6, 6, 6));
    window_->SetAlignment(HA_CENTER, VA_CENTER);
    window_->SetName("Window");

    // Create Window 'titlebar' container
    auto* titleBar = new UIElement(context_);
    titleBar->SetMinSize(0, 24);
    titleBar->SetMaxHeight(24);
    titleBar->SetVerticalAlignment(VA_TOP);
    titleBar->SetLayoutMode(LM_HORIZONTAL);

    auto* localization = GetSubsystem<Localization>();

    // Create the Window title Text
    auto* windowTitle = new Text(context_);
    windowTitle->SetName("GraphicsSettings");
    windowTitle->SetText(localization->Get("SETTINGS"));

    // Create the Window's close button
    auto* buttonClose = new Button(context_);
    buttonClose->SetName("CloseButton");

    // Add the controls to the title bar
    titleBar->AddChild(windowTitle);
    titleBar->AddChild(buttonClose);

    // Add the title bar to the Window
    window_->AddChild(titleBar);

    // Apply styles
    window_->SetStyleAuto();
    windowTitle->SetStyleAuto();
    buttonClose->SetStyle("CloseButton");

    SubscribeToEvent(buttonClose, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "SettingsWindow";
        SendEvent(MyEvents::E_CLOSE_WINDOW, data);
    });

    window_->SetMovable(true);
    window_->SetResizable(true);
}

void SettingsWindow::InitControls()
{
    tabs_ = new UITabPanel(context_);
    tabs_->SetStyleAuto();
    window_->AddChild(tabs_);

    CreateVideoTab();
    CreateGraphicsTab();
    CreateAudioTab();
    CreateControllersTab();
#if !defined(__ANDROID__)
    CreateControlsTab();
#endif
    CreateGameTab();

    SubscribeToEvent(E_UITAB_CHANGED, URHO3D_HANDLER(SettingsWindow, HandleTabChanged));
}

void SettingsWindow::CreateVideoTab()
{
    auto* localization = GetSubsystem<Localization>();
    auto video_tab = tabs_->AddTab(localization->Get("VIDEO"));


    opt_fullscreen_ = new UIMultiOption(context_);
    opt_fullscreen_->SetName("OptFullscreen");
    opt_fullscreen_->SetOptionName(localization->Get("DISPLAY_MODE"));
    opt_fullscreen_->SetStyleAuto();
    opt_fullscreen_->SetTags({ "video" });

    StringVector fullscreen_options;
    fullscreen_options.Push(localization->Get("WINDOW"));
    fullscreen_options.Push(localization->Get("BORDERLESS_WINDOW"));
    fullscreen_options.Push(localization->Get("FULLSCREEN"));
    opt_fullscreen_->SetStrings(fullscreen_options);

    opt_monitor_ = new UIMultiOption(context_);
    opt_monitor_->SetName("OptMonitor");
    opt_monitor_->SetOptionName(localization->Get("MONITOR"));
    opt_monitor_->SetStyleAuto();
    opt_monitor_->SetTags({ "video" });

    opt_resolution_ = new UIMultiOption(context_);
    opt_resolution_->SetName("OptResolution");
    opt_resolution_->SetOptionName("Resolution");
    opt_resolution_->SetStyleAuto();
    opt_resolution_->SetTags({ "video" });

    opt_rate_ = new UIMultiOption(context_);
    opt_rate_->SetName("OptRate");
    opt_rate_->SetOptionName("Refresh Rate");
    opt_rate_->SetStyleAuto();
    opt_rate_->SetTags({ "video" });

    opt_vsync_ = new UIBoolOption(context_);
    opt_vsync_->SetName("OptVsync");
    opt_vsync_->SetOptionName("V-Sync");
    opt_vsync_->SetStyleAuto();
    opt_vsync_->SetTags({ "video" });

    auto elm = new UIElement(context_);
    elm->SetMinSize(0, 32);
    elm->SetMaxHeight(30);
    elm->SetVerticalAlignment(VA_TOP);
    btn_apply_ = new Button(context_);
    btn_apply_->SetFixedSize(100, 28);

    auto btn_text = new Text(context_);
    btn_text->SetText(localization->Get("APPLY"));
    btn_apply_->AddChild(btn_text);
    btn_text->SetAlignment(HA_CENTER, VA_CENTER);

    elm->AddChild(btn_apply_);
    btn_apply_->SetHorizontalAlignment(HA_RIGHT);

    btn_apply_->SetStyleAuto();
    btn_text->SetStyleAuto();
    elm->SetStyleAuto();

    opt_resizable_ = new UIBoolOption(context_);
    opt_resizable_->SetName("OptResizable");
    opt_resizable_->SetOptionName("Resizable Window");
    opt_resizable_->SetStyleAuto();
    opt_resizable_->SetTags({ "misc-video" });

    opt_fpslimit_ = new UIMultiOption(context_);
    opt_fpslimit_->SetName("OptFpsLimit");
    opt_fpslimit_->SetOptionName(localization->Get("FPS_LIMIT"));
    opt_fpslimit_->SetStyleAuto();
    opt_fpslimit_->SetTags({ "misc-video" });
    {
        StringVector options;
        options.Push("Custom");
        options.Push("Unlimited");
        options.Push("30");
        options.Push("60");
        options.Push("75");
        options.Push("100");
        options.Push("144");
        options.Push("240");
        opt_fpslimit_->SetStrings(options);

        int currentFps = GetSubsystem<Engine>()->GetMaxFps();
        if (0 == currentFps) {
            opt_fpslimit_->SetOptionIndex(1);
        }

        for (int i = 0; i < options.Size(); i++) {
            if (ToInt(options.At(i)) == currentFps) {
                opt_fpslimit_->SetOptionIndex(i);
            }
        }
    }

    gamma_ = new UISliderOption(context_);
    gamma_->SetName("Gamma");
    gamma_->SetOptionName(localization->Get("GAMMA"));
    gamma_->SetRange(2.0f);
    gamma_->SetValue(GetSubsystem<ConfigManager>()->GetFloat("postprocess", "Gamma", 1.0f));
    gamma_->SetStyleAuto();
    gamma_->SetTags({"misc-video"});

#if !defined(__ANDROID__) && !defined(__EMSCRIPTEN__)
    video_tab->AddItem(opt_fullscreen_);
    video_tab->AddItem(opt_monitor_);
    video_tab->AddItem(opt_resolution_);
    video_tab->AddItem(opt_rate_);
#endif
    video_tab->AddItem(opt_vsync_);
    video_tab->AddItem(elm);
#if !defined(__ANDROID__) && !defined(__EMSCRIPTEN__)
    video_tab->AddItem(opt_resizable_);
#endif
    video_tab->AddItem(opt_fpslimit_);
    video_tab->AddItem(gamma_);

    SubscribeToEvent(btn_apply_, E_RELEASED, URHO3D_HANDLER(SettingsWindow, HandleApply));
}

void SettingsWindow::CreateGraphicsTab()
{
    auto* localization = GetSubsystem<Localization>();
    auto graphics_tab = tabs_->AddTab(localization->Get("GRAPHICS"));


    // graphics tab
    opt_texture_quality_ = new UIMultiOption(context_);
    opt_texture_quality_->SetName("OptTextureQuality");
    opt_texture_quality_->SetOptionName(localization->Get("TEXTURE_QUALITY"));
    opt_texture_quality_->SetStyleAuto();
    opt_texture_quality_->SetTags({ "graphics" });

    StringVector quality_options;
    quality_options.Push(localization->Get("LOW"));
    quality_options.Push(localization->Get("MEDIUM"));
    quality_options.Push(localization->Get("HIGH"));
    opt_texture_quality_->SetStrings(quality_options);

    opt_material_quality_ = new UIMultiOption(context_);
    opt_material_quality_->SetName("OptMaterialQuality");
    opt_material_quality_->SetOptionName(localization->Get("MATERIAL_QUALITY"));
    opt_material_quality_->SetStyleAuto();
    opt_material_quality_->SetTags({ "graphics" });
    opt_material_quality_->SetStrings(quality_options);

    opt_shadows_ = new UIMultiOption(context_);
    opt_shadows_->SetName("OptShadows");
    opt_shadows_->SetOptionName(localization->Get("SHADOWS"));
    opt_shadows_->SetStyleAuto();
    opt_shadows_->SetTags({ "graphics" });
    {
        StringVector options;
        options.Push(localization->Get("OFF"));
        options.Push(localization->Get("LOW"));
        options.Push(localization->Get("MEDIUM"));
        options.Push(localization->Get("HIGH"));
        opt_shadows_->SetStrings(options);
    }

    opt_shadow_quality_ = new UIMultiOption(context_);
    opt_shadow_quality_->SetName("OptShadowQuality");
    opt_shadow_quality_->SetOptionName(localization->Get("SHADOW_QUALITY"));
    opt_shadow_quality_->SetStyleAuto();
    opt_shadow_quality_->SetTags({ "graphics" });
    {
        StringVector options;
        options.Push("Simple 16 bit");
        options.Push("Simple 24 bit");
        options.Push("PCF 16 bit");
        options.Push("PCF 24 bit");
        options.Push("VSM");
        options.Push("Blur VSM");
        opt_shadow_quality_->SetStrings(options);
    }

    StringVector options_bool;
    options_bool.Push(localization->Get("OFF"));
    options_bool.Push(localization->Get("ON"));

    opt_occlusion_ = new UIMultiOption(context_);
    opt_occlusion_->SetName("OptOcclusion");
    opt_occlusion_->SetOptionName(localization->Get("OCCLUSION"));
    opt_occlusion_->SetStyleAuto();
    opt_occlusion_->SetTags({ "graphics" });
    opt_occlusion_->SetStrings(options_bool);

    opt_instancing_ = new UIMultiOption(context_);
    opt_instancing_->SetName("OptInstancing");
    opt_instancing_->SetOptionName(localization->Get("INSTANCING"));
    opt_instancing_->SetStyleAuto();
    opt_instancing_->SetTags({ "graphics" });
    opt_instancing_->SetStrings(options_bool);

    opt_specular_ = new UIMultiOption(context_);
    opt_specular_->SetName("OptSpecular");
    opt_specular_->SetOptionName(localization->Get("SPECULAR_LIGHTING"));
    opt_specular_->SetStyleAuto();
    opt_specular_->SetTags({ "graphics" });
    opt_specular_->SetStrings(options_bool);

    opt_hdr_ = new UIMultiOption(context_);
    opt_hdr_->SetName("OptHdr");
    opt_hdr_->SetOptionName("HDR");
    opt_hdr_->SetStyleAuto();
    opt_hdr_->SetTags({ "graphics" });
    opt_hdr_->SetStrings(options_bool);

    opt_ssao_ = new UIMultiOption(context_);
    opt_ssao_->SetName("OptSSAO");
    opt_ssao_->SetOptionName("SSAO");
    opt_ssao_->SetStyleAuto();
    opt_ssao_->SetTags({ "misc-video" });
    opt_ssao_->SetStrings(options_bool);

//    graphics_tab->AddItem(opt_texture_quality_);
    graphics_tab->AddItem(opt_material_quality_);
    graphics_tab->AddItem(opt_shadows_);
    graphics_tab->AddItem(opt_shadow_quality_);
    graphics_tab->AddItem(opt_occlusion_);
    graphics_tab->AddItem(opt_instancing_);
    graphics_tab->AddItem(opt_specular_);
    graphics_tab->AddItem(opt_hdr_);
#if !defined(__ANDROID__) && !defined(__EMSCRIPTEN__)
    graphics_tab->AddItem(opt_ssao_);
#endif
}

void SettingsWindow::CreateAudioTab()
{
    auto* localization = GetSubsystem<Localization>();
    auto audio_tab = tabs_->AddTab(localization->Get("AUDIO"));

    audio_settings_[SOUND_MASTER] = new UISliderOption(context_);
    audio_settings_[SOUND_MASTER]->SetName("Master");
    audio_settings_[SOUND_MASTER]->SetOptionName("Master volume");
    audio_settings_[SOUND_MASTER]->SetRange(1.0f);
    audio_settings_[SOUND_MASTER]->SetValue(GetSubsystem<Audio>()->GetMasterGain(SOUND_MASTER));
    audio_settings_[SOUND_MASTER]->SetStyleAuto();
    audio_settings_[SOUND_MASTER]->SetTags({"audio"});
    audio_settings_[SOUND_MASTER]->SetVar("AudioType", SOUND_MASTER);
    audio_tab->AddItem(audio_settings_[SOUND_MASTER]);

    audio_settings_[SOUND_EFFECT] = new UISliderOption(context_);
    audio_settings_[SOUND_EFFECT]->SetName("Master");
    audio_settings_[SOUND_EFFECT]->SetOptionName("Sound effects volume");
    audio_settings_[SOUND_EFFECT]->SetRange(1.0f);
    audio_settings_[SOUND_EFFECT]->SetValue(GetSubsystem<Audio>()->GetMasterGain(SOUND_EFFECT));
    audio_settings_[SOUND_EFFECT]->SetStyleAuto();
    audio_settings_[SOUND_EFFECT]->SetTags({"audio"});
    audio_settings_[SOUND_EFFECT]->SetVar("AudioType", SOUND_EFFECT);
    audio_tab->AddItem(audio_settings_[SOUND_EFFECT]);

    audio_settings_[SOUND_AMBIENT] = new UISliderOption(context_);
    audio_settings_[SOUND_AMBIENT]->SetName("Master");
    audio_settings_[SOUND_AMBIENT]->SetOptionName("Ambient volume");
    audio_settings_[SOUND_AMBIENT]->SetRange(1.0f);
    audio_settings_[SOUND_AMBIENT]->SetValue(GetSubsystem<Audio>()->GetMasterGain(SOUND_AMBIENT));
    audio_settings_[SOUND_AMBIENT]->SetStyleAuto();
    audio_settings_[SOUND_AMBIENT]->SetTags({"audio"});
    audio_settings_[SOUND_AMBIENT]->SetVar("AudioType", SOUND_AMBIENT);
    audio_tab->AddItem(audio_settings_[SOUND_AMBIENT]);

    audio_settings_[SOUND_VOICE] = new UISliderOption(context_);
    audio_settings_[SOUND_VOICE]->SetName("Master");
    audio_settings_[SOUND_VOICE]->SetOptionName("Voice volume");
    audio_settings_[SOUND_VOICE]->SetRange(1.0f);
    audio_settings_[SOUND_VOICE]->SetValue(GetSubsystem<Audio>()->GetMasterGain(SOUND_VOICE));
    audio_settings_[SOUND_VOICE]->SetStyleAuto();
    audio_settings_[SOUND_VOICE]->SetTags({"audio"});
    audio_settings_[SOUND_VOICE]->SetVar("AudioType", SOUND_VOICE);
    audio_tab->AddItem(audio_settings_[SOUND_VOICE]);

    audio_settings_[SOUND_MUSIC] = new UISliderOption(context_);
    audio_settings_[SOUND_MUSIC]->SetName("Master");
    audio_settings_[SOUND_MUSIC]->SetOptionName("Music volume");
    audio_settings_[SOUND_MUSIC]->SetRange(1.0f);
    audio_settings_[SOUND_MUSIC]->SetValue(GetSubsystem<Audio>()->GetMasterGain(SOUND_MUSIC));
    audio_settings_[SOUND_MUSIC]->SetStyleAuto();
    audio_settings_[SOUND_MUSIC]->SetTags({"audio"});
    audio_settings_[SOUND_MUSIC]->SetVar("AudioType", SOUND_MUSIC);
    audio_tab->AddItem(audio_settings_[SOUND_MUSIC]);
}

void SettingsWindow::CreateControllersTab()
{
    auto* localization = GetSubsystem<Localization>();
    auto controllers_tab = tabs_->AddTab(localization->Get("CONTROLLERS"));

    {
        auto label = new Text(context_);
        label->SetStyleAuto();
        label->SetText(localization->Get("MOUSE"));
        controllers_tab->AddItem(label);

        invert_mouse_x = new UIBoolOption(context_);
        invert_mouse_x->SetOptionName(localization->Get("INVERT_X_AXIS"));
        invert_mouse_x->SetOptionValue(GetSubsystem<ConfigManager>()->GetBool("mouse", "InvertX", false));
        invert_mouse_x->SetTags({"invert_mouse_x"});
        invert_mouse_x->SetStyleAuto();
        controllers_tab->AddItem(invert_mouse_x);

        invert_mouse_y = new UIBoolOption(context_);
        invert_mouse_y->SetOptionName(localization->Get("INVERT_Y_AXIS"));
        invert_mouse_y->SetOptionValue(GetSubsystem<ConfigManager>()->GetBool("mouse", "InvertY", false));
        invert_mouse_y->SetTags({"invert_mouse_y"});
        invert_mouse_y->SetStyleAuto();
        controllers_tab->AddItem(invert_mouse_y);

        mouse_sensitivity = new UISliderOption(context_);
        mouse_sensitivity->SetName("Mouse");
        mouse_sensitivity->SetOptionName(localization->Get("SENSITIVITY"));
        mouse_sensitivity->SetRange(10.0f);
        mouse_sensitivity->SetValue(GetSubsystem<ConfigManager>()->GetFloat("mouse", "Sensitivity", 2.0f));
        mouse_sensitivity->SetStyleAuto();
        mouse_sensitivity->SetTags({"mouse_sensitivity"});
        controllers_tab->AddItem(mouse_sensitivity);
    }

    {
        auto label = new Text(context_);
        label->SetStyleAuto();
        label->SetText(localization->Get("JOYSTICK"));
        controllers_tab->AddItem(label);

        invert_joystic_x = new UIBoolOption(context_);
        invert_joystic_x->SetOptionName(localization->Get("INVERT_X_AXIS"));
        invert_joystic_x->SetOptionValue(GetSubsystem<ConfigManager>()->GetBool("joystick", "InvertX", false));
        invert_joystic_x->SetTags({"invert_joystick_x"});
        invert_joystic_x->SetStyleAuto();
        controllers_tab->AddItem(invert_joystic_x);

        invert_joystick_y = new UIBoolOption(context_);
        invert_joystick_y->SetOptionName(localization->Get("INVERT_Y_AXIS"));
        invert_joystick_y->SetOptionValue(GetSubsystem<ConfigManager>()->GetBool("joystick", "InvertY", false));
        invert_joystick_y->SetTags({"invert_joystick_y"});
        invert_joystick_y->SetStyleAuto();
        controllers_tab->AddItem(invert_joystick_y);

        joystick_sensitivity_x = new UISliderOption(context_);
        joystick_sensitivity_x->SetName("Mouse");
        joystick_sensitivity_x->SetOptionName(localization->Get("SENSITIVITY_X_AXIS"));
        joystick_sensitivity_x->SetRange(100.0f);
        joystick_sensitivity_x->SetValue(GetSubsystem<ConfigManager>()->GetFloat("joystick", "SensitivityX", 2.0f));
        joystick_sensitivity_x->SetStyleAuto();
        joystick_sensitivity_x->SetTags({"joystick_sensitivity_x"});
        controllers_tab->AddItem(joystick_sensitivity_x);

        joystick_sensitivity_y = new UISliderOption(context_);
        joystick_sensitivity_y->SetName("Mouse");
        joystick_sensitivity_y->SetOptionName(localization->Get("SENSITIVITY_Y_AXIS"));
        joystick_sensitivity_y->SetRange(100.0f);
        joystick_sensitivity_y->SetValue(GetSubsystem<ConfigManager>()->GetFloat("joystick", "SensitivityY", 2.0f));
        joystick_sensitivity_y->SetStyleAuto();
        joystick_sensitivity_y->SetTags({"joystick_sensitivity_y"});
        controllers_tab->AddItem(joystick_sensitivity_y);

        deadzone_ = new UISliderOption(context_);
        deadzone_->SetName("Mouse");
        deadzone_->SetOptionName(localization->Get("DEADZONE"));
        deadzone_->SetRange(10.0f);
        deadzone_->SetValue(GetSubsystem<ConfigManager>()->GetFloat("joystick", "Deadzone", 0.5f));
        deadzone_->SetStyleAuto();
        deadzone_->SetTags({"deadzone"});
        controllers_tab->AddItem(deadzone_);
    }

    {
        multiple_controllers_ = new UIBoolOption(context_);
        multiple_controllers_->SetOptionName(localization->Get("MULTIPLE_CONTROLLER_SUPPORT"));
        multiple_controllers_->SetOptionValue(GetSubsystem<ConfigManager>()->GetBool("joystick", "MultipleControllers", true));
        multiple_controllers_->SetTags({"multiple_controllers"});
        multiple_controllers_->SetStyleAuto();
        controllers_tab->AddItem(multiple_controllers_);

        joystick_as_first_ = new UIBoolOption(context_);
        joystick_as_first_->SetOptionName(localization->Get("JOYSTICK_AS_FIRST_CONTROLLER"));
        joystick_as_first_->SetOptionValue(GetSubsystem<ConfigManager>()->GetBool("joystick", "JoystickAsFirstController", true));
        joystick_as_first_->SetTags({"joystick_as_first"});
        joystick_as_first_->SetStyleAuto();
        controllers_tab->AddItem(joystick_as_first_);
    }
}

void SettingsWindow::CreateControlsTab()
{
    auto* localization = GetSubsystem<Localization>();
    auto controls_tab = tabs_->AddTab(localization->Get("CONTROLS"));

    auto controllerInput = GetSubsystem<ControllerInput>();
    auto names = controllerInput->GetControlNames();

    // Loop trough all of the controls
    for (auto it = names.Begin(); it != names.End(); ++it) {
        int actionId = (*it).first_;
        //controllerInput->GetActionKeyName(actionId)
        auto control = new UIOption(context_);

        control->SetOptionName((*it).second_);
        control->SetStyleAuto();

        auto button = control->GetControl()->CreateChild<Button>();
        button->SetVar("Action", actionId);
        button->SetStyleAuto();
        button->SetName("Controls_" + String(actionId));
        button->SetStyle("TransparentButton");

        control_mappings_[actionId] = button->CreateChild<Text>();
        control_mappings_[actionId]->SetName("Label");
        control_mappings_[actionId]->SetStyleAuto();
        control_mappings_[actionId]->SetText(controllerInput->GetActionKeyName(actionId));
        control_mappings_[actionId]->SetAlignment(HA_CENTER, VA_CENTER);

        controls_tab->AddItem(control);


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

    SubscribeToEvent(MyEvents::E_INPUT_MAPPING_FINISHED, [&](StringHash eventType, VariantMap& eventData) {
        RefreshControlsTab();
        Input* input = GetSubsystem<Input>();
        if (!input->IsMouseVisible()) {
            input->SetMouseVisible(true);
        }
    });

    SubscribeToEvent(MyEvents::E_STOP_INPUT_MAPPING, [&](StringHash eventType, VariantMap& eventData) {
        RefreshControlsTab();
        Input* input = GetSubsystem<Input>();
        if (!input->IsMouseVisible()) {
            input->SetMouseVisible(true);
        }
    });
}

void SettingsWindow::RefreshControlsTab()
{
    auto controllerInput = GetSubsystem<ControllerInput>();
    auto names = controllerInput->GetControlNames();

    // Loop trough all of the controls and update their labels
    for (auto it = names.Begin(); it != names.End(); ++it) {
        int actionId = (*it).first_;
        if (control_mappings_.Contains(actionId) && control_mappings_[actionId]) {
            control_mappings_[actionId]->SetText(controllerInput->GetActionKeyName(actionId));
        }
    }
}

void SettingsWindow::CreateGameTab()
{
    auto* localization = GetSubsystem<Localization>();
    auto game_tab = tabs_->AddTab(localization->Get("GAME"));

    // graphics tab
    language_selection_ = new UIMultiOption(context_);
    language_selection_->SetName("Language");
    language_selection_->SetOptionName(localization->Get("LANGUAGE"));
    language_selection_->SetStyleAuto();
    language_selection_->SetTags({ "language" });

    StringVector languages;
    for (unsigned int i = 0; i < localization->GetNumLanguages(); i++) {
        languages.Push(localization->GetLanguage(i));
    }
    language_selection_->SetStrings(languages);
    language_selection_->SetOptionIndex(localization->GetLanguageIndex());
    game_tab->AddItem(language_selection_);

    enable_mods_ = new UIBoolOption(context_);
    enable_mods_->SetName("Mods");
    enable_mods_->SetOptionName(localization->Get("MODS"));
    enable_mods_->SetStyleAuto();
    enable_mods_->SetTags({ "mods" });
    enable_mods_->SetOptionValue(GetSubsystem<ConfigManager>()->GetBool("game", "LoadMods", true));
    game_tab->AddItem(enable_mods_);

    developer_console_ = new UIBoolOption(context_);
    developer_console_->SetName("DeveloperConsole");
    developer_console_->SetOptionName(localization->Get("DEVELOPER_CONSOLE"));
    developer_console_->SetStyleAuto();
    developer_console_->SetTags({ "developer_console" });
    developer_console_->SetOptionValue(GetSubsystem<ConfigManager>()->GetBool("game", "DeveloperConsole", true));
    game_tab->AddItem(developer_console_);

    auto elm = new UIElement(context_);
    elm->SetMinSize(100, 32);
    elm->SetMaxHeight(30);
    elm->SetVerticalAlignment(VA_TOP);
    clear_achievements_ = new Button(context_);
    clear_achievements_->SetFixedSize(200, 28);

    auto btn_text = new Text(context_);
    btn_text->SetText(localization->Get("CLEAR_ACHIEVEMENTS"));
    clear_achievements_->AddChild(btn_text);
    btn_text->SetAlignment(HA_CENTER, VA_CENTER);

    elm->AddChild(clear_achievements_);
    clear_achievements_->SetHorizontalAlignment(HA_RIGHT);

    clear_achievements_->SetStyleAuto();
    btn_text->SetStyleAuto();
    elm->SetStyleAuto();
    game_tab->AddItem(elm);

    SubscribeToEvent(clear_achievements_, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        GetSubsystem<Achievements>()->ClearAchievementsProgress();

        VariantMap& data = GetEventDataMap();
        data["Message"] = "Achievements cleared!";
        SendEvent("ShowNotification", data);
    });
}

void SettingsWindow::FillRates(int monitor) {
    auto rates = GetFullscreenRefreshRates(monitor);
    for (unsigned i = 0; i < rates.Size() / 2; i++) {
        Swap(*(rates.Begin() + i), *(rates.Begin() + rates.Size() - 1 - i));
    }
    StringVector res;
    for (auto r : rates) {
        res.Push(ToString("%d", r));
    }

    opt_rate_->SetStrings(res);
    opt_rate_->SetOptionIndex(res.Size() - 1);
}

void SettingsWindow::FillResolutions(int monitor, int rate) {
    ResolutionVector resolutions = GetFullscreenResolutions(monitor, rate);
    for (unsigned i = 0; i < resolutions.Size() / 2; i++) {
        Swap(*(resolutions.Begin() + i), *(resolutions.Begin() + resolutions.Size() - 1 - i));
    }
    StringVector res;
    for (auto r : resolutions) {
        res.Push((r.ToString(false)));
    }

    opt_resolution_->SetStrings(res);
    opt_resolution_->SetOptionIndex(res.Size() - 1);
}

void SettingsWindow::RefreshVideoOptions() {
    refreshing_ = true;

    Graphics* graphics = context_->GetSubsystem<Graphics>();
    if (!graphics->GetFullscreen() && !graphics->GetBorderless())
    {
        windowed_resolution_ = graphics->GetSize();
        windowed_position_ = graphics->GetWindowPosition();
    }

    btn_apply_->SetFocusMode(needs_apply_ ? FM_FOCUSABLE : FM_NOTFOCUSABLE);
    btn_apply_->SetEnabled(needs_apply_);

    int monitor = graphics->GetMonitor();

    StringVector monitor_names;
    for (int i = 0; i < GetMonitorCount(); i++) {
        monitor_names.Push(SDL_GetDisplayName(i));
    }

    opt_monitor_->SetStrings(monitor_names);
    opt_monitor_->SetOptionIndex(monitor);

    FillRates(monitor);

    int rate = -1;
    rate = ToInt(opt_rate_->GetValue());
    FillResolutions(monitor, rate);

    RenderWindowMode mode = RWM_WINDOWED;
    if (!graphics->GetFullscreen() && graphics->GetBorderless()) {
        mode = RWM_FULLSCREEN_WINDOW;
    } else
        mode = RWM_FULLSCREEN;

    opt_fullscreen_->SetOptionIndex((int)RWM_WINDOWED);

    IntVector2 graphics_size = graphics->GetSize();

    // find the current fullscreen resolution and set the resolution option to it
    auto res_index = -1;
    if (graphics->GetFullscreen()) {
        int refreshrate = graphics->GetRefreshRate();
        ResolutionVector resolutions = GetFullscreenResolutions(
                opt_monitor_->GetOptionIndex(),
                ToInt(opt_rate_->GetValue()));
        // reverse resolutions to low -> high
        for (unsigned i = 0; i < resolutions.Size() / 2; i++) {
            Swap(*(resolutions.Begin() + i), *(resolutions.Begin() + resolutions.Size() - 1 - i));
        }

        int i = 0;
        for (auto it = resolutions.Begin(); it != resolutions.End(); ++it, ++i) {
            if (it->width == graphics_size.x_ && it->height == graphics_size.y_ && it->refreshrate == refreshrate) {
                res_index = i;
                break;
            }
        }
    }

    if (res_index != -1)
        opt_resolution_->SetOptionIndex(res_index);


    opt_vsync_->SetOptionValue(graphics->GetVSync());

    opt_resizable_->SetOptionValue(graphics->GetResizable());

    refreshing_ = false;
}

void SettingsWindow::ApplyVideoOptions()
{
    Graphics* graphics = context_->GetSubsystem<Graphics>();

    int fullscreen = opt_fullscreen_->GetOptionIndex();

    Resolution res;


    // In fullscreen borderless window, resolution must be 0x0, Urho3D will apply accordingly
    URHO3D_LOGINFOF("fullscreen %d", fullscreen);
    if (fullscreen == RWM_WINDOWED) {
        res = Resolution::FromString(opt_resolution_->GetValue());
    } if (fullscreen == RWM_FULLSCREEN_WINDOW) {
        res = Resolution(0, 0);
    } else if (fullscreen == RWM_FULLSCREEN) {
        res = Resolution::FromString(opt_resolution_->GetValue());
        res.refreshrate = ToInt(opt_rate_->GetValue());
    }

    graphics->SetMode(
            res.width,
            res.height,
            fullscreen == 2,
            fullscreen == 1,
            true,
            false,
            !!opt_vsync_->GetOptionValue(),
            false,
            0,
            opt_monitor_->GetOptionIndex(),
            res.refreshrate
    );

    if (fullscreen == 0) {
        graphics->SetWindowPosition(windowed_position_);
    }

    SDL_RaiseWindow(graphics->GetWindow());

    GetSubsystem<ConfigManager>()->Set("video", "WindowMode", fullscreen);
    GetSubsystem<ConfigManager>()->Set("video", "Width", res.width);
    GetSubsystem<ConfigManager>()->Set("video", "Height", res.height);
    GetSubsystem<ConfigManager>()->Set("video", "RefreshRate", res.refreshrate);
    GetSubsystem<ConfigManager>()->Set("video", "VSync", opt_vsync_->GetOptionValue());
    GetSubsystem<ConfigManager>()->Set("video", "Monitor", opt_monitor_->GetOptionIndex());

    SendEvent(MyEvents::E_VIDEO_SETTINGS_CHANGED);

}

void SettingsWindow::RefreshGraphicsOptions()
{
    // mark as refreshing so options that are being read don't get applied
    refreshing_ = true;
    auto renderer = GetSubsystem<Renderer>();

    opt_texture_quality_->SetOptionIndex((int)renderer->GetTextureQuality());
    opt_material_quality_->SetOptionIndex((int)renderer->GetMaterialQuality());

    if (renderer->GetDrawShadows()) {
        opt_shadows_->SetOptionIndex(renderer->GetShadowMapSize() / 512);
    } else
        opt_shadows_->SetOptionIndex(0);

    opt_shadow_quality_->SetOptionIndex((int)renderer->GetShadowQuality());
    opt_occlusion_->SetOptionIndex(renderer->GetMaxOccluderTriangles() > 0 ? 1 : 0);
    opt_instancing_->SetOptionIndex(renderer->GetDynamicInstancing() ? 1 : 0);
    opt_specular_->SetOptionIndex(renderer->GetSpecularLighting() ? 1 : 0);
    opt_hdr_->SetOptionIndex(renderer->GetHDRRendering() ? 1 : 0);
    opt_ssao_->SetOptionIndex(GetSubsystem<ConfigManager>()->GetBool("postprocess", "SSAO", true) ? 1 : 0);

    refreshing_ = false;
}

void SettingsWindow::ApplyGraphicsOptions()
{
    if (refreshing_)
        return;

    auto renderer = GetSubsystem<Renderer>();

    renderer->SetTextureQuality((Urho3D::MaterialQuality)opt_texture_quality_->GetOptionIndex());
    renderer->SetMaterialQuality((Urho3D::MaterialQuality)opt_material_quality_->GetOptionIndex());
    renderer->SetDrawShadows(opt_shadows_->GetOptionIndex() != 0);
    renderer->SetShadowMapSize(opt_shadows_->GetOptionIndex() * 512);
    renderer->SetShadowQuality((ShadowQuality)opt_shadow_quality_->GetOptionIndex());
    renderer->SetMaxOccluderTriangles(opt_occlusion_->GetOptionIndex() > 0 ? 5000 : 0);
    renderer->SetDynamicInstancing(opt_instancing_->GetOptionIndex() > 0);
    renderer->SetSpecularLighting(opt_specular_->GetOptionIndex() > 0);
    renderer->SetHDRRendering(opt_hdr_->GetOptionIndex() > 0);

    GetSubsystem<ConfigManager>()->Set("graphics", "TextureQuality", opt_texture_quality_->GetOptionIndex());
    GetSubsystem<ConfigManager>()->Set("graphics", "MaterialQuality", opt_material_quality_->GetOptionIndex());
    GetSubsystem<ConfigManager>()->Set("graphics", "DrawShadows", opt_shadows_->GetOptionIndex() != 0);
    GetSubsystem<ConfigManager>()->Set("graphics", "ShadowMapSize", opt_shadows_->GetOptionIndex() * 512);
    GetSubsystem<ConfigManager>()->Set("graphics", "ShadowQuality", opt_shadow_quality_->GetOptionIndex());
    GetSubsystem<ConfigManager>()->Set("graphics", "MaxOccluderTriangles", opt_occlusion_->GetOptionIndex() > 0 ? 5000 : 0);
    GetSubsystem<ConfigManager>()->Set("graphics", "DynamicInstancing", opt_instancing_->GetOptionIndex() > 0);
    GetSubsystem<ConfigManager>()->Set("graphics", "SpecularLighting", opt_specular_->GetOptionIndex() > 0);
    GetSubsystem<ConfigManager>()->Set("graphics", "HDRRendering", opt_hdr_->GetOptionIndex() > 0);
}

void SettingsWindow::HandleTabChanged(StringHash eventType, VariantMap& eventData) {
    using namespace UITabChanged;
    int index = eventData[P_INDEX].GetInt();

    switch (index) {
        case 0:
            RefreshVideoOptions();
            break;
        case 1:
            RefreshGraphicsOptions();
            break;
        default:
            break;
    }
}

void SettingsWindow::HandleOptionChanged(StringHash eventType, VariantMap& eventData)
{
    if (refreshing_)
        return;

    using namespace UIOptionChanged;
    auto option = static_cast<UIOption*>(eventData[P_OPTION].GetPtr());
    auto name = option->GetName();

    if (name == "OptMonitor") {
        int monitor = opt_monitor_->GetOptionIndex();
        FillRates(monitor);
        int rate = ToInt(opt_rate_->GetValue());
        FillResolutions(monitor, rate);
    } else if (name == "OptRate") {
        int rate = ToInt(opt_rate_->GetValue());
        int monitor = opt_monitor_->GetOptionIndex();
        FillResolutions(monitor, rate);
    }

    needs_apply_ |= option->HasTag("video");
    btn_apply_->SetFocusMode(needs_apply_ ? FM_FOCUSABLE : FM_NOTFOCUSABLE);
    btn_apply_->SetEnabled(needs_apply_);

    if (option->HasTag("misc-video")) {
        auto graphics = GetSubsystem<Graphics>();
        SDL_SetWindowResizable(graphics->GetWindow(), opt_resizable_->GetOptionValue() ? SDL_TRUE : SDL_FALSE);

        auto engine = GetSubsystem<Engine>();
        int fps_limit = 0;
        if (opt_fpslimit_->GetOptionIndex() == 0) {
            fps_limit = GetSubsystem<Engine>()->GetMaxFps();
        } else if (opt_fpslimit_->GetOptionIndex() > 1) {
            fps_limit = Clamp(ToInt(opt_fpslimit_->GetValue()), 30, 300);
        } else if (opt_fpslimit_->GetOptionIndex() == 1) {
            fps_limit = 0;
        }

        engine->SetMaxFps(fps_limit);
        GetSubsystem<ConfigManager>()->Set("video", "ResizableWindow", opt_resizable_->GetOptionValue() ? true : false);
        GetSubsystem<ConfigManager>()->Set("engine", "FPSLimit", fps_limit);
        GetSubsystem<ConfigManager>()->Set("postprocess", "Gamma", gamma_->GetValue());
        GetSubsystem<ConfigManager>()->Set("postprocess", "SSAO", opt_ssao_->GetOptionIndex() > 0);

        SendEvent("postprocess");
    }

    if (option->HasTag("graphics")) {
        ApplyGraphicsOptions();
    }

    // Audio settings
    if (option->HasTag("audio")) {
        String type = option->GetVar("AudioType").GetString();
        URHO3D_LOGINFOF("Audio: %s", type.CString());
        if (!type.Empty()) {
            GetSubsystem<Audio>()->SetMasterGain(type, audio_settings_[type]->GetValue());
            GetSubsystem<ConfigManager>()->Set("audio", type, audio_settings_[type]->GetValue());
        }
    }

    // Controller settings
    auto controllerInput = GetSubsystem<ControllerInput>();
    if (option->HasTag("invert_mouse_x")) {
        controllerInput->SetInvertX(invert_mouse_x->GetOptionValue(), ControllerType::MOUSE);
        GetSubsystem<ConfigManager>()->Set("mouse", "InvertX", invert_mouse_x->GetOptionValue());
        VariantMap& data = GetEventDataMap();
        data["Message"] = "Invert mouse X!";
        SendEvent("ShowNotification", data);
    }
    if (option->HasTag("invert_mouse_y")) {
        controllerInput->SetInvertX(invert_mouse_y->GetOptionValue(), ControllerType::MOUSE);
        GetSubsystem<ConfigManager>()->Set("mouse", "InvertY", invert_mouse_y->GetOptionValue());
    }

    if (option->HasTag("invert_joystick_x")) {
        controllerInput->SetInvertX(invert_joystic_x->GetOptionValue(), ControllerType::JOYSTICK);
        GetSubsystem<ConfigManager>()->Set("joystick", "InvertX", invert_joystic_x->GetOptionValue());
    }
    if (option->HasTag("invert_joystick_y")) {
        controllerInput->SetInvertX(invert_joystick_y->GetOptionValue(), ControllerType::JOYSTICK);
        GetSubsystem<ConfigManager>()->Set("joystick", "InvertY", invert_joystick_y->GetOptionValue());
    }
    if (option->HasTag("multiple_controllers")) {
        controllerInput->SetMultipleControllerSupport(multiple_controllers_->GetOptionValue());
        GetSubsystem<ConfigManager>()->Set("joystick", "MultipleControllers", multiple_controllers_->GetOptionValue());
    }

    if (option->HasTag("joystick_as_first")) {
        controllerInput->SetJoystickAsFirstController(joystick_as_first_->GetOptionValue());
        GetSubsystem<ConfigManager>()->Set("joystick", "JoystickAsFirstController", joystick_as_first_->GetOptionValue());
    }

    if (option->HasTag("mouse_sensitivity")) {
        controllerInput->SetSensitivityX(mouse_sensitivity->GetValue(), ControllerType::MOUSE);
        controllerInput->SetSensitivityY(mouse_sensitivity->GetValue(), ControllerType::MOUSE);
        GetSubsystem<ConfigManager>()->Set("mouse", "Sensitivity", mouse_sensitivity->GetValue());
    }

    if (option->HasTag("joystick_sensitivity_x")) {
        controllerInput->SetSensitivityX(joystick_sensitivity_x->GetValue(), ControllerType::JOYSTICK);
        GetSubsystem<ConfigManager>()->Set("joystick", "SensitivityX", joystick_sensitivity_x->GetValue());
    }

    if (option->HasTag("joystick_sensitivity_y")) {
        controllerInput->SetSensitivityY(joystick_sensitivity_y->GetValue(), ControllerType::JOYSTICK);
        GetSubsystem<ConfigManager>()->Set("joystick", "SensitivityY", joystick_sensitivity_y->GetValue());
    }

    if (option->HasTag("deadzone")) {
        controllerInput->SetJoystickDeadzone(deadzone_->GetValue());
        GetSubsystem<ConfigManager>()->Set("joystick", "Deadzone", deadzone_->GetValue());
    }

    // Game settings
    if (option->HasTag("language")) {
        GetSubsystem<ConfigManager>()->Set("game", "Language", language_selection_->GetValue());

        auto* localization = GetSubsystem<Localization>();
        VariantMap& data = GetEventDataMap();
        data["Title"] = localization->Get("WARNING");
        data["Message"] = localization->Get("RESTART_TO_APPLY");
        data["Name"] = "PopupMessageWindow";
        data["Type"] = "warning";
        data["ClosePrevious"] = true;
        SendEvent(MyEvents::E_OPEN_WINDOW, data);
    }

    if (option->HasTag("mods")) {
        GetSubsystem<ConfigManager>()->Set("game", "LoadMods", enable_mods_->GetOptionValue());

        auto* localization = GetSubsystem<Localization>();
        VariantMap& data = GetEventDataMap();
        data["Title"] = localization->Get("WARNING");
        data["Message"] = localization->Get("RESTART_TO_APPLY");
        data["Name"] = "PopupMessageWindow";
        data["Type"] = "warning";
        data["ClosePrevious"] = true;
        SendEvent(MyEvents::E_OPEN_WINDOW, data);
    }

    if (option->HasTag("developer_console")) {
        GetSubsystem<ConfigManager>()->Set("game", "DeveloperConsole", developer_console_->GetOptionValue());
    }

    GetSubsystem<ConfigManager>()->Save(true);
}

void SettingsWindow::HandleApply(StringHash eventType, VariantMap& eventData)
{
    ApplyVideoOptions();
    needs_apply_ = false;
}
