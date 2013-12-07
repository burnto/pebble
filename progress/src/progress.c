// Progress

#include "pebble.h"
#include <math.h>

Window *window;

Layer *display_layer;

void draw_horiz_line(GContext *ctx, GRect frame, int y) {
  GPoint p0 = {frame.origin.x, frame.origin.y + y};
  GPoint p1 = {frame.origin.x + frame.size.w, frame.origin.y + y};
  graphics_draw_line(ctx, p0, p1);
}

void display_layer_update_callback(Layer *me, GContext* ctx) {

  GRect frame = layer_get_frame(me);

  time_t now = time(NULL);

  struct tm *t = localtime(&now);

  graphics_context_set_stroke_color(ctx, GColorWhite);

  int hours = t->tm_hour % 12;

  float hrHeight = frame.size.h / 12.0f;
  float minuteWidth = frame.size.w / 60.0f;

  // Draw hours elapsed

  GRect hrRect = frame;
  hrRect.size.h = hrHeight * hours;
  graphics_fill_rect(ctx, hrRect, 0, GCornerNone);

  // Draw minutes elapsed
  GRect elMinutesR;
  elMinutesR.origin = frame.origin;
  elMinutesR.origin.y = hrRect.size.h + hrRect.origin.y;
  elMinutesR.size.w = floor(minuteWidth * t->tm_min);
  elMinutesR.size.h = ceil(hrHeight);
  graphics_fill_rect(ctx, elMinutesR, 0, GCornerNone);

  // Draw blinking current minute
  GRect currMinuteR = elMinutesR;
  currMinuteR.origin.x = elMinutesR.size.w + elMinutesR.origin.x + 1;
  currMinuteR.size.w = ceil(minuteWidth);
  GColor fillColor = (t->tm_sec % 2 == 0) ? GColorWhite : GColorBlack;
  graphics_context_set_fill_color(ctx, fillColor);
  graphics_fill_rect(ctx, currMinuteR, 0, GCornerNone);

  // Draw hour intervals
  for (int hr = 0; hr <= hours; hr++) {
    draw_horiz_line(ctx, frame, hr * hrHeight);
  }

}

void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(display_layer);
}


static void do_init(void) {
  window = window_create();
  window_stack_push(window, true);

  window_set_background_color(window, GColorWhite);

  Layer *root_layer = window_get_root_layer(window);
  GRect frame = layer_get_frame(root_layer);

  // Init the layer for the display
  display_layer = layer_create(frame);
  layer_set_update_proc(display_layer, &display_layer_update_callback);
  layer_add_child(root_layer, display_layer);

  tick_timer_service_subscribe(SECOND_UNIT, &handle_second_tick);
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
