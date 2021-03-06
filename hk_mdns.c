#include "hk_mdns.h"
#include "utils/hk_logging.h"
#include "utils/hk_store.h"
#include "utils/hk_util.h"

#include <mdns.h>

void hk_mdns_add_txt(const char *key, const char *format, ...)
{
    va_list arg_ptr;
    va_start(arg_ptr, format);

    char value[128];
    int value_len = vsnprintf(value, sizeof(value), format, arg_ptr);

    va_end(arg_ptr);

    if (value_len && value_len < sizeof(value) - 1)
    {
        HK_LOGD("Adding service text: %s (%s)", key, value);
        ESP_ERROR_CHECK(mdns_service_txt_item_set("_hap", "_tcp", key, value));
    }
    else
    {
        HK_LOGE("Could not add service text: %s (%s)", key, value);
    }
}

void hk_mdns_init(const char *name, hk_categories_t category, size_t config_version)
{
    //Free in case of reconnecting to wifi
    mdns_free(); 

    //initialize mDNS
    ESP_ERROR_CHECK(mdns_init());
    //set mDNS hostname (required if you want to advertise services)
    ESP_ERROR_CHECK(mdns_hostname_set(name));
    //set mDNS instance name
    ESP_ERROR_CHECK(mdns_instance_name_set(name));
    ESP_ERROR_CHECK(mdns_service_add(name, "_hap", "_tcp", 5556, NULL, 0)); // accessory model name (required)

    hk_mdns_add_txt("md", "%s", name);
    // protocol version (required)
    hk_mdns_add_txt("pv", "1.0");
    // device ID (required)
    // should be in format XX:XX:XX:XX:XX:XX, otherwise devices will ignore it
    hk_mem *id = hk_mem_create();
    hk_util_get_accessory_id(id);
    char* id_str = hk_mem_get_str(id);
    hk_mdns_add_txt("id", "%s", id_str);
    hk_mem_free(id);
    free(id_str);
    // current configuration number (required)
    hk_mdns_add_txt("c#", "%d", config_version);
    // current state number (required)
    hk_mdns_add_txt("s#", "1");
    // feature flags (required if non-zero)
    //   bit 0 - supports HAP pairing. required for all HomeKit accessories
    //   bits 1-7 - reserved
    // above is what the specification says
    // but we have to set this to zero for uncertified accessories (mfi)
    hk_mdns_add_txt("ff", "0");
    // accessory category identifier
    hk_mdns_add_txt("ci", "%d", category);
    hk_mdns_update_paired(true);
}

void hk_mdns_update_paired(bool initial)
{
    // if item is not paired, we need this flag. Otherwise not.
    if (!initial)
    {
        ESP_ERROR_CHECK(mdns_service_txt_item_remove("_hap", "_tcp", "sf"));
    }

    bool paired = hk_store_is_paired_get();
    if (!paired)
    {
        // status flags
        //   bit 0 - not paired
        //   bit 1 - not configured to join WiFi
        //   bit 2 - problem detected on accessory
        //   bits 3-7 - reserved
        hk_mdns_add_txt("sf", "1");
    }else{
        HK_LOGI("Not advertising for pairing, because we are coupled already.");
    }
}