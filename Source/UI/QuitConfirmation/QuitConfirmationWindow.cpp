#include <Urho3D/Input/Input.h>
#include <Urho3D/Resource/Localization.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include "QuitConfirmationWindow.h"
#include "../../MyEvents.h"
#include "../../Global.h"

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
    _baseWindow->SetWidth(300);
    _baseWindow->SetMinHeight(50);
    _baseWindow->SetLayout(LM_VERTICAL, 10, IntRect(10, 10, 10, 10));
    _baseWindow->BringToFront();
    _baseWindow->GetParent()->SetPriority(_baseWindow->GetParent()->GetPriority() + 1000);

    SharedPtr<UIElement> titleContainer(_baseWindow->CreateChild<UIElement>());
    titleContainer->SetLayoutMode(LM_HORIZONTAL);
    auto title = titleContainer->CreateChild<Text>();
    title->SetStyleAuto();
    title->SetText(localization->Get("ARE_YOU_SURE"));
    title->SetTextAlignment(HA_CENTER);
    title->SetFontSize(24);

    SharedPtr<UIElement> buttonsContainer(_baseWindow->CreateChild<UIElement>());
    buttonsContainer->SetLayout(LM_HORIZONTAL, 10);

    _yesButton = CreateButton(localization->Get("YES"));
    SubscribeToEvent(_yesButton, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        SendEvent(MyEvents::E_CLOSE_ALL_WINDOWS);

        VariantMap& data = GetEventDataMap();
        data["Name"] = "ExitGame";
        SendEvent(MyEvents::E_SET_LEVEL, data);
    });

    _noButton = CreateButton(localization->Get("NO"));
    SubscribeToEvent(_noButton, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "QuitConfirmationWindow";
        SendEvent(MyEvents::E_CLOSE_WINDOW, data);
    });

    buttonsContainer->AddChild(_yesButton);
    buttonsContainer->AddChild(_noButton);
    _baseWindow->UpdateLayout();
}

void QuitConfirmationWindow::SubscribeToEvents()
{
}

Button* QuitConfirmationWindow::CreateButton(const String& text)
{
    auto* cache = GetSubsystem<ResourceCache>();
    auto* font = cache->GetResource<Font>(APPLICATION_FONT);

    auto* button = new Button(context_);
    button->SetStyleAuto();
    button->SetFixedWidth(150);
    button->SetFixedHeight(40);

    auto* buttonText = button->CreateChild<Text>();
    buttonText->SetFont(font, 16);
    buttonText->SetText(text);
    buttonText->SetAlignment(HA_CENTER, VA_CENTER);
    buttonText->SetTextAlignment(HA_CENTER);

    return button;
}
