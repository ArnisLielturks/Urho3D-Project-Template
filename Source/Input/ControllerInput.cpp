#include <Urho3D/Core/Context.h>
#include <Urho3D/Engine/DebugHud.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/Log.h>
#include "ControllerInput.h"
#include "Controllers/KeyboardInput.h"
#include "Controllers/MouseInput.h"
#include "Controllers/JoystickInput.h"
#include "Controllers/ScreenJoystickInput.h"
#include "ControllerEvents.h"
#include "ControlDefines.h"

using namespace Urho3D;
using namespace ControllerEvents;

ControllerInput::ControllerInput(Context* context) :
    Object(context),
    multipleControllerSupport_(true),
    activeAction_(-1)
{

    context_->RegisterFactory<BaseInput>();
    context_->RegisterFactory<KeyboardInput>();
    context_->RegisterFactory<MouseInput>();
    context_->RegisterFactory<JoystickInput>();
    context_->RegisterFactory<ScreenJoystickInput>();

    inputHandlers_[ControllerType::KEYBOARD] = context_->CreateObject<KeyboardInput>();
    inputHandlers_[ControllerType::MOUSE] = context_->CreateObject<MouseInput>();
    inputHandlers_[ControllerType::JOYSTICK] = context_->CreateObject<JoystickInput>();
    inputHandlers_[ControllerType::SCREEN_JOYSTICK] = context_->CreateObject<ScreenJoystickInput>();

    controlMapNames_[CTRL_FORWARD]  = "Move forward";
    controlMapNames_[CTRL_BACK]     = "Move backward";
    controlMapNames_[CTRL_LEFT]     = "Strafe left";
    controlMapNames_[CTRL_RIGHT]    = "Strafe right";
    controlMapNames_[CTRL_JUMP]     = "Jump";
    controlMapNames_[CTRL_ACTION]   = "Primary action";
    controlMapNames_[CTRL_SECONDARY] = "Secondary action";
    controlMapNames_[CTRL_SPRINT]   = "Sprint";
    controlMapNames_[CTRL_UP]       = "Move up";
    controlMapNames_[CTRL_SCREENSHOT] = "Take screenshot";
    controlMapNames_[CTRL_DETECT] = "Detect";
    controlMapNames_[CTRL_CHANGE_ITEM] = "Change item";

    Init();
}

ControllerInput::~ControllerInput()
{
}

void ControllerInput::Init()
{
    // Subscribe to global events for camera movement
    SubscribeToEvents();

    // there must be at least one controller available at start
    controls_[0] = Controls();

    if (GetSubsystem<DebugHud>()) {
        GetSubsystem<DebugHud>()->SetAppStats("Controls", controls_.Size());
    }
}

void ControllerInput::LoadConfig()
{
    GetSubsystem<ConfigManager>();

#ifdef __EMSCRIPTEN__
    inputHandlers_[ControllerType::KEYBOARD]->SetKeyToAction(KEY_W, CTRL_FORWARD);
    inputHandlers_[ControllerType::KEYBOARD]->SetKeyToAction(KEY_S, CTRL_BACK);
    inputHandlers_[ControllerType::KEYBOARD]->SetKeyToAction(KEY_A, CTRL_LEFT);
    inputHandlers_[ControllerType::KEYBOARD]->SetKeyToAction(KEY_D, CTRL_RIGHT);
    inputHandlers_[ControllerType::KEYBOARD]->SetKeyToAction(KEY_SPACE, CTRL_JUMP);
    inputHandlers_[ControllerType::KEYBOARD]->SetKeyToAction(KEY_LSHIFT, CTRL_SPRINT);
    inputHandlers_[ControllerType::MOUSE]->SetKeyToAction(MOUSEB_LEFT, CTRL_ACTION);
    inputHandlers_[ControllerType::MOUSE]->SetKeyToAction(MOUSEB_RIGHT, CTRL_SECONDARY);
#endif

    for (auto it = controlMapNames_.Begin(); it != controlMapNames_.End(); ++it) {
        String controlName = (*it).second_;
        controlName.Replace(" ", "_");
        int controlCode = (*it).first_;
        if (GetSubsystem<ConfigManager>()->GetInt("keyboard", controlName, -1) != -1) {
            int key = GetSubsystem<ConfigManager>()->GetInt("keyboard", controlName, 0);
            inputHandlers_[ControllerType::KEYBOARD]->SetKeyToAction(key, controlCode);
        }
        if (GetSubsystem<ConfigManager>()->GetInt("mouse", controlName, -1) != -1) {
            int key = GetSubsystem<ConfigManager>()->GetInt("mouse", controlName, 0);
            inputHandlers_[ControllerType::MOUSE]->SetKeyToAction(key, controlCode);
        }
        if (GetSubsystem<ConfigManager>()->GetInt("joystick", controlName, -1) != -1) {
            int key = GetSubsystem<ConfigManager>()->GetInt("joystick", controlName, 0);
            inputHandlers_[ControllerType::JOYSTICK]->SetKeyToAction(key, controlCode);
        }
    }

    for (auto it = inputHandlers_.Begin(); it != inputHandlers_.End(); ++it) {
        (*it).second_->LoadConfig();
    }

    // Single player mode, all the input is handled by single Controls object
    SetMultipleControllerSupport(GetSubsystem<ConfigManager>()->GetBool("joystick", "MultipleControllers", false));
    // Keyboard/mouse - 1st player, all the connected joysticks control new players
    // This will have no effect if `SetMultipleControllerSupport` is set to `false`
    SetJoystickAsFirstController(GetSubsystem<ConfigManager>()->GetBool("joystick", "JoystickAsFirstController", true));
}

void ControllerInput::SaveConfig()
{
    for (auto it = controlMapNames_.Begin(); it != controlMapNames_.End(); ++it) {
        String controlName = (*it).second_;
        controlName.Replace(" ", "_");
        int controlCode = (*it).first_;
        GetSubsystem<ConfigManager>()->Set("keyboard", controlName, "-1");
        GetSubsystem<ConfigManager>()->Set("mouse", controlName, "-1");
        GetSubsystem<ConfigManager>()->Set("joystick", controlName, "-1");
    }

    for (auto it = inputHandlers_.Begin(); it != inputHandlers_.End(); ++it) {
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
             if (controlMapNames_.Contains(controlCode) && !controlMapNames_[controlCode].Empty()) {
                String controlName = controlMapNames_[controlCode];
                controlName.Replace(" ", "_");
                String value = String(keyCode);
                GetSubsystem<ConfigManager>()->Set(map[type], controlName, value);
            }
        }
    }

    GetSubsystem<ConfigManager>()->Save(true);
}

void ControllerInput::SubscribeToEvents()
{
    SubscribeToEvent(E_START_INPUT_MAPPING, URHO3D_HANDLER(ControllerInput, HandleStartInputListening));

    SubscribeToEvent("StartInputMappingConsole", URHO3D_HANDLER(ControllerInput, HandleStartInputListeningConsole));
    RegisterConsoleCommands();
}

void ControllerInput::ReleaseConfiguredKey(int key, int action)
{
    // Clear all input handler mappings against key and actions
    for (auto it = inputHandlers_.Begin(); it != inputHandlers_.End(); ++it) {
        (*it).second_->ReleaseAction(action);
        (*it).second_->ReleaseKey(key);
    }
}

void ControllerInput::SetConfiguredKey(int action, int key, String controller)
{
    // Clear previously assigned key and/or action
    ReleaseConfiguredKey(key, action);
    auto* input = GetSubsystem<Input>();
    if (controller == "keyboard") {
        inputHandlers_[ControllerType::KEYBOARD]->SetKeyToAction(key, action);
    }
    if (controller == "mouse") {
        inputHandlers_[ControllerType::MOUSE]->SetKeyToAction(key, action);
    }
    if (controller == "joystick") {
        inputHandlers_[ControllerType::JOYSTICK]->SetKeyToAction(key, action);
    }

    // Stop listening for keyboard key mapping
    for (auto it = inputHandlers_.Begin(); it != inputHandlers_.End(); ++it) {
        (*it).second_->StopMappingAction();
        activeAction_ = -1;
    }

    // Send out event with all the details about the mapped control
    using namespace InputMappingFinished;
    VariantMap& data = GetEventDataMap();
    data[P_CONTROLLER] = controller;
    data[P_CONTROL_ACTION] = action;
    data[P_ACTION_NAME] = controlMapNames_[action];
    data[P_KEY] = key;
    data[P_KEY_NAME] = input->GetKeyName(static_cast<Key>(key));
    SendEvent(E_INPUT_MAPPING_FINISHED, data);

    SaveConfig();
}

void ControllerInput::StopInputMapping()
{
    using namespace StopInputMapping;
    VariantMap& data = GetEventDataMap();
    data[P_CONTROL_ACTION] = activeAction_;

    activeAction_ = -1;
    // Stop listening for keyboard key mapping
    for (auto it = inputHandlers_.Begin(); it != inputHandlers_.End(); ++it) {
        (*it).second_->StopMappingAction();
    }

    SendEvent(E_STOP_INPUT_MAPPING, data);

    mappingTimer_.Reset();
}

void ControllerInput::HandleStartInputListening(StringHash eventType, VariantMap& eventData)
{
    using namespace StartInputMapping;
    if (eventData[P_CONTROL_ACTION].GetType() == VAR_INT) {
        activeAction_ = eventData[P_CONTROL_ACTION].GetInt();
    }
    if (eventData[P_CONTROL_ACTION].GetType() == VAR_STRING) {
        String control = eventData[P_CONTROL_ACTION].GetString();
        for (auto it = controlMapNames_.Begin(); it != controlMapNames_.End(); ++it) {
            if ((*it).second_ == control) {
                activeAction_ = (*it).first_;
            }
        }
    }

    if (activeAction_ >= 0) {
        // Prepare all input handlers for key mapping against specific action
        for (auto it = inputHandlers_.Begin(); it != inputHandlers_.End(); ++it) {
            (*it).second_->StartMappingAction(activeAction_);
        }
        URHO3D_LOGINFO("Starting to map action!");
    }
}

void ControllerInput::RegisterConsoleCommands()
{
    VariantMap& data = GetEventDataMap();
    data["ConsoleCommandName"] = "map_input";
    data["ConsoleCommandEvent"] = "StartInputMappingConsole";
    data["ConsoleCommandDescription"] = "Listening for keystroke";
    SendEvent("ConsoleCommandAdd", data);
}

void ControllerInput::HandleStartInputListeningConsole(StringHash eventType, VariantMap& eventData)
{
    StringVector parameters = eventData["Parameters"].GetStringVector();
    if (parameters.Size() == 2) {
        using namespace StartInputMapping;
        VariantMap& data = GetEventDataMap();
        data[P_CONTROL_ACTION] = parameters[1];
        SendEvent(E_START_INPUT_MAPPING, data);
        return;
    }

    URHO3D_LOGERROR("Invalid number of parameters!");
}

Controls ControllerInput::GetControls(int index)
{
    if (!multipleControllerSupport_) {
        index = 0;
    }
    if (!controls_.Contains(index)) {
        return Controls();
    }
    return controls_[index];
}

void ControllerInput::UpdateYaw(float yaw, int index)
{
    if (!multipleControllerSupport_) {
        index = 0;
    }
    const float MOUSE_SENSITIVITY = 0.1f;
    controls_[index].yaw_ += MOUSE_SENSITIVITY * yaw;
}

void ControllerInput::UpdatePitch(float pitch, int index)
{
    if (!multipleControllerSupport_) {
        index = 0;
    }
    const float MOUSE_SENSITIVITY = 0.1f;
    controls_[index].pitch_ += MOUSE_SENSITIVITY * pitch;
    controls_[index].pitch_ = Clamp(controls_[index].pitch_, -89.0f, 89.0f);
}

void ControllerInput::CreateController(int controllerIndex)
{
    if (!multipleControllerSupport_) {
        return;
    }
    using namespace ControllerAdded;
    controls_[controllerIndex] = Controls();
    VariantMap& data           = GetEventDataMap();
    data[P_INDEX]              = controllerIndex;
    SendEvent(E_CONTROLLER_ADDED, data);

    if (GetSubsystem<DebugHud>()) {
        GetSubsystem<DebugHud>()->SetAppStats("Controls", controls_.Size());
    }
}

void ControllerInput::DestroyController(int controllerIndex)
{
    if (!multipleControllerSupport_) {
        return;
    }
    // Don't allow destroying first input controller
    if (controllerIndex > 0) {
        controls_.Erase(controllerIndex);

        using namespace ControllerRemoved;

        VariantMap& data = GetEventDataMap();
        data[P_INDEX] = controllerIndex;
        SendEvent(E_CONTROLLER_REMOVED, data);
    }
    if (GetSubsystem<DebugHud>()) {
        GetSubsystem<DebugHud>()->SetAppStats("Controls", controls_.Size());
    }
}

HashMap<int, String> ControllerInput::GetControlNames()
{
    return controlMapNames_;
}

String ControllerInput::GetActionKeyName(int action)
{
    if (action == activeAction_) {
        return "...";
    }
    for (auto it = inputHandlers_.Begin(); it != inputHandlers_.End(); ++it) {
        if ((*it).second_->IsActionUsed(action)) {
            String keyName = (*it).second_->GetActionKeyName(action);
            return keyName;
        }
    }

    return String::EMPTY;
}

void ControllerInput::SetActionState(int action, bool active, int index, float strength)
{
    if (!multipleControllerSupport_) {
        index = 0;
    }

    // Mapped control is about to change, send out events
    if (controls_[index].IsDown(action) != active) {
        if (active) {
            using namespace MappedControlPressed;
            VariantMap &data = GetEventDataMap();
            data[P_ACTION] = action;
            data[P_CONTROLLER] = index;
            SendEvent(E_MAPPED_CONTROL_PRESSED, data);
        } else {
            using namespace MappedControlReleased;
            VariantMap &data = GetEventDataMap();
            data[P_ACTION] = action;
            data[P_CONTROLLER] = index;
            SendEvent(E_MAPPED_CONTROL_RELEASED, data);
        }
    }
    controls_[index].Set(action, active);
    controls_[index].extraData_[StringHash(action)] = strength;
}

Vector<int> ControllerInput::GetControlIndexes()
{
    Vector<int> indexes;
    if (!multipleControllerSupport_) {
        indexes.Push(0);
        return indexes;
    }

    for (auto it = controls_.Begin(); it != controls_.End(); ++it) {
        indexes.Push((*it).first_);
    }

    if (indexes.Empty()) {
        indexes.Push(0);
    }
    return indexes;
}

void ControllerInput::SetMultipleControllerSupport(bool enabled)
{
    multipleControllerSupport_ = enabled;
}

bool ControllerInput::IsMappingInProgress()
{
    return activeAction_ >= 0 || mappingTimer_.GetMSec(false) < 20;
}

void ControllerInput::SetJoystickAsFirstController(bool enabled)
{
    BaseInput* input = inputHandlers_[ControllerType::JOYSTICK];
    JoystickInput* joystickInput = input->Cast<JoystickInput>();
    if (joystickInput) {
        joystickInput->SetJoystickAsFirstController(enabled);
    }
}

bool ControllerInput::GetJoystickAsFirstController()
{
    BaseInput* input = inputHandlers_[ControllerType::JOYSTICK];
    JoystickInput* joystickInput = input->Cast<JoystickInput>();
    if (joystickInput) {
        return joystickInput->GetJoystickAsFirstController();
    }

    return false;
}

void ControllerInput::SetInvertX(bool enabled, int controller)
{
    switch (controller) {
    case ControllerType::MOUSE:
        inputHandlers_[ControllerType::MOUSE]->SetInvertX(enabled);
        break;
    case ControllerType::JOYSTICK:
        inputHandlers_[ControllerType::JOYSTICK]->SetInvertX(enabled);
        break;
    }


}

bool ControllerInput::GetInvertX(int controller)
{
    switch (controller) {
    case ControllerType::MOUSE:
        return inputHandlers_[ControllerType::MOUSE]->GetInvertX();
        break;
    case ControllerType::JOYSTICK:
        return inputHandlers_[ControllerType::JOYSTICK]->GetInvertX();
        break;
    }
    return false;
}

void ControllerInput::SetInvertY(bool enabled, int controller)
{
    switch (controller) {
    case ControllerType::MOUSE:
        inputHandlers_[ControllerType::MOUSE]->SetInvertY(enabled);
        break;
    case ControllerType::JOYSTICK:
        inputHandlers_[ControllerType::JOYSTICK]->SetInvertY(enabled);
        break;
    }
}

bool ControllerInput::GetInvertY(int controller)
{
    switch (controller) {
    case ControllerType::MOUSE:
        return inputHandlers_[ControllerType::MOUSE]->GetInvertY();
        break;
    case ControllerType::JOYSTICK:
        return inputHandlers_[ControllerType::JOYSTICK]->GetInvertY();
        break;
    }
    return false;
}

void ControllerInput::SetSensitivityX(float value, int controller)
{
    switch (controller) {
    case ControllerType::MOUSE:
        inputHandlers_[ControllerType::MOUSE]->SetSensitivityX(value);
        break;
    case ControllerType::JOYSTICK:
        inputHandlers_[ControllerType::JOYSTICK]->SetSensitivityX(value);
        break;
    }
}

void ControllerInput::SetSensitivityY(float value, int controller)
{
    switch (controller) {
    case ControllerType::MOUSE:
        inputHandlers_[ControllerType::MOUSE]->SetSensitivityY(value);
        break;
    case ControllerType::JOYSTICK:
        inputHandlers_[ControllerType::JOYSTICK]->SetSensitivityY(value);
        break;
    }
}

float ControllerInput::GetSensitivityX(int controller)
{
    switch (controller) {
    case ControllerType::MOUSE:
        return inputHandlers_[ControllerType::MOUSE]->GetSensitivityX();
        break;
    case ControllerType::JOYSTICK:
        return inputHandlers_[ControllerType::JOYSTICK]->GetSensitivityX();
        break;
    }

    return 0.0f;
}

float ControllerInput::GetSensitivityY(int controller)
{
    switch (controller) {
    case ControllerType::MOUSE:
        return inputHandlers_[ControllerType::MOUSE]->GetSensitivityY();
        break;
    case ControllerType::JOYSTICK:
        return inputHandlers_[ControllerType::JOYSTICK]->GetSensitivityY();
        break;
    }

    return 0.0f;
}

void ControllerInput::SetJoystickDeadzone(float value)
{
    inputHandlers_[ControllerType::JOYSTICK]->SetDeadzone(value);
}

float ControllerInput::GetJoystickDeadzone()
{
    return inputHandlers_[ControllerType::JOYSTICK]->GetDeadzone();
}

void ControllerInput::ShowOnScreenJoystick()
{
    inputHandlers_[ControllerType::SCREEN_JOYSTICK]->Show();
}

void ControllerInput::HideOnScreenJoystick()
{
    inputHandlers_[ControllerType::SCREEN_JOYSTICK]->Hide();
}
