#include "hk_accessories_store.h"
#include "utils/hk_logging.h"
#include "utils/hk_ll.h"

hk_accessory_t *hk_accessories;

hk_accessory_t *hk_accessories_store_get_accessories()
{
    return hk_accessories;
}

void hk_accessories_store_add_accessory()
{
    hk_accessories = hk_ll_new(hk_accessories);
    hk_accessories->services = NULL;
}

void hk_accessories_store_add_service(hk_service_types_t type, bool primary, bool hidden)
{
    hk_service_t *service = hk_ll_new(hk_accessories->services);

    service->type = type;
    service->primary = primary;
    service->hidden = hidden;
    service->characteristics = NULL;

    hk_accessories->services = service;
}

void *hk_accessories_store_add_characteristic(hk_characteristic_types_t type, void *(*read)(), void (*write)(void *), bool can_notify)
{
    hk_characteristic_t *characteristic = hk_ll_new(hk_accessories->services->characteristics);

    characteristic->type = type;
    characteristic->static_value = NULL;
    characteristic->read = read;
    characteristic->write = write;
    characteristic->session = NULL;

    hk_accessories->services->characteristics = characteristic;

    return characteristic;
}

void hk_accessories_store_add_characteristic_static_read(hk_characteristic_types_t type, void *value)
{
    hk_characteristic_t *characteristic = hk_ll_new(hk_accessories->services->characteristics);

    characteristic->type = type;
    characteristic->static_value = value;
    characteristic->read = NULL;
    characteristic->write = NULL;
    characteristic->can_notify = false;

    hk_accessories->services->characteristics = characteristic;
}

void hk_accessories_store_end_config()
{
    if (hk_accessories)
    {
        size_t aid = 0;
        size_t iid = 0;
        hk_accessories = hk_ll_reverse(hk_accessories);
        hk_ll_foreach(hk_accessories, accessory)
        {
            iid = 0;
            accessory->aid = ++aid;
            if (accessory->services)
            {
                accessory->services = hk_ll_reverse(accessory->services);
                hk_ll_foreach(accessory->services, service)
                {
                    service->iid = ++iid;
                    if (service->characteristics)
                    {
                        service->characteristics = hk_ll_reverse(service->characteristics);
                        hk_ll_foreach(service->characteristics, characteristic)
                        {
                            characteristic->iid = ++iid;
                            characteristic->aid = accessory->aid;
                        }
                    }
                }
            }
        }
    }
}

hk_characteristic_t *hk_accessories_store_get_characteristic(size_t aid, size_t iid)
{
    if (hk_accessories)
    {
        hk_ll_foreach(hk_accessories, accessory)
        {
            if (aid == accessory->aid && accessory->services)
            {
                hk_ll_foreach(accessory->services, service)
                {
                    if (service->characteristics)
                    {
                        hk_ll_foreach(service->characteristics, characteristic)
                        {
                            if (iid == characteristic->iid)
                            {
                                return characteristic;
                            }
                        }
                    }
                }
            }
        }
    }

    return NULL;
}

hk_characteristic_t *hk_accessories_store_get_identify_characteristic()
{
    if (hk_accessories)
    {
        hk_ll_foreach(hk_accessories, accessory)
        {
            if (accessory->services)
            {
                hk_ll_foreach(accessory->services, service)
                {
                    if (service->characteristics)
                    {
                        hk_ll_foreach(service->characteristics, characteristic)
                        {
                            if (HK_CHR_IDENTIFY == characteristic->type)
                            {
                                return characteristic;
                            }
                        }
                    }
                }
            }
        }
    }

    return NULL;
}

hk_format_t hk_accessories_store_get_format(hk_characteristic_types_t characteristic_type)
{
    switch (characteristic_type)
    {
    case HK_CHR_ADMINISTRATOR_ONLY_ACCESS:
    case HK_CHR_AUDIO_FEEDBACK:
    case HK_CHR_IDENTIFY:
    case HK_CHR_MOTION_DETECTED:
    case HK_CHR_OBSTRUCTION_DETECTED:
    case HK_CHR_ON:
    case HK_CHR_OUTLET_IN_USE:
    case HK_CHR_HOLD_POSITION:
    case HK_CHR_STATUS_ACTIVE:
    case HK_CHR_MUTE:
    case HK_CHR_NIGHT_VISION:
        return HK_FORMAT_BOOL;
    case HK_CHR_CURRENT_DOOR_STATE:
    case HK_CHR_CURRENT_HEATING_COOLING_STATE:
    case HK_CHR_LOCK_CURRENT_STATE:
    case HK_CHR_LOCK_LAST_KNOWN_ACTION:
    case HK_CHR_LOCK_MANAGEMENT_AUTO_SECURITY_TIMEOUT:
    case HK_CHR_LOCK_TARGET_STATE:
    case HK_CHR_TARGET_DOORSTATE:
    case HK_CHR_TARGET_HEATING_COOLING_STATE:
    case HK_CHR_TEMPERATURE_DISPLAY_UNITS:
    case HK_CHR_AIR_PARTICULATE_SIZE:
    case HK_CHR_SECURITY_SYSTEM_CURRENT_STATE:
    case HK_CHR_SECURITY_SYSTEM_TARGET_STATE:
    case HK_CHR_BATTERY_LEVEL:
    case HK_CHR_CARBON_MONOXIDE_DETECTED:
    case HK_CHR_CONTACT_SENSOR_STATE:
    case HK_CHR_CURRENT_POSITION:
    case HK_CHR_LEAK_DETECTED:
    case HK_CHR_OCCUPANCY_DETECTED:
    case HK_CHR_POSITION_STATE:
    case HK_CHR_PROGRAMMABLE_SWITCH_EVENT:
    case HK_CHR_SMOKE_DETECTED:
    case HK_CHR_STATUS_FAULT:
    case HK_CHR_STATUS_JAMMED:
    case HK_CHR_STATUS_LOW_BATTERY:
    case HK_CHR_STATUS_TAMPERED:
    case HK_CHR_TARGET_POSITION:
    case HK_CHR_SECURITY_SYSTEM_ALARM_TYPE:
    case HK_CHR_CHARGING_STATE:
    case HK_CHR_CARBON_DIOXIDE_DETECTED:
    case HK_CHR_AIR_QUALITY:
    case HK_CHR_VOLUME:
    case HK_CHR_LOCK_PHYSICAL_CONTROLS:
    case HK_CHR_CURRENT_AIR_PURIFIER_STATE:
    case HK_CHR_CURRENT_SLAT_STATE:
    case HK_CHR_SLAT_TYPE:
    case HK_CHR_FILTER_CHANGE_INDICATION:
    case HK_CHR_RESET_FILTER_INDICATION:
    case HK_CHR_TARGET_AIR_PURIFIER_STATE:
    case HK_CHR_TARGET_FAN_STATE:
    case HK_CHR_CURRENT_FAN_STATE:
    case HK_CHR_ACTIVE:
    case HK_CHR_SWING_MODE:
    case HK_CHR_SERVICE_LABEL_INDEX:
    case HK_CHR_SERVICE_LABEL_NAMESPACE:
        return HK_FORMAT_UINT8;
    case HK_CHR_ACCESSORY_FLAGS:
    case HK_CHR_COLOR_TEMPERATURE:
        return HK_FORMAT_UINT32;
    // case HK_CHR:
    //     return HK_FORMAT_UINT64
    case HK_CHR_BRIGHTNESS:
    case HK_CHR_ROTATION_DIRECTION:
    case HK_CHR_CURRENT_HORIZONTAL_TILT_ANGLE:
    case HK_CHR_CURRENT_VERTICAL_TILT_ANGLE:
    case HK_CHR_TARGET_HORIZONTAL_TILT_ANGLE:
    case HK_CHR_TARGET_VERTICAL_TILT_ANGLE:
    case HK_CHR_CURRENT_TILT_ANGLE:
    case HK_CHR_TARGET_TILT_ANGLE:
        return HK_FORMAT_INT;
    case HK_CHR_COOLING_THRESHOLD_TEMPERATURE:
    case HK_CHR_CURRENT_RELATIVE_HUMIDITY:
    case HK_CHR_CURRENT_TEMPERATURE:
    case HK_CHR_HEATING_THRESHOLD_TEMPERATURE:
    case HK_CHR_HUE:
    case HK_CHR_ROTATION_SPEED:
    case HK_CHR_SATURATION:
    case HK_CHR_AIR_PARTICULATE_DENSITY:
    case HK_CHR_CURRENT_AMBIENT_LIGHT_LEVEL:
    case HK_CHR_CARBON_MONOXIDE_LEVEL:
    case HK_CHR_CARBON_MONOXIDE_PEAK_LEVEL:
    case HK_CHR_CARBON_DIOXIDE_LEVEL:
    case HK_CHR_CARBON_DIOXIDE_PEAK_LEVEL:
    case HK_CHR_OPTICAL_ZOOM:
    case HK_CHR_DIGITAL_ZOOM:
    case HK_CHR_IMAGE_ROTATION:
    case HK_CHR_IMAGE_MIRRORING:
    case HK_CHR_FILTER_LIFE_LEVEL:
    case HK_CHR_OZONE_DENSITY:
    case HK_CHR_NITROGEN_DIOXIDE_DENSITY:
    case HK_CHR_SULPHUR_DIOXIDE_DENSITY:
    case HK_CHR_PM25_DENSITY:
    case HK_CHR_PM10_DENSITY:
    case HK_CHR_VOC_DENSITY:
        return HK_FORMAT_FLOAT;
    case HK_CHR_FIRMWARE_REVISION:
    case HK_CHR_HARDWARE_REVISION:
    case HK_CHR_MANUFACTURER:
    case HK_CHR_MODEL:
    case HK_CHR_NAME:
    case HK_CHR_SERIAL_NUMBER:
    case HK_CHR_TARGET_RELATIVE_HUMIDITY:
    case HK_CHR_TARGET_TEMPERATURE:
    case HK_CHR_VERSION:
        return HK_FORMAT_STRING;
    case HK_CHR_LOCK_CONTROL_POINT:
    case HK_CHR_LOGS:
    case HK_CHR_STREAMING_STATUS:
    case HK_CHR_SUPPORTED_VIDEO_STREAMING_CONFIGURATION:
    case HK_CHR_SUPPORTED_AUDIO_STREAMING_CONFIGURATION:
    case HK_CHR_SUPPORTED_RTP_CONFIGURATION:
    case HK_CHR_SETUP_ENDPOINTS:
    case HK_CHR_SELECTED_RTP_STREAM_CONFIGURATION:
        return HK_FORMAT_TLV8;
    // case HK_CHR:
    //     return HK_FORMAT_DATA
    default:
        HK_LOGE("Could not find format for type %X", characteristic_type);
        return HK_FORMAT_UNKNOWN;
    }
}

void hk_accessories_free()
{
    // we do not free ressources, because this method is used in unit testing only
    hk_accessories = NULL;
}