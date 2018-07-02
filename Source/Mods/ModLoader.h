#pragma once

#include <Urho3D/Urho3DAll.h>

using namespace Urho3D;

class ModLoader : public Object
{
    URHO3D_OBJECT(ModLoader, Object);

public:
    /// Construct.
    ModLoader(Context* context);

    virtual ~ModLoader();

    void Create();

    void Dispose();

protected:
    virtual void Init();

private:

    void SubscribeToEvents();

    /**
     * List of all the loaded mods
     */
    Vector<SharedPtr<ScriptFile>> _mods;
};