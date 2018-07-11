#include <Urho3D/Urho3DAll.h>
#include "ControllerInput.h"
#include "../MyEvents.h"
#include "../Config/ConfigFile.h"
#include "../Global.h"

/// Construct.
ControllerInput::ControllerInput(Context* context) :
    Object(context),
	_activeAction(0)
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
	SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(ControllerInput, HandleKeyDown));
	SubscribeToEvent(E_KEYUP, URHO3D_HANDLER(ControllerInput, HandleKeyUp));
	SubscribeToEvent("StartInputMappingConsole", URHO3D_HANDLER(ControllerInput, HandleStartInputListeningConsole));
	RegisterConsoleCommands();
}

void ControllerInput::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
	using namespace KeyDown;
	int key = eventData[P_KEY].GetInt();

	if (_activeAction > 0 && _timer.GetMSec(false) > 100) {
		int oldKey = _mappedControls[_activeAction];
		_mappedControls[_activeAction] = key;

		URHO3D_LOGINFO("Changed control '" + _controlMapNames[_activeAction] + "' from key " + String(oldKey) + " to key " + String(key));

		SaveConfig();

		_activeAction = 0;
		return;
	}

	auto* input = GetSubsystem<Input>();
	_controls.Set(CTRL_FORWARD, input->GetKeyDown(_mappedControls[CTRL_FORWARD]));
	_controls.Set(CTRL_BACK, input->GetKeyDown(_mappedControls[CTRL_BACK]));
	_controls.Set(CTRL_LEFT, input->GetKeyDown(_mappedControls[CTRL_LEFT]));
	_controls.Set(CTRL_RIGHT, input->GetKeyDown(_mappedControls[CTRL_RIGHT]));
	_controls.Set(CTRL_JUMP, input->GetKeyDown(_mappedControls[CTRL_JUMP]));
	_controls.Set(CTRL_ACTION, input->GetMouseButtonDown(_mappedControls[CTRL_ACTION]));
}

void ControllerInput::HandleKeyUp(StringHash eventType, VariantMap& eventData)
{
	using namespace KeyUp;
	int key = eventData[P_KEY].GetInt();

	if (_activeAction > 0) {
		return;
	}

	auto* input = GetSubsystem<Input>();
	_controls.Set(CTRL_FORWARD, input->GetKeyDown(_mappedControls[CTRL_FORWARD]));
	_controls.Set(CTRL_BACK, input->GetKeyDown(_mappedControls[CTRL_BACK]));
	_controls.Set(CTRL_LEFT, input->GetKeyDown(_mappedControls[CTRL_LEFT]));
	_controls.Set(CTRL_RIGHT, input->GetKeyDown(_mappedControls[CTRL_RIGHT]));
	_controls.Set(CTRL_JUMP, input->GetKeyDown(_mappedControls[CTRL_JUMP]));
	_controls.Set(CTRL_ACTION, input->GetMouseButtonDown(_mappedControls[CTRL_ACTION]));
}

void ControllerInput::HandleStartInputListening(StringHash eventType, VariantMap& eventData)
{
	URHO3D_LOGINFO("Starting input listener!");
	using namespace MyEvents::StartInputMapping;
	if (eventData[P_CONTROL_ACTION].GetType() == VAR_INT) {
		_activeAction = eventData[P_CONTROL_ACTION].GetInt();
		URHO3D_LOGINFO("Control: " + _mappedControls[_activeAction]);
	}
	if (eventData[P_CONTROL_ACTION].GetType() == VAR_STRING) {
		String control = eventData[P_CONTROL_ACTION].GetString();
		for (auto it = _controlMapNames.Begin(); it != _controlMapNames.End(); ++it) {
			if ((*it).second_ == control) {
				_activeAction = (*it).first_;
				URHO3D_LOGINFO("Control: " + control);
			}
		}
	}

	_timer.Reset();
	// SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(ControllerInput, HandleUpdate));
}

void ControllerInput::RegisterConsoleCommands()
{
	VariantMap data = GetEventDataMap();
    data["ConsoleCommandName"] = "map_input";
    data["ConsoleCommandEvent"] = "StartInputMappingConsole";
    data["ConsoleCommandDescription"] = "Listening for keystroke";
    SendEvent("ConsoleCommandAdd", data);
}

void ControllerInput::HandleStartInputListeningConsole(StringHash eventType, VariantMap& eventData)
{
	using namespace MyEvents::StartInputMapping;
	StringVector parameters = eventData["Parameters"].GetStringVector();
	if (parameters.Size() == 2) {
		VariantMap data = GetEventDataMap();
		data[P_CONTROL_ACTION] = parameters[1];
		SendEvent(MyEvents::E_START_INPUT_MAPPING, data);
		return;
	}

	URHO3D_LOGERROR("Invalid number of parameters!");
}

Controls ControllerInput::GetControls()
{
	return _controls;
}

void ControllerInput::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	// if (_activeAction > 0) {
	// 	Input* input = GetSubsystem<Input>();

	// 	int oldKey = _mappedControls[_activeAction];
	// 	_mappedControls[_activeAction] = key;

	// 	URHO3D_LOGINFO("Changed control '" + _controlMapNames[_activeAction] + "' from key " + String(oldKey) + " to key " + String(key));

	// 	SaveConfig();

	// 	_activeAction = 0;
	// 	UnsubscribeFromEvent(E_UPDATE);
	// }
}