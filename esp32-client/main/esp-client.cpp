#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "esp_log.h"
#include "esp-lib.h"

#include "../../shared/fin-api.h"

namespace fin {
    static const char * TAG = "CLIENT";
    static int tcp_socket = -1;
    static WaitQueue<PacketBuffer> tcpIncoming;

    void tcp_client_task( void * pvParameters ) {
        char addr_str[128];
        int addr_family;
        int ip_protocol;

        while ( 1 ) {
            struct sockaddr_in dest_addr;
            dest_addr.sin_addr.s_addr = inet_addr( SERVER_IP );
            dest_addr.sin_family = AF_INET;
            dest_addr.sin_port = htons( SERVER_TCP_PORT );
            addr_family = AF_INET;
            ip_protocol = IPPROTO_IP;
            inet_ntoa_r( dest_addr.sin_addr, addr_str, sizeof( addr_str ) - 1 );

            tcp_socket = socket( addr_family, SOCK_STREAM, ip_protocol );
            if ( tcp_socket < 0 ) {
                ESP_LOGE( TAG, "Unable to create socket: errno %d", errno );
                vTaskDelay( pdMS_TO_TICKS( 2000 ) );
                continue;
            }
            ESP_LOGI( TAG, "Socket created, connecting to %s:%d", SERVER_IP, SERVER_TCP_PORT );

            int err = connect( tcp_socket, (struct sockaddr *)&dest_addr, sizeof( dest_addr ) );
            if ( err != 0 ) {
                ESP_LOGE( TAG, "Socket unable to connect: errno %d", errno );
                close( tcp_socket );
                vTaskDelay( pdMS_TO_TICKS( 2000 ) );
                continue;
            }
            ESP_LOGI( TAG, "Successfully connected" );

            for ( ;; ) {
                PacketBuffer buffer = {};
                int len = recv( tcp_socket, static_cast<char *>(buffer.data), sizeof( buffer.data ), 0 );
                if ( len < 0 ) {
                    ESP_LOGE( TAG, "recv failed: errno %d", errno );
                    break;
                }
                else {
                    tcpIncoming.Push( buffer );
                }
            }

            close( tcp_socket );
            ESP_LOGI( TAG, "Socket closed" );
            vTaskDelay( pdMS_TO_TICKS( 5000 ) );
        }
    }

    static WaitQueue<PacketBuffer> udpIncoming;

    void udp_client_task( void * pvParameters ) {
        const char * multicast_ip = "239.255.0.1";
        const int multicast_port = 54001;

        while ( 1 ) {
            int sock = socket( AF_INET, SOCK_DGRAM, IPPROTO_IP );
            if ( sock < 0 ) {
                ESP_LOGE( TAG, "Unable to create socket: errno %d", errno );
                vTaskDelay( pdMS_TO_TICKS( 2000 ) );
                continue;
            }
            ESP_LOGI( TAG, "Socket created" );

            // Allow multiple sockets to use the same PORT number
            u8_t reuse = 1;
            setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof( reuse ) );

            // Bind to receive on the multicast port (from any address)
            struct sockaddr_in bind_addr = {};
            bind_addr.sin_family = AF_INET;
            bind_addr.sin_port = htons( multicast_port );
            bind_addr.sin_addr.s_addr = htonl( INADDR_ANY );

            if ( bind( sock, (struct sockaddr *)&bind_addr, sizeof( bind_addr ) ) < 0 ) {
                ESP_LOGE( TAG, "Socket unable to bind: errno %d", errno );
                close( sock );
                vTaskDelay( pdMS_TO_TICKS( 2000 ) );
                continue;
            }

            // Join multicast group
            struct ip_mreq mreq = {};
            mreq.imr_multiaddr.s_addr = inet_addr( multicast_ip );
            mreq.imr_interface.s_addr = htonl( INADDR_ANY );

            if ( setsockopt( sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof( mreq ) ) < 0 ) {
                ESP_LOGE( TAG, "Failed to join multicast group: errno %d", errno );
                close( sock );
                vTaskDelay( pdMS_TO_TICKS( 2000 ) );
                continue;
            }

            ESP_LOGI( TAG, "Joined multicast group %s:%d", multicast_ip, multicast_port );

            // Receive loop
            while ( 1 ) {
                struct sockaddr_in source_addr;
                socklen_t socklen = sizeof( source_addr );

                PacketBuffer buffer = {};
                int len = recvfrom( sock, buffer.data, sizeof( buffer.data ), 0,
                    (struct sockaddr *)&source_addr, &socklen );

                if ( len < 0 ) {
                    ESP_LOGE( TAG, "recvfrom failed: errno %d", errno );
                    break;
                }
                else {
                    udpIncoming.Push( buffer );
                }
            }

            close( sock );
            ESP_LOGI( TAG, "Socket closed" );
            vTaskDelay( pdMS_TO_TICKS( 5000 ) );
        }
    }

    void client_send( const void * data, int size ) {
        if ( tcp_socket < 0 ) {
            return;
        }

        send( tcp_socket, data, size, 0 );
    }

    void client_udp_recv( PacketBuffer & buffer ) {
        udpIncoming.Pop( buffer );
    }

    void client_init() {
        xTaskCreate( tcp_client_task, "TCP Client", 4096, NULL, 5, NULL );
        xTaskCreate( udp_client_task, "UDP Client", 4096, NULL, 5, NULL );
    }
}
