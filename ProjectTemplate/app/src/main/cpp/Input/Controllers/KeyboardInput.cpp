#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Input/Input.h>
#include "KeyboardInput.h"
#include "../ControllerInput.h"
#include "../ControlDefines.h"

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

    if (key == KEY_ESCAPE && activeAction_ > 0) {
        GetSubsystem<ControllerInput>()->StopInputMapping();
        activeAction_ = 0;
        URHO3D_LOGINFO("Control mapping stopped");
        return;
    }

    if (activeAction_ > 0 && timer_.GetMSec(false) > 100) {
        auto* controllerInput = GetSubsystem<ControllerInput>();
        controllerInput->SetConfiguredKey(activeAction_, key, "keyboard");
        activeAction_ = 0;
        return;
    }

    if (mappedKeyToControl_.Contains(key)) {
        auto* controllerInput = GetSubsystem<ControllerInput>();
        controllerInput->SetActionState(mappedKeyToControl_[key], true);
    }
}

void KeyboardInput::HandleKeyUp(StringHash eventType, VariantMap& eventData)
{
    using namespace KeyUp;
    int key = eventData[P_KEY].GetInt();

    if (activeAction_ > 0) {
        return;
    }

    if (mappedKeyToControl_.Contains(key)) {
        auto* controllerInput = GetSubsystem<ControllerInput>();
        controllerInput->SetActionState(mappedKeyToControl_[key], false);
    }
}

String KeyboardInput::GetActionKeyName(int action)
{
    if (mappedControlToKey_.Contains(action)) {
        auto* input = GetSubsystem<Input>();
        return input->GetKeyName(static_cast<Key>(mappedControlToKey_[action]));
    }

    return String::EMPTY;
}
