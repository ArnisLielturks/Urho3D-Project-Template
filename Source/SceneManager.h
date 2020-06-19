#pragma once

#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Timer.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Container/Vector.h>

using namespace Urho3D;

struct LoadingStep {
    String event;
    String name;
    bool finished;
    bool ack;
    bool ackSent;
    Timer ackTimer;
    float progress;
    Timer loadTime;
    bool failed;
    bool autoRemove;
    StringVector dependsOn;
    String map;
};

struct MapInfo {
    String map;
    String name;
    String description;
    String image;
    StringVector commands;
    Vector3 startPoint;
    String startNode;
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
    SharedPtr<Scene>& GetActiveScene() { return activeScene_; }

    /**
     * Scene loading progress
     */
    float GetProgress() { return progress_; }

    /**
     * Current loading status
     */
    const String& GetStatusMessage() const { return loadingStatus_; }

    /**
     * Set current progress to 0
     */
    void ResetProgress();

    void CleanupScene();

    const Vector<MapInfo>& GetAvailableMaps() const;

    const MapInfo* GetCurrentMapInfo() const;

private:

    void CleanupLoadingSteps();

    void LoadDefaultMaps();

    bool CanLoadingStepRun(LoadingStep& loadingStep);

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
     * Receive loading step should be fixed message
     */
    void HandleSkipLoadingStep(StringHash eventType, VariantMap& eventData);

    /**
     * Add map to the map selection
     */
    void HandleAddMap(StringHash eventType, VariantMap& eventData);

    MapInfo* GetMap(const String& filename);

    /**
     * Current active scene
     */
    SharedPtr<Scene> activeScene_;

    /**
     * Current progress
     */
    float progress_{0.0f};

    /**
     * Target progress, `progress` variable will reach `targetProgress` in few seconds
     */
    float targetProgress_{0.0f};

    /**
     * Current loading status
     */
    String loadingStatus_;

    /**
     * List of all the loading steps registered in the system
     */
    HashMap<StringHash, LoadingStep> loadingSteps_;

    Vector<MapInfo> availableMaps_;

    MapInfo* currentMap_;
};
