#ifdef __IOS__
//#import <UIKit/UIKit.h>
#endif


#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/Log.h>

#include "ServiceCmd.h"
#include "ServiceEvents.h"

#include "firebase/admob.h"
#include "firebase/admob/interstitial_ad.h"
#include "firebase/admob/rewarded_video.h"
#include "firebase/admob/types.h"
#include "firebase/app.h"
#include "firebase/future.h"
//#include <SDL/SDL.h>
//#include <SDL/SDL_events.h>
//#include <SDL/SDL_syswm.h>
////#include <SDL/SDL_video.h>
//#include <Urho3D/Graphics/Graphics.h>
////#include <Urho3D/ThirdParty/SDL/SDL_syswm.h>
//
using namespace ServiceEvents;
//
static ServiceCmd *handler = nullptr;
ServiceCmd* ServiceCmd::instance = nullptr;
//
//// The AdMob app IDs for the test app.
//#if defined(__ANDROID__)
//// If you change the AdMob app ID for your Android app, make sure to change it
//// in AndroidManifest.xml as well.
//const char* kAdMobAppID = "ca-app-pub-5635283337405077~7109320756";
//#else
//// If you change the AdMob app ID for your iOS app, make sure to change the
//// value for "GADApplicationIdentifier" in your Info.plist as well.
//const char* kAdMobAppID = "ca-app-pub-5635283337405077~7109320756";
//#endif
//
//// These ad units IDs have been created specifically for testing, and will
//// always return test ads.
//#if defined(__ANDROID__)
//const char* kBannerAdUnit = "ca-app-pub-3940256099942544/6300978111";
//const char* kInterstitialAdUnit = "ca-app-pub-3940256099942544/1033173712";
//const char* kRewardedVideoAdUnit = "ca-app-pub-3940256099942544/5224354917";
//#else
//const char* kBannerAdUnit = "ca-app-pub-3940256099942544/2934735716";
//const char* kInterstitialAdUnit = "ca-app-pub-3940256099942544/4411468910";
//const char* kRewardedVideoAdUnit = "ca-app-pub-3940256099942544/1712485313";
//#endif
//
//
//// Standard mobile banner size is 320x50.
//static const int kBannerWidth = 320;
//static const int kBannerHeight = 50;
//
//// Sample keywords to use in making the request.
//static const char* kKeywords[] = {"AdMob", "C++", "Fun"};
//
//// Sample test device IDs to use in making the request.
//static const char* kTestDeviceIDs[] = {"2077ef9a63d2b398840261c8221a0c9b",
//                                       "098fe087d987c9a878965454a65654d7"};
//
//// Sample birthday value to use in making the request.
//static const int kBirthdayDay = 10;
//static const int kBirthdayMonth = 11;
//static const int kBirthdayYear = 1976;

//#ifdef __ANDROID__
//SDL_SysWMinfo wmInfo;
////SDL_VERSION(&wmInfo.version);
////SDL_Window window;
//
//SDL_GetWindowWMInfo(&window, &wmInfo);

//UIWindow* uiWindow = wmInfo.info.uikit.window;
//UIViewController* rootViewController = uiWindow.rootViewController;
//firebase::admob::AdParent uiView = rootViewController.view;
//WindowContext GetWindowContext() { return nullptr; }
//#else
//#endif
//firebase::admob::AdParent windowContext = nullptr;
//
//firebase::admob::AdParent GetWindowContext() { return windowContext; }

#if defined(__ANDROID__)
#include <SDL/SDL.h>
#include <jni.h>
extern "C"
{
    int Android_JNI_SendMessage(int command, int param);


    JNIEXPORT void JNICALL
    Java_org_libsdl_app_SDLActivity_SendServiceCommand(JNIEnv *env, jobject obj, jint cmd, jint status, jstring message) {
        if (handler) {
            const char *nativeString = env->GetStringUTFChars(message, 0);
            handler->ReceiveCmdMessage(cmd, status, nativeString);

            env->ReleaseStringUTFChars(message, nativeString);
        }
    }
}
#else
// create dummy interface for all non-Android platforms
int Android_JNI_SendMessage(int command, int param){ return 0; }
#endif
//
//// A simple listener that logs changes to an InterstitialAd.
//class LoggingInterstitialAdListener
//        : public firebase::admob::InterstitialAd::Listener {
//public:
//    LoggingInterstitialAdListener() {}
//    void OnPresentationStateChanged(
//            firebase::admob::InterstitialAd* interstitial_ad,
//            firebase::admob::InterstitialAd::PresentationState state) override {
//        URHO3D_LOGINFOF("InterstitialAd PresentationState has changed to %d.", state);
//    }
//};
//
//firebase::admob::InterstitialAd* interstitial = nullptr;

ServiceCmd::ServiceCmd(Urho3D::Context* context)
        : Object(context)
{
    SubscribeToEvent(Urho3D::E_UPDATE, URHO3D_HANDLER(ServiceCmd, HandleUpdate));
    handler = this;
    instance = this;
}

ServiceCmd::~ServiceCmd()
{

}

void ServiceCmd::SendCmdMessage(int cmd, int param)
{
    Android_JNI_SendMessage(cmd, param);
}

void ServiceCmd::ReceiveCmdMessage(int cmd, int status, const char* message)
{
    Urho3D::MutexLock lock(mutexMessageLock_);

    messageList_.Resize(messageList_.Size() + 1);
    MessageData &messageData = messageList_[messageList_.Size() - 1];

    messageData.command  = cmd;
    messageData.status   = status;
    messageData.message  = message ? Urho3D::String(message) : Urho3D::String::EMPTY;
}

bool ServiceCmd::HasQueueMessage(MessageData& messageData)
{
    Urho3D::MutexLock lock(mutexMessageLock_);

    bool hasData = false;

    if (messageList_.Size())
    {
        messageData = messageList_[0];
        hasData = true;
    }

    return hasData;
}

void ServiceCmd::PopFrontQueue()
{
    Urho3D::MutexLock lock(mutexMessageLock_);

    if (messageList_.Size())
    {
        messageList_.Erase(0);
    }
}

void ServiceCmd::SendResponseMsg(const MessageData &msg)
{
    using namespace ServiceMessage;

    Urho3D::VariantMap& eventData = GetEventDataMap();
    eventData[P_COMMAND]  = msg.command;
    eventData[P_STATUS]   = msg.status;
    eventData[P_MESSAGE]  = msg.message;

    SendEvent(E_SERVICE_MESSAGE, eventData);

    eventData["Message"] = "Got cmd: " + Urho3D::String(msg.command) + "; Status: " + Urho3D::String(msg.status) + "; Msg: " + msg.message;
    SendEvent("ShowNotification", eventData);
}

void ServiceCmd::ProcessMessageQueue()
{
    MessageData messageData;

    while (HasQueueMessage(messageData))
    {
        SendResponseMsg(messageData);
        PopFrontQueue();
    }
}

void ServiceCmd::HandleUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    using namespace Urho3D::Update;
    ProcessMessageQueue();

//    if (interstitial) {
//        firebase::FutureBase future = interstitial->LoadAdLastResult();
//        if (future.error() == firebase::admob::kAdMobErrorNone) {
////            interstitial->Show();
////            URHO3D_LOGINFOF("Showing interstitial ad");
//        }
//    }
}

//void ServiceCmd::Test()
//{
//    URHO3D_LOGINFO("Received from Objective-C");
//}

void ServiceCmd::Init()
{
//    SDL_Window* window = GetSubsystem<Urho3D::Graphics>()->GetWindow();
//    if (!window) {
//        return;
//    }
//    SDL_SysWMinfo wmInfo;
//SDL_VERSION(&wmInfo.version);
//SDL_Window window;

//    SDL_GetWindowWMInfo(window, &wmInfo);
//    UIWindow* uiWindow = wmInfo.info.uikit.window;
//    UIViewController* rootViewController = uiWindow.rootViewController;
//    windowContext = rootViewController.view;

//    windowContext = wmInfo.info.cocoa.window;
//#ifdef __IOS__
//    NSWindow* uiWindow = wmInfo.info.cocoa.window;
//    UIViewController* rootViewController = uiWindow->rootViewController;
//    firebase::admob::AdParent uiView = rootViewController->view;
//    windowContext = uiView;
//#endif
//    firebase::App* app;
////    _testClass.Test();
//#if defined(__ANDROID__)
//    app = ::firebase::App::Create(GetJniEnv(), GetActivity());
//#else
//    app = ::firebase::App::Create();
//#endif  // defined(__ANDROID__)
//    firebase::admob::Initialize(*app, kAdMobAppID);
//
//    firebase::admob::AdRequest request;
//    // If the app is aware of the user's gender, it can be added to the targeting
//    // information. Otherwise, "unknown" should be used.
//    request.gender = firebase::admob::kGenderUnknown;
//
//    // This value allows publishers to specify whether they would like the request
//    // to be treated as child-directed for purposes of the Children’s Online
//    // Privacy Protection Act (COPPA).
//    // See http://business.ftc.gov/privacy-and-security/childrens-privacy.
//    request.tagged_for_child_directed_treatment =
//            firebase::admob::kChildDirectedTreatmentStateTagged;
//
//    // The user's birthday, if known. Note that months are indexed from one.
//    request.birthday_day = kBirthdayDay;
//    request.birthday_month = kBirthdayMonth;
//    request.birthday_year = kBirthdayYear;
//
//    // Additional keywords to be used in targeting.
//    request.keyword_count = sizeof(kKeywords) / sizeof(kKeywords[0]);
//    request.keywords = kKeywords;
//
//    // "Extra" key value pairs can be added to the request as well. Typically
//    // these are used when testing new features.
//    static const firebase::admob::KeyValuePair kRequestExtras[] = {
//            {"the_name_of_an_extra", "the_value_for_that_extra"}};
//    request.extras_count = sizeof(kRequestExtras) / sizeof(kRequestExtras[0]);
//    request.extras = kRequestExtras;
//
//    // This example uses ad units that are specially configured to return test ads
//    // for every request. When using your own ad unit IDs, however, it's important
//    // to register the device IDs associated with any devices that will be used to
//    // test the app. This ensures that regardless of the ad unit ID, those
//    // devices will always receive test ads in compliance with AdMob policy.
//    //
//    // Device IDs can be obtained by checking the logcat or the Xcode log while
//    // debugging. They appear as a long string of hex characters.
//    request.test_device_id_count =
//            sizeof(kTestDeviceIDs) / sizeof(kTestDeviceIDs[0]);
//    request.test_device_ids = kTestDeviceIDs;
//
//    // Create and test InterstitialAd.
//    URHO3D_LOGINFO("Creating the InterstitialAd.");
//    interstitial = new firebase::admob::InterstitialAd();
//    interstitial->Initialize(GetWindowContext(), kInterstitialAdUnit);
//    interstitial->LoadAd(request);

//    WaitForFutureCompletion(interstitial->InitializeLastResult());;
}
