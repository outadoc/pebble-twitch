#include "error_window.h"
#include "../constants.h"

#include <pebble.h>

#define ERROR_WINDOW_PADDING 8
#define ERROR_WINDOW_GAP 8

static Window *s_error_window;
static TextLayer *s_title_layer;
static TextLayer *s_message_layer;

static char s_title_buf[64];
static char s_message_buf[256];

static void back_click_handler(ClickRecognizerRef recognizer, void *context)
{
    window_stack_pop_all(true);
}

static void click_config_provider(void *context)
{
    window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
}

static void error_window_load(Window *window)
{
    Layer *root = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(root);

    window_set_background_color(window, COLOR_MODAL);
    window_set_click_config_provider(window, click_config_provider);

    int16_t content_x = bounds.origin.x + ERROR_WINDOW_PADDING;
    int16_t content_w = bounds.size.w - (ERROR_WINDOW_PADDING * 2);

    GFont font_title = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
    GFont font_message = fonts_get_system_font(FONT_KEY_GOTHIC_18);
    GRect measure_bounds = GRect(0, 0, content_w, 2000);

    int16_t title_h = graphics_text_layout_get_content_size(
                          s_title_buf, font_title, measure_bounds,
                          GTextOverflowModeWordWrap, GTextAlignmentCenter)
                          .h +
                      DETAIL_TEXT_ADDITIONAL_HEIGHT;

    int16_t message_h = graphics_text_layout_get_content_size(
                            s_message_buf, font_message, measure_bounds,
                            GTextOverflowModeWordWrap, GTextAlignmentCenter)
                            .h +
                        DETAIL_TEXT_ADDITIONAL_HEIGHT;

    int16_t total_h = title_h + ERROR_WINDOW_GAP + message_h;
    int16_t start_y = bounds.origin.y + (bounds.size.h - total_h) / 2;

    s_title_layer = text_layer_create(GRect(content_x, start_y, content_w, title_h));
    text_layer_set_background_color(s_title_layer, GColorClear);
    text_layer_set_text_color(s_title_layer, GColorWhite);
    text_layer_set_font(s_title_layer, font_title);
    text_layer_set_text_alignment(s_title_layer, GTextAlignmentCenter);
    text_layer_set_overflow_mode(s_title_layer, GTextOverflowModeWordWrap);
    text_layer_set_text(s_title_layer, s_title_buf);
    layer_add_child(root, text_layer_get_layer(s_title_layer));

    s_message_layer = text_layer_create(GRect(content_x, start_y + title_h + ERROR_WINDOW_GAP, content_w, message_h));
    text_layer_set_background_color(s_message_layer, GColorClear);
    text_layer_set_text_color(s_message_layer, GColorWhite);
    text_layer_set_font(s_message_layer, font_message);
    text_layer_set_text_alignment(s_message_layer, GTextAlignmentCenter);
    text_layer_set_overflow_mode(s_message_layer, GTextOverflowModeWordWrap);
    text_layer_set_text(s_message_layer, s_message_buf);
    layer_add_child(root, text_layer_get_layer(s_message_layer));
}

static void error_window_unload(Window *window)
{
    text_layer_destroy(s_title_layer);
    text_layer_destroy(s_message_layer);

    s_title_layer = NULL;
    s_message_layer = NULL;

    window_destroy(s_error_window);
    s_error_window = NULL;
}

void error_window_push(const char *title, const char *message)
{
    strncpy(s_title_buf, title, sizeof(s_title_buf) - 1);
    strncpy(s_message_buf, message, sizeof(s_message_buf) - 1);

    s_error_window = window_create();
    window_set_window_handlers(s_error_window, (WindowHandlers){
                                                   .load = error_window_load,
                                                   .unload = error_window_unload});
    window_stack_push(s_error_window, true);
}

void error_window_pop(void)
{
    if (s_error_window != NULL)
    {
        window_stack_remove(s_error_window, true);
    }
}
