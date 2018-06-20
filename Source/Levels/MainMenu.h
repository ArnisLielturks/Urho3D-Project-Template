#pragma once

#include <Urho3D/Urho3DAll.h>
#include "../BaseLevel.h"
#include "../Globals.h"
#include <vector>

namespace Levels {

	enum Maps {
		CAFFE,
		BALL,
		PARK,
		NONE
	};

	enum WindowTypes {
		WINDOW_NONE,
		NICKNAME,
		ADDRESS,
		NEW_GAME,
		NEW_GAME_SETTINGS,
		CHOOSE_MAP,
		PLAY_WITH_FRIENDS,
		PLAY_WITH_STRANGERS,
		SERVER_LIST
	};

	class MainMenu : public BaseLevel
	{
		URHO3D_OBJECT(MainMenu, BaseLevel);

	public:
		/// Construct.
		MainMenu(Context* context);

		virtual ~MainMenu();
		void HandleUpdate(StringHash eventType, VariantMap& eventData);

	protected:
		virtual void Init();

	private:
		void CreateScene();

		void CreateUI();

		void SubscribeToEvents();

		void CreateSounds();
		void HandleStartGame(StringHash eventType, VariantMap& eventData);

		SharedPtr<Button> _startButton;
	};
}