#include <Urho3D/Urho3DAll.h>
#include "Achievements.h"

void SingleAchievement::RegisterObject(Context* context)
{
    context->RegisterFactory<SingleAchievement>();
    URHO3D_ATTRIBUTE("Offset", float, _offset, 1, AM_FILE);
}

/// Construct.
SingleAchievement::SingleAchievement(Context* context) :
    Animatable(context)
{
    SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(SingleAchievement, HandlePostUpdate));

    _baseWindow = GetSubsystem<UI>()->GetRoot()->CreateChild<Window>();
    _baseWindow->SetStyleAuto();
    _baseWindow->SetAlignment(HA_LEFT, VA_BOTTOM);
    _baseWindow->SetSize(300, 100);
    _baseWindow->BringToFront();

    _baseWindow->SetBringToBack(true);

    _sprite = _baseWindow->CreateChild<Sprite>();
    _sprite->SetSize(80, 80);
    _sprite->SetAlignment(HA_LEFT, VA_TOP);
    _sprite->SetPosition(10, 10);

    auto *cache = GetSubsystem<ResourceCache>();
    auto *font = cache->GetResource<Font>("Fonts/ABeeZee-Regular.ttf");
    _title = _baseWindow->CreateChild<Text>();
    _title->SetFont(font, 10);
    _title->SetAlignment(HA_LEFT, VA_CENTER);
    _title->SetPosition(100, 0);
}

SingleAchievement::~SingleAchievement()
{
    _baseWindow->Remove();
    UnsubscribeFromEvent(E_POSTUPDATE);
}

void SingleAchievement::SetImage(String image)
{
    auto* cache = GetSubsystem<ResourceCache>();
    _sprite->SetTexture(cache->GetResource<Texture2D>(image));
}

void SingleAchievement::SetMessage(String message)
{
    _message = "";

    // Split longer messages into multiple lines
    String line;
    auto words = message.Split(' ', false);
    for (auto it = words.Begin(); it != words.End(); ++it) {
        if (line.Length() + (*it).Length() > 20) {
            _message += line + "\n";
            line = "";
        }
        line += (*it) + " ";
    }
    if (!line.Empty()) {
        _message += line;
    }

    _title->SetText(_message);
}

String SingleAchievement::GetMessage()
{
    return _message;
}

void SingleAchievement::SetVerticalPosition(int position)
{
    _verticalPos = position;
}

void SingleAchievement::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{
    auto graphics = GetSubsystem<Graphics>();

    using namespace PostUpdate;

    UpdateAttributeAnimations(eventData[P_TIMESTEP].GetFloat());

    _baseWindow->SetPosition(_offset, _verticalPos);
}

void SingleAchievement::OnAttributeAnimationAdded()
{
    if (attributeAnimationInfos_.Size() == 1)
        SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(SingleAchievement, HandlePostUpdate));
}

void SingleAchievement::OnAttributeAnimationRemoved()
{
    if (attributeAnimationInfos_.Empty())
        UnsubscribeFromEvent(E_POSTUPDATE);
}

void SingleAchievement::SetVar(StringHash key, const Variant& value)
{
    vars_[key] = value;
}

const Variant& SingleAchievement::GetVar(const StringHash& key) const
{
    VariantMap::ConstIterator i = vars_.Find(key);
    return i != vars_.End() ? i->second_ : Variant::EMPTY;
}

// ------------------------------------------

/// Construct.
Achievements::Achievements(Context* context) :
    Object(context)
{
    Init();
}

Achievements::~Achievements()
{
}

void Achievements::Init()
{
    Create();
    LoadAchievementList();
    SubscribeToEvents();
}

void Achievements::Create()
{
}

void Achievements::SubscribeToEvents()
{
    SubscribeToEvent("NewAchievement", URHO3D_HANDLER(Achievements, HandleNewAchievement));
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Achievements, HandleUpdate));
}

void Achievements::Dispose()
{
}

void Achievements::HandleNewAchievement(StringHash eventType, VariantMap& eventData)
{
    String message = eventData["Message"].GetString();
    URHO3D_LOGINFO("New achievement: " + message);
    
    for (auto it = _activeAchievements.Begin(); it != _activeAchievements.End(); ++it) {
        if ((*it)->GetMessage() == message) {
            URHO3D_LOGINFO("Achievement already visible!");
            return;
        }
    }

    SharedPtr<SingleAchievement> singleAchievement = context_->CreateObject<SingleAchievement>();
    singleAchievement->SetImage("Textures/UrhoIcon.png");
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
    positionAnimation2->SetKeyFrame(5.0f, 10.0f);
    positionAnimation2->SetKeyFrame(6.0f, -300.0f);
    positionAnimation2->SetKeyFrame(10.0f, -400.0f);
    objAnimation->AddAttributeAnimation("Offset", positionAnimation2);

    singleAchievement->SetObjectAnimation(objAnimation);
    singleAchievement->SetVar("Lifetime", 6.0f);

    int position = -_activeAchievements.Size() * 110 - 10;
    singleAchievement->SetVerticalPosition(position);
    _activeAchievements.Push(singleAchievement);
}

void Achievements::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;

    float timeStep = eventData[P_TIMESTEP].GetFloat();
    for (auto it = _activeAchievements.Begin(); it != _activeAchievements.End(); ++it) {
        if (!(*it)) {
            _activeAchievements.Erase(it);
            return;
        }
        float lifetime = (*it)->GetVar("Lifetime").GetFloat();
        if (lifetime <= 0) {
            // (*it)->Remove();
            _activeAchievements.Erase(it);
            return; 
        }
        lifetime -= timeStep;
        (*it)->SetVar("Lifetime", lifetime);
    }
}

void Achievements::HandleGameEnd(StringHash eventType, VariantMap& eventData)
{
    _activeAchievements.Clear();
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