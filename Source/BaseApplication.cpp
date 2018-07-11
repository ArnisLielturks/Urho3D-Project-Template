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
#include "Config/ConfigFile.h"
#include "Input/ControllerInput.h"

#include <Urho3D/UI/Button.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/DebugNew.h>
#include <Urho3D/Math/MathDefs.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/AngelScript/ScriptAPI.h>
#include <string>


URHO3D_DEFINE_APPLICATION_MAIN(BaseApplication);

BaseApplication::BaseApplication(Context* context) :
    Application(context)
{
	ConfigFile::RegisterObject(context);
    context_->RegisterFactory<ControllerInput>();
    context_->RegisterFactory<LevelManager>();
    context_->RegisterFactory<Message>();
    context_->RegisterFactory<Notifications>();
    context_->RegisterFactory<Achievements>();
    context_->RegisterFactory<ModLoader>();
    context_->RegisterFactory<WindowManager>();
}

void BaseApplication::Setup()
{
    LoadConfig("Data/Config/Config.json", "", true);

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
	// Does not work, resource cache should be initialized
	//LoadINIConfig("Config/config.cfg");

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

    SubscribeToEvents();

    levelManager = context_->CreateObject<LevelManager>();
    _alertMessage = context_->CreateObject<Message>();
    _notifications = context_->CreateObject<Notifications>();
    _achievements = context_->CreateObject<Achievements>();
	context_->RegisterSubsystem<ModLoader>();
    _windowManager = context_->CreateObject<WindowManager>();

	context_->RegisterSubsystem<ControllerInput>();

    VariantMap& eventData = GetEventDataMap();
    eventData["Name"] = "MainMenu";
    SendEvent(MyEvents::E_SET_LEVEL, eventData);

    RegisterConsoleCommands();
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
			int newIntVal = std::stoi(params[1].CString());
			SetGlobalVar(params[0], newIntVal);
			newValue = String(newIntVal);
		}
		if (value.GetType() == VAR_FLOAT) {
			oldValue = String(value.GetFloat());
			float newFloatVal = std::stof(params[1].CString());
			SetGlobalVar(params[0], newFloatVal);
			newValue = String(newFloatVal);
		}
		if (value.GetType() == VAR_DOUBLE) {
			oldValue = String(value.GetDouble());
			float newFloatVal = std::stof(params[1].CString());
			SetGlobalVar(params[0], newFloatVal);
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
	auto* cache = GetSubsystem<ResourceCache>();
	auto configFile = cache->GetResource<ConfigFile>(filename);

	auto width = configFile->GetInt("engine", "WindowWidth", 1024);
	auto height = configFile->GetInt("engine", "WindowHeight", 768);

	URHO3D_LOGINFO("Width: " + String(width));
	URHO3D_LOGINFO("Height: " + String(height));
}