#include "hk_pair_verify.h"

#include "utils/hk_logging.h"
#include "utils/hk_tlv.h"
#include "utils/hk_util.h"
#include "utils/hk_store.h"
#include "utils/hk_res.h"
#include "crypto/hk_curve25519.h"
#include "crypto/hk_hkdf.h"
#include "crypto/hk_chacha20poly1305.h"
#include "crypto/hk_ed25519.h"
#include "hk_session.h"
#include "hk_html.h"
#include "hk_pairings_store.h"

esp_err_t hk_create_session_security(hk_session_t *session, hk_mem *shared_secret)
{
    esp_err_t ret = ESP_OK;

    ret = hk_hkdf(HK_HKDF_CONTROL_READ, shared_secret, session->response->key);

    if (!ret)
        ret = hk_hkdf(HK_HKDF_CONTROL_WRITE, shared_secret, session->request->key);

    return ret;
}

esp_err_t hk_pair_verify_start(hk_session_t *session, hk_tlv_t *tlv)
{
    hk_mem *retult = session->response->content;
    esp_err_t ret = ESP_OK;
    hk_ed25519_key_t *accessory_key = hk_ed25519_init_key();
    hk_curve25519_key_t *accessory_curve = hk_curve25519_init_key();
    hk_curve25519_key_t *device_curve = hk_curve25519_init_key();
    session->session_key = hk_mem_create();
    session->accessory_curve_public_key = hk_mem_create();
    session->device_curve_public_key = hk_mem_create();
    hk_mem *shared_secret = hk_mem_create();
    hk_mem *accessory_info = hk_mem_create();
    hk_mem *accessory_id = hk_mem_create();
    hk_mem *accessory_signature = hk_mem_create();
    hk_mem *accessory_public_key = hk_mem_create();
    hk_mem *accessory_private_key = hk_mem_create();
    hk_mem *sub_retult = hk_mem_create();
    hk_mem *encrypted = hk_mem_create();
    hk_tlv_t *sub_tlv_data = NULL;
    hk_tlv_t *tlv_data = NULL;

    if (hk_store_keys_can_get())
    {
        hk_store_key_pub_get(accessory_public_key);
        hk_store_key_priv_get(accessory_private_key);
    }
    else
    {
        HK_LOGE("Cannot get key from store for pairing.");
        ret = HK_RES_UNKNOWN;
    }

    if (accessory_public_key->size < 1 || accessory_private_key->size < 1)
    {
        HK_LOGE("Verfiy was called, but accessory keys are not present.");
        ret = HK_RES_UNKNOWN;
    }

    if (!ret)
        ret = hk_tlv_get_mem_by_type(tlv, HK_TLV_PublicKey, session->device_curve_public_key);

    if (!ret)
        ret = hk_curve25519_generate_key_from_public_key(device_curve, session->device_curve_public_key);

    if (!ret)
        ret = hk_curve25519_generate_key(accessory_curve);

    if (!ret)
        ret = hk_curve25519_shared_secret(accessory_curve, device_curve, shared_secret);

    if (!ret)
        ret = hk_curve25519_export_public_key(accessory_curve, session->accessory_curve_public_key);

    if (!ret)
    {
        ret = hk_ed25519_generate_key_from_keys(accessory_key, accessory_private_key, accessory_public_key);
    }

    if (!ret)
    {
        hk_util_get_accessory_id(accessory_id);
        hk_mem_append(accessory_info, session->accessory_curve_public_key);
        hk_mem_append(accessory_info, accessory_id);
        hk_mem_append(accessory_info, session->device_curve_public_key);

        ret = hk_ed25519_sign(accessory_key, accessory_info, accessory_signature);
    }

    if (!ret)
    {
        sub_tlv_data = hk_tlv_add(sub_tlv_data, HK_TLV_Identifier, accessory_id);
        sub_tlv_data = hk_tlv_add(sub_tlv_data, HK_TLV_Signature, accessory_signature);

        hk_tlv_serialize(sub_tlv_data, sub_retult);
    }

    if (!ret)
        ret = hk_hkdf(HK_HKDF_PAIR_VERIFY_ENCRYPT, shared_secret, session->session_key);

    if (!ret)
        ret = hk_chacha20poly1305_encrypt(session->session_key, HK_CHACHA_VERIFY_MSG2, sub_retult, encrypted);

    if (!ret)
    {
        ret = hk_create_session_security(session, shared_secret);
    }

    tlv_data = hk_tlv_add_state(tlv_data, 2);
    if (!ret)
    {
        tlv_data = hk_tlv_add(tlv_data, HK_TLV_PublicKey, session->accessory_curve_public_key); // there is an error in the specification, dont use the srp proof
        tlv_data = hk_tlv_add(tlv_data, HK_TLV_EncryptedData, encrypted);
    }
    else
    {
        tlv_data = hk_tlv_add_error(tlv_data, HK_TLV_ERROR_Authentication);
    }

    hk_tlv_serialize(tlv_data, retult);

    hk_tlv_free(tlv_data);
    hk_tlv_free(sub_tlv_data);
    hk_mem_free(shared_secret);
    hk_mem_free(accessory_info);
    hk_mem_free(accessory_id);
    hk_mem_free(accessory_signature);
    hk_mem_free(accessory_public_key);
    hk_mem_free(accessory_private_key);
    hk_mem_free(sub_retult);
    hk_mem_free(encrypted);
    hk_ed25519_free_key(accessory_key);
    hk_curve25519_free_key(accessory_curve);
    hk_curve25519_free_key(device_curve);

    return ret;
}

esp_err_t hk_pair_verify_finish(hk_session_t *session, hk_tlv_t *tlv)
{
    hk_ed25519_key_t *device_key = hk_ed25519_init_key();
    hk_mem *encrypted_data = hk_mem_create();
    hk_mem *decrypted_data = hk_mem_create();
    hk_mem *device_id = hk_mem_create();
    hk_mem *device_signature = hk_mem_create();
    hk_mem *device_public_key = hk_mem_create();
    hk_mem *device_info = hk_mem_create();
    hk_tlv_t *tlv_data_decrypted = NULL;
    hk_tlv_t *tlv_data = NULL;

    esp_err_t ret = hk_tlv_get_mem_by_type(tlv, HK_TLV_EncryptedData, encrypted_data);

    if (!ret)
    {
        HK_LOGV("Decrypting pairing request.");
        hk_chacha20poly1305_decrypt(session->session_key, HK_CHACHA_VERIFY_MSG3, encrypted_data, decrypted_data);
        tlv_data_decrypted = hk_tlv_deserialize(decrypted_data);
        ret = hk_tlv_get_mem_by_type(tlv_data_decrypted, HK_TLV_Identifier, device_id);
    }

    if (!ret)
    {
        HK_LOGV("Getting session id.");
        hk_session_set_device_id(session, device_id);
        ret = hk_tlv_get_mem_by_type(tlv_data_decrypted, HK_TLV_Signature, device_signature);
    }

    if (!ret)
    {
        HK_LOGV("Getting long term public key.");
        ret = hk_pairings_store_ltpk_get(device_id, device_public_key);
    }

    if (!ret)
    {
        HK_LOGV("Generating key.");
        ret = hk_ed25519_generate_key_from_public_key(device_key, device_public_key);
    }

    if (!ret)
    {
        HK_LOGV("Verifying key.");
        hk_mem_append(device_info, session->device_curve_public_key);
        hk_mem_append(device_info, device_id);
        hk_mem_append(device_info, session->accessory_curve_public_key);
        ret = hk_ed25519_verify(device_key, device_info, device_signature);
    }

    tlv_data = hk_tlv_add_state(tlv_data, 4);
    if (ret)
    {
        tlv_data = hk_tlv_add_error(tlv_data, HK_TLV_ERROR_Authentication);
    }

    HK_LOGV("Serializing answer.");
    hk_tlv_serialize(tlv_data, session->response->content);

    HK_LOGV("Done.");
    hk_ed25519_free_key(device_key);
    hk_tlv_free(tlv_data);
    hk_tlv_free(tlv_data_decrypted);
    hk_mem_free(device_info);
    hk_mem_free(encrypted_data);
    hk_mem_free(decrypted_data);
    hk_mem_free(device_id);
    hk_mem_free(device_signature);
    hk_mem_free(device_public_key);

    hk_mem_free(session->session_key);
    hk_mem_free(session->accessory_curve_public_key);
    hk_mem_free(session->device_curve_public_key);

    return ret;
}

void hk_pair_verify(hk_session_t *session)
{
    hk_tlv_t *tlv_data = hk_tlv_deserialize(session->request->content);
    hk_tlv_t *type_tlv = hk_tlv_get_tlv_by_type(tlv_data, HK_TLV_State);

    if (type_tlv == NULL)
    {
        HK_LOGE("Could not find tlv with type state.");
        session->response->result = HK_RES_MALFORMED_REQUEST;
    }
    else
    {
        switch (*type_tlv->value)
        {
        case 1:
            session->response->result = hk_pair_verify_start(session, tlv_data);
            break;
        case 3:
            session->response->result = hk_pair_verify_finish(session, tlv_data);
            break;
        default:
            HK_LOGE("Unexpected value in tlv in pair setup: %d", *type_tlv->value);
            session->response->result = HK_RES_MALFORMED_REQUEST;
        }
    }

    hk_session_send(session);

    if (*type_tlv->value == 3 && session->response->result == HK_RES_OK)
    {
        session->is_encrypted = true; // needs to be done after sending the response
        HK_LOGD("%d - Pairing verified, now communicating encrypted.", session->socket);
    }

    hk_tlv_free(tlv_data);
}