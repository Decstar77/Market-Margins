#pragma once
#include "fin-pch.h"
#include "fin-api.h"

namespace fin {

    void TCPServerSocket( const std::string_view & port );
    void TCPServerSendPacket( const PacketBuffer & buffer );
    void TCPServerMulticastPacket( const void * data, size_t size );
    void TCPServerMulticastPacket( const PacketBuffer & buffer );
    bool TCPServerReceivePacket( PacketBuffer & buffer );

    void UDPServerSocket( const std::string_view& port, const std::string_view& multicastIP );
    void UDPServerMulticastPacket( const void * data, size_t size );
    void UDPServerMulticastPacket( const PacketBuffer & buffer );

    void TCPClientSocket( const std::string_view & ip, const std::string_view & port );
    void TCPClientSendPacket( const std::vector<byte> & data );
    void TCPClientSendPacket( const std::string_view & data );
    bool TCPClientRecievePacket( std::vector<byte> & data );

}


