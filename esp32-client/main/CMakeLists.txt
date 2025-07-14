idf_component_register(
    NAME fin_client
    SRCS "esp-main.cpp" "esp-wifi.cpp" "esp-storage.cpp" "esp-logic.cpp" "esp-client.cpp" "esp-display.cpp" "../../shared/fin-client.cpp" "esp-device-interface.cpp" "esp-button.cpp" "esp-eth.cpp"
    INCLUDE_DIRS "."
    )
