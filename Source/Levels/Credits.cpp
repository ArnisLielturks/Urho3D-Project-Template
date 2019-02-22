#include <Urho3D/Urho3DAll.h>
#include "Credits.h"
#include "../MyEvents.h"
#include "../Global.h"
#include "../Messages/Achievements.h"

using namespace Levels;

namespace Levels {
    /// Construct.
    Credits::Credits(Context* context) :
            BaseLevel(context),
            _totalCreditsHeight(0),
            _creditLengthInSeconds(0)
    {
    }

    Credits::~Credits()
    {
    }

    void Credits::Init()
    {
        // Disable achievement showing for this level
        GetSubsystem<Achievements>()->SetShowAchievements(false);

        BaseLevel::Init();

        // Create the scene content
        CreateScene();

        // Create the UI content
        CreateUI();
    }

    void Credits::CreateScene()
    {
        return;
    }

    void Credits::CreateUI()
    {
        _timer.Reset();
        UI* ui = GetSubsystem<UI>();
        ResourceCache* cache = GetSubsystem<ResourceCache>();

        _creditsBase = ui->GetRoot()->CreateChild<UIElement>();
        _creditsBase->SetAlignment(HA_CENTER, VA_BOTTOM);
        _creditsBase->SetStyleAuto();

        const int HEADER_SIZE = 30;
        const int HEADER_MARGIN = 6;
        const int PARAGRAPH = 20;
        const int IMAGE_SIZE = 50;

        CreateImageLine("Textures/UrhoIcon.png", IMAGE_SIZE);
        CreateSingleLine("Creator", HEADER_SIZE);
        CreateSingleLine("", HEADER_MARGIN);
        CreateSingleLine("Arnis Lielturks", PARAGRAPH);
        CreateSingleLine("", PARAGRAPH);

        CreateSingleLine("Helpers", HEADER_SIZE);
        CreateSingleLine("", HEADER_MARGIN);
        CreateSingleLine("@CG-SS", PARAGRAPH);
        CreateSingleLine("@urnenfeld", PARAGRAPH);
        CreateSingleLine("", PARAGRAPH);

        CreateSingleLine("Community", HEADER_SIZE);
        CreateSingleLine("", HEADER_MARGIN);
        CreateSingleLine("INI file parser: @carnalis", PARAGRAPH);
        CreateSingleLine("INI file parser: @carnalis", PARAGRAPH);
        CreateSingleLine("Level manager: @artgolf1000", PARAGRAPH);
        CreateSingleLine("Icons: https://game-icons.net", PARAGRAPH);
        CreateSingleLine("Sounds and music: https://freesound.org", PARAGRAPH);
        CreateSingleLine("", PARAGRAPH);

        CreateSingleLine("Special thanks to the creators", HEADER_SIZE);
        CreateSingleLine("of the Urho3D engine!", HEADER_SIZE);

        _creditLengthInSeconds = _credits.Size() * 1.5;

        SharedPtr<ObjectAnimation> animation(new ObjectAnimation(context_));
        SharedPtr<ValueAnimation> colorAnimation(new ValueAnimation(context_));
        // Use spline interpolation method
        colorAnimation->SetInterpolationMethod(IM_SPLINE);
        // Set spline tension
        colorAnimation->SetSplineTension(0.7f);
        colorAnimation->SetKeyFrame(0.0f, IntVector2(0, 0));
        colorAnimation->SetKeyFrame(_creditLengthInSeconds, IntVector2(0, -GetSubsystem<Graphics>()->GetHeight() - _totalCreditsHeight - 50));
        colorAnimation->SetKeyFrame(_creditLengthInSeconds * 2, IntVector2(0, -GetSubsystem<Graphics>()->GetHeight() - _totalCreditsHeight - 50));
        animation->AddAttributeAnimation("Position", colorAnimation);

        _creditsBase->SetObjectAnimation(animation);

        SubscribeToEvents();
    }

    void Credits::SubscribeToEvents()
    {
        SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Credits, HandleUpdate));
    }

    void Credits::HandleUpdate(StringHash eventType, VariantMap& eventData)
    {
        Input* input = GetSubsystem<Input>();
        if (input->IsMouseVisible()) {
            input->SetMouseVisible(false);
        }
        if (input->GetKeyDown(KEY_ESCAPE)) {
            UnsubscribeFromEvent(E_UPDATE);
            HandleEndCredits();
        }
        if (_timer.GetMSec(false) > _creditLengthInSeconds * 1000) {
            UnsubscribeFromEvent(E_UPDATE);
            HandleEndCredits();
        }
    }

    void Credits::HandleEndCredits()
    {
        UnsubscribeFromEvent(E_UPDATE);
        VariantMap data = GetEventDataMap();
        data["Name"] = "MainMenu";
        SendEvent(MyEvents::E_SET_LEVEL, data);

        SendEvent("CreditsEnd");
    }

    void Credits::CreateSingleLine(String content, int fontSize)
    {
        _totalCreditsHeight += fontSize + 10;

        auto cache = GetSubsystem<ResourceCache>();
        auto* font = cache->GetResource<Font>(APPLICATION_FONT);

        SharedPtr<Text> text(_creditsBase->CreateChild<Text>());
        text->SetPosition(IntVector2(0, _totalCreditsHeight));
        text->SetAlignment(HA_CENTER, VA_TOP);
        text->SetStyleAuto();
        text->SetFont(font, fontSize);
        text->SetText(content);
        _credits.Push(text);
        _totalCreditsHeight += 20;
    }

    void Credits::CreateImageLine(const String& image, int size)
    {
        _totalCreditsHeight += size;

        auto cache = GetSubsystem<ResourceCache>();
        auto texture = cache->GetResource<Texture2D>(image);
        float originalWidth = texture->GetWidth();
        float originalHeight = texture->GetHeight();

        SharedPtr<Sprite> sprite(_creditsBase->CreateChild<Sprite>());
        sprite->SetTexture(texture);
        sprite->SetFixedHeight(size);
        sprite->SetFixedWidth(((float)sprite->GetHeight() / originalHeight) * originalWidth);
        sprite->SetHotSpot(sprite->GetWidth() / 2, sprite->GetHeight() / 2);

        _credits.Push(sprite);
    }
}