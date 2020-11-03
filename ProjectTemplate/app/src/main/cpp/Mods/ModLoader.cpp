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
#include "../Config/ConfigManager.h"
#include "../Console/ConsoleHandlerEvents.h"

using namespace ConsoleHandlerEvents;

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
    if (GetSubsystem<DebugHud>()) {
        GetSubsystem<DebugHud>()->SetAppStats("Package files", packageFiles.Size());
    }
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
            asMods_.Push(scriptFile);
            asScriptMap_["Mods/" + (*it)] = scriptFile;
        }
    }

    URHO3D_LOGINFO("Initializing all loaded AS mods");
    for (auto it = asMods_.Begin(); it != asMods_.End(); ++it) {
        if ((*it)->Execute("void Start()")) {
        }
    }

    if (GetSubsystem<DebugHud>()) {
        GetSubsystem<DebugHud>()->SetAppStats("Total AS mods loaded", asMods_.Size());
    }
    #endif
}

void ModLoader::LoadLuaMods()
{
    #ifdef URHO3D_LUA
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    // Scan Data/Mods directory for all *.as files
    GetSubsystem<FileSystem>()->ScanDir(luaMods_, GetSubsystem<FileSystem>()->GetProgramDir() + String("Data/Mods"), String("*.lua"), SCAN_FILES, false);
    URHO3D_LOGINFO("Total LUA mods found: " + String(luaMods_.Size()));

    auto packageFiles = GetSubsystem<ResourceCache>()->GetPackageFiles();
    for (auto it = packageFiles.Begin(); it != packageFiles.End(); ++it) {
        auto files = (*it)->GetEntryNames();
        for (auto it2 = files.Begin(); it2 != files.End(); ++it2) {
            if ((*it2).StartsWith("Mods/") && (*it2).EndsWith(".lua") && (*it2).Split('/')  .Size() == 2) {
                luaMods_.Push((*it2).Split('/').At(1));
            }
        }
    }

    // Load each of the *.lua files and launch their Start() method
    for (auto it = luaMods_.Begin(); it != luaMods_.End(); ++it) {
        URHO3D_LOGINFO("Loading mod: " + (*it));
        auto luaScript = GetSubsystem<LuaScript>();
        if (luaScript->ExecuteFile("Mods/" + (*it)))
        {
            String scriptNameTrimmed = (*it).Substring(0, (*it).Length() - 4);
            scriptNameTrimmed.Replace("Mods/", "");
            URHO3D_LOGINFO("Loading LUA method " + scriptNameTrimmed + "Start");
            luaScript->ExecuteFunction(scriptNameTrimmed + "Start");
        }
    }

    if (GetSubsystem<DebugHud>()) {
        GetSubsystem<DebugHud>()->SetAppStats("Total LUA mods loaded", luaMods_.Size());
    }
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
    asMods_.Clear();
    #endif
}

void ModLoader::Reload()
{
    #ifdef URHO3D_ANGELSCRIPT
    if (GetSubsystem<ConfigManager>()->GetBool("game", "LoadMods", true)) {
        for (auto it = asMods_.Begin(); it != asMods_.End(); ++it) {
            if ((*it)) {
                (*it)->Execute("void Stop()");
                (*it)->Execute("void Start()");
            }
        }
        CheckAllMods();
    }
    #endif
}

#include <string>
#include <Urho3D/IO/MemoryBuffer.h>
void ModLoader::SubscribeConsoleCommands()
{
    using namespace ConsoleCommandAdd;

    VariantMap& data = GetEventDataMap();
    data[P_NAME] = "reload_mods";
    data[P_EVENT] = "HandleReloadMods";
    data[P_DESCRIPTION] = "Reload all scripts";
    SendEvent(E_CONSOLE_COMMAND_ADD, data);

    SendEvent(
            E_CONSOLE_COMMAND_ADD,
            ConsoleCommandAdd::P_NAME, "mod_test",
            ConsoleCommandAdd::P_EVENT, "#mod_test",
            ConsoleCommandAdd::P_DESCRIPTION, "mod_test",
            ConsoleCommandAdd::P_OVERWRITE, true
    );
    SubscribeToEvent("#mod_test", [&](StringHash eventType, VariantMap& eventData) {
        StringVector params = eventData["Parameters"].GetStringVector();

        if (params.Size() >= 2) {
            auto cache = GetSubsystem<ResourceCache>();
            auto file = cache->GetFile(params[1]);
            if (file) {
                String content;
                while(!file->IsEof()) {
                    content += file->ReadLine() + "\n";
                }
                URHO3D_LOGINFOF("Reading file %s", content.CString());
            }
//            String filename = params[1];
//            SharedPtr<ScriptFile> file = SharedPtr<ScriptFile>(GetSubsystem<ResourceCache>()->GetResource<ScriptFile>(filename));
//            if (!file) {
//                file = SharedPtr<ScriptFile>(new ScriptFile(context_));
//                file->SetName(filename);
//                GetSubsystem<ResourceCache>()->AddManualResource(file);
//                URHO3D_LOGINFOF("Creating new manual resource %s", filename.CString());
//            }
//            String data;
//            for (int i = 2; i < params.Size(); i++) {
//                data += " " + params[i];
//            }
//            std::string content = data.CString();
//            MemoryBuffer buffer((const void*)content.c_str(), content.length());
//            file->Load(buffer);

//            auto luaScript = GetSubsystem<LuaScript>();
//            luaScript->LoadRawFile()

//            Resource* res = GetSubsystem<ResourceCache>()->GetResource<Resource>(filename);
        }
//        std::string filename = "aa.as";
//        std::string content = "a= 0;";
//        URHO3D_LOGINFOF("Adding new resource filename=%s ; content=%s", filename.c_str(), content.c_str());

//        using namespace Urho3D;
//        Context* context = context_;
//
//        SharedPtr<ScriptFile> file = SharedPtr<ScriptFile>(GetSubsystem<ResourceCache>()->GetResource<ScriptFile>(filename.c_str()));
//        URHO3D_LOGINFO(">>1");
//        if (!file) {
//            file = SharedPtr<ScriptFile>(new ScriptFile(context));
//            file->SetName(filename.c_str());
//            GetSubsystem<ResourceCache>()->AddManualResource(file);
//            URHO3D_LOGINFOF("Creating new manual resource %s", filename.c_str());
//        }
//
//        URHO3D_LOGINFO(">>2");
//        MemoryBuffer buffer((const void*)content.c_str(), (unsigned)content.length());
//        URHO3D_LOGINFO(">>2.1");
//        if (file) {
//            file->Load(buffer);
//            URHO3D_LOGINFO("File exists");
//        } else {
//            URHO3D_LOGINFO("File doesnt exists");
//        }
//
//        URHO3D_LOGINFO(">>3");
    });
}

void ModLoader::HandleReload(StringHash eventType, VariantMap& eventData)
{
    Reload();
}

void ModLoader::CheckAllMods()
{
    Vector<String> result;
    #ifdef URHO3D_ANGELSCRIPT
    result.Reserve(asMods_.Size());
    for (auto it = asMods_.Begin(); it != asMods_.End(); ++it) {
        result.Push((*it)->GetName());
    }
    #endif
    #ifdef URHO3D_LUA
    for (auto it = luaMods_.Begin(); it != luaMods_.End(); ++it) {
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

    if (asScriptMap_.Contains(filename)) {
        URHO3D_LOGINFO("Reloading mod " + filename);
        if (asScriptMap_[filename]->GetFunction("void Stop()")) {
            asScriptMap_[filename]->Execute("void Stop()");
        }
        auto* cache = GetSubsystem<ResourceCache>();
        cache->ReleaseResource<ScriptFile>(filename, true);
        asScriptMap_[filename].Reset();

        asScriptMap_[filename] = cache->GetResource<ScriptFile>(filename);
        if (!asScriptMap_[filename]) {
            URHO3D_LOGWARNING("Mod '" + filename + "' removed!");
            asScriptMap_.Erase(filename);

           CheckAllMods();

        } else {
            if (asScriptMap_[filename]->GetFunction("void Start()")) {
                asScriptMap_[filename]->Execute("void Start()");
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
                asScriptMap_[filename] = cache->GetResource<ScriptFile>(filename);
                if (!asScriptMap_[filename]) {
                    URHO3D_LOGWARNING("Mod '" + filename + "' can't be loaded!");
                    asScriptMap_.Erase(filename);
                } else {
                    if (asScriptMap_[filename]->GetFunction("void Start()")) {
                        asScriptMap_[filename]->Execute("void Start()");
                    }
                }
            }
        }
        for (auto it = asScriptMap_.Begin(); it != asScriptMap_.End(); ++it) {
            if (!loadedMods.Contains((*it).first_)) {
                if (asScriptMap_[(*it).first_]->GetFunction("void Stop()")) {
                    asScriptMap_[(*it).first_]->Execute("void Stop()");
                }
                URHO3D_LOGWARNING("Unloading mod '" + (*it).first_ + "'");
                asScriptMap_.Erase((*it).first_);
            }
        }
        CheckAllMods();
    }
    #endif
}
#endif
