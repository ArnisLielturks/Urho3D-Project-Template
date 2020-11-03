#pragma once

#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Mutex.h>
//#include "NSObject.h"

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

class ServiceCmd : public Urho3D::Object
{
    URHO3D_OBJECT(ServiceCmd, Object);
public:
    ServiceCmd(Urho3D::Context* context);
    ~ServiceCmd();

    void SendCmdMessage(int cmd, int param);

    void ReceiveCmdMessage(int cmd, int status, const char* message);

    static ServiceCmd* instance;

    void Test();

    void Init();

protected:
    void ProcessMessageQueue();
    void HandleUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

private:
    struct MessageData
    {
        int command;
        int status;
        Urho3D::String message;
    };

    Urho3D::Vector<MessageData> messageList_;
    Urho3D::Mutex               mutexMessageLock_;

    bool HasQueueMessage(MessageData& messageData);
    void PopFrontQueue();
    void SendResponseMsg(const MessageData &msg);
};
