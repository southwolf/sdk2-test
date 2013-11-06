#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

static Window *window;

static TextLayer *temperature_layer;
static char temperature[16];

static BitmapLayer *icon_layer;
static GBitmap *icon_bitmap;
static bool icon_loaded = false;

static AppSync sync;
static uint8_t sync_buffer[32];

enum WeatherKey {
  WEATHER_ICON_KEY = 0x0,         // TUPLE_INT
  WEATHER_TEMPERATURE_KEY = 0x1,  // TUPLE_CSTRING
};

static uint32_t WEATHER_ICONS[] = {
  RESOURCE_ID_IMAGE_SUN,
  RESOURCE_ID_IMAGE_CLOUD,
  RESOURCE_ID_IMAGE_RAIN,
  RESOURCE_ID_IMAGE_SNOW
};

static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  switch (key) {
    case WEATHER_ICON_KEY:
      if (icon_loaded) {
        gbitmap_destroy(icon_bitmap);
      }

      icon_bitmap = gbitmap_create_with_resource(WEATHER_ICONS[new_tuple->value->uint8]);
      icon_loaded = true;

      layer_mark_dirty(bitmap_layer_get_layer(icon_layer));
      break;

    case WEATHER_TEMPERATURE_KEY:
      // App Sync keeps the new_tuple around, so we may use it directly
      text_layer_set_text(temperature_layer, new_tuple->value->cstring);
      break;
  }
}

static void send_cmd() {
  Tuplet value = TupletInteger(1, 1);

  DictionaryIterator *iter;
  app_message_out_get(&iter);

  if (iter == NULL) {
    return;
  }

  dict_write_tuplet(iter, &value);
  dict_write_end(iter);

  app_message_out_send();
  app_message_out_release();
}

static void init() {
  window = window_create();
  window_set_background_color(window, GColorBlack);
  window_set_fullscreen(window, true);

  icon_layer = bitmap_layer_create(GRect(32, 10, 80, 80));
  bitmap_layer_set_bitmap(icon_layer, icon_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(icon_layer));

  temperature_layer = text_layer_create(GRect(0, 100, 144, 68));
  text_layer_set_text_color(temperature_layer, GColorWhite);
  text_layer_set_background_color(temperature_layer, GColorClear);
  text_layer_set_font(temperature_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(temperature_layer, GTextAlignmentCenter);
  text_layer_set_text(temperature_layer, temperature);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(temperature_layer));

  Tuplet initial_values[] = {
    TupletInteger(WEATHER_ICON_KEY, (uint8_t) 1),
    TupletCString(WEATHER_TEMPERATURE_KEY, "1234\u00B0C"),
  };

  app_message_open(64, 64);
  send_cmd();

  app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);


  window_stack_push(window, true /* Animated */);
}

static void deinit() {
  app_sync_deinit(&sync);

  if (icon_loaded) {
    gbitmap_destroy(icon_bitmap);
  }
  app_sync_deinit(&sync);

  text_layer_destroy(temperature_layer);
  bitmap_layer_destroy(icon_layer);
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
