#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/BorderImage.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Core/ProcessUtils.h>
#include "ScreenJoystickInput.h"
#include "../ControllerInput.h"
#include "../ControllerEvents.h"
#include "../../LevelManager.h"
#include "../ControlDefines.h"

ScreenJoystickInput::ScreenJoystickInput(Context* context) :
    BaseInput(context)
{
    SetMinSensitivity(2.0f);

    Init();
    auto* input = GetSubsystem<Input>();
    for (int i = 0; i < input->GetNumJoysticks(); i++) {
        axisPosition_[i] = Vector2::ZERO;
    }
}

ScreenJoystickInput::~ScreenJoystickInput()
{
}

void ScreenJoystickInput::Init()
{
    // Subscribe to global events for camera movement
    SubscribeToEvents();
}

void ScreenJoystickInput::SubscribeToEvents()
{
    SubscribeToEvent(ControllerEvents::E_UI_JOYSTICK_TOGGLE, [&](StringHash eventType, VariantMap& eventData) {
        using namespace ControllerEvents::UIJoystickToggle;
        bool enabled = eventData[P_ENABLED].GetBool();
        if (enabled && GetSubsystem<LevelManager>()->GetCurrentLevel() == "Level") {
            Show();
        } else {
            Hide();
        }
    });
}

void ScreenJoystickInput::Show()
{
    if (!GetSubsystem<ConfigManager>()->GetBool("joystick", "UIJoystick", GetPlatform() == "Android" || GetPlatform() == "iOS")) {
        return;
    }

    if (screenJoystick_) {
        screenJoystick_->Remove();
    }

    auto* input = GetSubsystem<Input>();
    auto cache = GetSubsystem<ResourceCache>();
    auto ui = GetSubsystem<UI>();
    auto style = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
    XMLFile *layout = cache->GetResource<XMLFile>("UI/ScreenJoystick.xml");
    screenJoystick_ = ui->LoadLayout(layout, style);
    if (!screenJoystick_)
        return;

    screenJoystick_->SetSize(ui->GetRoot()->GetSize());
    screenJoystick_->SetName("ScreenJoystick");
    ui->GetRoot()->AddChild(screenJoystick_);

    leftAxis_ = screenJoystick_->GetChild("Axis0", false);
    leftAxis_->SetEnabled(true);
    leftAxisInner_ = leftAxis_->GetChild("InnerButton", false);
    leftAxisInner_->SetEnabled(true);

    settingsButton_ = screenJoystick_->GetChild("Settings", true);
    jumpButton_ = screenJoystick_->GetChild("Jump", true);
    SubscribeToEvent(settingsButton_, E_RELEASED, URHO3D_HANDLER(ScreenJoystickInput, HandleSettings));
//    SubscribeToEvent(jumpButton_, E_PRESSED, URHO3D_HANDLER(ScreenJoystickInput, HandleJumpPress));
//    SubscribeToEvent(jumpButton_, E_RELEASED, URHO3D_HANDLER(ScreenJoystickInput, HandleJumpRelease));

    SubscribeToEvent(E_TOUCHBEGIN, URHO3D_HANDLER(ScreenJoystickInput, HandleScreenJoystickTouch));
    SubscribeToEvent(E_TOUCHEND, URHO3D_HANDLER(ScreenJoystickInput, HandleScreenJoystickTouchEnd));

    URHO3D_LOGINFO("Found Axis0");
    SubscribeToEvent(leftAxisInner_, E_DRAGBEGIN, URHO3D_HANDLER(ScreenJoystickInput, HandleJoystickDrag));
    SubscribeToEvent(leftAxisInner_, E_DRAGMOVE, URHO3D_HANDLER(ScreenJoystickInput, HandleJoystickDrag));
    SubscribeToEvent(leftAxisInner_, E_DRAGEND, URHO3D_HANDLER(ScreenJoystickInput, HandleJoystickDrag));
}

void ScreenJoystickInput::Hide()
{
    if (screenJoystick_) {
        screenJoystick_->Remove();
    }
}

void ScreenJoystickInput::HandleJoystickDrag(StringHash eventType, VariantMap& eventData)
{
    using namespace DragMove;
    BorderImage *borderImage = (BorderImage*)eventData[P_ELEMENT].GetVoidPtr();
    int X = eventData[P_X].GetInt();
    int Y = eventData[P_Y].GetInt();

    const float maxRadiusScaler = 0.2f;
    IntVector2 outerSize = leftAxis_->GetSize();
    IntVector2 innerSize = leftAxisInner_->GetSize();

    IntVector2 buttonOffset_ = (outerSize - innerSize) / 2;
    float innerRadius_ = innerSize.Length() * maxRadiusScaler;

    const IntVector2 &buttonOffset = buttonOffset_;
    const Vector2 centerOffset(buttonOffset);
    const float innerRadius = innerRadius_;

    if (eventType == E_DRAGBEGIN)
    {
        IntVector2 p = borderImage->GetPosition();
        borderImage->SetVar("DELTA", IntVector2(p.x_ - X, p.y_ - Y));
        GetSubsystem<UI>()->SetFocusElement(nullptr);
    }
    else if (eventType == E_DRAGMOVE)
    {
        IntVector2 d = borderImage->GetVar("DELTA").GetIntVector2();
        int iX = X + d.x_;
        int iY = Y + d.y_;

        IntVector2 iPos(iX, iY);
        Vector2 fPos = Vector2(iPos);
        Vector2 seg = fPos - centerOffset;

        if (seg.LengthSquared() > M_EPSILON)
        {
            float dist = Min(seg.Length(), innerRadius);
            Vector2 constrainedPos = centerOffset + seg.Normalized() * dist;
            iPos = IntVector2((int)constrainedPos.x_, (int)constrainedPos.y_);
            seg = constrainedPos - centerOffset;
        }

        borderImage->SetPosition(iPos);

        inputValue_.x_ = (seg.x_ / innerRadius);
        inputValue_.y_ = (seg.y_ / innerRadius);
    }
    else if (eventType == E_DRAGEND)
    {
        borderImage->SetPosition(buttonOffset);
        inputValue_ = Vector2::ZERO;
    }

    auto* controllerInput = GetSubsystem<ControllerInput>();
    if (Abs(inputValue_.x_) < deadzone_) {
        controllerInput->SetActionState(CTRL_LEFT, false);
        controllerInput->SetActionState(CTRL_RIGHT, false);
    } else {
        controllerInput->SetActionState(CTRL_LEFT, inputValue_.x_ < 0, 0, Abs(inputValue_.x_));
        controllerInput->SetActionState(CTRL_RIGHT, inputValue_.x_ > 0, 0, Abs(inputValue_.x_));
    }
    if (Abs(inputValue_.y_) < deadzone_) {
        controllerInput->SetActionState(CTRL_FORWARD, false);
        controllerInput->SetActionState(CTRL_BACK, false);
    } else {
        controllerInput->SetActionState(CTRL_FORWARD, inputValue_.y_ < 0, 0, Abs(inputValue_.y_));
        controllerInput->SetActionState(CTRL_BACK, inputValue_.y_ > 0, 0, Abs(inputValue_.y_));
    }
}

void ScreenJoystickInput::LoadConfig()
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

void ScreenJoystickInput::HandleJumpPress(StringHash eventType, VariantMap &eventData)
{
    auto* controllerInput = GetSubsystem<ControllerInput>();
    controllerInput->SetActionState(CTRL_JUMP, true);
    SendEvent("ShowNotification", "Message", "Jumping true");
}

void ScreenJoystickInput::HandleJumpRelease(StringHash eventType, VariantMap &eventData)
{
    auto* controllerInput = GetSubsystem<ControllerInput>();
    controllerInput->SetActionState(CTRL_JUMP, false);
    SendEvent("ShowNotification", "Message", "Jumping false");
}

void ScreenJoystickInput::HandleSettings(StringHash eventType, VariantMap& eventData)
{
    SendEvent("SettingsButtonPressed");
}

void ScreenJoystickInput::HandleScreenJoystickTouch(StringHash eventType, VariantMap& eventData)
{
    if (!jumpButton_ || !jumpButton_->IsVisible()) {
        return;
    }

    using namespace TouchBegin;
    IntVector2 position(eventData[P_X].GetInt(), eventData[P_Y].GetInt());
    position = GetSubsystem<UI>()->ConvertSystemToUI(position);
    auto element = GetSubsystem<UI>()->GetElementAt(position);

    if (element && element == jumpButton_) {
        auto* controllerInput = GetSubsystem<ControllerInput>();
        controllerInput->SetActionState(CTRL_JUMP, true);
    }
}

void ScreenJoystickInput::HandleScreenJoystickTouchEnd(StringHash eventType, VariantMap& eventData)
{
    using namespace TouchEnd;
    auto* controllerInput = GetSubsystem<ControllerInput>();
    controllerInput->SetActionState(CTRL_JUMP, false);
}