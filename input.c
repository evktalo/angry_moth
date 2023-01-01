

#include "config.h"

#include "allegro.h"

#include <math.h>
#include <stdio.h>

#include "globvars.h"

#include "stuff.h"

#include "palette.h"
#include "display.h"
#include "level.h"
#include "bullet.h"
#include "cloud.h"
#include "ship.h"

#include "sound.h"

#define SCREENSHOT

#ifdef SCREENSHOT
#include <string.h>
extern RGB palet [256];

#endif

void player_fires(int p, int f);
void player_fires_secondary(int p, int w);
void run_player_secondary(int p, int w, int keypress);
void fire_rockets(int p);
void fire_missiles(int p);
int player_acquire_weapon_target(int p, int x, int y, int range, int exclude_type);
void player_gets_a_new_target(int p, int a, int e);
int player_acquire_af_missile_target(int p, int x, int y, int range, int exclude_type);
int player_acquire_deflect_target(int p, int x, int y, int range, int exclude_type);
void player_acquire_target(int p);

void set_command(int p, int w, int com);
void set_wing_command(int p, int w, int com);
void get_fighter_for_wing(int p, int w, int want_class);
void reinforce_wing(int p, int w);
void call_up_to_wing(int p, int w, int want_class);
void dismiss_wing(int p, int w);


int worm_angle_even [6] =
{
ANGLE_2 - ANGLE_16,
ANGLE_2 + ANGLE_16,
ANGLE_2 - ANGLE_8,
ANGLE_2 + ANGLE_8,
ANGLE_2 - ANGLE_4,
ANGLE_2 + ANGLE_4

};

int worm_angle_odd [5] =
{
ANGLE_2,
ANGLE_2 - ANGLE_8 + ANGLE_32,
ANGLE_2 + ANGLE_8 - ANGLE_32,
ANGLE_2 - ANGLE_4 + ANGLE_32,
ANGLE_2 + ANGLE_4 - ANGLE_32

};

void get_input(int p)
{

// int c;

#ifdef SCREENSHOT

static int scrs = 0;
static int sshot_counter = 0;


 char sfile [20];
 char istr [20];

 if (sshot_counter > 0)
  sshot_counter --;

 if (key [KEY_F1] && sshot_counter <= 0)
 {
  BITMAP *scrshot_bmp;
   scrshot_bmp = create_bitmap(800, 600);
   blit(screen, scrshot_bmp, 0,0,0,0,800,600);

  strcpy(sfile, "sshot");
  {
    char buf[20];
    sprintf(buf, "%d", scrs);
    strcat(sfile, buf);
  }
  strcat(sfile, ".bmp");
  save_bitmap(sfile, scrshot_bmp, palet);
  rectfill(screen, 0, 0, 800, 600, COL_WHITE);
//  clear_to_color(screen, COL_WHITE);
  scrs ++;
  sshot_counter = 15;
  destroy_bitmap(scrshot_bmp);
 }

#endif

#define DEBUGGING

#ifdef DEBUGGING
/*
 if (key [KEY_Q])
 {
    player[p].x_speed += xpart(player[p].angle, 1000);
    player[p].y_speed += ypart(player[p].angle, 1000);
 }

 if (key [KEY_W])
  player[p].shield_time = 150;
*/
if (key [KEY_F5])
 arena.debug_invulnerable = 1;

#endif


    if (player[p].respawning > 0)
    {
     player[p].respawning --;
     return;
    }


    int xc = 0, yc = 0;

//  int i;

   char key_left = 0;
   char key_right = 0;
   char key_left2 = 0;
   char key_right2 = 0;
   char key_up = 0;
   char key_down = 0;
   char key_fire1 = 0;
   char key_fire2 = 0;
   char key_fire3 = 0;
   char key_fire4 = 0;
   char key_command = 0;

 if (PP.control == CONTROL_JOY_A
  || PP.control == CONTROL_JOY_B)
 {
   poll_joystick();

   int js = PP.control - CONTROL_JOY_A;

   key_fire1 = joy[js].button[options.joy_button [js] [JBUTTON_FIRE1]].b;
   key_fire2 = joy[js].button[options.joy_button [js] [JBUTTON_FIRE2]].b;
   key_fire3 = joy[js].button[options.joy_button [js] [JBUTTON_FIRE3]].b;
   key_fire4 = joy[js].button[options.joy_button [js] [JBUTTON_FIRE4]].b;
   key_left2 = joy[js].button[options.joy_button [js] [JBUTTON_LEFT2]].b;
   key_right2 = joy[js].button[options.joy_button [js] [JBUTTON_RIGHT2]].b;
   key_command = joy[js].button[options.joy_button [js] [JBUTTON_COMMAND]].b;

/*   key_up = 0;
   if (joy[0].button[options.joy_button [2]].b)
    key_up = 1;

   key_down = 0;
   if (joy[0].button[options.joy_button [3]].b)
    key_down = 1;*/

//   if (joy[js].num_sticks > 0 && options.joystick_dual)
   if (options.joy_stick [js] [2] != -1
    && options.joy_axis [js] [2] != -1)
   {
    xc = joy[js].stick[options.joy_stick [js] [2]].axis[options.joy_axis [js] [2]].pos;

    if (xc != 0)
    {
      if (xc < -60)
       key_left2 = 1;
      if (xc > 60)
       key_right2 = 1;
    }
   }


   if (options.joy_stick [js] [1] != -1
    && options.joy_axis [js] [1] != -1)
    {
     xc = joy[js].stick[options.joy_stick [js] [1]].axis[options.joy_axis [js] [1]].pos;
    }
   if (options.joy_stick [js] [0] != -1
    && options.joy_axis [js] [0] != -1)
    {
     yc = joy[js].stick[options.joy_stick [js] [0]].axis[options.joy_axis [js] [0]].pos;
    }

   if (xc != 0 || yc != 0)
   {
     if (yc < -60)
      key_up = 1;
//     if (yc > 100)
//      key_down = 1;
     if (xc < -60)
      key_left = 1;
     if (xc > 60)
      key_right = 1;
   }

/*
// This is all still set up for direct movement, not inertia
   xc = joy[0].stick[options.joy_stick].axis[0].pos * options.joy_sensitivity;
   yc = joy[0].stick[options.joy_stick].axis[1].pos * options.joy_sensitivity;

   int jangle = radians_to_angle(atan2(yc, xc));
   jangle &= 1023;
   if (xc == 0 && yc == 0)
    jangle = ANGLE_4;
   int dist = hypot(yc, xc);

   if (dist > 6000)
    dist = 6000;

   if (joy[0].button[options.joy_button [2]].b)
    dist /= 2;

   xc = xpart(jangle, dist);
   yc = ypart(jangle, dist);

  if (xc < -6000)
   xc = -6000;
  if (yc < -6000)
   yc = -6000;
  if (xc > 6000)
   xc = 6000;
  if (yc > 6000)
   yc = 6000;


     player[p].x += xc;
     player[p].y += yc;
     if (player[p].x < X_MIN)
       player[p].x = X_MIN;
     if (player[p].y < Y_MIN)
       player[p].y = Y_MIN;
     if (player[p].x > X_MAX)
       player[p].x = X_MAX;
     if (player[p].y > Y_MAX)
       player[p].y = Y_MAX;

*/
 }
  else
  {

   int kb = PP.control;
// can assume PP.control will be 0 or 1.

   if (key [options.ckey [kb] [CKEY_LEFT]])
    key_left = 1;
   if (key [options.ckey [kb] [CKEY_RIGHT]])
    key_right = 1;
   if (key [options.ckey [kb] [CKEY_LEFT2]])
    key_left2 = 1;
   if (key [options.ckey [kb] [CKEY_RIGHT2]])
    key_right2 = 1;
   if (key [options.ckey [kb] [CKEY_UP]])
    key_up = 1;
//   if (key [options.ckey [kb] [CKEY_DOWN]])
//    key_down = 1;
   if (key [options.ckey [kb] [CKEY_FIRE1]])
    key_fire1 = 1;
   if (key [options.ckey [kb] [CKEY_FIRE2]])
    key_fire2 = 1;
   if (key [options.ckey [kb] [CKEY_FIRE3]])
    key_fire3 = 1;
   if (key [options.ckey [kb] [CKEY_FIRE4]])
    key_fire4 = 1;
   if (key [options.ckey [kb] [CKEY_COMMAND]])
    key_command = 1;

  }
//#define PLAYER_DRAG 1000

//if (key_fire2 && arena.counter%50 == 0)
//    quick_cloud(CLOUD_SMALL_SHOCK, PP.x, PP.y, 0, 0, SMALL_SHOCK_TIME, 0, 0);


#define TURNING_MAX 35
#define TURNING_UPGRADE 4
// was 24/4
//#define DRIVE_BASE 190
//#define DRIVE_SLIDE 110
//#define DRIVE_BASE 120
#define DRIVE_SLIDE 70
#define DRIVE_UPGRADE 30

 if (key_left2 && key_right2)
 {
  key_left2 = 0;
  key_right2 = 0;
 }

 if (key_up && key_down)
 {
  key_down = 0;
 }



 if (PP.command_key > 0)
 {
   if (PP.command_key == 1)
   {
    if (key_fire1 == 0)
     PP.command_key = 0;
    key_fire1 = 0;
   }
   if (PP.command_key == 2)
   {
    if (key_fire2 == 0)
     PP.command_key = 0;
    key_fire2 = 0;
   }
   if (PP.command_key == 3)
   {
    if (key_fire3 == 0)
     PP.command_key = 0;
    key_fire3 = 0;
   }
   if (PP.command_key == 4)
   {
    if (key_fire4 == 0)
     PP.command_key = 0;
    key_fire4 = 0;
   }
   if (PP.command_key == 5)
   {
    if (key_left2 == 0)
     PP.command_key = 0;
    key_left2 = 0;
   }
   if (PP.command_key == 6)
   {
    if (key_right2 == 0)
     PP.command_key = 0;
    key_right2 = 0;
   } }

// if you're using more than one engine, output is split.
// note that opposing buttons (eg both up and down pressed at same time) have already been fixed above (both set to 0).
   char engines_running = key_up + key_down + key_left2 + key_right2;
   char engine_power = 100;
   if (engines_running > 1)
    engine_power /= engines_running;
   int slide_power = (DRIVE_SLIDE * engine_power) / 100; // if DRIVE_SLIDE is changed, change it in ship.c as well.
   int base_power = (eclass[PP.type].engine_output * engine_power) / 100;

 PP.debug = engines_running;


   if (key_left)
   {
/*    if (key_fire3) // slide button
    {
     player[p].x_speed += xpart(player[p].angle - ANGLE_4, DRIVE_BASE);
     player[p].y_speed += ypart(player[p].angle - ANGLE_4, DRIVE_BASE);
     player[p].drive [2] += 2;
     if (player[p].drive [2] > 6)
      player[p].drive [2] = 6;
     if (counter % 8 == 0)
      play_effectwfv(WAV_DRIVE, 300, 20);
    }
     else*/
     {
      player[p].turn_speed -= 5;
      if (player[p].turn_speed < eclass[PP.type].turn * -1)
       player[p].turn_speed = eclass[PP.type].turn * -1;
//      player[p].flap [0] += 2;
//      if (player[p].flap [0] > 10)
//       player[p].flap [0] = 10;
     }

   }
    else
    {
     if (key_right)
     {
/*      if (key_fire3) // slide button
      {
       player[p].x_speed += xpart(player[p].angle + ANGLE_4, DRIVE_BASE);
       player[p].y_speed += ypart(player[p].angle + ANGLE_4, DRIVE_BASE);
       player[p].drive [3] += 2;
       if (player[p].drive [3] > 6)
        player[p].drive [3] = 6;
       if (counter % 8 == 0)
        play_effectwfv(WAV_DRIVE, 300, 20);
      }
       else*/
       {
        player[p].turn_speed += 5;
        if (player[p].turn_speed > eclass[PP.type].turn)
         player[p].turn_speed = eclass[PP.type].turn;
        player[p].flap [1] += 2;
        if (player[p].flap [1] > 10)
         player[p].flap [1] = 10;
       }
     }
    }

   if (!key_left && !key_right)
   {
    if (player[p].turn_speed < 0)
     player[p].turn_speed += 4;
    if (player[p].turn_speed > 0)
     player[p].turn_speed -= 4;
    if (player[p].turn_speed < 4 && player[p].turn_speed > -4)
     player[p].turn_speed = 0;

   }

      if (eclass[PP.type].move_mode == MOVE_NORMAL)
      {
       key_left2 = 0;
       key_right2 = 0;
      }

      if (key_left2) // slide button
      {
       player[p].x_speed += xpart(player[p].angle - ANGLE_4, slide_power);
       player[p].y_speed += ypart(player[p].angle - ANGLE_4, slide_power);
       player[p].drive [2] ++;
       if (player[p].drive [2] > 4 - engines_running)
        player[p].drive [2] = 4 - engines_running;
       if (counter % 8 == 0)
        play_effectwfv(WAV_DRIVE, 1000, 220);
      }
       else
       {
        if (PP.drive [2] > 0)
         PP.drive [2] --;
       }

      if (key_right2) // slide button
      {
       player[p].x_speed += xpart(player[p].angle + ANGLE_4, slide_power);
       player[p].y_speed += ypart(player[p].angle + ANGLE_4, slide_power);
       player[p].drive [3] ++;
       if (player[p].drive [3] > 4 - engines_running)
        player[p].drive [3] = 4 - engines_running;
       if (counter % 8 == 0)
        play_effectwfv(WAV_DRIVE, 900, 220);
      }
       else
       {
        if (PP.drive [3] > 0)
         PP.drive [3] --;
       }

   PP.accelerating = 0;

   if (key_up)
   {
    player[p].x_speed += xpart(player[p].angle, base_power);
    player[p].y_speed += ypart(player[p].angle, base_power);
    player[p].drive [0] ++;
    if (player[p].drive [0] > eclass[player[p].type].engine_power [0] + 1 - engines_running) //12 - (engines_running*3))
     player[p].drive [0] = eclass[player[p].type].engine_power [0] + 1 - engines_running; //12 - (engines_running*3);
    if (counter % 8 == 0)
     play_effectwfv(WAV_DRIVE, 800, 220);
    PP.accelerating = 1; // this tells player's wingmates to accelerate as well,
     // if they're close and in formation

   }
    else
    {
     PP.drive [0] --;
     if (PP.drive [0] < 0)
      PP.drive [0] = 0;
     if (key_down)
     {
      player[p].x_speed -= xpart(player[p].angle, slide_power);
      player[p].y_speed -= ypart(player[p].angle, slide_power);
      player[p].drive [1] += 2;
      if (player[p].drive [1] > 5 - engines_running)
       player[p].drive [1] = 5 - engines_running;
      if (counter % 8 == 0)
       play_effectwfv(WAV_DRIVE, 800, 220);

/*      int dragged = 1000;
      player[p].x_speed *= dragged;
      player[p].x_speed >>= 10;
      player[p].y_speed *= dragged;
      player[p].y_speed >>= 10;
      player[p].flap [0] += 2;
      if (player[p].flap [0] > 10)
       player[p].flap [0] = 10;
      player[p].flap [1] += 2;
      if (player[p].flap [1] > 10)
       player[p].flap [1] = 10;*/
     }
    }

    if (PP.just_commanded > 0)
     PP.just_commanded --;

    if (PP.commanding == -1 // has cancelled command menu but still probably has key pressed
     && !key_command)
      PP.commanding = 0;

    if (PP.commanding == 0
     && key_command)
     {
      PP.commanding = 2;
      PP.command_mode = CMODE_TACTIC;
     }

    if (PP.commanding == 1
     && key_command)
      PP.commanding = -1;

    if (PP.commanding == 2
     && !key_command)
      PP.commanding = 1;

    if (PP.commanding > 0)
    {
     if (PP.command_mode == CMODE_TACTIC)
     {
      if (key_fire1)
      {
       if (PP.wing_command == -1)
       {
        set_command(p, 0, COMMAND_ATTACK);
        set_command(p, 1, COMMAND_ATTACK);
       }
        else
         set_command(p, PP.wing_command, COMMAND_ATTACK);
       PP.command_key = 1;
      }
      if (key_fire2)
      {
       if (PP.wing_command == -1)
       {
        set_command(p, 0, COMMAND_COVER);
        set_command(p, 1, COMMAND_COVER);
       }
        else
         set_command(p, PP.wing_command, COMMAND_COVER);
       PP.command_key = 2;
      }
      if (key_fire3)
      {
       if (PP.wing_command == -1)
       {
        set_command(p, 0, COMMAND_FORM);
        set_command(p, 1, COMMAND_FORM);
       }
        else
         set_command(p, PP.wing_command, COMMAND_FORM);
       PP.command_key = 3;
      }
      if (key_left2)
      {
       if (PP.wing_size [0] == 0
        || PP.wing_size [1] == 0)
       {
        PP.wing_command = -1;
        PP.command_key = 5;
       }
        else
        {
          PP.wing_command = decr(PP.wing_command, -1, 1);
          PP.command_key = 5;
        }
      }
      if (key_right2)
      {
       if (PP.wing_size [0] == 0
        || PP.wing_size [1] == 0)
       {
        PP.wing_command = -1;
        PP.command_key = 6;
       }
        else
        {
          PP.wing_command = incr(PP.wing_command, -1, 1);
          PP.command_key = 6;
        }
      }
// now for the special commands:
      if (key_fire4)
      {
//       set_command(p, PP.wing_command, COMMAND_DEFEND);
       PP.command_mode = CMODE_WING;
       PP.command_key = 4;
      }
      key_fire1 = 0;
      key_fire2 = 0;
      key_fire3 = 0;
      key_fire4 = 0;
     }
      else // must be CMODE_WING
      {
       if (key_fire1)
       {
        if (PP.wing_size [0] == 1)
         reinforce_wing(p, 0);
          else
         {
          if (PP.wing_size [0] == 0)
           call_up_to_wing(p, 0, FCLASS_FIGHTER);
         }
         PP.command_key = 1;
         PP.just_commanded = 15;
         PP.commanding = -1;
         // doesn't do anything if the wing is full
       }
       if (key_fire2)
       {
        if (PP.wing_size [0] > 0)
         dismiss_wing(p, 0);
          else
           call_up_to_wing(p, 0, FCLASS_BOMBER);
        PP.command_key = 2;
        PP.just_commanded = 15;
        PP.commanding = -1;
       }
       if (key_fire3)
       {
        if (PP.wing_size [1] == 1)
         reinforce_wing(p, 1);
          else
         {
          if (PP.wing_size [1] == 0)
           call_up_to_wing(p, 1, FCLASS_FIGHTER);
         }
         PP.command_key = 3;
         PP.just_commanded = 15;
         PP.commanding = -1;
         // doesn't do anything if the wing is full
       }
       if (key_fire4)
       {
        if (PP.wing_size [1] > 0)
         dismiss_wing(p, 1);
          else
           call_up_to_wing(p, 1, FCLASS_BOMBER);
        PP.command_key = 4;
        PP.just_commanded = 15;
        PP.commanding = -1;
       }


      }
    }

    run_player_secondary(p, 0, key_fire2);
    run_player_secondary(p, 1, key_fire3);

   if (key_fire1
    && PP.weapon_block [0] == 0
    && PP.weapon_block [1] == 0)
     player_fires(p, 0);
/*
   if (key_fire2
    && PP.weapon_block [0] == 0
    && PP.weapon_block [2] == 0)
     player_fires(p, 1);

   if (key_fire3
    && PP.weapon_block [0] == 0
    && PP.weapon_block [1] == 0)
     player_fires(p, 2);
*/

   if (key_fire2
    && PP.weapon_block [1] == 0)
     player_fires_secondary(p, 0);

   if (key_fire3
    && PP.weapon_block [0] == 0)
     player_fires_secondary(p, 1);

/*
   if (key_fire1
    && PP.weapon_block [0] == 0
    && PP.weapon_block [1] == 0)
     player_fires(p, 0);

   if (key_fire2
    && PP.weapon_block [1] == 0)
     player_fires_secondary(p, 0);

   if (key_fire3
    && PP.weapon_block [0] == 0)
     player_fires_secondary(p, 1);
*/
   if (key_fire4)
   {
     PP.target_key_hold++;
     if (PP.target_key_hold > 30)
     {
      PP.target_key_hold = 0;
      PP.target_auto ^= 1;
      if (PP.target_auto == 1)
       indicator(WAV_SINBLIP, NOTE_1G, 70, p);
        else
         indicator(WAV_SINBLIP, NOTE_1C, 70, p);
     }
     player_acquire_target(p);
   }
    else
     PP.target_key_hold = 0;


/*
   if (key_fire2 && !key_fire1 && player[p].rocket_burst == 0 && player[p].rocket_recycle == 0)
   {
    if (PP.charge < 10000 && PP.charge > 9909)
     play_effectwfv(WAV_BLIP, 1000, 70);
    PP.charge += 90;
    if (PP.charge > 10000)
     PP.charge = 10000;

   }*/

  player[p].turning = player[p].turn_speed;
  player[p].angle += player[p].turn_speed;
  PP.angle &= ANGLE_MASK;


//  } // end keyboard control

}

void set_command(int p, int w, int com)
{

 PP.just_commanded = 15;
 PP.commanding = -1;
 int i;

//   if (com != COMMAND_DEFEND)
//   {
    for (i = 0; i < WING_SIZE; i ++)
    {
     if (PP.wing [w] [i] != -1)
      ship[TEAM_FRIEND][PP.wing [w] [i]].action = ACT_WING_AWAY;
    }
//   }



 switch(com)
 {
  case COMMAND_FORM:
   PP.wing_orders [w] = COMMAND_FORM;
   set_wing_command(p, w, COMMAND_FORM);
   break;
  case COMMAND_COVER:
   PP.wing_orders [w] = COMMAND_COVER;
   set_wing_command(p, w, COMMAND_COVER);
   break;
  case COMMAND_ATTACK:
   if (PP.target_a != TEAM_ENEMY) // i.e. is friend or no target
   {
    PP.wing_orders [w] = COMMAND_ENGAGE;
    set_wing_command(p, w, COMMAND_ENGAGE);
   }
    else
    {
     PP.wing_orders [w] = COMMAND_ATTACK;
     set_wing_command(p, w, COMMAND_ATTACK);
    }
   break;
/*  case COMMAND_DEFEND:
   PP.wing_orders = COMMAND_DEFEND;
   set_wing_command(p, COMMAND_DEFEND);
   for (w = 0; w < WING_SIZE; w ++)
   {
    if (PP.wing [w] != -1)
    {
     ship[TEAM_FRIEND][PP.wing [w]].action = ACT_AWAY;
     ship[TEAM_FRIEND][PP.wing [w]].mission = MISSION_GUARD;
     fighter_find_wship_to_guard(TEAM_FRIEND, PP.wing [w] [i], ship[TEAM_FRIEND][PP.wing [w]].x, ship[TEAM_FRIEND][PP.wing [w]].y);
    }
   }
   break;
*/
 }

}


void set_wing_command(int p, int w, int com)
{

 int i;

 for (i = 0; i < WING_SIZE; i ++)
 {
  if (PP.wing [w] [i] != -1)
  {
   ship[TEAM_FRIEND][PP.wing [w] [i]].player_command = com;
   ship[TEAM_FRIEND][PP.wing [w] [i]].target_range = ship[TEAM_FRIEND][PP.wing [w] [i]].base_target_range;
   switch(com)
   {
    case COMMAND_FORM: ship[TEAM_FRIEND][PP.wing [w] [i]].action = ACT_WING_FORM; break;
    case COMMAND_COVER:
     ship[TEAM_FRIEND][PP.wing [w] [i]].action = ACT_WING_FORM;
     ship[TEAM_FRIEND][PP.wing [w] [i]].target_range = COVER_RANGE;
     break;
    case COMMAND_ATTACK:
     ship[TEAM_FRIEND][PP.wing [w] [i]].target = PP.target_e;
     break;
   }
  }
 }

}


void reinforce_wing(int p, int w)
{

 int want_class = FCLASS_FIGHTER;

 int i;

 for (i = 0; i < WING_SIZE; i ++)
 {
  if (PP.wing [w] [i] != -1)
  {
   want_class = eclass[ship[TEAM_FRIEND][PP.wing [w] [i]].type].fighter_class;
   break;
  }
 }

 get_fighter_for_wing(p, w, want_class);
// get_fighter_for_wing(p, w, want_class);

}

void call_up_to_wing(int p, int w, int want_class)
{

 get_fighter_for_wing(p, w, want_class);
 get_fighter_for_wing(p, w, want_class);

}



void get_fighter_for_wing(int p, int w, int want_class)
{

 int e;
 int a = TEAM_FRIEND;

 int dist;
 int closest_dist_fighter = 99999999;
 int closest_e_fighter = -1;

 for (e = 0; e < NO_SHIPS; e ++)
 {
  if (EE.type == SHIP_NONE
   || eclass[EE.type].ship_class != ECLASS_FIGHTER
   || eclass[EE.type].fighter_class != want_class
   || EE.player_leader != -1 // already in a formation
   || EE.formation_size > 0) // can't call up formation leaders - too complicated
   continue;

  dist = abs(PP.y - EE.y) + abs(PP.x - EE.x);

  if (dist > closest_dist_fighter)
   continue; // do a quick and rough test first before we go into hypot:
  dist = hypot(PP.y - EE.y, PP.x - EE.x);
  if (dist < closest_dist_fighter)
  {
    closest_dist_fighter = dist;
    closest_e_fighter = e;
  }
 }

 if (closest_e_fighter == -1)
  return; // sorry, can't find one

 e = closest_e_fighter;

 if (EE.leader != -1)
  ship[a][EE.leader].formation_size --;

 EE.player_leader = p;
 EE.leader = -1;
 EE.action = ACT_WING_FORM;
 EE.mission = MISSION_PLAYER_WING;
 EE.player_command = PP.wing_orders [w];
 if (EE.player_command == COMMAND_COVER)
  EE.target_range = COVER_RANGE;
   else
    EE.target_range = EE.base_target_range;

 EE.formation_position = 1;
 if (PP.wing [w] [0] != -1)
  EE.formation_position = 2;
// formation_position is adjusted below because the 2nd lot needs to have position 2 and 3
 PP.wing [w] [EE.formation_position-1] = e;
 PP.wing_size [w] ++;

 if (w == 1)
  EE.formation_position += 2;


}



void dismiss_wing(int p, int w)
{

 int i;
 int e;
 int a = TEAM_FRIEND;

   for (i = 0; i < WING_SIZE; i ++)
   {
    if (PP.wing [w] [i] != -1)
    {
     e = PP.wing [w] [i];
     EE.action = ACT_AWAY;
     EE.think_count = 5;
     EE.mission = MISSION_GUARD;
     EE.player_leader = -1;
     EE.player_command = COMMAND_NONE;
     EE.target_range = EE.base_target_range;
     fighter_find_wship_to_guard(TEAM_FRIEND, e, EE.x, EE.y);
     PP.wing [w] [i] = -1;
     PP.wing_size [w] --;
    }
   }


}





void player_fires(int p, int f)
{

 if (player[p].recycle > 0
  || PP.over_recycle > 0)
  return;

 int b, i, c;
//#define DEBUG_SHOT
 int a = TEAM_FRIEND;

#ifdef DEBUG_SHOT

 if (f == 1)
 {
  for (i = 0; i < 2; i ++)
  {

  b = create_bullet(PBULLET_SHOT, a);

 if (b != -1)
 {
   BL.x = player[p].x + xpart(player[p].angle, 8000);
   BL.y = player[p].y + ypart(player[p].angle, 8000);
   if (i == 0)
   {
    BL.x += xpart(player[p].angle - ANGLE_4, 3000);
    BL.y += ypart(player[p].angle - ANGLE_4, 3000);
   }
    else
    {
     BL.x += xpart(player[p].angle + ANGLE_4, 3000);
     BL.y += ypart(player[p].angle + ANGLE_4, 3000);
    }
   BL.x_speed = player[p].x_speed + xpart(player[p].angle, 9000);
   BL.y_speed = player[p].y_speed + ypart(player[p].angle, 9000);
   BL.timeout = 30;
   BL.colour = 2;
   BL.angle = player[p].angle;
   BL.size = 5000;
   BL.damage = 300;
   BL.force = 100;


  }

  player[p].recycle = 10;


 }

  return;
 }

#endif

/*
  for (i = 0; i < 2; i ++)
  {

      a = TEAM_FRIEND;

  b = create_bullet(BULLET_SHOT, a);

 if (b != -1)
 {
   BL.x = player[p].x + xpart(player[p].angle, 8000);
   BL.y = player[p].y + ypart(player[p].angle, 8000);
   if (i == 0)
   {
    BL.x += xpart(player[p].angle - ANGLE_4, 3000);
    BL.y += ypart(player[p].angle - ANGLE_4, 3000);
   }
    else
    {
     BL.x += xpart(player[p].angle + ANGLE_4, 3000);
     BL.y += ypart(player[p].angle + ANGLE_4, 3000);
    }
   BL.x_speed = player[p].x_speed + xpart(player[p].angle, 12500 + player[p].upgrade [U_RANGE] * 1600);
   BL.y_speed = player[p].y_speed + ypart(player[p].angle, 12500 + player[p].upgrade [U_RANGE] * 1600);
   BL.timeout = 24;

   BL.colour = 0;
   BL.angle = player[p].angle;
   BL.size = 5000;
   BL.damage = 300;
   BL.force = 100;
   BL.status = 3;
   BL.status2 = 0;
   if (p == 0)
    BL.owner = -1;
   if (p == 1)
    BL.owner = -2;

    c = simple_cloud(CLOUD_BALL_COL2, BL.x, BL.y, 0, 15);
    if (c != -1)
    {
     cloud[c].x_speed = PP.x_speed;
     cloud[c].y_speed = PP.y_speed;
    }


  }
 }
*/
  PP.recycle = fighter_fire(TEAM_FRIEND, -1, p, WPN_AUTOCANNON,0,0);

//  player[p].recycle = 10; // see also draw_player and draw_other_player in display.cc for muzzle flash - change there too
  PP.mflash [0] = 10;
  PP.mflash [1] = 10;

  PP.wing_fire1 = 12;

}

// range of lock-on, from reticule:
#define AWS_MISSILE_RANGE 180
// distance of reticule from ship:
#define AWS_MISSILE_DIST 320

void player_fires_secondary(int p, int w)
{

 int x, y, b, i;
 int a = TEAM_ENEMY;

 switch(PP.weapon_type [w])
 {

  case WPN_RAIN:
   if (PP.over_recycle > 0)
    break;
   PP.weapon_status [w] = 1;
   PP.weapon_block [w] = 1;
   if (PP.weapon_status2 [w] > 5
    || PP.weapon_firing [w])
    break;
   PP.weapon_charge [w] ++;
   if (PP.weapon_charge [w] > 20)
   {
    PP.weapon_charge [w] = 0;
    PP.weapon_status2 [w] ++;
    if (PP.weapon_status2 [w] == 6)
     indicator(WAV_SINBLIP, NOTE_2G, 70, p);
      else
       indicator(WAV_SINBLIP, NOTE_2C, 70, p);
   }
   break;
  case WPN_ROCKET:
   if (PP.over_recycle > 0)
    break;
   PP.weapon_status [w] = 1;
   PP.weapon_block [w] = 1;
   if (PP.weapon_status2 [w] > 5
    || PP.weapon_firing [w])
    break;
   PP.weapon_charge [w] ++;
   if (PP.weapon_charge [w] > 20)
   {
    PP.weapon_charge [w] = 0;
    PP.weapon_status2 [w] ++;
    if (PP.weapon_status2 [w] == 6)
     indicator(WAV_SINBLIP, NOTE_2G, 70, p);
      else
       indicator(WAV_SINBLIP, NOTE_2C, 70, p);
   }
   break;
  case WPN_TORP:
   if (PP.weapon_charge [w] == 0
    && PP.over_recycle <= 1)
   {
    PP.weapon_charge [w] = 1;
    PP.weapon_block [w] = 1;
    indicator(WAV_BLIP2, NOTE_2C, 250, p);
   }
   break; // end torpedo
  case WPN_AWS_MISSILE:
   PP.wing_fire2 = 50;
//   PP.weapon_sight_visible [w] [0] = 0;
    if (//PP.weapon_target [w] [0] != TARGET_NONE &&
      PP.weapon_charge [w] <= 0
   && PP.over_recycle <= 1)
     {

/*
        a = TEAM_FRIEND;
        b = create_bullet(BULLET_AWS_MISSILE, a);

        if (b != -1)
        {
         BL.x = player[p].x + xpart(player[p].angle, 8000);
         BL.y = player[p].y + ypart(player[p].angle, 8000);
         BL.x_speed = player[p].x_speed + xpart(player[p].angle, 100);
         BL.y_speed = player[p].y_speed + ypart(player[p].angle, 100);
         BL.x2 = BL.x;
         BL.y2 = BL.y;
         BL.x3 = PP.x_speed;
         BL.y3 = PP.y_speed;
         BL.timeout = 250;
         BL.colour = 0;
         BL.angle = player[p].angle;
         BL.size = 8000;
         BL.damage = 1000; // increased to 5000 after priming
         BL.force = 900;
         BL.status = -1; // link to first cloud
         BL.status2 = 2 + grand(5);
         BL.turning = 0;
         BL.target_e = TARGET_NONE;
         BL.status2 = 10;
         if (PP.weapon_lock [w] >= 1000)
         {
          BL.target_e = PP.weapon_target [w] [0];
          if (PP.weapon_lock [w] >= 2000)
           BL.status2 = 20;
         }
         if (p == 0)
          BL.owner = -1;
         if (p == 1)
          BL.owner = -2;
          */
        PP.weapon_charge [w] = fighter_fire(TEAM_FRIEND, -1, p, WPN_AWS_MISSILE, PP.weapon_lock [w], PP.weapon_target [w] [0]);
        PP.over_recycle = PP.weapon_charge [w] + 1;


//    PP.weapon_block [w] = 0;


     }
   break;

  case WPN_LW_MISSILE:
   PP.wing_fire1 = 12;
//   PP.weapon_sight_visible [w] [0] = 0;
    if (//PP.weapon_target [w] [0] != TARGET_NONE &&
      PP.weapon_charge [w] <= 0
   && PP.over_recycle <= 0)
     {
//        a = TEAM_FRIEND;

        PP.weapon_charge [w] = fighter_fire(TEAM_FRIEND, -1, p, WPN_LW_MISSILE, PP.weapon_lock [w], PP.weapon_target [w] [0]);

        PP.weapon_target [w] [0] = TARGET_NONE;
        PP.weapon_lock [w] = 0;

//      PP.over_recycle = 50;
//        }
//    PP.weapon_block [w] = 0;


     }
   break;
  case WPN_ADV_LW_MISSILE:
   PP.wing_fire1 = 12;
    if (PP.weapon_charge [w] <= 0
   && PP.over_recycle <= 0)
     {
        PP.weapon_charge [w] = fighter_fire(TEAM_FRIEND, -1, p, WPN_ADV_LW_MISSILE, PP.weapon_lock [w], PP.weapon_target [w] [0]);
//        PP.weapon_target [w] [0] = TARGET_NONE;
//        PP.weapon_lock [w] = 0;
     }
   break;
  case WPN_HVY_LW_MISSILE:
   PP.wing_fire1 = 12;
    if (PP.weapon_charge [w] <= 0
   && PP.over_recycle <= 0)
     {
        PP.weapon_charge [w] = fighter_fire(TEAM_FRIEND, -1, p, WPN_HVY_LW_MISSILE, PP.weapon_lock [w], PP.weapon_target [w] [0]);
//        PP.weapon_target [w] [0] = TARGET_NONE;
//        PP.weapon_lock [w] = 0;
     }
   break;

  case WPN_AF_MISSILE:
   PP.wing_fire1 = 12;
//   PP.weapon_sight_visible [w] [0] = 0;
    if (//PP.weapon_target [w] [0] != TARGET_NONE &&
      PP.weapon_charge [w] <= 0
   && PP.over_recycle <= 0)
     {
        a = TEAM_FRIEND;
        b = create_bullet(BULLET_AF_MISSILE, a);

        if (b != -1)
        {
         BL.x = player[p].x + xpart(player[p].angle, 8000);
         BL.y = player[p].y + ypart(player[p].angle, 8000);
         BL.x_speed = player[p].x_speed + xpart(player[p].angle, 1000);
         BL.y_speed = player[p].y_speed + ypart(player[p].angle, 1000);
         BL.x2 = BL.x;
         BL.y2 = BL.y;
         BL.x3 = PP.x_speed;
         BL.y3 = PP.y_speed;
         BL.timeout = 250;
         BL.colour = 0;
         BL.angle = player[p].angle;
         BL.size = 8000;
         BL.damage = 4000;
         BL.force = 600;
         BL.status = -1; // link to first cloud
         BL.status2 = 2 + grand(5);
         BL.turning = 0;
         BL.target_e = TARGET_NONE;
         BL.status2 = 30;
         if (PP.weapon_lock [w] >= 20)
         {
          BL.target_e = PP.weapon_target [w] [0];
          if (PP.weapon_lock [w] >= 40)
           BL.status2 = 60;
         }
         if (p == 0)
          BL.owner = -1;
         if (p == 1)
          BL.owner = -2;
      PP.weapon_charge [w] = 200;
      PP.weapon_target [w] [0] = TARGET_NONE;
      PP.weapon_lock [w] = 0;
      play_effectwfv(WAV_WHOOSH2, 1200, 90);

//      PP.over_recycle = 50;
        }
//    PP.weapon_block [w] = 0;


     }
   break;

  case WPN_HOMING:
   x = PP.x + xpart(PP.angle, 250<<10);
   y = PP.y + ypart(PP.angle, 250<<10);
   if (PP.weapon_target [w] [0] == TARGET_NONE)
   {
    PP.weapon_target [w] [0] = player_acquire_weapon_target(p, x, y, AWS_MISSILE_RANGE<<10, ECLASS_WSHIP);
    break;
   }
   PP.weapon_block [w] = 1;
   int aws_dist;
   aws_dist = hypot(y - ship[a][PP.weapon_target [w] [0]].y, x - ship[a][PP.weapon_target [w] [0]].x);
   if (aws_dist > AWS_MISSILE_RANGE<<10)
   {
    PP.weapon_charge [w] -= (aws_dist>>10) - AWS_MISSILE_RANGE;
    if (PP.weapon_charge [w] <= 0)
    {
     PP.weapon_target [w] [0] = TARGET_NONE;
     PP.weapon_charge [w] = 0;
     PP.weapon_block [w] = 0;
     break;
    }
    break;
   }
   PP.weapon_charge [w] += AWS_MISSILE_RANGE - (aws_dist>>10);
   if (PP.weapon_charge [w] >= 2000)
    PP.weapon_charge [w] = 2000;
   break;
  case WPN_DEFLECT:
  if (PP.recycle > 0)
   break;
  PP.wing_fire1 = 12;

  a = TEAM_FRIEND;
  b = create_bullet(BULLET_SHOT, a);
  if (b != -1)
  {
   BL.x = player[p].x + xpart(player[p].angle, 8000);
   BL.y = player[p].y + ypart(player[p].angle, 8000);
   if (PP.weapon_status [w] == 0)
   {
    BL.x += xpart(player[p].angle - ANGLE_4, 4000);
    BL.y += ypart(player[p].angle - ANGLE_4, 4000);
    PP.mflash [0] = 10;
   }
    else
    {
     BL.x += xpart(player[p].angle + ANGLE_4, 4000);
     BL.y += ypart(player[p].angle + ANGLE_4, 4000);
     PP.mflash [1] = 10;
    }
   PP.weapon_status [w] ^= 1;
   BL.angle = PP.angle;
#define BSPEED 11500
   if (PP.weapon_target [w] [0] != TARGET_NONE)
   {
//    BL.angle = radians_to_angle(atan2(ship[TEAM_ENEMY][PP.weapon_target [w] [0]].y - PP.y, ship[TEAM_ENEMY][PP.weapon_target [w] [0]].x - PP.x));
    int x, y, xs, ys, dist, time;

    x = ship[TEAM_ENEMY][PP.weapon_target [w] [0]].x;
    y = ship[TEAM_ENEMY][PP.weapon_target [w] [0]].y;
    xs = ship[TEAM_ENEMY][PP.weapon_target [w] [0]].x_speed;
    ys = ship[TEAM_ENEMY][PP.weapon_target [w] [0]].y_speed;

    dist = hypot(PP.y - y, PP.x - x);
    time = dist / BSPEED;
    x += time * xs;
    y += time * ys;

    BL.angle = radians_to_angle(
     atan2(y - (PP.y + PP.y_speed * time), x - (PP.x + PP.x_speed * time)));

    BL.angle &= ANGLE_MASK;
    PP.angle &= ANGLE_MASK; // just in case

/*    int ad = PP.angle - BL.angle;

    if (ad > ANGLE_4)
     BL.angle = (PP.angle + ANGLE_4) & ANGLE_MASK;
    if (ad < -ANGLE_4)
     BL.angle = (PP.angle - ANGLE_4) & ANGLE_MASK;*/



    int ad = angle_difference_signed(PP.angle, BL.angle);

// PP.rocket_burst = ad;

    if (ad > ANGLE_8)
     BL.angle = (PP.angle + ANGLE_8) & ANGLE_MASK;
    if (ad < -ANGLE_8)
     BL.angle = (PP.angle - ANGLE_8) & ANGLE_MASK;
/*
    int ad = angle_difference_signed(BL.angle, PP.angle);

    if (ad > ANGLE_4)
     BL.angle = (PP.angle + ANGLE_4) & ANGLE_MASK;
    if (ad < -ANGLE_4)
     BL.angle = (PP.angle + ANGLE_4) & ANGLE_MASK;*/
   }
   BL.angle += grand(120);
   BL.angle -= grand(120);
   BL.angle &= ANGLE_MASK;
   BL.x_speed = player[p].x_speed + xpart(BL.angle, BSPEED);
   BL.y_speed = player[p].y_speed + ypart(BL.angle, BSPEED);
   BL.timeout = 24;
   BL.colour = 0;
//   BL.angle = PP.weapon_angle [w];
   BL.size = 5000;
   BL.damage = 300;
   BL.force = 100;
   BL.status = 3;
   BL.status2 = 0;
   if (p == 0)
    BL.owner = -1;
   if (p == 1)
    BL.owner = -2;
  }
  play_effectwfv(WAV_FIRE, 1600, 150);
   player[p].recycle = 5; // see also draw_player and draw_other_player in display.cc for muzzle flash - change there too


   break;

  case WPN_ROCKET2:
    if (PP.weapon_charge [w] <= 0)
     {
        a = TEAM_FRIEND;
        b = create_bullet(BULLET_ROCKET2, a);

        if (b != -1)
        {
         BL.x = player[p].x + xpart(player[p].angle, 8000);
         BL.y = player[p].y + ypart(player[p].angle, 8000);
         BL.x_speed = player[p].x_speed + xpart(player[p].angle, 100);
         BL.y_speed = player[p].y_speed + ypart(player[p].angle, 100);
         BL.timeout = 150;
         BL.colour = 0;
         BL.angle = player[p].angle;
         BL.size = 5000;
         BL.damage = 3200;
         BL.force = 500;
         BL.status = -1; // link to first cloud
         BL.status3 = -1;
         if (p == 0)
          BL.owner = -1;
         if (p == 1)
          BL.owner = -2;
         PP.weapon_charge [w] = 50;
//         PP.over_recycle = 51;

//         PP.over_recycle = 51;
         play_effectwfv(WAV_WHOOSH2, 1400, 80);
        }
//    PP.weapon_block [w] = 0;


     }
   break;



 case WPN_BLASTER:
   if (PP.recycle > 0)
    break;
  PP.wing_fire2 = 20;
  for (i = 0; i < 2; i ++)
  {

      a = TEAM_FRIEND;

  b = create_bullet(BULLET_BLAST, a);

 if (b != -1)
 {
   BL.x = player[p].x + xpart(player[p].angle, 8000);
   BL.y = player[p].y + ypart(player[p].angle, 8000);
   if (i == 0)
   {
    BL.x += xpart(player[p].angle - ANGLE_4, 4000);
    BL.y += ypart(player[p].angle - ANGLE_4, 4000);
   }
    else
    {
     BL.x += xpart(player[p].angle + ANGLE_4, 4000);
     BL.y += ypart(player[p].angle + ANGLE_4, 4000);
    }
   BL.x_speed = player[p].x_speed + xpart(player[p].angle, 6000);
   BL.y_speed = player[p].y_speed + ypart(player[p].angle, 6000);
   BL.timeout = 62 + grand(5);
   BL.colour = 0;
   BL.angle = player[p].angle;
   BL.size = 9000;
   BL.damage = 1000;
   BL.force = 300;
   BL.status = 5;
   BL.status2 = 0;
   if (p == 0)
    BL.owner = -1;
   if (p == 1)
    BL.owner = -2;


  }
 }

  play_effectwfv(WAV_FIRE, 1200, 150);
  player[p].recycle = 30;
  PP.mflash [0] = 15;
  PP.mflash [1] = 15;
  break;
  case WPN_WROCKET:
   if (PP.weapon_charge [w] == 0
    && PP.over_recycle <= 1)
   {
    PP.weapon_block [w] = 1;
    PP.weapon_charge [w] = -80;
    fighter_fire(TEAM_FRIEND, -1, p, WPN_WROCKET,0,0);
    PP.wing_fire2 = 20;
//    PP.weapon_charge [w] = 1;
//    indicator(WAV_BLIP2, NOTE_2C, 250, p);
   }
   break; // end torpedo
  case WPN_HROCKET:
   if (PP.weapon_charge [w] == 0
    && PP.over_recycle <= 1)
   {
    PP.weapon_charge [w] = 1;
    PP.weapon_block [w] = 1;
    indicator(WAV_BLIP2, NOTE_2C, 250, p);
   }
   break; // end torpedo


 }

}

int player_acquire_weapon_target(int p, int x, int y, int range, int exclude_type)
{

 int a = TEAM_ENEMY;
 int e;

 int dist;
 int closest_dist = range;
 int closest_e = TARGET_NONE;

 for (e = 0; e < NO_SHIPS; e ++)
 {
  if (ship[a][e].type == SHIP_NONE
   || eclass[ship[a][e].type].ship_class == exclude_type)
   continue;

  // do a quick and rough test first before we go into hypot:
//  designated target is just compared against max range because we want to choose it if it's a valid
//  target even if it's not the closest:
  if (PP.target_a == a && PP.target_e == e)
  {
   if (abs(y - ship[a][e].y) > range || abs(x - ship[a][e].x) > range)
    continue;
  }
   else
   {
    if (abs(y - ship[a][e].y) > closest_dist || abs(x - ship[a][e].x) > closest_dist)
     continue;
   }


  dist = hypot(y - ship[a][e].y, x - ship[a][e].x);
  if (dist < closest_dist
       || (PP.target_a == a && PP.target_e == e && dist < range))
  {
    closest_dist = dist;
    closest_e = e;
    if (PP.target_a == a && PP.target_e == e)
     return e; // as long as target is a valid choice, choose it
  }

 }

// if (closest_e == TARGET_NONE)
  return closest_e;

}


void player_acquire_target(int p)
{

 int a;
 int e;
 int x = PP.x + xpart(PP.angle, 250<<10);
 int y = PP.y + ypart(PP.angle, 250<<10);

 int dist;
 int closest_dist = 9999999;
 int closest_e = TARGET_NONE;
 int closest_a = -1;

 for (a = 0; a < 2; a ++)
 {
 for (e = 0; e < NO_SHIPS; e ++)
 {
  if (ship[a][e].type == SHIP_NONE)
   continue;

  // do a quick and rough test first before we go into hypot:
  if (abs(y - ship[a][e].y) > closest_dist
   || abs(x - ship[a][e].x) > closest_dist)
   continue;
  dist = hypot(y - ship[a][e].y, x - ship[a][e].x);
  if (dist < closest_dist)
  {
    closest_dist = dist;
    closest_e = e;
    closest_a = a;
  }

 }
 }


 player_gets_a_new_target(p, closest_a, closest_e);

/*
 if (closest_a != PP.target_a || closest_e != PP.target_e)
  PP.target_new = 10;

 PP.target_a = closest_a;
 PP.target_e = closest_e;

 int w;

 for (w = 0; w < 2; w ++)
 {
  if (PP.weapon_type [w] == WPN_AWS_MISSILE)
  {
   if (PP.target_a != TEAM_FRIEND
    && eclass[ship[PP.target_a][PP.target_e].type].ship_class == ECLASS_WSHIP
    && PP.target_e != PP.weapon_target [w] [0])
    {
     PP.weapon_target [w] [0] = PP.target_e;
     PP.weapon_lock [w] = 0;
    }
  }
  if (PP.weapon_type [w] == WPN_AF_MISSILE)
  {
   if (PP.target_a != TEAM_FRIEND
    && eclass[ship[PP.target_a][PP.target_e].type].ship_class == ECLASS_FIGHTER
    && PP.target_e != PP.weapon_target [w] [0])
    {
     PP.weapon_target [w] [0] = PP.target_e;
     PP.weapon_lock [w] = 0;
    }
  }
 }
*/
}

void player_gets_a_new_target(int p, int a, int e)
{


 if (a != PP.target_a || e != PP.target_e)
  PP.target_new = 10;

 PP.target_a = a;
 PP.target_e = e;

 if (a == -1)
  return;

 int w;

 for (w = 0; w < 2; w ++)
 {
  if (PP.weapon_type [w] == WPN_AWS_MISSILE)
  {
   if (PP.target_a != TEAM_FRIEND
    && eclass[ship[PP.target_a][PP.target_e].type].ship_class == ECLASS_WSHIP
    && PP.target_e != PP.weapon_target [w] [0])
    {
     PP.weapon_target [w] [0] = PP.target_e;
     PP.weapon_lock [w] = 0;
    }
  }
  if (PP.weapon_type [w] == WPN_AF_MISSILE)
  {
   if (PP.target_a != TEAM_FRIEND
    && eclass[ship[PP.target_a][PP.target_e].type].ship_class == ECLASS_FIGHTER
    && PP.target_e != PP.weapon_target [w] [0])
    {
     PP.weapon_target [w] [0] = PP.target_e;
     PP.weapon_lock [w] = 0;
    }
  }
  if (PP.weapon_type [w] == WPN_LW_MISSILE
   || PP.weapon_type [w] == WPN_ADV_LW_MISSILE
   || PP.weapon_type [w] == WPN_HVY_LW_MISSILE)
  {
   if (PP.target_a != TEAM_FRIEND
    && eclass[ship[PP.target_a][PP.target_e].type].ship_class == ECLASS_FIGHTER
    && PP.target_e != PP.weapon_target [w] [0])
    {
     PP.weapon_target [w] [0] = PP.target_e;
     PP.weapon_lock [w] = 0;
    }
  }
 }






 if (PP.target_new == 10)
  indicator(WAV_SINBLIP, NOTE_2G, 250, p);


}


int player_acquire_af_missile_target(int p, int x, int y, int range, int exclude_type)
{

 int a = TEAM_ENEMY;
 int e;

 int dist;
 int angle, angle_diff;
 int closest_dist = range;
 int closest_e = TARGET_NONE;

 for (e = 0; e < NO_SHIPS; e ++)
 {
  if (ship[a][e].type == SHIP_NONE
   || eclass[ship[a][e].type].ship_class == exclude_type)
   continue;

// do a quick and rough test first before we go into hypot:
//  designated target is just compared against max range because we want to choose it if it's a valid
//  target even if it's not the closest:
  if (PP.target_a == a && PP.target_e == e)
  {
   if (abs(y - ship[a][e].y) > range || abs(x - ship[a][e].x) > range)
    continue;
  }
   else
   {
    if (abs(y - ship[a][e].y) > closest_dist || abs(x - ship[a][e].x) > closest_dist)
     continue;
   }


// now test for distance:
  dist = hypot(y - ship[a][e].y, x - ship[a][e].x);
  if (dist < closest_dist
       || (PP.target_a == a && PP.target_e == e && dist < range))
  {
// target is close enough but must be within ANGLE_16 of the point 100 pixels behind player:
    angle = radians_to_angle(atan2(ship[a][e].y - PP.y + ypart(PP.angle + ANGLE_2, 100<<10),
                                    ship[a][e].x - PP.x + xpart(PP.angle + ANGLE_2, 100<<10)));
 angle &= ANGLE_MASK;
    angle_diff = angle_difference(angle, PP.angle);
// target must also be in front of player:
    if (angle_diff < ANGLE_16)
    {
     angle = radians_to_angle(atan2(ship[a][e].y - PP.y,
                                    ship[a][e].x - PP.x));
     angle &= ANGLE_MASK;
     angle_diff = angle_difference(angle, PP.angle);
// have we found a target?
     if (angle_diff < ANGLE_4)
     {
      closest_dist = dist;
      closest_e = e;
      if (PP.target_a == a && PP.target_e == e)
       return e; // as long as target is a valid choice, choose it
     }
    }
  }

 }

// if (closest_e == TARGET_NONE)
  return closest_e;

}


int player_acquire_deflect_target(int p, int x, int y, int range, int exclude_type)
{

 int a = TEAM_ENEMY;
 int e;

 int dist;
 int angle, angle_diff;
 int closest_dist = range;
 int closest_e = TARGET_NONE;

 for (e = 0; e < NO_SHIPS; e ++)
 {
  if (ship[a][e].type == SHIP_NONE
   || eclass[ship[a][e].type].ship_class == exclude_type)
   continue;

// do a quick and rough test first before we go into hypot:
//  designated target is just compared against max range because we want to choose it if it's a valid
//  target even if it's not the closest:
  if (PP.target_a == a && PP.target_e == e)
  {
   if (abs(y - ship[a][e].y) > range || abs(x - ship[a][e].x) > range)
    continue;
  }
   else
   {
    if (abs(y - ship[a][e].y) > closest_dist || abs(x - ship[a][e].x) > closest_dist)
     continue;
   }


// now test for distance:
  dist = hypot(y - ship[a][e].y, x - ship[a][e].x);
  if (dist < closest_dist
       || (PP.target_a == a && PP.target_e == e && dist < range))
  {
// target is close enough but must be within ANGLE_16 of the player's angle:
    angle = radians_to_angle(atan2(ship[a][e].y - PP.y, ship[a][e].x - PP.x));
    angle &= ANGLE_MASK;
    angle_diff = angle_difference(angle, PP.angle);
// target must also be in front of player:
    if (angle_diff < ANGLE_16 + ANGLE_32)
    {
     angle = radians_to_angle(atan2(ship[a][e].y - PP.y,
                                    ship[a][e].x - PP.x));
     angle &= ANGLE_MASK;
     angle_diff = angle_difference(angle, PP.angle);
// have we found a target?
     if (angle_diff < ANGLE_4)
     {
      closest_dist = dist;
      closest_e = e;
      if (PP.target_a == a && PP.target_e == e)
       return e; // as long as target is a valid choice, choose it
     }
    }
  }

 }

// if (closest_e == TARGET_NONE)
  return closest_e;

}



void run_player_secondary(int p, int w, int keypress)
{


 int b, x, y, angle, i;

 int a = TEAM_FRIEND;

 switch(PP.weapon_type [w])
 {
  case WPN_RAIN:
// charge = loading rocket while key pressed
// status = whether fire is being pressed
// status2 = number of rockets loaded
// firing = currently releasing loaded rockets
   if (PP.weapon_firing [w] == 1)
   {
     PP.wing_fire2 = 30;
     PP.weapon_recycle [w] --;
     if (PP.weapon_recycle [w] == 0)
     {

      fighter_fire(TEAM_FRIEND, -1, p, WPN_RAIN,0,0);

      PP.weapon_recycle [w] = 10;
      PP.weapon_status2 [w] --;
//      PP.over_recycle = 10;
      if (PP.weapon_status2 [w] == 0)
      {
       PP.weapon_firing [w] = 0;
       PP.weapon_block [w] = 0;
      }
     }
     break;
   }
   if (PP.weapon_status2 [w] == 0
    && PP.weapon_charge [w] == 0)
   {
    PP.weapon_block [w] = 0;
    break;
   }
   if (PP.weapon_status [w] == 1) // pressing button - don't fire
   {
    PP.weapon_status [w] = 0;
    break;
   }
   if (PP.weapon_status2 [w] == 0)
   {
    PP.weapon_charge [w] = 0;
    break;
   }
   PP.weapon_charge [w] = 0;
   PP.weapon_firing [w] = 1;
   PP.weapon_recycle [w] = 10;
   break;

  case WPN_ROCKET:
// charge = loading rocket while key pressed
// status = whether fire is being pressed
// status2 = number of rockets loaded
// firing = currently releasing loaded rockets
   if (PP.weapon_firing [w] == 1)
   {
     PP.wing_fire2 = 30;
     PP.weapon_recycle [w] --;
     if (PP.weapon_recycle [w] == 0)
     {

        b = create_bullet(BULLET_ROCKET, a);

        if (b != -1)
        {
         BL.x = player[p].x + xpart(player[p].angle, 8000);
         BL.y = player[p].y + ypart(player[p].angle, 8000);
         BL.x_speed = player[p].x_speed + xpart(player[p].angle, 300);
         BL.y_speed = player[p].y_speed + ypart(player[p].angle, 300);
         BL.x2 = BL.x;
         BL.y2 = BL.y;
         BL.x3 = PP.x_speed;
         BL.y3 = PP.y_speed;
         BL.timeout = 70+grand(20);
         BL.colour = 0;
         BL.angle = player[p].angle;
         BL.size = 5000;
         BL.damage = 3000;
         BL.force = 500;
         BL.turning = 0;
         BL.status = 0;
         BL.status2 = 0;
         BL.status3 = -1;
         if (p == 0)
          BL.owner = -1;
         if (p == 1)
          BL.owner = -2;
        }
      play_effectwfv(WAV_WHOOSH2, 1500, 80);
      PP.weapon_recycle [w] = 10;
      PP.weapon_status2 [w] --;
//      PP.over_recycle = 10;
      if (PP.weapon_status2 [w] == 0)
      {
       PP.weapon_firing [w] = 0;
       PP.weapon_block [w] = 0;
      }
     }
     break;
   }
   if (PP.weapon_status2 [w] == 0
    && PP.weapon_charge [w] == 0)
   {
    PP.weapon_block [w] = 0;
    break;
   }
   if (PP.weapon_status [w] == 1) // pressing button - don't fire
   {
    PP.weapon_status [w] = 0;
    break;
   }
   if (PP.weapon_status2 [w] == 0)
   {
    PP.weapon_charge [w] = 0;
    break;
   }
   PP.weapon_charge [w] = 0;
   PP.weapon_firing [w] = 1;
   PP.weapon_recycle [w] = 10;
   break;

  case WPN_WROCKET:
  if (PP.weapon_charge [w] == 0)
  {
   PP.weapon_block [w] = 0;
   break;
  }
  if (PP.weapon_charge [w] < 0)
  {
   PP.weapon_charge [w] ++;
   if (PP.weapon_charge [w] == 0)
   {
    indicator(WAV_BLIP2, NOTE_2E, 250, p);
   }
   break;
  }
/*
  if (PP.weapon_charge [w] < 40)
  {
   PP.weapon_charge [w] ++;
   if (PP.weapon_charge [w] > 30)
     PP.wing_fire2 = 20;
   return;
  }
*/
  //PP.weapon_charge [w] = 0;

/*
 for (i = 0; i < 2; i ++)
 {

  b = create_bullet(BULLET_ROCKET, a);
  angle = PP.angle + ANGLE_4;
  if (i == 1)
   angle = PP.angle - ANGLE_4;

  if (b != -1)
  {
         BL.x = player[p].x + xpart(angle, 8000);
         BL.y = player[p].y + ypart(angle, 8000);
         BL.x_speed = player[p].x_speed + xpart(angle, 200);
         BL.y_speed = player[p].y_speed + ypart(angle, 200);
         BL.x2 = BL.x;
         BL.y2 = BL.y;
         BL.x3 = PP.x_speed;
         BL.y3 = PP.y_speed;
         BL.timeout = 90+grand(20);
         BL.colour = 0;
         BL.angle = player[p].angle;
         BL.size = 5000;
         BL.damage = 700; // is *6 after priming
         BL.force = 100; // is *6 after priming
         BL.turning = 0;
         BL.status = 30;
         BL.status2 = 0;
         BL.status3 = -1;
         if (p == 0)
          BL.owner = -1;
         if (p == 1)
          BL.owner = -2;


   }
  }

  play_effectwfv(WAV_WHOOSH2, 700, 110);
*/
  break; // end wrocket

  case WPN_HROCKET:
  if (PP.weapon_charge [w] == 0)
  {
   PP.weapon_block [w] = 0;
   break;
  }
  if (PP.weapon_charge [w] < 0)
  {
   PP.weapon_charge [w] ++;
   if (PP.weapon_charge [w] == 0)
   {
    indicator(WAV_BLIP2, NOTE_2E, 250, p);
   }
   break;
  }
  if (PP.weapon_charge [w] < 40)
  {
   PP.weapon_charge [w] ++;
   if (PP.weapon_charge [w] > 30)
     PP.wing_fire2 = 20;
   return;
  }

  PP.weapon_block [w] = 1;
  PP.weapon_charge [w] = -80;
  fighter_fire(TEAM_FRIEND, -1, p, WPN_HROCKET,0,0);

  break; // end hrocket


  case WPN_TORP:
  if (PP.weapon_charge [w] == 0)
   break;
  if (PP.weapon_charge [w] < 0)
  {
   PP.weapon_charge [w] ++;
   if (PP.weapon_charge [w] == 0)
   {
    indicator(WAV_BLIP2, NOTE_2E, 250, p);
   }
   break;
  }
  if (PP.weapon_charge [w] < 40)
  {
   PP.weapon_charge [w] ++;
   if (PP.weapon_charge [w] > 30)
     PP.wing_fire2 = 50;
   return;
  }

  PP.weapon_block [w] = 0;
  PP.weapon_charge [w] = fighter_fire(TEAM_FRIEND, -1, p, WPN_TORP,0,0) * -1;

  break; // end torpedo
  case WPN_AWS_MISSILE:
   PP.weapon_sight_visible [w] [0] = 0;
   a = TEAM_ENEMY;
   if (PP.weapon_charge [w] > 0)
   {
    PP.weapon_charge [w] --;
/*    PP.weapon_lock [w] -= 200;
    if (PP.weapon_lock [w] < 0)
     PP.weapon_lock [w] = 0;*/
   }
   x = PP.x + xpart(PP.angle, AWS_MISSILE_DIST<<10);
   y = PP.y + ypart(PP.angle, AWS_MISSILE_DIST<<10);
   if (PP.weapon_target [w] [0] == TARGET_NONE)
   {
    PP.weapon_target [w] [0] = player_acquire_weapon_target(p, x, y, AWS_MISSILE_RANGE<<10, ECLASS_FIGHTER);
    if (PP.weapon_target [w] [0] == TARGET_NONE)
    {

PP.weapon_angle [w] &= ANGLE_MASK;
     if (angle_difference(PP.weapon_angle [w], ANGLE_1-ANGLE_4) < 21)
      PP.weapon_angle [w] = ANGLE_1-ANGLE_4;
       else
        PP.weapon_angle [w] =  turn_towards_angle(PP.weapon_angle [w], ANGLE_1-ANGLE_4, 20);
/*

     if (PP.weapon_angle [w] < ANGLE_12)
      PP.weapon_angle [w] -= 10;
     if (PP.weapon_angle [w] < 0)
      PP.weapon_angle [w] = 0;
     if (PP.weapon_angle [w] > ANGLE_12)
      PP.weapon_angle [w] += 10;
     if (PP.weapon_angle [w] > ANGLE_6)
      PP.weapon_angle [w] = 0;*/
    }
    break;
   }
/*
   PP.weapon_angle [w] =
   radians_to_angle(atan2(PP.y + ypart(PP.angle, 180<<10) - ship[a][PP.weapon_target [w][0]].y,
                                    PP.x + xpart(PP.angle, 180<<10) - ship[a][PP.weapon_target [w][0]].x)) - PP.angle + ANGLE_4;

   break;*/

   angle =
   radians_to_angle(atan2(PP.y + ypart(PP.angle, AWS_MISSILE_DIST<<10) - ship[a][PP.weapon_target [w][0]].y,
                                    PP.x + xpart(PP.angle, AWS_MISSILE_DIST<<10) - ship[a][PP.weapon_target [w][0]].x)) - PP.angle + ANGLE_4;
    angle &= ANGLE_MASK;
//   radians_to_angle(atan2(PP.y + ypart(PP.angle, 180<<10) - ship[a][PP.weapon_target [w][0]].y,
//                                    PP.x + xpart(PP.angle, 180<<10) - ship[a][PP.weapon_target [w][0]].x))
//                                    -  ANGLE_3 - ANGLE_6;
   if (angle_difference(PP.weapon_angle [w], angle) < 91)
    PP.weapon_angle [w] = angle;// + PP.angle;
     else
      PP.weapon_angle [w] =  turn_towards_angle(PP.weapon_angle [w], angle, 90);

      PP.weapon_angle [w] &= ANGLE_MASK;


   int aws_dist;
   aws_dist = hypot(y - ship[a][PP.weapon_target [w] [0]].y, x - ship[a][PP.weapon_target [w] [0]].x);
   if (aws_dist > (AWS_MISSILE_RANGE + 5)<<10)
   {
    PP.weapon_lock [w] -= (aws_dist>>10) - AWS_MISSILE_RANGE;
    if (PP.weapon_lock [w] <= 0)
    {
     PP.weapon_target [w] [0] = TARGET_NONE;
     PP.weapon_lock [w] = 0;
     break;
    }
    break;
   }
   int old = PP.weapon_lock [w];
   PP.weapon_lock [w] += (AWS_MISSILE_RANGE - (aws_dist>>10)) >> 1;
   if (PP.weapon_lock [w] < 0)
    PP.weapon_lock [w] = 0;
   if (PP.weapon_lock [w] >= 2000)
   {
    PP.weapon_lock [w] = 2000;
    if (old < 2000)
     indicator(WAV_SINBLIP2, NOTE_3C, 250, p);
   }
   //PP.weapon_angle [w] =
/*   angle =
   radians_to_angle(atan2(PP.y + ypart(PP.angle, 180<<10) - ship[a][PP.weapon_target [w][0]].y,
                                    PP.x + xpart(PP.angle, 180<<10) - ship[a][PP.weapon_target [w][0]].x)) - PP.angle + ANGLE_4;

//   radians_to_angle(atan2(PP.y + ypart(PP.angle, 180<<10) - ship[a][PP.weapon_target [w][0]].y,
//                                    PP.x + xpart(PP.angle, 180<<10) - ship[a][PP.weapon_target [w][0]].x))
//                                    -  ANGLE_3 - ANGLE_6;
   if (angle_difference(PP.weapon_angle [w], angle) < 41)
    PP.weapon_angle [w] = angle;// + PP.angle;
     else
      PP.weapon_angle [w] =  turn_towards_angle(PP.weapon_angle [w], angle, 40);*/
   break;


  case WPN_AF_MISSILE:
  case WPN_LW_MISSILE:
  case WPN_ADV_LW_MISSILE:
  case WPN_HVY_LW_MISSILE:
   PP.weapon_sight_visible [w] [0] = 0;
   a = TEAM_ENEMY;
   PP.weapon_angle [w] += PP.weapon_status [w];
   if (PP.weapon_charge [w] > 0)
   {
    PP.weapon_charge [w] --;
    if (PP.weapon_charge [w] == 0)
     indicator(WAV_BLIP2, NOTE_1DS, 250, p);

   }
    else
     if (PP.weapon_target [w] [0] == TARGET_NONE)
     {
      PP.weapon_lock [w] = 0;
      if ((arena.counter >> 2) & 1)
       PP.weapon_target [w] [0] = player_acquire_af_missile_target(p, PP.x, PP.y, 500 << 10, ECLASS_WSHIP);

      PP.weapon_angle [w] = 0;
      PP.weapon_status [w] *= -1;
      break;
     }




   angle =
   radians_to_angle(atan2(ship[a][PP.weapon_target [w][0]].y - (PP.y + ypart(PP.angle+ANGLE_2, 300<<10)),
                                    ship[a][PP.weapon_target [w][0]].x - (PP.x + xpart(PP.angle+ANGLE_2, 300<<10))));
    angle &= ANGLE_MASK;
   if (angle_difference(PP.angle, angle) > ANGLE_16)
   {
    PP.weapon_target [w] [0] = TARGET_NONE;
    break;
   }

   angle =
   radians_to_angle(atan2(ship[a][PP.weapon_target [w][0]].y - PP.y,
                          ship[a][PP.weapon_target [w][0]].x - PP.x));
    angle &= ANGLE_MASK;
   if (angle_difference(PP.angle, angle) > ANGLE_4)
   {
    PP.weapon_target [w] [0] = TARGET_NONE;
    break;
   }

   if (hypot(PP.y - ship[a][PP.weapon_target [w] [0]].y, PP.x - ship[a][PP.weapon_target [w] [0]].x) > 400<<10)
   {
    PP.weapon_target [w] [0] = TARGET_NONE;
    break;
   }

   if (PP.weapon_lock [w] < 40)
   {
    PP.weapon_lock [w] ++;

    if (PP.weapon_lock [w] >= 40)
    {
     PP.weapon_lock [w] = 40;
     indicator(WAV_BLIP, NOTE_2D, 220, p);
    }
   }

break;


  case WPN_HOMING:
   PP.weapon_sight_visible [w] [0] = 0;
   if (!keypress)
   {
    if (PP.weapon_target [w] [0] != TARGET_NONE
     && PP.weapon_charge [w] > 500)
     {
        b = create_bullet(BULLET_AWS_MISSILE, a);

        if (b != -1)
        {
         BL.x = player[p].x + xpart(player[p].angle, 8000);
         BL.y = player[p].y + ypart(player[p].angle, 8000);
         BL.x_speed = player[p].x_speed + xpart(player[p].angle, 4000);
         BL.y_speed = player[p].y_speed + ypart(player[p].angle, 4000);
         BL.x2 = BL.x;
         BL.y2 = BL.y;
         BL.x3 = PP.x_speed;
         BL.y3 = PP.y_speed;
         BL.timeout = 250;
         BL.colour = 0;
         BL.angle = player[p].angle;
         BL.size = 8000;
         BL.damage = 3000;
         BL.force = 900;
         BL.status = -1; // link to first cloud
         BL.status2 = 2 + grand(5);
         BL.turning = 0;
         BL.target_e = PP.weapon_target [w] [0];
         if (p == 0)
          BL.owner = -1;
         if (p == 1)
          BL.owner = -2;
        }


     }
    PP.weapon_target [w] [0] = TARGET_NONE;
    PP.weapon_charge [w] = 0;
    PP.weapon_block [w] = 0;
   }
   break;
  case WPN_DEFLECT:
   PP.weapon_sight_visible [w] [0] = 0;
   a = TEAM_ENEMY;
   if (PP.weapon_lock [w] > 0)
    PP.weapon_lock [w] -= 10;
//   PP.weapon_angle [w] += PP.weapon_status [w];
//   if (PP.weapon_target [w] [0] == TARGET_NONE)
   if (!keypress)
   {
    int old_target = PP.weapon_target [w] [0];
    if ((arena.counter >> 2) & 1)
     PP.weapon_target [w] [0] = player_acquire_deflect_target(p, PP.x, PP.y, 400 << 10, -1);
    if (old_target != PP.weapon_target [w] [0]
     && PP.weapon_target [w] [0] != TARGET_NONE)
      PP.weapon_lock [w] = 50;
//    PP.weapon_angle [w] = 0;
//    PP.weapon_status [w] *= -1;
    break;
   }

   if (!keypress)
   {
    angle = radians_to_angle(atan2(ship[a][PP.weapon_target [w][0]].y - PP.y,
                                    ship[a][PP.weapon_target [w][0]].x - PP.x));
    angle &= ANGLE_MASK;
    if (angle_difference(PP.angle, angle) > ANGLE_16)
    {
     PP.weapon_target [w] [0] = TARGET_NONE;
     break;
    }

    if (hypot(PP.y - ship[a][PP.weapon_target [w] [0]].y, PP.x - ship[a][PP.weapon_target [w] [0]].x) > 400<<10)
    {
     PP.weapon_target [w] [0] = TARGET_NONE;
     break;
    }
   }

//   PP.weapon_lock [w] ++;
//   if (PP.weapon_lock [w] > 40)
//    PP.weapon_lock [w] = 40;

 break;
 case WPN_ROCKET2:
  if (PP.weapon_charge [w] > 0)
  {
   PP.weapon_charge [w] --;
//   if (PP.weapon_charge [w] == 0)
//    indicator(WAV_SINBLIP, NOTE_2E, 100, p);
  }
  break;


 }

}

void init_joystick(void)
{

   options.joystick_available [0] = 0;
   options.joystick_available [1] = 0;

   if (install_joystick(JOY_TYPE_AUTODETECT) != 0)
      return;

   if (num_joysticks == 0)
    return;

//   if (options.joy_analogue)
//    analogmode = TRUE;
//     else

   options.joystick_available [0] = 1;

   if (num_joysticks == 2)
    options.joystick_available [1] = 1;

}

