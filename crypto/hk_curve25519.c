#include "hk_curve25519.h"

#include "hk_crypto_logging.h"

#define WOLFSSL_USER_SETTINGS
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/curve25519.h>

hk_curve25519_key_t *hk_curve25519_init_key()
{
    hk_curve25519_key_t *key = malloc(sizeof(hk_curve25519_key_t));
    key->internal = malloc(sizeof(curve25519_key));
    return key;
}

size_t hk_curve25519_generate_key(hk_curve25519_key_t *key)
{
    WC_RNG rng;
    int ret = wc_InitRng(&rng);
    if (ret)
    {
        HK_CRYPOT_ERR("Failed to initialize random", ret);
        return ret;
    }

    ret = wc_curve25519_init((curve25519_key *)key->internal);
    if (ret)
    {
        HK_CRYPOT_ERR("Failed initializing key", ret);
        return ret;
    }

    ret = wc_curve25519_make_key(&rng, CURVE25519_KEYSIZE, (curve25519_key *)key->internal);
    if (ret)
    {
        HK_CRYPOT_ERR("Error making accessory curve key", ret);
        return ret;
    }

    wc_FreeRng(&rng);

    return ret;
}

size_t hk_curve25519_generate_key_from_public_key(hk_curve25519_key_t *key, hk_mem *public_key)
{
    int ret = wc_curve25519_import_public_ex((byte *)public_key->ptr, public_key->size, (curve25519_key *)key->internal, EC25519_LITTLE_ENDIAN);
    if (ret)
        HK_CRYPOT_ERR("Error importing device public curve key", ret);

    return ret;
}

size_t hk_curve25519_shared_secret(hk_curve25519_key_t *key1, hk_curve25519_key_t *key2, hk_mem *shared_secret)
{
    hk_mem_set(shared_secret, CURVE25519_KEYSIZE);
    int ret = wc_curve25519_shared_secret_ex(
        (curve25519_key *)key1->internal,
        (curve25519_key *)key2->internal,
        (byte *)shared_secret->ptr, &shared_secret->size, EC25519_LITTLE_ENDIAN);

    if (ret)
        HK_CRYPOT_ERR("Error creating curve shared secret", ret);

    return ret;
}

size_t hk_curve25519_export_public_key(hk_curve25519_key_t *key, hk_mem *public_key)
{
    hk_mem_set(public_key, CURVE25519_KEYSIZE);
    int ret = wc_curve25519_export_public_ex(
        (curve25519_key *)key->internal,
        (byte *)public_key->ptr, &public_key->size,
        EC25519_LITTLE_ENDIAN);

    if (ret)
    {
        HK_CRYPOT_ERR("Error exporting accessory public curve key", ret);
    }

    return ret;
}

void hk_curve25519_free_key(hk_curve25519_key_t *key)
{
    if (key->internal != NULL)
    {
        wc_curve25519_free((curve25519_key *)key->internal);
        free(key->internal);
        key->internal = NULL;
    }

    free(key);
}