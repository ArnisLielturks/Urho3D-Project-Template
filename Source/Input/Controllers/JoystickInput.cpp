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
		controllerInput->SetActionState(_mappedKeyToControl[key], true);
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
		controllerInput->SetActionState(_mappedKeyToControl[key], false);
	}
}

void JoystickInput::HandleAxisMove(StringHash eventType, VariantMap& eventData)
{
	using namespace JoystickAxisMove;
	URHO3D_PARAM(P_JOYSTICKID, JoystickID);        // int
	URHO3D_PARAM(P_AXIS, Button);                  // int
	URHO3D_PARAM(P_POSITION, Position);
	int id = eventData[P_JOYSTICKID].GetInt();
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
			controllerInput->SetActionState(CTRL_FORWARD, true);
			controllerInput->SetActionState(CTRL_BACK, false);
		}
		else if (position > 0) {
			controllerInput->SetActionState(CTRL_FORWARD, false);
			controllerInput->SetActionState(CTRL_BACK, true);
		}
		else {
			controllerInput->SetActionState(CTRL_FORWARD, false);
			controllerInput->SetActionState(CTRL_BACK, false);
		}
	}
	if (buttonId == 0) {
		auto* controllerInput = GetSubsystem<ControllerInput>();
		if (position < 0) {
			controllerInput->SetActionState(CTRL_LEFT, true);
			controllerInput->SetActionState(CTRL_RIGHT, false);
		}
		else if (position > 0) {
			controllerInput->SetActionState(CTRL_LEFT, false);
			controllerInput->SetActionState(CTRL_RIGHT, true);
		}
		else {
			controllerInput->SetActionState(CTRL_LEFT, false);
			controllerInput->SetActionState(CTRL_RIGHT, false);
		}
	}
	if (buttonId == 3) {
		_axisPosition.x_ = position;
	}
	if (buttonId == 4) {
		_axisPosition.y_ = position;
	}
}

void JoystickInput::HandleHatMove(StringHash eventType, VariantMap& eventData)
{
	using namespace JoystickHatMove;
	int id = eventData[P_JOYSTICKID].GetInt();
	int buttonId = eventData[P_HAT].GetInt();
	float position = eventData[P_POSITION].GetFloat();
	const float JOYSTICK_MOVEMENT_THRESHOLD = 0.01f;
	//URHO3D_LOGINFO(">>>> HAT Joystick ID : " + String(id) + " => " + String(buttonId) + " => " + String(position));
	if (buttonId == 0) {
		_axisPosition.x_ = position;
		if (Abs(_axisPosition.x_) < JOYSTICK_MOVEMENT_THRESHOLD) {
			_axisPosition.x_ = 0;
		}
	}
	if (buttonId == 1) {
		_axisPosition.y_ = position;
		if (Abs(_axisPosition.y_) < JOYSTICK_MOVEMENT_THRESHOLD) {
			_axisPosition.y_ = 0;
		}
	}
}

void JoystickInput::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	auto* controllerInput = GetSubsystem<ControllerInput>();
	controllerInput->UpdateYaw(_axisPosition.x_ * 10);
	controllerInput->UpdatePitch(_axisPosition.y_ * 10);
}

String JoystickInput::GetActionKeyName(int action)
{
	if (_mappedControlToKey.Contains(action)) {
		auto* input = GetSubsystem<Input>();
		return "Joy_" + String(_mappedControlToKey[action]);
	}

	return String::EMPTY;
}
