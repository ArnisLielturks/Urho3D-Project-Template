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
    LoadConfig();

    engineParameters_[EP_FULL_SCREEN] = engine_->GetGlobalVar("Fullscreen").GetBool();
    engineParameters_[EP_WINDOW_WIDTH] = engine_->GetGlobalVar("ScreenWidth").GetInt();
    engineParameters_[EP_WINDOW_HEIGHT] = engine_->GetGlobalVar("ScreenHeight").GetInt();
    engineParameters_[EP_BORDERLESS] = false;
    engineParameters_[EP_FRAME_LIMITER] = engine_->GetGlobalVar("FrameLimiter").GetBool();
    engineParameters_[EP_WINDOW_TITLE] = "EmptyProject";
    engineParameters_[EP_WINDOW_ICON] = "Data/Textures/UrhoIcon.png";

    // Logs
    engineParameters_[EP_LOG_NAME] = "EmptyProject.log";
    engineParameters_[EP_LOG_LEVEL] = LOG_INFO;
    engineParameters_[EP_LOG_QUIET] = false;

    // Graphics
    engineParameters_[EP_LOW_QUALITY_SHADOWS] = engine_->GetGlobalVar("LowQualityShadows").GetBool(); 
    engineParameters_[EP_MATERIAL_QUALITY] = engine_->GetGlobalVar("MaterialQuality").GetInt(); // 0 - 15
    engineParameters_[EP_MONITOR] = engine_->GetGlobalVar("Monitor").GetInt();
    engineParameters_[EP_MULTI_SAMPLE] = engine_->GetGlobalVar("MultiSample").GetInt(); // 1 - N
    engineParameters_[EP_SHADOWS] = engine_->GetGlobalVar("Shadows").GetBool();
    engineParameters_[EP_TEXTURE_ANISOTROPY] = engine_->GetGlobalVar("TextureAnisotropy").GetInt();
    engineParameters_[EP_TEXTURE_FILTER_MODE] = engine_->GetGlobalVar("TextureFilterMode").GetInt();
    /*
    FILTER_NEAREST = 0,
    FILTER_BILINEAR = 1,
    FILTER_TRILINEAR = 2,
    FILTER_ANISOTROPIC = 3,
    FILTER_NEAREST_ANISOTROPIC = 4,
    FILTER_DEFAULT = 5,
    MAX_FILTERMODES = 6
    */
    engineParameters_[EP_TEXTURE_QUALITY] = engine_->GetGlobalVar("TextureQuality").GetInt();
    engineParameters_[EP_TRIPLE_BUFFER] = engine_->GetGlobalVar("TripleBuffer").GetBool();
    engineParameters_[EP_VSYNC] = engine_->GetGlobalVar("VSync").GetBool();

    // engineParameters_[EP_PACKAGE_CACHE_DIR] = engine_->GetGlobalVar("FrameLimiter").GetBool();
    // engineParameters_[EP_RENDER_PATH] = engine_->GetGlobalVar("FrameLimiter").GetBool();
    // engineParameters_[EP_RESOURCE_PACKAGES] = engine_->GetGlobalVar("FrameLimiter").GetBool();
    engineParameters_[EP_RESOURCE_PATHS] = engine_->GetGlobalVar("ResourcePaths").GetString();
    // engineParameters_[EP_RESOURCE_PREFIX_PATHS] = engine_->GetGlobalVar("FrameLimiter").GetBool();
    // engineParameters_[EP_SHADER_CACHE_DIR] = engine_->GetGlobalVar("FrameLimiter").GetBool();

    // Sound
    engineParameters_[EP_SOUND] = engine_->GetGlobalVar("Sound").GetBool();
    engineParameters_[EP_SOUND_BUFFER] = engine_->GetGlobalVar("SoundBuffer").GetInt();
    engineParameters_[EP_SOUND_INTERPOLATION] = engine_->GetGlobalVar("SoundInterpolation").GetBool();
    engineParameters_[EP_SOUND_MIX_RATE] = engine_->GetGlobalVar("SoundMixRate").GetInt();
    engineParameters_[EP_SOUND_STEREO] = engine_->GetGlobalVar("SoundStereo").GetBool();
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

    levelManager = context_->CreateObject<LevelManager>();
    _alertMessage = context_->CreateObject<Message>();
    _notifications = context_->CreateObject<Notifications>();
    _achievements = context_->CreateObject<Achievements>();
    _modLoader = context_->CreateObject<ModLoader>();
    _windowManager = context_->CreateObject<WindowManager>();

    VariantMap& eventData = GetEventDataMap();
    eventData["Name"] = "Splash";
    SendEvent(MyEvents::E_SET_LEVEL, eventData);

    RegisterConsoleCommands();

    SubscribeToEvents();
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
    json.LoadFile(GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Config/Config.json");
    JSONValue& content = json.GetRoot();
    if (content.IsObject()) {
        for (auto it = content.Begin(); it != content.End(); ++it) {
            //URHO3D_LOGINFO("Loading setting '" + String((*it).first_) + "'");
            _globalSettings[StringHash((*it).first_)] = (*it).first_;
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

void BaseApplication::SaveConfig()
{
    URHO3D_LOGINFO("Saving config");
    JSONFile json(context_);
    JSONValue& content = json.GetRoot();
    for (auto it = _globalSettings.Begin(); it != _globalSettings.End(); ++it) {
        const Variant value = engine_->GetGlobalVar((*it).first_);
        if (value.GetType() == VAR_STRING) {
            content.Set((*it).second_.GetString(), value.GetString());
        }
        if (value.GetType() == VAR_BOOL) {
            content.Set((*it).second_.GetString(), value.GetBool());
        }
        if (value.GetType() == VAR_INT) {
            content.Set((*it).second_.GetString(), value.GetInt());
        }
    }
    json.SaveFile(GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Config/Config.json");
}

void BaseApplication::HandleSaveConfig(StringHash eventType, VariantMap& eventData)
{
    SaveConfig();
}

void BaseApplication::RegisterConsoleCommands()
{
    VariantMap data = GetEventDataMap();
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

void BaseApplication::SubscribeToEvents()
{
    SubscribeToEvent(MyEvents::E_SAVE_CONFIG, URHO3D_HANDLER(BaseApplication, HandleSaveConfig));
}