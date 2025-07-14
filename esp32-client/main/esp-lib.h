#ifndef FIN_LIB_H
#define FIN_LIB_H

#define SERVER_IP   "192.168.2.31"
#define SERVER_TCP_PORT 54000
#define SERVER_UDP_PORT 54001
#define CAT_STRING(x, y, z) x y z

namespace fin {
    class PacketBuffer;
    class DeviceInterface;

    void                wifi_init();
    bool                wifi_connected();
    const char *        wifi_ip();

    void                ethernet_init();
    bool                eth_connected();
    const char *        eth_ip();

    void                storage_init();
    void                storage_save_int( const char * key, int32_t value );
    int32_t             storage_load_int( const char * key, int32_t defaultValue );

    void                logic_init();
    void                button_init();
    void                swap_strategy();

    void                client_init();
    void                client_send( const void * data, int size );

    void                client_udp_recv( PacketBuffer & buffer );

    void                display_init();
    void                display_set_header_text( const char * text );
    void                display_append_trade( const char * text, int price, int quantity );

    DeviceInterface *   device_interface();
}

#endif

