#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/IO/Log.h>

#include "ServiceCmd.h"

#include <Urho3D/DebugNew.h>
//=============================================================================
//=============================================================================
#if defined(__ANDROID__)
#include <SDL/SDL.h>
extern "C"
{
int Android_JNI_SendMessage(int command, int param);
}

#else
// create dummy interface for all non-Android platforms
typedef void (*UserActivityCallback)(int id1, int istat, const char *str, void *param);
void SDL_RegisterUserActivityCallback(UserActivityCallback callback, void *param){}
int Android_JNI_SendMessage(int command, int param){ return 0; }
#endif

//=============================================================================
//=============================================================================
ServiceCmd::ServiceCmd(Context* context)
        : Object(context)
{
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(ServiceCmd, HandleUpdate));
//    SDL_RegisterUserActivityCallback(&ServiceCmd::JavaActivityCallback, this);
}

ServiceCmd::~ServiceCmd()
{
//    SDL_RegisterUserActivityCallback(NULL, NULL);
}

void ServiceCmd::JavaActivityCallback(int ival, int istat, const char *pstr, void *param)
{
    if (param)
    {
        ((ServiceCmd*)param)->ActivityCallback(ival, istat, pstr);
    }
}

void ServiceCmd::SendCmdMessage(int cmd, int param)
{
    Android_JNI_SendMessage(cmd, param);
}

void ServiceCmd::ActivityCallback(int val, int stat, const char *pstr)
{
    MutexLock lock(mutexMessageLock_);

    messageList_.Resize(messageList_.Size() + 1);
    MessageData &messageData = messageList_[messageList_.Size() - 1];

    messageData.val_     = val;
    messageData.stat_    = stat;
    messageData.message_ = pstr?String(pstr):String::EMPTY;
}

bool ServiceCmd::HasQueueMessage(MessageData& messageData)
{
    MutexLock lock(mutexMessageLock_);

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
    MutexLock lock(mutexMessageLock_);

    if (messageList_.Size())
    {
        messageList_.Erase(0);
    }
}

void ServiceCmd::SendResponseMsg(const MessageData &msg)
{
    using namespace ServiceMessage;

    VariantMap& eventData = GetEventDataMap();
    eventData[P_COMMAND]  = msg.val_;
    eventData[P_STATUS]   = msg.stat_;
    eventData[P_MESSAGE]  = msg.message_;

    SendEvent(E_SERVICE_MESSAGE, eventData);
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

void ServiceCmd::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;

    ProcessMessageQueue();
}
