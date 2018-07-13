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

    // Load each of the *.as files and launch their Start() method
    for (auto it = result.Begin(); it != result.End(); ++it) {
        URHO3D_LOGINFO("Loading mod: " + (*it));
        SharedPtr<ScriptFile> scriptFile(GetSubsystem<ResourceCache>()->GetResource<ScriptFile>(GetSubsystem<FileSystem>()->GetProgramDir() + "/Data/Mods/" + (*it)));
        if (scriptFile && scriptFile->Execute("void Start()")) {
            URHO3D_LOGINFO("Mod " + (*it) + " succesfully loaded!");
        }
        _mods.Push(scriptFile);
        _scriptMap["Mods/" + (*it)] = scriptFile;
    }

    SubscribeToEvent(E_FILECHANGED, URHO3D_HANDLER(ModLoader, HandleReloadScript));

    CheckAllMods();

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

void ModLoader::CheckAllMods()
{
    Vector<String> result;
    GetSubsystem<FileSystem>()->ScanDir(result, GetSubsystem<FileSystem>()->GetProgramDir() + String("/Data/Mods"), String("*.as"), SCAN_FILES, false);
    StringVector modNames;
    for (auto it = result.Begin(); it != result.End(); ++it) {
        modNames.Push((*it));
    }
    VariantMap data = GetEventDataMap();
    data["Mods"] = modNames;
    SendEvent("ModsLoaded", data);
}

void ModLoader::HandleReloadScript(StringHash eventType, VariantMap& eventData)
{
    using namespace FileChanged;
    String filename = eventData[P_RESOURCENAME].GetString();
    if (_scriptMap.Contains(filename)) {
        URHO3D_LOGINFO("Reloading mod " + filename);
        if (_scriptMap[filename]->GetFunction("void Stop()")) {
            _scriptMap[filename]->Execute("void Stop()");
        }
        auto* cache = GetSubsystem<ResourceCache>();
        cache->ReleaseResource<ScriptFile>(filename, true);
        _scriptMap[filename].Reset();

        _scriptMap[filename] = cache->GetResource<ScriptFile>(filename);
        if (!_scriptMap[filename]) {
            URHO3D_LOGWARNING("Mod '" + filename + "' removed!");
            _scriptMap.Erase(filename);

           CheckAllMods();

        } else {
            if (_scriptMap[filename]->GetFunction("void Start()")) {
                _scriptMap[filename]->Execute("void Start()");
            }
        }
    } else {
        Vector<String> result;
        // Scan Data/Mods directory for all *.as files
        GetSubsystem<FileSystem>()->ScanDir(result, GetSubsystem<FileSystem>()->GetProgramDir() + String("/Data/Mods"), String("*.as"), SCAN_FILES, false);
		VariantMap loadedMods;
        for (auto it = result.Begin(); it != result.End(); ++it) {
			loadedMods["Mods/" + (*it)] = true;
            // Check if reloaded file is in the mods directory
            if ("Mods/" + (*it) == filename) {
                auto* cache = GetSubsystem<ResourceCache>();
                // Try to load the script file
                _scriptMap[filename] = cache->GetResource<ScriptFile>(filename);
                if (!_scriptMap[filename]) {
                    URHO3D_LOGWARNING("Mod '" + filename + "' can't be loaded!");
                    _scriptMap.Erase(filename);
                } else {
                    if (_scriptMap[filename]->GetFunction("void Start()")) {
                        _scriptMap[filename]->Execute("void Start()");
                    }
                }
            }
        }
		for (auto it = _scriptMap.Begin(); it != _scriptMap.End(); ++it) {
			if (!loadedMods.Contains((*it).first_)) {
				if (_scriptMap[(*it).first_]->GetFunction("void Stop()")) {
					_scriptMap[(*it).first_]->Execute("void Stop()");
				}
				URHO3D_LOGWARNING("Unloading mod '" + (*it).first_ + "'");
				_scriptMap.Erase((*it).first_);
			}
		}
        CheckAllMods();
    }
}
