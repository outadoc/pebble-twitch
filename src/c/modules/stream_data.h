#pragma once

#include <pebble.h>

#define MAX_STREAMS 20
#define MAX_USERNAME_LEN 32
#define MAX_CATEGORY_LEN 32
#define MAX_TITLE_LEN 256

#define STATE_WAITING -1
#define STATE_NOT_CONFIGURED -2
#define STATE_NETWORK_ERROR -3

typedef struct
{
    char username[MAX_USERNAME_LEN];
    char category[MAX_CATEGORY_LEN];
    int32_t viewer_count;
    char title[MAX_TITLE_LEN];
} StreamInfo;

StreamInfo *stream_data_get(int index);
int stream_data_get_total(void);
void stream_data_set_total(int total);
int stream_data_get_received(void);
void stream_data_set_received(int received);
void format_viewer_count(char *dest, size_t dest_size, int32_t viewer_count);
