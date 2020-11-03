#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/UI/UI.h>
#include "MouseInput.h"
#include "../ControllerInput.h"
#include "../ControlDefines.h"

MouseInput::MouseInput(Context* context) :
    BaseInput(context)
{
    SetMinSensitivity(0.1f);
    Init();
}

MouseInput::~MouseInput()
{
}

void MouseInput::Init()
{
    // Subscribe to global events for camera movement
    SubscribeToEvents();
}

void MouseInput::SubscribeToEvents()
{
    SubscribeToEvent(E_MOUSEBUTTONDOWN, URHO3D_HANDLER(MouseInput, HandleKeyDown));
    SubscribeToEvent(E_MOUSEBUTTONUP, URHO3D_HANDLER(MouseInput, HandleKeyUp));
    SubscribeToEvent(E_MOUSEMOVE, URHO3D_HANDLER(MouseInput, HandleMouseMove));
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(MouseInput, HandleUpdate));

    SubscribeToEvent(E_TOUCHBEGIN, URHO3D_HANDLER(MouseInput, HandleTouchUI));
    SubscribeToEvent(E_TOUCHMOVE, URHO3D_HANDLER(MouseInput, HandleTouchUI));
    SubscribeToEvent(E_TOUCHEND, URHO3D_HANDLER(MouseInput, HandleTouchUI));
//    GetSubsystem<Input>()->SetTouchEmulation(true);
}

void MouseInput::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    using namespace MouseButtonDown;
    int key = eventData[P_BUTTON].GetInt();

    if (activeAction_ > 0 && timer_.GetMSec(false) > 100) {
        auto* controllerInput = GetSubsystem<ControllerInput>();
        controllerInput->SetConfiguredKey(activeAction_, key, "mouse");
        activeAction_ = 0;
        return;
    }

    if (mappedKeyToControl_.Contains(key)) {
        auto* controllerInput = GetSubsystem<ControllerInput>();
        controllerInput->SetActionState(mappedKeyToControl_[key], true);
    }
}

void MouseInput::HandleKeyUp(StringHash eventType, VariantMap& eventData)
{
    using namespace MouseButtonUp;
    int key = eventData[P_BUTTON].GetInt();

    if (activeAction_ > 0) {
        return;
    }

    if (mappedKeyToControl_.Contains(key)) {
        auto* controllerInput = GetSubsystem<ControllerInput>();
        controllerInput->SetActionState(mappedKeyToControl_[key], false);
    }
}

void MouseInput::HandleMouseMove(StringHash eventType, VariantMap& eventData)
{
    using namespace MouseMove;
    float dx = eventData[P_DX].GetInt() * sensitivityX_;
    float dy = eventData[P_DY].GetInt() * sensitivityY_;
    if (invertX_) {
        dx *= -1.0f;
    }
    if (invertY_) {
        dy *= -1.0f;
    }
    ControllerInput* controllerInput = GetSubsystem<ControllerInput>();
    controllerInput->UpdateYaw(dx);
    controllerInput->UpdatePitch(dy);
}

String MouseInput::GetActionKeyName(int action)
{
    if (mappedControlToKey_.Contains(action)) {
        auto* input = GetSubsystem<Input>();
        int key = mappedControlToKey_[action];
        if (key == MOUSEB_LEFT) {
            return "MOUSEB_LEFT";
        }
        if (key == MOUSEB_MIDDLE) {
            return "MOUSEB_MIDDLE";
        }
        if (key == MOUSEB_RIGHT) {
            return "MOUSEB_RIGHT";
        }
    }

    return String::EMPTY;
}

void MouseInput::LoadConfig()
{
    sensitivityX_ = GetSubsystem<ConfigManager>()->GetFloat("mouse", "Sensitivity", 1.0f);
    sensitivityY_ = GetSubsystem<ConfigManager>()->GetFloat("mouse", "Sensitivity", 1.0f);
    invertX_ = GetSubsystem<ConfigManager>()->GetBool("mouse", "InvertX", false);
    invertY_ = GetSubsystem<ConfigManager>()->GetBool("mouse", "InvertY", false);
}

void MouseInput::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    auto input = GetSubsystem<Input>();
    auto ui = GetSubsystem<UI>();
    ControllerInput* controllerInput = GetSubsystem<ControllerInput>();
    for (unsigned i = 0; i < input->GetNumTouches(); ++i) {
        TouchState* state = input->GetTouch(i);
        if (!state->touchedElement_)    // Touch on empty space
        {
            if (state->delta_.x_ || state->delta_.y_)
            {
                float yaw = sensitivityX_ * state->delta_.x_;
                float pitch = sensitivityY_ * state->delta_.y_;
                controllerInput->UpdateYaw(yaw);
                controllerInput->UpdatePitch(pitch);
            }
        }
    }
}

void MouseInput::HandleTouchUI(StringHash eventType, VariantMap& eventData)
{
    using namespace TouchBegin;
    auto input = GetSubsystem<Input>();
    auto ui = GetSubsystem<UI>();

    int touchId = eventData[P_TOUCHID].GetInt();

    ControllerInput* controllerInput = GetSubsystem<ControllerInput>();
    for (unsigned i = 0; i < input->GetNumTouches(); ++i) {
        TouchState* state = input->GetTouch(i);
        if (state->touchID_ == touchId) {
            if (eventType == E_TOUCHBEGIN) {
                UIElement* element = ui->GetElementAt(ui->ConvertSystemToUI(state->position_));
                if (!element) {
                    state->touchedElement_.Reset();
                    continue;
                }
                state->touchedElement_ = element;
            } else if (eventType == E_TOUCHEND) {
                state->touchedElement_.Reset();
            }
        }
    }
}
