#include "SceneManager.h"

using namespace Urho3D;

static int loadingIndex = 0;

SceneManager::SceneManager(Context* context) :
        Object(context),
        progress(0)
{
}

SceneManager::~SceneManager()
{
}

void SceneManager::LoadScene(const String& filename)
{
    _activeScene.Reset();
    _activeScene = new Scene(context_);
    _activeScene->SetAsyncLoadingMs(1);
    _activeScene->LoadAsyncXML(new File(context_, filename, FILE_READ));
    _loadingStatus = "Loading scene";
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

    // Imitate slower loading
    progress = 0.0f;

    _activeScene->SetUpdateEnabled(false);

    GetSubsystem<Script>()->SetDefaultScene(_activeScene);

    _loadingStatus = "Creating objects";
    UnsubscribeFromEvent(E_ASYNCLOADPROGRESS);
    UnsubscribeFromEvent(E_ASYNCLOADFINISHED);

    auto* cache = GetSubsystem<ResourceCache>();
    const unsigned NUM_OBJECTS = 10;
    for (unsigned i = 0; i < NUM_OBJECTS; ++i)
    {
    }

    _activeScene->SetUpdateEnabled(true);
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(SceneManager, HandleUpdate));
    URHO3D_LOGINFO("Scene loaded: " + _activeScene->GetFileName());
}

void SceneManager::HandleUpdate(StringHash eventType, VariantMap& eventData)
{

    // Imitate slower loading with multiple loading steps
    if (_timer.GetMSec(false) > 300) {
        static const char *Messages[] = {
            "Smelling flowers",
            "Improving CPU skills",
            "Discussing religion",
            "Retrieving sensitive user information",
            "Gaining profit",
            "Loosing trust in people",
            "Meditating"
        };
        if (loadingIndex > 6) {
            loadingIndex = 6;
            progress = 1.0f;
        }
        _loadingStatus = Messages[loadingIndex++];
        _timer.Reset();
    }

    using namespace Update;
    progress += eventData[P_TIMESTEP].GetFloat() / 2.f;
    if (progress >= 1.0f) {
        progress = 1.0f;
    }
}

void SceneManager::ResetProgress()
{
    progress = 0.0f;
    loadingIndex = 0;
}