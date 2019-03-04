#pragma once

#include <Urho3D/Urho3DAll.h>

struct LoadingStep {
    String event;
    String name;
    bool finished;
    bool ack;
    bool ackSent;
    Timer ackTimer;
    float progress;
    Timer loadTime;
};

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

    /**
     * Set current progress to 0
     */
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
     * Add new loading step to the loading screen
     */
    void HandleRegisterLoadingStep(StringHash eventType, VariantMap& eventData);

    /**
     * Ackownledge the loading step and let it finish
     */
    void HandleLoadingStepAck(StringHash eventType, VariantMap& eventData);

    /**
     * Receive update status for specific loading step
     */
    void HandleLoadingStepProgress(StringHash eventType, VariantMap& eventData);

    /**
     * Receive loading step finished message
     */
    void HandleLoadingStepFinished(StringHash eventType, VariantMap& eventData);

    /**
     * Current active scene
     */
    SharedPtr<Scene> _activeScene;

    /**
     * Current progress
     */
    float progress;

    /**
     * Target progress, `progress` variable will reach `targetProgress` in few seconds
     */
    float targetProgress;

    /**
     * Current loading status
     */
    String _loadingStatus;

    /**
     * List of all the loading steps registered in the system
     */
    HashMap<StringHash, LoadingStep> _loadingSteps;
};