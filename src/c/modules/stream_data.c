#include "stream_data.h"

static StreamInfo s_streams[MAX_STREAMS];
static int s_streams_total = STATE_WAITING;
static int s_streams_received = 0;

StreamInfo *stream_data_get(int index)
{
    return &s_streams[index];
}

int stream_data_get_total(void)
{
    return s_streams_total;
}

void stream_data_set_total(int total)
{
    s_streams_total = total > MAX_STREAMS ? MAX_STREAMS : total;
}

int stream_data_get_received(void)
{
    return s_streams_received;
}

void stream_data_set_received(int received)
{
    s_streams_received = received;
}

void format_viewer_count(char *dest, size_t dest_size, int32_t viewer_count)
{
    if (viewer_count >= 1000000)
    {
        int32_t int_part = viewer_count / 1000000;
        int32_t frac_digit = (viewer_count % 1000000) / 100000;
        snprintf(dest, dest_size, "%ld.%ldM", int_part, frac_digit);
    }
    else if (viewer_count >= 1000)
    {
        int32_t int_part = viewer_count / 1000;
        int32_t frac_digit = (viewer_count % 1000) / 100;
        snprintf(dest, dest_size, "%ld.%ldK", int_part, frac_digit);
    }
    else
    {
        snprintf(dest, dest_size, "%ld", viewer_count);
    }
}
