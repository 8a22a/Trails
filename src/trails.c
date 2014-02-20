#include <pebble.h>
		
Window *window;

BitmapLayer *background_layer;
RotBitmapLayer *minute_hand_layer, *hour_hand_layer;

GBitmap *background_image, *minute_hand_image, *hour_hand_image;

void update_watch(struct tm *t){

  long minute_hand_rotation = TRIG_MAX_ANGLE * (t->tm_min * 6) / 360;
  long hour_hand_rotation = TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 30) +
					      (t->tm_min/2)) / 360;
  rot_bitmap_layer_set_angle(minute_hand_layer, minute_hand_rotation);
  rot_bitmap_layer_set_angle(hour_hand_layer, hour_hand_rotation);

  layer_mark_dirty(bitmap_layer_get_layer((BitmapLayer *)minute_hand_layer));
  layer_mark_dirty(bitmap_layer_get_layer((BitmapLayer *)hour_hand_layer));

}

// Called once per second
void handle_minute_tick(struct tm *t, TimeUnits units_changed) {
  (void)units_changed;
  update_watch(t);
}


// Handle the start-up of the app
void handle_init() {

  // Create our app's base window
  window = window_create();
  Layer *root_window_layer = window_get_root_layer(window);
  GRect root_window_bounds = layer_get_bounds(root_window_layer);

  window_stack_push(window, true);
  window_set_background_color(window, GColorBlack);

  // Set up a layer for the static watch face background
  background_layer = bitmap_layer_create(root_window_bounds);
  background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  bitmap_layer_set_bitmap(background_layer, background_image);
  layer_add_child(root_window_layer,
		  bitmap_layer_get_layer(background_layer));

  // Set up a layer for the minute hand.
  // Compositing tricks take the place of PNG transparency.
  minute_hand_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MINUTE_TRAIL);
  minute_hand_layer = rot_bitmap_layer_create(minute_hand_image);
  // Default frame for this RotBitmapLayer is GRect(0, 0, 185, 185)
  layer_set_frame(bitmap_layer_get_layer((BitmapLayer *)minute_hand_layer),
		  GRect(-20, -9, 185, 185));
  rot_bitmap_set_compositing_mode(minute_hand_layer, GCompOpOr);
  layer_add_child(root_window_layer,
		  bitmap_layer_get_layer((BitmapLayer *)minute_hand_layer));

  // Set up a layer for the hour hand.
  // Compositing tricks take the place of PNG transparency.
  hour_hand_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_HOUR_TRAIL);
  hour_hand_layer = rot_bitmap_layer_create(hour_hand_image);

  // Default frame for this RotBitmapLayer is GRect(0, 0, 151, 151).
  // It's offset one pixel higher and righter than I'd expect
  // to fit in with the minute hand.
  layer_set_frame(bitmap_layer_get_layer((BitmapLayer *)hour_hand_layer),
		  GRect(-3, 7, 151, 151));
  rot_bitmap_set_compositing_mode(hour_hand_layer, GCompOpOr);
  layer_add_child(root_window_layer,
		  bitmap_layer_get_layer((BitmapLayer *)hour_hand_layer));

  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  update_watch(t);

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);

}

void handle_deinit() {

  // Fairly sure casting a RotBitmapLayer to BitmapLayer won't
  // fully destroy it, but it's the best we can do. :-(
  bitmap_layer_destroy((BitmapLayer *)hour_hand_layer);
  gbitmap_destroy(hour_hand_image);
  bitmap_layer_destroy((BitmapLayer *)minute_hand_layer);
  gbitmap_destroy(minute_hand_image);
  gbitmap_destroy(background_image);
  bitmap_layer_destroy(background_layer);
  window_destroy(window);

}


int main(void) {

  handle_init();
  app_event_loop();
  handle_deinit();

}
