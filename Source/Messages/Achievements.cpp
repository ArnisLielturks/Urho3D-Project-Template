#include <Urho3D/Urho3DAll.h>
#include "Achievements.h"

void SingleAchievement::RegisterObject(Context* context)
{
    context->RegisterFactory<SingleAchievement>();
    URHO3D_ATTRIBUTE("Size", float, _size, 1, AM_FILE);
}

/// Construct.
SingleAchievement::SingleAchievement(Context* context) :
    Animatable(context)
{
    //SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(SingleAchievement, HandlePostUpdate));
}

SingleAchievement::~SingleAchievement()
{
}

void SingleAchievement::SetImage(String image)
{
    auto* cache = GetSubsystem<ResourceCache>();
    _imageTexture = cache->GetResource<Texture2D>(image);
    if (_imageTexture) {
        _image = nk_image_ptr((void*)_imageTexture.Get());
    }
}

void SingleAchievement::SetMessage(String message)
{
    _message = message;
}

void SingleAchievement::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{
    auto graphics = GetSubsystem<Graphics>();
    auto nuklear = GetSubsystem<NuklearUI>();
    auto ctx = nuklear->GetNkContext();
    nk_style_default(ctx);

    if (nk_begin(nuklear->GetNkContext(), _message.CString(), nk_rect((int)_size, graphics->GetHeight() - 110, 300, 100), NK_WINDOW_NO_SCROLLBAR)) {

        nk_layout_row_begin(nuklear->GetNkContext(), NK_DYNAMIC, 90, 3);
        {
            if (_image.handle.ptr == NULL)
            {
            }
            else {
                nk_layout_row_push(nuklear->GetNkContext(), 0.35);
                nk_button_image_label(nuklear->GetNkContext(), _image, "logo", NK_TEXT_CENTERED);
            }
            nk_layout_row_push(nuklear->GetNkContext(), 0.05);
            nk_spacing(nuklear->GetNkContext(), 1);
            nk_layout_row_push(nuklear->GetNkContext(), 0.6);
            nk_label_wrap(nuklear->GetNkContext(), _message.CString());
        }
        nk_layout_row_end(nuklear->GetNkContext());
    }
    nk_end(ctx);

    using namespace PostUpdate;

    UpdateAttributeAnimations(eventData[P_TIMESTEP].GetFloat());
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
    SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(Achievements, HandleUpdate));
}

void Achievements::Dispose()
{
}

void Achievements::HandleNewAchievement(StringHash eventType, VariantMap& eventData)
{
    String message = eventData["Message"].GetString();
    URHO3D_LOGINFO("New achievement: " + message);
    
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
    objAnimation->AddAttributeAnimation("Size", positionAnimation2);

    singleAchievement->SetObjectAnimation(objAnimation);
    singleAchievement->SetVar("Lifetime", 8.0f);
    _activeAchievements.Push(singleAchievement);
}

void Achievements::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;

    float timeStep = eventData[P_TIMESTEP].GetFloat();
    for (auto it = _activeAchievements.Begin(); it != _activeAchievements.End(); ++it) {
        if ((*it).Refs() == 0) {
            _activeAchievements.Remove((*it));
            return;
        }
        float lifetime = (*it)->GetVar("Lifetime").GetFloat();
        if (lifetime <= 0) {
            // (*it)->Remove();
            _activeAchievements.Remove((*it));
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