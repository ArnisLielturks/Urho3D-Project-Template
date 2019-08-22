#include <Urho3D/Input/Input.h>
#include <Urho3D/Resource/Localization.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include "QuitConfirmationWindow.h"
#include "../../MyEvents.h"
#include "../../Audio/AudioManagerDefs.h"
#include "../../Global.h"

static const int BUTTON_WIDTH = 150;
static const int BUTTON_HEIGHT = 40;
static const int BUTTON_MARGIN = 30;

QuitConfirmationWindow::QuitConfirmationWindow(Context* context) :
    BaseWindow(context)
{
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

    auto* localization = GetSubsystem<Localization>();

    _baseWindow = CreateOverlay()->CreateChild<Window>();
    _baseWindow->SetStyleAuto();
    _baseWindow->SetAlignment(HA_CENTER, VA_CENTER);
    _baseWindow->SetSize(BUTTON_WIDTH * 2 + BUTTON_MARGIN * 3, BUTTON_HEIGHT + BUTTON_MARGIN * 2);
    _baseWindow->BringToFront();

    _yesButton = CreateButton(localization->Get("YES"), BUTTON_WIDTH, IntVector2(BUTTON_MARGIN, 0));
    _yesButton->SetAlignment(HA_LEFT, VA_CENTER);

    SubscribeToEvent(_yesButton, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        SendEvent(MyEvents::E_CLOSE_ALL_WINDOWS);

        VariantMap& data = GetEventDataMap();
        data["Name"] = "ExitGame";
        SendEvent(MyEvents::E_SET_LEVEL, data);
    });

    _noButton = CreateButton(localization->Get("NO"), BUTTON_WIDTH, IntVector2(-BUTTON_MARGIN, 0));
    _noButton->SetAlignment(HA_RIGHT, VA_CENTER);

    SubscribeToEvent(_noButton, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
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
    auto* font = cache->GetResource<Font>(APPLICATION_FONT);

    auto* button = _baseWindow->CreateChild<Button>();
    button->SetStyleAuto();
    button->SetFixedWidth(width);
    button->SetFixedHeight(BUTTON_HEIGHT);
    button->SetPosition(position);

    auto* buttonText = button->CreateChild<Text>();
    buttonText->SetFont(font, 16);
    buttonText->SetAlignment(HA_CENTER, VA_CENTER);
    buttonText->SetText(text);

    return button;
}
