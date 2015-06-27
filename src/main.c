#include <pebble.h>
  
static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_battery_layer; //text layer for battery percentage
static TextLayer *s_date_layer; //Layer layer for current date 
static TextLayer *s_dow_layer; //Layer layer for current date
static TextLayer *s_disconnect_layer;
static GFont *s_time_font,*s_date_font,*s_battery_font;
static GBitmap *s_bg_bitmap,*s_hour_bitmap,*s_black_bitmap,*s_fade_bitmap;
static BitmapLayer *s_bg_layer,*s_hour_layer,*s_black_layer,*s_fade_layer;
int fade_sec =0; //set time since last fade to 0
int frame=0;
int f_delay =150;
int g_interval =18; //glitces every 'g_interval' seconds
bool vib_hour =true;
int last_hour =24; //that's not possible!
bool batflash =true;
bool wasDisconnected =false; //previouly disconnected? 
static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    //Use 2h hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    //Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
  
  // push the string to the layer
  //text_layer_set_text(s_time_layer, time);
  
  //--- DATE CODE---
  // Need to be static because they're used by the system later.
  static char s_date_text[] = "Xxxx\n 00";
  
  strftime(s_date_text, sizeof(s_date_text), "%b\n %e", tick_time);
 
  text_layer_set_text(s_date_layer, s_date_text);  
  
  //update day of week!
  static char week_day[] = "Xxx";
  strftime(week_day,sizeof(week_day),"%a",tick_time);
  text_layer_set_text(s_dow_layer,week_day);
}




  static void battery_handler(BatteryChargeState new_state) {
  // Write to buffer and display
  static char s_battery_buffer[32];  //char for the battery percent
  snprintf(s_battery_buffer, sizeof(s_battery_buffer), "%d%%", new_state.charge_percent); //get he battery info, and add the approprate text
  text_layer_set_text(s_battery_layer, s_battery_buffer); //set the layer battery text (percent + text) to the battery layer
  }


static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);
  
    //Create GFont for clock| Font:Tonik BRK Font [by Blambot Comic]
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_time_48));
  
  //create font for date/dow  |Font: Flipside BRK Font [by Ænigma Fonts]
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_date_24));
  
  //create font for Battery |Font: NinePin Font  [by Digital Graphic Labs]
  s_battery_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_battery_18));

  
  //Create background
  s_bg_bitmap = gbitmap_create_with_resource(RESOURCE_ID_bg);
  s_bg_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_bg_layer, s_bg_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bg_layer));
  
  // Create Time 
  s_time_layer = text_layer_create(GRect(1, -2, 148, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  #ifdef PBL_PLATFORM_APLITE //if running on older pebble
    text_layer_set_text_color(s_time_layer,GColorWhite); /// black text
  #elif PBL_PLATFORM_BASALT  //if running on pebble time
    text_layer_set_text_color(s_time_layer, GColorDarkCandyAppleRed);
  #endif
  text_layer_set_text(s_time_layer, "00:00");
  
  //create layer for background
  s_bg_bitmap = gbitmap_create_with_resource(RESOURCE_ID_bg);
  s_bg_layer = bitmap_layer_create(window_bounds);
  bitmap_layer_set_bitmap(s_bg_layer,s_bg_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bg_layer));
  
  //battery setup
  s_battery_layer = text_layer_create(GRect(20, 150, window_bounds.size.w, window_bounds.size.h));
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentLeft); //left align the text in this layer
  text_layer_set_background_color(s_battery_layer, GColorClear); //clear background
  #ifdef PBL_PLATFORM_APLITE //if running on older pebble
    text_layer_set_text_color(s_battery_layer, GColorWhite); /// black text
   #elif PBL_PLATFORM_BASALT  //if running on pebble time
    text_layer_set_text_color(s_battery_layer,GColorBulgarianRose); //red text
  #endif
  text_layer_set_font(s_battery_layer, s_battery_font); //change the font
  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));//Add battery layer to our window
 
  // Get the current battery level
  battery_handler(battery_state_service_peek());
  
  //Create Date layer:
  //s_date_layer = text_layer_create(GRect(20,49, 136, 100)); old
  s_date_layer = text_layer_create(GRect(78,120, 136, 100));
  text_layer_set_background_color(s_date_layer, GColorClear);
   #ifdef PBL_PLATFORM_APLITE //if running on older pebble
    text_layer_set_text_color(s_date_layer,GColorWhite); /// black text
   #elif PBL_PLATFORM_BASALT  //if running on pebble time
    text_layer_set_text_color(s_date_layer, GColorDarkCandyAppleRed); //blueish text
  #endif
  
  text_layer_set_font(s_date_layer,  s_date_font);
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  
  
  //Create 'DAY OF WEEK layer:
 // s_dow_layer = text_layer_create(GRect(83,100, 136, 100)); old
  s_dow_layer = text_layer_create(GRect(1,120, 136, 100));
  text_layer_set_background_color(s_dow_layer, GColorClear);
  #ifdef PBL_PLATFORM_APLITE //if running on older pebble
    text_layer_set_text_color(s_dow_layer,  GColorWhite); /// black text
  #elif PBL_PLATFORM_BASALT  //if running on pebble time
    text_layer_set_text_color(s_dow_layer, GColorDarkCandyAppleRed); //dark blue text
  #endif
  
  text_layer_set_font(s_dow_layer, s_date_font);
  layer_add_child(window_layer, text_layer_get_layer(s_dow_layer));
  
  //set up layer for special hour change
  s_hour_bitmap = gbitmap_create_with_resource(RESOURCE_ID_hour_changed_bg);
  s_hour_layer = bitmap_layer_create(window_bounds);
  bitmap_layer_set_bitmap(s_hour_layer,s_hour_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_hour_layer));
  layer_set_hidden( (Layer *)s_hour_layer,true); 


  //layer for bt disconect
  s_disconnect_layer = text_layer_create(GRect(15,140, 139, 50));
  text_layer_set_background_color(s_disconnect_layer, GColorClear);
  #ifdef PBL_PLATFORM_APLITE //if running on older pebble
    text_layer_set_text_color(s_disconnect_layer,  GColorWhite); /// black text
  #elif PBL_PLATFORM_BASALT  //if running on pebble time
    text_layer_set_text_color(s_disconnect_layer, GColorDarkCandyAppleRed); //dark blue text
  #endif
  text_layer_set_text(s_disconnect_layer, "-Connection Lost-");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_disconnect_layer));
  layer_set_hidden( (Layer *)s_disconnect_layer,true); //hide it.
  
  
    //create layer for fade
  s_fade_layer = bitmap_layer_create(window_bounds); 
  #ifdef PBL_PLATFORM_APLITE //if running on older pebble
  bitmap_layer_set_compositing_mode(s_fade_layer, GCompOpOr);
  s_fade_bitmap = gbitmap_create_with_resource(RESOURCE_ID_glitch_bw_WHITE);
  
  #elif PBL_PLATFORM_BASALT  //if running on pebble time
    bitmap_layer_set_compositing_mode(s_fade_layer, GCompOpSet);
    s_fade_bitmap = gbitmap_create_with_resource(RESOURCE_ID_glitch_color);
  #endif

  
  #ifdef PBL_PLATFORM_APLITE //classic pebble only, 
    //set up 'black portion
    s_black_bitmap = gbitmap_create_with_resource(RESOURCE_ID_glitch_bw_BLACK);
    s_black_layer = bitmap_layer_create(window_bounds);
    bitmap_layer_set_bitmap(s_black_layer, s_black_bitmap);
    bitmap_layer_set_compositing_mode(s_black_layer, GCompOpClear);      
    layer_add_child(window_layer, bitmap_layer_get_layer(s_black_layer));
  #endif
  
  bitmap_layer_set_bitmap(s_fade_layer,s_fade_bitmap);
  
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_fade_layer));
  layer_set_hidden( (Layer *)s_fade_layer,true); //hide it.
  #ifdef PBL_PLATFORM_APLITE
    layer_set_hidden( (Layer *)s_black_layer,true); 
  #endif

  //time
  //Apply to TextLayer
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  

  
  // Make sure the time is displayed from the start
  update_time();
}

static void main_window_unload(Window *window) {
  //Unload GFont
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_date_font);
  fonts_unload_custom_font(s_battery_font);
  
  //Destroy GBitmap
  gbitmap_destroy(s_bg_bitmap);
  gbitmap_destroy(s_fade_bitmap);
  gbitmap_destroy(s_black_bitmap);
  gbitmap_destroy(s_hour_bitmap);


  //Destroy BitmapLayer
  bitmap_layer_destroy(s_bg_layer);
  bitmap_layer_destroy( s_black_layer);
  bitmap_layer_destroy(s_fade_layer);
  bitmap_layer_destroy(s_hour_layer);
  
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_battery_layer);
  text_layer_destroy(s_dow_layer);
  text_layer_destroy(s_disconnect_layer);
}


void bg_glitch(void * data){  
    frame++;
    int last_frame = 12;  
    int delay = f_delay;
    if (frame == 1 ||frame == 3 || frame == 5 ||frame == 7 ||frame == 9 || frame == 11)
      {
      layer_set_hidden( (Layer *)s_fade_layer,false); //show it. 
        #ifdef PBL_PLATFORM_APLITE 
           layer_set_hidden( (Layer *)s_black_layer,false);  
        #endif
      }
    if (frame == 4 || frame == 6 ||frame == 8)
        {
        layer_set_hidden( (Layer *)s_fade_layer,true); //hide the fade layer
          #ifdef PBL_PLATFORM_APLITE 
            layer_set_hidden( (Layer *)s_black_layer,true);  
          #endif
      }
    if (frame != last_frame)
      app_timer_register(delay, bg_glitch, NULL);
  
    if (frame == last_frame){ //we're done! pack it up!
      layer_set_hidden( (Layer *)s_fade_layer,true); //hide the fade layer
      #ifdef PBL_PLATFORM_APLITE 
       layer_set_hidden( (Layer *)s_black_layer,true);  
      #endif
      frame =0; // Set frame back to zero. NOTE: THIS MUST BE THE LAS LINE IN THE FRAME COUNTER, otherwise: if (frame != last_frame) will be tripped.  
     }
}



static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    fade_sec ++;//time has passed since last blink
    int seconds = tick_time->tm_sec;
    int hours = tick_time ->tm_hour;
    int min =  tick_time ->tm_min;
  
    //Bluetooth Diconnect handler
    if (bluetooth_connection_service_peek() ==false && wasDisconnected==false){ //if we WERE connected, and now are not, then do the following
    layer_set_hidden( (Layer *) s_disconnect_layer,false);  //show it on the screen
    wasDisconnected = true; //at last check, we WERE disconnected, 
    vibes_double_pulse(); //double vibrate, so they will know it's not a normal notication.
    }
  
   if (bluetooth_connection_service_peek() ==true && wasDisconnected ==true){ //if we WERE DISconnected (last we checked), and now are connected again, then do the following
    layer_set_hidden( (Layer *) s_disconnect_layer,true);//hide the layer
    wasDisconnected = false;// at last check  (now) we WERE connected
    // layer_mark_dirty(bitmap_layer_get_layer( s_disconnected_layer)); 
   }
  
    if (last_hour == 24) //if at the last update the hour was 24, then we just started the watch
      last_hour=hours; //our new 'last hour' will be the current time
    
    if (hours != last_hour && batflash==true) 
  //if (seconds ==15 || seconds ==25 || seconds ==35) //DEBUG!
      {
        layer_set_hidden( (Layer *)s_hour_layer,false);  //i'm batman
        #ifdef PBL_PLATFORM_BASALT  //if running on pebble time
          text_layer_set_text_color(s_time_layer, GColorSunsetOrange);
         #endif
    
    }
    if(batflash==true && min==0 && seconds ==5)
  // if (seconds ==20 || seconds ==30 || seconds ==40) //debug
      {
        #ifdef PBL_PLATFORM_BASALT  //if running on pebble time
          text_layer_set_text_color(s_time_layer, GColorDarkCandyAppleRed);
         #endif
         layer_set_hidden( (Layer *)s_hour_layer,true);
    }
  
    if (hours != last_hour && vib_hour ==true) //if hour has changed
     {
        vibes_short_pulse(); //vibrate (short) 
        last_hour=hours;//our new 'last hour' will be the current time   
      }

  
   // if (min == 59 && seconds == 59)
     // app_timer_register(500, glitch_hour_ani, NULL); //start the hour glitch

   // if (seconds ==59)
   //  app_timer_register(500, glitch_min_ani, NULL); //start the min glitch
  
    if (seconds == 0)
      {
      update_time();
      battery_handler(battery_state_service_peek());
    }
    if( fade_sec == g_interval) 
    {
     app_timer_register(0, bg_glitch, NULL);
      fade_sec=0;//yo, we just blinked, check it!
    }
}
  
static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(SECOND_UNIT, (TickHandler) tick_handler);
  
}


static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
