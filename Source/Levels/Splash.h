#pragma once

#include <Urho3D/Urho3DAll.h>
#include "../BaseLevel.h"

namespace Levels {

    class Splash : public BaseLevel
    {
        URHO3D_OBJECT(Splash, BaseLevel);

	public:
        /// Construct.
        Splash(Context* context);

        virtual ~Splash();
        void HandleUpdate(StringHash eventType, VariantMap& eventData);

    protected:
        void Init () override;

    private:
		friend void CheckThreading(const WorkItem* item, unsigned threadIndex);

        void CreateScene();

        void CreateUI();

        void SubscribeToEvents();

        void HandleEndSplash();

		void HandleWorkItemFinished(StringHash eventType, VariantMap& eventData);

        Timer _timer;
    };
}