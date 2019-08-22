#pragma once

#include <Urho3D/Core/Object.h>

using namespace Urho3D;

#if defined(__ANDROID__)
#include <jni.h>
extern "C" {
    JNIEXPORT void JNICALL
    Java_org_libsdl_app_SDLActivity_SendServiceCommand(JNIEnv *env, jobject, jint cmd, jint status, jstring message);
}
#endif

namespace Urho3D
{
class Controls;
}

class ServiceCmd : public Object
{
    URHO3D_OBJECT(ServiceCmd, Object);
public:
    ServiceCmd(Context* context);
    ~ServiceCmd();

    void SendCmdMessage(int cmd, int param);

    void ReceiveCmdMessage(int cmd, int status, const char* message);

protected:
    void ProcessMessageQueue();
    void HandleUpdate(StringHash eventType, VariantMap& eventData);

private:
    struct MessageData
    {
        int command;
        int status;
        String message;
    };

    Vector<MessageData> messageList_;
    Mutex               mutexMessageLock_;

    bool HasQueueMessage(MessageData& messageData);
    void PopFrontQueue();
    void SendResponseMsg(const MessageData &msg);
};
