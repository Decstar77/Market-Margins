#include "desk-platform.h"
#include "fin-spsc-queue.h"

#include <winsock2.h>
#include <ws2tcpip.h>

namespace fin {
    static SOCKET tcpServerSocket = INVALID_SOCKET;
    static SOCKET tcpClientSocket = INVALID_SOCKET;
    static std::thread tcpConnectionThread;
    static std::thread tcpReadThread;
    static std::vector<NetId> tcpClients;

    // NOTE: Small amount for now.
    static SPSCQueue<PacketBuffer> packetQueue( 1024 );

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

        if ( connect( tcpClientSocket, (sockaddr *)&serverAddr, sizeof( serverAddr ) ) == SOCKET_ERROR ) {
            throw std::runtime_error( "connect failed" );
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
                        packetQueue.Push( packet );
                    }
                }
            } );
    }

    void TCPClientSendPacket( const void * data, int size ) {
        send( tcpClientSocket, (const char *)data, size, 0 );
    }

    bool TCPClientRecievePacket( PacketBuffer & buffer ) {
        PacketBuffer packet = {};
        if ( packetQueue.Pop( packet ) ) {
            buffer = packet;
            return true;
        }
        return false;
    }

    class DesktopInterface : public DeviceInterface {
    public:
        virtual void Send( const void * data, int size ) override {
            TCPClientSendPacket( data, size );
        }

        virtual void Recv( PacketBuffer & buffer ) override {
            TCPClientRecievePacket( buffer );
        }

        virtual void DisplayHeaderText( const char * text ) override {

        }

        virtual void DisplayTrade( const char * text, int price, int quantity ) override {

        }
    };

    DeviceInterface * GetDeviceInterface() {
        static DesktopInterface device;
        return &device;
    }
}


