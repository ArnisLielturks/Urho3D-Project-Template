#pragma once

#include "BaseInput.h"
#include "../../Config/ConfigFile.h"

class MouseInput : public BaseInput
{
    URHO3D_OBJECT(MouseInput, BaseInput);

public:
    /// Construct.
    MouseInput(Context* context);

    virtual ~MouseInput();
    virtual String GetActionKeyName(int action) override;

    /**
     * Load mouse config from config.cfg file [mouse] block
     */
    virtual void LoadConfig() override;

protected:
    virtual void Init();

private:
    void SubscribeToEvents();

    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
    void HandleKeyUp(StringHash eventType, VariantMap& eventData);
    void HandleMouseMove(StringHash eventType, VariantMap& eventData);
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleTouchUI(StringHash eventType, VariantMap& eventData);
};
