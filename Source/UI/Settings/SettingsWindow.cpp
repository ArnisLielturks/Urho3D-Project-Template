#include <Urho3D/Urho3DAll.h>
#include "SettingsWindow.h"
#include "../../MyEvents.h"
#include "../../Input/ControllerInput.h"

/// Construct.
SettingsWindow::SettingsWindow(Context* context) :
    BaseWindow(context, IntVector2(400, 400))
{
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
    ClearView();
    _openedView = SettingsViewType::VIDEO_VIEW;
    auto* graphics = GetSubsystem<Graphics>();

    String activeResolution = String(GetGlobalVar("ScreenWidth").GetInt()) + "x" + String(GetGlobalVar("ScreenHeight").GetInt());

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
    _activeSettingElements.Push(CreateMenu(_base, "Resolution", supportedResolutions, activeIndex, IntVector2(20, 60)));
    _activeSettingElements.Push(CreateCheckbox(_base, "Fullscreen", GetGlobalVar("Fullscreen").GetBool(), IntVector2(20, 90)));
    _activeSettingElements.Push(CreateCheckbox(_base, "Vertical sync", GetGlobalVar("VSync").GetBool(), IntVector2(20, 120)));
    _activeSettingElements.Push(CreateCheckbox(_base, "Triple buffer", GetGlobalVar("TripleBuffer").GetBool(), IntVector2(20, 150)));

    _activeSettingElements.Push(CreateCheckbox(_base, "Enable shadows", GetGlobalVar("Shadows").GetBool(), IntVector2(20, 200)));
    _activeSettingElements.Push(CreateCheckbox(_base, "Low quality shadows", GetGlobalVar("LowQualityShadows").GetBool(), IntVector2(20, 230)));

    StringVector supportedQualitySettings;
    supportedQualitySettings.Push("Low"); //0
    supportedQualitySettings.Push("Medium"); // 1
    supportedQualitySettings.Push("High"); // 2
    supportedQualitySettings.Push("Max"); // 15
    int activeTextureQualityIndex = GetGlobalVar("TextureQuality").GetInt();
    if (activeTextureQualityIndex > 2) {
        activeTextureQualityIndex = 3;
    }
    _activeSettingElements.Push(CreateMenu(_base, "TextureQuality", supportedQualitySettings, activeTextureQualityIndex, IntVector2(20, 260)));

    // FILTER_NEAREST = 0,
    // FILTER_BILINEAR = 1,
    // FILTER_TRILINEAR = 2,
    // FILTER_ANISOTROPIC = 3,
    // FILTER_NEAREST_ANISOTROPIC = 4,
    // FILTER_DEFAULT = 5,
    // MAX_FILTERMODES
    StringVector textureAnisotropySettings;
    textureAnisotropySettings.Push("Nearest");
    textureAnisotropySettings.Push("Bilinear");
    textureAnisotropySettings.Push("Trilinear");
    textureAnisotropySettings.Push("Anistropic");
    textureAnisotropySettings.Push("Nearest anistropic");
    textureAnisotropySettings.Push("Default");
    textureAnisotropySettings.Push("Max");
    _activeSettingElements.Push(CreateMenu(_base, "Texture anisotropy level", textureAnisotropySettings, GetGlobalVar("TextureAnisotropy").GetInt(), IntVector2(20, 290)));

    //  FILTER_NEAREST = 0,
    // FILTER_BILINEAR,
    // FILTER_TRILINEAR,
    // FILTER_ANISOTROPIC,
    // FILTER_NEAREST_ANISOTROPIC,
    // FILTER_DEFAULT,
    // MAX_FILTERMODES
    StringVector textureFilterModes;
    textureFilterModes.Push("Nearest");
    textureFilterModes.Push("Bilinear");
    textureFilterModes.Push("Trilinear");
    textureFilterModes.Push("Anistropic");
    textureFilterModes.Push("Nearest anistropic");
    textureFilterModes.Push("Default");
    textureFilterModes.Push("Max");
    _activeSettingElements.Push(CreateMenu(_base, "Texture filer mode", textureFilterModes, GetGlobalVar("TextureFilterMode").GetInt(), IntVector2(20, 320)));
}

void SettingsWindow::CreateAudioSettingsView()
{
    ClearView();
    _openedView = SettingsViewType::AUDIO_VIEW;
    // _activeSettingElements.Push(CreateCheckbox(_base, "Audio?", true, IntVector2(20, 60)));
    // _activeSettingElements.Push(CreateCheckbox(_base, "Audio!", false, IntVector2(20, 90)));
    _activeSettingElements.Push(CreateCheckbox(_base, "Enable audio", GetGlobalVar("Sound").GetBool(), IntVector2(20, 60)));
    _activeSettingElements.Push(CreateCheckbox(_base, "Stereo", GetGlobalVar("SoundStereo").GetBool(), IntVector2(20, 90)));
    _activeSettingElements.Push(CreateCheckbox(_base, "Sound interpolation", GetGlobalVar("SoundInterpolation").GetBool(), IntVector2(20, 120)));
	_activeSettingElements.Push(CreateSlider(_base, "Volume", IntVector2(20, 150), GetGlobalVar("SoundVolume").GetFloat()));

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
    // auto* graphics = GetSubsystem<Graphics>();
    // {
    //     URHO3D_LOGINFO("Monitor count: " + String(graphics->GetMonitorCount()));
    //     URHO3D_LOGINFO("Currently selected monitor: " + String(graphics->GetCurrentMonitor()));
    //     IntVector2 desktopResolution = graphics->GetDesktopResolution(0);
    //     //Desktop resolution
    //     URHO3D_LOGINFO(">>>>>>> 0 SCREEN X " + String(desktopResolution.x_) + "; Y " + String(desktopResolution.y_));
    //     PODVector<IntVector3> resolutions = graphics->GetResolutions(0);
    //     for (auto it = resolutions.Begin(); it != resolutions.End(); ++it) {
    //         //All supported resolutions
    //         URHO3D_LOGINFO(">>>>>>> 0 SCREEN X " + String((*it).x_) + "; Y " + String((*it).y_) + "; HZ " + String((*it).z_));
    //     }
    // }
    // graphics->SetMode(
    //     GetGlobalVar("ScreenWidth").GetInt(),
    //     GetGlobalVar("ScreenHeight").GetInt(),
    //     GetGlobalVar("Fullscreen").GetBool(),
    //     false,
    //     false,
    //     false,
    //     GetGlobalVar("VSync").GetBool(),
    //     GetGlobalVar("TripleBuffer").GetBool(),
    //     GetGlobalVar("MultiSample").GetInt(),
    //     GetGlobalVar("Monitor").GetInt(),
    //     0
    // );
    //graphics->ResetRenderTargets();
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