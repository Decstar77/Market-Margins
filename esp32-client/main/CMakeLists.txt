idf_component_register(
    NAME fin_client
    SRCS "fin-main.cpp" "fin-wifi.cpp" "fin-storage.cpp" "fin-logic.cpp" "fin-client.cpp" "fin-display.cpp" "../../shared/fin-client.cpp" "fin-esp32-device-interface.cpp" "fin-button.cpp"
    INCLUDE_DIRS "."
    )
