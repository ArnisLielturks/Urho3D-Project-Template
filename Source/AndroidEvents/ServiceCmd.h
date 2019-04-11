#pragma once

#include <Urho3D/Core/Object.h>

using namespace Urho3D;

namespace Urho3D
{
class Controls;
}

//=============================================================================
//=============================================================================
enum ServiceCommands
{
    // test enums used by characterdemo
    MIN_COMMAND = 6,

    COMMAND_TEST1 = 6,
    COMMAND_TEST2 = 7,
    COMMAND_TEST3 = 8,
    COMMAND_TEST4 = 9,

    MAX_COMMANS = 9,
};

//=============================================================================
//=============================================================================
URHO3D_EVENT(E_SERVICE_MESSAGE, ServiceMessage)
{
    URHO3D_PARAM(P_COMMAND, Command); // int
    URHO3D_PARAM(P_STATUS, Status);   // int
    URHO3D_PARAM(P_MESSAGE, Message); // String
}

//=============================================================================
//=============================================================================
class ServiceCmd : public Object
{
    URHO3D_OBJECT(ServiceCmd, Object);
public:
    ServiceCmd(Context* context);
    ~ServiceCmd();

    void SendCmdMessage(int cmd, int param);

protected:
    static void JavaActivityCallback(int ival, int istat, const char *pstr, void *param);
    void ActivityCallback(int val, int istat, const char *pstr);
    void ProcessMessageQueue();
    void HandleUpdate(StringHash eventType, VariantMap& eventData);

protected:


private:
    struct MessageData
    {
        int val_;
        int stat_;
        String message_;
    };

    Vector<MessageData> messageList_;
    Mutex               mutexMessageLock_;

    bool HasQueueMessage(MessageData& messageData);
    void PopFrontQueue();
    void SendResponseMsg(const MessageData &msg);
};
