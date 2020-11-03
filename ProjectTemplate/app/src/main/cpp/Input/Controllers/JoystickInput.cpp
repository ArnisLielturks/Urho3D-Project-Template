#include <Urho3D/Input/Input.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/DebugHud.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Core/ProcessUtils.h>
#include "JoystickInput.h"
#include "../ControllerInput.h"
#include "../ControlDefines.h"

JoystickInput::JoystickInput(Context* context) :
    BaseInput(context),
    joystickAsFirstController_(true)
{
    SetMinSensitivity(2.0f);

    Init();
    auto* input = GetSubsystem<Input>();
    for (int i = 0; i < input->GetNumJoysticks(); i++) {
        axisPosition_[i] = Vector2::ZERO;
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
    if (GetPlatform() != "Android") {
        SubscribeToEvent(E_JOYSTICKBUTTONDOWN, URHO3D_HANDLER(JoystickInput, HandleKeyDown));
        SubscribeToEvent(E_JOYSTICKBUTTONUP, URHO3D_HANDLER(JoystickInput, HandleKeyUp));
        SubscribeToEvent(E_JOYSTICKCONNECTED, URHO3D_HANDLER(JoystickInput, HandleJoystickConnected));
        SubscribeToEvent(E_JOYSTICKDISCONNECTED, URHO3D_HANDLER(JoystickInput, HandleJoystickDisconnected));
        SubscribeToEvent(E_JOYSTICKAXISMOVE, URHO3D_HANDLER(JoystickInput, HandleAxisMove));
        SubscribeToEvent(E_JOYSTICKHATMOVE, URHO3D_HANDLER(JoystickInput, HandleHatMove));
        SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(JoystickInput, HandleUpdate));
    }
}

void JoystickInput::SetJoystickAsFirstController(bool enabled)
{
    joystickAsFirstController_ = enabled;
}

bool JoystickInput::GetJoystickAsFirstController()
{
    return joystickAsFirstController_;
}

void JoystickInput::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    using namespace JoystickButtonDown;
    int key = eventData[P_BUTTON].GetInt();
    int joystick = eventData[P_JOYSTICKID].GetInt();
    if (!joystickAsFirstController_) {
        joystick++;
    }
    auto* input = GetSubsystem<Input>();
    //URHO3D_LOGINFO("Joystick down " + input->GetKeyName(static_cast<Key>(key)) + " => " + String(key));

    if (activeAction_ > 0 && timer_.GetMSec(false) > 100) {
        auto* controllerInput = GetSubsystem<ControllerInput>();
        controllerInput->SetConfiguredKey(activeAction_, key, "joystick");
        activeAction_ = 0;
        return;
    }

    if (mappedKeyToControl_.Contains(key)) {
        auto* controllerInput = GetSubsystem<ControllerInput>();
        controllerInput->SetActionState(mappedKeyToControl_[key], true, joystick);
    }

    if (GetSubsystem<DebugHud>()) {
        GetSubsystem<DebugHud>()->SetAppStats("Key " + String(key), true);
    }
}

void JoystickInput::HandleKeyUp(StringHash eventType, VariantMap& eventData)
{
    using namespace JoystickButtonDown;
    int key = eventData[P_BUTTON].GetInt();
    int joystick = eventData[P_JOYSTICKID].GetInt();
    if (!joystickAsFirstController_) {
        joystick++;
    }
    auto* input = GetSubsystem<Input>();
    //URHO3D_LOGINFO("Joystick up " + input->GetKeyName(static_cast<Key>(key)) + " => " + String(key));

    if (activeAction_ > 0) {
        return;
    }

    if (mappedKeyToControl_.Contains(key)) {
        auto* controllerInput = GetSubsystem<ControllerInput>();
        controllerInput->SetActionState(mappedKeyToControl_[key], false, joystick);
    }

    if (GetSubsystem<DebugHud>()) {
        GetSubsystem<DebugHud>()->SetAppStats("JoyKey " + String(key), false);
    }
}

void JoystickInput::HandleAxisMove(StringHash eventType, VariantMap& eventData)
{
    using namespace JoystickAxisMove;
    int joystick = eventData[P_JOYSTICKID].GetInt();

    if (!joystickAsFirstController_) {
        joystick++;
    }
    int buttonId = eventData[P_AXIS].GetInt();
    float position = eventData[P_POSITION].GetFloat();

    if (GetSubsystem<DebugHud>()) {
        GetSubsystem<DebugHud>()->SetAppStats("JoyAxisMove" + String(buttonId), position);
    }

    if (Abs(position) < deadzone_) {
        position = 0.0f;
    }
    //URHO3D_LOGINFO("Joystick ID : " + String(joystick) + " => " + String(buttonId) + " => " + String(position));
    if (buttonId == joystickMapping_.y_) {
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
    if (buttonId == joystickMapping_.x_) {
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
    if (buttonId == joystickMapping_.z_) {
        if (invertX_) {
            position *= -1.0f;
        }
        axisPosition_[joystick].x_ = position;
    }
    if (buttonId == joystickMapping_.w_) {
        if (invertY_) {
            position *= -1.0f;
        }
        axisPosition_[joystick].y_ = position;
    }
}

void JoystickInput::HandleHatMove(StringHash eventType, VariantMap& eventData)
{
    using namespace JoystickHatMove;
    int joystick = eventData[P_JOYSTICKID].GetInt();
    if (!joystickAsFirstController_) {
        joystick++;
    }
    int buttonId = eventData[P_HAT].GetInt();
    int id = eventData[P_JOYSTICKID].GetInt();
    float position = eventData[P_POSITION].GetFloat();

    if (GetSubsystem<DebugHud>()) {
        GetSubsystem<DebugHud>()->SetAppStats("JoyHatMove" + String(buttonId), position);
    }

    if (buttonId == 0) {
        if (invertX_) {
            position *= -1.0f;
        }
        axisPosition_[joystick].x_ = position;
        if (Abs(axisPosition_[joystick].x_) < deadzone_) {
            axisPosition_[joystick].x_ = 0;
        }
    }
    if (buttonId == 1) {
        if (invertY_) {
            position *= -1.0f;
        }
        axisPosition_[joystick].y_ = position;
        if (Abs(axisPosition_[joystick].y_) < deadzone_) {
            axisPosition_[joystick].y_ = 0;
        }
    }
}

void JoystickInput::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    auto* controllerInput = GetSubsystem<ControllerInput>();
    auto* input = GetSubsystem<Input>();
    for (auto it = axisPosition_.Begin(); it != axisPosition_.End(); ++it) {
        controllerInput->UpdateYaw((*it).second_.x_ * sensitivityX_, (*it).first_);
        controllerInput->UpdatePitch((*it).second_.y_ * sensitivityX_, (*it).first_);
    }
}

String JoystickInput::GetActionKeyName(int action)
{
    if (mappedControlToKey_.Contains(action)) {
        auto* input = GetSubsystem<Input>();
        return "Joy_" + String(mappedControlToKey_[action]);
    }

    return String::EMPTY;
}

void JoystickInput::HandleJoystickConnected(StringHash eventType, VariantMap& eventData)
{
    using namespace JoystickConnected;
    int id = eventData[P_JOYSTICKID].GetInt();
    if (!joystickAsFirstController_) {
        id++;
    }
    URHO3D_LOGINFO("Joystick connected : " + String(id));
    axisPosition_[id] = Vector2::ZERO;
    auto* controllerInput = GetSubsystem<ControllerInput>();
    controllerInput->CreateController(id);
}

void JoystickInput::HandleJoystickDisconnected(StringHash eventType, VariantMap& eventData)
{
    using namespace JoystickDisconnected;
    int id = eventData[P_JOYSTICKID].GetInt();
    if (!joystickAsFirstController_) {
        id++;
    }
    URHO3D_LOGINFO("Joystick disconnected : " + String(id));
    axisPosition_.Erase(id);
    auto* controllerInput = GetSubsystem<ControllerInput>();
    controllerInput->DestroyController(id);
}

void JoystickInput::LoadConfig()
{
    sensitivityX_ = GetSubsystem<ConfigManager>()->GetFloat("joystick", "SensitivityX", 1.0f);
    sensitivityY_ = GetSubsystem<ConfigManager>()->GetFloat("joystick", "SensitivityY", 1.0f);
    invertX_ = GetSubsystem<ConfigManager>()->GetBool("joystick", "InvertX", false);
    invertY_ = GetSubsystem<ConfigManager>()->GetBool("joystick", "InvertY", false);

    //TODO put these settings inside controllers tab
    joystickMapping_.x_ = GetSubsystem<ConfigManager>()->GetInt("joystick", "MoveXAxis", 0);
    joystickMapping_.y_ = GetSubsystem<ConfigManager>()->GetInt("joystick", "MoveYAxis", 1);
    joystickMapping_.z_ = GetSubsystem<ConfigManager>()->GetInt("joystick", "RotateXAxis");
    joystickMapping_.w_ = GetSubsystem<ConfigManager>()->GetInt("joystick", "RotateYAxis");
}
