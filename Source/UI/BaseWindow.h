#pragma once

#include <Urho3D/Urho3DAll.h>

class BaseWindow : public Object
{
    URHO3D_OBJECT(BaseWindow, Object);
public:
    BaseWindow(Context* context) :
        Object(context) {
        SubscribeToBaseEvents();
        Init();
    }

    virtual ~BaseWindow()
    {
        Dispose();
    }

    virtual void Init() {
        UI* ui = GetSubsystem<UI>();
        _base = ui->GetRoot()->CreateChild<Window>();
        _base->SetStyleAuto();
        _base->SetSize(IntVector2(500, 500));
        _base->SetAlignment(HA_CENTER, VA_CENTER);
    };

private:
    void SubscribeToBaseEvents()
    {
    }

    virtual void Create() {};

protected:

    virtual void Dispose()
    {
        if (_base) {
            _base->Remove();
        }
    }

    SharedPtr<Window> _base;
};