#include <Urho3D/Urho3DAll.h>
#include "Achievements.h"

/// Construct.
Achievements::Achievements(Context* context) :
	Object(context)
{
	Init();
}

Achievements::~Achievements()
{
}

void Achievements::Init()
{
	Create();
	LoadAchievementList();
	SubscribeToEvents();
}

void Achievements::Create()
{
	UI* ui = GetSubsystem<UI>();

	_baseElement = ui->GetRoot()->CreateChild("Menu");
	_baseElement->SetVerticalAlignment(VA_BOTTOM);
	_baseElement->SetHorizontalAlignment(HA_LEFT);
	_baseElement->SetPosition(IntVector2(10, -10));
}

void Achievements::SubscribeToEvents()
{
	SubscribeToEvent("NewAchievement", URHO3D_HANDLER(Achievements, HandleNewAchievement));
	SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(Achievements, HandleUpdate));
}

void Achievements::Dispose()
{
}

void Achievements::HandleNewAchievement(StringHash eventType, VariantMap& eventData)
{
	if (!_baseElement) {
		Create();
	}
	auto* cache = GetSubsystem<ResourceCache>();

	String message = eventData["Message"].GetString();
	URHO3D_LOGINFO("New achievement: " + message);
	// Construct new Text object
	SharedPtr<UIElement> achievementElement(_baseElement->CreateChild<UIElement>());

	File achievementLayout(context_, GetSubsystem<FileSystem>()->GetProgramDir() + "Data/UI/Achievement.xml", FILE_READ);
	achievementElement->LoadXML(achievementLayout);

	Text* messageElement = dynamic_cast<Text*>(achievementElement->GetChild("Text", true));
	// Set String to display
	messageElement->SetText(message);

	Sprite* image = dynamic_cast<Sprite*>(achievementElement->GetChild("Image", true));
	auto* img = cache->GetResource<Texture2D>("Textures/UrhoIcon.png");
	image->SetTexture(img);

	// Create light animation
	SharedPtr<ObjectAnimation> notificationAnimation(new ObjectAnimation(context_));

	// Create light position animation
	SharedPtr<ValueAnimation> positionAnimation(new ValueAnimation(context_));
	// Use spline interpolation method
	positionAnimation->SetInterpolationMethod(IM_SPLINE);
	// Set spline tension
	positionAnimation->SetSplineTension(0.7f);
	positionAnimation->SetKeyFrame(0.0f, IntVector2(-achievementElement->GetWidth() - 20, 0));
	positionAnimation->SetKeyFrame(1.0f, IntVector2(0, 0));
	positionAnimation->SetKeyFrame(5.0f, IntVector2(0, 0));
	positionAnimation->SetKeyFrame(6.0f, IntVector2(-achievementElement->GetWidth() - 20, 0));
	positionAnimation->SetKeyFrame(10.0f, IntVector2(-achievementElement->GetWidth() - 20, 0));
	notificationAnimation->AddAttributeAnimation("Position", positionAnimation);

	achievementElement->SetObjectAnimation(notificationAnimation);
	achievementElement->SetVar("Lifetime", 10.0f);

	_messages.Push(achievementElement);
}

void Achievements::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	using namespace Update;

	float timeStep = eventData[P_TIMESTEP].GetFloat();
	for (auto it = _messages.Begin(); it != _messages.End(); ++it) {
		if ((*it).Refs() == 0) {
			_messages.Remove((*it));
			return;
		}
		float lifetime = (*it)->GetVar("Lifetime").GetFloat();
		if (lifetime <= 0) {
			(*it)->Remove();
			_messages.Remove((*it));
			return; 
		}
		lifetime -= timeStep;
		(*it)->SetVar("Lifetime", lifetime);
	}
}

void Achievements::HandleGameEnd(StringHash eventType, VariantMap& eventData)
{
	_messages.Clear();
}

void Achievements::LoadAchievementList()
{
	JSONFile configFile(context_);
	configFile.LoadFile(GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Config/achievements.json");
	JSONValue value = configFile.GetRoot();
	if (value.IsArray()) {
		URHO3D_LOGINFOF("Achievements: %u", value.Size());
		for (int i = 0; i < value.Size(); i++) {
			JSONValue mapInfo = value[i];
			if (mapInfo.Contains("Event")
				&& mapInfo["Event"].IsString()
				&& mapInfo.Contains("Image")
				&& mapInfo["Image"].IsString()
				&& mapInfo.Contains("Text")
				&& mapInfo["Text"].IsString()
				&& mapInfo.Contains("Threshold")
				&& mapInfo["Threshold"].IsString()
				&& mapInfo.Contains("Type")
				&& mapInfo["Type"].IsString()) {

				String evt = mapInfo["Event"].GetString();
				String img = mapInfo["Image"].GetString();
				String txt = mapInfo["Text"].GetString();
				String type = mapInfo["Type"].GetString();
				int threshold = mapInfo["Threshold"].GetInt();
				URHO3D_LOGINFO("Achievement: '" + txt + "'");
				URHO3D_LOGINFO("Image: " + img);
				URHO3D_LOGINFO("Evt: " + evt);
				URHO3D_LOGINFO("Type: " + type);
				URHO3D_LOGINFOF("Threshold: %i", threshold);

			}
			else {
				URHO3D_LOGINFO("Map array element doesnt contain all needed info!");
			}
		}
	}
	else {
		URHO3D_LOGINFO("Map config json is not an array!");
	}
}