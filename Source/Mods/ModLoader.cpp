#include <Urho3D/Urho3DAll.h>
#include "ModLoader.h"
#include "../MyEvents.h"
#include "../Config/ConfigManager.h"

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
    if (GetSubsystem<ConfigManager>()->GetBool("game", "LoadMods", true)) {
        auto asScript = new Script(context_);
        context_->RegisterSubsystem(asScript);
        asScript->SetExecuteConsoleCommands(false);
        auto luaScript = new LuaScript(context_);
        context_->RegisterSubsystem(luaScript);
        luaScript->SetExecuteConsoleCommands(false);
        Create();
        SubscribeToEvents();
    }
}

void ModLoader::Create()
{
    LoadASMods();
    LoadLuaMods();

    SubscribeToEvent(E_FILECHANGED, URHO3D_HANDLER(ModLoader, HandleReloadScript));

    CheckAllMods();

}

void ModLoader::LoadASMods()
{
    Vector<String> result;

    // Scan Data/Mods directory for all *.as files
    GetSubsystem<FileSystem>()->ScanDir(result, GetSubsystem<FileSystem>()->GetProgramDir() + String("/Data/Mods"), String("*.as"), SCAN_FILES, false);
    URHO3D_LOGINFO("Total AS mods found: " + String(result.Size()));

#ifdef __ANDROID__
    result.Push("Debugger.as");
    result.Push("GameMode.as");
    result.Push("LevelLoader.as");
    result.Push("LoadingScreen.as");
    result.Push("LoadStepImitator.as");
    result.Push("LogoRotate.as");
    result.Push("Skybox.as");
#endif

    // Load each of the *.as files and launch their Start() method
    for (auto it = result.Begin(); it != result.End(); ++it) {
        URHO3D_LOGINFO("Loading mod: " + (*it));
        SharedPtr<ScriptFile> scriptFile(GetSubsystem<ResourceCache>()->GetResource<ScriptFile>("Mods/" + (*it)));
        if (scriptFile) {
            _asMods.Push(scriptFile);
            _asScriptMap["Mods/" + (*it)] = scriptFile;
        }
    }

    URHO3D_LOGINFO("Initializing all loaded AS mods");
    for (auto it = _asMods.Begin(); it != _asMods.End(); ++it) {
        if ((*it)->Execute("void Start()")) {
        }
    }

    GetSubsystem<DebugHud>()->SetAppStats("Total AS mods loaded", _asMods.Size());
}

void ModLoader::LoadLuaMods()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Vector<String> result;

    // Scan Data/Mods directory for all *.as files
    GetSubsystem<FileSystem>()->ScanDir(result, GetSubsystem<FileSystem>()->GetProgramDir() + String("/Data/Mods"), String("*.lua"), SCAN_FILES, false);
    URHO3D_LOGINFO("Total LUA mods found: " + String(result.Size()));

    // Load each of the *.as files and launch their Start() method
    for (auto it = result.Begin(); it != result.End(); ++it) {
        URHO3D_LOGINFO("Loading mod: " + (*it));
        auto luaScript = GetSubsystem<LuaScript>();
        if (luaScript->ExecuteFile(GetSubsystem<FileSystem>()->GetProgramDir() + "/Data/Mods/" + (*it)))
        {
            String scriptNameTrimmed = (*it).Substring(0, (*it).Length() - 4);
            URHO3D_LOGINFO("Loading LUA method " + scriptNameTrimmed + "Start");
            luaScript->ExecuteFunction(scriptNameTrimmed + "Start");
        }
        /*SharedPtr<LuaScript> scriptFile(GetSubsystem<ResourceCache>()->GetResource<LuaScript>(GetSubsystem<FileSystem>()->GetProgramDir() + "/Data/Mods/" + (*it)));
        if (scriptFile && scriptFile->ExecuteFunction("Start")) {
            URHO3D_LOGINFO("Mod " + (*it) + " succesfully loaded!");
        }*/
        //_mods.Push(scriptFile);
        //_scriptMap["Mods/" + (*it)] = scriptFile;
    }

    GetSubsystem<DebugHud>()->SetAppStats("Total LUA mods loaded", result.Size());
}

void ModLoader::SubscribeToEvents()
{
	SubscribeConsoleCommands();
	SubscribeToEvent("HandleReloadMods", URHO3D_HANDLER(ModLoader, HandleReload));
}

void ModLoader::Dispose()
{
    _asMods.Clear();
}

void ModLoader::Reload()
{
    if (GetSubsystem<ConfigManager>()->GetBool("game", "LoadMods", true)) {
        //_mods.Clear();
        for (auto it = _asMods.Begin(); it != _asMods.End(); ++it) {
            if ((*it)) {
                (*it)->Execute("void Stop()");
                (*it)->Execute("void Start()");
            }
        }
        CheckAllMods();
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
    if (!GetSubsystem<ConfigManager>()->GetBool("game", "LoadMods", true)) {
        return;
    }

    using namespace FileChanged;
    String filename = eventData[P_RESOURCENAME].GetString();
    if (!filename.Contains(".as") && !filename.Contains(".lua")) {
        // We don't care about resources other than .as and .lua
        return;
    }

    if (_asScriptMap.Contains(filename)) {
        URHO3D_LOGINFO("Reloading mod " + filename);
        if (_asScriptMap[filename]->GetFunction("void Stop()")) {
            _asScriptMap[filename]->Execute("void Stop()");
        }
        auto* cache = GetSubsystem<ResourceCache>();
        cache->ReleaseResource<ScriptFile>(filename, true);
        _asScriptMap[filename].Reset();

        _asScriptMap[filename] = cache->GetResource<ScriptFile>(filename);
        if (!_asScriptMap[filename]) {
            URHO3D_LOGWARNING("Mod '" + filename + "' removed!");
            _asScriptMap.Erase(filename);

           CheckAllMods();

        } else {
            if (_asScriptMap[filename]->GetFunction("void Start()")) {
                _asScriptMap[filename]->Execute("void Start()");
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
                _asScriptMap[filename] = cache->GetResource<ScriptFile>(filename);
                if (!_asScriptMap[filename]) {
                    URHO3D_LOGWARNING("Mod '" + filename + "' can't be loaded!");
                    _asScriptMap.Erase(filename);
                } else {
                    if (_asScriptMap[filename]->GetFunction("void Start()")) {
                        _asScriptMap[filename]->Execute("void Start()");
                    }
                }
            }
        }
		for (auto it = _asScriptMap.Begin(); it != _asScriptMap.End(); ++it) {
			if (!loadedMods.Contains((*it).first_)) {
				if (_asScriptMap[(*it).first_]->GetFunction("void Stop()")) {
                    _asScriptMap[(*it).first_]->Execute("void Stop()");
				}
				URHO3D_LOGWARNING("Unloading mod '" + (*it).first_ + "'");
                _asScriptMap.Erase((*it).first_);
			}
		}
        CheckAllMods();
    }
}
