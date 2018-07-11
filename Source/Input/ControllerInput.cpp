#include <Urho3D/Urho3DAll.h>
#include "ControllerInput.h"
#include "../MyEvents.h"
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
	_controlMapNames[CTRL_SPRINT] = "CTRL_SPRINT";

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
	_configFile = cache->GetResource<ConfigFile>("Config/controls.cfg");

	CreateConfigMaps();
}

void ControllerInput::CreateConfigMaps()
{
	_mappedControlsToKeys.Clear();
	_mappedKeysToControls.Clear();

	_mappedControlsToKeys[CTRL_FORWARD] = _configFile->GetInt("controls", "CTRL_FORWARD", KEY_W);
	_mappedControlsToKeys[CTRL_BACK] = _configFile->GetInt("controls", "CTRL_BACK", KEY_S);
	_mappedControlsToKeys[CTRL_LEFT] = _configFile->GetInt("controls", "CTRL_LEFT", KEY_A);
	_mappedControlsToKeys[CTRL_RIGHT] = _configFile->GetInt("controls", "CTRL_RIGHT", KEY_D);
	_mappedControlsToKeys[CTRL_JUMP] = _configFile->GetInt("controls", "CTRL_JUMP", KEY_SPACE);
	_mappedControlsToKeys[CTRL_ACTION] = _configFile->GetInt("controls", "CTRL_ACTION", KEY_E);
	_mappedControlsToKeys[CTRL_SPRINT] = _configFile->GetInt("controls", "CTRL_SPRINT", KEY_LSHIFT);

	for (auto it = _mappedControlsToKeys.Begin(); it != _mappedControlsToKeys.End(); ++it) {
		_mappedKeysToControls[(*it).second_] = (*it).first_;
	}
}

void ControllerInput::SaveConfig()
{

	for (auto it = _mappedControlsToKeys.Begin(); it != _mappedControlsToKeys.End(); ++it) {
		_configFile->Set("controls", _controlMapNames[(*it).first_], String((*it).second_));
	}

	Urho3D::File file(context_, GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Config/controls.cfg", Urho3D::FILE_WRITE);
	_configFile->Save(file, true);
	file.Close();
}

void ControllerInput::SubscribeToEvents()
{
	SubscribeToEvent(MyEvents::E_START_INPUT_MAPPING, URHO3D_HANDLER(ControllerInput, HandleStartInputListening));
	SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(ControllerInput, HandleKeyDown));
	SubscribeToEvent(E_KEYUP, URHO3D_HANDLER(ControllerInput, HandleKeyUp));
	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(ControllerInput, HandleUpdate));
	SubscribeToEvent("StartInputMappingConsole", URHO3D_HANDLER(ControllerInput, HandleStartInputListeningConsole));
	RegisterConsoleCommands();
}

void ControllerInput::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
	using namespace KeyDown;
	int key = eventData[P_KEY].GetInt();

	if (_activeAction > 0 && _timer.GetMSec(false) > 100) {
		int oldKey = _mappedControlsToKeys[_activeAction];
		_mappedControlsToKeys[_activeAction] = key;

		URHO3D_LOGINFO("Changed control '" + _controlMapNames[_activeAction] + "' from key " + String(oldKey) + " to key " + String(key));

		SaveConfig();

		_activeAction = 0;

		CreateConfigMaps();
		return;
	}

	auto* input = GetSubsystem<Input>();
	_controls.Set(_mappedKeysToControls[key], true);
}

void ControllerInput::HandleKeyUp(StringHash eventType, VariantMap& eventData)
{
	using namespace KeyUp;
	int key = eventData[P_KEY].GetInt();

	if (_activeAction > 0) {
		return;
	}

	auto* input = GetSubsystem<Input>();
	_controls.Set(_mappedKeysToControls[key], false);
}

void ControllerInput::HandleStartInputListening(StringHash eventType, VariantMap& eventData)
{
	URHO3D_LOGINFO("Starting input listener!");
	using namespace MyEvents::StartInputMapping;
	if (eventData[P_CONTROL_ACTION].GetType() == VAR_INT) {
		_activeAction = eventData[P_CONTROL_ACTION].GetInt();
		URHO3D_LOGINFO("Control: " + _mappedControlsToKeys[_activeAction]);
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
	auto* input = GetSubsystem<Input>();
    // Mouse sensitivity as degrees per pixel
    const float MOUSE_SENSITIVITY = 0.1f;

    // Use this frame's mouse motion to adjust camera node yaw and pitch. Clamp the pitch between -90 and 90 degrees
    IntVector2 mouseMove = input->GetMouseMove();
    _controls.yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
    _controls.pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
    _controls.pitch_ = Clamp(_controls.pitch_, -90.0f, 90.0f);
}