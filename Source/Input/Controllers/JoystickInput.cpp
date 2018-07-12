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
	SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(JoystickInput, HandleKeyDown));
	SubscribeToEvent(E_KEYUP, URHO3D_HANDLER(JoystickInput, HandleKeyUp));
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

String JoystickInput::GetActionKeyName(int action)
{
	if (_mappedControlToKey.Contains(action)) {
		auto* input = GetSubsystem<Input>();
		return "Joy_" + String(_mappedControlToKey[action]);
	}

	return String::EMPTY;
}
