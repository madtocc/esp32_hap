#pragma once

#include "utils/hk_mem.h"

#include "hk_session.h"

#include <esp_err.h>

esp_err_t hk_html_parser_parse(hk_mem* message, hk_session_t *data);
esp_err_t hk_html_parser_get_query_value(hk_mem *query, const char *key, hk_mem *value);