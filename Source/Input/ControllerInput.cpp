#include <Urho3D/Urho3DAll.h>
#include "ControllerInput.h"
#include "../MyEvents.h"
#include "../Config/ConfigFile.h"
#include "../Global.h"

/// Construct.
ControllerInput::ControllerInput(Context* context) :
    Object(context)
{
	_controlMapNames[CTRL_FORWARD] = "CTRL_FORWARD";
	_controlMapNames[CTRL_BACK] = "CTRL_BACK";
	_controlMapNames[CTRL_LEFT] = "CTRL_LEFT";
	_controlMapNames[CTRL_RIGHT] = "CTRL_RIGHT";
	_controlMapNames[CTRL_JUMP] = "CTRL_JUMP";
	_controlMapNames[CTRL_ACTION] = "CTRL_ACTION";

	Init();
}

ControllerInput::~ControllerInput()
{
}

void ControllerInput::Init()
{
    // Subscribe to global events for camera movement
    SubscribeToEvents();

	LoadConfig();
}

void ControllerInput::LoadConfig()
{
	auto* cache = GetSubsystem<ResourceCache>();
	auto configFile = cache->GetResource<ConfigFile>("Config/controls.cfg");

	_mappedControls[CTRL_FORWARD] = configFile->GetInt("controls", "CTRL_FORWARD", KEY_W);
	_mappedControls[CTRL_BACK] = configFile->GetInt("controls", "CTRL_BACK", KEY_S);
	_mappedControls[CTRL_LEFT] = configFile->GetInt("controls", "CTRL_LEFT", KEY_A);
	_mappedControls[CTRL_RIGHT] = configFile->GetInt("controls", "CTRL_RIGHT", KEY_D);
	_mappedControls[CTRL_JUMP] = configFile->GetInt("controls", "CTRL_JUMP", KEY_SPACE);
	_mappedControls[CTRL_ACTION] = configFile->GetInt("controls", "CTRL_ACTION", KEY_E);

	SaveConfig();
}

void ControllerInput::SaveConfig()
{
	auto* cache = GetSubsystem<ResourceCache>();
	auto configFile = cache->GetResource<ConfigFile>("Config/controls.cfg");

	for (auto it = _mappedControls.Begin(); it != _mappedControls.End(); ++it) {
		configFile->Set("controls", _controlMapNames[(*it).first_], String((*it).second_));
	}
	Urho3D::File file(context_, GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Config/controls.cfg", Urho3D::FILE_WRITE);
	configFile->Save(file, true);
	file.Close();
}

void ControllerInput::SubscribeToEvents()
{
	SubscribeToEvent(MyEvents::E_START_INPUT_MAPPING, URHO3D_HANDLER(ControllerInput, HandleStartInputListening));
}

void ControllerInput::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
	using namespace KeyDown;
	int key = eventData[P_KEY].GetInt();
	_mappedControls[_activeAction] = key;
	_activeAction = 0;

	KEY_ESCAPE;
	UnsubscribeFromEvent(E_KEYDOWN);
}

void ControllerInput::HandleStartInputListening(StringHash eventType, VariantMap& eventData)
{
	using namespace MyEvents::StartInputMapping;
	_activeAction = eventData[P_CONTROL_ACTION].GetInt();

	SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(ControllerInput, HandleKeyDown));
}