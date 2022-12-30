#include <string.h>

#include "config.h"

#include "allegro.h"

#include "globvars.h"

#include "palette.h"
#include "game.h"
#include "input.h"
#include "briefing.h"
#include "display.h"
#include "sound.h"
#include "savefile.h"

#include "stuff.h"

#define TRANS_MODE drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
#define END_TRANS_MODE drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
#define END_TRANS drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);

#define MENU_CIRCLES 6

#define FREQ_SELECT 1200
#define FREQ_SELECT1 (FREQ_SELECT * 1.05946 * 1.05946 * 1.05946 * 1.05946)
#define FREQ_SELECT2 (FREQ_SELECT * 1.05946 * 1.05946)

#define VOL_SELECT 100
#define VOL_SELECT1 100

#define MENU_TEXT COL_BOX4

 extern RGB palet [256];

 RGB splash_palette [256];

 BITMAP *butter_bitmap;

 extern FONT* small_font;

void key_box(const char ktext [], int command, int p);
int acceptable_char(int scode);
void define_keys(void);
void show_controls_display(void);
int run_controls_setting(void);
int cselect_final_row(void);
char check_duplicate_key(int i, int j);
char check_duplicate_button(int i, int j);

int ship_select(void);
void print_standard_names(int x, int y, int w);
void set_options(void);
void joystick_display(void);

void aabox(int x1, int y1, int x2, int y2);

void congratulations(void);
void congrat_wait(int waiting);

void scancode_to_keyname(int scanc, char sstr [30]);

void menu_soundwf(int wv, int f);

extern volatile unsigned char ticked;

extern BITMAP *display [3];

int menu_select;
int key_wait;
int thing;
//int esc_wait; // prevents escape key being registered on two successive menus

int menu_counter;
int counter2;

int new_menu_circle;

int menu_circle_number [MENU_CIRCLES];
int menu_circle_size [MENU_CIRCLES];
int menu_circle_max_size [MENU_CIRCLES];
int menu_circle_rad [MENU_CIRCLES];
int menu_circle_rot [MENU_CIRCLES];
int menu_circle_grow_rate [MENU_CIRCLES];
int menu_circle_angle [MENU_CIRCLES];

int minicircle_size [MENU_CIRCLES];
int minicircle_max_size [MENU_CIRCLES];
int minicircle_distance [MENU_CIRCLES];

void init_menu_background(void);
void run_menu_background(void);
void display_menu_background(void);

void run_menu_circles(void);
void init_menu_circles(void);
void draw_menu_circle(int i, int j);
void goodbye_menu_circles(void);

void draw_upgrade_screen(void);
void draw_upgrade_hello(int hello);
void upgrade_box(int x1, int y1, int x2, int y2, int col_out, int col_in);
void urect(int x1, int y1, int x2, int y2, int col);

void draw_choose_screen(void);
void draw_choose_hello(int hello);
int choose_menu(void);

void draw_jbox(int x, int y, int js);

int load_game_menu(void);

//extern RLE_SPRITE *RLE_player [PLAYER_RLES];

enum
{
MENU_LOAD,
MENU_START,
MENU_STAGE, // reinstated for now
MENU_PLAYERS,
MENU_P1_CONTROL,
MENU_P2_CONTROL,
MENU_KEYS,
MENU_OPTIONS,
MENU_EXIT
};

//extern FONT* gfont;


enum
{
OPT_SFX_VOLUME,
OPT_MUSIC_VOLUME,
OPT_MODE,
OPT_VSYNC,
OPT_CAMERA,
//OPT_STICK,
OPT_EXIT
};

void _increase_stage(void);
int menu_command(int cmd);
int any_joystick_input(void);

enum
{
MC_UP,
MC_LEFT,
MC_DOWN,
MC_RIGHT,
MC_SELECT
};

void set_options(void);


void reset_menu_palette(void);
void run_menu_background(void);

int select_a_ship(void);

//extern BITMAP *interlude_screen;


void reset_menu_palette(void)
{

 vsync();
// clear_bitmap(screen);
 clear_bitmap(display [0]);
 init_palette(0);

// arena.target_palette = 0;
// set_base_palette();

//   set_base_palette();
//   int newpal = build_new_palette(arena.change_palette, 0, 0, 0);
//   clear_bitmap(screen);
//   set_palette(palet [newpal]);

}



void startup_menu(void)
{


 reset_menu_palette();


 menu_select = MENU_START;
 key_wait = 30;
// esc_wait = 0;
 arena.stage = 1;

// int counter;

 menu_counter = 0;
 counter2 = 0;

 int y1 = 170;
 int y2 = 190 + menu_select * 30;
 int y3 = 218 + menu_select * 30;
 int y4 = 218 + menu_select * 30;

 int anykey = 0;

 int i, p;

 init_menu_background();

 while (TRUE)
 {

 if (key_wait == 0 && key [KEY_ESC])
   exit(0);

 grand(100);

 clear_to_color(display [0], COL_STAR1);


 run_menu_background();
 display_menu_background();

 //joystick_display();


 menu_counter += 4;
 if (menu_counter >= 40)
  menu_counter = 0;

 counter2 ++;
 if (counter2 >= 256)
  counter2 = 0;

 textprintf_right_ex(display[0], small_font, 250, 100, COL_BOX3, -1, "Angry Moth");
 textprintf_right_ex(display[0], small_font, 250, 130, COL_BOX2, -1, "May 2011 demo/beta (revised)");
// textprintf_centre_ex(display, small_font, 320, 140, MENU_TEXT, -1, "another demo for shmup-dev.com");
// draw_rle_sprite(display[0], white_RLE, 150, 150);
/*
 struct msgstruct rmsg;
 struct linestruct bline [LINES];
 strcpy(rmsg.text, "testing testing 1 2 3 testing testing 1 2 3 testing testing 1 2 3 testing testing 1 2 3 testing testing 1 2 3 testing testing 1 2 3 testing testing 1 2 3 testing testing 1 2 3 testing testing 1 2 3 testing testing 1 2 3 X ");
 generate_message(rmsg, 200, 15, COL_WHITE, bline);
 display_message(display[0], bline, 10, 10);
*/

 int my = 200;

 y1 = my - 23;
 y2 = my - 10 + menu_select * 30 - 30;
 y3 = my + 19 + menu_select * 30 - 30;
 y4 = my + 150;


// rectfill(display[0], 370, y1, 600, y2, COL_COL1);
 TRANS_MODE
 rectfill(display[0], 370, y2 + 9, 800, y3 - 2, TRANS_BLUE2);
 vline(display[0], 369, y2 + 9, y3 - 2, TRANS_BLUE3);
 hline(display[0], 370, y2 + 8, 800, TRANS_BLUE3);
 hline(display[0], 370, y3 - 1, 800, TRANS_BLUE3);
// rect(display, 369, y2 + 8, 641, y3 - 1, TRANS_BLUE3);

// rect(display, 368, y2 + 7, 641, y3 - 0, TRANS_BLUE3);
 END_TRANS
// rectfill(display[0], 370, y3, 600, y4, TRANS_DGREEN);

 if (arena.players == 2 && player[0].control == player[1].control)
 {
  TRANS_MODE
  rectfill(display[0], 390, my + 60 - 5, 640, my + 60 + 52, TRANS_RED1);
  vline(display[0], 390, my + 60 - 5 + 1, my + 60 + 52 - 1, TRANS_RED2);
  vline(display[0], 640, my + 60 - 5 + 1, my + 60 + 52 - 1, TRANS_RED2);
  hline(display[0], 390 + 1, my + 60 - 5, 640 - 1, TRANS_RED2);
  hline(display[0], 390 + 1, my + 60 + 52, 640 - 1, TRANS_RED2);
  END_TRANS
 }

// textprintf_ex(display[0], small_font, 400, my - 30, MENU_TEXT, -1, "start game");
textprintf_ex(display[0], small_font, 400, my - 30, MENU_TEXT, -1, "load game");
 textprintf_ex(display[0], small_font, 400, my, MENU_TEXT, -1, "start new game");
 textprintf_ex(display[0], small_font, 400, my + 30, MENU_TEXT, -1, "stage");
 textprintf_ex(display[0], small_font, 530, my + 30, MENU_TEXT, -1, "%i", arena.stage);
 textprintf_ex(display[0], small_font, 520, my + 30, MENU_TEXT, -1, "<     >");
 textprintf_ex(display[0], small_font, 400, my + 60, MENU_TEXT, -1, "players");
 textprintf_ex(display[0], small_font, 530, my + 60, MENU_TEXT, -1, "%i", arena.players);
 textprintf_ex(display[0], small_font, 520, my + 60, MENU_TEXT, -1, "<     >");
 textprintf_ex(display[0], small_font, 400, my + 90, MENU_TEXT, -1, "player 1 control");
 switch(player[0].control)
 {
  case CONTROL_KEY_A: textprintf_ex(display[0], small_font, 530, my + 90, MENU_TEXT, -1, "keyboard A"); break;
  case CONTROL_KEY_B: textprintf_ex(display[0], small_font, 530, my + 90, MENU_TEXT, -1, "keyboard B"); break;
  case CONTROL_JOY_A: textprintf_ex(display[0], small_font, 530, my + 90, MENU_TEXT, -1, "controller A"); break;
  case CONTROL_JOY_B: textprintf_ex(display[0], small_font, 530, my + 90, MENU_TEXT, -1, "controller B"); break;
 }
 textprintf_ex(display[0], small_font, 520, my + 90, MENU_TEXT, -1, "<                                >");
 textprintf_ex(display[0], small_font, 400, my + 120, MENU_TEXT, -1, "player 2 control");
 switch(player[1].control)
 {
  case CONTROL_KEY_A: textprintf_ex(display[0], small_font, 530, my + 120, MENU_TEXT, -1, "keyboard A"); break;
  case CONTROL_KEY_B: textprintf_ex(display[0], small_font, 530, my + 120, MENU_TEXT, -1, "keyboard B"); break;
  case CONTROL_JOY_A: textprintf_ex(display[0], small_font, 530, my + 120, MENU_TEXT, -1, "controller A"); break;
  case CONTROL_JOY_B: textprintf_ex(display[0], small_font, 530, my + 120, MENU_TEXT, -1, "controller B"); break;
 }
 textprintf_ex(display[0], small_font, 520, my + 120, MENU_TEXT, -1, "<                                >");
// textprintf_ex(display[0] [2], small_font, 400, 260, MENU_TEXT, -1, "STAGE - %i", arena.starting_level);
 textprintf_ex(display[0], small_font, 400, my + 150, MENU_TEXT, -1, "set controls");
 textprintf_ex(display[0], small_font, 400, my + 180, MENU_TEXT, -1, "options");
 textprintf_ex(display[0], small_font, 400, my + 210, MENU_TEXT, -1, "exit");

// textprintf_ex(display[0], small_font, 40, 10, MENU_TEXT, -1, "%i", joy[0].stick[options.joy_stick].axis[0].pos);
// textprintf_ex(display[0], small_font, 40, 30, MENU_TEXT, -1, "%i", joy[0].stick[options.joy_stick].axis[1].pos);

/*
 if (options.joystick)
  textprintf_ex(display[0], small_font, 400, my + 120, MENU_TEXT, -1, "CALIBRATE JOYSTICK");
   else
   {
    textprintf_ex(display[0], small_font, 400, my + 120, MENU_TEXT, -1, "NO JOYSTICK");
    drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
    rectfill(display[0], 399, my + 110, 530, my + 135, CONVERT_WHITE_TO_GREY);
    drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
   }
*/
/* if (options.joystick)
 {
  if (options.key_or_joy == 1)
  textprintf_ex(display[0], small_font, 400, my + 30, MENU_TEXT, -1, "controls - joystick");
   else
    textprintf_ex(display[0], small_font, 400, my + 30, MENU_TEXT, -1, "controls - keyboard");
 }
  else
  {
   textprintf_ex(display[0], small_font, 400, my + 30, MENU_TEXT, -1, "no joystick");
   drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
   rectfill(display[0], 399, my + 29, 530, my + 45, CONVERT_WHITE_TO_GREY);
   drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);

  }*/


// textprintf_ex(display[0], small_font, 5, 5, MENU_TEXT, -1, "%i", any_joystick_input());



 my = 330;

// y2 = my + 7 + arena.just_got_highscore * 30;
// y3 = my + 29 + arena.just_got_highscore * 30;

 textprintf_right_ex(display[0], small_font, 790, 582, MENU_TEXT, -1, "by linley henzell");

 anykey = 0;

  for (i = KEY_A; i < KEY_CAPSLOCK + 1; i ++)
  {
   if (key [i])
   {
    anykey = 1;
   }
  }

  if (anykey == 0 && any_joystick_input() == 0)
   key_wait = 0;



 if (key_wait == 0)
 {
  if (menu_command(MC_UP))
  {
   menu_select --;
   if (menu_select < 0)
    menu_select = MENU_EXIT;
   key_wait = 7;
   menu_soundwf(WAV_SELECT0, FREQ_SELECT);

  }
  if (menu_command(MC_DOWN))
  {
   menu_select ++;
   if (menu_select > MENU_EXIT)
    menu_select = 0;
   key_wait = 7;
   menu_soundwf(WAV_SELECT0, FREQ_SELECT);
  }
  if (menu_command(MC_LEFT))
  {
    if (menu_select == MENU_STAGE)
    {
      if (arena.stage == 1)
          arena.stage = 6;
      else
          arena.stage = arena.stage - 1;
      menu_soundwf(WAV_SELECT0, FREQ_SELECT1);
   }
   if (menu_select == MENU_PLAYERS)
   {
    if (arena.players == 1)
     arena.players = 2;
      else
       arena.players = 1;
    menu_soundwf(WAV_SELECT0, FREQ_SELECT1);
   }
   if (menu_select == MENU_P1_CONTROL
    || menu_select == MENU_P2_CONTROL)
   {
    p = 0;
    if (menu_select == MENU_P2_CONTROL)
     p = 1;
//    do
    {
     PP.control --;
     if (PP.control < 0)
      PP.control = CONTROL_JOY_B;
     if (PP.control == CONTROL_JOY_B
      && !options.joystick_available [1])
       PP.control = CONTROL_JOY_A;
     if (PP.control == CONTROL_JOY_A
      && !options.joystick_available [0])
       PP.control = CONTROL_KEY_B;
    } //while (player[p].control == player[p^1].control);
    menu_soundwf(WAV_SELECT0, FREQ_SELECT1);
   }
   key_wait = 7;
  }
  if (menu_command(MC_RIGHT))
  {
    if (menu_select == MENU_STAGE)
      _increase_stage();
   if (menu_select == MENU_PLAYERS)
   {
    if (arena.players == 1)
     arena.players = 2;
      else
       arena.players = 1;
    menu_soundwf(WAV_SELECT0, FREQ_SELECT1);
   }
   if (menu_select == MENU_P1_CONTROL
    || menu_select == MENU_P2_CONTROL)
   {
    p = 0;
    if (menu_select == MENU_P2_CONTROL)
     p = 1;
//    do
    {
     PP.control ++;
     if (PP.control > CONTROL_JOY_B)
      PP.control = CONTROL_KEY_A;
     if (PP.control == CONTROL_JOY_A
      && !options.joystick_available [0])
       PP.control = CONTROL_JOY_B;
     if (PP.control == CONTROL_JOY_B
      && !options.joystick_available [1])
       PP.control = CONTROL_KEY_A;
    }// while (player[p].control == player[p^1].control);
   }
   key_wait = 7;
   menu_soundwf(WAV_SELECT0, FREQ_SELECT1);
  }

//  if (key [KEY_ESC])
//   exit(0);
  if (menu_command(MC_SELECT))
  {
   if (menu_select == MENU_EXIT)
   {
    menu_soundwf(WAV_SELECT1, FREQ_SELECT1);
    exit(0);
   }
   if (menu_select == MENU_STAGE)
       _increase_stage();
   if (menu_select == MENU_PLAYERS)
   {
    if (arena.players == 1)
     arena.players = 2;
      else
       arena.players = 1;
    menu_soundwf(WAV_SELECT0, FREQ_SELECT1);
   }
   if (menu_select == MENU_P1_CONTROL
    || menu_select == MENU_P2_CONTROL)
   {
    p = 0;
    if (menu_select == MENU_P2_CONTROL)
     p = 1;
//    do
    {
     PP.control ++;
     if (PP.control > CONTROL_JOY_B)
      PP.control = CONTROL_KEY_A;
     if (PP.control == CONTROL_JOY_A
      && !options.joystick_available [0])
       PP.control = CONTROL_JOY_B;
     if (PP.control == CONTROL_JOY_B
      && !options.joystick_available [1])
       PP.control = CONTROL_KEY_A;
    } //while (player[p].control == player[p^1].control);
    menu_soundwf(WAV_SELECT0, FREQ_SELECT1);
    key_wait = 7;
   }

   if (menu_select == MENU_KEYS)
   {
    key_wait = 10;
    define_keys();
    key_wait = 10;
    menu_soundwf(WAV_SELECT1, FREQ_SELECT1);
   }

   if (menu_select == MENU_OPTIONS)
   {
    key_wait = 10;
    menu_soundwf(WAV_SELECT1, FREQ_SELECT1);
    set_options();
    key_wait = 10;
   }

/*   if (menu_select == MENU_CALIBRATE)
   {
    jstick_calibrate();
    key_wait = 20;
    ticked = 0;
   }*/

   if (menu_select == MENU_START)
   {
    ticked = 0;
    key_wait = 30;
    menu_soundwf(WAV_SELECT1, FREQ_SELECT1);
//    arena.stage = 6;

    start_new_game(); // in game.c - calls run_game

    init_menu_background();
    key_wait = 100;
    ticked = 0;
   }

   if (menu_select == MENU_LOAD)
   {
    ticked = 0;
    key_wait = 30;
    menu_soundwf(WAV_SELECT1, FREQ_SELECT1);
    load_game_menu(); // this can call run_game
   }

//    else
//     key_wait = 20;

  }
 }
  else
   key_wait --;




    do
    {
        rest(1);
//        thing ++;
    } while (ticked == 0);
    ticked = 0;


 vsync();
 blit(display[0], screen, 0, 0, 0, 0, 800, 600);



 };


}

void _increase_stage(void)
{
  if (arena.stage < 6)
    arena.stage = arena.stage + 1;
  else
    arena.stage = 1;
  menu_soundwf(WAV_SELECT0, FREQ_SELECT1);
}

#define MENU_SENSE 100

int menu_command(int cmd)
{

 if (options.joystick_available [0]
  || options.joystick_available [1])
  poll_joystick();


 switch(cmd)
 {
  case MC_UP:
   if (key [KEY_UP] || key [KEY_8_PAD])
    return 1;
   if (options.joystick_available [0] && joy[0].stick[0].axis[1].pos < -MENU_SENSE)
    return 1;
   return 0;
  case MC_DOWN:
   if (key [KEY_DOWN] || key [KEY_2_PAD])
    return 1;
   if (options.joystick_available [0] && joy[0].stick[0].axis[1].pos > MENU_SENSE)
    return 1;
   return 0;
  case MC_LEFT:
   if (key [KEY_LEFT] || key [KEY_4_PAD])
    return 1;
   if (options.joystick_available [0] && joy[0].stick[0].axis[0].pos < -MENU_SENSE)
    return 1;
   return 0;


  case MC_RIGHT:
   if (key [KEY_RIGHT] || key [KEY_6_PAD])
    return 1;
   if (options.joystick_available [0] && joy[0].stick[0].axis[0].pos > MENU_SENSE)
    return 1;
   return 0;
  case MC_SELECT:
   if (key [KEY_SPACE] || key [KEY_ENTER] || key [KEY_ENTER_PAD] || key [KEY_Z])// || key [options.ckey [0] [CKEY_FIRE1]])
    return 1;
   if (options.joystick_available [0] && joy[0].button[options.joy_button [0] [0]].b)
    return 1;
   return 0;

 }

 return 0;

}


int any_joystick_input(void)
{

 if (!options.joystick_available [0]
  && !options.joystick_available [1])
   return 0;

  poll_joystick();

 int j;

 for (j = 0; j < 2; j ++)
 {
  if (options.joystick_available [j] == 0)
   continue;

  if (joy[j].stick[0].axis[0].pos < - MENU_SENSE
   || joy[j].stick[0].axis[0].pos > MENU_SENSE
   || joy[j].stick[0].axis[1].pos < -MENU_SENSE
   || joy[j].stick[0].axis[1].pos > MENU_SENSE)
    return 1;

   if (joy[j].button[options.joy_button [0] [0]].b)
    return 1;
 }

 return 0;
}


int player_joystick_input(int p)
{

 if (PP.control < CONTROL_JOY_A)
  return 0;

 poll_joystick();

 int js = PP.control - CONTROL_JOY_A;


  if (joy[js].stick[0].axis[0].pos < - MENU_SENSE
   || joy[js].stick[0].axis[0].pos > MENU_SENSE
   || joy[js].stick[0].axis[1].pos < -MENU_SENSE
   || joy[js].stick[0].axis[1].pos > MENU_SENSE)
    return 1;

   if (joy[js].button[options.joy_button [js] [0]].b
    || joy[js].button[options.joy_button [js] [1]].b)
    return 1;

 return 0;
}



int cselect_row;
int cselect_col;
int cselect_selecting;
int select_cursor;

void define_keys(void)
{

 cselect_row = 0;
 cselect_col = 0;
 cselect_selecting = 0;

 char finished = 0;

 do
 {

  show_controls_display();

  finished = run_controls_setting();


    do
    {
        rest(1);
    } while (ticked == 0);
    ticked = 0;

    select_cursor ++;

 }
  while (finished == 0);

 set_config_int("Misc", "key1_up", options.ckey [0] [CKEY_UP]);
 set_config_int("Misc", "key1_left", options.ckey [0] [CKEY_LEFT]);
 set_config_int("Misc", "key1_right", options.ckey [0] [CKEY_RIGHT]);
 set_config_int("Misc", "key1_down", options.ckey [0] [CKEY_DOWN]);
 set_config_int("Misc", "key1_left2", options.ckey [0] [CKEY_LEFT2]);
 set_config_int("Misc", "key1_right2", options.ckey [0] [CKEY_RIGHT2]);
 set_config_int("Misc", "key1_fire1", options.ckey [0] [CKEY_FIRE1]);
 set_config_int("Misc", "key1_fire2", options.ckey [0] [CKEY_FIRE2]);
 set_config_int("Misc", "key1_fire3", options.ckey [0] [CKEY_FIRE3]);
 set_config_int("Misc", "key1_fire4", options.ckey [0] [CKEY_FIRE4]);
 set_config_int("Misc", "key1_command", options.ckey [0] [CKEY_COMMAND]);

 set_config_int("Misc", "key2_up", options.ckey [1] [CKEY_UP]);
 set_config_int("Misc", "key2_left", options.ckey [1] [CKEY_LEFT]);
 set_config_int("Misc", "key2_right", options.ckey [1] [CKEY_RIGHT]);
 set_config_int("Misc", "key2_down", options.ckey [1] [CKEY_DOWN]);
 set_config_int("Misc", "key2_left2", options.ckey [1] [CKEY_LEFT2]);
 set_config_int("Misc", "key2_right2", options.ckey [1] [CKEY_RIGHT2]);
 set_config_int("Misc", "key2_fire1", options.ckey [1] [CKEY_FIRE1]);
 set_config_int("Misc", "key2_fire2", options.ckey [1] [CKEY_FIRE2]);
 set_config_int("Misc", "key2_fire3", options.ckey [1] [CKEY_FIRE3]);
 set_config_int("Misc", "key2_fire4", options.ckey [1] [CKEY_FIRE4]);
 set_config_int("Misc", "key2_command", options.ckey [1] [CKEY_COMMAND]);

 if (options.joystick_available [0])
 {

  set_config_int("Misc", "joy1_button_1", options.joy_button [0] [0]);
  set_config_int("Misc", "joy1_button_2", options.joy_button [0] [1]);
  set_config_int("Misc", "joy1_button_3", options.joy_button [0] [2]);
  set_config_int("Misc", "joy1_button_4", options.joy_button [0] [3]);
  set_config_int("Misc", "joy1_button_5", options.joy_button [0] [4]);
  set_config_int("Misc", "joy1_button_6", options.joy_button [0] [5]);
  set_config_int("Misc", "joy1_button_7", options.joy_button [0] [6]);

  set_config_int("Misc", "joy1_stick_1", options.joy_stick [0] [0]);
  set_config_int("Misc", "joy1_stick_2", options.joy_stick [0] [1]);
  set_config_int("Misc", "joy1_stick_3", options.joy_stick [0] [2]);
  set_config_int("Misc", "joy1_axis_1", options.joy_axis [0] [0]);
  set_config_int("Misc", "joy1_axis_2", options.joy_axis [0] [1]);
  set_config_int("Misc", "joy1_axis_3", options.joy_axis [0] [2]);
 }

 if (options.joystick_available [0])
 {
  set_config_int("Misc", "joy2_button_1", options.joy_button [1] [0]);
  set_config_int("Misc", "joy2_button_2", options.joy_button [1] [1]);
  set_config_int("Misc", "joy2_button_3", options.joy_button [1] [2]);
  set_config_int("Misc", "joy2_button_4", options.joy_button [1] [3]);
  set_config_int("Misc", "joy2_button_5", options.joy_button [1] [4]);
  set_config_int("Misc", "joy2_button_6", options.joy_button [1] [5]);
  set_config_int("Misc", "joy2_button_7", options.joy_button [1] [6]);

  set_config_int("Misc", "joy2_stick_1", options.joy_stick [1] [0]);
  set_config_int("Misc", "joy2_stick_2", options.joy_stick [1] [1]);
  set_config_int("Misc", "joy2_stick_3", options.joy_stick [1] [2]);
  set_config_int("Misc", "joy2_axis_1", options.joy_axis [1] [0]);
  set_config_int("Misc", "joy2_axis_2", options.joy_axis [1] [1]);
  set_config_int("Misc", "joy2_axis_3", options.joy_axis [1] [2]);
 }

}

#define CONTROL_X -40
#define CONTROL_Y 50
#define CONTROL_ROW 17
#define CONTROL_COL 180


enum
{
MJBUTTON_FIRE1,
MJBUTTON_FIRE2,
MJBUTTON_FIRE3,
MJBUTTON_FIRE4,
MJBUTTON_LEFT2,
MJBUTTON_RIGHT2,
MJBUTTON_COMMAND,
MJSTICK_1,
MJAXIS_1,
MJSTICK_2,
MJAXIS_2,
MJSTICK_3,
MJAXIS_3,
MJBUTTONS
// any additions to this list need to be added manually
//  to the get_config_int and set_config_int bits in main and menu.
};

#define BOX_UP 1
#define BOX_DOWN 14

void show_controls_display(void)
{
 int i, j, x, y, col;
 char sstr [30];

 rectfill(display[0], 10, CONTROL_Y, 790, 550, COL_BOX1);
 rect(display[0], 10, CONTROL_Y, 790, 550, COL_BOX2);

 textprintf_centre_ex(display[0], small_font, 400, CONTROL_Y + 20, MENU_TEXT, -1, "set controls");
 textprintf_centre_ex(display[0], small_font, CONTROL_X + CONTROL_COL, CONTROL_Y + 50, COL_BOX4, -1, "keyboard A");
 textprintf_centre_ex(display[0], small_font, CONTROL_X + CONTROL_COL*2, CONTROL_Y + 50, COL_BOX4, -1, "keyboard B");
 textprintf_centre_ex(display[0], small_font, CONTROL_X + CONTROL_COL*3, CONTROL_Y + 50, COL_BOX4, -1, "controller A");
 textprintf_centre_ex(display[0], small_font, CONTROL_X + CONTROL_COL*4, CONTROL_Y + 50, COL_BOX4, -1, "controller B");

 for (i = 0; i < 2; i ++)
 {
  for (j = 0; j < CKEY_END; j ++)
  {

   x = CONTROL_X + CONTROL_COL*(i+1);
   y = CONTROL_Y + 90 + j*CONTROL_ROW;

   if (cselect_col == i
    && cselect_row == j)
   {
    col = COL_BOX2;
    if (cselect_selecting)
     col = COL_EBOX2;
    rectfill(display[0], x - 90, y - BOX_UP, x + 70, y + BOX_DOWN, col);
   }
    else
    {
     col = COL_EBOX1;
     if (check_duplicate_key(i, j))
      rectfill(display[0], x - 90, y - BOX_UP, x + 70, y + BOX_DOWN, col);
    }

   switch(j)
   {
    case CKEY_LEFT:
      textprintf_right_ex(display[0], small_font, x, y, COL_BOX3, -1,
       "turn left -"); break;
    case CKEY_RIGHT:
      textprintf_right_ex(display[0], small_font, x, y, COL_BOX3, -1,
       "turn right -"); break;
    case CKEY_LEFT2:
      textprintf_right_ex(display[0], small_font, x, y, COL_BOX3, -1,
       "slide left -"); break;
    case CKEY_RIGHT2:
      textprintf_right_ex(display[0], small_font, x, y, COL_BOX3, -1,
       "slide right -"); break;
    case CKEY_UP:
      textprintf_right_ex(display[0], small_font, x, y, COL_BOX3, -1,
       "forwards -"); break;
    case CKEY_DOWN:
      textprintf_right_ex(display[0], small_font, x, y, COL_BOX3, -1,
       "backwards -"); break;
    case CKEY_FIRE1:
      textprintf_right_ex(display[0], small_font, x, y, COL_BOX3, -1,
       "cannon -"); break;
    case CKEY_FIRE2:
      textprintf_right_ex(display[0], small_font, x, y, COL_BOX3, -1,
       "weapon 1 -"); break;
    case CKEY_FIRE3:
      textprintf_right_ex(display[0], small_font, x, y, COL_BOX3, -1,
       "weapon 2 -"); break;
    case CKEY_FIRE4:
      textprintf_right_ex(display[0], small_font, x, y, COL_BOX3, -1,
       "target -"); break;
    case CKEY_COMMAND:
      textprintf_right_ex(display[0], small_font, x, y, COL_BOX3, -1,
       "command -"); break;
   }


   if (cselect_selecting
    && cselect_col == i
    && cselect_row == j)
   {
//    textprintf_ex(display[0], small_font, x + 12, y, MENU_TEXT, -1, "<   >");
    if (select_cursor % 20 <= 10)
     textprintf_ex(display[0], small_font, x + 2, y, COL_BOX3, -1, "press key");
   }
    else
    {
      scancode_to_keyname(options.ckey [i] [j], sstr);
      textprintf_ex(display[0], small_font, x + 2, y, COL_BOX4, -1,
       sstr);
    }
  }

 }


 for (i = 2; i < 4; i ++)
 {

  if (!options.joystick_available [i - 2])
  {

      x = CONTROL_X + CONTROL_COL*(i+1);

      textprintf_centre_ex(display[0], small_font, x, 170, COL_BOX2, -1, "no");
      textprintf_centre_ex(display[0], small_font, x, 190, COL_BOX2, -1, "controller");
      textprintf_centre_ex(display[0], small_font, x, 210, COL_BOX2, -1, "detected");
      continue;
  }

  for (j = 0; j < MJBUTTONS; j ++)
  {

   x = CONTROL_X + CONTROL_COL*(i+1);
   y = CONTROL_Y + 90 + j*CONTROL_ROW;

   if (cselect_col == i
    && cselect_row == j)
   {
    col = COL_BOX2;
    if (cselect_selecting)
     col = COL_EBOX2;
    rectfill(display[0], x - 90, y - BOX_UP, x + 70, y + BOX_DOWN, col);
   }
    else
    {
     col = COL_EBOX1;
     if (check_duplicate_button(i, j))
      rectfill(display[0], x - 90, y - BOX_UP, x + 70, y + BOX_DOWN, col);
    }

   switch(j)
   {
    case MJBUTTON_LEFT2:
      textprintf_right_ex(display[0], small_font, x, y, COL_BOX3, -1,
       "slide left -"); break;
    case MJBUTTON_RIGHT2:
      textprintf_right_ex(display[0], small_font, x, y, COL_BOX3, -1,
       "slide right -"); break;
    case MJBUTTON_FIRE1:
      textprintf_right_ex(display[0], small_font, x, y, COL_BOX3, -1,
       "cannon -"); break;
    case MJBUTTON_FIRE2:
      textprintf_right_ex(display[0], small_font, x, y, COL_BOX3, -1,
       "weapon 1 -"); break;
    case MJBUTTON_FIRE3:
      textprintf_right_ex(display[0], small_font, x, y, COL_BOX3, -1,
       "weapon 2 -"); break;
    case MJBUTTON_FIRE4:
      textprintf_right_ex(display[0], small_font, x, y, COL_BOX3, -1,
       "target -"); break;
    case MJBUTTON_COMMAND:
      textprintf_right_ex(display[0], small_font, x, y, COL_BOX3, -1,
       "command -"); break;
    case MJSTICK_1:
      y += 2;
      textprintf_right_ex(display[0], small_font, x, y, COL_BOX3, -1,
       "Accel stick -"); break;
    case MJAXIS_1:
      textprintf_right_ex(display[0], small_font, x, y, COL_BOX3, -1,
       "Accel axis -"); break;
    case MJSTICK_2:
      y += 2;
      textprintf_right_ex(display[0], small_font, x, y, COL_BOX3, -1,
       "Turn stick -"); break;
    case MJAXIS_2:
      textprintf_right_ex(display[0], small_font, x, y, COL_BOX3, -1,
       "Turn axis -"); break;
    case MJSTICK_3:
      y += 2;
      textprintf_right_ex(display[0], small_font, x, y, COL_BOX3, -1,
       "Slide stick -"); break;
    case MJAXIS_3:
      textprintf_right_ex(display[0], small_font, x, y, COL_BOX3, -1,
       "Slide axis -"); break;
   }

   int s = -1;

  if (j < MJSTICK_1)
  {
   if (cselect_selecting
    && cselect_col == i
    && cselect_row == j)
   {
    if (select_cursor % 20 <= 10)
     textprintf_ex(display[0], small_font, x + 2, y, COL_BOX3, -1, "press button");
   }
    else
    {
      textprintf_ex(display[0], small_font, x + 2, y, COL_BOX4, -1, "%i", options.joy_button [i - 2] [j] + 1);
    }
  }

#define XSPACE1 2

     if (j >= MJSTICK_1)
       {
        if (cselect_selecting
         && cselect_col == i
         && cselect_row == j)
          textprintf_ex(display[0], small_font, x + 5, y, COL_WHITE, -1, "<                  >");

        switch(j)
        {
         case MJSTICK_1: s = 0;
         case MJSTICK_2: if (s == -1) s = 1;
         case MJSTICK_3: if (s == -1) s = 2;
         if (options.joy_stick [i - 2] [s] == -1)
          textprintf_ex(display[0], small_font, x + XSPACE1, y, COL_EBOX4, -1, "not used");
           else
            textprintf_ex(display[0], small_font, x + XSPACE1, y, COL_BOX4, -1, "%i", options.joy_stick [i - 2] [s] + 1); break;
         case MJAXIS_1: s = 0;
         case MJAXIS_2: if (s == -1) s = 1;
         case MJAXIS_3: if (s == -1) s = 2;
         if (options.joy_stick [i - 2] [s] == -1)
         {
          textprintf_ex(display[0], small_font, x + XSPACE1, y, COL_EBOX4, -1, "not used");
          break;
         }
          switch(options.joy_axis [i - 2] [s])
          {
           case -1: textprintf_ex(display[0], small_font, x + XSPACE1, y, COL_EBOX4, -1, "not used"); break;
           case 0: textprintf_ex(display[0], small_font, x + XSPACE1, y, COL_BOX4, -1, "X axis"); break;
           case 1: textprintf_ex(display[0], small_font, x + XSPACE1, y, COL_BOX4, -1, "Y axis"); break;
           case 2: textprintf_ex(display[0], small_font, x + XSPACE1, y, COL_BOX4, -1, "Z axis"); break;
           default: textprintf_ex(display[0], small_font, x + XSPACE1, y, COL_EBOX4, -1, "unknown"); break;
          }
          break;

        }
       }
    }

//  }

  y += 20;

//  if (cselect_col == i)
   draw_jbox(x - 40, y, i-2);

/*
  if (joy[i-2].num_sticks > 1 && options.joystick_dual)
  {
      textprintf_centre_ex(display[0], small_font, x, y, COL_EBOX2, -1,
       "multi-stick detected!");
      y += 20;
      textprintf_centre_ex(display[0], small_font, x, y, COL_EBOX2, -1,
       "second stick");
      y += 12;
      textprintf_centre_ex(display[0], small_font, x, y, COL_EBOX2, -1,
       "may be usable");
      y += 12;
      textprintf_centre_ex(display[0], small_font, x, y, COL_EBOX2, -1,
       "for slide");
      y += 12;
      textprintf_centre_ex(display[0], small_font, x, y, COL_EBOX2, -1,
       "(although probably not)");

  }
*/
 }


   y = CONTROL_Y + 280 + CKEY_END*CONTROL_ROW;

   if (cselect_row == cselect_final_row())
   {
    col = COL_F3 + TRANS_BLUE2;
    if (cselect_selecting)
     col = COL_F4 + TRANS_RED4;
    rectfill(display[0], 120, y - BOX_UP, 680, y + BOX_DOWN, col);
   }

      textprintf_centre_ex(display[0], small_font, 400, y, MENU_TEXT, -1, "exit");

// textprintf_ex(display [0], small_font, 100, 100, MENU_TEXT, COL_STAR1, "Sticks %i",
//  joy[0].num_sticks);


 blit(display[0], screen, 0, 0, 0, 0, 800, 600);

}

void draw_jbox(int x, int y, int js)
{

 int x2 = x + 5;
// char axis_name [15] = "Error";

 rectfill(display[0], x-15, y, x + 150, y + 140, COL_BOX2);
 rect(display[0], x-15, y, x + 150, y + 140, COL_BOX4);

 int y2 = y + 10;
 int i, j;

 textprintf_ex(display[0], small_font, x2 + 20, y2, COL_BOX0, -1, "Controller %i", js + 1);
 y2 += 12;
 for (i = 0; i < joy[js].num_sticks; i ++)
 {
  y2 += 5;
  textprintf_ex(display[0], small_font, x2, y2, COL_BOX4, -1, "Stick %i", i + 1);
  y2 += 12;
  for (j = 0; j < joy[js].stick[i].num_axis; j ++)
  {
   switch(j)
   {
    case 0: textprintf_ex(display[0], small_font, x2 + 20, y2, COL_BOX4, -1, "X-axis: %i", joy[js].stick[i].axis[j].pos); break;
    case 1: textprintf_ex(display[0], small_font, x2 + 20, y2, COL_BOX4, -1, "Y-axis: %i", joy[js].stick[i].axis[j].pos); break;
    case 2: textprintf_ex(display[0], small_font, x2 + 20, y2, COL_BOX4, -1, "Z-axis: %i", joy[js].stick[i].axis[j].pos); break;
    default: textprintf_ex(display[0], small_font, x2 + 20, y2, COL_BOX4, -1, "?-axis: %i", joy[js].stick[i].axis[j].pos); break;
   }
   y2 += 12;
  }
 }

/* y2 += 5;
 int pressed = 0;

 textprintf_ex(display[0], small_font, x2, y2, COL_BOX4, -1, "Buttons:");

 x2 += text_length(small_font, "Buttons:    ");

 for (i = 0; i < joy[js].num_buttons; i ++)
 {
     if (joy[js].button [i].b)
     {
       textprintf_ex(display[0], small_font, x2, y2, COL_BOX4, -1, "%i", i);
       x2 += 10;
       pressed ++;
     }
  if (pressed > 6)
   break;
 }

*/

}


char check_duplicate_key(int i, int j)
{
 int k, l;

 for (k = 0; k < 2; k ++)
 {
  for (l = 0; l < CKEY_END; l ++)
  {
   if ((i != k || j != l)
    && options.ckey [k] [l] == options.ckey [i] [j])
    return 1;
  }
 }

 return 0;

}

char check_duplicate_button(int i, int j)
{
 int k;

 switch(j)
 {
  case MJSTICK_1:
  case MJAXIS_1:
   if (options.joy_stick [i - CONTROL_JOY_A] [0] != -1
    && ((options.joy_stick [i - CONTROL_JOY_A] [0] == options.joy_stick [i - CONTROL_JOY_A] [1]
        && options.joy_axis [i - CONTROL_JOY_A] [0] == options.joy_axis [i - CONTROL_JOY_A] [1])
    || (options.joy_stick [i - CONTROL_JOY_A] [0] == options.joy_stick [i - CONTROL_JOY_A] [2]
        && options.joy_axis [i - CONTROL_JOY_A] [0] == options.joy_axis [i - CONTROL_JOY_A] [2])))
         return 1;
    return 0;
  case MJSTICK_2:
  case MJAXIS_2:
   if (options.joy_stick [i - CONTROL_JOY_A] [1] != -1
    && ((options.joy_stick [i - CONTROL_JOY_A] [1] == options.joy_stick [i - CONTROL_JOY_A] [0]
        && options.joy_axis [i - CONTROL_JOY_A] [1] == options.joy_axis [i - CONTROL_JOY_A] [0])
    || (options.joy_stick [i - CONTROL_JOY_A] [1] == options.joy_stick [i - CONTROL_JOY_A] [2]
        && options.joy_axis [i - CONTROL_JOY_A] [1] == options.joy_axis [i - CONTROL_JOY_A] [2])))
         return 1;
    return 0;
  case MJSTICK_3:
  case MJAXIS_3:
   if (options.joy_stick [i - CONTROL_JOY_A] [2] != -1
    && ((options.joy_stick [i - CONTROL_JOY_A] [2] == options.joy_stick [i - CONTROL_JOY_A] [0]
        && options.joy_axis [i - CONTROL_JOY_A] [2] == options.joy_axis [i - CONTROL_JOY_A] [0])
    || (options.joy_stick [i - CONTROL_JOY_A] [2] == options.joy_stick [i - CONTROL_JOY_A] [1]
        && options.joy_axis [i - CONTROL_JOY_A] [2] == options.joy_axis [i - CONTROL_JOY_A] [1])))
         return 1;
    return 0;

 }

 for (k = 0; k < JBUTTONS; k ++)
 {
  if (k != j && options.joy_button [i - CONTROL_JOY_A] [j] == options.joy_button [i - CONTROL_JOY_A] [k])
   return 1;
 }
 return 0;

}


int run_controls_setting(void)
{

  char anykey = 0;
  char inputted = 0;
  int i, js;

  key_wait --;

  for (i = KEY_A; i < KEY_CAPSLOCK + 1; i ++)
  {
   if (key [i])
   {
    anykey = 1;
    inputted = i;
   }
  }


  if (anykey == 0 && any_joystick_input() == 0)
   key_wait = 0;

 if (key_wait <= 0)
 {
  if (cselect_selecting
   && cselect_col <= CONTROL_KEY_B
   && anykey)
  {
   if (inputted == KEY_ESC)
   {
    key_wait = 30;
//    esc_wait = 1;
    cselect_selecting = 0;
    return 0;
   }
   if (acceptable_char(inputted))
   {
    options.ckey [cselect_col] [cselect_row] = inputted;
    cselect_selecting = 0;
    key_wait = 40;
    return 0;
   }
  }

  if (cselect_selecting
   && cselect_col >= CONTROL_JOY_A)
  {
   js = 0;
   if (cselect_col == CONTROL_JOY_B)
    js = 1;
   if (cselect_row < MJSTICK_1)
   {
    for (i = 0; i < joy[js].num_buttons; i ++)
    {
     if (joy[js].button [i].b)
     {
      options.joy_button [js] [cselect_row] = i;
      key_wait = 50;
      cselect_selecting = 0;
      return 0;
     }
    }
   }
    else
    {
     int sa = -1;
     switch(cselect_row)
     {
      case MJSTICK_1: sa = 0;
      case MJSTICK_2: if (sa == -1) sa = 1;
      case MJSTICK_3: if (sa == -1) sa = 2;
       if (menu_command(MC_SELECT))
       {
        cselect_selecting = 0;
        key_wait = 50;
        return 0;
       }
       if (menu_command(MC_LEFT))
       {
        options.joy_stick [js] [sa] --;
        if (options.joy_stick [js] [sa] < -1)
         options.joy_stick [js] [sa] = joy[js].num_sticks - 1;
        key_wait = 30;
        return 0;
       }
       if (menu_command(MC_RIGHT))
       {
        options.joy_stick [js] [sa] ++;
        if (options.joy_stick [js] [sa] >= joy[js].num_sticks)
         options.joy_stick [js] [sa] = -1;
        key_wait = 30;
        return 0;
       }
       break;
      case MJAXIS_1: sa = 0;
      case MJAXIS_2: if (sa == -1) sa = 1;
      case MJAXIS_3: if (sa == -1) sa = 2;
       if (menu_command(MC_SELECT))
       {
        cselect_selecting = 0;
        key_wait = 50;
        return 0;
       }
       if (options.joy_stick [js] [sa] == -1)
        break;
       if (menu_command(MC_LEFT))
       {
        options.joy_axis [js] [sa] --;
        if (options.joy_axis [js] [sa] < 0)
         options.joy_axis [js] [sa] = joy[js].stick[options.joy_stick [js] [sa]].num_axis - 1;
        key_wait = 30;
        return 0;
       }
       if (menu_command(MC_RIGHT))
       {
        options.joy_axis [js] [sa] ++;
        if (options.joy_axis [js] [sa] >= joy[js].stick[options.joy_stick [js] [sa]].num_axis)
         options.joy_axis [js] [sa] = 0;
        key_wait = 30;
        return 0;
       }
       break;
     }


    }
   if (key [KEY_ESC])
   {
    key_wait = 30;
    cselect_selecting = 0;
    return 0;
   }
   return 0;
  }



  if (menu_command(MC_UP))
  {
   cselect_row --;
   if (cselect_row < 0)
    cselect_row = cselect_final_row();
   key_wait = 7;
   menu_soundwf(WAV_SELECT0, FREQ_SELECT);
  }
  if (menu_command(MC_DOWN))
  {
   cselect_row ++;
   if (cselect_row > cselect_final_row())
    cselect_row = 0;
   key_wait = 7;
   menu_soundwf(WAV_SELECT0, FREQ_SELECT);
  }
  if (menu_command(MC_LEFT)
   && cselect_row != cselect_final_row())
  {
   cselect_col --;
   if (cselect_col < CONTROL_KEY_A)
   {
    cselect_col = CONTROL_JOY_B;
    if (!options.joystick_available [1])
    {
     cselect_col = CONTROL_JOY_A;
     if (!options.joystick_available [0])
      cselect_col = CONTROL_KEY_B;
    }
   }
   if (cselect_row > cselect_final_row())
    cselect_row = cselect_final_row() - 1;
   key_wait = 7;
   menu_soundwf(WAV_SELECT0, FREQ_SELECT);
  }
  if (menu_command(MC_RIGHT)
   && cselect_row != cselect_final_row())
  {
   cselect_col ++;
   if (cselect_col == CONTROL_JOY_A
    && !options.joystick_available [0])
     cselect_col = CONTROL_JOY_B;
   if (cselect_col == CONTROL_JOY_B
    && !options.joystick_available [1])
     cselect_col = CONTROL_KEY_A;
   if (cselect_row > cselect_final_row())
    cselect_row = cselect_final_row() - 1;
   key_wait = 7;
   menu_soundwf(WAV_SELECT0, FREQ_SELECT);
  }
  if (menu_command(MC_SELECT))
  {
   if (cselect_row == cselect_final_row())
   {
    key_wait = 10;
    return 1; // finished
   }
   key_wait = 100;
   cselect_selecting = 1;
   select_cursor = 0;
  }

 }

 return 0; // not finished

}

int cselect_final_row(void)
{

 if (cselect_col == CONTROL_KEY_A
  || cselect_col == CONTROL_KEY_B)
   return CKEY_END;

 return MJBUTTONS;

}


void key_box(const char ktext [], int command, int p)
{

 rectfill(display[0], 200, 240, 400, 290, COL_OUTLINE);
 rect(display[0], 201, 241, 399, 289, COL_LGREY);

 textprintf_centre_ex(display[0], small_font, 300, 257, MENU_TEXT, -1, ktext);

 vsync();
 blit(display[0], screen, 0, 0, 0, 0, 800, 600);

 int inputted = KEY_ESC;

 int i;
 int anykey = 0;

 char holding = 1;

 do
 {

  do
    {
        thing ++;
    } while (ticked == 0);
    ticked = 0;
  key_wait --;


 anykey = 0;

  for (i = KEY_A; i < KEY_CAPSLOCK + 1; i ++)
  {
   if (key [i])
   {
    anykey = 1;
   }
  }

  if (anykey == 0)
   holding = 0;


//  if (key_wait > 0)
//   continue;


  if (holding == 0)
  {

  for (i = KEY_A; i < KEY_CAPSLOCK + 1; i ++)
  {
   if (key [i])
   {
    inputted = i;
    holding = 1;
   }
  }
 }
//   while(acceptable_char(inputted) == 0);
  if (acceptable_char(inputted) != 0)
   break;

 } while (TRUE);


// PP.ckey [command] = inputted;

 key_wait = 10;

}

#define OPTION_X 100
#define OPTION_Y 150
#define CENTRE_X 150

void set_options(void)
{

 int i, anykey = 0, y;

 int option_select = 0;

 while (TRUE)
 {

 rectfill(display[0], OPTION_X, OPTION_Y, OPTION_X + 200, OPTION_Y + 220, COL_BOX1);
 rect(display[0], OPTION_X, OPTION_Y, OPTION_X + 200, OPTION_Y + 220, COL_BOX2);
 //rect(display[0], 201, 241, 399, 509, COL_LGREY);

 rectfill(display[0], OPTION_X + 1, OPTION_Y + 15, OPTION_X + 200 - 1, OPTION_Y + 42, COL_BOX2);
 textprintf_centre_ex(display[0], small_font, CENTRE_X, OPTION_Y + 20, MENU_TEXT, -1, "options");

 rectfill(display[0], OPTION_X + 1, OPTION_Y + 57 + option_select * 20, OPTION_X + 200 - 1, OPTION_Y + 75 + option_select * 20, COL_BOX2);
// hline(display[0], 202, 298 + option_select * 20, 398, TRANS_GREEN2);
// hline(display[0], 202, 320 + option_select * 20, 398, TRANS_GREEN2);
 y = OPTION_Y + 60;

 textprintf_ex(display[0], small_font, CENTRE_X, y, MENU_TEXT, -1, "effects volume - %i", options.sfx_volume);
 y += 20;
 textprintf_ex(display[0], small_font, CENTRE_X, y, MENU_TEXT, -1, "music volume - %i", options.ambience_volume);
 y += 20;

 if (options.windowed == 0)
  textprintf_ex(display[0], small_font, CENTRE_X, y, MENU_TEXT, -1, "mode - fullscreen");
   else
    textprintf_ex(display[0], small_font, CENTRE_X, y, MENU_TEXT, -1, "mode - windowed");
 y += 20;

 if (options.run_vsync == 0)
  textprintf_ex(display[0], small_font, CENTRE_X, y, MENU_TEXT, -1, "vsync - off");
   else
    textprintf_ex(display[0], small_font, CENTRE_X, y, MENU_TEXT, -1, "vsync - on");
 y += 20;

 if (options.fix_camera_angle == 0)
  textprintf_ex(display[0], small_font, CENTRE_X, y, MENU_TEXT, -1, "camera angle - follow");
   else
    textprintf_ex(display[0], small_font, CENTRE_X, y, MENU_TEXT, -1, "camera angle - fixed");
/* y += 20;

 if (options.joystick_dual == 0)
  textprintf_ex(display[0], small_font, CENTRE_X, y, MENU_TEXT, -1, "detect dual sticks - off");
   else
    textprintf_ex(display[0], small_font, CENTRE_X, y, MENU_TEXT, -1, "detect dual sticks - on");*/
 y += 20;

/*
 if (options.joystick)
 {
  textprintf_ex(display[0], small_font, CENTRE_X, OPTION_Y + 140, MENU_TEXT, -1, "fire 1 - button %i", options.joy_button [0] + 1);
  textprintf_ex(display[0], small_font, CENTRE_X, OPTION_Y + 160, MENU_TEXT, -1, "fire 2 - button %i", options.joy_button [1] + 1);
  textprintf_ex(display[0], small_font, CENTRE_X, OPTION_Y + 180, MENU_TEXT, -1, "engine - button %i", options.joy_button [2] + 1);
  textprintf_ex(display[0], small_font, CENTRE_X, OPTION_Y + 200, MENU_TEXT, -1, "brake - button %i", options.joy_button [3] + 1);
//  textprintf_ex(display[0], small_font, 220, 420, MENU_TEXT, -1, "fire 3 - joystick %i", options.joy_button [3] + 1);
//  textprintf_ex(display[0], small_font, 220, 440, MENU_TEXT, -1, "slow move - joystick %i", options.joy_button [2] + 1);

 }
  else
  {
   textprintf_ex(display[0], small_font, CENTRE_X, OPTION_Y + 140, MENU_TEXT, -1, "no joystick");
   TRANS_MODE
   rectfill(display[0], CENTRE_X - 1, OPTION_Y + 140 - 1, CENTRE_X + 100, OPTION_Y + 140 + 15, CONVERT_WHITE_TO_GREY);
   END_TRANS

  }
*/
  textprintf_ex(display[0], small_font, CENTRE_X, y, MENU_TEXT, -1, "exit");



 vsync();
 blit(display[0], screen, 0, 0, 0, 0, 800, 600);

 anykey = 0;


  for (i = KEY_A; i < KEY_CAPSLOCK + 1; i ++)
  {
   if (key [i])
   {
    anykey = 1;
   }
  }

  if (anykey == 0 && any_joystick_input() == 0)
   key_wait = 0;



 if (key_wait <= 0)
 {
  if (menu_command(MC_UP))
  {
   option_select --;
   if (option_select < 0)
    option_select = OPT_EXIT;
//   if (option_select == OPT_JOY4 && options.joystick == 0)
//    option_select = OPT_VSYNC;
   key_wait = 7;
   menu_soundwf(WAV_SELECT0, FREQ_SELECT);
  }
  if (menu_command(MC_DOWN))
  {
   option_select ++;
   if (option_select > OPT_EXIT)
    option_select = 0;
//   if (option_select == OPT_JOY1 && options.joystick == 0)
//    option_select = OPT_EXIT;
   key_wait = 7;
   menu_soundwf(WAV_SELECT0, FREQ_SELECT);
  }

  if (menu_command(MC_LEFT))
  {
   if (option_select == OPT_SFX_VOLUME)
   {
    options.sfx_volume -= 10;
    if (options.sfx_volume < 0)
     options.sfx_volume = 0;
    menu_soundwf(WAV_SELECT0, FREQ_SELECT1);
   }
   if (option_select == OPT_MUSIC_VOLUME)
   {
    options.ambience_volume -= 10;
    if (options.ambience_volume < 0)
     options.ambience_volume = 0;
    menu_soundwf(WAV_SELECT0, FREQ_SELECT1);
   }
/*
   if (option_select == OPT_JOY1)
   {
    if (options.joy_button [0] == 0)
     options.joy_button [0] = joy[0].num_buttons - 1;
      else
       options.joy_button [0] --;
    play_basicwfv(WAV_SELECT0, FREQ_SELECT, VOL_SELECT);
   }
   if (option_select == OPT_JOY2)
   {
    if (options.joy_button [1] == 0)
     options.joy_button [1] = joy[0].num_buttons - 1;
      else
       options.joy_button [1] --;
    play_basicwfv(WAV_SELECT0, FREQ_SELECT, VOL_SELECT);
   }
   if (option_select == OPT_JOY3)
   {
    if (options.joy_button [2] == 0)
     options.joy_button [2] = joy[0].num_buttons - 1;
      else
       options.joy_button [2] --;
    play_basicwfv(WAV_SELECT0, FREQ_SELECT, VOL_SELECT);
   }
   if (option_select == OPT_JOY4)
   {
    if (options.joy_button [3] == 0)
     options.joy_button [3] = joy[0].num_buttons - 1;
      else
       options.joy_button [3] --;
    play_basicwfv(WAV_SELECT0, FREQ_SELECT, VOL_SELECT);
   }*/
/*   if (option_select == OPT_JOYSLOW)
   {
    if (options.joy_button [2] == 0)
     options.joy_button [2] = joy[0].num_buttons - 1;
      else
       options.joy_button [2] --;
   }
   if (option_select == OPT_JOY3)
   {
    if (options.joy_button [3] == 0)
     options.joy_button [3] = joy[0].num_buttons - 1;
      else
       options.joy_button [3] --;
   }*/
   key_wait = 7;
  }

  if (menu_command(MC_RIGHT))
  {
   if (option_select == OPT_SFX_VOLUME)
   {
    options.sfx_volume += 10;
    if (options.sfx_volume > 100)
     options.sfx_volume = 100;
    menu_soundwf(WAV_SELECT0, FREQ_SELECT1);
   }
   if (option_select == OPT_MUSIC_VOLUME)
   {
    options.ambience_volume += 10;
    if (options.ambience_volume > 100)
     options.ambience_volume = 100;
    menu_soundwf(WAV_SELECT0, FREQ_SELECT1);
   }
/*
   if (option_select == OPT_JOY1)
   {
    if (options.joy_button [0] >= joy[0].num_buttons - 1)
     options.joy_button [0] = 0;
      else
       options.joy_button [0] ++;
    play_basicwfv(WAV_SELECT0, FREQ_SELECT, VOL_SELECT);
   }
   if (option_select == OPT_JOY2)
   {
    if (options.joy_button [1] >= joy[0].num_buttons - 1)
     options.joy_button [1] = 0;
      else
       options.joy_button [1] ++;
    play_basicwfv(WAV_SELECT0, FREQ_SELECT, VOL_SELECT);
   }
   if (option_select == OPT_JOY3)
   {
    if (options.joy_button [2] >= joy[0].num_buttons - 1)
     options.joy_button [2] = 0;
      else
       options.joy_button [2] ++;
    play_basicwfv(WAV_SELECT0, FREQ_SELECT, VOL_SELECT);
   }
   if (option_select == OPT_JOY4)
   {
    if (options.joy_button [3] >= joy[0].num_buttons - 1)
     options.joy_button [3] = 0;
      else
       options.joy_button [3] ++;
    play_basicwfv(WAV_SELECT0, FREQ_SELECT, VOL_SELECT);
   }*/
/*   if (option_select == OPT_JOY3)
   {
    if (options.joy_button [3] >= joy[0].num_buttons - 1)
     options.joy_button [3] = 0;
      else
       options.joy_button [3] ++;
   }
   if (option_select == OPT_JOYSLOW)
   {
    if (options.joy_button [2] >= joy[0].num_buttons - 1)
     options.joy_button [2] = 0;
      else
       options.joy_button [2] ++;
   }*/
   key_wait = 7;
  }

  if (key [KEY_ESC])
   break;

  if (menu_command(MC_SELECT))
  {
   if (option_select == OPT_EXIT)
   {
    menu_soundwf(WAV_SELECT1, FREQ_SELECT1);
    break;
   }
  }

  if (menu_command(MC_SELECT) || menu_command(MC_LEFT) || menu_command(MC_RIGHT))
  {
   if (option_select == OPT_MODE)
   {
    options.windowed ^= 1;
    key_wait = 7;
    menu_soundwf(WAV_SELECT0, FREQ_SELECT1);
   }
   if (option_select == OPT_VSYNC)
   {
    options.run_vsync ^= 1;
    key_wait = 7;
    menu_soundwf(WAV_SELECT0, FREQ_SELECT1);
   }
   if (option_select == OPT_CAMERA)
   {
    options.fix_camera_angle ^= 1;
    key_wait = 7;
    menu_soundwf(WAV_SELECT0, FREQ_SELECT1);
   }
/*   if (option_select == OPT_STICK)
   {
    options.joystick_dual ^= 1;
    key_wait = 7;
    menu_soundwf(WAV_SELECT0, FREQ_SELECT1);
   }*/
  }


/*   if (menu_select == MENU_KEYS)
   {
    key_wait = 10;
    define_keys();
    key_wait = 10;
   }

   if (menu_select == MENU_OPTIONS)
   {
    key_wait = 10;
    set_options();
    key_wait = 10;
   }*/



 }
//  else
   key_wait --;




    do
    {
        thing ++;
    } while (ticked == 0);
    ticked = 0;

 vsync();
 blit(display[0], screen, 0, 0, 0, 0, 800, 600);



 };

 set_config_int("Misc", "sfx_volume", options.sfx_volume);
 set_config_int("Misc", "ambience_volume", options.ambience_volume);
// if (options.joystick)
// {
//  set_config_int("Misc", "joy_button_1", options.joy_button [0]);
  //set_config_int("Misc", "joy_button_2", options.joy_button [1]);
//  set_config_int("Misc", "joy_button_slow", options.joy_button [2]);
//  set_config_int("Misc", "joy_button_3", options.joy_button [3]);
// }
 set_config_int("Misc", "vsync", options.run_vsync);
 set_config_int("Misc", "Windowed", options.windowed);
 set_config_int("Misc", "fix_camera_angle", options.fix_camera_angle);
// set_config_int("Misc", "joystick_dual", options.joystick_dual);


}

void menu_soundwf(int wv, int f)
{

    int v = VOL_SELECT;

    v *= options.sfx_volume;
    v /= 100;

    play_basicwfv(wv, f, v);

}


int acceptable_char(int scode)
{

 switch(scode)
 {
  case KEY_ESC:
   key_wait = 7;
   return 0;
  case KEY_ENTER:
  case KEY_ENTER_PAD:
   key_wait = 7;
   return 0;
 }
 return 1;

}



void aabox(int x1, int y1, int x2, int y2)
{

 TRANS_MODE
 rectfill(display[0], x1, y1, x2, y2, TRANS_BLUE1);
 rect(display[0], x1, y1, x2, y2, TRANS_BLUE3);
 rect(display[0], x1 + 1, y1 - 1, x2 + 1, y2 - 1, TRANS_BLUE2);
 rect(display[0], x1 - 1, y1 + 1, x2 - 1, y2 + 1, TRANS_BLUE2);
 rect(display[0], x1 - 2, y1 + 2, x2 - 2, y2 + 2, TRANS_BLUE1);
 END_TRANS

}


void congratulations(void)
{

// clear_bitmap(screen);
 vsync();
 clear_bitmap(screen);
 clear_bitmap(display[0]);

// arena.target_palette = BASECOL_CFLOWER;
// set_base_palette();

//   int newpal = build_new_palette(arena.change_palette, 0, 0, 0);
   vsync();
//   set_palette(palet [newpal]);

 int y;
 int y_int = 35;

 int c_count = 0;

// init_cflowers();

 do
 {

//   run_cflowers();

     y = 90;

 textprintf_centre_ex(display[0], small_font, 320, y, MENU_TEXT, -1, "V I C T O R Y ! !");

 y += y_int * 2;

 if (c_count >= 100)
  textprintf_centre_ex(display[0], small_font, 320, y, MENU_TEXT, -1, "YOU  HAVE  DEFEATED  ALL  OF  THE  HELLSPIDERS");
 y += y_int;

 if (c_count >= 200)
  textprintf_centre_ex(display[0], small_font, 320, y, MENU_TEXT, -1, "AND  SAVED  OUR  WORLD  FROM  DESTRUCTION.");
 y += y_int;

 if (c_count >= 350)
  textprintf_centre_ex(display[0], small_font, 320, y, MENU_TEXT, -1, "NICE  WORK.");

 y += y_int * 2;
/* if (c_count >= 450)
 {
  switch(arena.difficulty)
  {
     case 0: textprintf_centre_ex(display[0], small_font, 320, y, MENU_TEXT, -1, "BUT  CAN  YOU  DEFEAT  A  MORE  CHALLENGING  LEVEL?"); break;
     case 1: textprintf_centre_ex(display[0], small_font, 320, y, MENU_TEXT, -1, "YOUR  ACHIEVEMENT  IS  IMPRESSIVE!"); break;
     case 2: textprintf_centre_ex(display[0], small_font, 320, y, MENU_TEXT, -1, "SURELY THIS HAS BEEN PUNISHMENT ENOUGH."); break;
  }
 }*/

 if (c_count >= 550)
 {
  y += y_int * 2;
  textprintf_centre_ex(display[0], small_font, 320, y, MENU_TEXT, -1, "press space to continue");

  TRANS_MODE
//  rectfill(display[0], 100, y, 500, y + 15, CONVERT_WHITE_TO_GREY);
  END_TRANS_MODE

  if (key [KEY_SPACE])
   break;
 }

 blit(display[0], screen, 0, 0, 0, 0, 800, 600);

 do
 {
  rest(5);
 } while (ticked == 0);

 ticked --;
 c_count ++;
 if (c_count > 30000)
  c_count = 30000;

 } while(TRUE);

 clear_bitmap(screen);
 rest(200);




 clear_bitmap(screen);
 vsync();
 clear_bitmap(display[0]);

// arena.target_palette = BASECOL_L1_SPRING;
// set_base_palette();

//   newpal = build_new_palette(arena.change_palette, 0, 0, 0);
   vsync();
//   set_palette(palet [newpal]);



}
















int mship_x;
int mship_y;
int mship_x_speed;
int mship_y_speed;
int mship_frame;
int mship_drive;
int mship_phase;
int next_move;

#define MSTARS 300

int mstar_x [MSTARS];
int mstar_y [MSTARS];
int mstar_bright1 [MSTARS];
int mstar_bright2 [MSTARS];
int mstar_speed;

#define MSHIP_BASE_X (200<<10)
#define MSHIP_BASE_Y (280<<10)


void init_menu_background(void)
{

 mship_x = MSHIP_BASE_X;
 mship_y = -500 << 10;
 mship_x_speed = 0;
 mship_phase = 0;
 mship_x_speed = 0;
 mship_y_speed = 5 << 10;
 mship_drive = 0;
 mship_frame = 0;
 next_move = 10;

 int i;

 for (i = 0; i < MSTARS; i ++)
 {
  mstar_x [i] = grand(800) << 10;
  mstar_y [i] = grand(600) << 10;
  mstar_bright1 [i] = COL_STAR2 + grand(5);
//  mstar_bright2 [i] = mstar_bright1 [i] - 2;
 }
 mstar_speed = 0;

}

void run_menu_background(void)
{


/*

 mship_x += mship_x_speed;
 mship_y += mship_y_speed;

 switch(mship_phase)
 {
  case 0:
   if (mship_y > 250 << 10)
    mship_phase = 1;
   break;
  case 1:
   if (mship_drive < 100)
    mship_drive ++;
   if (mship_frame < 100)
    mship_frame += 3;
   mship_y_speed -= 1 << 6;
   if (mship_y < 200 << 10)
    mship_phase = 2;
   mship_x_speed *= 980;
   mship_x_speed >>= 10;
   mship_y_speed *= 980;
   mship_y_speed >>= 10;
   break;
  case 2:
   if (mship_drive < 100)
    mship_drive ++;
   if (mship_frame < 100)
    mship_frame += 3;


   next_move --;
   if (next_move <= 0)
   {
    if (mship_x <= MSHIP_BASE_X)
     mship_x_speed += grand(1000);
    if (mship_x > MSHIP_BASE_X)
     mship_x_speed -= grand(1000);
//    if (mship_y <= MSHIP_BASE_Y)
//     mship_y_speed += grand(1000);
//    if (mship_y > MSHIP_BASE_Y)
     mship_y_speed -= grand(1000);
    next_move = 10 + grand(50);
   }
   if (mship_y < 150 << 10)
    mship_phase = 3;
   mship_x_speed *= 980;
   mship_x_speed >>= 10;
   mship_y_speed *= 980;
   mship_y_speed >>= 10;
   break;
  case 3:
   mship_drive -= 4;
   if (mship_drive < 0)
    mship_drive = 0;
   mship_frame -= 3;
   if (mship_frame < 0)
    mship_frame = 0;
   mship_y_speed += 1 << 8;
   if (mship_y > 300 << 10)
    mship_phase = 2;
   mship_x_speed *= 980;
   mship_x_speed >>= 10;
   mship_y_speed *= 980;
   mship_y_speed >>= 10;
   break;

 }
*/
 mstar_speed += 50;
 if (mstar_speed > 4 << 10)
  mstar_speed = 4 << 10;

 int i;

 for (i = 0; i < MSTARS; i ++)
 {
  mstar_y [i] += mstar_speed;
  if (mstar_y [i] > 610 << 10)
  {
   mstar_x [i] = grand(800) << 10;
   mstar_y [i] = -20 << 10;
   mstar_bright1 [i] = COL_STAR2 + grand(5);
//   mstar_bright2 [i] = mstar_bright1 [i] - 2;
  }
 }

}

void display_menu_background(void)
{

 int i;

 for (i = 0; i < MSTARS; i ++)
 {
  vline(display[0], mstar_x [i] >> 10, mstar_y [i] >> 10, (mstar_y [i] - mstar_speed) >> 10, mstar_bright1 [i]);
//  putpixel(display[0], mstar_x [i] >> 10, mstar_y [i] >> 10, mstar_bright1 [i]);
 }
}





void scancode_to_keyname(int scanc, char sstr [30])
{

switch(scanc)
{
case KEY_A: strcpy(sstr, " a"); break;
case KEY_B: strcpy(sstr, " b"); break;
case KEY_C: strcpy(sstr, " c"); break;
case KEY_D: strcpy(sstr, " d"); break;
case KEY_E: strcpy(sstr, " e"); break;
case KEY_F: strcpy(sstr, " f"); break;
case KEY_G: strcpy(sstr, " g"); break;
case KEY_H: strcpy(sstr, " h"); break;
case KEY_I: strcpy(sstr, " i"); break;
case KEY_J: strcpy(sstr, " j"); break;
case KEY_K: strcpy(sstr, " k"); break;
case KEY_L: strcpy(sstr, " l"); break;
case KEY_M: strcpy(sstr, " m"); break;
case KEY_N: strcpy(sstr, " n"); break;
case KEY_O: strcpy(sstr, " o"); break;
case KEY_P: strcpy(sstr, " p"); break;
case KEY_Q: strcpy(sstr, " q"); break;
case KEY_R: strcpy(sstr, " r"); break;
case KEY_S: strcpy(sstr, " s"); break;
case KEY_T: strcpy(sstr, " t"); break;
case KEY_U: strcpy(sstr, " u"); break;
case KEY_V: strcpy(sstr, " v"); break;
case KEY_W: strcpy(sstr, " w"); break;
case KEY_X: strcpy(sstr, " x"); break;
case KEY_Y: strcpy(sstr, " y"); break;
case KEY_Z: strcpy(sstr, " z"); break;
case KEY_0: strcpy(sstr, " 0"); break;
case KEY_1: strcpy(sstr, " 1"); break;
case KEY_2: strcpy(sstr, " 2"); break;
case KEY_3: strcpy(sstr, " 3"); break;
case KEY_4: strcpy(sstr, " 4"); break;
case KEY_5: strcpy(sstr, " 5"); break;
case KEY_6: strcpy(sstr, " 6"); break;
case KEY_7: strcpy(sstr, " 7"); break;
case KEY_8: strcpy(sstr, " 8"); break;
case KEY_9: strcpy(sstr, " 9"); break;
case KEY_0_PAD: strcpy(sstr, "Pad Ins"); break;
case KEY_1_PAD: strcpy(sstr, " Pad 1"); break;
case KEY_2_PAD: strcpy(sstr, " Pad 2"); break;
case KEY_3_PAD: strcpy(sstr, " Pad 3"); break;
case KEY_4_PAD: strcpy(sstr, " Pad 4"); break;
case KEY_5_PAD: strcpy(sstr, " Pad 5"); break;
case KEY_6_PAD: strcpy(sstr, " Pad 6"); break;
case KEY_7_PAD: strcpy(sstr, " Pad 7"); break;
case KEY_8_PAD: strcpy(sstr, " Pad 8"); break;
case KEY_9_PAD: strcpy(sstr, " Pad 9"); break;
case KEY_F1: strcpy(sstr, " F1"); break;
case KEY_F2: strcpy(sstr, " F2"); break;
case KEY_F3: strcpy(sstr, " F3"); break;
case KEY_F4: strcpy(sstr, " F4"); break;
case KEY_F5: strcpy(sstr, " F5"); break;
case KEY_F6: strcpy(sstr, " F6"); break;
case KEY_F7: strcpy(sstr, " F7"); break;
case KEY_F8: strcpy(sstr, " F8"); break;
case KEY_F9: strcpy(sstr, " F9"); break;
case KEY_F10: strcpy(sstr, " F10"); break;
case KEY_F11: strcpy(sstr, " F11"); break;
case KEY_F12: strcpy(sstr, " F12"); break;
case KEY_ESC: strcpy(sstr, " Esc"); break; // invalid!
case KEY_TILDE: strcpy(sstr, " `"); break;
case KEY_MINUS: strcpy(sstr, " -"); break;
case KEY_EQUALS: strcpy(sstr, " ="); break;
case KEY_BACKSPACE: strcpy(sstr, " Backspace"); break;
case KEY_TAB: strcpy(sstr, " Tab"); break;
case KEY_OPENBRACE: strcpy(sstr, " {"); break;
case KEY_CLOSEBRACE: strcpy(sstr, " }"); break;
case KEY_ENTER: strcpy(sstr, " Enter"); break;
case KEY_COLON: strcpy(sstr, " ;"); break;
case KEY_QUOTE: strcpy(sstr, " '"); break;
case KEY_BACKSLASH: strcpy(sstr, " \\"); break;
case KEY_BACKSLASH2: strcpy(sstr, " \\"); break; // ????
case KEY_COMMA: strcpy(sstr, " ,"); break;
case KEY_STOP: strcpy(sstr, " ."); break;
case KEY_SLASH: strcpy(sstr, " /"); break;
case KEY_SPACE: strcpy(sstr, " Space"); break;
case KEY_INSERT: strcpy(sstr, " Insert"); break;
case KEY_DEL: strcpy(sstr, " Delete"); break;
case KEY_HOME: strcpy(sstr, " Home"); break;
case KEY_END: strcpy(sstr, " End"); break;
case KEY_PGUP: strcpy(sstr, " Pg Up"); break;
case KEY_PGDN: strcpy(sstr, " Pg Down"); break;
case KEY_LEFT: strcpy(sstr, " Left"); break;
case KEY_RIGHT: strcpy(sstr, " Right"); break;
case KEY_UP: strcpy(sstr, " Up"); break;
case KEY_DOWN: strcpy(sstr, " Down"); break;
case KEY_SLASH_PAD: strcpy(sstr, " Pad /"); break;
case KEY_ASTERISK: strcpy(sstr, " Pad *"); break;
case KEY_MINUS_PAD: strcpy(sstr, " Pad -"); break;
case KEY_PLUS_PAD: strcpy(sstr, " Pad +"); break;
case KEY_DEL_PAD: strcpy(sstr, " Pad ."); break;
case KEY_ENTER_PAD: strcpy(sstr, " Pad Enter"); break;
case KEY_PRTSCR: strcpy(sstr, " PrScr"); break;
case KEY_PAUSE: strcpy(sstr, " Pause"); break;
case KEY_ABNT_C1: strcpy(sstr, " unknown"); break; // may as well put these in
case KEY_YEN: strcpy(sstr, " Yen"); break;
case KEY_KANA: strcpy(sstr, " Kana"); break;
case KEY_CONVERT: strcpy(sstr, " Convert"); break;
case KEY_NOCONVERT: strcpy(sstr, " NOCONVERT"); break;
case KEY_AT: strcpy(sstr, " At"); break;
case KEY_CIRCUMFLEX: strcpy(sstr, " Crcmflx"); break;
case KEY_COLON2: strcpy(sstr, " Colon2"); break;
case KEY_KANJI: strcpy(sstr, " Kanji"); break;
case KEY_LSHIFT: strcpy(sstr, " L-Shift"); break;
case KEY_RSHIFT: strcpy(sstr, " R-Shift"); break;
case KEY_LCONTROL: strcpy(sstr, " L-Ctrl"); break;
case KEY_RCONTROL: strcpy(sstr, " R-Ctrl"); break;
case KEY_ALT: strcpy(sstr, " L-Alt"); break;
case KEY_ALTGR: strcpy(sstr, " R-Alt"); break;
case KEY_LWIN: strcpy(sstr, " L-Win"); break;
case KEY_RWIN: strcpy(sstr, " R-Win"); break;
case KEY_MENU: strcpy(sstr, " Menu"); break;
case KEY_SCRLOCK: strcpy(sstr, " S-Lock"); break;
case KEY_NUMLOCK: strcpy(sstr, " N-Lock"); break;
case KEY_CAPSLOCK: strcpy(sstr, " C-Lock"); break;

default: strcpy(sstr, " unknown key"); break;

}


}

void joystick_display(void)
{

 int i;

  poll_joystick();

  int y = 20;

 textprintf_ex(display[0], small_font, 10, y, MENU_TEXT, -1, "Axis 0 - %i", joy[0].stick[0].axis[0].pos);
 y += 10;
 textprintf_ex(display[0], small_font, 10, y, MENU_TEXT, -1, "Axis 1 - %i", joy[0].stick[0].axis[1].pos);
 y += 10;
 for (i = 0; i < 5; i ++)
 {
  textprintf_ex(display[0], small_font, 10, y, MENU_TEXT, -1, "Button %i - %i", i+1, joy[0].button[i].b);
  y += 10;
 }


}

enum
{
SAVE_GAME,
LOAD_GAME
};

enum
{
SDETAIL_STAGE,
SDETAILS
};

#define SAVE_X 200
#define SAVE_TEXT_X (SAVE_X + 50)
#define SAVE_Y 150
#define SAVE_W 400
#define SAVE_H 330
#define SAVE_X2 (SAVE_X+SAVE_W)
#define SAVE_Y2 (SAVE_Y+SAVE_H)

int savefile_select;

int save_detail [5] [SDETAILS];

void savefile_display(int sl);


int load_game_menu(void)
{

 savefile_select = 0;

 do
 {

  key_wait --;

  if (key [KEY_ESC])
  {
   key_wait = 30;
   return 0;
  }

  if (key_wait <= 0)
  {

  if (menu_command(MC_SELECT))
  {
      key_wait = 7;
      if (savefile_select == 5)
      {
          menu_soundwf(WAV_SELECT0, FREQ_SELECT);
          return 0;
      }
      if (save_detail [savefile_select] [SDETAIL_STAGE] != -1)
      {
       if (load_game(savefile_select) == 0)
        return 0;
       ticked = 0;
       key_wait = 30;
       menu_soundwf(WAV_SELECT1, FREQ_SELECT1);

       run_game(); // in game.c
// the other way run_game can be called is when MENU_START is selected

       init_menu_background();
       key_wait = 100;
       ticked = 0;
       return 1;
      }
  }
  if (menu_command(MC_UP))
  {
   savefile_select = decr(savefile_select, 0, 5);
   menu_soundwf(WAV_SELECT0, FREQ_SELECT);
   key_wait = 7;
  }
  if (menu_command(MC_DOWN))
  {
   savefile_select = incr(savefile_select, 0, 5);
   menu_soundwf(WAV_SELECT0, FREQ_SELECT);
   key_wait = 7;
  }
  }

  do
  {
      rest(1);
  } while (ticked == 0);
  ticked = 0;

   savefile_display(LOAD_GAME);

 } while(TRUE);

 return 0;

}

// this function is used by both load and save routines
void savefile_display(int sl)
{


 rectfill(display[0], SAVE_X, SAVE_Y, SAVE_X2, SAVE_Y2, COL_BOX1);
 rect(display[0], SAVE_X, SAVE_Y, SAVE_X2, SAVE_Y2, COL_BOX3);

 rectfill(display[0], SAVE_X, SAVE_Y + 10, SAVE_X2, SAVE_Y + 30, COL_BOX3);

 if (sl == SAVE_GAME)
  textprintf_centre_ex(display[0], small_font, SAVE_X + (SAVE_W/2), SAVE_Y + 14, COL_WHITE, -1, "Save Game");
   else
    textprintf_centre_ex(display[0], small_font, SAVE_X + (SAVE_W/2), SAVE_Y + 14, COL_WHITE, -1, "Load Game");


 int y = SAVE_Y + 40;
 int i;

 y += 5;

 for (i = 0; i < 5; i ++)
 {

  if (i == savefile_select)
  {
    rectfill(display[0], SAVE_X, y - 4, SAVE_X2, y +39, COL_BOX2);
  }

  textprintf_ex(display[0], small_font, SAVE_TEXT_X, y, COL_BOX3, -1, "Save file %i", i+1);
  y += 15;
  if (save_detail [i] [SDETAIL_STAGE] == -1)
   textprintf_ex(display[0], small_font, SAVE_TEXT_X + 30, y, COL_BOX4, -1, "Empty");
    else
     textprintf_ex(display[0], small_font, SAVE_TEXT_X + 30, y, COL_BOX4, -1, "Stage %i", save_detail [i] [SDETAIL_STAGE]);
  y += 30;


 }

 y += 15;

 if (savefile_select == 5)
  rectfill(display[0], SAVE_X, y - 3, SAVE_X2, y + 17, COL_BOX2);

 textprintf_ex(display[0], small_font, SAVE_TEXT_X, y, MENU_TEXT, -1, "exit");

 vsync();
 blit(display[0], screen, 0, 0, 0, 0, 800, 600);

}


// called from main.c during startup
void fill_sdetails(void)
{

 int i;
 char sname [20];

 for (i = 0; i < 5; i++)
 {
  switch(i)
  {
      case 0: strcpy(sname, "Save1"); break;
      case 1: strcpy(sname, "Save2"); break;
      case 2: strcpy(sname, "Save3"); break;
      case 3: strcpy(sname, "Save4"); break;
      case 4: strcpy(sname, "Save5"); break;
  }

  save_detail [i] [SDETAIL_STAGE] = get_config_int(sname, "Stage", -1);

 }


}

// call this when saving a game
void update_sdetails(int f)
{


 char sname [20];

  switch(f)
  {
      case 0: strcpy(sname, "Save1"); break;
      case 1: strcpy(sname, "Save2"); break;
      case 2: strcpy(sname, "Save3"); break;
      case 3: strcpy(sname, "Save4"); break;
      case 4: strcpy(sname, "Save5"); break;
  }

  save_detail [f] [SDETAIL_STAGE] = arena.stage;

  set_config_int(sname, "Stage", save_detail [f] [SDETAIL_STAGE]);


}



