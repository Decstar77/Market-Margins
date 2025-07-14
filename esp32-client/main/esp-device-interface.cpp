#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "esp_log.h"
#include "esp-lib.h"

#include "../../shared/fin-client.h"

namespace fin {
    class Esp32DeviceInterface : public DeviceInterface {
    public:
        virtual void Send( const void * data, int size ) override {
            client_send( data, size );
        }
        virtual void Recv( PacketBuffer & buffer ) override {
            client_udp_recv( buffer );
        }
        virtual void DisplayHeaderText( const char * text ) override {
            display_set_header_text( text );
        }
        virtual void DisplayTrade( const char * text, int price, int quantity ) override {
            display_append_trade( text, price, quantity );
        }
    };

    DeviceInterface * device_interface() {
        static Esp32DeviceInterface device;
        return &device;
    }
}