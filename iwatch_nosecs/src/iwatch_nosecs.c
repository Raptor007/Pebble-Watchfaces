// iWatch by Raptor007
// v2.0.1 for Pebble SDK 2.x

#include <pebble.h>

Window *window = NULL;
GBitmap *background_image = NULL;
BitmapLayer *background_layer = NULL;
TextLayer *time_layer = NULL;
TextLayer *date_layer = NULL;

char time_text[] = "00:00 xx";
char date_text[] = "September 00, 0000\nWednesday";


void handle_init( void );
void handle_deinit( void );
void handle_tick( struct tm *t, TimeUnits units_changed );
void update_display( const struct tm *current_time );


int main( void )
{
	handle_init();
	app_event_loop();
	handle_deinit();
}


void handle_init( void )
{
	// Create the window, add it to the stack, and get its boundaries.
	window = window_create();
	window_stack_push( window, true );
	GRect window_bounds = layer_get_bounds( window_get_root_layer(window) );
	
	// Init the background image layer.
	background_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BACKGROUND );
	background_layer = bitmap_layer_create( window_bounds );
	bitmap_layer_set_bitmap( background_layer, background_image );
	layer_add_child( window_get_root_layer(window), bitmap_layer_get_layer(background_layer) );
	
	// Init the text layer used to show the time.
	time_layer = text_layer_create( GRect( 54, 1, 144, 168 ) );
	text_layer_set_text_color( time_layer, GColorBlack );
	text_layer_set_background_color( time_layer, GColorClear );
	text_layer_set_font( time_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_CHICAGO_13)) );
	layer_add_child( window_get_root_layer(window), text_layer_get_layer(time_layer) );
	
	// Init the text layer used to show the date.
	date_layer = text_layer_create( GRect( 9, 86, 144, 168 ) );
	text_layer_set_text_color( date_layer, GColorBlack );
	text_layer_set_background_color( date_layer, GColorClear );
	text_layer_set_font( date_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GENEVA_12)) );
	layer_add_child( window_get_root_layer(window), text_layer_get_layer(date_layer) );
	
	// Avoid a blank screen on watch start.
	time_t t = time( NULL );
	struct tm *tick_time = localtime( &t );
	update_display( tick_time );
	
	// Request an update signal every minute.
	tick_timer_service_subscribe( MINUTE_UNIT, handle_tick );
}


void handle_deinit( void )
{
	text_layer_destroy( date_layer );
	text_layer_destroy( time_layer );
	gbitmap_destroy( background_image );
	bitmap_layer_destroy( background_layer );
	window_destroy( window );
}


void handle_tick( struct tm *t, TimeUnits units_changed )
{
	(void) units_changed;
	
	update_display( t );
}


void update_display( const struct tm *current_time )
{
	if( clock_is_24h_style() )
		strftime( time_text, sizeof(time_text), "   %H:%M", current_time );
	else
		strftime( time_text, sizeof(time_text), "%I:%M%p", current_time );
	
	strftime( date_text, sizeof(date_text), "%b %d, %Y%n%A", current_time );
	
	text_layer_set_text( time_layer, time_text );
	text_layer_set_text( date_layer, date_text );
}
