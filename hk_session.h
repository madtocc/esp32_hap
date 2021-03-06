#pragma once

#include "utils/hk_mem.h"

#include <esp_err.h>
#include <stdbool.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#define HK_SESSION_RESPONSE_MESSAGE 0
#define HK_SESSION_RESPONSE_EVENT 1

#define HK_SESSION_CONTENT_TLV 0
#define HK_SESSION_CONTENT_JSON 1

struct hk_session;

typedef enum
{
    HK_SESSION_HTML_METHOD_NOT_SET = -1,
    HK_SESSION_HTML_METHOD_GET = 1,
    HK_SESSION_HTML_METHOD_POST = 3,
    HK_SESSION_HTML_METHOD_PUT = 4,
} hk_session_html_methods_t;

typedef struct
{
    hk_mem *content;
    hk_mem *query;
    hk_mem *key;
    hk_mem *url;
    hk_session_html_methods_t method;
} hk_session_request_t;

typedef struct
{
    QueueHandle_t data_to_send;
    hk_mem *key;
    hk_mem *data;
    hk_mem *content;
    size_t result;
    size_t type;
    size_t content_type;
} hk_session_response_t;

typedef struct hk_session
{
    hk_session_request_t *request;
    hk_session_response_t *response;
    char *device_id;
    bool is_encrypted; // states that verification was done
    int socket;
    bool should_close;
    bool kill; // states that all sessions with the same device id have to be killed
    int received_frame_count;
    int sent_frame_count;

    //the following variables are used during verification process, they are allocated and freed during it
    hk_mem *session_key;
    hk_mem *accessory_curve_public_key;
    hk_mem *device_curve_public_key;
} hk_session_t;

void hk_session_init(hk_session_t *session, int socket);
void hk_session_clean(hk_session_t *session);
void hk_session_clean_response(hk_session_t *session);
void hk_session_dispose(hk_session_t *session_context);

void hk_session_set_device_id(hk_session_t *session, hk_mem *device_id);
void hk_session_send(hk_session_t *session);