#pragma once

#include <Urho3D/Urho3DAll.h>

class SceneManager : public Object
{
URHO3D_OBJECT(SceneManager, Object);
public:
    SceneManager(Context* context);

    ~SceneManager();

    void LoadScene(const String& filename);

    Scene* GetActiveScene() { return _activeScene; }
    float GetProgress() { return progress; }
private:
    void HandleAsyncSceneLoadingProgress(StringHash eventType, VariantMap& eventData);
    void HandleAsyncSceneLoadingFinished(StringHash eventType, VariantMap& eventData);
    void HandleUpdate(StringHash eventType, VariantMap& eventData);

    SharedPtr<Scene> _activeScene;

    float progress;
};