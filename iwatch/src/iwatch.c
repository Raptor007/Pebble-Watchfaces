#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

// 3067D564-F8D2-45D8-BE71-45B9961D1BC8
#define MY_UUID { 0x30, 0x67, 0xD5, 0x64, 0xF8, 0xD2, 0x45, 0xD8, 0xBE, 0x71, 0x45, 0xB9, 0x96, 0x1D, 0x1B, 0xC8 }
PBL_APP_INFO( MY_UUID,
              "iWatch", "Raptor007",
              1, 0, /* App major/minor version */
              RESOURCE_ID_IMAGE_MENU_ICON,
              APP_INFO_WATCH_FACE );

Window window;
BmpContainer background_image;
TextLayer time_layer;

char time_text[] = "00:00:00";


void update_display( PblTm *current_time );
void handle_tick( AppContextRef ctx, PebbleTickEvent *t );
void handle_init( AppContextRef ctx );
void handle_deinit( AppContextRef ctx );
void pbl_main( void *params );


void update_display( PblTm *current_time )
{
	if( clock_is_24h_style() )
		string_format_time( time_text, sizeof(time_text), "%T", current_time );
	else
		string_format_time( time_text, sizeof(time_text), "%r", current_time );

	text_layer_set_text( &time_layer, time_text );
}


void handle_tick( AppContextRef ctx, PebbleTickEvent *t )
{
	(void) ctx;

	update_display( t->tick_time );
}


void handle_init( AppContextRef ctx )
{
	(void) ctx;

	window_init( &window, "iWatch" );
	window_stack_push( &window, true );

	resource_init_current_app( &APP_RESOURCES );

	// Init the background image layer.
	bmp_init_container( RESOURCE_ID_IMAGE_BACKGROUND, &background_image );
	layer_add_child( &window.layer, &background_image.layer.layer );

	// Init the text layer used to show the time.
	text_layer_init( &time_layer, GRect( 32, 0, 144, 168 ) );
	text_layer_set_text_color( &time_layer, GColorBlack );
	text_layer_set_background_color( &time_layer, GColorClear );
	text_layer_set_font( &time_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_CHICAGO_12)) );
	layer_add_child( &window.layer, &time_layer.layer );

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
