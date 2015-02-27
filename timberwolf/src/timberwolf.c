// Timberwolf by Raptor007
// v2.0.0 for Pebble SDK 2.x

#include <pebble.h>

Window *window = NULL;
GBitmap *background_image = NULL;
BitmapLayer *background_layer = NULL;
TextLayer *time_layer = NULL;
TextLayer *date_layer = NULL;
TextLayer *wday_layer = NULL;
TextLayer *ampm_layer = NULL;

char time_text[] = "00:00:00";
char ampm_text[] = "  ";
char date_text[] = "YYYY-MM-DD";
char wday_text[] = "      ";


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
	time_layer = text_layer_create( GRect( 2, -5, 144, 168 ) );
	text_layer_set_text_color( time_layer, GColorBlack );
	text_layer_set_background_color( time_layer, GColorClear );
	text_layer_set_font( time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD) );
	layer_add_child( window_get_root_layer(window), text_layer_get_layer(time_layer) );
	
	// Init the text layer used to show the date.
	date_layer = text_layer_create( GRect( 83, 0, 144, 168 ) );
	text_layer_set_text_color( date_layer, GColorBlack );
	text_layer_set_background_color( date_layer, GColorClear );
	text_layer_set_font( date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14) );
	layer_add_child( window_get_root_layer(window), text_layer_get_layer(date_layer) );
	
	// Init the text layer used to show the weekday.
	wday_layer = text_layer_create( GRect( 118, 13, 144, 168 ) );
	text_layer_set_text_color( wday_layer, GColorBlack );
	text_layer_set_background_color( wday_layer, GColorClear );
	text_layer_set_font( wday_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14) );
	layer_add_child( window_get_root_layer(window), text_layer_get_layer(wday_layer) );
	
	// Init the text layer used to show AM/PM in 12-hour mode.
	ampm_layer = text_layer_create( GRect( 2, 22, 144, 168 ) );
	text_layer_set_text_color( ampm_layer, GColorBlack );
	text_layer_set_background_color( ampm_layer, GColorClear );
	text_layer_set_font( ampm_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14) );
	layer_add_child( window_get_root_layer(window), text_layer_get_layer(ampm_layer) );
	
	// Avoid a blank screen on watch start.
	time_t t = time( NULL );
	struct tm *tick_time = localtime( &t );
	update_display( tick_time );
	
	// Request an update signal every second.
	tick_timer_service_subscribe( SECOND_UNIT, handle_tick );
}


void handle_deinit( void )
{
	text_layer_destroy( date_layer );
	text_layer_destroy( time_layer );
	text_layer_destroy( wday_layer );
	text_layer_destroy( ampm_layer );
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
	{
		strftime( time_text, sizeof(time_text), "%T", current_time );
		ampm_text[ 0 ] = '\0';
	}
	else
	{
		strftime( time_text, sizeof(time_text), "%I:%M:%S", current_time );
		strftime( ampm_text, sizeof(ampm_text), "%p", current_time );
	}
	
	strftime( date_text, sizeof(date_text), "%Y-%m-%d", current_time );
	
	switch( current_time->tm_wday )
	{
		// Since the font is not monospaced and we can't align right, add spaces.
		
		case 0: // Sun
			strftime( wday_text, sizeof(wday_text), " %a", current_time );
			break;
		case 2: // Tue
		case 4: // Thu
		case 6: // Sat
			strftime( wday_text, sizeof(wday_text), "  %a", current_time );
			break;
		case 5: // Fri
			strftime( wday_text, sizeof(wday_text), "   %a", current_time );
			break;
		default:
			strftime( wday_text, sizeof(wday_text), "%a", current_time );
	}
	
	text_layer_set_text( time_layer, time_text );
	text_layer_set_text( date_layer, date_text );
	text_layer_set_text( wday_layer, wday_text );
	text_layer_set_text( ampm_layer, ampm_text );
}
