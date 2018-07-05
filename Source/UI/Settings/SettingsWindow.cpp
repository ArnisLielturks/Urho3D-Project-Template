#include <Urho3D/Urho3DAll.h>
#include "SettingsWindow.h"
#include "../../MyEvents.h"

/// Construct.
SettingsWindow::SettingsWindow(Context* context) :
    BaseWindow(context, IntVector2(400, 300))
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

    _closeButton = CreateButton("X", IntVector2(-5, 5), IntVector2(20, 20), HA_RIGHT, VA_TOP);

    int index = 0;
    _playerTabButton = CreateButton("Player", IntVector2(margin + index * margin + index * width, 5), IntVector2(width, 20), HA_LEFT, VA_TOP);
    index++;
    _audioTabButton = CreateButton("Audio", IntVector2(margin + index * margin + index * width, 5), IntVector2(width, 20), HA_LEFT, VA_TOP);
    index++;
    _graphicsTabButton = CreateButton("Video", IntVector2(margin + index * margin + index * width, 5), IntVector2(width, 20), HA_LEFT, VA_TOP);
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
    // SubscribeToEvent(_graphicsTabButton, E_RELEASED, URHO3D_HANDLER(SettingsWindow, ChangeVideoSettings));
}

void SettingsWindow::HandleClose(StringHash eventType, VariantMap& eventData)
{
    VariantMap data;
    data["Name"] = "SettingsWindow";
    SendEvent(MyEvents::E_CLOSE_WINDOW, data);
}

void SettingsWindow::ChangeVideoSettings(StringHash eventType, VariantMap& eventData)
{

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