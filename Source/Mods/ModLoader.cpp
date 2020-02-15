#if defined(URHO3D_LUA) || defined(URHO3D_ANGELSCRIPT)
#include <Urho3D/Core/Context.h>
#ifdef URHO3D_ANGELSCRIPT
#include <Urho3D/AngelScript/Script.h>
#endif
#ifdef URHO3D_LUA
#include <Urho3D/LuaScript/LuaScript.h>
#endif
#include <Urho3D/Resource/ResourceEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/IO/PackageFile.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Engine/DebugHud.h>
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
        #ifdef URHO3D_ANGELSCRIPT
        auto asScript = new Script(context_);
        context_->RegisterSubsystem(asScript);
        asScript->SetExecuteConsoleCommands(false);
        #endif

        #ifdef URHO3D_LUA
        auto luaScript = new LuaScript(context_);
        context_->RegisterSubsystem(luaScript);
        luaScript->SetExecuteConsoleCommands(false);
        #endif
        Create();
        SubscribeToEvents();
    }
}

void ModLoader::Create()
{
    #ifdef URHO3D_ANGELSCRIPT
    LoadASMods();
    #endif

    #ifdef URHO3D_LUA
    LoadLuaMods();
    #endif

    SubscribeToEvent(E_FILECHANGED, URHO3D_HANDLER(ModLoader, HandleReloadScript));

    CheckAllMods();

}

void ModLoader::LoadASMods()
{
    #ifdef URHO3D_ANGELSCRIPT
    Vector<String> result;

    // Scan Data/Mods directory for all *.as files
    GetSubsystem<FileSystem>()->ScanDir(result, GetSubsystem<FileSystem>()->GetProgramDir() + String("Data/Mods"), String("*.as"), SCAN_FILES, false);
    URHO3D_LOGINFO("Total AS mods found: " + String(result.Size()));

    auto packageFiles = GetSubsystem<ResourceCache>()->GetPackageFiles();
    GetSubsystem<DebugHud>()->SetAppStats("Package files", packageFiles.Size());
    for (auto it = packageFiles.Begin(); it != packageFiles.End(); ++it) {
        auto files = (*it)->GetEntryNames();
        for (auto it2 = files.Begin(); it2 != files.End(); ++it2) {
            if ((*it2).StartsWith("Mods/") && (*it2).EndsWith(".as") && (*it2).Split('/')  .Size() == 2) {
                result.Push((*it2).Split('/').At(1));
            }
        }
    }

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
    #endif
}

void ModLoader::LoadLuaMods()
{
    #ifdef URHO3D_LUA
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    // Scan Data/Mods directory for all *.as files
    GetSubsystem<FileSystem>()->ScanDir(_luaMods, GetSubsystem<FileSystem>()->GetProgramDir() + String("Data/Mods"), String("*.lua"), SCAN_FILES, false);
    URHO3D_LOGINFO("Total LUA mods found: " + String(_luaMods.Size()));

    auto packageFiles = GetSubsystem<ResourceCache>()->GetPackageFiles();
    for (auto it = packageFiles.Begin(); it != packageFiles.End(); ++it) {
        auto files = (*it)->GetEntryNames();
        for (auto it2 = files.Begin(); it2 != files.End(); ++it2) {
            if ((*it2).StartsWith("Mods/") && (*it2).EndsWith(".lua") && (*it2).Split('/')  .Size() == 2) {
                _luaMods.Push((*it2));
            }
        }
    }

    // Load each of the *.lua files and launch their Start() method
    for (auto it = _luaMods.Begin(); it != _luaMods.End(); ++it) {
        URHO3D_LOGINFO("Loading mod: " + (*it));
        auto luaScript = GetSubsystem<LuaScript>();
        if (luaScript->ExecuteFile("Mods/" + (*it)))
        {
            String scriptNameTrimmed = (*it).Substring(0, (*it).Length() - 4);
            scriptNameTrimmed.Replace("Mods/", "");
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

    GetSubsystem<DebugHud>()->SetAppStats("Total LUA mods loaded", _luaMods.Size());
    #endif
}

void ModLoader::SubscribeToEvents()
{
    SubscribeConsoleCommands();
    SubscribeToEvent("HandleReloadMods", URHO3D_HANDLER(ModLoader, HandleReload));
}

void ModLoader::Dispose()
{
    #ifdef URHO3D_ANGELSCRIPT
    _asMods.Clear();
    #endif
}

void ModLoader::Reload()
{
    #ifdef URHO3D_ANGELSCRIPT
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
    #endif
}

void ModLoader::SubscribeConsoleCommands()
{
    using namespace MyEvents::ConsoleCommandAdd;

    VariantMap& data = GetEventDataMap();
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
    #ifdef URHO3D_ANGELSCRIPT
    result.Reserve(_asMods.Size());
    for (auto it = _asMods.Begin(); it != _asMods.End(); ++it) {
        result.Push((*it)->GetName());
    }
    #endif
    #ifdef URHO3D_LUA
    for (auto it = _luaMods.Begin(); it != _luaMods.End(); ++it) {
        result.Push((*it));
    }
    #endif
    VariantMap& data = GetEventDataMap();
    data["Mods"] = result;
    SendEvent("ModsLoaded", data);
}

void ModLoader::HandleReloadScript(StringHash eventType, VariantMap& eventData)
{
    #ifdef URHO3D_ANGELSCRIPT
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
    #endif
}
#endif
