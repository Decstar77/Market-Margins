#pragma once
#include "fin-pch.h"
#include "fin-api.h"

namespace fin {

    void TCPServerSocket( const std::string_view & port );
    void TCPServerMulticastPacket( const void * data, size_t size );
    void TCPServerMulticastPacket( const RpcCallData & buffer );
    bool TCPServerReceiveRPC( RpcCallData & buffer );

    void UDPServerSocket( const std::string_view& port, const std::string_view& multicastIP );
    void UDPServerMulticastPacket( const void * data, size_t size );
    void UDPServerMulticastPacket( const PacketBuffer & buffer );
}


