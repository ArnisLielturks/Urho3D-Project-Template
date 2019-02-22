#pragma once

#include <Urho3D/Urho3DAll.h>
#include "../BaseLevel.h"

namespace Levels {

    class Credits : public BaseLevel
    {
        URHO3D_OBJECT(Credits, BaseLevel);

	public:
        /// Construct.
        Credits(Context* context);

        virtual ~Credits();

    protected:
        void Init () override;

    private:

        void CreateScene();

        void CreateUI();

        void SubscribeToEvents();

        void HandleEndCredits();

		void CreateSingleLine(String content, int fontSize);
		void CreateImageLine(const String& image, int size);

		void HandleUpdate(StringHash eventType, VariantMap& eventData);

        Timer _timer;

		Vector<SharedPtr<UIElement>> _credits;
		SharedPtr<UIElement> _creditsBase;
		int _totalCreditsHeight;
		int _creditLengthInSeconds;
    };
}