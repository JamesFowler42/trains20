#include "pebble.h"

#define PAD 5

static Window *window;

static TextLayer *from_layer;
static TextLayer *to_layer;
static TextLayer *time1_layer;
static TextLayer *time2_layer;
static TextLayer *time3_layer;
static TextLayer *dest1_layer;
static TextLayer *dest2_layer;
static TextLayer *dest3_layer;
static TextLayer *plat1_layer;
static TextLayer *plat2_layer;
static TextLayer *plat3_layer;
static TextLayer *current_layer;
static BitmapLayer *icon_layer;
static GBitmap *icon_bitmap = NULL;

static AppSync sync;
static uint8_t sync_buffer[512];

static char g_time_text[11];

enum TrainsKey {
  TRAINS_TIME1 = 1,               // TUPLE_CSTRING
  TRAINS_DEST1 = 2,				  // TUPLE_CSTRING
  TRAINS_PLAT1 = 3,				  // TUPLE_CSTRING
  TRAINS_TIME2 = 4,               // TUPLE_CSTRING
  TRAINS_DEST2 = 5,				  // TUPLE_CSTRING
  TRAINS_PLAT2 = 6,				  // TUPLE_CSTRING
  TRAINS_TIME3 = 7,               // TUPLE_CSTRING
  TRAINS_DEST3 = 8,				  // TUPLE_CSTRING
  TRAINS_PLAT3 = 9,				  // TUPLE_CSTRING
  FROM = 10,		              // TUPLE_CSTRING
  TO = 11                         // TUPLE_CSTRING
};

static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  switch (key) {
    case TRAINS_TIME1:
      text_layer_set_text(time1_layer, new_tuple->value->cstring);
      break;
    case TRAINS_DEST1:
      text_layer_set_text(dest1_layer, new_tuple->value->cstring);
      break;
    case TRAINS_PLAT1:
      text_layer_set_text(plat1_layer, new_tuple->value->cstring);
      break;
    case TRAINS_TIME2:
      text_layer_set_text(time2_layer, new_tuple->value->cstring);
      break;
    case TRAINS_DEST2:
      text_layer_set_text(dest2_layer, new_tuple->value->cstring);
      break;
    case TRAINS_PLAT2:
      text_layer_set_text(plat2_layer, new_tuple->value->cstring);
      break;
    case TRAINS_TIME3:
      text_layer_set_text(time3_layer, new_tuple->value->cstring);
      break;
    case TRAINS_DEST3:
      text_layer_set_text(dest3_layer, new_tuple->value->cstring);
      break;
    case TRAINS_PLAT3:
      text_layer_set_text(plat3_layer, new_tuple->value->cstring);
      break;
    case FROM:
      text_layer_set_text(from_layer, new_tuple->value->cstring);
      break;
    case TO:
      text_layer_set_text(to_layer, new_tuple->value->cstring);
      break;
  }
}

static void send_cmd(void) {
  Tuplet value = TupletCString(1, " ");

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL) {
    return;
  }

  dict_write_tuplet(iter, &value);
  dict_write_end(iter);

  app_message_outbox_send();
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  int top = 0;
  icon_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  layer_add_child(window_layer, bitmap_layer_get_layer(icon_layer));
  icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_TRAIN);
  bitmap_layer_set_bitmap(icon_layer, icon_bitmap);

  from_layer = text_layer_create(GRect(8, 42, 36, 20));
  text_layer_set_text_color(from_layer, GColorBlack);
  text_layer_set_background_color(from_layer, GColorClear);
  text_layer_set_font(from_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(from_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(from_layer));

  to_layer = text_layer_create(GRect(104, 42, 36, 20));
  text_layer_set_text_color(to_layer, GColorBlack);
  text_layer_set_background_color(to_layer, GColorClear);
  text_layer_set_font(to_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(to_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(to_layer));


  top = top + 10 + 52 + PAD;

  time1_layer = text_layer_create(GRect(0, top, 36, 20));
  text_layer_set_text_color(time1_layer, GColorWhite);
  text_layer_set_background_color(time1_layer, GColorClear);
  text_layer_set_font(time1_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(time1_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(time1_layer));

  dest1_layer = text_layer_create(GRect(36, top, 72, 20));
  text_layer_set_text_color(dest1_layer, GColorWhite);
  text_layer_set_background_color(dest1_layer, GColorClear);
  text_layer_set_font(dest1_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(dest1_layer, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(dest1_layer));

  plat1_layer = text_layer_create(GRect(108, top, 36, 20));
  text_layer_set_text_color(plat1_layer, GColorWhite);
  text_layer_set_background_color(plat1_layer, GColorClear);
  text_layer_set_font(plat1_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(plat1_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(plat1_layer));
  top = top + 20 + PAD;

  time2_layer = text_layer_create(GRect(0, top, 36, 20));
  text_layer_set_text_color(time2_layer, GColorWhite);
  text_layer_set_background_color(time2_layer, GColorClear);
  text_layer_set_font(time2_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(time2_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(time2_layer));

  dest2_layer = text_layer_create(GRect(36, top, 72, 20));
  text_layer_set_text_color(dest2_layer, GColorWhite);
  text_layer_set_background_color(dest2_layer, GColorClear);
  text_layer_set_font(dest2_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(dest2_layer, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(dest2_layer));

  plat2_layer = text_layer_create(GRect(108, top, 36, 20));
  text_layer_set_text_color(plat2_layer, GColorWhite);
  text_layer_set_background_color(plat2_layer, GColorClear);
  text_layer_set_font(plat2_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(plat2_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(plat2_layer));
  top = top + 20 + PAD;

  time3_layer = text_layer_create(GRect(0, top, 36, 20));
  text_layer_set_text_color(time3_layer, GColorWhite);
  text_layer_set_background_color(time3_layer, GColorClear);
  text_layer_set_font(time3_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(time3_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(time3_layer));

  dest3_layer = text_layer_create(GRect(36, top, 72, 20));
  text_layer_set_text_color(dest3_layer, GColorWhite);
  text_layer_set_background_color(dest3_layer, GColorClear);
  text_layer_set_font(dest3_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(dest3_layer, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(dest3_layer));

  plat3_layer = text_layer_create(GRect(108, top, 36, 20));
  text_layer_set_text_color(plat3_layer, GColorWhite);
  text_layer_set_background_color(plat3_layer, GColorClear);
  text_layer_set_font(plat3_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(plat3_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(plat3_layer));
  top = top + 20;

  current_layer = text_layer_create(GRect(0, top, 144, 27));
  text_layer_set_text_color(current_layer, GColorBlack);
  text_layer_set_background_color(current_layer, GColorClear);
  text_layer_set_font(current_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(current_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(current_layer));

  Tuplet initial_values[] = {
    TupletCString(TRAINS_TIME1, ""),
    TupletCString(TRAINS_DEST1, ""),
    TupletCString(TRAINS_PLAT1, ""),
    TupletCString(TRAINS_TIME2, ""),
    TupletCString(TRAINS_DEST2, ""),
    TupletCString(TRAINS_PLAT2, ""),
    TupletCString(TRAINS_TIME3, ""),
    TupletCString(TRAINS_DEST3, ""),
    TupletCString(TRAINS_PLAT3, ""),
    TupletCString(FROM, ""),
    TupletCString(TO, ""),
  };

  app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);

  send_cmd();
}

static void window_unload(Window *window) {
  app_sync_deinit(&sync);

  gbitmap_destroy(icon_bitmap);
  text_layer_destroy(from_layer);
  text_layer_destroy(to_layer);
  text_layer_destroy(time1_layer);
  text_layer_destroy(time2_layer);
  text_layer_destroy(time3_layer);
  text_layer_destroy(dest1_layer);
  text_layer_destroy(dest2_layer);
  text_layer_destroy(dest3_layer);
  text_layer_destroy(plat1_layer);
  text_layer_destroy(plat2_layer);
  text_layer_destroy(plat3_layer);
  text_layer_destroy(current_layer);

  bitmap_layer_destroy(icon_layer);
}

/*
 * Clock tick
 */
void handle_tick(struct tm *tick_time, TimeUnits units_changed) {

    strftime(g_time_text, sizeof(g_time_text), "%R", tick_time);

    text_layer_set_text(current_layer, g_time_text);

    if (tick_time->tm_min%5 == 0)
    	send_cmd();
}

static void init(void) {
  window = window_create();
  window_set_background_color(window, GColorBlack);
  window_set_fullscreen(window, true);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });

  const int inbound_size = app_message_inbox_size_maximum();
  const int outbound_size = app_message_outbox_size_maximum();

  app_message_open(inbound_size, outbound_size);

  const bool animated = true;
  window_stack_push(window, animated);

  tick_timer_service_subscribe(MINUTE_UNIT, &handle_tick);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
