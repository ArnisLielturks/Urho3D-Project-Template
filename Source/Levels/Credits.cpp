#include <Urho3D/Urho3DAll.h>
#include "Credits.h"
#include "../MyEvents.h"
#include "../Global.h"
#include "../Messages/Achievements.h"

using namespace Levels;

namespace Levels {
	/// Construct.
	Credits::Credits(Context* context) :
		BaseLevel(context),
		_totalCreditsHeight(0),
		_creditLengthInSeconds(0)
	{
	}

	Credits::~Credits()
	{
	}

	void Credits::Init()
	{
        // Disable achievement showing for this level
        GetSubsystem<Achievements>()->SetShowAchievements(false);

		BaseLevel::Init();

		// Create the scene content
		CreateScene();

		// Create the UI content
		CreateUI();
	}

	void Credits::CreateScene()
	{
		return;
	}

	void Credits::CreateUI()
	{
		_timer.Reset();
		UI* ui = GetSubsystem<UI>();
		ResourceCache* cache = GetSubsystem<ResourceCache>();

		_creditsBase = ui->GetRoot()->CreateChild<UIElement>();
		_creditsBase->SetAlignment(HA_CENTER, VA_BOTTOM);
		_creditsBase->SetStyleAuto();

		CreateSingleLine("Creator", 30);
		CreateSingleLine("Arnis Lielturks", 20);
		CreateSingleLine("", 20);
		CreateSingleLine("Community", 30);
		CreateSingleLine("INI file parser: @carnalis", 20);
		CreateSingleLine("Level manager: @artgolf1000", 20);
        CreateSingleLine("Icons: https://game-icons.net", 20);
        CreateSingleLine("Sounds and music: https://freesound.org", 20);
        CreateSingleLine("", 20);
		CreateSingleLine("Special thanks to the creators", 30);
        CreateSingleLine("of the Urho3D engine!", 30);

		_creditLengthInSeconds = _credits.Size() * 2;

		SharedPtr<ObjectAnimation> animation(new ObjectAnimation(context_));
		SharedPtr<ValueAnimation> colorAnimation(new ValueAnimation(context_));
		// Use spline interpolation method
		colorAnimation->SetInterpolationMethod(IM_SPLINE);
		// Set spline tension
		colorAnimation->SetSplineTension(0.7f);
		colorAnimation->SetKeyFrame(0.0f, IntVector2(0, 0));
		colorAnimation->SetKeyFrame(_creditLengthInSeconds, IntVector2(0, -GetSubsystem<Graphics>()->GetHeight() - _totalCreditsHeight - 50));
		colorAnimation->SetKeyFrame(_creditLengthInSeconds * 2, IntVector2(0, -GetSubsystem<Graphics>()->GetHeight() - _totalCreditsHeight - 50));
		animation->AddAttributeAnimation("Position", colorAnimation);

		_creditsBase->SetObjectAnimation(animation);

		SubscribeToEvents();
	}

	void Credits::SubscribeToEvents()
	{
		SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Credits, HandleUpdate));
	}

	void Credits::HandleUpdate(StringHash eventType, VariantMap& eventData)
	{
		Input* input = GetSubsystem<Input>();
		if (input->IsMouseVisible()) {
			input->SetMouseVisible(false);
		}
		if (input->GetKeyDown(KEY_ESCAPE)) {
			UnsubscribeFromEvent(E_UPDATE);
			HandleEndCredits();
		}
		if (_timer.GetMSec(false) > _creditLengthInSeconds * 1000) {
			UnsubscribeFromEvent(E_UPDATE);
			HandleEndCredits();
		}
	}

	void Credits::HandleEndCredits()
	{
		UnsubscribeFromEvent(E_UPDATE);
		VariantMap data = GetEventDataMap();
		data["Name"] = "MainMenu";
		SendEvent(MyEvents::E_SET_LEVEL, data);

		SendEvent("CreditsEnd");
	}

	void Credits::CreateSingleLine(String content, int fontSize)
	{
		_totalCreditsHeight += fontSize + 10;

		auto cache = GetSubsystem<ResourceCache>();
		auto* font = cache->GetResource<Font>(APPLICATION_FONT);

		SharedPtr<Text> text(_creditsBase->CreateChild<Text>());
		text->SetPosition(IntVector2(0, _totalCreditsHeight));
		text->SetAlignment(HA_CENTER, VA_TOP);
		text->SetStyleAuto();
		text->SetFont(font, fontSize);
		text->SetText(content);
		_credits.Push(text);
		_totalCreditsHeight += 20;
	}
}