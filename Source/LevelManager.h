#pragma once

#include <Urho3D/Urho3DAll.h>
#include "Levels/Splash.h"
#include "Levels/MainMenu.h"
#include "Levels/Level.h"
#include "Levels/ExitGame.h"
#include "Levels/Loading.h"
#include "MyEvents.h"

class LevelManager : public Object
{
	URHO3D_OBJECT(LevelManager, Object);
public:
	LevelManager(Context* context) :
		Object(context)
	{
		// Register all classes
		RegisterAllFactories();

		// Listen to set level event
		SubscribeToEvent(MyEvents::E_SET_LEVEL, URHO3D_HANDLER(LevelManager, HandleSetLevelQueue));
	}

private:
	void RegisterAllFactories()
	{
		// Register classes
		context_->RegisterFactory<Levels::Splash>();
		context_->RegisterFactory<Levels::Level>();
		context_->RegisterFactory<Levels::MainMenu>();
		context_->RegisterFactory<Levels::ExitGame>();
		context_->RegisterFactory<Levels::Loading>();
	}

	void HandleSetLevelQueue(StringHash eventType, VariantMap& eventData)
	{
		// Busy now
		if (level_queue_.Size()) {
			URHO3D_LOGERROR("Level manager busy! Level already in the queue: " + level_queue_.Front());
			return;
		}
		// Push to queue
		level_queue_.Push(eventData[MyEvents::E_SET_LEVEL].GetString());
		data_ = eventData;

		// Subscribe HandleUpdate() function for processing update events
		SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(LevelManager, HandleUpdate));

		// Init fade status
		fade_status_ = 0;
	}

	void HandleUpdate(StringHash eventType, VariantMap& eventData)
	{
		using namespace Update;

		// Take the frame time step, which is stored as a float
		float timeStep = eventData[P_TIMESTEP].GetFloat();

		// Move sprites, scale movement with time step
		fade_time_ -= timeStep;

		// Prepare to fade out
		if (fade_status_ == 0) {
			// No old level
			if (!level_) {
				fade_status_++;
				return;
			}
			// Add a new fade layer
			AddFadeLayer();
			fade_window_->SetOpacity(0.0f);
			fade_time_ = MAX_FADE_TIME;
			fade_status_++;
			return;
		}

		// Fade out
		if (fade_status_ == 1) {
			// No old level
			if (!level_) {
				fade_status_++;
				return;
			}
			fade_window_->SetOpacity(1.0f - fade_time_ / MAX_FADE_TIME);

			// Increase fade status
			if (fade_time_ <= 0.0f) {
				fade_status_++;
			}
			return;
		}

		// Release old level
		if (fade_status_ == 2) {
			// No old level
			if (!level_) {
				fade_status_++;
				return;
			}
			// We can not create new level here, or it may cause errors, we have to create it at the next update point.
			level_ = SharedPtr<Object>();
			fade_status_++;
			return;
		}

		// Create new level
		if (fade_status_ == 3) {
			// Create new level
			level_ = context_->CreateObject(StringHash(level_queue_.Front()));
			level_->SendEvent("LevelStart", data_);
			// Remove the old fade layer
			if (fade_window_) {
				fade_window_->Remove();
			}
			// Add a new fade layer
			AddFadeLayer();
			fade_window_->SetOpacity(1.0f);
			fade_time_ = MAX_FADE_TIME;
			fade_status_++;
			return;
		}

		// Fade in
		if (fade_status_ == 4) {
			fade_window_->SetOpacity(fade_time_ / MAX_FADE_TIME);

			// Increase fade status
			if (fade_time_ <= 0.0f) {
				fade_status_++;
			}
			return;
		}

		// Finished
		if (fade_status_ == 5) {
			// Remove fade layer
			fade_window_->Remove();
			fade_window_ = SharedPtr<Window>();
			// Unsubscribe update event
			UnsubscribeFromEvent(E_UPDATE);
			// Remove the task
			level_queue_.PopFront();
			level_->SendEvent("LevelLoaded");
			// Release all unused resources
			GetSubsystem<ResourceCache>()->ReleaseAllResources(false);
			return;
		}
	}

	void AddFadeLayer()
	{
		fade_window_ = new Window(context_);
		// Make the window a child of the root element, which fills the whole screen.
		GetSubsystem<UI>()->GetRoot()->AddChild(fade_window_);
		fade_window_->SetSize(GetSubsystem<Graphics>()->GetWidth(), GetSubsystem<Graphics>()->GetHeight());
		fade_window_->SetLayout(LM_FREE);
		// Urho has three layouts: LM_FREE, LM_HORIZONTAL and LM_VERTICAL.
		// In LM_FREE the child elements of this window can be arranged freely.
		// In the other two they are arranged as a horizontal or vertical list.

		// Center this window in it's parent element.
		fade_window_->SetAlignment(HA_CENTER, VA_CENTER);
		// Black color
		fade_window_->SetColor(Color(0.0f, 0.0f, 0.0f, 1.0f));
		// Make it topmost
		fade_window_->BringToFront();
	}

	List<String> level_queue_;
	SharedPtr<Object> level_;
	SharedPtr<Window> fade_window_;
	VariantMap data_;
	float fade_time_;
	int fade_status_;
	const float MAX_FADE_TIME = 0.5f;
};