#include <Urho3D/Input/Input.h>
#include <Urho3D/Resource/Localization.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Font.h>
#include "PauseWindow.h"
#include "../../Global.h"
#include "../../LevelManagerEvents.h"
#include "../WindowEvents.h"

using namespace LevelManagerEvents;
using namespace WindowEvents;

PauseWindow::PauseWindow(Context* context) :
    BaseWindow(context)
{
}

PauseWindow::~PauseWindow()
{
    _continueButton->Remove();
    _mainMenuButton->Remove();
    _exitButton->Remove();
    _baseWindow->Remove();
}

void PauseWindow::Init()
{
    Create();

    SubscribeToEvents();
}

void PauseWindow::Create()
{
    Input* input = GetSubsystem<Input>();
    if (!input->IsMouseVisible()) {
        input->SetMouseVisible(true);
    }

    auto* localization = GetSubsystem<Localization>();

    _baseWindow = CreateOverlay()->CreateChild<Window>();
    _baseWindow->SetStyleAuto();
    _baseWindow->SetAlignment(HA_CENTER, VA_CENTER);
    _baseWindow->SetSize(300, 190);
    _baseWindow->BringToFront();

    _continueButton = CreateButton(localization->Get("CONTINUE"), 200, IntVector2(0, 20));
    _continueButton->SetAlignment(HA_CENTER, VA_TOP);

    SubscribeToEvent(_continueButton, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        SendEvent(E_CLOSE_ALL_WINDOWS);
    });

    _mainMenuButton = CreateButton(localization->Get("RETURN_TO_MENU"), 200, IntVector2(0, 60));
    _mainMenuButton->SetAlignment(HA_CENTER, VA_TOP);

    SubscribeToEvent(_mainMenuButton, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "PauseWindow";
        SendEvent(E_CLOSE_WINDOW, data);

        data["Name"] = "MainMenu";
        SendEvent(E_SET_LEVEL, data);
    });

    _settingsButton = CreateButton(localization->Get("SETTINGS"), 200, IntVector2(0, 100));
    _settingsButton->SetAlignment(HA_CENTER, VA_TOP);

    SubscribeToEvent(_settingsButton, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "SettingsWindow";
        SendEvent(E_OPEN_WINDOW, data);
    });

    _exitButton = CreateButton(localization->Get("EXIT_GAME"), 200, IntVector2(0, 140));
    _exitButton->SetAlignment(HA_CENTER, VA_TOP);

    SubscribeToEvent(_exitButton, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "QuitConfirmationWindow";
        SendEvent(E_OPEN_WINDOW, data);
    });

}

void PauseWindow::SubscribeToEvents()
{
}

Button* PauseWindow::CreateButton(const String& text, int width, IntVector2 position)
{
    auto* cache = GetSubsystem<ResourceCache>();
    auto* font = cache->GetResource<Font>(APPLICATION_FONT);

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
