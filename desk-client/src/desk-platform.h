#pragma once

#include "desk-defines.h"

namespace fin {
    void                TCPClientSocket( const std::string_view & ip, const std::string_view & port );
    void                TCPClientSendPacket( const void * data, int size );
    bool                TCPClientRecievePacket( PacketBuffer & buffer );

    DeviceInterface *   GetDeviceInterface();
}
