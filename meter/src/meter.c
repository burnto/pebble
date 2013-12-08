// Meter

#include "pebble.h"
#include <math.h>

Window *window;

Layer *display_layer;

void display_layer_update_callback(Layer *me, GContext* ctx) {

  GRect frame = layer_get_frame(me);

  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  int hours = t->tm_hour % 12;
  int minutes = t->tm_min;

  int cols = 2;
  int rows = 12 / cols;
  int margin = 10;

  GSize s;
  s.w = (frame.size.w - (cols + 1) * margin) / cols;
  s.h = floor((frame.size.h - (rows + 1) * margin) / (float) rows);

  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_fill_color(ctx, GColorWhite);

  GRect r;
  r.size = s;
  float mInc = r.size.w / (float) 60;
  for (int x = 0; x < cols; x++) {
    r.origin.x = frame.origin.x + margin + x * (margin + s.w);
    for (int y = 0; y < rows; y++) {
      r.origin.y = frame.origin.y + margin + y * (margin + s.h);

      int i = 6 * x + (5 - y);
      // APP_LOG(APP_LOG_LEVEL_DEBUG, "%d, %d, %d, %d", x, y, i, hours);
      if (i < hours) {
        graphics_fill_rect(ctx, r, 0, GCornerNone);
      } else {
        graphics_draw_rect(ctx, r);
        if (i == hours) {
          GRect m = r;
          m.size.w = floor(minutes * mInc);
          graphics_fill_rect(ctx, m, 0, GCornerNone);
        }
      }
    }
  }
}

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(display_layer);
}


static void do_init(void) {
  window = window_create();
  window_stack_push(window, true);

  window_set_background_color(window, GColorBlack);

  Layer *root_layer = window_get_root_layer(window);
  GRect frame = layer_get_frame(root_layer);

  // Init the layer for the display
  display_layer = layer_create(frame);
  layer_set_update_proc(display_layer, &display_layer_update_callback);
  layer_add_child(root_layer, display_layer);

  tick_timer_service_subscribe(MINUTE_UNIT, &handle_minute_tick);
}

static void do_deinit(void) {
  layer_destroy(display_layer);
  window_destroy(window);
}


int main(void) {
  srand(time(NULL));
  do_init();
  app_event_loop();
  do_deinit();
}
