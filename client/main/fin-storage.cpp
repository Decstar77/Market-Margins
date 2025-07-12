#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "esp_partition.h"

#include "fin-lib.h"

namespace fin {
    static const char * TAG = "STORAGE";

    void storage_init() {
        esp_err_t ret = nvs_flash_init();
        if ( ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND ) {
            ESP_ERROR_CHECK( nvs_flash_erase() );
            ret = nvs_flash_init();
        }

        const esp_partition_t * nvs_partition = esp_partition_find_first(
            ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, "nvs" );
        if ( nvs_partition ) {
            ESP_LOGI( TAG, "NVS partition size: %lu bytes", nvs_partition->size );
            ESP_LOGI( TAG, "NVS covers %lu sectors (4KB each)", nvs_partition->size / 4096 );
        }

        nvs_stats_t nvs_stats;
        esp_err_t err = nvs_get_stats( NULL, &nvs_stats );
        if ( err == ESP_OK ) {
            ESP_LOGI( TAG, "NVS Entries: Used = %u, Free = %u, Total = %u", nvs_stats.used_entries, nvs_stats.free_entries, nvs_stats.total_entries );
        }

        ESP_ERROR_CHECK( ret );
    }

    void storage_save_int( const char * key, int32_t value ) {
        nvs_handle_t my_handle;
        esp_err_t err = nvs_open( "fin", NVS_READWRITE, &my_handle );
        if ( err != ESP_OK ) {
            ESP_LOGE( TAG, "Failed to open nvs handle: %s", esp_err_to_name( err ) );
            return;
        }
        err = nvs_set_i32( my_handle, key, value );
        if ( err != ESP_OK ) {
            ESP_LOGE( TAG, "Failed to set int value: %s", esp_err_to_name( err ) );
        }
        err = nvs_commit( my_handle );
        if ( err != ESP_OK ) {
            ESP_LOGE( TAG, "Failed to commit nvs: %s", esp_err_to_name( err ) );
        }
        nvs_close( my_handle );
    }

    int32_t storage_load_int( const char * key, int32_t defaultValue ) {
        nvs_handle_t my_handle;
        esp_err_t err = nvs_open( "fin", NVS_READONLY, &my_handle );
        if ( err != ESP_OK ) {
            ESP_LOGE( TAG, "Failed to open nvs handle: %s", esp_err_to_name( err ) );
            return defaultValue;
        }
        int32_t value = defaultValue;
        nvs_get_i32( my_handle, key, &value );
        nvs_close( my_handle );
        return value;
    }
}