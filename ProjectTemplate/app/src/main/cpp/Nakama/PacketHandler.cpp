#ifdef NAKAMA_SUPPORT
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/IO/Log.h>
#include "PacketHandler.h"
#include "NetworkProtocol.h"

using namespace Urho3D;

void PacketHandler::Handle(const NMatchData& data)
{
    MemoryBuffer buffer(data.data.c_str(), data.data.size());
    switch (data.opCode) {
        case NakamaProtocol::MSG_POSITION: {
            Vector3 position = buffer.ReadVector3();
            URHO3D_LOGINFO("Received MSG_POSITION " + position.ToString());
            break;
        }
        default: {
            URHO3D_LOGWARNINGF("Unknown message received from server: %d", data.opCode);
        }
    }
    URHO3D_LOGINFOF("Received match data %d", data.opCode);
}
#endif
