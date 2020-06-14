#include <Urho3D/UI/UI.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Font.h>
#include "ScoreboardWindow.h"
#include "../../Global.h"
#include "../../Levels/Player/PlayerEvents.h"

ScoreboardWindow::ScoreboardWindow(Context* context) :
    BaseWindow(context)
{
}

ScoreboardWindow::~ScoreboardWindow()
{
    baseWindow_->Remove();
}

void ScoreboardWindow::Init()
{
    Create();

    SubscribeToEvents();
}

void ScoreboardWindow::Create()
{
    baseWindow_ = GetSubsystem<UI>()->GetRoot()->CreateChild<Window>();
    baseWindow_->SetStyleAuto();
    baseWindow_->SetAlignment(HA_CENTER, VA_CENTER);
    baseWindow_->SetSize(300, 300);
    baseWindow_->BringToFront();
    baseWindow_->SetLayout(LayoutMode::LM_VERTICAL, 20, IntRect(20, 20, 20, 20));

    CreatePlayerScores();
    //URHO3D_LOGINFO("Player scores " + String(GetGlobalVar("PlayerScores").Get));

}

void ScoreboardWindow::SubscribeToEvents()
{
    SubscribeToEvent(PlayerEvents::E_PLAYER_SCORES_UPDATED, URHO3D_HANDLER(ScoreboardWindow, HandleScoresUpdated));
}

void ScoreboardWindow::HandleScoresUpdated(StringHash eventType, VariantMap& eventData)
{
    CreatePlayerScores();
}

void ScoreboardWindow::CreatePlayerScores()
{
    baseWindow_->RemoveAllChildren();

    {
        auto container = baseWindow_->CreateChild<UIElement>();
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

    VariantMap players = GetGlobalVar("Players").GetVariantMap();
    for (auto it = players.Begin(); it != players.End(); ++it) {
        VariantMap playerData = (*it).second_.GetVariantMap();
        auto container = baseWindow_->CreateChild<UIElement>();
        container->SetAlignment(HA_LEFT, VA_TOP);
        container->SetLayout(LM_HORIZONTAL, 20);

        auto *cache = GetSubsystem<ResourceCache>();
        auto* font = cache->GetResource<Font>(APPLICATION_FONT);

        // Create log element to view latest logs from the system
        auto name = container->CreateChild<Text>();
        name->SetFont(font, 14);
        name->SetText(playerData["Name"].GetString());
        name->SetFixedWidth(200);
        name->SetColor(Color::GREEN);
        name->SetTextEffect(TextEffect::TE_SHADOW);

        // Create log element to view latest logs from the system
        auto score = container->CreateChild<Text>();
        score->SetFont(font, 14);
        score->SetText(String(playerData["Score"].GetInt()));
        score->SetFixedWidth(100);
        score->SetColor(Color::GREEN);
        score->SetTextEffect(TextEffect::TE_SHADOW);
    }
}
