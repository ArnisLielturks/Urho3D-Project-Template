#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Input/Input.h>

#include "ServiceCmd.h"
#include "ServiceEvents.h"

using namespace ServiceEvents;

static ServiceCmd *handler = nullptr;

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


ServiceCmd::ServiceCmd(Context* context)
        : Object(context)
{
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(ServiceCmd, HandleUpdate));
    handler = this;
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
    MutexLock lock(mutexMessageLock_);

    messageList_.Resize(messageList_.Size() + 1);
    MessageData &messageData = messageList_[messageList_.Size() - 1];

    messageData.command  = cmd;
    messageData.status   = status;
    messageData.message  = message ? String(message) : String::EMPTY;
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
    eventData[P_COMMAND]  = msg.command;
    eventData[P_STATUS]   = msg.status;
    eventData[P_MESSAGE]  = msg.message;

    SendEvent(E_SERVICE_MESSAGE, eventData);

    eventData["Message"] = "Got cmd: " + String(msg.command) + "; Status: " + String(msg.status) + "; Msg: " + msg.message;
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

void ServiceCmd::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;
    ProcessMessageQueue();
}
