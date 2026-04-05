#include <pebble.h>

#include "modules/stream_data.h"
#include "windows/main_window.h"

// ---- AppMessage ----

static void inbox_received_callback(DictionaryIterator *iterator, void *context)
{
    Tuple *count_t = dict_find(iterator, MESSAGE_KEY_STREAM_COUNT);
    if (count_t)
    {
        stream_data_set_total((int)count_t->value->int32);
        stream_data_set_received(0);
        main_window_reload_data();
    }

    Tuple *index_t = dict_find(iterator, MESSAGE_KEY_STREAM_INDEX);
    if (index_t)
    {
        int idx = (int)index_t->value->int32;
        if (idx >= 0 && idx < MAX_STREAMS)
        {
            Tuple *username_t = dict_find(iterator, MESSAGE_KEY_STREAM_USERNAME);
            Tuple *category_t = dict_find(iterator, MESSAGE_KEY_STREAM_CATEGORY);
            Tuple *viewers_t = dict_find(iterator, MESSAGE_KEY_STREAM_VIEWER_COUNT);
            Tuple *title_t = dict_find(iterator, MESSAGE_KEY_STREAM_TITLE);

            StreamInfo *stream = stream_data_get(idx);
            if (username_t)
                strncpy(stream->username, username_t->value->cstring, MAX_USERNAME_LEN - 1);
            if (category_t)
                strncpy(stream->category, category_t->value->cstring, MAX_CATEGORY_LEN - 1);
            if (viewers_t)
                stream->viewer_count = viewers_t->value->int32;
            if (title_t)
                strncpy(stream->title, title_t->value->cstring, MAX_TITLE_LEN - 1);

            stream_data_set_received(stream_data_get_received() + 1);
            main_window_reload_data();
        }
    }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context)
{
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped! Reason: %d", (int)reason);
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context)
{
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed! Reason: %d", (int)reason);
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context)
{
    APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

// ---- App lifecycle ----

static void init(void)
{
    main_window_push();

    app_message_register_inbox_received(inbox_received_callback);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_register_outbox_failed(outbox_failed_callback);
    app_message_register_outbox_sent(outbox_sent_callback);

    app_message_open(512, 32);
}

static void deinit(void)
{
    main_window_destroy();
}

int main(void)
{
    init();
    app_event_loop();
    deinit();
}
