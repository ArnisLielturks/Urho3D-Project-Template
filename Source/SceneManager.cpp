#include "SceneManager.h"

using namespace Urho3D;

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

    URHO3D_LOGINFO("Scene manager loading scene: " + filename);

    SubscribeToEvent(_activeScene, E_ASYNCLOADPROGRESS, URHO3D_HANDLER(SceneManager, HandleAsyncSceneLoadingProgress));
    SubscribeToEvent(_activeScene, E_ASYNCLOADFINISHED, URHO3D_HANDLER(SceneManager, HandleAsyncSceneLoadingFinished));
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(SceneManager, HandleUpdate));
}

void SceneManager::HandleAsyncSceneLoadingProgress(StringHash eventType, VariantMap& eventData)
{
    using namespace  AsyncLoadProgress;
    progress = eventData[P_PROGRESS].GetFloat();
    int nodesLoaded = eventData[P_LOADEDNODES].GetInt();
    int totalNodes = eventData[P_TOTALNODES].GetInt();
    int resourcesLoaded = eventData[P_LOADEDRESOURCES].GetInt();
    int totalResources = eventData[P_TOTALRESOURCES].GetInt();

    URHO3D_LOGINFOF("Loading progress %f %i/%i %i/%i", progress, nodesLoaded, totalNodes, resourcesLoaded, totalResources);
}

void SceneManager::HandleAsyncSceneLoadingFinished(StringHash eventType, VariantMap& eventData)
{
    using namespace AsyncLoadFinished;
//    _activeScene = _newScene;
//    _newScene.Reset();

    // Imitate slower loading
    progress = 0.1f;

    UnsubscribeFromEvent(E_ASYNCLOADPROGRESS);
    UnsubscribeFromEvent(E_ASYNCLOADFINISHED);

    URHO3D_LOGINFO("Scene loaded: " + _activeScene->GetFileName());
}

void SceneManager::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    // Imitate slower loading
    using namespace Update;
    progress += eventData[P_TIMESTEP].GetFloat() / 5.0f;
}