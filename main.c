


#include "config.h"

#include "allegro.h"

//#include <string.h>
#include <math.h>

#include "globvars.h"
#include "stuff.h"
#include "game.h"
#include "display_init.h"
#include "sound.h"
#include "menu.h"
#include "input.h"
#include "debug.h"

// timer interupt functions and variables:
void framecount(void);

volatile int framecounter;
volatile int frames_per_second;

void tickover(void);

volatile unsigned char ticked;
int slacktime;

void init_at_startup(void);
void begin_game(void);


struct playerstruct player [2];
struct arenastruct arena;
struct gamestruct game;
struct starstruct star [2] [NO_STARS];
struct cloudstruct cloud [NO_CLOUDS];
struct bulletstruct bullet [NO_TEAMS] [NO_BULLETS];
struct optionstruct options;
struct shipstruct ship [NO_TEAMS] [NO_SHIPS];
struct convoystruct convoy [NO_CONVOYS];
struct commstruct comm [COMMS];
struct linestruct ctext [COMMS] [LINES];

int glob;

void framecount(void)
{
   frames_per_second = framecounter;
   framecounter = 0;
}
END_OF_FUNCTION (framecount);

/*
Is called every 25ms; lets the game know that it's time to move on to the next frame.
*/
void tickover(void)
{
 ticked ++;
}
END_OF_FUNCTION (tickover);



int main(void)
{

/*
The following code is for use if your system is set up to prevent an application writing to its own directory.
Normally this will prevent highscores and key configurations being saved, but the following code puts the
initfile in a better place. This may be a default on some systems (eg Unixes and possibly Windows Vista)
and can be set on some other systems.

The only problem with this is that you'll have to manually type or copy the various other options (windowed,
vsync, joystick buttons etc) into the initfile in this new location (or just copy the initfile across, or
just rely on the default values and let the game make a new initfile).

Thanks to Thomas Harte for this code! I had no idea it was necessary (it isn't on my Windows XP computer, and
I haven't tested it).

*/




int allint =  allegro_init();
   if (allint == -1)
   {
      set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
      allegro_message("Failed to initialise Allegro! This isn't going to work, sorry.");
      exit(1);
   }

//#define UNIX_OSX_VISTA_ETC

#ifdef UNIX_OSX_VISTA_ETC

   {

//     char *HPath = getenv("HOME");

// use this if for some reason you're running Windows Vista:
        char *HPath = getenv("APPDATA");

	 char ConfigPath[2048];

	 sprintf(ConfigPath, "%s/.AMoth", HPath);
	 set_config_file(ConfigPath);

   }
#else
   set_config_file("init.txt");
#endif

   install_keyboard();
   install_timer();

   three_finger_flag = 0;
   key_led_flag = 0;

 init_at_startup();

// now we've initialised, let's play the game!
// game_loop();
 startup_menu();

 return 0;

}

END_OF_MAIN();

/*
Self-explanatory.
*/
void init_at_startup(void)
{

   LOCK_FUNCTION (framecount);
   LOCK_FUNCTION (tickover);
   LOCK_VARIABLE (ticked);
   LOCK_VARIABLE (frames_per_second);
   LOCK_VARIABLE (framecounter);
   LOCK_VARIABLE (turns_per_second);
   LOCK_VARIABLE (turncounter);

   install_int (framecount, 1000);
   install_int (tickover, 20);

   set_color_depth(8);

   int randseed = get_config_int("Misc", "Seed", 0);
   srand(randseed);

   options.windowed = get_config_int("Misc", "Windowed", 0);


 int windowed;
 switch(options.windowed)
 {
    default:
     case 1: windowed = GFX_AUTODETECT_WINDOWED; break;
     case 0: windowed = GFX_AUTODETECT_FULLSCREEN; break;
 }

//   windowed  = GFX_AUTODETECT_WINDOWED;
//   windowed = GFX_AUTODETECT_FULLSCREEN;

//   if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, 640, 480, 0, 0) != 0)
//   if (set_gfx_mode(windowed, 640, 480, 0, 0) != 0)
   if (set_gfx_mode(windowed, 800, 600, 0, 0) != 0)
   {
      set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
      allegro_message("Unable to set 800x600 256 colour display mode. Sorry!\n%s\n", allegro_error);
      exit(1);
   }

 init_trig();

 prepare_display();

 options.sound_init = get_config_int("Misc", "SoundInit", 0);
 options.music = get_config_int("Misc", "Music", 0);
 init_sound();

/*
 PP.ckey [CKEY_UP] = KEY_UP;
 PP.ckey [CKEY_DOWN] = KEY_DOWN;
 PP.ckey [CKEY_LEFT] = KEY_LEFT;
 PP.ckey [CKEY_RIGHT] = KEY_RIGHT;
 PP.ckey [CKEY_FIRE1] = KEY_Z;
 PP.ckey [CKEY_FIRE2] = KEY_X;
 PP.ckey [CKEY_FIRE3] = KEY_C;
*/
 int p = 0;

 init_joystick();

 player[0].control = get_config_int("misc", "player1_control", CONTROL_KEY_A);
 player[1].control = get_config_int("misc", "player2_control", CONTROL_KEY_B);

// if a particular joystick isn't available, default to keyboard controls.
//  note that this doesn't set the config values
 for (p = 0; p < 2; p ++)
 {
  if (options.joystick_available [0] == 0
   && PP.control == CONTROL_JOY_A)
   {
    PP.control = CONTROL_KEY_A;
    if (PP.control == CONTROL_KEY_A)
     PP.control = CONTROL_KEY_B;
   }
  if (options.joystick_available [1] == 0
   && PP.control == CONTROL_JOY_B)
   {
    PP.control = CONTROL_KEY_A;
    if (PP.control == CONTROL_KEY_A)
     PP.control = CONTROL_KEY_B;
   }
 }

 options.ckey [0] [CKEY_UP] = get_config_int("Misc", "key1_up", KEY_8_PAD);
 options.ckey [0] [CKEY_DOWN] = get_config_int("Misc", "key1_down", KEY_2_PAD);
 options.ckey [0] [CKEY_LEFT] = get_config_int("Misc", "key1_left", KEY_4_PAD);
 options.ckey [0] [CKEY_RIGHT] = get_config_int("Misc", "key1_right", KEY_6_PAD);
 options.ckey [0] [CKEY_LEFT2] = get_config_int("Misc", "key1_left2", KEY_1_PAD);
 options.ckey [0] [CKEY_RIGHT2] = get_config_int("Misc", "key1_right2", KEY_3_PAD);
 options.ckey [0] [CKEY_FIRE1] = get_config_int("Misc", "key1_fire1", KEY_Z);
 options.ckey [0] [CKEY_FIRE2] = get_config_int("Misc", "key1_fire2", KEY_X);
 options.ckey [0] [CKEY_FIRE3] = get_config_int("Misc", "key1_fire3", KEY_C);
 options.ckey [0] [CKEY_FIRE4] = get_config_int("Misc", "key1_fire4", KEY_V);
 options.ckey [0] [CKEY_COMMAND] = get_config_int("Misc", "key1_command", KEY_A);

 options.ckey [1] [CKEY_UP] = get_config_int("Misc", "key2_up", KEY_UP);
 options.ckey [1] [CKEY_DOWN] = get_config_int("Misc", "key2_down", KEY_DOWN);
 options.ckey [1] [CKEY_LEFT] = get_config_int("Misc", "key2_left", KEY_LEFT);
 options.ckey [1] [CKEY_RIGHT] = get_config_int("Misc", "key2_right", KEY_RIGHT);
 options.ckey [1] [CKEY_LEFT2] = get_config_int("Misc", "key2_left2", KEY_J);
 options.ckey [1] [CKEY_RIGHT2] = get_config_int("Misc", "key2_right2", KEY_K);
 options.ckey [1] [CKEY_FIRE1] = get_config_int("Misc", "key2_fire1", KEY_T);
 options.ckey [1] [CKEY_FIRE2] = get_config_int("Misc", "key2_fire2", KEY_Y);
 options.ckey [1] [CKEY_FIRE3] = get_config_int("Misc", "key2_fire3", KEY_U);
 options.ckey [1] [CKEY_FIRE4] = get_config_int("Misc", "key2_fire4", KEY_I);
 options.ckey [1] [CKEY_COMMAND] = get_config_int("Misc", "key2_command", KEY_O);

 fill_sdetails(); // in menu.c - fills in information about savefiles

/*
 PP.ckey [CKEY_UP] = get_config_int("Misc", "key_up", KEY_UP);
 PP.ckey [CKEY_DOWN] = get_config_int("Misc", "key_down", KEY_DOWN);
 PP.ckey [CKEY_LEFT] = get_config_int("Misc", "key_left", KEY_LEFT);
 PP.ckey [CKEY_RIGHT] = get_config_int("Misc", "key_right", KEY_RIGHT);
 PP.ckey [CKEY_FIRE1] = get_config_int("Misc", "key_fire1", KEY_Z);
 PP.ckey [CKEY_FIRE2] = get_config_int("Misc", "key_fire2", KEY_X);
 PP.ckey [CKEY_FIRE3] = get_config_int("Misc", "key_fire3", KEY_C);
 PP.ckey [CKEY_FIRE4] = get_config_int("Misc", "key_fire4", KEY_V);
*/
/*
 player[1].ckey [CKEY_UP] = get_config_int("MiscP2", "key_up", KEY_UP);
 player[1].ckey [CKEY_DOWN] = get_config_int("MiscP2", "key_down", KEY_DOWN);
 player[1].ckey [CKEY_LEFT] = get_config_int("MiscP2", "key_left", KEY_LEFT);
 player[1].ckey [CKEY_RIGHT] = get_config_int("MiscP2", "key_right", KEY_RIGHT);
 player[1].ckey [CKEY_LEFT2] = get_config_int("MiscP2", "key_left2", KEY_K);
 player[1].ckey [CKEY_RIGHT2] = get_config_int("MiscP2", "key_right2", KEY_L);
 player[1].ckey [CKEY_FIRE1] = get_config_int("MiscP2", "key_fire1", KEY_0_PAD);
 player[1].ckey [CKEY_FIRE2] = get_config_int("MiscP2", "key_fire2", KEY_P);
 player[1].ckey [CKEY_FIRE3] = get_config_int("MiscP2", "key_fire3", KEY_O);
 player[1].ckey [CKEY_FIRE4] = get_config_int("MiscP2", "key_fire4", KEY_I);

 options.joy_stick = get_config_int("Misc", "joy_stick", 0);

 options.joy_sensitivity = get_config_int("Misc", "joy_sensitivity", 70);
 options.init_joystick = get_config_int("Misc", "joy_init", 1);*/

// options.joystick = 0;
// options.key_or_joy = 0; // don't put in initfile!

 options.sfx_volume = get_config_int("Misc", "sfx_volume", 70);
 options.ambience_volume = get_config_int("Misc", "ambience_volume", 100);
 options.run_vsync = get_config_int("Misc", "run_vsync", 0);
 options.fix_camera_angle = get_config_int("Misc", "fix_camera_angle", 0);
// options.joystick_dual = get_config_int("Misc", "joystick_dual", 1);

// set_config_int("Misc", "Tourist", 3);
// set_config_int("Misc", "joy_stick", 0);

// if (options.init_joystick)
 if (options.joystick_available [0]) // set in init_joystick
 {
  options.joy_button [0] [0] = get_config_int("Misc", "joy1_button_1", 0);
  if (options.joy_button [0] [0] > joy[0].num_buttons)
   options.joy_button [0] [0] = joy[0].num_buttons - 1;
  options.joy_button [0] [1] = get_config_int("Misc", "joy1_button_2", 1);
  if (options.joy_button [0] [1] > joy[0].num_buttons)
   options.joy_button [0] [1] = joy[0].num_buttons - 1;
  options.joy_button [0] [2] = get_config_int("Misc", "joy1_button_3", 2);
  if (options.joy_button [0] [2] > joy[0].num_buttons)
   options.joy_button [0] [2] = joy[0].num_buttons - 1;
  options.joy_button [0] [3] = get_config_int("Misc", "joy1_button_4", 3);
  if (options.joy_button [0] [3] > joy[0].num_buttons)
   options.joy_button [0] [3] = joy[0].num_buttons - 1;
  options.joy_button [0] [4] = get_config_int("Misc", "joy1_button_5", 4);
  if (options.joy_button [0] [4] > joy[0].num_buttons)
   options.joy_button [0] [4] = joy[0].num_buttons - 1;
  options.joy_button [0] [5] = get_config_int("Misc", "joy1_button_6", 5);
  if (options.joy_button [0] [5] > joy[0].num_buttons)
   options.joy_button [0] [5] = joy[0].num_buttons - 1;
  options.joy_button [0] [6] = get_config_int("Misc", "joy1_button_7", 6);
  if (options.joy_button [0] [6] > joy[0].num_buttons)
   options.joy_button [0] [6] = joy[0].num_buttons - 1;

  options.joy_stick [0] [0] = get_config_int("Misc", "joy1_stick_1", 0);
  if (options.joy_stick [0] [0] > joy[0].num_sticks)
   options.joy_stick [0] [0] = -1;
  options.joy_stick [0] [1] = get_config_int("Misc", "joy1_stick_2", 0);
  if (options.joy_stick [0] [1] > joy[0].num_sticks)
   options.joy_stick [0] [1] = -1;
  options.joy_stick [0] [2] = get_config_int("Misc", "joy1_stick_3", 1);
  if (options.joy_stick [0] [2] > joy[0].num_sticks)
   options.joy_stick [0] [2] = -1;

  options.joy_axis [0] [0] = get_config_int("Misc", "joy1_axis_1", 1);
  if (options.joy_stick [0] [0] == -1
   || options.joy_axis [0] [0] > joy[0].stick[options.joy_stick [0] [0]].num_axis)
   options.joy_axis [0] [0] = -1;
  options.joy_axis [0] [1] = get_config_int("Misc", "joy1_axis_2", 0);
  if (options.joy_stick [0] [1] == -1
   || options.joy_axis [0] [1] > joy[0].stick[options.joy_stick [0] [1]].num_axis)
   options.joy_axis [0] [1] = -1;
  options.joy_axis [0] [2] = get_config_int("Misc", "joy1_axis_3", 0);
  if (options.joy_stick [0] [2] == -1
   || options.joy_axis [0] [2] > joy[0].stick[options.joy_stick [0] [2]].num_axis)
   options.joy_axis [0] [2] = -1;
 }

 if (options.joystick_available [1]) // set in init_joystick
 {
  options.joy_button [1] [0] = get_config_int("Misc", "joy2_button_1", 0);
  if (options.joy_button [1] [0] > joy[1].num_buttons)
   options.joy_button [1] [0] = joy[1].num_buttons - 1;
  options.joy_button [1] [1] = get_config_int("Misc", "joy2_button_2", 1);
  if (options.joy_button [1] [1] > joy[1].num_buttons)
   options.joy_button [1] [1] = joy[1].num_buttons - 1;
  options.joy_button [1] [2] = get_config_int("Misc", "joy2_button_3", 2);
  if (options.joy_button [1] [2] > joy[1].num_buttons)
   options.joy_button [1] [2] = joy[1].num_buttons - 1;
  options.joy_button [1] [3] = get_config_int("Misc", "joy2_button_4", 3);
  if (options.joy_button [1] [3] > joy[1].num_buttons)
   options.joy_button [1] [3] = joy[1].num_buttons - 1;
  options.joy_button [1] [4] = get_config_int("Misc", "joy2_button_5", 4);
  if (options.joy_button [1] [4] > joy[1].num_buttons)
   options.joy_button [1] [4] = joy[1].num_buttons - 1;
  options.joy_button [1] [5] = get_config_int("Misc", "joy2_button_6", 5);
  if (options.joy_button [1] [5] > joy[1].num_buttons)
   options.joy_button [1] [5] = joy[1].num_buttons - 1;
  options.joy_button [1] [6] = get_config_int("Misc", "joy2_button_7", 6);
  if (options.joy_button [1] [6] > joy[1].num_buttons)
   options.joy_button [1] [6] = joy[1].num_buttons - 1;


  options.joy_stick [1] [0] = get_config_int("Misc", "joy2_stick_1", 0);
  if (options.joy_stick [1] [0] > joy[1].num_sticks)
   options.joy_stick [1] [0] = -1;
  options.joy_stick [1] [1] = get_config_int("Misc", "joy2_stick_2", 0);
  if (options.joy_stick [1] [1] > joy[1].num_sticks)
   options.joy_stick [1] [1] = -1;
  options.joy_stick [1] [2] = get_config_int("Misc", "joy2_stick_3", 1);
  if (options.joy_stick [1] [2] > joy[1].num_sticks)
   options.joy_stick [1] [2] = -1;

  options.joy_axis [1] [0] = get_config_int("Misc", "joy2_axis_1", 0);
  if (options.joy_stick [1] [0] == -1
   || options.joy_axis [1] [0] > joy[0].stick[options.joy_stick [1] [0]].num_axis)
   options.joy_axis [1] [0] = -1;
  options.joy_axis [1] [1] = get_config_int("Misc", "joy2_axis_2", 1);
  if (options.joy_stick [1] [1] == -1
   || options.joy_axis [1] [1] > joy[0].stick[options.joy_stick [1] [1]].num_axis)
   options.joy_axis [1] [1] = -1;
  options.joy_axis [1] [2] = get_config_int("Misc", "joy2_axis_3", 0);
  if (options.joy_stick [1] [2] == -1
   || options.joy_axis [1] [2] > joy[0].stick[options.joy_stick [1] [2]].num_axis)
   options.joy_axis [1] [2] = -1;
  }
// }

 ticked = 0;

 arena.players = 1;

 init_logfile(); // in debug.c
/*
 log_num(100);
 log_str("Hello!", 1);
 log_num(200);
 log_num(50);
 log_num(1);*/
}



