#include <pebble.h>

static Window *s_main_window;
#define WINDOW_WIDTH 144
#define WINDOW_HEIGHT 168
static int const FRAME_BUFFER_SIZE = WINDOW_HEIGHT/2 * WINDOW_WIDTH/2;
static uint8_t s_frame_buffer[WINDOW_HEIGHT/2][WINDOW_WIDTH/2];

static void update_proc_frame_buffer(Layer *layer, GContext *ctx) {
    GBitmap *fb = graphics_capture_frame_buffer(ctx);
    GRect fb_bounds = gbitmap_get_bounds(fb);
    uint8_t *row = gbitmap_get_data(fb);
    uint16_t row_bytes = gbitmap_get_bytes_per_row(fb);
    row += fb_bounds.origin.y * row_bytes;
    for (int16_t y = fb_bounds.origin.y; y < fb_bounds.size.h; y++) {
        memcpy(row, s_frame_buffer[y%(WINDOW_HEIGHT/2)], row_bytes);
        row += row_bytes;
    }
    graphics_release_frame_buffer(ctx, fb);
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "in_received_handler");
    Tuple *frame = dict_find(iter, MESSAGE_KEY_FRAME_DATA);
    if (frame) {
        APP_LOG(APP_LOG_LEVEL_WARNING, "frame->length: %d", frame->length);
        memcpy(s_frame_buffer, frame->value->data, frame->length);
        layer_set_update_proc(window_get_root_layer(s_main_window), update_proc_frame_buffer);
    } else {
        APP_LOG(APP_LOG_LEVEL_WARNING, "dict_find MESSAGE_KEY_FRAME_DATA failed!");
    }
}

static void in_dropped_handler(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Dropped!");
}

static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Failed to Send!");
}

static void window_load(Window *window) {
}
static void window_unload(Window *window) {
}
static void click_config_provider(void *context) {
}

static void init(void) {
    s_main_window = window_create();
    window_set_background_color(s_main_window, PBL_IF_COLOR_ELSE(GColorIndigo, GColorBlack));
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload
    });
    window_set_click_config_provider(s_main_window, click_config_provider);
    window_stack_push(s_main_window, true);

    app_message_register_inbox_received(in_received_handler);
    app_message_register_inbox_dropped(in_dropped_handler);
    app_message_register_outbox_failed(out_failed_handler);
    app_message_open(FRAME_BUFFER_SIZE, 64);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "App init");
}

static void deinit(void) {
    window_destroy(s_main_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}

