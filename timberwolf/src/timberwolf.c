#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

// 58BBF9E3-3471-4F9D-B56E-8044605D81E7
#define MY_UUID { 0x58, 0xBB, 0xF9, 0xE3, 0x34, 0x71, 0x4F, 0x9D, 0xB5, 0x6E, 0x80, 0x44, 0x60, 0x5D, 0x81, 0xE7 }
PBL_APP_INFO( MY_UUID,
              "Timberwolf", "Raptor007",
              1, 0, /* App major/minor version */
              RESOURCE_ID_IMAGE_MENU_ICON,
              APP_INFO_WATCH_FACE );

Window window;
BmpContainer background_image;
TextLayer time_layer;
TextLayer date_layer;
TextLayer wday_layer;
TextLayer ampm_layer;

char time_text[] = "00:00:00";
char ampm_text[] = "  ";
char date_text[] = "YYYY-MM-DD";
char wday_text[] = "      ";


void update_display( PblTm *current_time );
void handle_tick( AppContextRef ctx, PebbleTickEvent *t );
void handle_init( AppContextRef ctx );
void handle_deinit( AppContextRef ctx );
void pbl_main( void *params );


void update_display( PblTm *current_time )
{
	if( clock_is_24h_style() )
	{
		string_format_time( time_text, sizeof(time_text), "%T", current_time );
		ampm_text[ 0 ] = '\0';
	}
	else
	{
		string_format_time( time_text, sizeof(time_text), "%I:%M:%S", current_time );
		string_format_time( ampm_text, sizeof(ampm_text), "%p", current_time );
	}

	string_format_time( date_text, sizeof(date_text), "%Y-%m-%d", current_time );

	switch( current_time->tm_wday )
	{
		// Since the font is not monospaced and we can't align right, add spaces.
		
		case 0: // Sun
			string_format_time( wday_text, sizeof(wday_text), " %a", current_time );
			break;
		case 2: // Tue
		case 4: // Thu
		case 6: // Sat
			string_format_time( wday_text, sizeof(wday_text), "  %a", current_time );
			break;
		case 5: // Fri
			string_format_time( wday_text, sizeof(wday_text), "   %a", current_time );
			break;
		default:
			string_format_time( wday_text, sizeof(wday_text), "%a", current_time );
	}

	text_layer_set_text( &time_layer, time_text );
	text_layer_set_text( &date_layer, date_text );
	text_layer_set_text( &wday_layer, wday_text );
	text_layer_set_text( &ampm_layer, ampm_text );
}


void handle_tick( AppContextRef ctx, PebbleTickEvent *t )
{
	(void) ctx;

	update_display( t->tick_time );
}


void handle_init( AppContextRef ctx )
{
	(void) ctx;

	window_init( &window, "Timberwolf" );
	window_stack_push( &window, true );

	resource_init_current_app( &APP_RESOURCES );

	// Init the background image layer.
	bmp_init_container( RESOURCE_ID_IMAGE_BACKGROUND, &background_image );
	layer_add_child( &window.layer, &background_image.layer.layer );

	// Init the text layer used to show the time.
	text_layer_init( &time_layer, GRect( 2, -5, 144, 168 ) );
	text_layer_set_text_color( &time_layer, GColorBlack );
	text_layer_set_background_color( &time_layer, GColorClear );
	text_layer_set_font( &time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD) );
	layer_add_child( &window.layer, &time_layer.layer );

	// Init the text layer used to show the date.
	text_layer_init( &date_layer, GRect( 83, 0, 144, 168 ) );
	text_layer_set_text_color( &date_layer, GColorBlack );
	text_layer_set_background_color( &date_layer, GColorClear );
	text_layer_set_font( &date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14) );
	layer_add_child( &window.layer, &date_layer.layer );

	// Init the text layer used to show the weekday.
	text_layer_init( &wday_layer, GRect( 118, 13, 144, 168 ) );
	text_layer_set_text_color( &wday_layer, GColorBlack );
	text_layer_set_background_color( &wday_layer, GColorClear );
	text_layer_set_font( &wday_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14) );
	layer_add_child( &window.layer, &wday_layer.layer );

	// Init the text layer used to show AM/PM in 12-hour mode.
	text_layer_init( &ampm_layer, GRect( 2, 22, 144, 168 ) );
	text_layer_set_text_color( &ampm_layer, GColorBlack );
	text_layer_set_background_color( &ampm_layer, GColorClear );
	text_layer_set_font( &ampm_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14) );
	layer_add_child( &window.layer, &ampm_layer.layer );

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
