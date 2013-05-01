#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

// 2A17AFEA-F7B3-4655-8BF6-DC6ED8C20686
#define MY_UUID { 0x2A, 0x17, 0xAF, 0xEA, 0xF7, 0xB3, 0x46, 0x55, 0x8B, 0xF6, 0xDC, 0x6E, 0xD8, 0xC2, 0x06, 0x86 }
PBL_APP_INFO( MY_UUID,
              "iWatch (no secs)", "Raptor007",
              1, 0, /* App major/minor version */
              RESOURCE_ID_IMAGE_MENU_ICON,
              APP_INFO_WATCH_FACE );

Window window;
BmpContainer background_image;
TextLayer time_layer;
TextLayer date_layer;

char time_text[] = "00:00 xx ";
char date_text[] = "September 00, 0000\nSaturday";


void update_display( PblTm *current_time );
void handle_tick( AppContextRef ctx, PebbleTickEvent *t );
void handle_init( AppContextRef ctx );
void handle_deinit( AppContextRef ctx );
void pbl_main( void *params );


void update_display( PblTm *current_time )
{
	if( clock_is_24h_style() )
		string_format_time( time_text, sizeof(time_text), "    %H:%M", current_time );
	else
		string_format_time( time_text, sizeof(time_text), "%I:%M %p", current_time );

	string_format_time( date_text, sizeof(date_text), "%b %d, %Y%n%A", current_time );

	text_layer_set_text( &time_layer, time_text );
	text_layer_set_text( &date_layer, date_text );
}


void handle_tick( AppContextRef ctx, PebbleTickEvent *t )
{
	(void) ctx;

	update_display( t->tick_time );
}


void handle_init( AppContextRef ctx )
{
	(void) ctx;

	window_init( &window, "iWatch (no secs)" );
	window_stack_push( &window, true );

	resource_init_current_app( &APP_RESOURCES );

	// Init the background image layer.
	bmp_init_container( RESOURCE_ID_IMAGE_BACKGROUND, &background_image );
	layer_add_child( &window.layer, &background_image.layer.layer );

	// Init the text layer used to show the time.
	text_layer_init( &time_layer, GRect( 54, 1, 144, 168 ) );
	text_layer_set_text_color( &time_layer, GColorBlack );
	text_layer_set_background_color( &time_layer, GColorClear );
	text_layer_set_font( &time_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_CHICAGO_13)) );
	layer_add_child( &window.layer, &time_layer.layer );

	// Init the text layer used to show the date.
	text_layer_init( &date_layer, GRect( 9, 86, 144, 168 ) );
	text_layer_set_text_color( &date_layer, GColorBlack );
	text_layer_set_background_color( &date_layer, GColorClear );
	text_layer_set_font( &date_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GENEVA_12)) );
	layer_add_child( &window.layer, &date_layer.layer );

	// Avoid a blank screen on watch start.
	PblTm tick_time;
	get_time( &tick_time );
	update_display( &tick_time );
}


void handle_deinit( AppContextRef ctx )
{
	(void) ctx;

	bmp_deinit_container( &background_image );
}


void pbl_main( void *params )
{
	PebbleAppHandlers handlers =
	{
		.init_handler = &handle_init,
		.deinit_handler = &handle_deinit,

		.tick_info =
		{
			.tick_handler = &handle_tick,
			.tick_units = SECOND_UNIT
		}
	};
	
	app_event_loop( params, &handlers );
}
