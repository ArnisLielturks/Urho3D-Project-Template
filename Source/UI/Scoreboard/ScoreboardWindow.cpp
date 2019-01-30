#include <Urho3D/Urho3DAll.h>
#include "ScoreboardWindow.h"
#include "../../MyEvents.h"
#include "../../Global.h"

/// Construct.
ScoreboardWindow::ScoreboardWindow(Context* context) :
    BaseWindow(context)
{
    Init();
}

ScoreboardWindow::~ScoreboardWindow()
{
    _baseWindow->Remove();
}

void ScoreboardWindow::Init()
{
    Create();

    SubscribeToEvents();
}

void ScoreboardWindow::Create()
{
    _baseWindow = GetSubsystem<UI>()->GetRoot()->CreateChild<Window>();
    _baseWindow->SetStyleAuto();
    _baseWindow->SetAlignment(HA_CENTER, VA_CENTER);
    _baseWindow->SetSize(300, 300);
    _baseWindow->BringToFront();
    _baseWindow->SetLayout(LayoutMode::LM_VERTICAL, 20, IntRect(20, 20, 20, 20));

    CreatePlayerScores();
    //URHO3D_LOGINFO("Player scores " + String(GetGlobalVar("PlayerScores").Get));

}

void ScoreboardWindow::SubscribeToEvents()
{
    SubscribeToEvent(MyEvents::E_PLAYER_SCORES_UPDATED, URHO3D_HANDLER(ScoreboardWindow, HandleScoresUpdated));
}

void ScoreboardWindow::HandleScoresUpdated(StringHash eventType, VariantMap& eventData)
{
    CreatePlayerScores();
}

void ScoreboardWindow::CreatePlayerScores()
{
    _baseWindow->RemoveAllChildren();

    {
        auto container = _baseWindow->CreateChild<UIElement>();
        container->SetAlignment(HA_LEFT, VA_TOP);
        container->SetLayout(LM_HORIZONTAL, 20);

        auto *cache = GetSubsystem<ResourceCache>();
        auto* font = cache->GetResource<Font>(APPLICATION_FONT);

        // Create log element to view latest logs from the system
        auto name = container->CreateChild<Text>();
        name->SetFont(font, 16);
        name->SetText("Player");
        name->SetFixedWidth(200);
        name->SetColor(Color::GRAY);
        name->SetTextEffect(TextEffect::TE_SHADOW);

        // Create log element to view latest logs from the system
        auto score = container->CreateChild<Text>();
        score->SetFont(font, 16);
        score->SetText("Score");
        score->SetFixedWidth(100);
        score->SetColor(Color::GRAY);
        score->SetTextEffect(TextEffect::TE_SHADOW);
    }

    for (unsigned int i = 0; i < GetGlobalVar("Players").GetInt(); i++) {
        auto container = _baseWindow->CreateChild<UIElement>();
        container->SetAlignment(HA_LEFT, VA_TOP);
        container->SetLayout(LM_HORIZONTAL, 20);

        auto *cache = GetSubsystem<ResourceCache>();
        auto* font = cache->GetResource<Font>(APPLICATION_FONT);

        // Create log element to view latest logs from the system
        auto name = container->CreateChild<Text>();
        name->SetFont(font, 14);
        name->SetText("Player" + String(i));
        name->SetFixedWidth(200);
        name->SetColor(Color::GREEN);
        name->SetTextEffect(TextEffect::TE_SHADOW);

        // Create log element to view latest logs from the system
        auto score = container->CreateChild<Text>();
        score->SetFont(font, 14);
        score->SetText(String(GetGlobalVar("Player" + String(i) + "Score").GetInt()));
        score->SetFixedWidth(100);
        score->SetColor(Color::GREEN);
        score->SetTextEffect(TextEffect::TE_SHADOW);
    }
}