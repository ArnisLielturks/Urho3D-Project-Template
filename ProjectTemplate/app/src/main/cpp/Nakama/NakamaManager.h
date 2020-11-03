#pragma once
#ifdef NAKAMA_SUPPORT
#include <Urho3D/Core/Object.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Core/Timer.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Node.h>
#include <nakama-cpp/Nakama.h>
#include "PacketHandler.h"

using namespace Urho3D;
using namespace Nakama;

class NakamaManager : public Object
{
URHO3D_OBJECT(NakamaManager, Object);

public:
    NakamaManager(Context* context);
    virtual ~NakamaManager();

    void Init();
    static void RegisterObject(Context* context);
    void LogIn(const String& email, const String& password);
    void Register(const String& email, const String& password, const String& username);
    void CreateMatch();
    void JoinMatch(const String& matchID);
    void LeaveMatch();
    void SendChatMessage(const String& message);
    void SendData(int msgID, const VectorBuffer& data);
private:
    void SetupRTCClient();
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void ChannelMessageReceived(const NChannelMessage& message);
    void MatchDataReceived(const NMatchData& data);
    void MatchmakerMatched(NMatchmakerMatchedPtr matched);
    void MatchPresence(const NMatchPresenceEvent& event);
    void ChannelJoined(NChannelPtr channel);
    NClientPtr client_;
    NSessionPtr session_;
    NRtClientPtr rtClient_;
    NChannelPtr globalChatChannel_;
    Timer updateTimer_;
    VariantMap storage_;
    NRtDefaultClientListener rtclistener_;
    String matchId_;
    PacketHandler packetHandler_;
};
#endif
