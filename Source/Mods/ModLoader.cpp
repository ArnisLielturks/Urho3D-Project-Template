#include <Urho3D/Urho3DAll.h>
#include "ModLoader.h"
#include "../MyEvents.h"

/// Construct.
ModLoader::ModLoader(Context* context) :
    Object(context)
{
    Init();
}

ModLoader::~ModLoader()
{
}

void ModLoader::Init()
{
    context_->RegisterSubsystem(new Script(context_));
    Create();
    SubscribeToEvents();
}

void ModLoader::Create()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Vector<String> result;

    // Scan Data/Mods directory for all *.as files
    GetSubsystem<FileSystem>()->ScanDir(result, GetSubsystem<FileSystem>()->GetProgramDir() + String("/Data/Mods"), String("*.as"), SCAN_FILES, false);
    URHO3D_LOGINFO("Total mods found: " + String(result.Size()));

    StringVector modNames;

    // Load each of the *.as files and launch their Start() method
    for (auto it = result.Begin(); it != result.End(); ++it) {
        URHO3D_LOGINFO("Loading mod: " + (*it));
        SharedPtr<ScriptFile> scriptFile(GetSubsystem<ResourceCache>()->GetResource<ScriptFile>(GetSubsystem<FileSystem>()->GetProgramDir() + "/Data/Mods/" + (*it)));
        if (scriptFile && scriptFile->Execute("void Start()")) {
            URHO3D_LOGINFO("Mod " + (*it) + " succesfully loaded!");
        }
        _mods.Push(scriptFile);
        modNames.Push((*it));
    }

    VariantMap data = GetEventDataMap();
    data["Mods"] = modNames;
    SendEvent("ModsLoaded", data);

}

void ModLoader::SubscribeToEvents()
{
	SubscribeConsoleCommands();
	SubscribeToEvent("HandleReloadMods", URHO3D_HANDLER(ModLoader, HandleReload));
}

void ModLoader::Dispose()
{
    _mods.Clear();
}

void ModLoader::Reload()
{
	//_mods.Clear();
	for (auto it = _mods.Begin(); it != _mods.End(); ++it) {
		(*it)->RemoveEventHandlers();
		(*it)->Execute("void Stop()");
		(*it)->Execute("void Start()");
	}
}

void ModLoader::SubscribeConsoleCommands()
{
	using namespace MyEvents::ConsoleCommandAdd;

	VariantMap data = GetEventDataMap();
	data[P_NAME] = "reload_mods";
	data[P_EVENT] = "HandleReloadMods";
	data[P_DESCRIPTION] = "Reload all scripts";
	SendEvent(MyEvents::E_CONSOLE_COMMAND_ADD, data);
}

void ModLoader::HandleReload(StringHash eventType, VariantMap& eventData)
{
	Reload();
}
