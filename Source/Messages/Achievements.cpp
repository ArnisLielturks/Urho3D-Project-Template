#include <Urho3D/Urho3DAll.h>
#include "Achievements.h"
#include "../Audio/AudioManagerDefs.h"
#include "../MyEvents.h"

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
    LoadAchievementList();
    SubscribeToEvents();
}


void Achievements::SubscribeToEvents()
{
    SubscribeToEvent(MyEvents::E_NEW_ACHIEVEMENT, URHO3D_HANDLER(Achievements, HandleNewAchievement));
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
}

void Achievements::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;

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
    JSONFile configFile(context_);
    configFile.LoadFile(GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Config/Achievements.json");
    JSONValue value = configFile.GetRoot();
    if (value.IsArray()) {
        URHO3D_LOGINFOF("Achievements: %u", value.Size());
        for (int i = 0; i < value.Size(); i++) {
            JSONValue mapInfo = value[i];
            if (mapInfo.Contains("Event")
                && mapInfo["Event"].IsString()
                && mapInfo.Contains("Image")
                && mapInfo["Image"].IsString()
                && mapInfo.Contains("Text")
                && mapInfo["Text"].IsString()
                && mapInfo.Contains("Threshold")
                && mapInfo["Threshold"].IsNumber()
                && mapInfo.Contains("Type")
                && mapInfo["Type"].IsString()) {

                String evt = mapInfo["Event"].GetString();
                String img = mapInfo["Image"].GetString();
                String txt = mapInfo["Text"].GetString();
                String type = mapInfo["Type"].GetString();
                int threshold = mapInfo["Threshold"].GetInt();
                // URHO3D_LOGINFO("Achievement: '" + txt + "'");
                // URHO3D_LOGINFO("Image: " + img);
                // URHO3D_LOGINFO("Evt: " + evt);
                // URHO3D_LOGINFO("Type: " + type);
                // URHO3D_LOGINFOF("Threshold: %i", threshold);

            }
            else {
                URHO3D_LOGINFO("Achievement array element doesnt contain all needed info!");
            }
        }
    }
    else {
        URHO3D_LOGINFO("Map config json is not an array!");
    }
}