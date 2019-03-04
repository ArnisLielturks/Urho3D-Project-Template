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
        void CreateScene();

        /**
         * Create the actual splash screen content
         */
        void CreateUI();

        void SubscribeToEvents();

        /**
         * Show next screen
         */
        void HandleEndSplash();

		/**
		 * Timer to check splash screen lifetime
		 */
        Timer _timer;

        /**
         * Current logo index
         */
        int _logoIndex;

        /**
         * List of all the logos that splash screen should show
         */
        Vector<String> _logos;
    };
}