#pragma once

#include <Urho3D/Urho3DAll.h>

class SceneManager : public Object
{
URHO3D_OBJECT(SceneManager, Object);
public:
    SceneManager(Context* context);

    ~SceneManager();

    /**
     * Start loading scene from the file
     */
    void LoadScene(const String& filename);

    /**
     * Currently active scene
     */
    Scene* GetActiveScene() { return _activeScene; }

    /**
     * Scene loading progress
     */
    float GetProgress() { return progress; }

    /**
     * Current loading status
     */
    const String& GetStatusMessage() const { return _loadingStatus; }

    void ResetProgress();

private:
    /**
     * Scene loading in progress
     */
    void HandleAsyncSceneLoadingProgress(StringHash eventType, VariantMap& eventData);

    /**
     * Scene loading finished
     */
    void HandleAsyncSceneLoadingFinished(StringHash eventType, VariantMap& eventData);

    void HandleUpdate(StringHash eventType, VariantMap& eventData);

    /**
     * Current active scene
     */
    SharedPtr<Scene> _activeScene;

    /**
     * Current progress
     */
    float progress;

    /**
     * Current loading status
     */
    String _loadingStatus;

    Timer _timer;
};