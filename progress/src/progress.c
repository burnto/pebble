// Progress

#include "pebble.h"
#include <math.h>

Window *window;

Layer *display_layer;

inline void draw_vert_segment(GContext *ctx, GPoint origin, int length) {
  GPoint end = origin;
  end.y += length;
  graphics_draw_line(ctx, origin, end);
}

inline void draw_vert_line(GContext *ctx, GRect frame, int x) {
  GPoint p0 = {frame.origin.y, frame.origin.x + x};
  draw_vert_segment(ctx, p0, frame.size.h);
}

inline void draw_horiz_segment(GContext *ctx, GPoint origin, int length) {
  GPoint end = origin;
  end.x += length;
  graphics_draw_line(ctx, origin, end);
}

inline void draw_horiz_line(GContext *ctx, GRect frame, int y) {
  GPoint p0 = {frame.origin.x, frame.origin.y + y};
  draw_horiz_segment(ctx, p0, frame.size.w);
}

void display_layer_update_callback(Layer *me, GContext* ctx) {

  GRect frame = layer_get_frame(me);

  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  int hours = t->tm_hour % 12;
  int minutes = t->tm_min;

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
  elMinutesR.size.w = floor(minuteWidth * minutes) - 1; // Leave room for a gap
  elMinutesR.size.h = ceil(hrHeight);
  graphics_fill_rect(ctx, elMinutesR, 0, GCornerNone);

  // Draw blinking current minute
  GRect currMinuteR = elMinutesR;
  currMinuteR.origin.x = elMinutesR.size.w + elMinutesR.origin.x + 1; // Aforementioned gap
  currMinuteR.size.w = ceil(minuteWidth);
  GColor fillColor = (t->tm_sec % 2 == 0) ? GColorWhite : GColorBlack;
  graphics_context_set_fill_color(ctx, fillColor);
  graphics_fill_rect(ctx, currMinuteR, 0, GCornerNone);

  // Hour interval lines
  graphics_context_set_stroke_color(ctx, GColorWhite);
  for (int hr = 0; hr <= hours; hr++) {
    draw_horiz_line(ctx, frame, hr * hrHeight);
  }

  graphics_context_set_text_color(ctx, GColorWhite);
  GFont font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SLKSCR_8));

  // Draw hour text in last hour bar
  char hour_text[] = "00";
  char *hour_disp_text = hour_text;
  strftime(hour_text, sizeof(hour_text), "%I", t);
  if (hour_text[0] == '0') {
    hour_disp_text += sizeof(char);
  }
  int hrY = (hrRect.origin.y + hrRect.size.h - hrHeight + 2);
  graphics_draw_text(ctx, hour_disp_text, font, GRect(1, hrY, frame.size.w, 8), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

  // Draw minute text in minute bar or to right of it, depending on space.
  char minute_text[] = "00";
  strftime(minute_text, sizeof(minute_text), "%M", t);
  int minuteY = hrY + hrHeight;
  int minuteX = hrRect.origin.x + 1;

  GSize minRenderSize = graphics_text_layout_get_max_used_size(ctx, minute_text, font, GRect(0, 0, 1000, 1000), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
  if (minRenderSize.w > elMinutesR.size.w - 2) {
    minuteX += currMinuteR.origin.x + currMinuteR.size.w;
    graphics_context_set_text_color(ctx, GColorBlack);
  }
  graphics_draw_text(ctx, minute_text, font, GRect(minuteX, minuteY, frame.size.w, 8), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);


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
