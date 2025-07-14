#include "fin-pch.h"
#include "fin-platform.h"
#include "fin-queues.h"

#if _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>

namespace fin {
    static SOCKET tcpServerSocket = INVALID_SOCKET;
    static std::thread tcpConnectionThread;
    static std::thread tcpReadThread;
    static std::vector<NetId> tcpClients;

    // NOTE: Small amount for now.
    static SPSCQueue<PacketBuffer> packetQueue( 1024 );

    void TCPServerSocket( const std::string_view & port ) {
        WSADATA wsaData = {};
        if ( WSAStartup( MAKEWORD( 2, 2 ), &wsaData ) != 0 ) {
            throw std::runtime_error( "WSAStartup failed" );
        }

        tcpServerSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
        if ( tcpServerSocket == INVALID_SOCKET ) {
            throw std::runtime_error( "socket failed" );
        }

        // Make socket non-blocking
        u_long iMode = 1;
        if ( ioctlsocket( tcpServerSocket, FIONBIO, &iMode ) != 0 ) {
            throw std::runtime_error( "ioctlsocket failed" );
        }

        sockaddr_in serverAddr = {};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons( std::stoi( port.data() ) );
        serverAddr.sin_addr.s_addr = INADDR_ANY;

        if ( bind( tcpServerSocket, (sockaddr *)&serverAddr, sizeof( serverAddr ) ) == SOCKET_ERROR ) {
            throw std::runtime_error( "bind failed" );
        }

        std::cout << "TCP Server socket bound to port " << port << std::endl;

        tcpConnectionThread = std::thread( [&]()
            {
                std::cout << "Connection thread started" << std::endl;
                while ( true ) {
                    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

                    if ( listen( tcpServerSocket, SOMAXCONN ) == SOCKET_ERROR ) {
                        std::cout << "listen failed" << std::endl;
                        continue;
                    }

                    SOCKET clientSocket = accept( tcpServerSocket, NULL, NULL );
                    if ( clientSocket != INVALID_SOCKET ) {
                        tcpClients.push_back( clientSocket );
                        std::cout << "Client connected" << std::endl;
                    }
                }
            } );

        tcpReadThread = std::thread( [&]()
            {
                std::cout << "Read thread started" << std::endl;
                while ( true ) {
                    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
                    for ( auto & client : tcpClients ) {
                        char buffer[512] = {};
                        int bytesReceived = recv( client, buffer, sizeof( buffer ), 0 );
                        if ( bytesReceived == SOCKET_ERROR ) {
                            int err = WSAGetLastError();
                            if ( err == WSAEWOULDBLOCK ) {
                                continue;
                            }
                            else {
                                std::cerr << "recv failed with error: " << err << "\n";
                            }
                        }
                        else if ( bytesReceived == 0 ) {
                            std::cout << "Connection closed\n";
                        }
                        else {
                            PacketBuffer packet = {};
                            packet.id = client;
                            packet.length = bytesReceived;
                            memcpy_s( packet.data, sizeof( packet.data ), buffer, bytesReceived );
                            packetQueue.Push( packet );
                        }
                    }
                }
            } );
    }

    void TCPServerMulticastPacket( const void * data, size_t size ) {
        for ( auto & client : tcpClients ) {
            send( client, (const char *)data, static_cast<int>(size), 0 );
        }
    }

    void TCPServerMulticastPacket( const PacketBuffer & buffer ) {
        for ( auto & client : tcpClients ) {
            send( client, (const char *)buffer.data, sizeof( buffer.data ), 0 );
        }
    }

    void TCPServerSendPacket( const PacketBuffer & buffer ) {
        send( buffer.id, (const char *)buffer.data, sizeof( buffer.data ), 0 );
    }

    bool TCPServerReceivePacket( PacketBuffer & buffer ) {
        return packetQueue.Pop( buffer );
    }
}

namespace fin {
    static SOCKET udpServerSocket = INVALID_SOCKET;
    static sockaddr_in multicastAddr = {};

    void UDPServerSocket( const std::string_view & port, const std::string_view & multicastIP ) {
        WSADATA wsaData = {};
        if ( WSAStartup( MAKEWORD( 2, 2 ), &wsaData ) != 0 ) {
            throw std::runtime_error( "WSAStartup failed" );
        }

        udpServerSocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
        if ( udpServerSocket == INVALID_SOCKET ) {
            throw std::runtime_error( "socket failed" );
        }

        // Optional: set TTL for multicast packets
        int ttl = 1;
        if ( setsockopt( udpServerSocket, IPPROTO_IP, IP_MULTICAST_TTL, (const char *)&ttl, sizeof( ttl ) ) < 0 ) {
            throw std::runtime_error( "setsockopt TTL failed" );
        }

        // Optional: set outgoing interface (if you have more than one NIC)
        // IN_ADDR localInterface;
        // localInterface.s_addr = inet_addr("192.168.1.100"); // Replace with your interface IP
        // setsockopt(udpServerSocket, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface));

        // Setup the multicast destination address
        multicastAddr = {};
        multicastAddr.sin_family = AF_INET;
        multicastAddr.sin_port = htons( std::stoi( port.data() ) );
        multicastAddr.sin_addr.s_addr = inet_addr( multicastIP.data() );

        std::cout << "Multicast UDP socket created, target IP " << multicastIP << " on port " << port << std::endl;
    }

    void UDPServerMulticastPacket( const void * data, size_t size ) {
        if ( sendto( udpServerSocket, (const char *)data, (int)size, 0, (sockaddr *)&multicastAddr, sizeof( multicastAddr ) ) == SOCKET_ERROR ) {
            std::cerr << "sendto failed with error: " << WSAGetLastError() << std::endl;
        }
    }

    void UDPServerMulticastPacket( const PacketBuffer & buffer ) {
        UDPServerMulticastPacket( buffer.data, buffer.length );
    }

    void UDPServerMulticastPacket( const std::string & text ) {
        UDPServerMulticastPacket( text.data(), text.size() );
    }
}

#else

#endif // _WIN32
