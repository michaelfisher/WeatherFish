
#include <pebble.h>
  
#define KEY_UPDATEREQ 0
#define KEY_LOCATION 1  
#define KEY_TEMPERATURE 2
#define KEY_CONDITIONS 3
#define KEY_TEMPLOW 4 
#define KEY_TEMPHIGH 5  
#define KEY_FORECAST 6
#define KEY_LASTUPDATE 7
#define KEY_TEMPERATURE_C 8
#define KEY_TEMPLOW_C 9
#define KEY_TEMPHIGH_C 10

bool bt_connection = true;
  
static Window *s_main_window;
static TextLayer *s_date_time_background_layer;
static TextLayer *s_date_layer;
static TextLayer *s_time_layer;
static TextLayer *s_ampm_layer;
static TextLayer *s_conditions_layer;
static TextLayer *s_temperature_layer;
static TextLayer *s_templow_layer;
static TextLayer *s_temphigh_layer;
//static TextLayer *s_forecast_layer;
static TextLayer *s_location_layer;
static TextLayer *s_lastupdate_layer;
static TextLayer *s_bluetooth_layer;
static TextLayer *s_battery_layer;

static GFont s_date_font;
static GFont s_time_font;
static GFont s_ampm_font;
static GFont s_conditions_font;
static GFont s_temperature_font;
static GFont s_temphighlow_font;
//static GFont s_forecast_font;
static GFont s_location_font;
static GFont s_lastupdate_font;
static GFont s_bluetooth_font;
static GFont s_battery_font;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char date_buffer[] = "Www, Mmm DD";
  static char time_buffer[24];
  static char ampm_buffer[8];
  
  // Write the current date into the date buffer
  strftime(date_buffer,sizeof(date_buffer), "%a, %b %d", tick_time);  
  
  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(time_buffer, sizeof(time_buffer), "%H:%M", tick_time);
  } else {
    // Use 12 hour format
    strftime(time_buffer, sizeof(time_buffer), "%I:%M", tick_time);
    // Strip the zero-padding from the 12 hour time
    if (time_buffer[0] == '0') {
    	memmove(time_buffer, &time_buffer[1], sizeof(time_buffer) - 1);
    }
    strftime(ampm_buffer, sizeof(ampm_buffer), "%p", tick_time);
  }
  
  // Display the date on the date layer
  text_layer_set_text(s_date_layer, date_buffer);

  // Display the time on the time layer
  text_layer_set_text(s_time_layer, time_buffer);
  
  // Display the AM/PM on the AM/PM layer
  text_layer_set_text(s_ampm_layer, ampm_buffer);
}

// Handle bluetooth connections
static void bt_handler(bool connected) {
  // Show current connection state
  if (bt_connection == true) {
    if (connected) {
      text_layer_set_text_color(s_bluetooth_layer, GColorBlack);
      text_layer_set_text(s_bluetooth_layer, "\uf116");
    }
    else {
      text_layer_set_text_color(s_bluetooth_layer, GColorBlack);
      text_layer_set_text(s_bluetooth_layer, "");
      
      // Vibe pattern for bluetooth disconnect: ON for 300ms, OFF for 100ms, ON for 300ms, OFF for 100ms, ON for 300ms
      static const uint32_t const segments[] = { 500, 250, 500, 250, 500 };
      VibePattern pat = {
        .durations = segments,
        .num_segments = ARRAY_LENGTH(segments),
      };
      vibes_enqueue_custom_pattern(pat);
      bt_connection = false;
    }
  }
  else if (bt_connection == false) {
    if (connected) {
      text_layer_set_text_color(s_bluetooth_layer, GColorBlack);
      text_layer_set_text(s_bluetooth_layer, "\uf116");
      
      // Vibe pattern for bluetooth connect: ON for 100ms, OFF for 100ms, ON for 100ms, OFF for 100ms, ON for 100ms
      static const uint32_t const segments[] = { 100, 150, 100, 150, 100 };
      VibePattern pat = {
        .durations = segments,
        .num_segments = ARRAY_LENGTH(segments),
      };
      vibes_enqueue_custom_pattern(pat);
      bt_connection = true;
    }
    else {
      text_layer_set_text_color(s_bluetooth_layer, GColorBlack);
      text_layer_set_text(s_bluetooth_layer, "");
      
      // Vibe pattern for bluetooth disconnect: ON for 300ms, OFF for 100ms, ON for 300ms, OFF for 100ms, ON for 300ms
      static const uint32_t const segments[] = { 500, 250, 500, 250, 500 };
      VibePattern pat = {
        .durations = segments,
        .num_segments = ARRAY_LENGTH(segments),
      };
      vibes_enqueue_custom_pattern(pat);
      bt_connection = false;
    }
  }
}



// Handle battery status
static void battery_handler(BatteryChargeState charge_state) {
  // Write to buffer and display
  static char s_battery_buffer[8];
  if (charge_state.is_charging) {
    snprintf(s_battery_buffer, sizeof(s_battery_buffer), "\uf137");
  }
  else if (charge_state.is_plugged) {
      snprintf(s_battery_buffer, sizeof(s_battery_buffer), "\uf111");
  }
  else if (charge_state.charge_percent <= 10) {
    snprintf(s_battery_buffer, sizeof(s_battery_buffer), "\uf112");
  }
  else if (charge_state.charge_percent <= 40) {
    snprintf(s_battery_buffer, sizeof(s_battery_buffer), "\uf115");
  }
  else if (charge_state.charge_percent <= 70) {
    snprintf(s_battery_buffer, sizeof(s_battery_buffer), "\uf114");
  }
  else if (charge_state.charge_percent <= 100) {
    snprintf(s_battery_buffer, sizeof(s_battery_buffer), "\uf113");
  }
  text_layer_set_text(s_battery_layer, s_battery_buffer);
}

static void main_window_load(Window *window) {
  // Set window background color
  window_set_background_color(s_main_window, GColorBlack);
  
  // Create date/time background layer
  s_date_time_background_layer = text_layer_create(GRect(0, 0, 144, 70));
  
  // Apply to date/time background layer
  #ifdef PBL_COLOR
    text_layer_set_background_color(s_date_time_background_layer, GColorWhite);
  #else
    text_layer_set_background_color(s_date_time_background_layer, GColorWhite);
  #endif
  
  // Add time layer as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_time_background_layer));
  
  // Create date layer
  s_date_layer = text_layer_create(GRect(0, 0, 144, 20));
  
  // Create date font
  s_date_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  
  // Apply to date layer
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorBlack);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  text_layer_set_font(s_date_layer, s_date_font);
  
  // Add date layer as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  
  // Create time layer
  s_time_layer = text_layer_create(GRect(0, 15, 144, 50));
  
  // Create time font
  s_time_font = fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD);
  // s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_REPETITION_SCROLLING_50));

  // Apply to time layer
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_font(s_time_layer, s_time_font);

  // Add time layer as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
  // Create AM/PM layer
  s_ampm_layer = text_layer_create(GRect(0, 53, 142, 20));
  
  // Create AM/PM font
  s_ampm_font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  // s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_REPETITION_SCROLLING_50));

  // Apply to AM/PM layer
  text_layer_set_background_color(s_ampm_layer, GColorClear);
  text_layer_set_text_color(s_ampm_layer, GColorBlack);
  text_layer_set_text_alignment(s_ampm_layer, GTextAlignmentRight);
  text_layer_set_font(s_ampm_layer, s_ampm_font);

  // Add AM/PM layer as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_ampm_layer));
  
  // Create conditions layer
  s_conditions_layer = text_layer_create(GRect(0, 122, 144, 28));
  
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
  s_temperature_layer = text_layer_create(GRect(0, 82, 144, 44));
  
  // Create temperature font
  s_temperature_font = fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT);

  // Apply to temperature layer
  text_layer_set_background_color(s_temperature_layer, GColorClear);
  text_layer_set_text_color(s_temperature_layer, GColorWhite);
  text_layer_set_text_alignment(s_temperature_layer, GTextAlignmentCenter);
  text_layer_set_font(s_temperature_layer, s_temperature_font);
  text_layer_set_text(s_temperature_layer, "");

  // Add temperature layer as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_temperature_layer));
  
  // Create temphighlow font
  s_temphighlow_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  
  // Create templow layer
  s_templow_layer = text_layer_create(GRect(0, 98, 30, 36));

  // Apply to templow layer
  text_layer_set_background_color(s_templow_layer, GColorClear);
  #ifdef PBL_COLOR
    text_layer_set_text_color(s_templow_layer, GColorCadetBlue);
  #else
    text_layer_set_text_color(s_templow_layer, GColorWhite);
  #endif
  text_layer_set_text_alignment(s_templow_layer, GTextAlignmentCenter);
  text_layer_set_font(s_templow_layer, s_temphighlow_font);
  text_layer_set_text(s_templow_layer, "");

  // Add templow layer as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_templow_layer));
  
  // Create temphigh layer
  s_temphigh_layer = text_layer_create(GRect(114, 98, 30, 36));

  // Apply to temphigh layer
  text_layer_set_background_color(s_temphigh_layer, GColorClear);
  #ifdef PBL_COLOR
    text_layer_set_text_color(s_temphigh_layer, GColorChromeYellow);
  #else
    text_layer_set_text_color(s_temphigh_layer, GColorWhite);
  #endif
  text_layer_set_text_alignment(s_temphigh_layer, GTextAlignmentCenter);
  text_layer_set_font(s_temphigh_layer, s_temphighlow_font);
  text_layer_set_text(s_temphigh_layer, "");

  // Add temphigh layer as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_temphigh_layer));
  
  // Create forecast layer
  //s_forecast_layer = text_layer_create(GRect(0, 126, 144, 28));
  
  // Create forecast font
  //s_forecast_font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);

  // Apply to forecast layer
  //text_layer_set_background_color(s_forecast_layer, GColorClear);
  //text_layer_set_text_color(s_forecast_layer, GColorWhite);
  //text_layer_set_text_alignment(s_forecast_layer, GTextAlignmentCenter);
  //text_layer_set_font(s_forecast_layer, s_forecast_font);
  //text_layer_set_text(s_forecast_layer, "");

  // Add forecast layer as a child layer to the Window's root layer
  //layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast_layer));
  
  // Create location layer
  s_location_layer = text_layer_create(GRect(0, 73, 144, 16));
  
  // Create location font
  s_location_font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);

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
  
  // Create bluetooth layer
  s_bluetooth_layer = text_layer_create(GRect(2, 0, 142, 16));
  
  // Create bluetooth font
  s_bluetooth_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IONICONS_16));
  
  // Apply to bluetooth layer
  text_layer_set_background_color(s_bluetooth_layer, GColorClear);
  text_layer_set_text_alignment(s_bluetooth_layer, GTextAlignmentLeft);
  text_layer_set_font(s_bluetooth_layer, s_bluetooth_font);
  
  // Add bluetooth layer as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_bluetooth_layer));
  
  // Create battery layer
  s_battery_layer = text_layer_create(GRect(0, 0, 142, 16));
  
  // Create battery font
  s_battery_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IONICONS_16));
  //s_battery_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  
  // Apply to battery layer
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_text_color(s_battery_layer, GColorBlack);
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentRight);
  text_layer_set_font(s_battery_layer, s_battery_font);
  
  // Add battery layer as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_battery_layer));
  
  // Show current bluetooth state
  bt_handler(bluetooth_connection_service_peek());
  
  // Make sure the time is displayed from the start
  update_time();
}

static void main_window_unload(Window *window) {
  // Unload fonts
  fonts_unload_custom_font(s_bluetooth_font);
  
  // Destroy layers
  text_layer_destroy(s_date_time_background_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_conditions_layer);
  text_layer_destroy(s_temperature_layer);
  text_layer_destroy(s_templow_layer);
  text_layer_destroy(s_temphigh_layer);
  //text_layer_destroy(s_forecast_layer);
  text_layer_destroy(s_location_layer);
  text_layer_destroy(s_lastupdate_layer);
  text_layer_destroy(s_bluetooth_layer);
  text_layer_destroy(s_battery_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  
  // Get weather update every time the minute is divisible evenly by 30 (e.g. top and bottom of the hour)
  // It will still update anytime the watch face is loaded
  if(tick_time->tm_min % 30 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_cstring(iter, 0, "Update requested.");

    // Send the message!
    app_message_outbox_send();
  }
  
  // Get the current battery level
  battery_handler(battery_state_service_peek());
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
  //static char forecast_layer_buffer[32];
  //static char forecast_buffer[32];
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
    //case KEY_FORECAST:
      //snprintf(forecast_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
      //break;
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
  //snprintf(forecast_layer_buffer, sizeof(forecast_layer_buffer), "%s", forecast_buffer);
  //text_layer_set_text(s_forecast_layer, forecast_layer_buffer);
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
  
  // Subscribe to bluetooth updates
  bluetooth_connection_service_subscribe(bt_handler);
  
  // Subscribe to the Battery State Service
  battery_state_service_subscribe(battery_handler);
  
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