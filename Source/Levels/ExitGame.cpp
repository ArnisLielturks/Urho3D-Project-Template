#include <Urho3D/Urho3DAll.h>
#include "ExitGame.h"
#include "../MyEvents.h"
#include "../Messages/Achievements.h"
#include "../Global.h"

using namespace Levels;

    /// Construct.
ExitGame::ExitGame(Context* context) :
    BaseLevel(context)
{
    _timer.Reset();
    SubscribeToEvents();
}

ExitGame::~ExitGame()
{
}

void ExitGame::Init()
{
    // Disable achievement showing for this level
    GetSubsystem<Achievements>()->SetShowAchievements(false);

    CreateUI();
}

void ExitGame::SubscribeToEvents()
{
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(ExitGame, HandleUpdate));
}

void ExitGame::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    if (_timer.GetMSec(false) > 3000) {
        GetSubsystem<Engine>()->Exit();
    }
}

void ExitGame::CreateUI()
{
    auto* localization = GetSubsystem<Localization>();
    UI* ui = GetSubsystem<UI>();
    auto *cache = GetSubsystem<ResourceCache>();
    auto *font = cache->GetResource<Font>(APPLICATION_FONT);
    
    Text* text = ui->GetRoot()->CreateChild<Text>();
    text->SetHorizontalAlignment(HA_CENTER);
    text->SetVerticalAlignment(VA_CENTER);
    text->SetStyleAuto();
    text->SetFont(font, 16);
    text->SetText(localization->Get("EXITING_GAME"));
}