#include <Urho3D/Urho3DAll.h>
#include "ControllerInput.h"
#include "../MyEvents.h"
#include "../Global.h"
#include "Controllers/KeyboardInput.h"
#include "Controllers/MouseInput.h"
#include "Controllers/JoystickInput.h"

/// Construct.
ControllerInput::ControllerInput(Context* context) :
    Object(context)
{

	context_->RegisterFactory<BaseInput>();
	context_->RegisterFactory<KeyboardInput>();
	context_->RegisterFactory<MouseInput>();
	context_->RegisterFactory<JoystickInput>();

	_inputHandlers[ControllerType::KEYBOARD] = context_->CreateObject<KeyboardInput>();
	_inputHandlers[ControllerType::MOUSE] = context_->CreateObject<MouseInput>();
	_inputHandlers[ControllerType::JOYSTICK] = context_->CreateObject<JoystickInput>();

	_controlMapNames[CTRL_FORWARD] = "Move forward";
	_controlMapNames[CTRL_BACK] = "Move backward";
	_controlMapNames[CTRL_LEFT] = "Strafe left";
	_controlMapNames[CTRL_RIGHT] = "Strafe right";
	_controlMapNames[CTRL_JUMP] = "Jump";
	_controlMapNames[CTRL_ACTION] = "Primary action";
	_controlMapNames[CTRL_SPRINT] = "Sprint";
	_controlMapNames[CTRL_UP] = "Move up";

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

	for (auto it = _controlMapNames.Begin(); it != _controlMapNames.End(); ++it) {
		String controlName = (*it).second_;
		controlName.Replace(" ", "_");
		int controlCode = (*it).first_;
		if (_configFile->GetInt("keyboard", controlName, -1) != -1) {
			int key = _configFile->GetInt("keyboard", controlName, 0);
			_inputHandlers[ControllerType::KEYBOARD]->SetKeyToAction(key, controlCode);
		}
		if (_configFile->GetInt("mouse", controlName, -1) != -1) {
			int key = _configFile->GetInt("mouse", controlName, 0);
			_inputHandlers[ControllerType::MOUSE]->SetKeyToAction(key, controlCode);
		}
		if (_configFile->GetInt("joystick", controlName, -1) != -1) {
			int key = _configFile->GetInt("joystick", controlName, 0);
			_inputHandlers[ControllerType::JOYSTICK]->SetKeyToAction(key, controlCode);
		}
	}
}

void ControllerInput::SaveConfig()
{
	for (auto it = _controlMapNames.Begin(); it != _controlMapNames.End(); ++it) {
		String controlName = (*it).second_;
		controlName.Replace(" ", "_");
		int controlCode = (*it).first_;
		_configFile->Set("keyboard", controlName, "-1");
		_configFile->Set("mouse", controlName, "-1");
		_configFile->Set("joystick", controlName, "-1");
	}

	for (auto it = _inputHandlers.Begin(); it != _inputHandlers.End(); ++it) {
		HashMap<int, int> configMap = (*it).second_->GetConfigMap();
		int type = (*it).first_;
		String typeName;
		HashMap<int, String> map;
		map[ControllerType::KEYBOARD] = "keyboard";
		map[ControllerType::MOUSE] = "mouse";
		map[ControllerType::JOYSTICK] = "joystick";

		for (auto it2 = configMap.Begin(); it2 != configMap.End(); ++it2) {
			int controlCode = (*it2).first_;
		 	int keyCode = (*it2).second_;
			 if (_controlMapNames.Contains(controlCode) && !_controlMapNames[controlCode].Empty()) {
				String controlName = _controlMapNames[controlCode];
				controlName.Replace(" ", "_");
				String value = String(keyCode);
				_configFile->Set(map[type], controlName, value);
				URHO3D_LOGINFO(">>>>>>>> Setting " + map[type] + " : " + controlName + " => " + value);
			}
		}
	}

	File file(context_, GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Config/controls.cfg", Urho3D::FILE_WRITE);
	_configFile->Save(file, true);
	file.Close();
}

void ControllerInput::SubscribeToEvents()
{
	SubscribeToEvent(MyEvents::E_START_INPUT_MAPPING, URHO3D_HANDLER(ControllerInput, HandleStartInputListening));

	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(ControllerInput, HandleUpdate));
	SubscribeToEvent("StartInputMappingConsole", URHO3D_HANDLER(ControllerInput, HandleStartInputListeningConsole));
	RegisterConsoleCommands();
}

void ControllerInput::ReleaseConfiguredKey(int key, int action)
{
	// Clear all input handler mappings against key and actions
	for (auto it = _inputHandlers.Begin(); it != _inputHandlers.End(); ++it) {
		(*it).second_->ReleaseKey(key);
		(*it).second_->ReleaseAction(key);
	}
}

void ControllerInput::SetConfiguredKey(int action, int key, String controller)
{
	// Clear previously assigned key and/or action
	ReleaseConfiguredKey(key, action);
	auto* input = GetSubsystem<Input>();
	if (controller == "keyboard") {
		_inputHandlers[ControllerType::KEYBOARD]->SetKeyToAction(key, action);
	}
	if (controller == "mouse") {
		_inputHandlers[ControllerType::MOUSE]->SetKeyToAction(key, action);
	}
	if (controller == "joystick") {
		_inputHandlers[ControllerType::JOYSTICK]->SetKeyToAction(key, action);
	}

	// Stop listening for keyboard key mapping
	for (auto it = _inputHandlers.Begin(); it != _inputHandlers.End(); ++it) {
		(*it).second_->StopMappingAction();
	}

	// Send out event with all the details about the mapped control
	using namespace MyEvents::InputMappingFinished;
	VariantMap data = GetEventDataMap();
	data[P_CONTROLLER] = controller;
	data[P_CONTROL_ACTION] = action;
	data[P_ACTION_NAME] = _controlMapNames[action];
	data[P_KEY] = key;
	data[P_KEY_NAME] = input->GetKeyName(key);
	SendEvent(MyEvents::E_INPUT_MAPPING_FINISHED, data);

	SaveConfig();
}

void ControllerInput::HandleStartInputListening(StringHash eventType, VariantMap& eventData)
{
	int activeAction = 0;

	using namespace MyEvents::StartInputMapping;
	if (eventData[P_CONTROL_ACTION].GetType() == VAR_INT) {
		activeAction = eventData[P_CONTROL_ACTION].GetInt();
	}
	if (eventData[P_CONTROL_ACTION].GetType() == VAR_STRING) {
		String control = eventData[P_CONTROL_ACTION].GetString();
		for (auto it = _controlMapNames.Begin(); it != _controlMapNames.End(); ++it) {
			if ((*it).second_ == control) {
				activeAction = (*it).first_;
			}
		}
	}

	if (activeAction > 0) {
		// Prepare all input handlers for key mapping against specific action
		for (auto it = _inputHandlers.Begin(); it != _inputHandlers.End(); ++it) {
			(*it).second_->StartMappingAction(activeAction);
		}
		URHO3D_LOGINFO("Starting to map action!");
	}
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
	StringVector parameters = eventData["Parameters"].GetStringVector();
	if (parameters.Size() == 2) {
		using namespace MyEvents::StartInputMapping;
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

HashMap<int, String> ControllerInput::GetControlNames()
{
	return _controlMapNames;
}

String ControllerInput::GetActionKeyName(int action)
{
	String keyName = _inputHandlers[ControllerType::KEYBOARD]->GetActionKeyName(action);
	if (!keyName.Empty()) {
		return keyName;
	}

	keyName = _inputHandlers[ControllerType::MOUSE]->GetActionKeyName(action);
	if (!keyName.Empty()) {
		return keyName;
	}

	return String::EMPTY;
}

void ControllerInput::SetActionState(int action, bool active)
{
	_controls.Set(action, active);
}