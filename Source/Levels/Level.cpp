#include <Urho3D/Urho3DAll.h>
#include "Level.h"
#include "../MyEvents.h"

using namespace Levels;

	/// Construct.
Level::Level(Context* context) :
	BaseLevel(context),
	shouldReturn(false)
{
}

Level::~Level()
{
}

void Level::Init()
{
	Renderer* renderer = GetSubsystem<Renderer>();
	renderer->SetNumViewports(1);

	SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(Level, HandlePostUpdate));

	Network* network = GetSubsystem<Network>();
	network->RegisterRemoteEvent("SendPlayerNodeID");

	URHO3D_LOGRAW("Starting level: Level");
	BaseLevel::Init();

	// Create the scene content
	CreateScene();

	// Create the UI content
	CreateUI();

	// Subscribe to global events for camera movement
	SubscribeToEvents();

	Input* input = GetSubsystem<Input>();
	input->SetMouseVisible(false);
}



void Level::CreateScene()
{
	scene_ = new Scene(context_);
	scene_->CreateComponent<Octree>(LOCAL);
	scene_->CreateComponent<PhysicsWorld>(LOCAL);
	scene_->CreateComponent<DebugRenderer>(LOCAL);
	File loadFile(context_, GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Scenes/Scene.xml", FILE_READ);
	scene_->LoadXML(loadFile);

	CreateCamera();
}

void Level::CreateUI()
{
	UI* ui = GetSubsystem<UI>();
	ResourceCache* cache = GetSubsystem<ResourceCache>();

	Text* text = ui->GetRoot()->CreateChild<Text>();
	text->SetHorizontalAlignment(HA_CENTER);
	text->SetVerticalAlignment(VA_CENTER);
	text->SetStyleAuto();
	text->SetText("This is ingame text! Press ESC to exit game...");
}

void Level::SubscribeToEvents()
{
}

void Level::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{
	float timeStep = eventData["TimeStep"].GetFloat();
	cameraNode_->Yaw(timeStep * 50);
	Input* input = GetSubsystem<Input>();
	if (input->IsMouseVisible()) {
		input->SetMouseVisible(false);
	}
	if (_controlledNode) {
		Vector3 position = _controlledNode->GetWorldPosition();
		position.y_ += 3.5;
		cameraNode_->SetPosition(position);
	}
	if (input->GetKeyDown(KEY_ESCAPE)) {
		VariantMap eventData;
		eventData[MyEvents::E_SET_LEVEL] = "ExitGame";
		eventData["Message"] = "";
		SendEvent(MyEvents::E_SET_LEVEL, eventData);
		UnsubscribeFromEvent(E_POSTUPDATE);
	}
}

void Level::OnLoaded()
{
	if (shouldReturn) {
		VariantMap eventData;
		eventData[MyEvents::E_SET_LEVEL] = "MainMenu";
		eventData["Message"] = returnMessage;
		SendEvent(MyEvents::E_SET_LEVEL, eventData);
	}
}