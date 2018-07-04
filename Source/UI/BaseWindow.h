#pragma once

#include <Urho3D/Urho3DAll.h>

class BaseWindow : public Object
{
    URHO3D_OBJECT(BaseWindow, Object);
public:
    BaseWindow(Context* context, IntVector2 size = IntVector2(300, 200)) :
        Object(context) {
        SubscribeToBaseEvents();
        _size = size;
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
        _base->SetSize(_size);
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
    IntVector2 _size;
};