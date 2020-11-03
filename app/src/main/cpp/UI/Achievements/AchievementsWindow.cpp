#include <Urho3D/Input/Input.h>
#include <Urho3D/Resource/Localization.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/UI/ToolTip.h>
#include <Urho3D/UI/Font.h>
#include "AchievementsWindow.h"
#include "../../Messages/Achievements.h"
#include "../WindowEvents.h"
#include "../../Messages/MessageEvents.h"
#include "../../Globals/GUIDefines.h"

using namespace WindowEvents;
using namespace MessageEvents;

AchievementsWindow::AchievementsWindow(Context* context) :
    BaseWindow(context)
{
}

AchievementsWindow::~AchievementsWindow()
{
    if (activeLine_) {
        activeLine_->Remove();
        activeLine_.Reset();
    }
    baseWindow_->Remove();
}

void AchievementsWindow::Init()
{
    Create();

    SubscribeToEvents();
}

void AchievementsWindow::Create()
{
    if (baseWindow_) {
        if (activeLine_) {
            activeLine_->Remove();
            activeLine_.Reset();
        }
        baseWindow_->Remove();
    }
    Input* input = GetSubsystem<Input>();
    if (!input->IsMouseVisible()) {
        input->SetMouseVisible(true);
    }

    auto* localization = GetSubsystem<Localization>();

    baseWindow_ = CreateOverlay()->CreateChild<Window>();
    baseWindow_->SetStyleAuto();
    baseWindow_->SetAlignment(HA_CENTER, VA_CENTER);
    baseWindow_->SetSize(700, 600);
    baseWindow_->BringToFront();

    // Create Window 'titlebar' container
    titleBar_ =baseWindow_->CreateChild<UIElement>();
    titleBar_->SetFixedSize(baseWindow_->GetWidth(), 32);
    titleBar_->SetVerticalAlignment(VA_TOP);
    titleBar_->SetLayoutMode(LM_HORIZONTAL);
    titleBar_->SetLayoutBorder(IntRect(4, 4, 4, 4));

    auto* cache = GetSubsystem<ResourceCache>();
    auto* font = cache->GetResource<Font>(APPLICATION_FONT);

    // Create the Window title Text
    auto* windowTitle = new Text(context_);
    windowTitle->SetName("WindowTitle");
    windowTitle->SetText(localization->Get("ACHIEVEMENTS"));
    windowTitle->SetFont(font, 14);


    // Create the Window's close button
    auto* buttonClose = new Button(context_);
    buttonClose->SetName("CloseButton");
    buttonClose->SetHorizontalAlignment(HA_RIGHT);

    // Add the controls to the title bar
    titleBar_->AddChild(windowTitle);
    titleBar_->AddChild(buttonClose);

    // Apply styles
    windowTitle->SetStyleAuto();
    buttonClose->SetStyle("CloseButton");

    SubscribeToEvent(buttonClose, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "AchievementsWindow";
        SendEvent(E_CLOSE_WINDOW, data);
    });

    listView_ = baseWindow_->CreateChild<ListView>();
    listView_->SetFixedWidth(baseWindow_->GetWidth() - 20);
    listView_->SetStyleAuto();
    listView_->SetMinHeight(baseWindow_->GetHeight() - 30 - 10);
    listView_->SetPosition(10, 30);
    //listView_->SetScrollBarsVisible(false, true);


    auto achievements = GetSubsystem<Achievements>()->GetAchievements();
    for (auto it = achievements.Begin(); it != achievements.End(); ++it) {
        CreateItem((*it).image, (*it).message, (*it).completed, (*it).current, (*it).threshold);
    }

    SubscribeToEvent(E_ACHIEVEMENT_UNLOCKED, URHO3D_HANDLER(AchievementsWindow, HandleAchievementUnlocked));
}

void AchievementsWindow::SubscribeToEvents()
{
}

UIElement* AchievementsWindow::CreateSingleLine()
{
    int top = 30;
    if (activeLine_) {
        top = activeLine_->GetPosition().y_ + activeLine_->GetHeight() + 10;
    }

    SharedPtr<UIElement> container(new UIElement(context_));
    container->SetAlignment(HA_LEFT, VA_TOP);
    container->SetLayout(LM_HORIZONTAL, 20);
    container->SetPosition(20, top);
    container->SetFixedWidth(listView_->GetWidth() - 20);
    container->SetMinHeight(20);
    listView_->AddItem(container);

    activeLine_ = container;

    return container;
}

Button* AchievementsWindow::CreateItem(const String& image, const String& message, bool completed, int progress, int threshold)
{
    CreateSingleLine();
    CreateSingleLine();

    auto* cache = GetSubsystem<ResourceCache>();

    auto sprite = activeLine_->CreateChild<Sprite>();
    // enable tooltip functionality
    sprite->SetEnabled(true);
    sprite->SetFixedHeight(activeLine_->GetWidth() / 4 - 20);
    sprite->SetFixedWidth(activeLine_->GetWidth() / 4 - 20);
    sprite->SetTexture(cache->GetResource<Texture2D>(image));

    auto* font = cache->GetResource<Font>(APPLICATION_FONT);

    int fontSize = 10;

    auto* achievementText = activeLine_->CreateChild<Text>();
    achievementText->SetStyle("ToolTipText");
    achievementText->SetFont(font, fontSize);
    achievementText->SetText(message);
    achievementText->SetPosition(sprite->GetWidth() + 10, sprite->GetHeight() / 2 - fontSize / 2.0);

    // Add a tooltip to Fish button
    auto* toolTip = new ToolTip(context_);
    activeLine_->AddChild(toolTip);
    IntVector2 position;// = activeLine_->GetScreenPosition();
    position.x_ += activeLine_->GetWidth() / 2;
    position.y_ += activeLine_->GetHeight() / 2;
    toolTip->SetPosition(position); // slightly offset from close button
    auto* textHolder = new BorderImage(context_);
    toolTip->AddChild(textHolder);
    textHolder->SetStyle("ToolTipBorderImage");
    auto* toolTipText = new Text(context_);
    textHolder->AddChild(toolTipText);
    toolTipText->SetStyle("ToolTipText");
    toolTipText->SetFont(font, fontSize);
    toolTipText->SetText("Progress: " + String(progress) + " / " + String(threshold));

    if (!completed) {
        sprite->SetColor(Color::GRAY);
    }

    return nullptr;
}

void AchievementsWindow::HandleAchievementUnlocked(StringHash eventType, VariantMap& eventData)
{
    // Redraw screen
    Create();
}
