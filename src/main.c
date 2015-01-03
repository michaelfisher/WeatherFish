#include <pebble.h>
  
#define KEY_LOCATION 0  
#define KEY_TEMPERATURE 1
#define KEY_CONDITIONS 2
#define KEY_TEMPLOW 3
#define KEY_TEMPHIGH 4
#define KEY_FORECAST 5
#define KEY_LASTUPDATE 6

static bool appStarted = false;
  
static Window *s_main_window;
static TextLayer *s_date_layer;
static TextLayer *s_time_layer;
static TextLayer *s_conditions_layer;
static TextLayer *s_temperature_layer;
static TextLayer *s_templow_layer;
static TextLayer *s_temphigh_layer;
static TextLayer *s_forecast_layer;
static TextLayer *s_location_layer;
static TextLayer *s_lastupdate_layer;

static GFont s_date_font;
static GFont s_time_font;
static GFont s_conditions_font;
static GFont s_temperature_font;
static GFont s_temphighlow_font;
static GFont s_forecast_font;
static GFont s_location_font;
static GFont s_lastupdate_font;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char date_buffer[] = "Www, Mmm DD";
  static char time_buffer[] = "00:00";
  
  // Write the current date into the date buffer
  strftime(date_buffer,sizeof("Www, Mmm DD"), "%a, %b %d", tick_time);  
  
  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(time_buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    // Use 12 hour format
    strftime(time_buffer, sizeof("00:00"), "%I:%M", tick_time);
  }
  
  // Display the date on the date layer
  text_layer_set_text(s_date_layer, date_buffer);

  // Display the time on the time layer
  text_layer_set_text(s_time_layer, time_buffer); 
}

static void main_window_load(Window *window) {
  // Set window background color
  window_set_background_color(s_main_window, GColorBlack);
  
  // Create date layer
  s_date_layer = text_layer_create(GRect(0, 0, 144, 20));
  
  // Create date font
  s_date_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  
  // Apply to date layer
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_font(s_date_layer, s_date_font);
  
  // Add date layer as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  
  // Create time layer
  s_time_layer = text_layer_create(GRect(0, 21, 144, 50));
  
  // Create time font
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_REPETITION_SCROLLING_50));

  // Apply to time layer
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_font(s_time_layer, s_time_font);

  // Add time layer as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
  // Create conditions layer
  s_conditions_layer = text_layer_create(GRect(0, 105, 144, 28));
  
  // Create conditions font
  s_conditions_font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);

  // Apply to conditions layer
  text_layer_set_background_color(s_conditions_layer, GColorClear);
  text_layer_set_text_color(s_conditions_layer, GColorWhite);
  text_layer_set_text_alignment(s_conditions_layer, GTextAlignmentCenter);
  text_layer_set_font(s_conditions_layer, s_conditions_font);
  text_layer_set_text(s_conditions_layer, "");

  // Add conditions layer as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_conditions_layer));
  
  // Create temperature layer
  s_temperature_layer = text_layer_create(GRect(0, 67, 144, 44));
  
  // Create temperature font
  s_temperature_font = fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);

  // Apply to temperature layer
  text_layer_set_background_color(s_temperature_layer, GColorClear);
  text_layer_set_text_color(s_temperature_layer, GColorWhite);
  text_layer_set_text_alignment(s_temperature_layer, GTextAlignmentCenter);
  text_layer_set_font(s_temperature_layer, s_temperature_font);
  text_layer_set_text(s_temperature_layer, "");

  // Add temperature layer as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_temperature_layer));
  
  // Create templow layer
  s_templow_layer = text_layer_create(GRect(0, 80, 30, 20));
  
  // Create temphighlow font
  s_temphighlow_font = fonts_get_system_font(FONT_KEY_GOTHIC_18);

  // Apply to templow layer
  text_layer_set_background_color(s_templow_layer, GColorClear);
  text_layer_set_text_color(s_templow_layer, GColorWhite);
  text_layer_set_text_alignment(s_templow_layer, GTextAlignmentCenter);
  text_layer_set_font(s_templow_layer, s_temphighlow_font);
  text_layer_set_text(s_templow_layer, "");

  // Add templow layer as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_templow_layer));
  
  // Create temphigh layer
  s_temphigh_layer = text_layer_create(GRect(114, 80, 30, 20));

  // Apply to temphigh layer
  text_layer_set_background_color(s_temphigh_layer, GColorClear);
  text_layer_set_text_color(s_temphigh_layer, GColorWhite);
  text_layer_set_text_alignment(s_temphigh_layer, GTextAlignmentCenter);
  text_layer_set_font(s_temphigh_layer, s_temphighlow_font);
  text_layer_set_text(s_temphigh_layer, "");

  // Add temphigh layer as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_temphigh_layer));
  
  // Create forecast layer
  s_forecast_layer = text_layer_create(GRect(0, 126, 144, 28));
  
  // Create forecast font
  s_forecast_font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);

  // Apply to forecast layer
  text_layer_set_background_color(s_forecast_layer, GColorClear);
  text_layer_set_text_color(s_forecast_layer, GColorWhite);
  text_layer_set_text_alignment(s_forecast_layer, GTextAlignmentCenter);
  text_layer_set_font(s_forecast_layer, s_forecast_font);
  text_layer_set_text(s_forecast_layer, "");

  // Add forecast layer as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast_layer));
  
  // Create location layer
  s_location_layer = text_layer_create(GRect(0, 17, 144, 16));
  
  // Create location font
  s_location_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);

  // Apply to location layer
  text_layer_set_background_color(s_location_layer, GColorClear);
  text_layer_set_text_color(s_location_layer, GColorWhite);
  text_layer_set_text_alignment(s_location_layer, GTextAlignmentCenter);
  text_layer_set_font(s_location_layer, s_location_font);
  text_layer_set_text(s_location_layer, "");

  // Add location layer as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_location_layer));
  
  // Create last update layer
  s_lastupdate_layer = text_layer_create(GRect(0, 150, 144, 16));
  
  // Create last update font
  s_lastupdate_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  
  // Apply to last update layer
  text_layer_set_background_color(s_lastupdate_layer, GColorClear);
  text_layer_set_text_color(s_lastupdate_layer, GColorWhite);
  text_layer_set_text_alignment(s_lastupdate_layer, GTextAlignmentRight);
  text_layer_set_font(s_lastupdate_layer, s_lastupdate_font);
  text_layer_set_text(s_lastupdate_layer, "");
  
  // Add last update layer as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_lastupdate_layer));
  
  // Make sure the time is displayed from the start
  update_time();
}

static void main_window_unload(Window *window) {
  // Unload fonts
  fonts_unload_custom_font(s_time_font);
  
  // Destroy layers
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_conditions_layer);
  text_layer_destroy(s_temperature_layer);
  text_layer_destroy(s_templow_layer);
  text_layer_destroy(s_temphigh_layer);
  text_layer_destroy(s_forecast_layer);
  text_layer_destroy(s_location_layer);
  text_layer_destroy(s_lastupdate_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  
  // Get weather update every time the minute is divisible evenly by 30 (e.g. top and bottom of the hour)
  if(tick_time->tm_min % 30 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
  }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char conditions_layer_buffer[32];
  static char conditions_buffer[32];
  static char temperature_layer_buffer[8];
  static char temperature_buffer[8];
  static char templow_layer_buffer[8];
  static char templow_buffer[8];
  static char temphigh_layer_buffer[8];
  static char temphigh_buffer[8];
  static char forecast_layer_buffer[32];
  static char forecast_buffer[32];
  static char location_layer_buffer[48];
  static char location_buffer[48];
  static char lastupdate_layer_buffer[16];
  static char lastupdate_buffer[16];
  
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_LOCATION:
      snprintf(location_buffer, sizeof(location_buffer), "%s", t->value->cstring);
      break;
    case KEY_TEMPERATURE:
      snprintf(temperature_buffer, sizeof(temperature_buffer), "%s", t->value->cstring);
      break;
    case KEY_CONDITIONS:
      snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
      break;
      case KEY_TEMPLOW:
      snprintf(templow_buffer, sizeof(templow_buffer), "%s", t->value->cstring);
      break;
    case KEY_TEMPHIGH:
      snprintf(temphigh_buffer, sizeof(temphigh_buffer), "%s", t->value->cstring);
      break;
    case KEY_FORECAST:
      snprintf(forecast_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
      break;
    case KEY_LASTUPDATE:
      snprintf(lastupdate_buffer, sizeof(lastupdate_buffer), "%s", t->value->cstring);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }
  
  // Assemble full string and display
  snprintf(location_layer_buffer, sizeof(location_layer_buffer), "%s", location_buffer);
  text_layer_set_text(s_location_layer, location_layer_buffer);
  snprintf(conditions_layer_buffer, sizeof(conditions_layer_buffer), "%s", conditions_buffer);
  text_layer_set_text(s_conditions_layer, conditions_layer_buffer);
  snprintf(temperature_layer_buffer, sizeof(temperature_layer_buffer), "%s", temperature_buffer);
  text_layer_set_text(s_temperature_layer, temperature_layer_buffer);
  snprintf(templow_layer_buffer, sizeof(templow_layer_buffer), "%s", templow_buffer);
  text_layer_set_text(s_templow_layer, templow_layer_buffer);
  snprintf(temphigh_layer_buffer, sizeof(temphigh_layer_buffer), "%s", temphigh_buffer);
  text_layer_set_text(s_temphigh_layer, temphigh_layer_buffer);
  snprintf(forecast_layer_buffer, sizeof(forecast_layer_buffer), "%s", forecast_buffer);
  text_layer_set_text(s_forecast_layer, forecast_layer_buffer);
  snprintf(lastupdate_layer_buffer, sizeof(lastupdate_layer_buffer), "%s", lastupdate_buffer);
  text_layer_set_text(s_lastupdate_layer, lastupdate_layer_buffer);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}
  
static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}