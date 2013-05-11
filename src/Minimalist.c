#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#include "bitmap.h"
#include "Minimalist.h"

#define MY_UUID { 0x3A, 0xE8, 0xC5, 0x47, 0xC9, 0xBB, 0x4E, 0x7E, 0xA9, 0x63, 0xDC, 0x68, 0x79, 0x77, 0x70, 0xD0 }
PBL_APP_INFO(MY_UUID,
	 "Minimalist", "Jnm",
	 1, 2, /* App version */
	 RESOURCE_ID_IMAGE_MENU_ICON,
	 APP_INFO_WATCH_FACE);

#define DRAW_CIRCLE true
#define DIGIT_SIZE 20
#define DIGIT_SPACE 2

#define SCREENW 144
#define SCREENH 168
#define CX 72
#define CY 84

#define NUM_IMAGES 10
const int digitId[NUM_IMAGES] = {
	RESOURCE_ID_IMAGE_0, RESOURCE_ID_IMAGE_1, RESOURCE_ID_IMAGE_2, RESOURCE_ID_IMAGE_3,
	RESOURCE_ID_IMAGE_4, RESOURCE_ID_IMAGE_5, RESOURCE_ID_IMAGE_6, RESOURCE_ID_IMAGE_7,
	RESOURCE_ID_IMAGE_8, RESOURCE_ID_IMAGE_9
};

HeapBitmap digitBmp[NUM_IMAGES];
Window window;
Layer layer;
bool clock12;

void update_display(Layer *layer, GContext *ctx) {
	static int y = CY-DIGIT_SIZE/2;
	static GPoint center = { CX, CY };
	int i, a, digit[4], x;
	PblTm now;

	get_time(&now);

	if (clock12) {
		now.tm_hour = now.tm_hour%12;
		if (now.tm_hour == 0) now.tm_hour = 12;
	}
 
	digit[0] = now.tm_hour/10;
	digit[1] = now.tm_hour%10;
	digit[2] = now.tm_min/10;
	digit[3] = now.tm_min%10;
	
	bmpFillRect(&bitmap, GRect(0,CY-DIGIT_SIZE/2,SCREENW,DIGIT_SIZE), GColorBlack);

	if (now.tm_min < 30) {
		a = 6*(now.tm_min-15);
	} else {
		a = 6*(now.tm_min-45);
	}

	for (i=0; i<4; i++) {
		if (i != 0 || digit[i] != 0) {
			if (now.tm_min < 30) {
				x = CX-DIGIT_SIZE+(DIGIT_SIZE+DIGIT_SPACE)*i+(DIGIT_SPACE*(i>1));
			} else {
				if (digit[0] == 0) {
					x = CX+DIGIT_SPACE+1+DIGIT_SIZE-(DIGIT_SIZE+DIGIT_SPACE)*(5-i)-(DIGIT_SPACE*(i<2));
				} else {
					x = CX+DIGIT_SPACE+1+DIGIT_SIZE-(DIGIT_SIZE+DIGIT_SPACE)*(4-i)-(DIGIT_SPACE*(i<2));
				}
			}
			bmpSub(&digitBmp[digit[i]].bmp, &bitmap, digitBmp[digit[i]].bmp.bounds, GPoint(x, y));
		}
	}

	bmpRotate(&bitmap, &bitmap2, a, GPoint(CX,CY), GPoint(0,0));
#if DRAW_CIRCLE
	bmpDrawCircle(&bitmap2, center, SCREENW/2-2, GColorWhite);
	bmpDrawCircle(&bitmap2, center, SCREENW/2-1, GColorWhite);
#endif
	graphics_draw_bitmap_in_rect(ctx, &bitmap2, GRect(0,0,SCREENW,SCREENH));
}

void handle_tick(AppContextRef ctx, PebbleTickEvent *e) {
	layer_mark_dirty(&layer);
}


void handle_init(AppContextRef ctx) {
	Layer *rootLayer;
	int i;

	window_init(&window, "Minimalist");
	window_stack_push(&window, true /* Animated */);
	window_set_background_color(&window, GColorBlack);

	clock12 = !clock_is_24h_style();

	resource_init_current_app(&APP_RESOURCES);

	for (i=0; i<NUM_IMAGES; i++) {
		heap_bitmap_init(&digitBmp[i], digitId[i]);
	}

	rootLayer = window_get_root_layer(&window);
	layer_init(&layer, GRect(0,0,SCREENW,SCREENH));
	layer_set_update_proc(&layer, &update_display);
	layer_add_child(rootLayer, &layer);

	layer_mark_dirty(&layer);
}

void handle_deinit(AppContextRef ctx) {
	int i;
	for (i=0; i<NUM_IMAGES; i++) {
		heap_bitmap_deinit(&digitBmp[i]);
	}
}

void pbl_main(void *params) {
	PebbleAppHandlers handlers = {
		.init_handler = &handle_init,
		.deinit_handler = &handle_deinit,

		.tick_info = {
			.tick_handler = &handle_tick,
			.tick_units   = MINUTE_UNIT
		}
	};
	app_event_loop(params, &handlers);
}
