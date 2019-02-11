#include "BaseApplication.h"
#include "Config/ConfigFile.h"
#include "Input/ControllerInput.h"
#include "Audio/AudioManager.h"
#include "Console/ConsoleHandler.h"
#include "SceneManager.h"
#include "MyEvents.h"

URHO3D_DEFINE_APPLICATION_MAIN(BaseApplication);

BaseApplication::BaseApplication(Context* context) :
    Application(context)
{
	ConfigFile::RegisterObject(context);
    ConfigManager::RegisterObject(context);

    context_->RegisterFactory<ControllerInput>();
    context_->RegisterFactory<LevelManager>();
    context_->RegisterFactory<Message>();
    context_->RegisterFactory<Notifications>();
    context_->RegisterFactory<Achievements>();
    SingleAchievement::RegisterObject(context_);
    context_->RegisterFactory<ModLoader>();
    context_->RegisterFactory<WindowManager>();
    context_->RegisterFactory<AudioManager>();
    context_->RegisterFactory<ConsoleHandler>();
    context_->RegisterFactory<SceneManager>();

    _configurationFile = GetSubsystem<FileSystem>()->GetProgramDir() + "/Data/Config/config.cfg";

    ConfigManager* configManager = new ConfigManager(context_, _configurationFile);
    context_->RegisterSubsystem(configManager);
    context_->RegisterSubsystem(new SceneManager(context_));

    auto* localization = GetSubsystem<Localization>();
    localization->LoadJSONFile(GetSubsystem<FileSystem>()->GetProgramDir() + "/Data/Translations/EN.json");

}

void BaseApplication::Setup()
{
    context_->RegisterSubsystem(new ConsoleHandler(context_));
    //LoadConfig("Data/Config/Config.json", "", true);
    LoadINIConfig(_configurationFile);
}

void BaseApplication::Start()
{
    UI* ui = GetSubsystem<UI>();
    GetSubsystem<ConsoleHandler>()->Create();

    DebugHud* debugHud = GetSubsystem<Engine>()->CreateDebugHud();
    auto* cache = GetSubsystem<ResourceCache>();
    XMLFile* xmlFile = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
    debugHud->SetDefaultStyle(xmlFile);

    cache->SetAutoReloadResources(true);
    ui->GetRoot()->SetDefaultStyle(cache->GetResource<XMLFile>("UI/DefaultStyle.xml"));

    SubscribeToEvents();

    GetSubsystem<FileSystem>()->SetExecuteConsoleCommands(false);

    context_->RegisterSubsystem<LevelManager>();
    context_->RegisterSubsystem<WindowManager>();
    context_->RegisterSubsystem<Message>();
    context_->RegisterSubsystem<Notifications>();
    context_->RegisterSubsystem<Achievements>();
	context_->RegisterSubsystem<ModLoader>();

    context_->RegisterSubsystem<AudioManager>();
    // Allow multiple music tracks to play at the same time
    context_->GetSubsystem<AudioManager>()->AllowMultipleMusicTracks(true);
    // Allow multiple ambient tracks to play at the same time
    context_->GetSubsystem<AudioManager>()->AllowMultipleAmbientTracks(true);

	context_->RegisterSubsystem<ControllerInput>();
    // Single player mode, all the input is handled by single Controls object
    context_->GetSubsystem<ControllerInput>()->SetMultipleControllerSupport(true);
    // Keyboard/mouse - 1st player, all the connected joysticks control new players
    // This will have no effect if `SetMultipleControllerSupport` is set to `false`
    context_->GetSubsystem<ControllerInput>()->SetJoystickAsFirstController(false);
    context_->GetSubsystem<ControllerInput>()->LoadConfig();

    SendEvent("GameStarted");

    RegisterConsoleCommands();

    ApplyGraphicsSettings();

    VariantMap& eventData = GetEventDataMap();
    eventData["Name"] = "MainMenu";
    SendEvent(MyEvents::E_SET_LEVEL, eventData);
}

void BaseApplication::Stop()
{
}

void BaseApplication::LoadConfig(String filename, String prefix, bool isMain)
{
    URHO3D_LOGINFO("Loading config file '" + filename + "' with '" + prefix + "' prefix");
    JSONFile json(context_);
    json.LoadFile(GetSubsystem<FileSystem>()->GetProgramDir() + filename);
    JSONValue& content = json.GetRoot();
    if (content.IsObject()) {
        for (auto it = content.Begin(); it != content.End(); ++it) {

            // If it's the main config file, we should only then register this
            // config parameter key for saving
            if (isMain) {
                _globalSettings[StringHash((*it).first_)] = (*it).first_;
            }
            if ((*it).second_.IsBool()) {
                engine_->SetGlobalVar(prefix + (*it).first_, (*it).second_.GetBool());
            }
            if ((*it).second_.IsString()) {
                engine_->SetGlobalVar(prefix + (*it).first_, (*it).second_.GetString());
            }
            if ((*it).second_.IsNumber()) {
                if ((*it).second_.GetNumberType() == JSONNT_FLOAT_DOUBLE) {
                    engine_->SetGlobalVar(prefix + (*it).first_, (*it).second_.GetFloat());
                }
                if ((*it).second_.GetNumberType() == JSONNT_INT) {
                    engine_->SetGlobalVar(prefix + (*it).first_, (*it).second_.GetInt());
                }
            }
        }
    }
    else {
        URHO3D_LOGERROR("Config file " + filename + " format is not correct!");
    }
}

void BaseApplication::HandleLoadConfig(StringHash eventType, VariantMap& eventData)
{
    String filename = eventData["Filepath"].GetString();
    String prefix = eventData["Prefix"].GetString();
    if (!filename.Empty()) {
        LoadConfig(filename, prefix);
    }
}

void BaseApplication::RegisterConsoleCommands()
{
    VariantMap data = GetEventDataMap();
    data["ConsoleCommandName"] = "exit";
    data["ConsoleCommandEvent"] = "HandleExit";
    data["ConsoleCommandDescription"] = "Exits game";
    SendEvent("ConsoleCommandAdd", data);

    SubscribeToEvent("HandleExit", URHO3D_HANDLER(BaseApplication, HandleExit));

    // How to use lambda (anonymous) functions
    SendEvent(MyEvents::E_CONSOLE_COMMAND_ADD, MyEvents::ConsoleCommandAdd::P_NAME, "debugger", MyEvents::ConsoleCommandAdd::P_EVENT, "#debugger", MyEvents::ConsoleCommandAdd::P_DESCRIPTION, "Show debug");
    SubscribeToEvent("#debugger", [&](StringHash eventType, VariantMap& eventData) {
        GetSubsystem<DebugHud>()->Toggle(DEBUGHUD_SHOW_STATS);
//        String stat = "ABCV";
//        GetSubsystem<DebugHud>()->SetAppStats("Hahaha", stat);
    });
}

void BaseApplication::HandleExit(StringHash eventType, VariantMap& eventData)
{
    GetSubsystem<Engine>()->Exit();
}

void BaseApplication::SubscribeToEvents()
{
    SubscribeToEvent(MyEvents::E_ADD_CONFIG, URHO3D_HANDLER(BaseApplication, HandleAddConfig));
    SubscribeToEvent(MyEvents::E_LOAD_CONFIG, URHO3D_HANDLER(BaseApplication, HandleLoadConfig));
}

void BaseApplication::HandleAddConfig(StringHash eventType, VariantMap& eventData)
{
    String paramName = eventData["Name"].GetString();
    if (!paramName.Empty()) {
        _globalSettings[paramName] = paramName;

        using namespace MyEvents::ConsoleCommandAdd;
        VariantMap data = GetEventDataMap();
        data[P_NAME] = paramName;
        data[P_EVENT] = "ConsoleGlobalVariableChange";
        data[P_DESCRIPTION] = "Show/Change global variable value";
        SendEvent(MyEvents::E_CONSOLE_COMMAND_ADD, data);
    }
}

void BaseApplication::LoadINIConfig(String filename)
{
	bool loaded = GetSubsystem<ConfigManager>()->Load(filename, true);
	if (!loaded) {
		URHO3D_LOGERROR("Unable to load configuration file '" + _configurationFile + "'");
		return;
	}

    SetEngineParameter(EP_MONITOR, GetSubsystem<ConfigManager>()->GetInt("engine", "Monitor", 0));
    SetEngineParameter(EP_FULL_SCREEN, GetSubsystem<ConfigManager>()->GetBool("engine", "FullScreen", false));
    SetEngineParameter(EP_WINDOW_WIDTH, GetSubsystem<ConfigManager>()->GetInt("engine", "WindowWidth", 800));
    SetEngineParameter(EP_WINDOW_HEIGHT, GetSubsystem<ConfigManager>()->GetInt("engine", "WindowHeight", 600));
    SetEngineParameter(EP_BORDERLESS, GetSubsystem<ConfigManager>()->GetBool("engine", "Borderless", false));
    SetEngineParameter(EP_FRAME_LIMITER, GetSubsystem<ConfigManager>()->GetBool("engine", "FrameLimiter", true));
    SetEngineParameter(EP_WINDOW_TITLE, "EmptyProject");
    SetEngineParameter(EP_WINDOW_ICON, "Data/Textures/UrhoIcon.png");

    // Logs
    SetEngineParameter(EP_LOG_NAME, GetSubsystem<ConfigManager>()->GetString("engine", "LogName", "EmptyProject.log"));
    SetEngineParameter(EP_LOG_LEVEL, GetSubsystem<ConfigManager>()->GetInt("engine", "LogLevel", LOG_INFO));
    SetEngineParameter(EP_LOG_QUIET, GetSubsystem<ConfigManager>()->GetBool("engine", "LogQuiet", false));

    // Graphics
    SetEngineParameter(EP_LOW_QUALITY_SHADOWS, GetSubsystem<ConfigManager>()->GetBool("engine", "LowQualityShadows", false));
    SetEngineParameter(EP_MATERIAL_QUALITY, GetSubsystem<ConfigManager>()->GetInt("engine", "MaterialQuality", 15));
    SetEngineParameter(EP_MULTI_SAMPLE, GetSubsystem<ConfigManager>()->GetInt("engine", "MultiSample", 1));
    SetEngineParameter(EP_SHADOWS, GetSubsystem<ConfigManager>()->GetBool("engine", "Shadows", true));
    SetEngineParameter(EP_TEXTURE_ANISOTROPY, GetSubsystem<ConfigManager>()->GetInt("engine", "TextureAnisotropy", 15));
    SetEngineParameter(EP_TEXTURE_FILTER_MODE, GetSubsystem<ConfigManager>()->GetInt("engine", "TextureFilterMode", 5));

    SetEngineParameter(EP_TEXTURE_QUALITY, GetSubsystem<ConfigManager>()->GetInt("engine", "TextureQuality", 2));
    SetEngineParameter(EP_TRIPLE_BUFFER, GetSubsystem<ConfigManager>()->GetBool("engine", "TripleBuffer", true));
    SetEngineParameter(EP_VSYNC, GetSubsystem<ConfigManager>()->GetBool("engine", "VSync", true));

    SetEngineParameter(EP_RESOURCE_PATHS, GetSubsystem<ConfigManager>()->GetString("engine", "ResourcePaths", "Data;CoreData"));

    // Sound
    SetEngineParameter(EP_SOUND, GetSubsystem<ConfigManager>()->GetBool("audio", "Sound", true));
    SetEngineParameter(EP_SOUND_BUFFER, GetSubsystem<ConfigManager>()->GetInt("audio", "SoundBuffer", 100));
    SetEngineParameter(EP_SOUND_INTERPOLATION, GetSubsystem<ConfigManager>()->GetBool("audio", "SoundInterpolation", true));
    SetEngineParameter(EP_SOUND_MIX_RATE, GetSubsystem<ConfigManager>()->GetInt("audio", "SoundMixRate", 44100));
    SetEngineParameter(EP_SOUND_STEREO, GetSubsystem<ConfigManager>()->GetBool("audio", "SoundStereo", true));

    SetEngineParameter(EP_FLUSH_GPU, GetSubsystem<ConfigManager>()->GetBool("engine", "FlushGPU", true));
    SetEngineParameter(EP_WORKER_THREADS, GetSubsystem<ConfigManager>()->GetBool("engine", "WorkerThreads ", true));
    engine_->SetGlobalVar("ShadowQuality", GetSubsystem<ConfigManager>()->GetInt("engine", "ShadowQuality", 5));

	engine_->SetGlobalVar("Master" , GetSubsystem<ConfigManager>()->GetFloat("audio", "Master", 1.0));
	engine_->SetGlobalVar("Effect", GetSubsystem<ConfigManager>()->GetFloat("audio", "Effect", 1.0));
	engine_->SetGlobalVar("Ambient", GetSubsystem<ConfigManager>()->GetFloat("audio", "Ambient", 1.0));
	engine_->SetGlobalVar("Voice", GetSubsystem<ConfigManager>()->GetFloat("audio", "Voice", 1.0));
	engine_->SetGlobalVar("Music", GetSubsystem<ConfigManager>()->GetFloat("audio", "Music", 1.0));

    Audio* audio = GetSubsystem<Audio>();
	audio->SetMasterGain(SOUND_MASTER, engine_->GetGlobalVar("Master").GetFloat());
	audio->SetMasterGain(SOUND_EFFECT, engine_->GetGlobalVar("Effect").GetFloat());
	audio->SetMasterGain(SOUND_AMBIENT, engine_->GetGlobalVar("Ambient").GetFloat());
	audio->SetMasterGain(SOUND_VOICE, engine_->GetGlobalVar("Voice").GetFloat());
	audio->SetMasterGain(SOUND_MUSIC, engine_->GetGlobalVar("Music").GetFloat());

    auto* localization = GetSubsystem<Localization>();
    localization->SetLanguage(GetSubsystem<ConfigManager>()->GetString("engine", "Language", "EN"));
}

void BaseApplication::ApplyGraphicsSettings()
{
    auto* renderer = GetSubsystem<Renderer>();
    renderer->SetShadowQuality((ShadowQuality)engine_->GetGlobalVar("ShadowQuality").GetInt());
}

void BaseApplication::SetEngineParameter(String parameter, Variant value)
{
    engineParameters_[parameter] = value;
    engine_->SetGlobalVar(parameter, value);
	_globalSettings[parameter] = parameter;

    using namespace MyEvents::ConsoleCommandAdd;
    VariantMap data = GetEventDataMap();
    data[P_NAME] = parameter;
    data[P_EVENT] = "ConsoleGlobalVariableChange";
    data[P_DESCRIPTION] = "Show/Change global variable value";
    SendEvent(MyEvents::E_CONSOLE_COMMAND_ADD, data);
}