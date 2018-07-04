#include <Urho3D/AngelScript/ScriptFile.h>
#include <Urho3D/AngelScript/Script.h>
#include <Urho3D/Core/Main.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/ResourceEvents.h>
#include <Urho3D/Engine/Console.h>

#include "BaseApplication.h"
#include <Urho3D/UI/Button.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/DebugNew.h>
#include <Urho3D/Math/MathDefs.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/AngelScript/ScriptAPI.h>


URHO3D_DEFINE_APPLICATION_MAIN(BaseApplication);

BaseApplication::BaseApplication(Context* context) :
    Application(context),
    commandLineRead_(false)
{
    context_->RegisterFactory<LevelManager>();
    context_->RegisterFactory<Message>();
    context_->RegisterFactory<Notifications>();
    context_->RegisterFactory<Achievements>();
    context_->RegisterFactory<ModLoader>();
    context_->RegisterFactory<WindowManager>();
}

void BaseApplication::Setup()
{
    // Web platform depends on the resource system to read any data files. Skip parsing the command line file now
    // and try later when the resource system is live
    // Read command line from a file if no arguments given. This is primarily intended for mobile platforms.
    // Note that the command file name uses a hardcoded path that does not utilize the resource system
    // properly (including resource path prefix), as the resource system is not yet initialized at this point
    FileSystem* filesystem = GetSubsystem<FileSystem>();
    const String commandFileName = filesystem->GetProgramDir() + "Data/CommandLine.txt";
    if (filesystem->FileExists(commandFileName)) {
        SharedPtr<File> commandFile(new File(context_, commandFileName));
        if (commandFile->IsOpen()) {
            commandLineRead_ = true;
            //while (!commandFile->IsEof()) {
                String commandLine = commandFile->ReadLine();
                URHO3D_LOGINFO("Line: " + commandLine);
                ParseArguments(commandLine, false);
                // Reparse engine startup parameters now
                engineParameters_ = Engine::ParseParameters(GetArguments());
            //}
            commandFile->Close();
        }
    }

    // Show usage if not found
    // Use the script file name as the base name for the log file
    engineParameters_[EP_LOG_NAME] = "EmptyProject.log";// filesystem->GetAppPreferencesDir("urho3d", "logs") + GetFileNameAndExtension(scriptFileName_) + ".log";
    engineParameters_[EP_FULL_SCREEN] = false;
    engineParameters_[EP_WINDOW_WIDTH] = 800;
    engineParameters_[EP_WINDOW_HEIGHT] = 600;
    engineParameters_[EP_LOG_LEVEL] = LOG_INFO;

    // Construct a search path to find the resource prefix with two entries:
    // The first entry is an empty path which will be substituted with program/bin directory -- this entry is for binary when it is still in build tree
    // The second and third entries are possible relative paths from the installed program/bin directory to the asset directory -- these entries are for binary when it is in the Urho3D SDK installation location
    if (!engineParameters_.Contains(EP_RESOURCE_PREFIX_PATHS)) {
        engineParameters_[EP_RESOURCE_PREFIX_PATHS] = ";../share/Resources;../share/Urho3D/Resources";
    }
}

void BaseApplication::Start()
{
    UI* ui = GetSubsystem<UI>();
    auto* cache = GetSubsystem<ResourceCache>();
    ui->GetRoot()->SetDefaultStyle(cache->GetResource<XMLFile>("UI/DefaultStyle.xml"));

    // Switch level
    // Reattempt reading the command line from the resource system now if not read before
    // Note that the engine can not be reconfigured at this point; only the script name can be specified
    if (GetArguments().Empty()) {
        SharedPtr<File> commandFile = GetSubsystem<ResourceCache>()->GetFile("CommandLine.txt", false);
        if (commandFile) {
            String commandLine = commandFile->ReadLine();
            commandFile->Close();
            ParseArguments(commandLine, false);
        }
    }

    LoadConfig();

    levelManager = context_->CreateObject<LevelManager>();
    _alertMessage = context_->CreateObject<Message>();
    _notifications = context_->CreateObject<Notifications>();
    _achievements = context_->CreateObject<Achievements>();
    _modLoader = context_->CreateObject<ModLoader>();
    _windowManager = context_->CreateObject<WindowManager>();

    VariantMap& eventData = GetEventDataMap();
    eventData["Name"] = "Level";
    SendEvent(MyEvents::E_SET_LEVEL, eventData);

    RegisterConsoleCommands();
}

void BaseApplication::Stop()
{
}

void BaseApplication::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
}

void BaseApplication::LoadConfig()
{
    JSONFile json(context_);
    json.LoadFile(GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Config/Game.json");
    JSONValue& content = json.GetRoot();
    if (content.IsObject()) {
        for (auto it = content.Begin(); it != content.End(); ++it) {
            //URHO3D_LOGINFO("Loading setting '" + String((*it).first_) + "'");
            if ((*it).second_.IsBool()) {
                engine_->SetGlobalVar((*it).first_, (*it).second_.GetBool());
                //URHO3D_LOGINFO("Value: " + String((*it).second_.GetBool()));
            }
            if ((*it).second_.IsString()) {
                engine_->SetGlobalVar((*it).first_, (*it).second_.GetString());
                //URHO3D_LOGINFO("Value: " + String((*it).second_.GetString()));
            }
            if ((*it).second_.IsNumber()) {
                engine_->SetGlobalVar((*it).first_, (*it).second_.GetInt());
                //URHO3D_LOGINFO("Value: " + String((*it).second_.GetInt()));
            }
        }
    }
    else {
        URHO3D_LOGERROR("Config file (Game.json) format not correct!");
    }
}

void BaseApplication::RegisterConsoleCommands()
{
    VariantMap data;
    data["ConsoleCommandName"] = "exit";
    data["ConsoleCommandEvent"] = "HandleExit";
    data["ConsoleCommandDescription"] = "Exits game";
    SendEvent("ConsoleCommandAdd", data);

    SubscribeToEvent("HandleExit", URHO3D_HANDLER(BaseApplication, HandleExit));
}

void BaseApplication::HandleExit(StringHash eventType, VariantMap& eventData)
{
    GetSubsystem<Engine>()->Exit();
}