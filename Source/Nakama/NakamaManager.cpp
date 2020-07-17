#ifdef NAKAMA_SUPPORT
#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include "NakamaManager.h"
#include "../Console/ConsoleHandlerEvents.h"
#include "NakamaEvents.h"
#include "NetworkProtocol.h"

using namespace Urho3D;
using namespace ConsoleHandlerEvents;
using namespace NakamaEvents;

NakamaManager::NakamaManager(Context* context) :
        Object(context)
{
}

NakamaManager::~NakamaManager()
{
}

void NakamaManager::RegisterObject(Context* context)
{
    context->RegisterFactory<NakamaManager>();
}

void NakamaManager::Init()
{
    NClientParameters parameters;
    parameters.serverKey = "defaultkey";
    parameters.host = "127.0.0.1";
    parameters.port = DEFAULT_PORT;
    client_ = createDefaultClient(parameters);

    SetupRTCClient();

    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(NakamaManager, HandleUpdate));

    SendEvent(
            E_CONSOLE_COMMAND_ADD,
            ConsoleCommandAdd::P_NAME, "nakama_login",
            ConsoleCommandAdd::P_EVENT, "#nakama_login",
            ConsoleCommandAdd::P_DESCRIPTION, "Log with the nakama server",
            ConsoleCommandAdd::P_OVERWRITE, true
    );
    SubscribeToEvent("#nakama_login", [&](StringHash eventType, VariantMap& eventData) {
        URHO3D_LOGINFO("Authenticating with nakama");
        StringVector params = eventData["Parameters"].GetStringVector();
        if (params.Size() != 3) {
            URHO3D_LOGERROR("You must provide email and password");
            return;
        }
        LogIn(params[1], params[2]);
    });

    SendEvent(
            E_CONSOLE_COMMAND_ADD,
            ConsoleCommandAdd::P_NAME, "nakama_login1",
            ConsoleCommandAdd::P_EVENT, "#nakama_login1",
            ConsoleCommandAdd::P_DESCRIPTION, "Log with the nakama server",
            ConsoleCommandAdd::P_OVERWRITE, true
    );
    SubscribeToEvent("#nakama_login1", [&](StringHash eventType, VariantMap& eventData) {
        URHO3D_LOGINFO("Authenticating with nakama");
        LogIn("hello@example.com", "somesupersecretpassword");
    });

    SendEvent(
            E_CONSOLE_COMMAND_ADD,
            ConsoleCommandAdd::P_NAME, "nakama_login2",
            ConsoleCommandAdd::P_EVENT, "#nakama_login2",
            ConsoleCommandAdd::P_DESCRIPTION, "Log with the nakama server",
            ConsoleCommandAdd::P_OVERWRITE, true
    );
    SubscribeToEvent("#nakama_login2", [&](StringHash eventType, VariantMap& eventData) {
        URHO3D_LOGINFO("Authenticating with nakama");
        LogIn("test@test.lv", "password");
    });

    SendEvent(
            E_CONSOLE_COMMAND_ADD,
            ConsoleCommandAdd::P_NAME, "nakama_register",
            ConsoleCommandAdd::P_EVENT, "#nakama_register",
            ConsoleCommandAdd::P_DESCRIPTION, "Register in nakama server",
            ConsoleCommandAdd::P_OVERWRITE, true
    );
    SubscribeToEvent("#nakama_register", [&](StringHash eventType, VariantMap& eventData) {
        URHO3D_LOGINFO("Authenticating with nakama");
        StringVector params = eventData["Parameters"].GetStringVector();
        if (params.Size() != 4) {
            URHO3D_LOGERROR("You must provide email, password and username");
            return;
        }
        Register(params[1], params[2], params[3]);
    });

    SendEvent(
            E_CONSOLE_COMMAND_ADD,
            ConsoleCommandAdd::P_NAME, "nakama_read",
            ConsoleCommandAdd::P_EVENT, "#nakama_read",
            ConsoleCommandAdd::P_DESCRIPTION, "Read user data from nakama",
            ConsoleCommandAdd::P_OVERWRITE, true
    );
    SubscribeToEvent("#nakama_read", [&](StringHash eventType, VariantMap& eventData) {
        StringVector params = eventData["Parameters"].GetStringVector();
        if (params.Size() != 2) {
            URHO3D_LOGERROR("You must specify the key name");
            return;
        }
        if (!session_ || session_->isExpired()) {
            URHO3D_LOGERROR("You must login into nakama server first!");
            return;
        }
        URHO3D_LOGINFO("Reading data from nakama server");
        auto readFailed = [&](const NError& error)
        {
            URHO3D_LOGWARNING("Failed to read from storage");
            SendEvent(E_LOGIN_FAILED);
        };

        auto readSuccess = [&](const NStorageObjects& storageObjects)
        {
            URHO3D_LOGINFO("Storage read succeeded");
            for (auto it = storageObjects.begin(); it != storageObjects.end(); ++it) {
                String collection((*it).collection.c_str());
                String key((*it).key.c_str());
                String value((*it).value.c_str());
                URHO3D_LOGINFOF("Key: %s", (*it).key.c_str());
                URHO3D_LOGINFOF("Value: %s", (*it).value.c_str());
                if (storage_.Contains(collection)) {
                    VariantMap data = storage_[collection].GetVariantMap();
                    data[key] = value;
                    storage_[collection] = data;
                } else {
                    VariantMap data;
                    data[key] = value;
                    storage_[collection] = data;
                }

            }
        };

        std::vector<NReadStorageObjectId> objectIds;
        objectIds.push_back({"collection", params[1].CString(), session_->getUserId()});
        client_->readStorageObjects(session_, objectIds, readSuccess, readFailed);
    });

    SendEvent(
            E_CONSOLE_COMMAND_ADD,
            ConsoleCommandAdd::P_NAME, "nakama_create_match",
            ConsoleCommandAdd::P_EVENT, "#nakama_create_match",
            ConsoleCommandAdd::P_DESCRIPTION, "Create a match",
            ConsoleCommandAdd::P_OVERWRITE, true
    );
    SubscribeToEvent("#nakama_create_match", [&](StringHash eventType, VariantMap& eventData) {
        StringVector params = eventData["Parameters"].GetStringVector();
        if (params.Size() != 1) {
            URHO3D_LOGERROR("This command doesn't take any arguments!");
            return;
        }
        CreateMatch();
    });

    SendEvent(
            E_CONSOLE_COMMAND_ADD,
            ConsoleCommandAdd::P_NAME, "nakama_join_match",
            ConsoleCommandAdd::P_EVENT, "#nakama_join_match",
            ConsoleCommandAdd::P_DESCRIPTION, "Join a match",
            ConsoleCommandAdd::P_OVERWRITE, true
    );
    SubscribeToEvent("#nakama_join_match", [&](StringHash eventType, VariantMap& eventData) {
        StringVector params = eventData["Parameters"].GetStringVector();
        if (params.Size() != 2) {
            URHO3D_LOGERROR("You must specify match id");
            return;
        }
        JoinMatch(params[1]);
    });

    SendEvent(
            E_CONSOLE_COMMAND_ADD,
            ConsoleCommandAdd::P_NAME, "nakama_matchmaker",
            ConsoleCommandAdd::P_EVENT, "#nakama_matchmaker",
            ConsoleCommandAdd::P_DESCRIPTION, "Create matchmaker",
            ConsoleCommandAdd::P_OVERWRITE, true
    );
    SubscribeToEvent("#nakama_matchmaker", [&](StringHash eventType, VariantMap& eventData) {
        StringVector params = eventData["Parameters"].GetStringVector();
        if (params.Size() != 1) {
            URHO3D_LOGERROR("You must specify match id");
            return;
        }
        auto successCallback = [](const NMatchmakerTicket& ticket)
        {
            URHO3D_LOGINFOF("Matchmaker ticket %s", ticket.ticket.c_str());
        };

        int32_t minCount = 2;
        int32_t maxCount = 4;
        std::string query = "*";
        NStringMap stringProperties;
        NStringDoubleMap numericProperties;

        stringProperties.emplace("region", "europe");
        numericProperties.emplace("rank", 8.0);

        rtClient_->addMatchmaker(
                minCount,
                maxCount,
                query,
                stringProperties,
                numericProperties,
                successCallback);
    });

    SendEvent(
            E_CONSOLE_COMMAND_ADD,
            ConsoleCommandAdd::P_NAME, "nakama_send_match_data",
            ConsoleCommandAdd::P_EVENT, "#nakama_send_match_data",
            ConsoleCommandAdd::P_DESCRIPTION, "Send match data",
            ConsoleCommandAdd::P_OVERWRITE, true
    );
    SubscribeToEvent("#nakama_send_match_data", [&](StringHash eventType, VariantMap& eventData) {
        StringVector params = eventData["Parameters"].GetStringVector();
        if (params.Size() != 2) {
            URHO3D_LOGERROR("You must specify match data opcode");
            return;
        }
        if (rtClient_) {
            VectorBuffer buffer;
            buffer.WriteVector3(Vector3(1, 2, 3));
            SendData(ToInt(params[1]), buffer);
        }
    });

    SendEvent(
            E_CONSOLE_COMMAND_ADD,
            ConsoleCommandAdd::P_NAME, "nakama_send_message",
            ConsoleCommandAdd::P_EVENT, "#nakama_send_message",
            ConsoleCommandAdd::P_DESCRIPTION, "Send chat message",
            ConsoleCommandAdd::P_OVERWRITE, true
    );
    SubscribeToEvent("#nakama_send_message", [&](StringHash eventType, VariantMap& eventData) {
        StringVector params = eventData["Parameters"].GetStringVector();
        if (params.Size() == 1) {
            URHO3D_LOGERROR("You have to specify a message you want to send!");
            return;
        }
        if (rtClient_) {
            String message;
            for (int i = 1; i < params.Size(); i++) {
                if (!message.Empty()) {
                    message += " ";
                }
                message += params[i];
            }
            SendChatMessage(message);
        }
    });

}

void NakamaManager::SetupRTCClient()
{
    rtClient_ = client_->createRtClient();
    rtclistener_.setConnectCallback([this]()
    {
        URHO3D_LOGINFO(">>>> Socket connected");

        rtClient_->updateStatus("Enjoying music", []()
        {
            URHO3D_LOGINFO(">>> Status updated");
        });

        rtClient_->joinChat("global", NChannelType::ROOM, true, false, [&](NChannelPtr channel) {
            globalChatChannel_ = channel;
            ChannelJoined(channel);
        });
    });
    rtclistener_.setChannelMessageCallback([&](const NChannelMessage& message) {
        ChannelMessageReceived(message);
    });
    rtclistener_.setMatchDataCallback([&](const NMatchData& data) {
        MatchDataReceived(data);
    });
    rtclistener_.setMatchmakerMatchedCallback([&](NMatchmakerMatchedPtr matched) {
        MatchmakerMatched(matched);
    });
    rtclistener_.setMatchPresenceCallback([&](const NMatchPresenceEvent& event) {
        MatchPresence(event);
    });
    rtClient_->setListener(&rtclistener_);
}

void NakamaManager::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    if (updateTimer_.GetMSec(false) < 100) {
        return;
    }
    updateTimer_.Reset();
    if (client_) {
        client_->tick();
    }
    if (rtClient_) {
        rtClient_->tick();
    }
}

void NakamaManager::LogIn(const String& email, const String& password)
{
    auto loginFailedCallback = [&](const NError& error)
    {
        URHO3D_LOGWARNING("Nakama login failed");
        SendEvent(E_LOGIN_FAILED);
    };

    auto loginSucceededCallback = [&](NSessionPtr session)
    {
        using namespace LoggedIn;
        VariantMap& data = GetEventDataMap();
        data[P_TOKEN] = String(session->getAuthToken().c_str());
        data[P_USER_ID] = String(session->getUserId().c_str());
        data[P_USERNAME] = String(session->getUsername().c_str());
        data[P_NEW_USER] = session->isCreated();
        URHO3D_LOGINFOF("Nakama login succes, user id = %s", session->getUserId().c_str());
        session->getAuthToken();
        session_ = session;
        rtClient_->connect(session_, true);
    };

    client_->authenticateEmail(email.CString(), password.CString(), "", true, {}, loginSucceededCallback, loginFailedCallback);
}

void NakamaManager::Register(const String& email, const String& password, const String& username)
{
    auto loginFailedCallback = [&](const NError& error)
    {
        URHO3D_LOGWARNING("Nakama register failed");
        SendEvent(E_LOGIN_FAILED);
    };

    auto loginSucceededCallback = [&](NSessionPtr session)
    {
        using namespace LoggedIn;
        VariantMap& data = GetEventDataMap();
        data[P_TOKEN] = String(session->getAuthToken().c_str());
        data[P_USER_ID] = String(session->getUserId().c_str());
        data[P_USERNAME] = String(session->getUsername().c_str());
        data[P_NEW_USER] = session->isCreated();
        URHO3D_LOGINFOF("Nakama register success, user id = %s", session->getUserId().c_str());
        session->getAuthToken();
        session_ = session;
        rtClient_->connect(session_, true);
    };

    client_->authenticateEmail(email.CString(), password.CString(), username.CString(), true, {}, loginSucceededCallback, loginFailedCallback);
}

void NakamaManager::ChannelMessageReceived(const NChannelMessage& message)
{
    URHO3D_LOGINFOF("Chat message received %s:%s", message.username.c_str(), message.content.c_str());
}

void NakamaManager::CreateMatch()
{
    if (!rtClient_) {
        URHO3D_LOGERROR("You must login into nakama first!");
    }
    rtClient_->createMatch([&](const NMatch& match)
    {
      URHO3D_LOGINFOF("Match created, ID=%s", match.matchId.c_str());
        matchId_ = String(match.matchId.c_str());
    });
}

void NakamaManager::JoinMatch(const String& matchID)
{
    if (!rtClient_) {
        URHO3D_LOGERROR("You must login into nakama first!");
    }
    rtClient_->joinMatch(matchID.CString(), {}, [&](const NMatch& match)
    {
        URHO3D_LOGINFOF("Joined match! Listing users:");
        matchId_ = matchID;
        for (auto& presence : match.presences)
        {
            if (presence.userId != match.self.userId)
            {
                URHO3D_LOGINFOF("User id=%s username=%s", presence.userId.c_str(), presence.username.c_str());
            }
        }
    });
}

void NakamaManager::MatchDataReceived(const NMatchData& data)
{
    packetHandler_.Handle(data);
}

void NakamaManager::MatchmakerMatched(NMatchmakerMatchedPtr matched)
{
    URHO3D_LOGINFOF("Matchmaker matched, match id=%s", matched->token.c_str());
    rtClient_->joinMatchByToken(matched->token, [&](const NMatch& match) {
        URHO3D_LOGINFOF("Joined match! Listing users:");
        matchId_ = match.matchId.c_str();
        for (auto& presence : match.presences)
        {
            if (presence.userId != match.self.userId)
            {
                URHO3D_LOGINFOF("User id=%s username=%s", presence.userId.c_str(), presence.username.c_str());
            }
        }
    });
}

void NakamaManager::MatchPresence(const NMatchPresenceEvent& event)
{
    for (auto it = event.joins.begin(); it != event.joins.end(); ++it) {
        URHO3D_LOGINFOF("User has entered match %s", (*it).username.c_str());
    }
    for (auto it = event.leaves.begin(); it != event.leaves.end(); ++it) {
        URHO3D_LOGINFOF("User has exited match %s", (*it).username.c_str());
    }
}

void NakamaManager::ChannelJoined(NChannelPtr channel)
{
    SendChatMessage("Hello, I'm " + String(session_->getUsername().c_str()));
}

void NakamaManager::SendChatMessage(const String& message)
{
    if (!globalChatChannel_) {
        URHO3D_LOGERROR("You must join a chat first!");
        return;
    }
    String content = "{\"content\":\"" + message + "\"}";
    rtClient_->writeChatMessage(globalChatChannel_->id, content.CString());
}

void NakamaManager::SendData(int msgID, const VectorBuffer& data)
{
    if (rtClient_) {
        NBytes content(reinterpret_cast<char const*>(data.GetBuffer().Buffer()), data.GetBuffer().Size());
        rtClient_->sendMatchData(matchId_.CString(), msgID, content);
    }
}

void NakamaManager::LeaveMatch()
{
    if (rtClient_) {
        rtClient_->leaveMatch(matchId_.CString());
        matchId_.Clear();
    }
}
#endif
