#include <Urho3D/Resource/Localization.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/Scene/ObjectAnimation.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include "PopupMessageWindow.h"
#include "../WindowEvents.h"
#include "../../Globals/GUIDefines.h"

using namespace WindowEvents;

PopupMessageWindow::PopupMessageWindow(Context* context) :
    BaseWindow(context)
{
}

PopupMessageWindow::~PopupMessageWindow()
{
    okButton_->Remove();
    baseWindow_->Remove();
}

void PopupMessageWindow::Init()
{
    Create();

    SubscribeToEvents();
}

void PopupMessageWindow::Create()
{
    auto* localization = GetSubsystem<Localization>();

    baseWindow_ = CreateOverlay()->CreateChild<Window>();
    baseWindow_->SetStyleAuto();
    baseWindow_->SetAlignment(HA_CENTER, VA_CENTER);
    baseWindow_->SetLayoutMode(LM_VERTICAL);
    baseWindow_->SetLayoutSpacing(10);
    baseWindow_->SetLayoutBorder(IntRect(10, 10, 10, 10));
    baseWindow_->SetWidth(300);
    baseWindow_->BringToFront();
    baseWindow_->GetParent()->SetPriority(baseWindow_->GetParent()->GetPriority() + 1);

    Color color = Color::GREEN;
    if (data_.Contains("Type")) {
        if (data_["Type"].GetString() == "warning") {
            color = Color::YELLOW;
        } else if (data_["Type"].GetString() == "error") {
            color = Color::RED;
        }
    }

    SharedPtr<ObjectAnimation> animation(new ObjectAnimation(context_));
    SharedPtr<ValueAnimation> colorAnimation(new ValueAnimation(context_));
    colorAnimation->SetInterpolationMethod(IM_LINEAR);
    colorAnimation->SetKeyFrame(0.0f, Color::WHITE);
    colorAnimation->SetKeyFrame(0.5f, color);
    colorAnimation->SetKeyFrame(1.0f, Color::WHITE);
    colorAnimation->SetInterpolationMethod(InterpMethod::IM_LINEAR);
    animation->AddAttributeAnimation("Color", colorAnimation);

    baseWindow_->SetObjectAnimation(animation);

    auto title = CreateLabel(data_["Title"].GetString(), 16);

    auto message = CreateLabel(data_["Message"].GetString(), 12);

    okButton_ = CreateButton(localization->Get("OK"), 80, IntVector2(20, 0));

    SubscribeToEvent(okButton_, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "PopupMessageWindow";
        SendEvent(E_CLOSE_WINDOW, data);

//        String type;
//        if (data_["Type"].GetString() == "info") {
//            type = "warning";
//        } else if (data_["Type"].GetString() == "warning") {
//            type = "error";
//        } else if (data_["Type"].GetString() == "error") {
//            type = "info";
//        }
//        data["Title"] = type;
//        data["Message"] = "Testing out " + type;
//        data["Name"] = "PopupMessageWindow";
//        data["Type"] = type;
//        data["ClosePrevious"] = true;
//        SendEvent(MyEvents::E_OPEN_WINDOW, data);
    });
}

void PopupMessageWindow::SubscribeToEvents()
{
}


Button* PopupMessageWindow::CreateButton(const String& text, int width, IntVector2 position)
{
    auto* cache = GetSubsystem<ResourceCache>();
    auto* font = cache->GetResource<Font>(APPLICATION_FONT);

    auto* uiElement = baseWindow_->CreateChild<UIElement>();
    uiElement->SetFixedHeight(30);

    auto* button = uiElement->CreateChild<Button>();
    button->SetStyleAuto();
    button->SetFixedWidth(width);
    button->SetFixedHeight(30);
    button->SetHorizontalAlignment(HA_CENTER);

    auto* buttonText = button->CreateChild<Text>();
    buttonText->SetFont(font, 12);
    buttonText->SetAlignment(HA_CENTER, VA_CENTER);
    buttonText->SetText(text);

    return button;
}

Text* PopupMessageWindow::CreateLabel(const String& text, int fontSize)
{
    auto *cache = GetSubsystem<ResourceCache>();
    auto* font = cache->GetResource<Font>(APPLICATION_FONT);

    auto* uiElement = baseWindow_->CreateChild<UIElement>();
    uiElement->SetMinHeight(30);

    // Create log element to view latest logs from the system
    auto *label = uiElement->CreateChild<Text>();
    label->SetFont(font, fontSize);
    label->SetWordwrap(true);
    label->SetWidth(uiElement->GetWidth());
    label->SetText(text);
    label->SetHorizontalAlignment(HA_CENTER);
    label->SetTextEffect(TE_SHADOW);
    label->SetTextAlignment(HA_CENTER);

    uiElement->SetFixedHeight(label->GetHeight());

    return label;
}
