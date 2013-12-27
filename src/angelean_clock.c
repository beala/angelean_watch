#include <stdio.h>
#include <string.h>

#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#define MY_UUID { 0x2E, 0x26, 0xF2, 0x85, 0xA3, 0xB6, 0x47, 0xD0, 0x9D, 0xC9, 0x6D, 0x0F, 0x63, 0xDB, 0xB3, 0xDD }
PBL_APP_INFO(MY_UUID,
             "Angelean Clock", "Alex Beal www.usrsb.in @beala",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;
TextLayer time_layer;
char display_str[8] = ""; // Buffer for the time string.

typedef struct AngTm {
  int day;
  int mday; /*milliday*/
} AngTm;

/**
 * Return true of year is a leap year, else false.
 */
int is_leap_year(int year) {
  if(year % 4 == 0) {
    if(year % 100 == 0) {
      if(year % 400) {
	return true;
      } else {
	return false;
      }
    } else {
      return true;
    }
  } else {
    return false;
  }
} 

/**
 * Get the current angelean time.
 */
void get_ang_time(AngTm *at) {
  PblTm pt;
  get_time(&pt);

  int days_in_year;
  if(pt.tm_yday + 11 == 365 && is_leap_year(pt.tm_year)) {
    days_in_year = 366;
  } else {
    days_in_year = 365;
  }
 
  at->day = (pt.tm_yday + 11) % days_in_year;
  at->mday = (((pt.tm_hour - 6) % 24)*60*60 + pt.tm_min * 60 + pt.tm_sec)/(24.*60.*60.)*1000.;
}

/**
 * Integer exponentiation for b >= 0 and e >= 0
 */
int powi(int b, int e) {
  int result = 1;
  int i;

  for(i=e; i>0; i--) {
    result *= b;
  }
  return result;
}

/**
 * Convert a positive integer to a string. If the string is less than *padding* chars long
 * then left pad with 0s. Does no bounds checking. Will end string with a null char.
 **/
void itoa(int n, char *str, int padding) {
  int n_buf = n;
  int digits = 1;
  int i, j, cur_digit;

  // Count the number of digits.
  while((n_buf /= 10) > 0) {
    digits++;
  }

  // Left pad
  j=0;
  for(i = padding-digits; i>0; i--) {
    str[j] = '0';
    j++;
  }

  // Convert, starting with the leftmost digit.
  n_buf = n;
  for(i=digits-1; i>=0; i--) {
    cur_digit = n_buf/powi(10, i);
    str[j] = cur_digit + '0';
    n_buf -= cur_digit * powi(10, i);
    j++;
  }  

  // Terminate string
  str[j] = '\0';
}

/**
 * Given the angelean time, write a formatted string to *str*
 * DDD
 * MMM
 * Where D is the day and M is the milliday.
 **/
void format_ang_time(AngTm *at, char *str) {
  snprintf(str, 8, "%03d\n%03d", at->day, at->mday);
}

int counter = 0;
/**
 * Write the current angelean time to the screen by updating
 * the layer and marking it as dirty.
 **/
void display_time() {
  AngTm at;

  get_ang_time(&at);
  format_ang_time(&at, display_str);
  text_layer_set_text(&time_layer, display_str);
  layer_mark_dirty((Layer *) &time_layer);
}

void handle_init(AppContextRef ctx) {
  window_init(&window, "Angelean Calendar");
  window_stack_push(&window, true /* Animated */);
  window_set_background_color(&window, GColorBlack);

  text_layer_init(&time_layer, GRect(0, 30, 144, 168));
  text_layer_set_text_alignment(&time_layer, GTextAlignmentCenter);
  text_layer_set_font(&time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  text_layer_set_background_color(&time_layer, GColorBlack);
  text_layer_set_text_color(&time_layer, GColorWhite);
  layer_add_child(&window.layer, &time_layer.layer);
  display_time();
}

void handle_tick(AppContextRef ctx, PebbleTickEvent *t) {
  display_time();
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .tick_info = {
      .tick_handler  = &handle_tick,
      .tick_units = SECOND_UNIT
    }
  };
  app_event_loop(params, &handlers);
}
