#pragma once

#include <Urho3D/Urho3DAll.h>

class BaseWindow : public Object
{
    URHO3D_OBJECT(BaseWindow, Object);
public:
    BaseWindow(Context* context);

    virtual ~BaseWindow();

    virtual void Init();

    void SetActive(bool active);

    bool IsActive();

private:
    void SubscribeToBaseEvents()
    {
    }

    virtual void Create() {};

protected:

    virtual void Dispose()
    {
    }

    bool _active;
};