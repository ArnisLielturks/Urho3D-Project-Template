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

SharedPtr<UIElement> SettingsWindow::CreateMenu(const String& label, StringVector options, int selected, IntVector2 position/*, EventHandler* handler*/)
{
    SharedPtr<UIElement> container(new UIElement(context_));
    container->SetPosition(position);
    container->SetAlignment(HA_LEFT, VA_TOP);
    container->SetLayout(LM_HORIZONTAL, 8);
    _base->AddChild(container);

    SharedPtr<Text> text(new Text(context_));
    container->AddChild(text);
    text->SetText(label);
    text->SetStyleAuto();
    text->SetMinWidth(200);
    //text->AddTag(TEXT_TAG);

    SharedPtr<DropDownList> list(new DropDownList(context_));
    container->AddChild(list);
    list->SetMinWidth(150);
    list->SetStyleAuto();

    for (auto it = options.Begin(); it != options.End(); ++it)
    {
        SharedPtr<Text> item(new Text(context_));
        list->AddItem(item);
        item->SetText((*it));
        item->SetStyleAuto();
        item->SetMinWidth(150);
        //item->AddTag(TEXT_TAG);
    }

    list->SetSelection(selected);

    //text->SetMaxWidth(text->GetRowWidth(0));

    //SubscribeToEvent(list, E_ITEMSELECTED, handler);
    return container;
}

SharedPtr<UIElement> SettingsWindow::CreateCheckbox(const String& label, bool isActive, IntVector2 position/*, EventHandler* handler*/)
{
    SharedPtr<UIElement> container(new UIElement(context_));
    container->SetPosition(position);
    container->SetAlignment(HA_LEFT, VA_TOP);
    container->SetLayout(LM_HORIZONTAL, 8);
    _base->AddChild(container);

    SharedPtr<Text> text(new Text(context_));
    container->AddChild(text);
    text->SetText(label);
    text->SetStyleAuto();
    text->SetMinWidth(200);

    SharedPtr<CheckBox> box(new CheckBox(context_));
    container->AddChild(box);
    box->SetStyleAuto();
    box->SetChecked(isActive);

    // text->AddTag(TEXT_TAG);

    //SubscribeToEvent(box, E_TOGGLED, handler);
    return container;
}

SharedPtr<UIElement> SettingsWindow::CreateSlider(const String& text, IntVector2 position, float value)
{
	SharedPtr<UIElement> container(_base->CreateChild<UIElement>());
	container->SetPosition(position);
	container->SetAlignment(HA_LEFT, VA_TOP);
	container->SetLayout(LM_HORIZONTAL, 8);

	auto* cache = GetSubsystem<ResourceCache>();
	auto* font = cache->GetResource<Font>("Fonts/Anonymous Pro.ttf");

	// Create text and slider below it
	auto* sliderText = container->CreateChild<Text>();
	//sliderText->SetPosition(IntV);
	sliderText->SetStyleAuto();
	sliderText->SetFont(font, 12);
	sliderText->SetText(text);
	sliderText->SetMinWidth(200);

	auto* slider = container->CreateChild<Slider>();
	slider->SetStyleAuto();
	//slider->SetPosition(position);
	//slider->SetSize(size);
	slider->SetMinWidth(150);
	// Use 0-1 range for controlling sound/music master volume
	slider->SetRange(1.0f);
	slider->SetValue(value);

	return container;
}

SharedPtr<UIElement> SettingsWindow::CreateControlsElement(const String& text, IntVector2 position, String value, String actionName)
{
	SharedPtr<UIElement> container(new UIElement(context_));//(_base->CreateChild<UIElement>());
    container->SetStyleAuto();
	container->SetPosition(position);
    container->SetMinHeight(30);
	container->SetAlignment(HA_LEFT, VA_TOP);
	container->SetLayout(LM_HORIZONTAL, 8, IntRect(4, 4, 4, 4));

	auto* cache = GetSubsystem<ResourceCache>();
	auto* font = cache->GetResource<Font>("Fonts/Anonymous Pro.ttf");

	// Create text and slider below it
	auto* label = container->CreateChild<Text>();
	//sliderText->SetPosition(IntV);
	label->SetStyleAuto();
	label->SetFont(font, 12);
	label->SetText(text);
	label->SetMinWidth(250);
    label->SetAlignment(HA_LEFT, VA_CENTER);
    label->SetPosition(IntVector2(10, 0));

    Button* button = container->CreateChild<Button>();
    button->SetStyleAuto();
    button->SetMinWidth(40);
    button->SetVar("ActionName", actionName);
    button->SetPosition(IntVector2(-10, 0));

    Text* buttonText = button->CreateChild<Text>();
    buttonText->SetText(value);
    buttonText->SetName(actionName);
    buttonText->SetStyleAuto();
    buttonText->SetAlignment(HA_CENTER, VA_CENTER);

    SubscribeToEvent(button, E_RELEASED, URHO3D_HANDLER(SettingsWindow, HandleChangeControls));

	// auto* lineEdit = container->CreateChild<LineEdit>();
	// lineEdit->SetStyleAuto();
	// lineEdit->SetMinWidth(130);
    // lineEdit->SetText(value);
    // lineEdit->SetVar("ActionName", actionName);

	return container;
}

void SettingsWindow::Create()
{
    UI* ui = GetSubsystem<UI>();

    int margin = 10;
    int width = 110;

    _closeButton = CreateButton("X", IntVector2(-5, 5), IntVector2(20, 20), HA_RIGHT, VA_TOP);

    int index = 0;
    _controlsTabButton = CreateButton("Controls", IntVector2(margin + index * margin + index * width, 5), IntVector2(width, 20), HA_LEFT, VA_TOP);
    index++;
    _audioTabButton = CreateButton("Audio", IntVector2(margin + index * margin + index * width, 5), IntVector2(width, 20), HA_LEFT, VA_TOP);
    index++;
    _graphicsTabButton = CreateButton("Video", IntVector2(margin + index * margin + index * width, 5), IntVector2(width, 20), HA_LEFT, VA_TOP);
    index++;
    _saveButton = CreateButton("Save", IntVector2(-20, -20), IntVector2(width, 20), HA_RIGHT, VA_BOTTOM);

    _saveButton->SetVisible(false);
    CreateControllerSettingsView();
}

void SettingsWindow::CreateGraphicsSettingsView()
{
    ClearView();
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
    _activeSettingElements.Push(CreateMenu("Resolution", supportedResolutions, activeIndex, IntVector2(20, 60)));
    _activeSettingElements.Push(CreateCheckbox("Fullscreen", GetGlobalVar("Fullscreen").GetBool(), IntVector2(20, 90)));
    _activeSettingElements.Push(CreateCheckbox("Vertical sync", GetGlobalVar("VSync").GetBool(), IntVector2(20, 120)));
    _activeSettingElements.Push(CreateCheckbox("Triple buffer", GetGlobalVar("TripleBuffer").GetBool(), IntVector2(20, 150)));

    _activeSettingElements.Push(CreateCheckbox("Enable shadows", GetGlobalVar("Shadows").GetBool(), IntVector2(20, 200)));
    _activeSettingElements.Push(CreateCheckbox("Low quality shadows", GetGlobalVar("LowQualityShadows").GetBool(), IntVector2(20, 230)));

    StringVector supportedQualitySettings;
    supportedQualitySettings.Push("Low"); //0
    supportedQualitySettings.Push("Medium"); // 1
    supportedQualitySettings.Push("High"); // 2
    supportedQualitySettings.Push("Max"); // 15
    int activeTextureQualityIndex = GetGlobalVar("TextureQuality").GetInt();
    if (activeTextureQualityIndex > 2) {
        activeTextureQualityIndex = 3;
    }
    _activeSettingElements.Push(CreateMenu("TextureQuality", supportedQualitySettings, activeTextureQualityIndex, IntVector2(20, 260)));

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
    _activeSettingElements.Push(CreateMenu("Texture anisotropy level", textureAnisotropySettings, GetGlobalVar("TextureAnisotropy").GetInt(), IntVector2(20, 290)));

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
    _activeSettingElements.Push(CreateMenu("Texture filer mode", textureFilterModes, GetGlobalVar("TextureFilterMode").GetInt(), IntVector2(20, 320)));
}

void SettingsWindow::CreateAudioSettingsView()
{
    ClearView();
    // _activeSettingElements.Push(CreateCheckbox("Audio?", true, IntVector2(20, 60)));
    // _activeSettingElements.Push(CreateCheckbox("Audio!", false, IntVector2(20, 90)));
    _activeSettingElements.Push(CreateCheckbox("Enable audio", GetGlobalVar("Sound").GetBool(), IntVector2(20, 60)));
    _activeSettingElements.Push(CreateCheckbox("Stereo", GetGlobalVar("SoundStereo").GetBool(), IntVector2(20, 90)));
    _activeSettingElements.Push(CreateCheckbox("Sound interpolation", GetGlobalVar("SoundInterpolation").GetBool(), IntVector2(20, 120)));
	_activeSettingElements.Push(CreateSlider("Volume", IntVector2(20, 150), GetGlobalVar("SoundVolume").GetFloat()));

}

void SettingsWindow::CreateControllerSettingsView()
{
    ControllerInput* controllerInput = GetSubsystem<ControllerInput>();
    HashMap<int, String> controlNames = controllerInput->GetControlNames();

    ClearView();

    auto* list = _base->CreateChild<ListView>();
    list->SetSelectOnClickEnd(true);
    list->SetHighlightMode(HM_ALWAYS);
    list->SetMinHeight(320);
    list->SetWidth(360);
    list->SetPosition(IntVector2(20, 60));
    list->SetStyleAuto();

    int index = 0;
    for (auto it = controlNames.Begin(); it != controlNames.End(); ++it) {
        SharedPtr<UIElement> singleItem(CreateControlsElement((*it).second_, IntVector2(20, 60 + index * 30), controllerInput->GetActionKeyName((*it).first_), (*it).second_));
        list->AddItem(singleItem);
        _activeSettingElements.Push(singleItem);
        index++;
    }
}

Button* SettingsWindow::CreateButton(String name, IntVector2 position, IntVector2 size, HorizontalAlignment hAlign, VerticalAlignment vAlign)
{
    Button* button = _base->CreateChild<Button>();
    button->SetSize(size);
    button->SetPosition(position);
    button->SetStyleAuto();
    button->SetAlignment(hAlign, vAlign);

    Text* text = button->CreateChild<Text>();
    text->SetText(name);
    text->SetStyleAuto();
    text->SetAlignment(HA_CENTER, VA_CENTER);

    return button;
}

void SettingsWindow::SubscribeToEvents()
{
    SubscribeToEvent(_closeButton, E_RELEASED, URHO3D_HANDLER(SettingsWindow, HandleClose));
    SubscribeToEvent(_graphicsTabButton, E_RELEASED, URHO3D_HANDLER(SettingsWindow, ShowVideoSettings));
    SubscribeToEvent(_controlsTabButton, E_RELEASED, URHO3D_HANDLER(SettingsWindow, ShowControllerSettings));
    SubscribeToEvent(_audioTabButton, E_RELEASED, URHO3D_HANDLER(SettingsWindow, ShowAudioSettings));
    SubscribeToEvent(_saveButton, E_RELEASED, URHO3D_HANDLER(SettingsWindow, HandleSave));

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
    _saveButton->SetVisible(true);
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
    _saveButton->SetVisible(true);
    CreateAudioSettingsView();
}

void SettingsWindow::ShowControllerSettings(StringHash eventType, VariantMap& eventData)
{
    _saveButton->SetVisible(false);
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
    if (!actionName.Empty()) {
        using namespace MyEvents::StartInputMapping;
		VariantMap data = GetEventDataMap();
		data[P_CONTROL_ACTION] = actionName;
		SendEvent(MyEvents::E_START_INPUT_MAPPING, data);
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
}