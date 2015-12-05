#include <pebble.h>

#define DAYLEN 1440 // mins in a day

static Window *main_window;
static Layer *panel_layer, *hand_layer;
static GPoint center;
static int radius, win_h, win_w = 0;
static AppSync sync;
static uint8_t sync_buffer[32];
static int risemins, setmins = 0;
static int nowhours, nowmins = 0;
  
enum APP_MSG_TYPE {
  RISEMINS,
  SETMINS
};

static void panel_update_handler(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  
  if (risemins != 0 && setmins != 0) {
    int daymins = setmins - risemins;
    int nightmins = DAYLEN - daymins;
    
    graphics_context_set_fill_color(ctx, GColorOrange);
    for (int i = 0; i < 12; i++) {
      int32_t angle = TRIG_MAX_ANGLE * (risemins + ((i / 12.0) * daymins)) / DAYLEN;
      APP_LOG(APP_LOG_LEVEL_INFO, "%d", (int)angle);
      
      GPoint dot = (GPoint) {
        .x = (int16_t)(sin_lookup(angle)
                       * (int16_t)(radius - 4) / TRIG_MAX_RATIO) + center.x,
        .y = (int16_t)(-cos_lookup(angle)
                       * (int16_t)(radius - 4) / TRIG_MAX_RATIO) + center.y
      };
      
      graphics_fill_circle(ctx, dot, 2);
    }
    
    graphics_context_set_fill_color(ctx, GColorBlue);
    for (int i = 0; i < 12; i++) {
      int32_t angle = TRIG_MAX_ANGLE * (setmins + ((i / 12.0) * nightmins)) / DAYLEN;
      
      GPoint dot = (GPoint) {
        .x = (int16_t)(sin_lookup(angle)
                       * (int16_t)(radius - 4) / TRIG_MAX_RATIO) + center.x,
        .y = (int16_t)(-cos_lookup(angle)
                       * (int16_t)(radius - 4) / TRIG_MAX_RATIO) + center.y
      };
      
      graphics_fill_circle(ctx, dot, 2);
    }
  }
  
  for (int i = 0; i < 24; i++) {
    graphics_context_set_fill_color(ctx, GColorMintGreen);
    
    GPoint dot = (GPoint) {
      .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * i / 24)
                     * (int32_t)(radius - 13) / TRIG_MAX_RATIO) + center.x,
      .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * i / 24)
                     * (int32_t)(radius - 13) / TRIG_MAX_RATIO) + center.y
    };
    
    graphics_fill_circle(ctx, dot, 1);
  }
  
  for (int i = 0; i < 60; i++) {
    int rad = 1;
    
    if (i % 5 == 0) {
      graphics_context_set_fill_color(ctx, GColorGreen);
      rad = 3;
    } else {
      graphics_context_set_fill_color(ctx, GColorDarkGray);
      rad = 2;
    }
    
    GPoint dot = (GPoint) {
      .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * i / 60)
                     * (int32_t)(radius - 23) / TRIG_MAX_RATIO) + center.x,
      .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * i / 60)
                     * (int32_t)(radius - 23) / TRIG_MAX_RATIO) + center.y
    };
    
    graphics_fill_circle(ctx, dot, rad);
  }
}

static void draw_hand_from_center(GContext *ctx, GPoint dot, float s0, float s1, GColor color0, GColor color1) {  
  // draw a thin short bar
  GPoint dot_0 = center;
  dot_0.x += (dot.x - center.x) * s0;
  dot_0.y += (dot.y - center.y) * s0;
  
  graphics_context_set_stroke_color(ctx, color0);
  graphics_context_set_stroke_width(ctx, 3);
  graphics_draw_line(ctx, center, dot_0);
  
  // draw a thick long bar
  GPoint dot_1 = center;
  dot_1.x += (dot.x - center.x) * s1;
  dot_1.y += (dot.y - center.y) * s1;
  
  graphics_context_set_stroke_color(ctx, color0);
  graphics_context_set_stroke_width(ctx, 6);
  graphics_draw_line(ctx, dot_0, dot_1);
  
  // draw inside the thick long bar
  graphics_context_set_stroke_color(ctx, color1);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_line(ctx, dot_0, dot_1);
}


static void hand_update_handler(Layer *layer, GContext *ctx) {
  GPoint minsdot = (GPoint) {
    .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * nowmins / 60)
                   * (int32_t)(radius) / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * nowmins / 60)
                   * (int32_t)(radius) / TRIG_MAX_RATIO) + center.y
  };
  
  int hours = nowhours > 12 ? nowhours - 12 : nowhours;
  GPoint hoursdot = (GPoint) {
    .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * hours / 12)
                   * (int32_t)(radius) / TRIG_MAX_RATIO) + center.x,
    .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * hours / 12)
                   * (int32_t)(radius) / TRIG_MAX_RATIO) + center.y
  };
  
  draw_hand_from_center(ctx, hoursdot, 0.2, 0.6, GColorWhite, GColorBlack);
  draw_hand_from_center(ctx, minsdot, 0.2, 0.95, GColorWhite, GColorBlack);
  
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, center, 4);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, center, 2);
  
  if (risemins != 0 && setmins != 0) {
    int32_t angle = TRIG_MAX_ANGLE * ((nowhours * 60) + nowmins) / DAYLEN;
    
    GPoint hebrewdot = (GPoint) {
      .x = (int16_t)(sin_lookup(angle)
                     * (int32_t)(radius) / TRIG_MAX_RATIO) + center.x,
      .y = (int16_t)(-cos_lookup(angle)
                     * (int32_t)(radius) / TRIG_MAX_RATIO) + center.y
    };
    
    GPoint dot_0, dot_1;
    dot_0 = center;
    dot_0.x += (hebrewdot.x - center.x);
    dot_0.y += (hebrewdot.y - center.y);
    
    dot_1 = center;
    dot_1.x -= (hebrewdot.x - center.x) * 0.2;
    dot_1.y -= (hebrewdot.y - center.y) * 0.2;

    graphics_context_set_stroke_color(ctx, GColorOrange);
    graphics_context_set_stroke_width(ctx, 2);
    graphics_draw_line(ctx, dot_0, dot_1);

    graphics_context_set_fill_color(ctx, GColorOrange);
    graphics_fill_circle(ctx, center, 3);
    graphics_context_set_fill_color(ctx, GColorOrange);
    graphics_fill_circle(ctx, center, 1);
  }
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);
  center = grect_center_point(&window_bounds);
  win_h = window_bounds.size.h;
  win_w = window_bounds.size.w;
  
  radius = win_h < win_w ? win_h : win_w;
  radius /= 2;
  radius -= 2;
  
  panel_layer = layer_create(window_bounds);
  layer_set_update_proc(panel_layer, panel_update_handler);
  hand_layer = layer_create(window_bounds);
  layer_set_update_proc(hand_layer, hand_update_handler);
  
  layer_add_child(window_layer, panel_layer);
  layer_add_child(window_layer, hand_layer);
}

static void window_unload(Window *window) {
  layer_destroy(panel_layer);
  layer_destroy(hand_layer);
}

static void hand_tick_handler(struct tm *tick_time, TimeUnits changed) {
  nowhours = tick_time->tm_hour;
  nowmins = tick_time->tm_min;
  
  APP_LOG(APP_LOG_LEVEL_INFO, "%d, %d", risemins, setmins);
  
  if (hand_layer) {
    layer_mark_dirty(hand_layer);
  }
}

static void sync_changed_handler(const uint32_t key, const Tuple *new_tuple, const Tuple *old_tuple, void *context) {
  if (new_tuple->key == RISEMINS) {
    risemins = new_tuple->value->int32;
  } else if (new_tuple->key == SETMINS) {
    setmins = new_tuple->value->int32;
  }
}

static void sync_error_handler(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  // An error occured!
}

static void init() {
  //srand(time(NULL));
  
  main_window = window_create();
  window_set_window_handlers(main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });
  window_stack_push(main_window, true);
  
  tick_timer_service_subscribe(MINUTE_UNIT, hand_tick_handler);
  
  Tuplet init_vals[] = 
    { TupletInteger(RISEMINS, 0),
      TupletInteger(SETMINS, 0) };
  app_message_open(app_message_inbox_size_maximum(),
                   app_message_outbox_size_maximum());
  app_sync_init(&sync, sync_buffer, sizeof(sync_buffer),
                init_vals, ARRAY_LENGTH(init_vals),
                sync_changed_handler, sync_error_handler, NULL);
}

static void deinit() {
  window_destroy(main_window);
  app_sync_deinit(&sync);
}

int main() {
  init();
  app_event_loop();
  deinit();
}