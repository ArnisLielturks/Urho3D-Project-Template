#pragma once

#include <Urho3D/Urho3DAll.h>
#include "SingleAchievement.h"

using namespace Urho3D;

struct AchievementRule {
    String eventName;
    String image;
    String message;
    int threshold;
    int current;
    bool completed;
};

class Achievements : public Object
{
    URHO3D_OBJECT(Achievements, Object);

public:
    /// Construct.
    Achievements(Context* context);

    virtual ~Achievements();

    /**
     * Enable/Disable achievements from showing
     * Achievements won't be lost but their displaying will be added
     * to the queue when this is disabled
     */
    void SetShowAchievements(bool show);

private:
    /**
     * Initialize achievements
     */
    void Init();

    /**
     * Subscribe to the notification events
     */
    void SubscribeToEvents();

    /**
     * Create or add new event to the queue
     */
    void HandleNewAchievement(StringHash eventType, VariantMap& eventData);

    /**
     * Update SingleAchievement statuses
     */
    void HandleUpdate(StringHash eventType, VariantMap& eventData);

    void HandleRegisteredEvent(StringHash eventType, VariantMap& eventData);

    /**
     * Load achievements configuration
     */
    void LoadAchievementList();

    /**
     * Active achievement list
     */
    List<SharedPtr<SingleAchievement>> _activeAchievements;

    /**
     * Only 1 achievement is allowed at a time,
     * so all additional achievements are added to the queue
     */
    List<VariantMap> _achievementQueue;

    /**
     * Disable achievements from showing, all incoming achievements
     * will be added to the queue. This might be useful to avoid showing achievements
     * on Splash or Credits screens for example
     */
    bool _showAchievements;

    HashMap<StringHash, List<AchievementRule>> _registeredAchievements;

};