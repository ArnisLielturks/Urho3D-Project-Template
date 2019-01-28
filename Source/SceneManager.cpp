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

    _loadingStatus = "Loading scene";
    URHO3D_LOGINFO("Scene manager loading scene: " + filename);

    SubscribeToEvent(_activeScene, E_ASYNCLOADPROGRESS, URHO3D_HANDLER(SceneManager, HandleAsyncSceneLoadingProgress));
    SubscribeToEvent(_activeScene, E_ASYNCLOADFINISHED, URHO3D_HANDLER(SceneManager, HandleAsyncSceneLoadingFinished));
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

    // Imitate slower loading
    progress = 0.0f;

    _loadingStatus = "Creating objects";
    UnsubscribeFromEvent(E_ASYNCLOADPROGRESS);
    UnsubscribeFromEvent(E_ASYNCLOADFINISHED);

    auto* cache = GetSubsystem<ResourceCache>();
    const unsigned NUM_OBJECTS = 0;
    for (unsigned i = 0; i < NUM_OBJECTS; ++i)
    {
        Node* mushroomNode = _activeScene->CreateChild("Mushroom");
        mushroomNode->SetPosition(Vector3(Random(200.0f) - 45.0f, 0.0f, Random(200.0f) - 100.0f));
        mushroomNode->SetRotation(Quaternion(0.0f, Random(360.0f), 0.0f));
        mushroomNode->SetScale(0.1f + Random(1.0f));
        auto* mushroomObject = mushroomNode->CreateComponent<StaticModel>();
        mushroomObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
        mushroomObject->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
    }

    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(SceneManager, HandleUpdate));
    URHO3D_LOGINFO("Scene loaded: " + _activeScene->GetFileName());
}

void SceneManager::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    static int index = 0;

    // Imitate slower loading with multiple loading steps
    if (_timer.GetMSec(false) > 1000) {
        static const char *Messages[] = {
            "Smelling flowers",
            "Improving CPU skills",
            "Discussing religion",
            "Retrieving sensitive user information",
            "Gaining profit",
            "Loosing trust in people",
            "Meditating"
        };
        if (index > 6) {
            index = 6;
            progress = 1.0f;
        }
        _loadingStatus = Messages[index++];
        _timer.Reset();
    }

    using namespace Update;
    progress += eventData[P_TIMESTEP].GetFloat() / 15.0f;
    if (progress >= 1.0f) {
        progress = 1.0f;
    }
}