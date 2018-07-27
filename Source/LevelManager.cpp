#pragma once

#include "LevelManager.h"
#include "Levels/Splash.h"
#include "Levels/MainMenu.h"
#include "Levels/Level.h"
#include "Levels/ExitGame.h"
#include "Levels/Loading.h"
#include "Levels/Credits.h"
#include "UI/NuklearUI.h"

static struct nk_rect _outerRect;

LevelManager::LevelManager(Context* context) :
    Object(context)
{
    // Register all classes
    RegisterAllFactories();

    // Listen to set level event
    SubscribeToEvent(MyEvents::E_SET_LEVEL, URHO3D_HANDLER(LevelManager, HandleSetLevelQueue));

    auto graphics = GetSubsystem<Graphics>();

    _outerRect.x = 0;
    _outerRect.w = graphics->GetWidth();
    _outerRect.y = 0;
    _outerRect.h = graphics->GetHeight();
}

LevelManager::~LevelManager()
{
}

void LevelManager::RegisterAllFactories()
{
    // Register classes
    context_->RegisterFactory<Levels::Splash>();
    context_->RegisterFactory<Levels::Level>();
    context_->RegisterFactory<Levels::MainMenu>();
    context_->RegisterFactory<Levels::ExitGame>();
    context_->RegisterFactory<Levels::Loading>();
	context_->RegisterFactory<Levels::Credits>();
}

void LevelManager::HandleSetLevelQueue(StringHash eventType, VariantMap& eventData)
{
    // Busy now
    if (level_queue_.Size() == 0) {
        // Init fade status
        fade_status_ = 0;
    }

    // Push to queue
    level_queue_.Push(eventData["Name"].GetString());
    data_ = eventData;

    // Subscribe HandleUpdate() function for processing update events
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(LevelManager, HandleUpdate));
}

void LevelManager::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;

    // Take the frame time step, which is stored as a float
    float timeStep = eventData[P_TIMESTEP].GetFloat();

    // Move sprites, scale movement with time step
    fade_time_ -= timeStep;

    // Prepare to fade out
    if (fade_status_ == 0) {
        using namespace MyEvents::LevelChangingStarted;
        VariantMap data = GetEventDataMap();
        data[P_FROM] = currentLevel_;
        data[P_TO] = level_queue_.Front();
        SendEvent(MyEvents::E_LEVEL_CHANGING_STARTED, data);
        // No old level
        if (!level_) {
            fade_status_++;
            return;
        }

        BaseLevel* baseLevel = level_->Cast<BaseLevel>();
        baseLevel->SetExpired();
        // Add a new fade layer
        fade_time_ = MAX_FADE_TIME;
        fade_status_++;
        return;
    }

    // Fade out
    if (fade_status_ == 1) {
        DrawFade(1.0f - fade_time_ / MAX_FADE_TIME);
        // No old level
        if (!level_) {
            fade_status_++;
            return;
        }

        // Increase fade status
        if (fade_time_ <= 0.0f) {
            fade_status_++;
        }
        return;
    }

    // Release old level
    if (fade_status_ == 2) {
        DrawFade(1.0f);
        // No old level
        if (!level_) {
            fade_status_++;
            return;
        }
        // We can not create new level here, or it may cause errors, we have to create it at the next update point.
        level_ = SharedPtr<Object>();
        fade_status_++;

        // Send event to close all active UI windows
        SendEvent(MyEvents::E_CLOSE_ALL_WINDOWS);
        return;
    }

    // Create new level
    if (fade_status_ == 3) {
        // Create new level
        level_ = context_->CreateObject(StringHash(level_queue_.Front()));
        previousLevel_ = currentLevel_;
        currentLevel_ = level_queue_.Front();
        level_->SendEvent("LevelStart", data_);
        // Add a new fade layer
        fade_time_ = MAX_FADE_TIME;
        fade_status_++;
        DrawFade(1.0f);
        return;
    }

    // Fade in
    if (fade_status_ == 4) {
        DrawFade(fade_time_ / MAX_FADE_TIME);

        // Increase fade status
        if (fade_time_ <= 0.0f) {
            fade_status_++;
        }
        return;
    }

    // Finished
    if (fade_status_ == 5) {
        // Unsubscribe update event
        UnsubscribeFromEvent(E_UPDATE);

        {
            using namespace MyEvents::LevelChangingFinished;
            VariantMap data = GetEventDataMap();
            data[P_FROM] = previousLevel_;
            data[P_TO] = level_queue_.Front();
            SendEvent(MyEvents::E_LEVEL_CHANGING_FINISHED, data);
        }

        // Remove the task
        level_queue_.PopFront();

        // Release all unused resources
        GetSubsystem<ResourceCache>()->ReleaseAllResources(false);

        if (level_queue_.Size()) {
            // Subscribe HandleUpdate() function for processing update events
            SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(LevelManager, HandleUpdate));

            // // Init fade status
            fade_status_ = 0;
        }
        return;
    }
}

void LevelManager::DrawFade(float opacity)
{
    auto nuklear = GetSubsystem<NuklearUI>();
    auto ctx = nuklear->GetNkContext();
    if (nuklear && ctx) {

        nk_style_default(ctx);

        int alpha = 255 * opacity;

        ctx->style.window.background = nk_rgba(0, 0, 0, alpha);
        ctx->style.window.fixed_background = nk_style_item_color(nk_rgba(0, 0, 0, alpha));
        ctx->style.window.border_color = nk_rgb(255, 165, 0);
        ctx->style.window.combo_border_color = nk_rgb(255, 165, 0);
        ctx->style.window.contextual_border_color = nk_rgb(255, 165, 0);
        ctx->style.window.menu_border_color = nk_rgb(255, 165, 0);
        ctx->style.window.group_border_color = nk_rgb(255, 165, 0);
        ctx->style.window.tooltip_border_color = nk_rgb(255, 165, 0);
        ctx->style.window.scrollbar_size = nk_vec2(16, 16);
        ctx->style.window.border_color = nk_rgba(0, 0, 0, 0);
        ctx->style.window.border = 1;

        if (nk_begin(nuklear->GetNkContext(), "FadeOuter", _outerRect, NK_WINDOW_NO_SCROLLBAR)) {

        }

        nk_end(ctx);

        nk_window_set_focus(nuklear->GetNkContext(), "FadeOuter");
    }
}