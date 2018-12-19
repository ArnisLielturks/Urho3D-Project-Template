#include <Urho3D/Urho3DAll.h>
#include "AchievementsWindow.h"
#include "../../MyEvents.h"
#include "../../Audio/AudioManagerDefs.h"
#include "../../Global.h"
#include "../../Messages/Achievements.h"

/// Construct.
AchievementsWindow::AchievementsWindow(Context* context) :
    BaseWindow(context)
{
    Init();
}

AchievementsWindow::~AchievementsWindow()
{
    if (_activeLine) {
        _activeLine->Remove();
        _activeLine.Reset();
    }
    _baseWindow->Remove();
}

void AchievementsWindow::Init()
{
    Create();

    SubscribeToEvents();
}

void AchievementsWindow::Create()
{
    if (_baseWindow) {
        if (_activeLine) {
            _activeLine->Remove();
            _activeLine.Reset();
        }
        _baseWindow->Remove();
    }
    Input* input = GetSubsystem<Input>();
    if (!input->IsMouseVisible()) {
        input->SetMouseVisible(true);
    }

    _baseWindow = GetSubsystem<UI>()->GetRoot()->CreateChild<Window>();
    _baseWindow->SetStyleAuto();
    _baseWindow->SetAlignment(HA_CENTER, VA_CENTER);
    _baseWindow->SetSize(500, 400);
    _baseWindow->BringToFront();

    // Create Window 'titlebar' container
    _titleBar =_baseWindow->CreateChild<UIElement>();
    _titleBar->SetFixedSize(_baseWindow->GetWidth(), 24);
    _titleBar->SetVerticalAlignment(VA_TOP);
    _titleBar->SetLayoutMode(LM_HORIZONTAL);
    _titleBar->SetLayoutBorder(IntRect(4, 4, 4, 4));

    auto* cache = GetSubsystem<ResourceCache>();
    auto* font = cache->GetResource<Font>(APPLICATION_FONT);

    // Create the Window title Text
    auto* windowTitle = new Text(context_);
    windowTitle->SetName("WindowTitle");
    windowTitle->SetText("Achievements");
    windowTitle->SetFont(font, 14);


    // Create the Window's close button
    auto* buttonClose = new Button(context_);
    buttonClose->SetName("CloseButton");
    buttonClose->SetHorizontalAlignment(HA_RIGHT);

    // Add the controls to the title bar
    _titleBar->AddChild(windowTitle);
    _titleBar->AddChild(buttonClose);

    // Apply styles
    windowTitle->SetStyleAuto();
    buttonClose->SetStyle("CloseButton");

    SubscribeToEvent(buttonClose, E_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        VariantMap& data = GetEventDataMap();
        data["Name"] = "AchievementsWindow";
        SendEvent(MyEvents::E_CLOSE_WINDOW, data);
    });

    auto achievements = GetSubsystem<Achievements>()->GetAchievements();
    for (auto it = achievements.Begin(); it != achievements.End(); ++it) {
        CreateItem((*it).image, (*it).message, (*it).completed);
    }

    SubscribeToEvent(MyEvents::E_ACHIEVEMENT_UNLOCKED, URHO3D_HANDLER(AchievementsWindow, HandleAchievementUnlocked));

}

void AchievementsWindow::SubscribeToEvents()
{
}

UIElement* AchievementsWindow::CreateSingleLine()
{
    int top = 30;
    if (_activeLine) {
        top = _activeLine->GetPosition().y_ + _activeLine->GetHeight() + 10;
    }

    SharedPtr<UIElement> container(new UIElement(context_));
    container->SetAlignment(HA_LEFT, VA_TOP);
    container->SetLayout(LM_HORIZONTAL, 20);
    container->SetPosition(10, top);
    container->SetFixedWidth(_baseWindow->GetWidth());
    _baseWindow->AddChild(container);

    _activeLine = container;

    return container;
}

Button* AchievementsWindow::CreateItem(const String& image, const String& message, bool completed)
{
    if (!_activeLine) {
        CreateSingleLine();
    }

    if (_activeLine->GetChildren().Size() > 4) {
        CreateSingleLine();
    }

    auto* cache = GetSubsystem<ResourceCache>();

    auto sprite = _activeLine->CreateChild<Sprite>();
    // enable tooltip functionality
    sprite->SetEnabled(true);
    sprite->SetFixedHeight(_activeLine->GetWidth() / 4 - 20);
    sprite->SetFixedWidth(_activeLine->GetWidth() / 4 - 20);
    sprite->SetTexture(cache->GetResource<Texture2D>(image));

    // Add a tooltip to Fish button
    auto* toolTip = new ToolTip(context_);
    sprite->AddChild(toolTip);
    toolTip->SetPosition(IntVector2(sprite->GetWidth() / 2, sprite->GetWidth() / 2)); // slightly offset from close button
    auto* textHolder = new BorderImage(context_);
    toolTip->AddChild(textHolder);
    textHolder->SetStyle("ToolTipBorderImage");
    auto* toolTipText = new Text(context_);
    textHolder->AddChild(toolTipText);
    toolTipText->SetStyle("ToolTipText");
    toolTipText->SetText(message);

    if (!completed) {
        sprite->SetColor(Color::GRAY);
    }
}

void AchievementsWindow::HandleAchievementUnlocked(StringHash eventType, VariantMap& eventData)
{
    // Redraw screen
    Create();
}