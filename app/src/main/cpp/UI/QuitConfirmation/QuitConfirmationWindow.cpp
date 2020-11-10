#include <Urho3D/Input/Input.h>
#include <Urho3D/Resource/Localization.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include "QuitConfirmationWindow.h"
#include "../../LevelManagerEvents.h"
#include "../WindowEvents.h"
#include "../../Globals/GUIDefines.h"

using namespace LevelManagerEvents;
using namespace WindowEvents;

QuitConfirmationWindow::QuitConfirmationWindow(Context* context) :
    BaseWindow(context)
{
}

QuitConfirmationWindow::~QuitConfirmationWindow()
{
    yesButton_->Remove();
    noButton_->Remove();
    baseWindow_->Remove();
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

    baseWindow_ = CreateOverlay()->CreateChild<Window>();
    baseWindow_->SetStyleAuto();
    baseWindow_->SetAlignment(HA_CENTER, VA_CENTER);
    baseWindow_->SetWidth(300);
    baseWindow_->SetMinHeight(50);
    baseWindow_->SetLayout(LM_VERTICAL, 10, IntRect(10, 10, 10, 10));
    baseWindow_->BringToFront();
    baseWindow_->GetParent()->SetPriority(baseWindow_->GetParent()->GetPriority() + 1000);

    SharedPtr<UIElement> titleContainer(baseWindow_->CreateChild<UIElement>());
    titleContainer->SetLayoutMode(LM_HORIZONTAL);
    auto title = titleContainer->CreateChild<Text>();
    title->SetStyleAuto();
    title->SetText(localization->Get("ARE_YOU_SURE"));
    title->SetTextAlignment(HA_CENTER);
    title->SetFontSize(24);

    SharedPtr<UIElement> buttonsContainer(baseWindow_->CreateChild<UIElement>());
    buttonsContainer->SetLayout(LM_HORIZONTAL, 10);

    yesButton_ = CreateButton(localization->Get("YES"));
    SubscribeToEvent(yesButton_, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        SendEvent(E_CLOSE_ALL_WINDOWS);

        VariantMap& data = GetEventDataMap();
        data["Name"] = "ExitGame";
        SendEvent(E_SET_LEVEL, data);
    });

    noButton_ = CreateButton(localization->Get("NO"));
    SubscribeToEvent(noButton_, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "QuitConfirmationWindow";
        SendEvent(E_CLOSE_WINDOW, data);
    });

    buttonsContainer->AddChild(yesButton_);
    buttonsContainer->AddChild(noButton_);
    baseWindow_->UpdateLayout();
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
