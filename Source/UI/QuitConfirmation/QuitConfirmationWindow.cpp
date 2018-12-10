#include <Urho3D/Urho3DAll.h>
#include "QuitConfirmationWindow.h"
#include "../../MyEvents.h"
#include "../../Audio/AudioManagerDefs.h"

/// Construct.
QuitConfirmationWindow::QuitConfirmationWindow(Context* context) :
    BaseWindow(context)
{
    Init();
}

QuitConfirmationWindow::~QuitConfirmationWindow()
{
    _yesButton->Remove();
    _noButton->Remove();
    _baseWindow->Remove();
}

void QuitConfirmationWindow::Init()
{
    Create();

    SubscribeToEvents();
}

void QuitConfirmationWindow::Create()
{
    Input* input = GetSubsystem<Input>();
    if (!input->IsMouseVisible()) {
        input->SetMouseVisible(true);
    }

    _baseWindow = GetSubsystem<UI>()->GetRoot()->CreateChild<Window>();
    _baseWindow->SetStyleAuto();
    _baseWindow->SetAlignment(HA_CENTER, VA_CENTER);
    _baseWindow->SetSize(220, 80);
    _baseWindow->BringToFront();

    _yesButton = CreateButton("Yes", 80, IntVector2(20, 0));
    _yesButton->SetAlignment(HA_LEFT, VA_CENTER);

    SubscribeToEvent(_yesButton, "Released", [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "QuitConfirmationWindow";
        SendEvent(MyEvents::E_CLOSE_WINDOW, data);

        data["Name"] = "ExitGame";
        SendEvent(MyEvents::E_SET_LEVEL, data);
    });

    _noButton = CreateButton("No", 80, IntVector2(-20, 0));
    _noButton->SetAlignment(HA_RIGHT, VA_CENTER);

    SubscribeToEvent(_noButton, "Released", [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "QuitConfirmationWindow";
        SendEvent(MyEvents::E_CLOSE_WINDOW, data);
    });
}

void QuitConfirmationWindow::SubscribeToEvents()
{
}

Button* QuitConfirmationWindow::CreateButton(const String& text, int width, IntVector2 position)
{
    auto* cache = GetSubsystem<ResourceCache>();
    auto* font = cache->GetResource<Font>("Fonts/Anonymous Pro.ttf");

    auto* button = _baseWindow->CreateChild<Button>();
    button->SetStyleAuto();
    button->SetFixedWidth(width);
    button->SetFixedHeight(30);
    button->SetPosition(position);

    auto* buttonText = button->CreateChild<Text>();
    buttonText->SetFont(font, 12);
    buttonText->SetAlignment(HA_CENTER, VA_CENTER);
    buttonText->SetText(text);

    return button;
}
