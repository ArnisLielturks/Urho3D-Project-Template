#include "BaseApplication.h"
#include "Config/ConfigFile.h"
#include "Input/ControllerInput.h"
#include "Audio/AudioManager.h"
#include "UI/NuklearUI.h"

URHO3D_DEFINE_APPLICATION_MAIN(BaseApplication);

BaseApplication::BaseApplication(Context* context) :
    Application(context)
{
	ConfigFile::RegisterObject(context);
    ConfigManager::RegisterObject(context);

    context_->RegisterFactory<NuklearUI>();
    context_->RegisterFactory<ControllerInput>();
    context_->RegisterFactory<LevelManager>();
    context_->RegisterFactory<Message>();
    context_->RegisterFactory<Notifications>();
    context_->RegisterFactory<Achievements>();
    SingleAchievement::RegisterObject(context_);
    context_->RegisterFactory<ModLoader>();
    context_->RegisterFactory<WindowManager>();
    context_->RegisterFactory<AudioManager>();

    _configManager = new ConfigManager(context);

    _configurationFile = GetSubsystem<FileSystem>()->GetProgramDir() + "/Data/Config/config.cfg";
}

void BaseApplication::Setup()
{
    //LoadConfig("Data/Config/Config.json", "", true);
    LoadINIConfig(_configurationFile);

}

void BaseApplication::Start()
{
    UI* ui = GetSubsystem<UI>();
    auto* cache = GetSubsystem<ResourceCache>();
    cache->SetAutoReloadResources(true);
    ui->GetRoot()->SetDefaultStyle(cache->GetResource<XMLFile>("UI/DefaultStyle.xml"));

    /*DebugHud* debugHud = GetSubsystem<Engine>()->CreateDebugHud();

    XMLFile* xmlFile = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
    debugHud->SetDefaultStyle(xmlFile);
    debugHud->ToggleAll();*/

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

    SubscribeToEvents();

    auto nuklear = new NuklearUI(context_);
    context_->RegisterSubsystem(nuklear);
    // Initialize default font of your choice or use default one.
    //nuklear->GetFontAtlas()->default_font = nk_font_atlas_add_default(nuklear->GetFontAtlas(), 13.f, 0);
    String fontPath = GetSubsystem<FileSystem>()->GetProgramDir() + "/Data/Fonts/Anonymous Pro.ttf";
    nuklear->GetFontAtlas()->default_font = nk_font_atlas_add_from_file(nuklear->GetFontAtlas(), fontPath.CString(), 16.0f, NULL);

    // Additional font initialization here. See https://github.com/vurtun/nuklear/blob/master/demo/sdl_opengl3/main.c
    nuklear->FinalizeFonts();

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
    context_->GetSubsystem<ControllerInput>()->SetMultipleControllerSupport(false);

    VariantMap& eventData = GetEventDataMap();
    eventData["Name"] = "Splash";
    SendEvent(MyEvents::E_SET_LEVEL, eventData);

    RegisterConsoleCommands();

    ApplyGraphicsSettings();
}

void BaseApplication::Stop()
{
}

void BaseApplication::HandleUpdate(StringHash eventType, VariantMap& eventData)
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
        if (value.GetType() == VAR_FLOAT) {
            content.Set((*it).second_.GetString(), value.GetFloat());
        }
        if (value.GetType() == VAR_DOUBLE) {
            content.Set((*it).second_.GetString(), value.GetFloat());
        }
    }
    json.SaveFile(GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Config/Config.json");
}

void BaseApplication::HandleLoadConfig(StringHash eventType, VariantMap& eventData)
{
    String filename = eventData["Filepath"].GetString();
    String prefix = eventData["Prefix"].GetString();
    if (!filename.Empty()) {
        LoadConfig(filename, prefix);
    }
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

	// Register all global variables as a console commands
	for (auto it = _globalSettings.Begin(); it != _globalSettings.End(); ++it) {
		using namespace MyEvents::ConsoleCommandAdd;
		VariantMap data = GetEventDataMap();
		data[P_NAME] = (*it).second_;
		data[P_EVENT] = "ConsoleGlobalVariableChange";
		data[P_DESCRIPTION] = "Show/Change global variable value";
		SendEvent(MyEvents::E_CONSOLE_COMMAND_ADD, data);
	}

	SubscribeToEvent(MyEvents::E_CONSOLE_GLOBAL_VARIABLE_CHANGE, URHO3D_HANDLER(BaseApplication, HandleConsoleGlobalVariableChange));
}

void BaseApplication::HandleExit(StringHash eventType, VariantMap& eventData)
{
    GetSubsystem<Engine>()->Exit();
}

void BaseApplication::SubscribeToEvents()
{
    SubscribeToEvent(MyEvents::E_SAVE_CONFIG, URHO3D_HANDLER(BaseApplication, HandleSaveConfig));
    SubscribeToEvent(MyEvents::E_ADD_CONFIG, URHO3D_HANDLER(BaseApplication, HandleAddConfig));
    SubscribeToEvent(MyEvents::E_LOAD_CONFIG, URHO3D_HANDLER(BaseApplication, HandleLoadConfig));
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(BaseApplication, HandleUpdate));
}

void BaseApplication::HandleAddConfig(StringHash eventType, VariantMap& eventData)
{
    String paramName = eventData["Name"].GetString();
    if (!paramName.Empty()) {
        URHO3D_LOGINFO("Adding new config value: " + paramName);
        _globalSettings[paramName] = paramName;
    }
}

void BaseApplication::HandleConsoleGlobalVariableChange(StringHash eventType, VariantMap& eventData)
{
	StringVector params = eventData["Parameters"].GetStringVector();

	const Variant value = engine_->GetGlobalVar(params[0]);

	// Only show variable
	if (params.Size() == 1 && !value.IsEmpty()) {
		String stringValue;
		if (value.GetType() == VAR_STRING) {
			stringValue = value.GetString();
		}
		if (value.GetType() == VAR_BOOL) {
			stringValue = value.GetBool() ? "1" : "0";
		}
		if (value.GetType() == VAR_INT) {
			stringValue = String(value.GetInt());
		}
		if (value.GetType() == VAR_FLOAT) {
			stringValue = String(value.GetFloat());
		}
		if (value.GetType() == VAR_DOUBLE) {
			stringValue = String(value.GetDouble());
		}
		URHO3D_LOGINFO("Global variable '" + params[0] + "' = " + stringValue);
		return;
	}

	// Read console input parameters and change global variable
	if (params.Size() == 2) {
		String oldValue;
		String newValue;
		if (value.GetType() == VAR_STRING) {
			oldValue = value.GetString();
			SetGlobalVar(params[0], params[1]);
			newValue = params[1];
		}
		if (value.GetType() == VAR_BOOL) {
			oldValue = value.GetBool() ? "1" : "0";
			if (params[1] == "1" || params[1] == "true") {
				SetGlobalVar(params[0], true);
				newValue = "1";
			}
			if (params[1] == "0" || params[1] == "false") {
				SetGlobalVar(params[0], false);
				newValue = "0";
			}
		}
		if (value.GetType() == VAR_INT) {
			oldValue = String(value.GetInt());
			int newIntVal = ToInt(params[1].CString());
			SetGlobalVar(params[0], newIntVal);
			newValue = String(newIntVal);
		}
		if (value.GetType() == VAR_FLOAT) {
			oldValue = String(value.GetFloat());
			float newFloatVal = ToFloat(params[1].CString());
			SetGlobalVar(params[0], newFloatVal);
			newValue = String(newFloatVal);
		}
		if (value.GetType() == VAR_DOUBLE) {
			oldValue = String(value.GetDouble());
			float newFloatVal = ToFloat(params[1].CString());
			SetGlobalVar(params[0],newFloatVal);
			newValue = String(newFloatVal);
		}
		URHO3D_LOGINFO("Changed global variable '" + params[0] + "' from '" + oldValue + "' to '" + newValue + "'");

		// Let others know that configuration was updated, to allow game tweaking accordingly
		using namespace MyEvents::ConsoleGlobalVariableChanged;
		VariantMap data = GetEventDataMap();
		data[P_NAME] = params[0];
		data[P_VALUE] = newValue;
		SendEvent(MyEvents::E_CONSOLE_GLOBAL_VARIABLE_CHANGED, data);
	}
}

void BaseApplication::LoadINIConfig(String filename)
{
	bool loaded = _configManager->Load(filename, true);
	if (!loaded) {
		URHO3D_LOGERROR("Unable to load configuration file '" + _configurationFile + "'");
		return;
	}

    SetEngineParameter(EP_FULL_SCREEN, _configManager->GetBool("engine", "FullScreen", false));
    SetEngineParameter(EP_WINDOW_WIDTH, _configManager->GetInt("engine", "WindowWidth", 800));
    SetEngineParameter(EP_WINDOW_HEIGHT, _configManager->GetInt("engine", "WindowHeight", 600));
    SetEngineParameter(EP_BORDERLESS, _configManager->GetBool("engine", "Borderless", false));
    SetEngineParameter(EP_FRAME_LIMITER, _configManager->GetBool("engine", "FrameLimiter", true));
    SetEngineParameter(EP_WINDOW_TITLE, "EmptyProject");
    SetEngineParameter(EP_WINDOW_ICON, "Data/Textures/UrhoIcon.png");

    // Logs
    SetEngineParameter(EP_LOG_NAME, _configManager->GetString("engine", "LogName", "EmptyProject.log"));
    SetEngineParameter(EP_LOG_LEVEL, _configManager->GetInt("engine", "LogLevel", LOG_INFO));
    SetEngineParameter(EP_LOG_QUIET, _configManager->GetBool("engine", "LogQuiet", false));

    // Graphics
    SetEngineParameter(EP_LOW_QUALITY_SHADOWS, _configManager->GetBool("engine", "LowQualityShadows", false));
    SetEngineParameter(EP_MATERIAL_QUALITY, _configManager->GetInt("engine", "MaterialQuality", 15));
    SetEngineParameter(EP_MONITOR, _configManager->GetInt("engine", "Monitor", 0));
    SetEngineParameter(EP_MULTI_SAMPLE, _configManager->GetInt("engine", "MultiSample", 1));
    SetEngineParameter(EP_SHADOWS, _configManager->GetBool("engine", "Shadows", true));
    SetEngineParameter(EP_TEXTURE_ANISOTROPY, _configManager->GetInt("engine", "TextureAnisotropy", 16));
    SetEngineParameter(EP_TEXTURE_FILTER_MODE, _configManager->GetInt("engine", "TextureFilterMode", 5));
    /*
    FILTER_NEAREST = 0,
    FILTER_BILINEAR = 1,
    FILTER_TRILINEAR = 2,
    FILTER_ANISOTROPIC = 3,
    FILTER_NEAREST_ANISOTROPIC = 4,
    FILTER_DEFAULT = 5,
    MAX_FILTERMODES = 6
    */
    SetEngineParameter(EP_TEXTURE_QUALITY, _configManager->GetInt("engine", "TextureQuality", 2));
    SetEngineParameter(EP_TRIPLE_BUFFER, _configManager->GetBool("engine", "TripleBuffer", true));
    SetEngineParameter(EP_VSYNC, _configManager->GetBool("engine", "VSync", true));

    // SetEngineParameter(EP_PACKAGE_CACHE_DIR, engine_->GetGlobalVar("FrameLimiter").GetBool());
    // SetEngineParameter(EP_RENDER_PATH, engine_->GetGlobalVar("FrameLimiter").GetBool());
    // SetEngineParameter(EP_RESOURCE_PACKAGES, engine_->GetGlobalVar("FrameLimiter").GetBool());
    SetEngineParameter(EP_RESOURCE_PATHS, _configManager->GetString("engine", "ResourcePaths", "Data;CoreData"));
    // SetEngineParameter(EP_RESOURCE_PREFIX_PATHS, engine_->GetGlobalVar("FrameLimiter").GetBool());
    // SetEngineParameter(EP_SHADER_CACHE_DIR, engine_->GetGlobalVar("FrameLimiter").GetBool());

    // Sound
    SetEngineParameter(EP_SOUND, _configManager->GetBool("engine", "Sound", true));
    SetEngineParameter(EP_SOUND_BUFFER, _configManager->GetInt("engine", "SoundBuffer", 100));
    SetEngineParameter(EP_SOUND_INTERPOLATION, _configManager->GetBool("engine", "SoundInterpolation", true));
    SetEngineParameter(EP_SOUND_MIX_RATE, _configManager->GetInt("engine", "SoundMixRate", 44100));
    SetEngineParameter(EP_SOUND_STEREO, _configManager->GetBool("engine", "SoundStereo", true));

	engine_->SetGlobalVar("SoundMasterVolume" , _configManager->GetFloat("engine", "SoundMasterVolume", 1.0));
	engine_->SetGlobalVar("SoundEffectsVolume", _configManager->GetFloat("engine", "SoundEffectsVolume", 1.0));
	engine_->SetGlobalVar("SoundAmbientVolume", _configManager->GetFloat("engine", "SoundAmbientVolume", 1.0));
	engine_->SetGlobalVar("SoundVoiceVolume", _configManager->GetFloat("engine", "SoundVoiceVolume", 1.0));
	engine_->SetGlobalVar("SoundMusicVolume", _configManager->GetFloat("engine", "SoundMusicVolume", 1.0));
    engine_->SetGlobalVar("ShadowQuality", _configManager->GetInt("engine", "ShadowQuality", 5));

    Audio* audio = GetSubsystem<Audio>();
	audio->SetMasterGain(SOUND_MASTER, engine_->GetGlobalVar("SoundMasterVolume").GetFloat());
	audio->SetMasterGain(SOUND_EFFECT, engine_->GetGlobalVar("SoundEffectsVolume").GetFloat());
	audio->SetMasterGain(SOUND_AMBIENT, engine_->GetGlobalVar("SoundAmbientVolume").GetFloat());
	audio->SetMasterGain(SOUND_VOICE, engine_->GetGlobalVar("SoundVoiceVolume").GetFloat());
	audio->SetMasterGain(SOUND_MUSIC, engine_->GetGlobalVar("SoundMusicVolume").GetFloat());
}

void BaseApplication::ApplyGraphicsSettings()
{
    auto* renderer = GetSubsystem<Renderer>();
    renderer->SetShadowQuality((ShadowQuality)engine_->GetGlobalVar("ShadowQuality").GetInt());
}

void BaseApplication::SetEngineParameter(String parameter, Variant value)
{
    URHO3D_LOGINFO(".... Setting Engine parameter " + parameter);
    engineParameters_[parameter] = value;
    engine_->SetGlobalVar(parameter, value);
	_globalSettings[parameter] = parameter;
}