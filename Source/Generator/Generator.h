#pragma once

#include <Urho3D/Core/Object.h>
#include <Urho3D/Container/Vector.h>
#include <Urho3D/UI/Text.h>

using namespace Urho3D;

class Generator : public Object
{
    URHO3D_OBJECT(Generator, Object);

public:
    /// Construct.
    Generator(Context* context);

    virtual ~Generator();

private:
    virtual void Init();

    /**
     * Subscribe to notification events
     */
    void SubscribeToEvents();
};