#include <Urho3D/Urho3DAll.h>
#include "ExitGame.h"
#include "../MyEvents.h"

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
    CreateUI();
}

void ExitGame::SubscribeToEvents()
{
	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(ExitGame, HandleUpdate));
}

void ExitGame::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	if (_timer.GetMSec(false) > 1000) {
		GetSubsystem<Engine>()->Exit();
	}
}

void ExitGame::CreateUI()
{
    UI* ui = GetSubsystem<UI>();
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    
    Text* text = ui->GetRoot()->CreateChild<Text>();
    text->SetHorizontalAlignment(HA_CENTER);
    text->SetVerticalAlignment(VA_CENTER);
    text->SetStyleAuto();
    text->SetText("Exiting game...");
}