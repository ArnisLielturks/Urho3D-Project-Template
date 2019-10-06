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
#include "MyEvents.h"

using namespace Urho3D;

/**
 * Wait this many MS before marking loading step as completed if no ACK request was received
 */
const int LOADING_STEP_ACK_MAX_TIME = 2000; // Max wait time in MS for ACK message for loading step
const int LOADING_STEP_MAX_EXECUTION_TIME = 3000; // Max loading step execution time in MS, 0 - infinite
const float PROGRESS_SPEED = 0.3f; // how fast should the progress bar increase each second, 1 - load 0 to 100% in 1 second

SceneManager::SceneManager(Context* context) :
        Object(context),
        progress(0),
        targetProgress(0)
{
    SubscribeToEvent(MyEvents::E_REGISTER_LOADING_STEP, URHO3D_HANDLER(SceneManager, HandleRegisterLoadingStep));
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

    GetSubsystem<DebugHud>()->SetAppStats("Scene manager map", filename);
    URHO3D_LOGINFO("Scene manager loading scene: " + filename);

    SubscribeToEvent(_activeScene, E_ASYNCLOADPROGRESS, URHO3D_HANDLER(SceneManager, HandleAsyncSceneLoadingProgress));
    SubscribeToEvent(_activeScene, E_ASYNCEXECFINISHED, URHO3D_HANDLER(SceneManager, HandleAsyncSceneLoadingFinished));
    SubscribeToEvent(_activeScene, E_ASYNCLOADFINISHED, URHO3D_HANDLER(SceneManager, HandleAsyncSceneLoadingFinished));
}

void SceneManager::HandleAsyncSceneLoadingProgress(StringHash eventType, VariantMap& eventData)
{
    using namespace  AsyncLoadProgress;
    //progress = eventData[P_PROGRESS].GetFloat();
    int nodesLoaded = eventData[P_LOADEDNODES].GetInt();
    int totalNodes = eventData[P_TOTALNODES].GetInt();
    int resourcesLoaded = eventData[P_LOADEDRESOURCES].GetInt();
    int totalResources = eventData[P_TOTALRESOURCES].GetInt();

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

    SubscribeToEvent(MyEvents::E_ACK_LOADING_STEP, URHO3D_HANDLER(SceneManager, HandleLoadingStepAck));
    SubscribeToEvent(MyEvents::E_LOADING_STEP_PROGRESS, URHO3D_HANDLER(SceneManager, HandleLoadingStepProgress));
    SubscribeToEvent(MyEvents::E_LOADING_STEP_FINISHED, URHO3D_HANDLER(SceneManager, HandleLoadingStepFinished));

    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(SceneManager, HandleUpdate));
    URHO3D_LOGINFO("Scene loaded: " + _activeScene->GetFileName());
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
            (*it).second_.ack = true;
            URHO3D_LOGINFO("Loading step skipped, no ACK retrieved for " + (*it).second_.name);
        }
        targetProgress = (float)completed / ( (float) _loadingSteps.Size() + 1.0f );

        if (!(*it).second_.finished) {

            // Handle loading steps which take too much time to execute
            if (LOADING_STEP_MAX_EXECUTION_TIME > 0 && (*it).second_.ackSent && (*it).second_.ackTimer.GetMSec(false) > LOADING_STEP_MAX_EXECUTION_TIME + LOADING_STEP_ACK_MAX_TIME) {
                (*it).second_.finished = true;
                (*it).second_.failed = true;
                URHO3D_LOGERROR("Loading step '" + (*it).second_.name + "' failed, took too long to execute!");

                // Note the the tasks could still succeed in the background, but the loading screen will move further without waiting it to finish
                return;
            }
            if (!(*it).second_.ackSent) {

                // Send out event to start this loading step
                _loadingStatus = (*it).second_.name;
                SendEvent((*it).second_.event);

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

        UnsubscribeFromEvent(MyEvents::E_ACK_LOADING_STEP);
        UnsubscribeFromEvent(MyEvents::E_LOADING_STEP_PROGRESS);
        UnsubscribeFromEvent(MyEvents::E_LOADING_STEP_FINISHED);
    }
}

void SceneManager::ResetProgress()
{
    progress = 0.0f;
    targetProgress = 0.0f;

    for (auto it = _loadingSteps.Begin(); it != _loadingSteps.End(); ++it) {
        (*it).second_.finished = false;
        (*it).second_.ack = false;
        (*it).second_.ackSent = false;
        (*it).second_.progress = 0.0f;
    }
}

void SceneManager::HandleRegisterLoadingStep(StringHash eventType, VariantMap& eventData)
{
    using namespace MyEvents::RegisterLoadingStep;
    LoadingStep step;
    step.name = eventData[P_NAME].GetString();
    step.event = eventData[P_EVENT].GetString();
    step.ackSent = false;
    step.finished = false;
    step.failed = false;
    step.progress = 0;
    if (step.name.Empty() || step.event.Empty()) {
        URHO3D_LOGERROR("Unable to register loading step " + step.name + ":" + step.event);
        return;
    }

    URHO3D_LOGINFO("Registering new loading step: " + step.name + "; " + step.event);
    _loadingSteps[step.event] = step;

    GetSubsystem<DebugHud>()->SetAppStats("Loading steps", _loadingSteps.Size());
}

void SceneManager::HandleLoadingStepAck(StringHash eventType, VariantMap& eventData)
{
    //
    using namespace MyEvents::AckLoadingStep;
    String name = eventData[P_EVENT].GetString();
    _loadingSteps[name].ack = true;
    _loadingSteps[name].loadTime.Reset();
    URHO3D_LOGINFO("Loading step  '" + name + "' acknowlished");
}

void SceneManager::HandleLoadingStepProgress(StringHash eventType, VariantMap& eventData)
{
    using namespace MyEvents::LoadingStepProgress;
    String event = eventData[P_EVENT].GetString();
    float progress = eventData[P_PROGRESS].GetFloat();
    progress = Clamp(progress, 0.0f, 1.0f);
    _loadingSteps[event].progress = progress;

    URHO3D_LOGINFO("Loading step progress update '" + event + "' : " + String(progress));
}

void SceneManager::HandleLoadingStepFinished(StringHash eventType, VariantMap& eventData)
{
    using namespace MyEvents::LoadingStepFinished;
    String event = eventData[P_EVENT].GetString();
    _loadingSteps[event].finished = true;

    URHO3D_LOGINFO("Loading step " + event + " finished");
}