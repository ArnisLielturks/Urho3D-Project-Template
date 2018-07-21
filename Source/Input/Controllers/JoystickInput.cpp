#include <Urho3D/Urho3DAll.h>
#include "JoystickInput.h"
#include "../ControllerInput.h"
#include "../../MyEvents.h"
#include "../../Global.h"

/// Construct.
JoystickInput::JoystickInput(Context* context) :
	BaseInput(context)
{
	Init();
	auto* input = GetSubsystem<Input>();
	for (int i = 0; i < input->GetNumJoysticks(); i++) {
		_axisPosition[i] = Vector2::ZERO;
	}
}

JoystickInput::~JoystickInput()
{
}

void JoystickInput::Init()
{
    // Subscribe to global events for camera movement
    SubscribeToEvents();
}

void JoystickInput::SubscribeToEvents()
{
	SubscribeToEvent(E_JOYSTICKBUTTONDOWN, URHO3D_HANDLER(JoystickInput, HandleKeyDown));
	SubscribeToEvent(E_JOYSTICKBUTTONUP, URHO3D_HANDLER(JoystickInput, HandleKeyUp));
	SubscribeToEvent(E_JOYSTICKCONNECTED, URHO3D_HANDLER(JoystickInput, HandleJoystickConnected));
	SubscribeToEvent(E_JOYSTICKDISCONNECTED, URHO3D_HANDLER(JoystickInput, HandleJoystickDisconnected));
	SubscribeToEvent(E_JOYSTICKAXISMOVE, URHO3D_HANDLER(JoystickInput, HandleAxisMove));
	SubscribeToEvent(E_JOYSTICKHATMOVE, URHO3D_HANDLER(JoystickInput, HandleHatMove));
	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(JoystickInput, HandleUpdate));
}

void JoystickInput::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
	using namespace JoystickButtonDown;
	int key = eventData[P_BUTTON].GetInt();
	int joystick = eventData[P_JOYSTICKID].GetInt();
	auto* input = GetSubsystem<Input>();
	URHO3D_LOGINFO("Joystick down " + input->GetKeyName(key) + " => " + String(key));

	if (_activeAction > 0 && _timer.GetMSec(false) > 100) {
		auto* controllerInput = GetSubsystem<ControllerInput>();
		controllerInput->SetConfiguredKey(_activeAction, key, "joystick");
		_activeAction = 0;
		return;
	}

	if (_mappedKeyToControl.Contains(key)) {
		auto* controllerInput = GetSubsystem<ControllerInput>();
		controllerInput->SetActionState(_mappedKeyToControl[key], true, joystick);
	}
}

void JoystickInput::HandleKeyUp(StringHash eventType, VariantMap& eventData)
{
	using namespace JoystickButtonDown;
	int key = eventData[P_BUTTON].GetInt();
	int joystick = eventData[P_JOYSTICKID].GetInt();
	auto* input = GetSubsystem<Input>();
	URHO3D_LOGINFO("Joystick up " + input->GetKeyName(key) + " => " + String(key));

	if (_activeAction > 0) {
		return;
	}

	if (_mappedKeyToControl.Contains(key)) {
		auto* controllerInput = GetSubsystem<ControllerInput>();
		controllerInput->SetActionState(_mappedKeyToControl[key], false, joystick);
	}
}

void JoystickInput::HandleAxisMove(StringHash eventType, VariantMap& eventData)
{
	using namespace JoystickAxisMove;
	URHO3D_PARAM(P_JOYSTICKID, JoystickID);        // int
	URHO3D_PARAM(P_AXIS, Button);                  // int
	URHO3D_PARAM(P_POSITION, Position);
	int joystick = eventData[P_JOYSTICKID].GetInt();
	int buttonId = eventData[P_AXIS].GetInt();
	float position = eventData[P_POSITION].GetFloat();

	const float JOYSTICK_MOVEMENT_THRESHOLD = 0.01f;
	if (Abs(position) < JOYSTICK_MOVEMENT_THRESHOLD) {
		position = 0.0f;
	}
	//URHO3D_LOGINFO("Joystick ID : " + String(id) + " => " + String(buttonId) + " => " + String(position));
	if (buttonId == 1) {
		auto* controllerInput = GetSubsystem<ControllerInput>();
		if (position < 0) {
			controllerInput->SetActionState(CTRL_FORWARD, true, joystick);
			controllerInput->SetActionState(CTRL_BACK, false, joystick);
		}
		else if (position > 0) {
			controllerInput->SetActionState(CTRL_FORWARD, false, joystick);
			controllerInput->SetActionState(CTRL_BACK, true, joystick);
		}
		else {
			controllerInput->SetActionState(CTRL_FORWARD, false, joystick);
			controllerInput->SetActionState(CTRL_BACK, false, joystick);
		}
	}
	if (buttonId == 0) {
		auto* controllerInput = GetSubsystem<ControllerInput>();
		if (position < 0) {
			controllerInput->SetActionState(CTRL_LEFT, true, joystick);
			controllerInput->SetActionState(CTRL_RIGHT, false, joystick);
		}
		else if (position > 0) {
			controllerInput->SetActionState(CTRL_LEFT, false, joystick);
			controllerInput->SetActionState(CTRL_RIGHT, true, joystick);
		}
		else {
			controllerInput->SetActionState(CTRL_LEFT, false, joystick);
			controllerInput->SetActionState(CTRL_RIGHT, false, joystick);
		}
	}
	if (buttonId == 3) {
		_axisPosition[joystick].x_ = position;
	}
	if (buttonId == 4) {
		_axisPosition[joystick].y_ = position;
	}
}

void JoystickInput::HandleHatMove(StringHash eventType, VariantMap& eventData)
{
	using namespace JoystickHatMove;
	int joystick = eventData[P_JOYSTICKID].GetInt();
	int buttonId = eventData[P_HAT].GetInt();
	float position = eventData[P_POSITION].GetFloat();
	const float JOYSTICK_MOVEMENT_THRESHOLD = 0.01f;
	//URHO3D_LOGINFO(">>>> HAT Joystick ID : " + String(id) + " => " + String(buttonId) + " => " + String(position));
	if (buttonId == 0) {
		_axisPosition[joystick].x_ = position;
		if (Abs(_axisPosition[joystick].x_) < JOYSTICK_MOVEMENT_THRESHOLD) {
			_axisPosition[joystick].x_ = 0;
		}
	}
	if (buttonId == 1) {
		_axisPosition[joystick].y_ = position;
		if (Abs(_axisPosition[joystick].y_) < JOYSTICK_MOVEMENT_THRESHOLD) {
			_axisPosition[joystick].y_ = 0;
		}
	}
}

void JoystickInput::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	auto* controllerInput = GetSubsystem<ControllerInput>();
	auto* input = GetSubsystem<Input>();
	const float JOYSTICK_AXIS_SENSITIVITY = 20.0f;
	for (auto it = _axisPosition.Begin(); it != _axisPosition.End(); ++it) {
		controllerInput->UpdateYaw((*it).second_.x_ * JOYSTICK_AXIS_SENSITIVITY, (*it).first_);
		controllerInput->UpdatePitch((*it).second_.y_ * JOYSTICK_AXIS_SENSITIVITY, (*it).first_);
	}
}

String JoystickInput::GetActionKeyName(int action)
{
	if (_mappedControlToKey.Contains(action)) {
		auto* input = GetSubsystem<Input>();
		return "Joy_" + String(_mappedControlToKey[action]);
	}

	return String::EMPTY;
}

void JoystickInput::HandleJoystickConnected(StringHash eventType, VariantMap& eventData)
{
	using namespace JoystickConnected;
	int id = eventData[P_JOYSTICKID].GetInt();
	URHO3D_LOGINFO("Joystick connected : " + String(id));
	_axisPosition[id] = Vector2::ZERO;
	auto* controllerInput = GetSubsystem<ControllerInput>();
	controllerInput->CreateController(id);
}

void JoystickInput::HandleJoystickDisconnected(StringHash eventType, VariantMap& eventData)
{
	using namespace JoystickDisconnected;
	int id = eventData[P_JOYSTICKID].GetInt();
	URHO3D_LOGINFO("Joystick disconnected : " + String(id));
	_axisPosition.Erase(id);
	auto* controllerInput = GetSubsystem<ControllerInput>();
	controllerInput->DestroyController(id);
}