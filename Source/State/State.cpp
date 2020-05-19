#include <Urho3D/Core/Context.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/IO/PackageFile.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/IO/Log.h>
#include "State.h"
#include "../Config/ConfigManager.h"
#include "../Global.h"
#include "StateEvents.h"

State::State(Context* context) :
    Object(context)
{
    String directory = GetSubsystem<FileSystem>()->GetUserDocumentsDir() + DOCUMENTS_DIR;
    if (!GetSubsystem<FileSystem>()->DirExists(directory)) {
        GetSubsystem<FileSystem>()->CreateDir(directory);
        URHO3D_LOGINFO("Creating savegame directory " + directory);
    }
    _fileLocation = directory + "/save.json";
    Load();
    SubscribeToEvents();
}

State::~State()
{
}

void State::RegisterObject(Context* context)
{
    context->RegisterFactory<State>();
}

void State::SubscribeToEvents()
{
    SubscribeToEvent(StateEvents::E_SET_STATE_PARAMETER, URHO3D_HANDLER(State, HandleSetParameter));
    SubscribeToEvent(StateEvents::E_INCREMENT_STATE_PARAMETER, URHO3D_HANDLER(State, HandleIncrementParameter));
}

void State::Load()
{
    JSONFile file(context_);
    if (file.LoadFile(_fileLocation)) {
        JSONValue value = file.GetRoot();
        _data = value.GetVariantMap();
        URHO3D_LOGINFO("Savegame loaded");
    }
}

void State::Save()
{
    JSONFile file(context_);
    file.GetRoot().SetVariantMap(_data);
    if (file.SaveFile(_fileLocation)) {
        URHO3D_LOGINFO("Savegame file saved in " + _fileLocation);
    } else {
        URHO3D_LOGERROR("Failed to save state in " + _fileLocation);
    }
}

void State::HandleSetParameter(StringHash eventType, VariantMap& eventData)
{
    using namespace StateEvents::SetStateParameter;
    SetValue(eventData[P_NAME].GetString(), eventData[P_VALUE], eventData[P_SAVE].GetBool());
}

void State::HandleIncrementParameter(StringHash eventType, VariantMap& eventData)
{
    using namespace StateEvents::IncrementStateParameter;
    String name = eventData[P_NAME].GetString();
    int value = GetValue(name).GetInt();
    SetValue(name, value + eventData[P_AMOUNT].GetInt(), eventData[P_SAVE].GetBool());
}

void State::SetValue(const String& name, const Variant& value, bool save)
{
    URHO3D_LOGINFO("Updating state parameter: " + name);
    _data[name] = value;
    if (save) {
        Save();
    }
}

const Variant& State::GetValue(const String& name)
{
    return _data[name];
}
