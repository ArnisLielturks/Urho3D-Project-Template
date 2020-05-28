#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Engine/DebugHud.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/IO/IOEvents.h>

#ifdef URHO3D_ANGELSCRIPT
#include <Urho3D/AngelScript/Script.h>
#endif
#include <Urho3D/Core/CoreEvents.h>
#include "SceneManager.h"
#include "SceneManagerEvents.h"
#include "Console/ConsoleHandlerEvents.h"
#include "LevelManagerEvents.h"

using namespace Urho3D;
using namespace ConsoleHandlerEvents;
using namespace SceneManagerEvents;
using namespace LevelManagerEvents;

/**
 * Wait this many MS before marking loading step as completed if no ACK request was received
 */
const int LOADING_STEP_ACK_MAX_TIME       = 2000; // Max wait time in MS for ACK message for loading step
const int LOADING_STEP_MAX_EXECUTION_TIME = 10 * 1000; // Max loading step execution time in MS, 0 - infinite
const float PROGRESS_SPEED                = 0.3f; // how fast should the progress bar increase each second, e.g. 1 would load 0 to 100% in 1 second

SceneManager::SceneManager(Context* context) :
        Object(context),
        progress(0),
        targetProgress(0)
{
    SubscribeToEvent(E_REGISTER_LOADING_STEP, URHO3D_HANDLER(SceneManager, HandleRegisterLoadingStep));
    SubscribeToEvent(E_ADD_MAP, URHO3D_HANDLER(SceneManager, HandleAddMap));
    SubscribeToEvent(E_LOADING_STEP_CRITICAL_FAIL, [&](StringHash eventType, VariantMap &eventData) {
        UnsubscribeFromEvent(E_UPDATE);
        using namespace LoadingStepCriticalFail;
        VariantMap& data = GetEventDataMap();
        data["Name"] = "MainMenu";
        data["Type"] = "error";
        data["Message"] = eventData[P_DESCRIPTION];
        SendEvent(E_SET_LEVEL, data);
    });

    LoadDefaultMaps();
}

SceneManager::~SceneManager()
{
}

void SceneManager::LoadScene(const String& filename)
{
    ResetProgress();
    _activeScene.Reset();
    _activeScene = new Scene(context_);
    _activeScene->SetAsyncLoadingMs(1);
    auto xmlFile = GetSubsystem<ResourceCache>()->GetFile(filename);
    _activeScene->LoadAsyncXML(xmlFile);
    _loadingStatus = "Loading scene";

    if (GetSubsystem<DebugHud>()) {
        GetSubsystem<DebugHud>()->SetAppStats("Scene manager map", filename);
    }
    URHO3D_LOGINFO("Scene manager loading scene: " + filename);

    SubscribeToEvent(_activeScene, E_ASYNCLOADPROGRESS, URHO3D_HANDLER(SceneManager, HandleAsyncSceneLoadingProgress));
    SubscribeToEvent(_activeScene, E_ASYNCEXECFINISHED, URHO3D_HANDLER(SceneManager, HandleAsyncSceneLoadingFinished));
    SubscribeToEvent(_activeScene, E_ASYNCLOADFINISHED, URHO3D_HANDLER(SceneManager, HandleAsyncSceneLoadingFinished));

    SendEvent(E_CONSOLE_COMMAND_ADD, ConsoleCommandAdd::P_NAME, "remove_local_nodes", ConsoleCommandAdd::P_EVENT, "#remove_local_nodes", ConsoleCommandAdd::P_DESCRIPTION, "Remove all local nodes");
    SubscribeToEvent("#remove_local_nodes", [&](StringHash eventType, VariantMap& eventData) {
        if (_activeScene) {
            _activeScene->Clear(false, true);
        }
    });

    SendEvent(E_CONSOLE_COMMAND_ADD, ConsoleCommandAdd::P_NAME, "remove_replicated_nodes", ConsoleCommandAdd::P_EVENT, "#remove_replicated_nodes", ConsoleCommandAdd::P_DESCRIPTION, "Remove all replicated nodes");
    SubscribeToEvent("#remove_replicated_nodes", [&](StringHash eventType, VariantMap& eventData) {
        if (_activeScene) {
            _activeScene->Clear(true, false);
        }
    });
}

void SceneManager::HandleAsyncSceneLoadingProgress(StringHash eventType, VariantMap& eventData)
{
    using namespace  AsyncLoadProgress;
    //progress = eventData[P_PROGRESS].GetFloat();
    int nodesLoaded     = eventData[P_LOADEDNODES].GetInt();
    int totalNodes      = eventData[P_TOTALNODES].GetInt();
    int resourcesLoaded = eventData[P_LOADEDRESOURCES].GetInt();
    int totalResources  = eventData[P_TOTALRESOURCES].GetInt();

    URHO3D_LOGINFOF("Loading progress %f %i/%i %i/%i", progress, nodesLoaded, totalNodes, resourcesLoaded, totalResources);
}

void SceneManager::HandleAsyncSceneLoadingFinished(StringHash eventType, VariantMap& eventData)
{
    using namespace AsyncLoadFinished;

    _activeScene->SetUpdateEnabled(false);

#ifdef URHO3D_ANGELSCRIPT
    if (GetSubsystem<Script>()) {
        GetSubsystem<Script>()->SetDefaultScene(_activeScene);
    }
#endif

    UnsubscribeFromEvent(E_ASYNCLOADPROGRESS);
    UnsubscribeFromEvent(E_ASYNCLOADFINISHED);

    SubscribeToEvent(E_ACK_LOADING_STEP, URHO3D_HANDLER(SceneManager, HandleLoadingStepAck));
    SubscribeToEvent(E_LOADING_STEP_PROGRESS, URHO3D_HANDLER(SceneManager, HandleLoadingStepProgress));
    SubscribeToEvent(E_LOADING_STEP_FINISHED, URHO3D_HANDLER(SceneManager, HandleLoadingStepFinished));
    SubscribeToEvent(E_LOADING_STEP_SKIP, URHO3D_HANDLER(SceneManager, HandleSkipLoadingStep));

    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(SceneManager, HandleUpdate));
    URHO3D_LOGINFO("Scene loaded: " + _activeScene->GetFileName());
}

void SceneManager::CleanupLoadingSteps()
{
    for (auto it = _loadingSteps.Begin(); it != _loadingSteps.End(); ++it) {
        if ((*it).second_.autoRemove) {
            URHO3D_LOGINFOF("Auto removing loading step %s", (*it).second_.name.CString());
            _loadingSteps.Erase(it);
            it--;
        }
    }
}

void SceneManager::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;
    progress += eventData[P_TIMESTEP].GetFloat() * PROGRESS_SPEED;
    if (progress > targetProgress) {
        progress = targetProgress;
    }

    float completed = 1;
    targetProgress = (float)completed / ( (float) _loadingSteps.Size() + 1.0f );
    for (auto it = _loadingSteps.Begin(); it != _loadingSteps.End(); ++it) {
        if ((*it).second_.finished) {
            completed++;
        }
        if ((*it).second_.ackSent && !(*it).second_.ack && (*it).second_.ackTimer.GetMSec(false) > LOADING_STEP_ACK_MAX_TIME) {
            (*it).second_.finished = true;
            (*it).second_.ack      = true;
        }
        targetProgress = (float)completed / ( (float) _loadingSteps.Size() + 1.0f );

        if (CanLoadingStepRun((*it).second_)) {

            //TODO: implement fix for web builds as the loading steps might take longer to execute
            // due to the inactive browsers tabs where game is running in the background
            // Handle loading steps which take too much time to execute
            if ((*it).second_.ackSent && (*it).second_.ackTimer.GetMSec(false) > LOADING_STEP_MAX_EXECUTION_TIME + LOADING_STEP_ACK_MAX_TIME) {
                (*it).second_.finished = true;
                (*it).second_.failed   = true;
                URHO3D_LOGERROR("Loading step '" + (*it).second_.name + "' failed, took too long to execute!");

                using namespace LoadingStepTimedOut;
                VariantMap& data = GetEventDataMap();
                data[P_EVENT] = (*it).second_.event;
                SendEvent(E_LOADING_STEP_TIMED_OUT, data);
                URHO3D_LOGINFO("Loading step skipped, no ACK retrieved for " + (*it).second_.name);
                return;

                // Note the the tasks could still succeed in the background, but the loading screen will move further without waiting it to finish
                return;
            }
            if (!(*it).second_.ackSent) {

                if (_loadingStatus != (*it).second_.name) {
                    _loadingStatus = (*it).second_.name;
                    // Delay loading step execution till the next frame
                    // to allow the status to be updated
                    using namespace LoadingStatusUpdate;
                    VariantMap data;
                    data[P_NAME] = (*it).second_.name;
                    SendEvent(E_LOADING_STATUS_UPDATE, data);
                    return;
                }

                VariantMap data;
                data["Map"] = _activeScene->GetFileName();
                // Send out event to start this loading step
                SendEvent((*it).second_.event, data);

                // We register that start event was sent out, loading step must send back ACK message
                // to let us know that the loading step was started, otherwise it will be automatically
                // marked as a finished job, to avoid app inifite loading
                (*it).second_.ackSent = true;
                (*it).second_.ackTimer.Reset();
            }
            completed += (*it).second_.progress;
            targetProgress = (float)completed / ( (float) _loadingSteps.Size() + 1.0f );
            return;
        }

    }

    if (progress >= 1.0f) {
        progress = 1.0f;
        UnsubscribeFromEvent(E_UPDATE);

        // Re-enable active scene
        _activeScene->SetUpdateEnabled(true);

        UnsubscribeFromEvent(E_ACK_LOADING_STEP);
        UnsubscribeFromEvent(E_LOADING_STEP_PROGRESS);
        UnsubscribeFromEvent(E_LOADING_STEP_FINISHED);
        UnsubscribeFromEvent(E_LOADING_STEP_SKIP);

        CleanupLoadingSteps();
    }
}

void SceneManager::ResetProgress()
{
    progress       = 0.0f;
    targetProgress = 0.0f;

    for (auto it = _loadingSteps.Begin(); it != _loadingSteps.End(); ++it) {
        (*it).second_.finished = false;
        (*it).second_.ack      = false;
        (*it).second_.ackSent  = false;
        (*it).second_.progress = 0.0f;
    }
}

void SceneManager::HandleRegisterLoadingStep(StringHash eventType, VariantMap& eventData)
{
    using namespace RegisterLoadingStep;
    LoadingStep step;
    step.name     = eventData[P_NAME].GetString();
    step.event    = eventData[P_EVENT].GetString();
    step.ackSent  = false;
    step.finished = false;
    step.failed   = false;
    step.progress = 0;
    step.autoRemove = false;
    step.dependsOn = eventData[P_DEPENDS_ON].GetStringVector();

    if (eventData.Contains(P_REMOVE_ON_FINISH) && eventData[P_REMOVE_ON_FINISH].GetBool()) {
        step.autoRemove = true;
    }
    if (step.name.Empty() || step.event.Empty()) {
        URHO3D_LOGERROR("Unable to register loading step " + step.name + ":" + step.event);
        return;
    }

    URHO3D_LOGINFO("Registering new loading step: " + step.name + "; " + step.event);
    _loadingSteps[step.event] = step;

    if (GetSubsystem<DebugHud>()) {
        GetSubsystem<DebugHud>()->SetAppStats("Loading steps", _loadingSteps.Size());
    }
}

void SceneManager::HandleLoadingStepAck(StringHash eventType, VariantMap& eventData)
{
    //
    using namespace AckLoadingStep;
    String name = eventData[P_EVENT].GetString();

    _loadingSteps[name].ack = true;
    _loadingSteps[name].loadTime.Reset();
    URHO3D_LOGINFO("Loading step  '" + name + "' acknowlished");
}

void SceneManager::HandleLoadingStepProgress(StringHash eventType, VariantMap& eventData)
{
    using namespace LoadingStepProgress;
    String event   = eventData[P_EVENT].GetString();
    float progress = eventData[P_PROGRESS].GetFloat();
    progress       = Clamp(progress, 0.0f, 1.0f);
    _loadingSteps[event].progress = progress;

    URHO3D_LOGINFO("Loading step progress update '" + event + "' : " + String(progress));
}

void SceneManager::HandleLoadingStepFinished(StringHash eventType, VariantMap& eventData)
{
    using namespace LoadingStepFinished;
    String event = eventData[P_EVENT].GetString();

    _loadingSteps[event].finished = true;

    URHO3D_LOGINFO("Loading step " + event + " finished");
}

void SceneManager::HandleSkipLoadingStep(StringHash eventType, VariantMap& eventData)
{
    using namespace LoadingStepSkip;
    String event = eventData[P_EVENT].GetString();
    if (_loadingSteps.Contains(event)) {
        _loadingSteps[event].finished = true;
    }
}

MapInfo* SceneManager::GetMap(const String& filename)
{
    for (auto it = _availableMaps.Begin(); it != _availableMaps.End(); ++it) {
        if ((*it).map == filename) {
            return &(*it);
        }
    }
    return nullptr;
}

void SceneManager::HandleAddMap(StringHash eventType, VariantMap &eventData)
{
    using namespace AddMap;
    String map = eventData[P_MAP].GetString();

    if (GetMap(map)) {
        URHO3D_LOGWARNINGF("Map '%s' already added to the list", map.CString());
        return;
    }

    MapInfo mapData;
    mapData.map = map;
    mapData.name = eventData[P_NAME].GetString();
    mapData.description = eventData[P_DESCRIPTION].GetString();
    mapData.image = eventData[P_IMAGE].GetString();
    mapData.startPoint = eventData[P_START_POINT].GetVector3();
    mapData.commands = eventData[P_COMMANDS].GetStringVector();
    _availableMaps.Push(mapData);

    URHO3D_LOGINFOF("Map '%s' added to the list", map.CString());
}

bool SceneManager::CanLoadingStepRun(LoadingStep& loadingStep)
{
    if (loadingStep.finished) {
        return false;
    }

    if (!loadingStep.dependsOn.Empty()) {
        for (auto it = loadingStep.dependsOn.Begin(); it != loadingStep.dependsOn.End(); ++it) {
            String eventName = (*it);
            if (_loadingSteps.Contains(eventName) && !_loadingSteps[eventName].finished) {
                return false;
            }
        }
    }

    return true;
}

void SceneManager::CleanupScene()
{
    if (_activeScene) {
        _activeScene->SetUpdateEnabled(false);
        _activeScene->Clear();
        _activeScene->Remove();
        _activeScene.Reset();
    }
}

const Vector<MapInfo>& SceneManager::GetAvailableMaps() const
{
    return _availableMaps;
}

void SceneManager::LoadDefaultMaps()
{
    using namespace AddMap;
    VariantMap& data = GetEventDataMap();
    data[P_MAP] = "Scenes/Flat.xml";
    data[P_NAME] = "Flatland";
    data[P_DESCRIPTION] = "Flat map where you could easily fall off";
    data[P_IMAGE] = "Textures/Box.png";

    StringVector commands;
    commands.Push("ambient_light 0.1 0.1 0.1");
    data[P_COMMANDS] = commands;
    SendEvent(E_ADD_MAP, data);
}

const MapInfo* SceneManager::GetCurrentMapInfo() const
{
    return _currentMap;
}
