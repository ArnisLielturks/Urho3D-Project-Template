#pragma once

#include "../BaseLevel.h"

namespace Levels {

    class Splash : public BaseLevel
    {
        URHO3D_OBJECT(Splash, BaseLevel);

    public:
        Splash(Context* context);
        virtual ~Splash();
        static void RegisterObject(Context* context);
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
        Timer timer_;

        /**
         * Current logo index
         */
        int logoIndex_;

        /**
         * List of all the logos that splash screen should show
         */
        Vector<String> logos_;
    };
}
