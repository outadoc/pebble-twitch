#include "main_window.h"
#include "detail_window.h"

#include <pebble.h>

#include "../constants.h"
#include "../modules/stream_data.h"

static Window *s_main_window;
static MenuLayer *s_menu_layer;
static StatusBarLayer *s_status_bar;

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data)
{
    if (stream_data_get_received() > 0)
    {
        return stream_data_get_received();
    }

    // One row for loading/empty state
    return 1;
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data)
{
    int total = stream_data_get_total();

    if (total == STATE_WAITING)
    {
        menu_cell_basic_draw(ctx, cell_layer, "Loading...", NULL, NULL);
        return;
    }

    if (total == STATE_NOT_CONFIGURED)
    {
        menu_cell_basic_draw(ctx, cell_layer, "Config. required", NULL, NULL);
        return;
    }

    if (total == STATE_NETWORK_ERROR)
    {
        menu_cell_basic_draw(ctx, cell_layer, "Network error", NULL, NULL);
        return;
    }

    if (total == 0)
    {
        menu_cell_basic_draw(ctx, cell_layer, "No live streams", NULL, NULL);
        return;
    }

    if (stream_data_get_received() > cell_index->row)
    {
        StreamInfo *stream = stream_data_get(cell_index->row);
        static char subtitle_buf[48];
        static char viewers_short[16];
        format_viewer_count(viewers_short, sizeof(viewers_short), stream->viewer_count);
        snprintf(subtitle_buf, sizeof(subtitle_buf), "%s \xc2\xb7 %s", viewers_short, stream->category);
        menu_cell_basic_draw(ctx, cell_layer, stream->username, subtitle_buf, NULL);
    }
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data)
{
    if (stream_data_get_received() == 0)
    {
        return;
    }

    if ((int)cell_index->row >= stream_data_get_received())
    {
        return;
    }

    show_detail_window(cell_index->row);
}

static void main_window_load(Window *window)
{
    Layer *root = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(root);

    s_status_bar = status_bar_layer_create();
    status_bar_layer_set_colors(s_status_bar, GColorWhite, GColorBlack);
    layer_set_frame(status_bar_layer_get_layer(s_status_bar), GRect(bounds.origin.x, bounds.origin.y, bounds.size.w, STATUS_BAR_LAYER_HEIGHT));
    layer_add_child(root, status_bar_layer_get_layer(s_status_bar));

    s_menu_layer = menu_layer_create(GRect(bounds.origin.x, bounds.origin.y + STATUS_BAR_LAYER_HEIGHT, bounds.size.w, bounds.size.h));
    menu_layer_set_highlight_colors(s_menu_layer, COLOR_ACCENT, COLOR_ON_ACCENT);
    menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks){.get_num_rows = menu_get_num_rows_callback, .draw_row = menu_draw_row_callback, .select_click = menu_select_callback});
    menu_layer_set_click_config_onto_window(s_menu_layer, window);
    layer_add_child(root, menu_layer_get_layer(s_menu_layer));
}

static void main_window_unload(Window *window)
{
    menu_layer_destroy(s_menu_layer);
    status_bar_layer_destroy(s_status_bar);

    s_menu_layer = NULL;
    s_status_bar = NULL;
}

void main_window_push(void)
{
    s_main_window = window_create();
    window_set_window_handlers(s_main_window, (WindowHandlers){
                                                  .load = main_window_load,
                                                  .unload = main_window_unload});
    window_stack_push(s_main_window, true);
}

void main_window_reload_data(void)
{
    menu_layer_reload_data(s_menu_layer);
}

void main_window_destroy(void)
{
    window_destroy(s_main_window);
}
