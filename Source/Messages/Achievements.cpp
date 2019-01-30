#include <Urho3D/Urho3DAll.h>
#include "Achievements.h"
#include "../Audio/AudioManagerDefs.h"
#include "../MyEvents.h"

void SaveProgressAsync(const WorkItem* item, unsigned threadIndex)
{
    Achievements* achievementHandler = reinterpret_cast<Achievements*>(item->aux_);
    achievementHandler->SaveProgress();
}

/// Construct.
Achievements::Achievements(Context* context) :
    Object(context),
    _showAchievements(false)
{
    Init();
}

Achievements::~Achievements()
{
    _activeAchievements.Clear();
}

void Achievements::SetShowAchievements(bool show)
{
    _showAchievements = show;
}

void Achievements::Init()
{
    SubscribeToEvents();
    LoadAchievementList();
}


void Achievements::SubscribeToEvents()
{
    SubscribeToEvent(MyEvents::E_NEW_ACHIEVEMENT, URHO3D_HANDLER(Achievements, HandleNewAchievement));
    SubscribeToEvent(MyEvents::E_ADD_ACHIEVEMENT, URHO3D_HANDLER(Achievements, HandleAddAchievement));
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Achievements, HandleUpdate));
}

void Achievements::HandleNewAchievement(StringHash eventType, VariantMap& eventData)
{
    using namespace MyEvents::NewAchievement;
    String message = eventData[P_MESSAGE].GetString();

    if (!_activeAchievements.Empty() || !_showAchievements) {
        _achievementQueue.Push(eventData);
        URHO3D_LOGINFO("Pushing achievement to the queue " + message);
        return;
    }
    
    for (auto it = _activeAchievements.Begin(); it != _activeAchievements.End(); ++it) {
        if ((*it)->GetMessage() == message) {
            URHO3D_LOGINFO("Achievement already visible!");
            return;
        }
    }

    URHO3D_LOGINFO("New achievement: " + message);

    SharedPtr<SingleAchievement> singleAchievement = context_->CreateObject<SingleAchievement>();

    String imageName = eventData[P_IMAGE].GetString();
    if (!imageName.Empty()) {
        singleAchievement->SetImage(imageName);
    } else {
        singleAchievement->SetImage("Textures/UrhoIcon.png");
    }

    singleAchievement->SetMessage(message);
    // Create light animation
    SharedPtr<ObjectAnimation> objAnimation(new ObjectAnimation(context_));

    // Create light position animation
    SharedPtr<ValueAnimation> positionAnimation2(new ValueAnimation(context_));
    // Use spline interpolation method
    positionAnimation2->SetInterpolationMethod(IM_LINEAR);
    // Set spline tension
    //positionAnimation2->SetSplineTension(0.7f);
    positionAnimation2->SetKeyFrame(0.0f, -300.0f);
    positionAnimation2->SetKeyFrame(1.0f, 10.0f);
    positionAnimation2->SetKeyFrame(4.0f, 10.0f);
    positionAnimation2->SetKeyFrame(5.0f, -300.0f);
    positionAnimation2->SetKeyFrame(10.0f, -400.0f);
    objAnimation->AddAttributeAnimation("Offset", positionAnimation2);

    singleAchievement->SetObjectAnimation(objAnimation);
    singleAchievement->SetVar("Lifetime", 5.0f);

    _activeAchievements.Push(singleAchievement);

    using namespace AudioDefs;
    using namespace MyEvents::PlaySound;
    VariantMap data = GetEventDataMap();
    data[P_INDEX] = SOUND_EFFECTS::ACHIEVEMENT;
    data[P_TYPE] = SOUND_EFFECT;
    SendEvent(MyEvents::E_PLAY_SOUND, data);

    using namespace MyEvents::AchievementUnlocked;
    VariantMap achievementData = GetEventDataMap();
    achievementData[MyEvents::AchievementUnlocked::P_MESSAGE] = message;
    SendEvent(MyEvents::E_ACHIEVEMENT_UNLOCKED, achievementData);
}

void Achievements::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;

    if (_activeAchievements.Empty() && !_achievementQueue.Empty() && _showAchievements) {
        HandleNewAchievement("", _achievementQueue.Front());
        _achievementQueue.PopFront();
    }

    float timeStep = eventData[P_TIMESTEP].GetFloat();
    for (auto it = _activeAchievements.Begin(); it != _activeAchievements.End(); ++it) {
        if (!(*it)) {
            _activeAchievements.Erase(it);

            if (!_achievementQueue.Empty() && _showAchievements) {
                HandleNewAchievement("", _achievementQueue.Front());
                _achievementQueue.PopFront();
            }

            return;
        }
        float lifetime = (*it)->GetVar("Lifetime").GetFloat();
        if (lifetime <= 0) {
            // (*it)->Remove();
            _activeAchievements.Erase(it);

            if (!_achievementQueue.Empty() && _showAchievements) {
                HandleNewAchievement("", _achievementQueue.Front());
                _achievementQueue.PopFront();
            }
            return; 
        }
        lifetime -= timeStep;
        (*it)->SetVar("Lifetime", lifetime);
    }
}

void Achievements::LoadAchievementList()
{
    LoadProgress();

    JSONFile configFile(context_);
    configFile.LoadFile(GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Config/Achievements.json");
    JSONValue value = configFile.GetRoot();
    if (value.IsArray()) {
        GetSubsystem<DebugHud>()->SetAppStats("Total achievements loaded", value.Size());
        URHO3D_LOGINFOF("Loading achievements config: %u", value.Size());
        for (int i = 0; i < value.Size(); i++) {
            JSONValue mapInfo = value[i];
            if (mapInfo.Contains("Event")
                && mapInfo["Event"].IsString()
                && mapInfo.Contains("Image")
                && mapInfo["Image"].IsString()
                && mapInfo.Contains("Message")
                && mapInfo["Message"].IsString()
                && mapInfo.Contains("Threshold")
                && mapInfo["Threshold"].IsNumber()) {

                String eventName = mapInfo["Event"].GetString();
                String image = mapInfo["Image"].GetString();
                String message = mapInfo["Message"].GetString();
                int threshold = mapInfo["Threshold"].GetInt();
                String parameterName;
                Variant parameterValue;

                if (mapInfo.Contains("ParameterName") && mapInfo["ParameterName"].IsString() && mapInfo.Contains("Value")) {
                    parameterName = mapInfo["ParameterName"].GetString();
                    switch (mapInfo["Value"].GetValueType()) {
                    case JSONValueType::JSON_BOOL:
                        parameterValue = mapInfo["Value"].GetBool();
                        break;
                    case JSONValueType::JSON_NUMBER:
                        parameterValue = mapInfo["Value"].GetInt();
                        break;
                    case JSONValueType::JSON_STRING:
                        parameterValue = mapInfo["Value"].GetString();
                        break;
                    }
                }

                AddAchievement(message, eventName, image, threshold, parameterName, parameterValue);
                
            }
            else {
                URHO3D_LOGINFOF("Achievement array element doesnt contain all needed info! Index: %u", i);
            }
        }
    }
    else {
        URHO3D_LOGERROR("Data/Config/Achievements.json must be an array");
    }
}

void Achievements::HandleRegisteredEvent(StringHash eventType, VariantMap& eventData)
{
    bool processed = false;
    if (_registeredAchievements.Contains(eventType)) {
        for (auto it = _registeredAchievements[eventType].Begin(); it != _registeredAchievements[eventType].End(); ++it) {
            if ((*it).deepCheck) {
                // check if the event contains specified parameter and same value
                if (eventData[(*it).parameterName] == (*it).parameterValue) {
                    (*it).current++;
                    processed = true;
                }
            } else {
                // No additional check needed, event was called, so we can increment our counter
                (*it).current++;
                processed = true;
            }

            //URHO3D_LOGINFOF("Achievement progress: '%s' => %i/%i",(*it).message.CString(), (*it).current, (*it).threshold);
            if ((*it).current >= (*it).threshold && !(*it).completed) {
                (*it).completed = true;
                VariantMap data = GetEventDataMap();
                data["Message"] = (*it).message;
                data["Image"] = (*it).image;
                SendEvent(MyEvents::E_NEW_ACHIEVEMENT, data);
            }
        }
    }

    // If any of the achievements were updated, save progress
    if (processed) {
        WorkQueue *workQueue = GetSubsystem<WorkQueue>();
        SharedPtr<WorkItem> item = workQueue->GetFreeItem();
        item->priority_ = M_MAX_UNSIGNED;
        item->workFunction_ = SaveProgressAsync;
        item->aux_ = this;
        // send E_WORKITEMCOMPLETED event after finishing WorkItem
        item->sendEvent_ = true;

        item->start_ = nullptr;
        item->end_ = nullptr;
        workQueue->AddWorkItem(item);
    }
}

List<AchievementRule> Achievements::GetAchievements()
{
    _achievements.Clear();
    for (auto it = _registeredAchievements.Begin(); it != _registeredAchievements.End(); ++it) {
        for (auto it2 = (*it).second_.Begin(); it2 != (*it).second_.End(); ++it2) {
            _achievements.Push((*it2));
        }
    }

    return _achievements;
}

void Achievements::SaveProgress()
{
    JSONFile file(context_);
    for (auto it = _registeredAchievements.Begin(); it != _registeredAchievements.End(); ++it) {
        for (auto achievement = (*it).second_.Begin(); achievement != (*it).second_.End(); ++achievement) {
            StringHash id = (*achievement).eventName + (*achievement).message;
            file.GetRoot()[id.ToString()] = (*achievement).current;
        }
    }
    file.SaveFile(GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Saves/Achievements.json");

    //URHO3D_LOGINFO("Achievement progress saving done!");
}

void Achievements::LoadProgress()
{
    JSONFile configFile(context_);
    configFile.LoadFile(GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Saves/Achievements.json");
    JSONValue value = configFile.GetRoot();
    if (value.IsObject()) {
        for (auto it = value.Begin(); it != value.End(); ++it) {
            _progress[(*it).first_] = (*it).second_.GetInt();
        }
    }
}

void Achievements::ClearAchievementsProgress()
{
    for (auto it = _registeredAchievements.Begin(); it != _registeredAchievements.End(); ++it) {
        for (auto achievement = (*it).second_.Begin(); achievement != (*it).second_.End(); ++achievement) {
            achievement->current = 0;
            achievement->completed = false;
        }
    }

    SaveProgress();
}

void Achievements::AddAchievement(String message, 
    String eventName, 
    String image, 
    int threshold, 
    String parameterName, 
    Variant parameterValue
)
{
    AchievementRule rule;
    rule.message = message;
    rule.eventName = eventName;
    rule.image = image;
    rule.threshold = threshold;
    rule.current = 0;
    rule.completed = false; 
    if (!parameterName.Empty() && !parameterValue.IsEmpty()) {
        rule.deepCheck = true;
    }
    else {
        rule.deepCheck = false;
    }
    rule.parameterName = parameterName;
    rule.parameterValue = parameterValue;

    // Check current achievement saved progress
    StringHash id = rule.eventName + rule.message;
    if (_progress.Contains(id.ToString())) {
        rule.current = _progress[id.ToString()];

        // Check if achievement was already unlocked
        if (rule.current >= rule.threshold) {
            rule.completed = true;
            URHO3D_LOGINFO("Achievement '" + rule.message + "' already unlocked!");
        }
    }    

    _registeredAchievements[eventName].Push(rule);

    URHO3D_LOGINFOF("Registering achievement [%s]", rule.message.CString());

    SubscribeToEvent(eventName, URHO3D_HANDLER(Achievements, HandleRegisteredEvent));
}

void Achievements::HandleAddAchievement(StringHash eventType, VariantMap& eventData)
{
    using namespace MyEvents::AddAchievement;
    if (eventData.Contains(P_EVENT)
        && eventData.Contains(P_MESSAGE)
        && eventData.Contains(P_IMAGE)
        && eventData.Contains(P_THRESHOLD)) {
        AddAchievement(eventData[P_MESSAGE].GetString(),
            eventData[P_EVENT].GetString(),
            eventData[P_IMAGE].GetString(),
            eventData[P_THRESHOLD].GetInt(),
            eventData[P_PARAMETER_NAME].GetString(),
            eventData[P_PARAMETER_VALUE]);
    }
    else {
        URHO3D_LOGERRORF("Unable to register achievement [%s], incomplete data provided!", eventData[P_MESSAGE].GetString().CString());
    }
}