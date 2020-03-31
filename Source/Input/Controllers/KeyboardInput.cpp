#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Input/Input.h>
#include "KeyboardInput.h"
#include "../ControllerInput.h"
#include "../../MyEvents.h"
#include "../../Global.h"

/// Construct.
KeyboardInput::KeyboardInput(Context* context) :
    BaseInput(context)
{
    Init();
}

KeyboardInput::~KeyboardInput()
{
}

void KeyboardInput::Init()
{
    // Subscribe to global events for camera movement
    SubscribeToEvents();
}

void KeyboardInput::SubscribeToEvents()
{
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(KeyboardInput, HandleKeyDown));
    SubscribeToEvent(E_KEYUP, URHO3D_HANDLER(KeyboardInput, HandleKeyUp));
}

void KeyboardInput::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    using namespace KeyDown;
    int key = eventData[P_KEY].GetInt();

    if (key == KEY_ESCAPE && _activeAction > 0) {
        GetSubsystem<ControllerInput>()->StopInputMapping();
        _activeAction = 0;
        URHO3D_LOGINFO("Control mapping stopped");
        return;
    }

    if (_activeAction > 0 && _timer.GetMSec(false) > 100) {
        auto* controllerInput = GetSubsystem<ControllerInput>();
        controllerInput->SetConfiguredKey(_activeAction, key, "keyboard");
        _activeAction = 0;
        return;
    }

    if (_mappedKeyToControl.Contains(key)) {
        auto* controllerInput = GetSubsystem<ControllerInput>();
        controllerInput->SetActionState(_mappedKeyToControl[key], true);
    }
}

void KeyboardInput::HandleKeyUp(StringHash eventType, VariantMap& eventData)
{
    using namespace KeyUp;
    int key = eventData[P_KEY].GetInt();

    if (_activeAction > 0) {
        return;
    }

    if (_mappedKeyToControl.Contains(key)) {
        auto* controllerInput = GetSubsystem<ControllerInput>();
        controllerInput->SetActionState(_mappedKeyToControl[key], false);
    }
}

String KeyboardInput::GetActionKeyName(int action)
{
    if (_mappedControlToKey.Contains(action)) {
        auto* input = GetSubsystem<Input>();
        return input->GetKeyName(static_cast<Key>(_mappedControlToKey[action]));
    }

    return String::EMPTY;
}
