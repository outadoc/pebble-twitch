#include <pebble.h>

#define MAX_STREAMS 20
#define MAX_USERNAME_LEN 32
#define MAX_CATEGORY_LEN 32
#define MAX_TITLE_LEN 128

typedef struct {
  char username[MAX_USERNAME_LEN];
  char category[MAX_CATEGORY_LEN];
  int viewer_count;
  char title[MAX_TITLE_LEN];
} StreamInfo;

typedef enum {
  STATE_LOADING,
  STATE_NO_STREAMS,
  STATE_LOADED
} AppState;

static StreamInfo s_streams[MAX_STREAMS];
static int s_stream_count = 0;
static int s_streams_received = 0;
static AppState s_state = STATE_LOADING;
static int s_selected_index = 0;

// Main window
static Window *s_main_window;
static MenuLayer *s_menu_layer;

// Detail window
static Window *s_detail_window;
static TextLayer *s_username_layer;
static TextLayer *s_viewers_layer;
static TextLayer *s_category_layer;
static TextLayer *s_title_layer;

// Persistent buffers for detail window text
static char s_viewers_buf[24];

static void format_viewer_count(char *buf, size_t len, int count) {
  snprintf(buf, len, "%d", count);
}

// ---- Detail window ----

static void detail_window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);
  StreamInfo *stream = &s_streams[s_selected_index];

  s_username_layer = text_layer_create(GRect(5, 5, bounds.size.w - 10, 36));
  text_layer_set_font(s_username_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_overflow_mode(s_username_layer, GTextOverflowModeTrailingEllipsis);
  text_layer_set_text(s_username_layer, stream->username);

  format_viewer_count(s_viewers_buf, sizeof(s_viewers_buf), stream->viewer_count);
  s_viewers_layer = text_layer_create(GRect(5, 44, bounds.size.w - 10, 22));
  text_layer_set_font(s_viewers_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text(s_viewers_layer, s_viewers_buf);

  s_category_layer = text_layer_create(GRect(5, 66, bounds.size.w - 10, 22));
  text_layer_set_font(s_category_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_overflow_mode(s_category_layer, GTextOverflowModeTrailingEllipsis);
  text_layer_set_text(s_category_layer, stream->category);

  s_title_layer = text_layer_create(GRect(5, 92, bounds.size.w - 10, bounds.size.h - 97));
  text_layer_set_font(s_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_overflow_mode(s_title_layer, GTextOverflowModeWordWrap);
  text_layer_set_text(s_title_layer, stream->title);

  layer_add_child(root, text_layer_get_layer(s_username_layer));
  layer_add_child(root, text_layer_get_layer(s_viewers_layer));
  layer_add_child(root, text_layer_get_layer(s_category_layer));
  layer_add_child(root, text_layer_get_layer(s_title_layer));
}

static void detail_window_unload(Window *window) {
  text_layer_destroy(s_username_layer);
  text_layer_destroy(s_viewers_layer);
  text_layer_destroy(s_category_layer);
  text_layer_destroy(s_title_layer);
  window_destroy(s_detail_window);
  s_detail_window = NULL;
}

static void show_detail_window(int index) {
  s_selected_index = index;
  s_detail_window = window_create();
  window_set_window_handlers(s_detail_window, (WindowHandlers) {
    .load = detail_window_load,
    .unload = detail_window_unload
  });
  window_stack_push(s_detail_window, true);
}

// ---- MenuLayer callbacks ----

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  if (s_state == STATE_LOADED) {
    return s_stream_count;
  }
  return 1; // One row for loading/empty state
}

static int16_t menu_get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  return 44;
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  switch (s_state) {
    case STATE_LOADING:
      menu_cell_basic_draw(ctx, cell_layer, "Loading...", NULL, NULL);
      break;
    case STATE_NO_STREAMS:
      menu_cell_basic_draw(ctx, cell_layer, "No live streams", "Configure settings", NULL);
      break;
    case STATE_LOADED: {
      StreamInfo *stream = &s_streams[cell_index->row];
      static char subtitle_buf[48];
      static char viewers_short[16];
      format_viewer_count(viewers_short, sizeof(viewers_short), stream->viewer_count);
      snprintf(subtitle_buf, sizeof(subtitle_buf), "%s \xc2\xb7 %s", viewers_short, stream->category);
      menu_cell_basic_draw(ctx, cell_layer, stream->username, subtitle_buf, NULL);
      break;
    }
  }
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  if (s_state != STATE_LOADED) return;
  show_detail_window(cell_index->row);
}

// ---- AppMessage ----

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  Tuple *count_t = dict_find(iterator, MESSAGE_KEY_STREAM_COUNT);
  if (count_t) {
    int count = (int)count_t->value->int32;
    if (count == 0) {
      s_state = STATE_NO_STREAMS;
      s_stream_count = 0;
    } else {
      s_stream_count = count > MAX_STREAMS ? MAX_STREAMS : count;
      s_streams_received = 0;
      s_state = STATE_LOADING;
    }
    menu_layer_reload_data(s_menu_layer);
  }

  Tuple *index_t = dict_find(iterator, MESSAGE_KEY_STREAM_INDEX);
  if (index_t) {
    int idx = (int)index_t->value->int32;
    if (idx >= 0 && idx < MAX_STREAMS) {
      Tuple *username_t = dict_find(iterator, MESSAGE_KEY_STREAM_USERNAME);
      Tuple *category_t = dict_find(iterator, MESSAGE_KEY_STREAM_CATEGORY);
      Tuple *viewers_t = dict_find(iterator, MESSAGE_KEY_STREAM_VIEWER_COUNT);
      Tuple *title_t = dict_find(iterator, MESSAGE_KEY_STREAM_TITLE);

      StreamInfo *stream = &s_streams[idx];
      if (username_t) strncpy(stream->username, username_t->value->cstring, MAX_USERNAME_LEN - 1);
      if (category_t) strncpy(stream->category, category_t->value->cstring, MAX_CATEGORY_LEN - 1);
      if (viewers_t) stream->viewer_count = (int)viewers_t->value->int32;
      if (title_t) strncpy(stream->title, title_t->value->cstring, MAX_TITLE_LEN - 1);

      s_streams_received++;
      if (s_streams_received >= s_stream_count) {
        s_state = STATE_LOADED;
      }
      menu_layer_reload_data(s_menu_layer);
    }
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped! Reason: %d", (int)reason);
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed! Reason: %d", (int)reason);
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

// ---- Main window ----

static void main_window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);

  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_rows = menu_get_num_rows_callback,
    .get_cell_height = menu_get_cell_height_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback
  });
  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  layer_add_child(root, menu_layer_get_layer(s_menu_layer));
}

static void main_window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);
}

// ---- App lifecycle ----

static void init() {
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);

  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_open(512, 256);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  dict_write_uint8(iter, MESSAGE_KEY_REQUEST_STREAMS, 1);
  app_message_outbox_send();
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
