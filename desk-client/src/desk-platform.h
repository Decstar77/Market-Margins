#pragma once

#include "desk-defines.h"

namespace fin {
    void                TCPClientSocket( const std::string_view & ip, const std::string_view & port );
    void                TCPClientSendPacket( const void * data, int size );

    void                UDPClientSocket( const std::string_view & ip, const std::string_view & port );
    void                UDPClientRecvPacket( PacketBuffer & buffer );

    DeviceInterface *   GetDeviceInterface();
}
