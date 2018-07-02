#include <Urho3D/Urho3DAll.h>
#include "ModLoader.h"

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

    VariantMap data;
    data["Mods"] = modNames;
    SendEvent("ModsLoaded", data);

}

void ModLoader::SubscribeToEvents()
{

}

void ModLoader::Dispose()
{
    _mods.Clear();
}
