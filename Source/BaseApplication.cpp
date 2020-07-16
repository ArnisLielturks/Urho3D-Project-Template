#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Engine/DebugHud.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Audio/Audio.h>
#include <Urho3D/Resource/Localization.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/PackageFile.h>
#include "BaseApplication.h"
#include "Config/ConfigFile.h"
#include "Input/ControllerInput.h"
#include "Audio/AudioManager.h"
#include "Console/ConsoleHandler.h"
#include "SceneManager.h"
#include "CustomEvents.h"
#include "Global.h"
#include "Generator/Generator.h"
#include "AndroidEvents/ServiceCmd.h"
#include "BehaviourTree/BehaviourTree.h"
#include "State/State.h"
#include "Console/ConsoleHandlerEvents.h"
#include "Console/ConsoleHandlerEvents.h"
#include "LevelManagerEvents.h"
#include "Input/ControllerEvents.h"

#if defined(URHO3D_LUA) || defined(URHO3D_ANGELSCRIPT)
#include "Mods/ModLoader.h"
#endif

#if defined(__EMSCRIPTEN__)
#include <Urho3D/Graphics/GraphicsEvents.h>
#include <Urho3D/Input/InputEvents.h>
#include <emscripten/emscripten.h>
#include <emscripten/bind.h>
static const Context *appContext;
static bool mouseVisible;
static unsigned int mouseMode;
#endif

URHO3D_DEFINE_APPLICATION_MAIN(BaseApplication);

using namespace ConsoleHandlerEvents;
using namespace LevelManagerEvents;
using namespace ControllerEvents;
using namespace CustomEvents;

BaseApplication::BaseApplication(Context* context) :
    Application(context)
{
    ConfigFile::RegisterObject(context);
    ConfigManager::RegisterObject(context);

    context_->RegisterFactory<ControllerInput>();
    context_->RegisterFactory<LevelManager>();
    context_->RegisterFactory<Notifications>();
    context_->RegisterFactory<Achievements>();
    SingleAchievement::RegisterObject(context_);
    State::RegisterObject(context_);
    LevelManager::RegisterObject(context_);

    #if defined(URHO3D_LUA) || defined(URHO3D_ANGELSCRIPT)
    context_->RegisterFactory<ModLoader>();
    #endif

    #if defined(__EMSCRIPTEN__)
    appContext = context;
    #endif

    context_->RegisterFactory<WindowManager>();
    context_->RegisterFactory<AudioManager>();
    context_->RegisterFactory<ConsoleHandler>();
    context_->RegisterFactory<SceneManager>();
    context_->RegisterFactory<Generator>();

    BehaviourTree::RegisterFactory(context_);

#ifdef __ANDROID__
    configurationFile_ = GetSubsystem<FileSystem>()->GetUserDocumentsDir() + DOCUMENTS_DIR + "/config.cfg";
#else

#ifdef __EMSCRIPTEN__
    configurationFile_ = GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Config/config.cfg";
#else
    configurationFile_ = "Data/Config/config.cfg";
#endif
#endif

    ConfigManager* configManager = new ConfigManager(context_);
    configManager->SetFilePath(configurationFile_);
    context_->RegisterSubsystem(configManager);
    context_->RegisterSubsystem(new State(context_));
    context_->RegisterSubsystem(new SceneManager(context_));

    context_->RegisterSubsystem(new ServiceCmd(context_));

#ifdef __ANDROID__
    String directory = GetSubsystem<FileSystem>()->GetUserDocumentsDir() + DOCUMENTS_DIR;
    if (!GetSubsystem<FileSystem>()->DirExists(directory)) {
        GetSubsystem<FileSystem>()->CreateDir(directory);
    }
#endif
}

void BaseApplication::Setup()
{
    context_->RegisterSubsystem(new ConsoleHandler(context_));
    LoadINIConfig(configurationFile_);

//    #if defined(__EMSCRIPTEN__)
//    SubscribeToEvent(E_SCREENMODE, [&](StringHash eventType, VariantMap& eventData) {
//        using namespace ScreenMode;
//        int width = eventData[P_WIDTH].GetInt();
//        int height = eventData[P_HEIGHT].GetInt();
//
//        URHO3D_LOGINFOF("Screen size changed %dx%d", width, height);
//
//        EM_ASM({
//            Module.SetRendererSize($0, $1);
//        }, width, height);
//    });
//
//    SubscribeToEvent(E_MOUSEVISIBLECHANGED, [&](StringHash eventType, VariantMap &eventData) {
//        using namespace MouseVisibleChanged;
//        mouseVisible = eventData[P_VISIBLE].GetBool();
//        URHO3D_LOGINFOF("mouseVisible = %d", mouseVisible);
//
//        EM_ASM({
//            Module.SetMouseVisible($0);
//        }, mouseVisible);
//    });
//    SubscribeToEvent(E_MOUSEMODECHANGED, [&](StringHash eventType, VariantMap &eventData) {
//        using namespace MouseModeChanged;
//        mouseMode = eventData[P_MODE].GetUInt();
//        URHO3D_LOGINFOF("mouseMode = %u", mouseMode);
//    });
//    #endif
}

//#if defined(__EMSCRIPTEN__)
//void BaseApplication::JSCanvasSize(int width, int height, bool fullscreen, float scale)
//{
//    URHO3D_LOGINFOF("JSCanvasSize: %dx%d", width, height);
//    appContext->GetSubsystem<Graphics>()->SetMode(width, height);
//    UI* ui = appContext->GetSubsystem<UI>();
//    ui->SetScale(scale);
////    appContext->GetSubsystem<UI>()->GetCursor()->SetPosition(appContext->GetSubsystem<Input>()->GetMousePosition());
//}
//
//void BaseApplication::JSMouseFocus()
//{
//    auto input = appContext->GetSubsystem<Input>();
//    input->SetMouseVisible(mouseVisible);
//    input->SetMouseMode(static_cast<MouseMode>(mouseMode));
////    appContext->GetSubsystem<UI>()->GetCursor()->SetPosition(appContext->GetSubsystem<Input>()->GetMousePosition());
//    EM_ASM({
//        Module.SetMouseVisible($0);
//    }, mouseVisible);
//    URHO3D_LOGINFOF("Mouse mode changed visibility = %d, mouseMode = %u", mouseVisible, mouseMode);
//}
//
//using namespace emscripten;
//EMSCRIPTEN_BINDINGS(Module) {
//    function("JSCanvasSize", &BaseApplication::JSCanvasSize);
//    function("JSMouseFocus", &BaseApplication::JSMouseFocus);
//}
//#endif

void BaseApplication::Start()
{
    GetSubsystem<ServiceCmd>()->Init();
    UI* ui = GetSubsystem<UI>();
    auto cache = GetSubsystem<ResourceCache>();
//    XMLFile* style = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
//    if(!GetSubsystem<UI>()->GetCursor())
//        GetSubsystem<UI>()->SetCursor(new Cursor(context_));
//    GetSubsystem<UI>()->GetCursor()->SetStyleAuto(style);
//    GetSubsystem<UI>()->GetCursor()->SetShape(CursorShape::CS_NORMAL);
//    GetSubsystem<UI>()->GetCursor()->SetVisible(true);
//    GetSubsystem<UI>()->GetCursor()->SetPosition(GetSubsystem<Input>()->GetMousePosition());
//#if defined(__EMSCRIPTEN__)
#ifdef __ANDROID__
    ui->SetScale(1.8);
#else
    ui->SetScale(GetSubsystem<ConfigManager>()->GetFloat("engine", "UIScale", 1.0));
#endif

#ifdef __APPLE__
    ui->SetScale(2.0);
    GetSubsystem<ConfigManager>()->Set("engine", "HighDPI", true);
#endif

    // TODO detect highDPI first
    // For high DPI we must increase the UI scale 2 times
//    if (GetSubsystem<ConfigManager>()->GetBool("engine", "HighDPI", false)) {
//        ui->SetScale(ui->GetScale() * 2.0);
//    }
    if (!GetSubsystem<Engine>()->IsHeadless()) {
        GetSubsystem<ConsoleHandler>()->Create();
        DebugHud* debugHud = GetSubsystem<Engine>()->CreateDebugHud();
        XMLFile* xmlFile = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
        debugHud->SetDefaultStyle(xmlFile);
    }

    cache->SetAutoReloadResources(true);
    ui->GetRoot()->SetDefaultStyle(cache->GetResource<XMLFile>("UI/DefaultStyle.xml"));

    SubscribeToEvents();

    GetSubsystem<FileSystem>()->SetExecuteConsoleCommands(false);

    context_->RegisterSubsystem(new LevelManager(context_));
    context_->RegisterSubsystem(new WindowManager(context_));
    context_->RegisterSubsystem(new Achievements(context_));
    context_->RegisterSubsystem(new Generator(context_));

#if defined(URHO3D_LUA) || defined(URHO3D_ANGELSCRIPT)
    context_->RegisterSubsystem(new ModLoader(context_));
#endif

    context_->RegisterSubsystem(new AudioManager(context_));

    // Allow multiple music tracks to play at the same time
    context_->GetSubsystem<AudioManager>()->AllowMultipleMusicTracks(true);
    // Allow multiple ambient tracks to play at the same time
    context_->GetSubsystem<AudioManager>()->AllowMultipleAmbientTracks(true);

    auto controllerInput = new ControllerInput(context_);
    context_->RegisterSubsystem(controllerInput);
    controllerInput->LoadConfig();

    context_->RegisterSubsystem(new Notifications(context_));

    RegisterConsoleCommands();

    ApplyGraphicsSettings();

    // Initialize the first level from the config file
    VariantMap& eventData = GetEventDataMap();
    if (GetSubsystem<ConfigManager>()->GetBool("dedicated_server", "enabled", false)) {
        eventData["Name"] = "Loading";
        eventData["StartServer"] = true;
        eventData["Map"] = GetSubsystem<ConfigManager>()->GetString("dedicated_server", "map", "Scenes/Flat.xml");
    } else {
        eventData["Name"] = GetSubsystem<ConfigManager>()->GetString("game", "FirstLevel", "Splash");
    }
    SendEvent(E_SET_LEVEL, eventData);

    LoadTranslationFiles();

    SendEvent("GameStarted");
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
                globalSettings_[StringHash((*it).first_)] = (*it).first_;
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
    VariantMap& data = GetEventDataMap();
    data["ConsoleCommandName"]        = "exit";
    data["ConsoleCommandEvent"]       = "HandleExit";
    data["ConsoleCommandDescription"] = "Exits game";
    SendEvent("ConsoleCommandAdd", data);

    SubscribeToEvent("HandleExit", URHO3D_HANDLER(BaseApplication, HandleExit));

    SendEvent(E_CONSOLE_COMMAND_ADD, ConsoleCommandAdd::P_NAME, "debugger", ConsoleCommandAdd::P_EVENT, "#debugger", ConsoleCommandAdd::P_DESCRIPTION, "Show debug");
    SubscribeToEvent("#debugger", [&](StringHash eventType, VariantMap& eventData) {
        if (GetSubsystem<DebugHud>()) {
            GetSubsystem<DebugHud>()->Toggle(DEBUGHUD_SHOW_STATS | DEBUGHUD_SHOW_MEMORY | DEBUGHUD_SHOW_EVENTPROFILER);
        }
    });

    SendEvent(E_CONSOLE_COMMAND_ADD, ConsoleCommandAdd::P_NAME, "mouse_visible", ConsoleCommandAdd::P_EVENT, "#mouse_visible", ConsoleCommandAdd::P_DESCRIPTION, "Toggle mouse visible");
    SubscribeToEvent("#mouse_visible", [&](StringHash eventType, VariantMap& eventData) {
        Input* input = GetSubsystem<Input>();
        if (input->IsMouseVisible()) {
            input->SetMouseVisible(!input->IsMouseVisible());
        }
    });
}

void BaseApplication::HandleExit(StringHash eventType, VariantMap& eventData)
{
    GetSubsystem<Engine>()->Exit();
}

void BaseApplication::SubscribeToEvents()
{
    SubscribeToEvent(E_ADD_CONFIG, URHO3D_HANDLER(BaseApplication, HandleAddConfig));
    SubscribeToEvent(E_LOAD_CONFIG, URHO3D_HANDLER(BaseApplication, HandleLoadConfig));

    SubscribeToEvent(E_MAPPED_CONTROL_RELEASED, [&](StringHash eventType, VariantMap& eventData) {
        using namespace MappedControlReleased;
        int action = eventData[P_ACTION].GetInt();
        if (action == CTRL_SCREENSHOT) {
            Graphics *graphics = GetSubsystem<Graphics>();
            Image screenshot(context_);
            graphics->TakeScreenShot(screenshot);
            // Here we save in the Data folder with date and time appended
            screenshot.SavePNG(GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Screenshot_" +
                               Time::GetTimeStamp().Replaced(':', '_').Replaced('.', '_').Replaced(' ', '_') + ".png");

            SendEvent("ScreenshotTaken");
        }
    });
}

void BaseApplication::HandleAddConfig(StringHash eventType, VariantMap& eventData)
{
    String paramName = eventData["Name"].GetString();
    if (!paramName.Empty()) {
        globalSettings_[paramName] = paramName;
    }
}

void BaseApplication::LoadINIConfig(String filename)
{
    bool loaded = GetSubsystem<ConfigManager>()->Load(filename, true);

    SetEngineParameter(EP_MONITOR, GetSubsystem<ConfigManager>()->GetInt("video", "Monitor", 0));
    int windowMode = GetSubsystem<ConfigManager>()->GetInt("video", "WindowMode", 0);
    SetEngineParameter(EP_FULL_SCREEN, windowMode == 2);
    SetEngineParameter(EP_BORDERLESS, windowMode == 1);

    // Dedicated server - headless mode
    SetEngineParameter(EP_HEADLESS, GetSubsystem<ConfigManager>()->GetBool("dedicated_server", "enabled", false));

    SetEngineParameter(EP_WINDOW_WIDTH, GetSubsystem<ConfigManager>()->GetInt("video", "Width", 1280));
    SetEngineParameter(EP_WINDOW_HEIGHT, GetSubsystem<ConfigManager>()->GetInt("video", "Height", 720));
    SetEngineParameter(EP_VSYNC, GetSubsystem<ConfigManager>()->GetBool("video", "VSync", true));
    SetEngineParameter(EP_REFRESH_RATE, GetSubsystem<ConfigManager>()->GetInt("video", "RefreshRate", 60));
#ifdef __EMSCRIPTEN
    SetEngineParameter(EP_WINDOW_RESIZABLE, true);
#else
    SetEngineParameter(EP_WINDOW_RESIZABLE, GetSubsystem<ConfigManager>()->GetBool("video", "ResizableWindow", false));
    SetEngineParameter(EP_BORDERLESS, false);
#endif


    // Engine settings
    GetSubsystem<Engine>()->SetMaxFps(GetSubsystem<ConfigManager>()->GetInt("engine", "FPSLimit", 60));
    SetEngineParameter(EP_HIGH_DPI, GetSubsystem<ConfigManager>()->GetBool("engine", "HighDPI", false));
    SetEngineParameter(EP_WINDOW_TITLE, "ProjectTemplate");
    SetEngineParameter(EP_WINDOW_ICON, "Textures/AppIcon.png");

    // Logs
    SetEngineParameter(EP_LOG_NAME, GetSubsystem<ConfigManager>()->GetString("engine", "LogName", "ProjectTemplate.log"));
    SetEngineParameter(EP_LOG_LEVEL, GetSubsystem<ConfigManager>()->GetInt("engine", "LogLevel", LOG_INFO));
    SetEngineParameter(EP_LOG_QUIET, GetSubsystem<ConfigManager>()->GetBool("engine", "LogQuiet", false));
    SetEngineParameter(EP_RESOURCE_PATHS, GetSubsystem<ConfigManager>()->GetString("engine", "ResourcePaths", "Data;CoreData"));
    SetEngineParameter(EP_FLUSH_GPU, GetSubsystem<ConfigManager>()->GetBool("engine", "FlushGPU", true));
    SetEngineParameter(EP_WORKER_THREADS, GetSubsystem<ConfigManager>()->GetBool("engine", "WorkerThreads ", true));

    SetEngineParameter("Master" , GetSubsystem<ConfigManager>()->GetFloat("audio", "Master", 1.0));
    SetEngineParameter("Effect", GetSubsystem<ConfigManager>()->GetFloat("audio", "Effect", 1.0));
    SetEngineParameter("Ambient", GetSubsystem<ConfigManager>()->GetFloat("audio", "Ambient", 1.0));
    SetEngineParameter("Voice", GetSubsystem<ConfigManager>()->GetFloat("audio", "Voice", 1.0));
    SetEngineParameter("Music", GetSubsystem<ConfigManager>()->GetFloat("audio", "Music", 1.0));


    Audio* audio = GetSubsystem<Audio>();

    audio->SetMasterGain(SOUND_MASTER, engine_->GetGlobalVar("Master").GetFloat());
    audio->SetMasterGain(SOUND_EFFECT, engine_->GetGlobalVar("Effect").GetFloat());
    audio->SetMasterGain(SOUND_AMBIENT, engine_->GetGlobalVar("Ambient").GetFloat());
    audio->SetMasterGain(SOUND_VOICE, engine_->GetGlobalVar("Voice").GetFloat());
    audio->SetMasterGain(SOUND_MUSIC, engine_->GetGlobalVar("Music").GetFloat());

    GetSubsystem<Log>()->SetTimeStamp(GetSubsystem<ConfigManager>()->GetBool("engine", "LogTimestamp", true));
}

void BaseApplication::ApplyGraphicsSettings()
{
    auto* renderer = GetSubsystem<Renderer>();
    if (renderer) {
        renderer->SetTextureQuality(
                (Urho3D::MaterialQuality) GetSubsystem<ConfigManager>()->GetInt("graphics", "TextureQuality",
                                                                                MaterialQuality::QUALITY_HIGH));
        renderer->SetMaterialQuality(
                (Urho3D::MaterialQuality) GetSubsystem<ConfigManager>()->GetInt("graphics", "MaterialQuality",
                                                                                MaterialQuality::QUALITY_MAX));
        renderer->SetDrawShadows(GetSubsystem<ConfigManager>()->GetBool("graphics", "DrawShadows", true));
        renderer->SetShadowMapSize(GetSubsystem<ConfigManager>()->GetInt("graphics", "ShadowMapSize", 512));
        renderer->SetShadowQuality((ShadowQuality) GetSubsystem<ConfigManager>()->GetInt("graphics", "ShadowQuality",
                                                                                         ShadowQuality::SHADOWQUALITY_PCF_16BIT));
        renderer->SetMaxOccluderTriangles(
                GetSubsystem<ConfigManager>()->GetInt("graphics", "MaxOccluderTriangles", 5000));
        renderer->SetDynamicInstancing(GetSubsystem<ConfigManager>()->GetBool("graphics", "DynamicInstancing", true));
        renderer->SetSpecularLighting(GetSubsystem<ConfigManager>()->GetBool("graphics", "SpecularLighting", true));
        renderer->SetHDRRendering(GetSubsystem<ConfigManager>()->GetBool("graphics", "HDRRendering", false));
    }
}

void BaseApplication::SetEngineParameter(String parameter, Variant value)
{
    engineParameters_[parameter] = value;
    engine_->SetGlobalVar(parameter, value);
    globalSettings_[parameter] = parameter;
}

void BaseApplication::LoadTranslationFiles()
{
    Vector<String> result;
    auto* localization = GetSubsystem<Localization>();

    // Get all translation files in the Data/Translations folder
    GetSubsystem<FileSystem>()->ScanDir(result, GetSubsystem<FileSystem>()->GetProgramDir() + String("Data/Translations"), String("*.json"), SCAN_FILES, true);

    // Get all the translations from packages
    auto packageFiles = GetSubsystem<ResourceCache>()->GetPackageFiles();
    for (auto it = packageFiles.Begin(); it != packageFiles.End(); ++it) {
        auto files = (*it)->GetEntryNames();
        for (auto it2 = files.Begin(); it2 != files.End(); ++it2) {
            if ((*it2).StartsWith("Translations/") && (*it2).EndsWith(".json") && (*it2).Split('/')  .Size() == 2) {
                result.Push((*it2).Split('/').At(1));
            }
        }
    }

    for (auto it = result.Begin(); it != result.End(); ++it) {
        String file = (*it);

        String filepath = "Translations/" + file;
        // Filename is handled as a language
        file.Replace(".json", "", false);

        auto jsonFile = GetSubsystem<ResourceCache>()->GetResource<JSONFile>(filepath);
        if (jsonFile) {
            // Load the actual file in the system
            localization->LoadSingleLanguageJSON(jsonFile->GetRoot(), file);
            URHO3D_LOGINFO("Loading translation file '" + filepath + "' to '" + file + "' language");
        } else {
            URHO3D_LOGERROR("Translation file '" + filepath + "' not found!");
        }
    }

    // Finally set the application language
    localization->SetLanguage(GetSubsystem<ConfigManager>()->GetString("game", "Language", "EN"));
}
