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

	void Reload();

protected:
    virtual void Init();

private:

    void SubscribeToEvents();
	void SubscribeConsoleCommands();
	void HandleReload(StringHash eventType, VariantMap& eventData);

    void HandleReloadScript(StringHash eventType, VariantMap& eventData);

    /**
     * List of all the loaded mods
     */
    Vector<SharedPtr<ScriptFile>> _mods;

    HashMap<String, SharedPtr<ScriptFile>> _scriptMap;
};