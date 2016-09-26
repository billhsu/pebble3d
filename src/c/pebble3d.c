#include <pebble.h>

static Window *s_main_window;
#define WINDOW_WIDTH 144
#define WINDOW_HEIGHT 168
static int const FRAME_BUFFER_SIZE = 1792;
static int const DATA_COUNTS = 14;
static uint8_t s_frame_buffer[WINDOW_HEIGHT][WINDOW_WIDTH];

static void send_req() {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    int dummy = 1;
    dict_write_int(iter, MESSAGE_KEY_REQ, &dummy, sizeof(int), true);

    dict_write_end(iter);
    app_message_outbox_send();
}

static void update_proc_frame_buffer(Layer *layer, GContext *ctx) {
    GBitmap *fb = graphics_capture_frame_buffer(ctx);
    GRect fb_bounds = gbitmap_get_bounds(fb);
    uint8_t *row = gbitmap_get_data(fb);
    uint16_t row_bytes = gbitmap_get_bytes_per_row(fb);
    for (int16_t y = fb_bounds.origin.y; y < fb_bounds.size.h; y++) {
        memcpy(row, s_frame_buffer[y], row_bytes);
        row += row_bytes;
    }
    graphics_release_frame_buffer(ctx, fb);
    send_req();
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
    Tuple *frame = dict_find(iter, MESSAGE_KEY_FRAME_DATA);
    Tuple *ready = dict_find(iter, MESSAGE_KEY_READY);
    if (frame) {
        uint8_t seq = frame->value->data[0];
        memcpy(s_frame_buffer[seq * 12], frame->value->data + 1, frame->length - 1);
        if (seq == DATA_COUNTS - 1) {
            layer_mark_dirty(window_get_root_layer(s_main_window));
            layer_set_update_proc(window_get_root_layer(s_main_window), update_proc_frame_buffer);
            APP_LOG(APP_LOG_LEVEL_DEBUG, "frame updated.");
        }
    } else if (ready) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "ready.");
        send_req();
    } else {
        APP_LOG(APP_LOG_LEVEL_WARNING, "dict_find failed!");
    }
}

static void in_dropped_handler(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "App Message Dropped - Reason: %d", reason);
}

static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "App Message Failed to Send - Reason: %d", reason);
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

