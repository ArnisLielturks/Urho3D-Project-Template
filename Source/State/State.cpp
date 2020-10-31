#include <Urho3D/Core/Context.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/IO/PackageFile.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/IO/Log.h>
#include "State.h"
#include "../Config/ConfigManager.h"
#include "../Globals/Settings.h"
#include "StateEvents.h"

#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#include <emscripten/bind.h>
#endif

State::State(Context* context) :
    Object(context)
{
    String directory = GetSubsystem<FileSystem>()->GetUserDocumentsDir() + DOCUMENTS_DIR;
    if (!GetSubsystem<FileSystem>()->DirExists(directory)) {
        GetSubsystem<FileSystem>()->CreateDir(directory);
        URHO3D_LOGINFO("Creating savegame directory " + directory);
    }
    fileLocation_ = directory + "/save.json";
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
#ifndef __EMSCRIPTEN__
    if (file.LoadFile(fileLocation_)) {
        JSONValue value = file.GetRoot();
        data_ = value.GetVariantMap();
        URHO3D_LOGINFO("Savegame loaded");
    }
#endif
}

void State::Save()
{
    JSONFile file(context_);
    file.GetRoot().SetVariantMap(data_);
#ifndef __EMSCRIPTEN__
    if (file.SaveFile(fileLocation_)) {
        URHO3D_LOGINFO("Savegame file saved in " + fileLocation_);
    } else {
        URHO3D_LOGERROR("Failed to save state in " + fileLocation_);
    }
#else
//    EM_ASM({
//        console.log('Storing state', $0);
//        window.localStorage.setItem('name', 'Obaseki Nosa');
//    }, file.ToString().CString());
#endif
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
    data_[name] = value;
    if (save) {
        Save();
    }
}

const Variant& State::GetValue(const String& name)
{
    return data_[name];
}
