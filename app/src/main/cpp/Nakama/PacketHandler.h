#pragma once
#ifdef NAKAMA_SUPPORT
#include <nakama-cpp/Nakama.h>

using namespace Nakama;

class PacketHandler {
public:
    void Handle(const NMatchData& data);
};

#endif
