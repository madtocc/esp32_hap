#include "hk_res.h"
#include "hk_tlv.h"
#include "hk_logging.h"
#include "hk_ll.h"

#include <string.h>
#include <stdio.h>

hk_tlv_t *hk_tlv_add_state(hk_tlv_t *tlv_list, enum hk_tlv_types type)
{
    hk_mem *t = hk_mem_create();
    hk_mem_append_buffer(t, (char *)&type, 1);
    tlv_list = hk_tlv_add(tlv_list, HK_TLV_State, t);
    hk_mem_free(t);

    return tlv_list;
}

hk_tlv_t *hk_tlv_add_error(hk_tlv_t *tlv_list, enum hk_tlv_errors error)
{
    hk_mem *e = hk_mem_create();
    hk_mem_append_buffer(e, (char *)&error, 1);
    tlv_list = hk_tlv_add(tlv_list, HK_TLV_Error, e);
    hk_mem_free(e);

    return tlv_list;
}

hk_tlv_t *hk_tlv_add(hk_tlv_t *tlv_list, char type, hk_mem *data)
{
    size_t size = data->size;
    size_t size_added = 0;
    do
    {
        size_t size_to_add = size > 255 ? 255 : size;
        size -= size_to_add;

        tlv_list = hk_ll_new(tlv_list);
        tlv_list->type = type;
        tlv_list->length = size_to_add;
        tlv_list->value = malloc(size_to_add);
        memcpy(tlv_list->value, data->ptr + size_added, size_to_add);

        size_added += size_to_add;
    } while (size > 0);

    return tlv_list;
}

esp_err_t hk_tlv_get_mem_by_type(hk_tlv_t *tlv, char type, hk_mem *result)
{
    esp_err_t res = HK_RES_MALFORMED_REQUEST;
    for (; tlv; tlv = hk_ll_next(tlv))
    {
        if (tlv->type == type)
        {
            hk_mem_append_buffer(result, tlv->value, tlv->length);
            res = HK_RES_OK;
        }
    }

    if (res != HK_RES_OK)
    {
        HK_LOGE("Error getting tlv for type %d.", type);
    }

    return res;
}

hk_tlv_t *hk_tlv_get_tlv_by_type(hk_tlv_t *tlv, char type)
{
    while (tlv)
    {
        if (tlv->type == type)
        {
            return tlv;
        }

        tlv = hk_ll_next(tlv);
    }

    return NULL;
}

void hk_tlv_serialize(hk_tlv_t *tlv_list, hk_mem *result)
{
    tlv_list = hk_ll_reverse(tlv_list);
    hk_ll_foreach(tlv_list, tlv)
    {
        hk_mem_append_buffer(result, &tlv->type, 1);
        hk_mem_append_buffer(result, &tlv->length, 1);
        hk_mem_append_buffer(result, tlv->value, tlv->length);
    }
    tlv_list = hk_ll_reverse(tlv_list);
}

hk_tlv_t *hk_tlv_deserialize(hk_mem *data)
{
    hk_tlv_t *tlv_list = NULL;

    for (size_t i = 0; i < data->size;)
    {
        char type = data->ptr[i++];
        char length = data->ptr[i++];

        tlv_list = hk_ll_new(tlv_list);
        tlv_list->type = type;
        tlv_list->length = length;
        tlv_list->value = malloc(tlv_list->length);
        memcpy(tlv_list->value, data->ptr + i, length);

        i += length;
    }

    tlv_list = hk_ll_reverse(tlv_list);

    return tlv_list;
}

void hk_tlv_free(hk_tlv_t *tlv_list)
{
    hk_ll_foreach(tlv_list, tlv)
    {
        free(tlv->value);
        tlv->value = NULL;
    }

    hk_ll_free(tlv_list);
}