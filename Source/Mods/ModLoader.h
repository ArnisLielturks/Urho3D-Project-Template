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

protected:
    virtual void Init();

private:

	/**
    * Load and create all scripts
    */
	void Create();

	/**
	 * Destroy all created scripts
	 */
	void Dispose();

	/**
	 * Reload changed scripts
	 */
	void Reload();

	/**
	 * Load all .as mods from Data/Mods directory
	 */
    void LoadASMods();

	/**
     * Load all .lua mods from Data/Mods directory
     */
    void LoadLuaMods();


    void SubscribeToEvents();

    /**
     * Subscribe for console commands to support script reloading
     */
	void SubscribeConsoleCommands();

	/**
	 * Handle script reload command
	 */
	void HandleReload(StringHash eventType, VariantMap& eventData);

	/**
	 * Handle single script reload
	 */
    void HandleReloadScript(StringHash eventType, VariantMap& eventData);

    /**
     * Generate list of mods, sent out events
     */
    void CheckAllMods();

    /**
     * List of all the loaded mods
     */
    Vector<SharedPtr<ScriptFile>> _asMods;

    /**
     * Script location, script object map
     */
    HashMap<String, SharedPtr<ScriptFile>> _asScriptMap;
};