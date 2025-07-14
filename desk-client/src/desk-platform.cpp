#include "desk-platform.h"
#include "fin-queues.h"

#include <winsock2.h>
#include <ws2tcpip.h>

namespace fin {
    static SOCKET                   tcpClientSocket = INVALID_SOCKET;
    static std::thread              tcpReadThread;
    static SPSCQueue<PacketBuffer>  tcpPacketQueue( 10000 );

    void TCPClientSocket( const std::string_view & ip, const std::string_view & port ) {
        WSADATA wsaData = {};
        if ( WSAStartup( MAKEWORD( 2, 2 ), &wsaData ) != 0 ) {
            throw std::runtime_error( "WSAStartup failed" );
        }

        tcpClientSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
        if ( tcpClientSocket == INVALID_SOCKET ) {
            throw std::runtime_error( "socket failed" );
        }

        sockaddr_in serverAddr = {};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons( std::stoi( port.data() ) );
        inet_pton( AF_INET, ip.data(), &serverAddr.sin_addr );

        while ( connect( tcpClientSocket, (sockaddr *)&serverAddr, sizeof( serverAddr ) ) == SOCKET_ERROR ) {
            std::cout << "Connecting to server..." << std::endl;
            std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );
        }

        std::cout << "Connected to server" << std::endl;

        tcpReadThread = std::thread( [&]()
            {
                std::cout << "Read thread started" << std::endl;
                while ( true ) {
                    char buffer[512] = {};
                    int bytesReceived = recv( tcpClientSocket, buffer, sizeof( buffer ), 0 );
                    if ( bytesReceived == SOCKET_ERROR ) {
                        int err = WSAGetLastError();
                        if ( err == WSAEWOULDBLOCK ) {
                            std::cout << "No data available right now\n";
                        }
                    }

                    if ( bytesReceived == 0 ) {
                        std::cout << "Connection closed\n";
                        break;
                    }

                    if ( bytesReceived > 0 ) {
                        PacketBuffer packet = {};
                        packet.id = tcpClientSocket;
                        memcpy_s( packet.data, sizeof( packet.data ), buffer, bytesReceived );
                        tcpPacketQueue.Push( packet );
                    }
                }
            } );
    }

    void TCPClientSendPacket( const void * data, int size ) {
        send( tcpClientSocket, (const char *)data, size, 0 );
    }
}

namespace fin {
    static SOCKET                   udpClientSocket = INVALID_SOCKET;
    static std::thread              udpReadThread;
    static WaitQueue<PacketBuffer>  udpPacketQueue;

    void UDPClientSocket( const std::string_view & ip, const std::string_view & port ) {
        WSADATA wsaData;
        if ( WSAStartup( MAKEWORD( 2, 2 ), &wsaData ) != 0 ) {
            throw std::runtime_error( "WSAStartup failed" );
        }

        udpClientSocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
        if ( udpClientSocket == INVALID_SOCKET ) {
            throw std::runtime_error( "socket failed" );
        }

        BOOL reuse = TRUE;
        if ( setsockopt( udpClientSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof( reuse ) ) < 0 ) {
            throw std::runtime_error( "setsockopt SO_REUSEADDR failed" );
        }

        sockaddr_in localAddr = {};
        localAddr.sin_family = AF_INET;
        localAddr.sin_port = htons( std::stoi( port.data() ) );
        localAddr.sin_addr.s_addr = htonl( INADDR_ANY );

        if ( bind( udpClientSocket, (sockaddr *)&localAddr, sizeof( localAddr ) ) < 0 ) {
            throw std::runtime_error( "bind failed" );
        }

        ip_mreq mreq = {};
        mreq.imr_multiaddr.s_addr = inet_addr( ip.data() );
        mreq.imr_interface.s_addr = htonl( INADDR_ANY );

        if ( setsockopt( udpClientSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof( mreq ) ) < 0 ) {
            throw std::runtime_error( "setsockopt IP_ADD_MEMBERSHIP failed" );
        }

        std::cout << "Listening for multicast on " << ip << ":" << port << "...\n";

        udpReadThread = std::thread( []()
            {
                sockaddr_in senderAddr = {};
                int senderLen = sizeof( senderAddr );
                while ( true ) {
                    char buffer[512] = {};
                    int bytesReceived = recvfrom( udpClientSocket, buffer, sizeof( buffer ) - 1, 0, (sockaddr *)&senderAddr, &senderLen );
                    if ( bytesReceived < 0 ) {
                        std::cerr << "recvfrom failed\n";
                        break;
                    }

                    if ( bytesReceived > 0 ) {
                        PacketBuffer packet = {};
                        packet.id = udpClientSocket;
                        memcpy_s( packet.data, sizeof( packet.data ), buffer, bytesReceived );
                        udpPacketQueue.Push( packet );
                    }
                }
            } );
    }

    void UDPClientRecvPacket( PacketBuffer & buffer ) {
        udpPacketQueue.Pop( buffer );
    }
}

namespace fin {
    class DesktopInterface : public DeviceInterface {
    public:
        void Send( const void * data, int size ) override {
            TCPClientSendPacket( data, size );
        }

        void Recv( PacketBuffer & buffer ) override {
            UDPClientRecvPacket( buffer ); 
        }

        void DisplayHeaderText( const char * text ) override {

        }

        void DisplayTrade( const char * text, int price, int quantity ) override {
            std::cout << text << ": " << price << " | " << quantity << std::endl;
        }

        void Log(const char* text) override {
            std::cout << text << std::endl;
        }
    };

    DeviceInterface * GetDeviceInterface() {
        static DesktopInterface device;
        return &device;
    }
}


