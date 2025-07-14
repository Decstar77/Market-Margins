#include <iostream>

#include "desk-platform.h"

int main() {
    fin::TCPClientSocket( "192.168.2.31", "54000" );
    fin::UDPClientSocket( "239.255.0.1", "54001" );

    fin::RandomClient randomClient( fin::GetDeviceInterface() );

    while ( true ) {
        std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
        randomClient.Think();
    }

    return 0;
}


