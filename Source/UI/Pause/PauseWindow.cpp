#include <Urho3D/Input/Input.h>
#include <Urho3D/Resource/Localization.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/Core/CoreEvents.h>
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
    continueButton_->Remove();
    mainMenuButton_->Remove();
    exitButton_->Remove();
    baseWindow_->Remove();
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

    baseWindow_ = CreateOverlay()->CreateChild<Window>();
    baseWindow_->SetStyleAuto();
    baseWindow_->SetAlignment(HA_CENTER, VA_CENTER);
    baseWindow_->SetSize(300, 190);
    baseWindow_->BringToFront();

    continueButton_ = CreateButton(localization->Get("CONTINUE"), 200, IntVector2(0, 20));
    continueButton_->SetAlignment(HA_CENTER, VA_TOP);

    SubscribeToEvent(continueButton_, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        SendEvent(E_CLOSE_ALL_WINDOWS);
    });

    mainMenuButton_ = CreateButton(localization->Get("RETURN_TO_MENU"), 200, IntVector2(0, 60));
    mainMenuButton_->SetAlignment(HA_CENTER, VA_TOP);

    SubscribeToEvent(mainMenuButton_, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "PauseWindow";
        SendEvent(E_CLOSE_WINDOW, data);

        data["Name"] = "MainMenu";
        SendEvent(E_SET_LEVEL, data);
    });

    settingsButton_ = CreateButton(localization->Get("SETTINGS"), 200, IntVector2(0, 100));
    settingsButton_->SetAlignment(HA_CENTER, VA_TOP);

    SubscribeToEvent(settingsButton_, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "SettingsWindow";
        SendEvent(E_OPEN_WINDOW, data);
    });

    exitButton_ = CreateButton(localization->Get("EXIT_GAME"), 200, IntVector2(0, 140));
    exitButton_->SetAlignment(HA_CENTER, VA_TOP);

    SubscribeToEvent(exitButton_, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "QuitConfirmationWindow";
        SendEvent(E_OPEN_WINDOW, data);
    });

}

void PauseWindow::SubscribeToEvents()
{
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(PauseWindow, HandleUpdate));
}

Button* PauseWindow::CreateButton(const String& text, int width, IntVector2 position)
{
    auto* cache = GetSubsystem<ResourceCache>();
    auto* font = cache->GetResource<Font>(APPLICATION_FONT);

    auto* button = baseWindow_->CreateChild<Button>();
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

void PauseWindow::HandleUpdate(StringHash eventType, VariantMap& eventData) {
    Input *input = GetSubsystem<Input>();
    if (!input->IsMouseVisible()) {
        input->SetMouseVisible(true);
    }
    if (input->GetKeyPress(KEY_BACKSPACE)) {
        UnsubscribeFromEvent(E_UPDATE);
        SendEvent(E_CLOSE_ALL_WINDOWS);
    }
}
