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

static StreamInfo s_streams[MAX_STREAMS];
static int s_streams_total = 0;
static int s_streams_received = 0;
static int s_selected_index = 0;

static GColor8 s_color_accent = GColorPurple;
static GColor8 s_color_on_accent = GColorWhite;

// Main window
static Window *s_main_window;
static MenuLayer *s_menu_layer;
static StatusBarLayer *s_status_bar;

// Detail window
static Window *s_detail_window;
static ScrollLayer *s_scroll_layer;
static TextLayer *s_username_layer;
static TextLayer *s_viewers_layer;
static TextLayer *s_category_layer;
static TextLayer *s_title_layer;

// Persistent buffers for detail window text
static char s_viewers_buf[24];

static int16_t measure_text(const char *text, GFont font, int16_t width) {
  GSize size = graphics_text_layout_get_content_size(
    text, font, GRect(0, 0, width, 2000),
    GTextOverflowModeWordWrap, GTextAlignmentLeft);
  return size.h;
}

// ---- Detail window ----

static void detail_window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);
  StreamInfo *stream = &s_streams[s_selected_index];

  s_status_bar = status_bar_layer_create();
  status_bar_layer_set_colors(s_status_bar, s_color_accent, s_color_on_accent);
  layer_set_frame(status_bar_layer_get_layer(s_status_bar), GRect(0, 0, bounds.size.w, STATUS_BAR_LAYER_HEIGHT));
  layer_add_child(root, status_bar_layer_get_layer(s_status_bar));

  s_scroll_layer = scroll_layer_create(GRect(0, STATUS_BAR_LAYER_HEIGHT, bounds.size.w, bounds.size.h));
  scroll_layer_set_click_config_onto_window(s_scroll_layer, window);

  const int16_t pad = 4;
  const int16_t x = 5;
  const int16_t w = bounds.size.w - 10;
  int16_t y = pad;

  GFont font_heading = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
  GFont font_title = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  GFont font_body = fonts_get_system_font(FONT_KEY_GOTHIC_18);

  int16_t username_h = measure_text(stream->username, font_heading, w);
  s_username_layer = text_layer_create(GRect(x, y, w, username_h));
  text_layer_set_font(s_username_layer, font_heading);
  text_layer_set_overflow_mode(s_username_layer, GTextOverflowModeTrailingEllipsis);
  text_layer_set_text(s_username_layer, stream->username);
  y += username_h + pad;

  snprintf(s_viewers_buf, sizeof(s_viewers_buf), "%d viewers", stream->viewer_count);
  int16_t viewers_h = measure_text(s_viewers_buf, font_body, w);
  s_viewers_layer = text_layer_create(GRect(x, y, w, viewers_h));
  text_layer_set_font(s_viewers_layer, font_body);
  text_layer_set_text(s_viewers_layer, s_viewers_buf);
  y += viewers_h + pad;

  int16_t category_h = measure_text(stream->category, font_title, w);
  s_category_layer = text_layer_create(GRect(x, y, w, category_h));
  text_layer_set_font(s_category_layer, font_title);
  text_layer_set_overflow_mode(s_category_layer, GTextOverflowModeTrailingEllipsis);
  text_layer_set_text(s_category_layer, stream->category);
  y += category_h + pad;

  int16_t title_h = measure_text(stream->title, font_body, w);
  s_title_layer = text_layer_create(GRect(x, y, w, title_h));
  text_layer_set_font(s_title_layer, font_body);
  text_layer_set_overflow_mode(s_title_layer, GTextOverflowModeWordWrap);
  text_layer_set_text(s_title_layer, stream->title);
  y += title_h + pad;

  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_username_layer));
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_viewers_layer));
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_category_layer));
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_title_layer));

  scroll_layer_set_content_size(s_scroll_layer, GSize(bounds.size.w, y));
  layer_add_child(root, scroll_layer_get_layer(s_scroll_layer));
}

static void detail_window_unload(Window *window) {
  text_layer_destroy(s_username_layer);
  text_layer_destroy(s_viewers_layer);
  text_layer_destroy(s_category_layer);
  text_layer_destroy(s_title_layer);
  scroll_layer_destroy(s_scroll_layer);
  status_bar_layer_destroy(s_status_bar);

  s_username_layer = NULL;
  s_viewers_layer = NULL;
  s_category_layer = NULL;
  s_title_layer = NULL;
  s_scroll_layer = NULL;
  s_status_bar = NULL;

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
  if (s_streams_received > 0) {
    return s_streams_received;
  }
  return 1; // One row for loading/empty state
}

static int16_t menu_get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  return 44;
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  if (s_streams_total == 0) {
    menu_cell_basic_draw(ctx, cell_layer, "No live streams", NULL, NULL);
    return;
  }

  if (s_streams_received > cell_index->row) {
    StreamInfo *stream = &s_streams[cell_index->row];
    static char subtitle_buf[48];
    static char viewers_short[16];
    snprintf(viewers_short, sizeof(viewers_short), "%d", stream->viewer_count);
    snprintf(subtitle_buf, sizeof(subtitle_buf), "%s \xc2\xb7 %s", viewers_short, stream->category);
    menu_cell_basic_draw(ctx, cell_layer, stream->username, subtitle_buf, NULL);
  }
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  if (s_streams_received == 0) return;
  if ((int)cell_index->row >= s_streams_received) return;
  show_detail_window(cell_index->row);
}

// ---- AppMessage ----

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  Tuple *count_t = dict_find(iterator, MESSAGE_KEY_STREAM_COUNT);
  if (count_t) {
    int count = (int)count_t->value->int32;

    if (count == 0) {
      s_streams_total = 0;
      s_streams_received = 0;
    } else {
      s_streams_total = count > MAX_STREAMS ? MAX_STREAMS : count;
      s_streams_received = 0;
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

  s_status_bar = status_bar_layer_create();
  status_bar_layer_set_colors(s_status_bar, s_color_accent, s_color_on_accent);
  layer_set_frame(status_bar_layer_get_layer(s_status_bar), GRect(0, 0, bounds.size.w, STATUS_BAR_LAYER_HEIGHT));
  layer_add_child(root, status_bar_layer_get_layer(s_status_bar));

  s_menu_layer = menu_layer_create(GRect(0, STATUS_BAR_LAYER_HEIGHT, bounds.size.w, bounds.size.h));
  menu_layer_set_highlight_colors(s_menu_layer, s_color_accent, s_color_on_accent);
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
  status_bar_layer_destroy(s_status_bar);

  s_menu_layer = NULL;
  s_status_bar = NULL;
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
