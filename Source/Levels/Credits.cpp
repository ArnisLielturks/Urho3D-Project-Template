#include <Urho3D/UI/UI.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Scene/ObjectAnimation.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/UI/Font.h>
#include "Credits.h"
#include "../MyEvents.h"
#include "../Global.h"
#include "../Messages/Achievements.h"
#include "../AndroidEvents/ServiceCmd.h"

using namespace Levels;
using namespace Urho3D;

const static float CREDITS_SCROLL_SPEED = 50.0f;

namespace Levels {
    /// Construct.
    Credits::Credits(Context* context) :
            BaseLevel(context),
            _offset(0)
    {
    }

    Credits::~Credits()
    {
    }

    void Credits::Init()
    {
        // Disable achievement showing for this level
        GetSubsystem<Achievements>()->SetShowAchievements(false);

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
        _creditsBase->SetAlignment(HA_CENTER, VA_TOP);
        _creditsBase->SetStyleAuto();
        _creditsBase->SetLayout(LayoutMode::LM_VERTICAL, 20);

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
        CreateSingleLine("@Modanung", PARAGRAPH);
        CreateSingleLine("", PARAGRAPH);

        CreateSingleLine("Community", HEADER_SIZE);
        CreateSingleLine("", HEADER_MARGIN);
        CreateSingleLine("Android event handler: @Lumak", PARAGRAPH);
        CreateSingleLine("INI file parser: @carnalis", PARAGRAPH);
        CreateSingleLine("Level manager: @artgolf1000", PARAGRAPH);
        CreateSingleLine("Settings window: @PredatorMF", PARAGRAPH);
        CreateSingleLine("Perlin Noise algorithm: @Reputeless", PARAGRAPH);
        CreateSingleLine("", PARAGRAPH);

        CreateSingleLine("Assets", HEADER_SIZE);
        CreateSingleLine("", HEADER_MARGIN);
        CreateSingleLine("Icons: https://game-icons.net", PARAGRAPH);
        CreateSingleLine("Sounds and music: https://freesound.org", PARAGRAPH);
        CreateSingleLine("", PARAGRAPH);;

        CreateSingleLine("Special thanks to the creators", HEADER_SIZE);
        CreateSingleLine("of the Urho3D engine!", HEADER_SIZE);

        _offset = GetSubsystem<Graphics>()->GetHeight() * 1.1 / GetSubsystem<UI>()->GetScale();
        _creditsBase->SetPosition(0, _offset);
        SubscribeToEvents();

        GetSubsystem<ServiceCmd>()->SendCmdMessage(ANDROID_AD_LOAD_REWARDED, 1);

        SubscribeToEvent(MyEvents::E_SERVICE_MESSAGE, [&](StringHash eventType, VariantMap& eventData) {
            using namespace MyEvents::ServiceMessage;
            int eventId = eventData[P_COMMAND].GetInt();
            if (eventId == ANDROID_AD_REWARDED_LOADED) {
                GetSubsystem<ServiceCmd>()->SendCmdMessage(ANDROID_AD_REWARDED_SHOW, 1);
            }
        });
    }

    void Credits::SubscribeToEvents()
    {
        SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Credits, HandleUpdate));
    }

    void Credits::HandleUpdate(StringHash eventType, VariantMap& eventData)
    {
        using namespace Update;
        float timestep = eventData[P_TIMESTEP].GetFloat();
        Input* input = GetSubsystem<Input>();
        if (input->IsMouseVisible()) {
            input->SetMouseVisible(false);
        }
        if (input->GetKeyDown(KEY_SPACE)) {
            UnsubscribeFromEvent(E_UPDATE);
            HandleEndCredits(true);
        }
        _offset -= timestep * CREDITS_SCROLL_SPEED * GetSubsystem<UI>()->GetScale();
        _creditsBase->SetPosition(_creditsBase->GetPosition().x_, _offset);

        if (_credits.Back()) {
            if (_credits.Back()->GetScreenPosition().y_ < -100) {
                HandleEndCredits(false);
            }
        }
    }

    void Credits::HandleEndCredits(bool forced)
    {
        UnsubscribeFromEvent(E_UPDATE);
        VariantMap& data = GetEventDataMap();
        data["Name"] = "MainMenu";
        SendEvent(MyEvents::E_SET_LEVEL, data);

        if (!forced) {
            SendEvent("CreditsEnd");
        }
    }

    UIElement* Credits::CreateEmptyLine(int height)
    {
        SharedPtr<UIElement> line(_creditsBase->CreateChild<UIElement>());
        line->SetAlignment(HA_CENTER, VA_TOP);
        line->SetFixedHeight(height);
        _credits.Push(line);
        return line;
    }

    void Credits::CreateSingleLine(String content, int fontSize)
    {
        auto cache = GetSubsystem<ResourceCache>();
        auto* font = cache->GetResource<Font>(APPLICATION_FONT);

        SharedPtr<Text> text(CreateEmptyLine(fontSize)->CreateChild<Text>());
        text->SetAlignment(HA_CENTER, VA_TOP);
        text->SetStyleAuto();
        text->SetFont(font, fontSize);
        text->SetText(content);
    }

    void Credits::CreateImageLine(const String& image, int size)
    {
        auto cache = GetSubsystem<ResourceCache>();
        auto texture = cache->GetResource<Texture2D>(image);
        float originalWidth = texture->GetWidth();
        float originalHeight = texture->GetHeight();

        SharedPtr<Sprite> sprite(CreateEmptyLine(size)->CreateChild<Sprite>());
        sprite->SetTexture(texture);
        sprite->SetFixedHeight(size);
        sprite->SetFixedWidth(((float)sprite->GetHeight() / originalHeight) * originalWidth);
        sprite->SetHotSpot(sprite->GetWidth() / 2, sprite->GetHeight() / 2);
    }
}
