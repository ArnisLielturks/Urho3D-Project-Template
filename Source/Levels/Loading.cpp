#include <Urho3D/Urho3DAll.h>
#include "Loading.h"
#include "../MyEvents.h"
#include "../Messages/Achievements.h"
#include "../SceneManager.h"
#include "../Config/ConfigManager.h"

using namespace Levels;

    /// Construct.
Loading::Loading(Context* context) :
    BaseLevel(context)
{
}

Loading::~Loading()
{
}

void Loading::Init()
{
    // Disable achievement showing for this level
    GetSubsystem<Achievements>()->SetShowAchievements(false);

    BaseLevel::Init();

    // Create the scene content
    CreateScene();

    // Create the UI content
    CreateUI();

    // Subscribe to global events for camera movement
    SubscribeToEvents();

    if (data_.Contains("Map")) {
        GetSubsystem<SceneManager>()->LoadScene(data_["Map"].GetString());
    } else {
        GetSubsystem<SceneManager>()->LoadScene(GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Scenes/Scene.xml");
    }
}

void Loading::CreateScene()
{
    return;
}

void Loading::CreateUI()
{
    UI* ui = GetSubsystem<UI>();
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    // Get the Urho3D fish texture
    auto* decalTex = cache->GetResource<Texture2D>("Textures/UrhoIcon.png");
    // Create a new sprite, set it to use the texture
    SharedPtr<Sprite> sprite(new Sprite(context_));
    sprite->SetTexture(decalTex);

    auto* graphics = GetSubsystem<Graphics>();

    // Get rendering window size as floats
    auto width = (float)graphics->GetWidth();
    auto height = (float)graphics->GetHeight();

    // The UI root element is as big as the rendering window, set random position within it
    sprite->SetPosition(width - decalTex->GetWidth(), height - decalTex->GetHeight() - 20);

    // Set sprite size & hotspot in its center
    sprite->SetSize(IntVector2(decalTex->GetWidth(), decalTex->GetHeight()));
    sprite->SetHotSpot(IntVector2(decalTex->GetWidth() / 2, decalTex->GetHeight() / 2));

    // Set random rotation in degrees and random scale
    sprite->SetRotation(Random() * 360.0f);

    // Set random color and additive blending mode
    sprite->SetColor(Color(Random(0.5f) + 0.5f, Random(0.5f) + 0.5f, Random(0.5f) + 0.5f));
    sprite->SetBlendMode(BLEND_ADD);


    // Add as a child of the root UI element
    ui->GetRoot()->AddChild(sprite);

    SharedPtr<ObjectAnimation> logoAnimation(new ObjectAnimation(context_));
    SharedPtr<ValueAnimation> rotation(new ValueAnimation(context_));

    // Use spline interpolation method
    rotation->SetInterpolationMethod(IM_LINEAR);
    // Set spline tension
    rotation->SetSplineTension(0.7f);
    rotation->SetKeyFrame(0.0f, 0.0f);
    rotation->SetKeyFrame(3.0f, 360.0f);
    logoAnimation->AddAttributeAnimation("Rotation", rotation);

    sprite->SetObjectAnimation(logoAnimation);

    _status = ui->GetRoot()->CreateChild<Text>();
    _status->SetHorizontalAlignment(HA_LEFT);
    _status->SetVerticalAlignment(VA_BOTTOM);
    _status->SetPosition(20, -30);
    _status->SetStyleAuto();
    _status->SetText("Progress: 0%");
    _status->SetTextEffect(TextEffect::TE_STROKE);
    _status->SetFontSize(16);
    _status->SetColor(Color(0.8f, 0.8f, 0.2f));
    _status->SetBringToBack(true);


    SharedPtr<ObjectAnimation> animation(new ObjectAnimation(context_));
    SharedPtr<ValueAnimation> colorAnimation(new ValueAnimation(context_));
    // Use spline interpolation method
    colorAnimation->SetInterpolationMethod(IM_LINEAR);
    // Set spline tension
    colorAnimation->SetSplineTension(0.7f);
    colorAnimation->SetKeyFrame(0.0f, Color::RED);
    colorAnimation->SetKeyFrame(1.0f, Color::YELLOW);
    colorAnimation->SetKeyFrame(2.0f, Color::GREEN);
    colorAnimation->SetKeyFrame(3.0f, Color::GRAY);
    colorAnimation->SetKeyFrame(4.0f, Color::BLUE);
    colorAnimation->SetKeyFrame(5.0f, Color::RED);
    animation->AddAttributeAnimation("Color", colorAnimation);

    _status->SetObjectAnimation(animation);

    CreateProgressBar();
}

void Loading::SubscribeToEvents()
{
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Loading, HandleUpdate));
    SubscribeToEvent(StringHash("EndLoading"), URHO3D_HANDLER(Loading, HandleEndLoading));
}

void Loading::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    Input* input = GetSubsystem<Input>();
    if (input->IsMouseVisible()) {
        input->SetMouseVisible(false);
    }

    float progress = GetSubsystem<SceneManager>()->GetProgress();
    _status->SetText(String((int)(progress * 100)) + "% " + GetSubsystem<SceneManager>()->GetStatusMessage() + "...");

    if (_loadingBar) {
        _loadingBar->SetWidth(progress * (GetSubsystem<Graphics>()->GetWidth() - 20));
    }

    if (progress >= 1.0f) {
        SendEvent("EndLoading");
        UnsubscribeFromEvent(E_UPDATE);
        GetSubsystem<SceneManager>()->ResetProgress();
    }
}

void Loading::HandleEndLoading(StringHash eventType, VariantMap& eventData)
{
	UnsubscribeFromEvent(E_UPDATE);
	VariantMap data = GetEventDataMap();
	data["Name"] = "Level";
    SendEvent(MyEvents::E_SET_LEVEL, data);
}

void Loading::CreateProgressBar()
{
    if (GetSubsystem<ConfigManager>()->GetBool("game", "ShowProgressBar", true)) {
        UI *ui = GetSubsystem<UI>();
        ResourceCache *cache = GetSubsystem<ResourceCache>();

        // Get the Urho3D fish texture
        auto *progressBarTexture = cache->GetResource<Texture2D>("Textures/Loading.png");
        // Create a new sprite, set it to use the texture
        _loadingBar = ui->GetRoot()->CreateChild<Sprite>();
        _loadingBar->SetTexture(progressBarTexture);

        auto *graphics = GetSubsystem<Graphics>();

        // Get rendering window size as floats
        auto width = (float) graphics->GetWidth();
        auto height = (float) graphics->GetHeight();

        // The UI root element is as big as the rendering window, set random position within it
        _loadingBar->SetPosition(10, height - 30);

        // Set sprite size & hotspot in its center
        _loadingBar->SetSize(0, 20);
    }
}