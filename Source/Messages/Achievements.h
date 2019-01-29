#pragma once

#include <Urho3D/Urho3DAll.h>
#include "SingleAchievement.h"

using namespace Urho3D;

struct AchievementRule {
    /**
     * Name of the event which should trigger achievement check
     */
    String eventName;
    /**
     * Image path
     */
    String image;
    /**
     * Message to display when this achievement is unlocked
     */
    String message;
    /**
     * How many times the event should be called till the achievement can be marked as unlocked
     */
    int threshold;
    /**
     * Counter how much times the event was called - when this is equal to threshold, achievement is unlocked
     */
    int current;
    /**
     * Whether achievement was displayed or not
     */
    bool completed;
    /**
     * Optional - event parameter to check and decide if the counter should be incremented or not
     */
    String parameterName;
    /**
     * Optional - event parameter value which value should match to allow counter incrementing
     */
    Variant parameterValue;
    /**
     * How to check if the achievement criteria was met
     * false - check only incoming event
     * true - check `eventData` object and compare `parameterName` and `parameterValue`
     */
    bool deepCheck;
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

    /**
     * Get all registered achievements
     */
    List<AchievementRule> GetAchievements();

    /**
     * Clear current progress of achievements
     */
    void ClearAchievementsProgress();

private:
    friend void SaveProgressAsync(const WorkItem* item, unsigned threadIndex);

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

    void HandleAddAchievement(StringHash eventType, VariantMap& eventData);

    void AddAchievement(
        String message, 
        String eventName, 
        String image, 
        int threshold, 
        String parameterName = String::EMPTY, 
        Variant parameterValue = Variant::EMPTY
    );

    /**
     * Update SingleAchievement statuses
     */
    void HandleUpdate(StringHash eventType, VariantMap& eventData);

    /**
     * Handle registered event statuses
     */
    void HandleRegisteredEvent(StringHash eventType, VariantMap& eventData);

    /**
     * Load achievements configuration
     */
    void LoadAchievementList();

    /**
     * Save achievement progress
     */
    void SaveProgress();

    /**
     * Load achievement progress
     */
    void LoadProgress();

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

    /**
     * All registered achievements
     */
    HashMap<StringHash, List<AchievementRule>> _registeredAchievements;

    /**
     * All achievements
     */
    List<AchievementRule> _achievements{};

    /**
     * Current achievement progress
     */
    HashMap<String, int> _progress;

};