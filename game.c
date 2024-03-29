
#include "config.h"

#include "allegro.h"

#include <math.h>

#include "globvars.h"

#include "stuff.h"

#include "palette.h"
#include "display.h"
#include "level.h"
#include "input.h"
#include "bullet.h"
#include "bullet.h"
#include "cloud.h"
#include "menu.h"
#include "ship.h"
#include "briefing.h"

#include "sound.h"

void begin_game(void);

void new_game(void);
void run_player(int p);

void init_stars(int p);

extern volatile unsigned char ticked;
extern volatile int framecounter;
extern int slacktime;

unsigned char counter;
void init_player_new_level(int p);
void init_player_spawn(int p);
void player_respawn(int p);
void run_stars(int p);
void set_star(int p, int i);
char run_pause(void);
//extern BITMAP *interlude_screen;

#ifdef SHOW_GRAPHS
extern int graph_slack [200];
extern int graph_slack_pos;
extern int graph_fps [200];
extern int graph_fps_pos;
extern volatile int frames_per_second;
#endif

#define STAR_DIST (600<<10)

void new_game(void)
{

arena.counter = 0;
//arena.stage = 2; - this is set in menu.c for now
 arena.only_player = 0;
 player[1].alive = 0;
 if (arena.players == 2)
 {
  arena.only_player = -1;
  player[1].alive = 1;
 }

 arena.camera_fix = 0;

 if (arena.players == 1
  && options.fix_camera_angle == 1)
  arena.camera_fix = 1;

 int i;

 for (i = 0; i < U_TYPES; i ++)
 {
  player[0].upgrade [i] = 0;
  player[1].upgrade [i] = 0;
 }

 player[0].ships = 2;
 player[0].starting_ships = player[0].ships + 1;
 player[0].ships_lost = 0;

 init_player_new_level(0);
 init_stars(0);
 init_clouds();
 init_bullets();
 init_voices();
// init_beats();
 init_ships();
// init_palette(hs[0].stage);
// init_stars(0);

 init_level();


 player[0].x = ship [TEAM_FRIEND] [0].x + 60000;
 player[0].y = ship [TEAM_FRIEND] [0].y + 60000;
 player[0].angle = ship [TEAM_FRIEND] [0].angle;

 player[0].target_auto = 0;

 setup_player_wing(0, SHIP_FIGHTER_FRIEND, 0);
 setup_player_wing(0, SHIP_FIGHTER_FRIEND, 1);

 if (arena.players == 2)
 {
  init_player_new_level(1);
  init_stars(1);
  player[0].ships = 2;
  player[0].starting_ships = player[0].ships + 2;
  player[0].ships_lost = 0;

 player[1].x = ship [TEAM_FRIEND] [0].x - 60000;
 player[1].y = ship [TEAM_FRIEND] [0].y + 60000;
 player[1].angle = ship [TEAM_FRIEND] [0].angle;
 player[1].target_auto = 0;

  setup_player_wing(1, SHIP_FIGHTER_FRIEND, 0);
  setup_player_wing(1, SHIP_FIGHTER_FRIEND, 1);

 }


// build_tracks();

 arena.game_over = 0;
 arena.end_stage = 0;

}


char run_pause(void)
{
 int pc = 0;

 do
 {
  pc ++;
  if (pc > 256)
   pc = 0;

  display_pause(pc);

// if (key [KEY_EQUALS])
//  ticked = 1;

   while(ticked == 0)
   {
       rest(1);
   };
   ticked --;

  if (key [KEY_Y])
  {
//   clear_bitmap(screen);
//   vsync();
   return 1;
  }

  if (key [KEY_N])
   return 0;
 }
  while (TRUE);

 return 0;

}


void game_loop(void)
{

// clear_to_color(interlude_screen, COL_1);

//       upgrade_menu();

 int playing = 1;

 counter = 0;

// if (choose_menu() == 0)
//  return;

 new_game(); // need a better place to put this

// upgrade_menu();

 do
 {

// get_input();
 //run_player();

 arena.counter ++;
 arena.counter &= 255;
 arena.subtime++;
 if (arena.subtime == 50)
 {
  arena.time ++;
  arena.subtime = 0;
 }
/*
 if (key [KEY_EQUALS]
  && arena.counter % 32 != 0)
  ticked = 1;*/

  if (ticked == 0)
  {
   run_display(1, 1);
   framecounter++;
  }
   else
    run_display(0, 0);

  slacktime = 0;

  while(ticked == 0)
  {
   rest(1);
   slacktime ++;
  };
  ticked --;

  counter ++;

  if (key [KEY_ESC])
  {
//      exit(0);
      if (run_pause() == 1)
       playing = 0;
  }

 if (arena.game_over > 0)
 {
  arena.game_over --;
  if (arena.game_over <= 0)
  {
   arena.game_over = 1;
   break;
  }
 }
  else
  {
    if (arena.mission_over > 0)
    {
     arena.mission_over --;
     if (arena.mission_over <= 0)
     {
      arena.mission_over = 1;
      break;
     }
    }
    if (arena.all_wships_lost > 0)
    {
     arena.all_wships_lost --;
     if (arena.all_wships_lost <= 0)
     {
      arena.all_wships_lost = 1;
      break;
     }
    }


/*  if (arena.end_stage > 0)
  {
   arena.end_stage --;
   if (arena.end_stage <= 0)
   {
      if (arena.stage == 25)
      {
       congratulations();
       break;
      }
      get_interlude_background();
      init_palette(arena.stage);
      init_player_new_level(0);
      build_tracks();
   }
  }*/
  }


// if (key [KEY_P])
//  continue;

  run_player(0);
  if (arena.players == 2)
   run_player(1);

  run_level();
  run_ships();
  run_bullets();
  run_clouds();
//  play_tracks();
  run_voices();


#ifdef SHOW_GRAPHS
 graph_slack_pos ++;
 if (graph_slack_pos >= 200)
  graph_slack_pos = 0;
 graph_slack [graph_slack_pos] = slacktime;

 graph_fps_pos ++;
 if (graph_fps_pos >= 200)
  graph_fps_pos = 0;
 graph_fps [graph_fps_pos] = frames_per_second;
#endif



 } while(playing == 1);


 if (arena.mission_over == 1)
  battle_report();

}


void run_player(int p)
{

 int i;

 if (player[p].alive == 0)
 {
  if (player[p].respawning > 0 && arena.game_over == 0 && arena.all_wships_lost == 0)
  {
   player[p].respawning --;
   PP.x += PP.x_speed;
   PP.y += PP.y_speed;
   if (player[p].respawning <= 0)
    player_respawn(p);
  }
  run_stars(p);
  return;
 }

 player[p].turning = 0;

 if (player[p].recycle > 0)
  player[p].recycle --;
 if (player[p].mflash [0] > 0)
  player[p].mflash [0] --;
 if (player[p].mflash [1] > 0)
  player[p].mflash [1] --;
 if (player[p].over_recycle > 0)
  player[p].over_recycle --;

 if (player[p].drive [0] > 0)
  player[p].drive [0] --;
 if (player[p].drive [1] > 0)
  player[p].drive [1] --;
 if (player[p].drive [2] > 0)
  player[p].drive [2] --;
 if (player[p].drive [3] > 0)
  player[p].drive [3] --;

 if (player[p].flap [0] > 0)
  player[p].flap [0] --;
 if (player[p].flap [1] > 0)
  player[p].flap [1] --;

 if (PP.target_new > 0)
  PP.target_new --;

 player[p].shield += 4; // change just below as well

 if (player[p].shield > player[p].max_shield)
  PP.shield = PP.max_shield;

 if (PP.shield >= (PP.max_shield >> 2))
 {
   if (PP.shield_up == 0)
   {
    PP.shield_just_up = 16;
    indicator(WAV_BLIP_L, NOTE_2G, 220, p);
   }
   PP.shield_up = 1;
 }

 if (PP.shield_just_up > 0)
   PP.shield_just_up --;

 if (PP.shield_flash > 0)
  PP.shield_flash --;

 get_input(p);

 player[p].x += player[p].x_speed;
 player[p].y += player[p].y_speed;

      int dragged = 1005;
/*      if (player[p].upgrade [U_AGILITY] == 1)
       dragged = 1009;
      if (player[p].upgrade [U_AGILITY] == 2)
       dragged = 1016;*/

 player[p].x_speed *= dragged;
 player[p].x_speed >>= 10;
 player[p].y_speed *= dragged;
 player[p].y_speed >>= 10;

    for (i = 0; i < PULSE; i ++)
    {
     if (PP.spulse_time [i] > 0)
     {
      PP.spulse_time [i] -= PP.spulse_time_delta [i];
     }
    }

    for (i = 0; i < HPULSE; i ++)
    {
     if (PP.hitpulse_thickness [0] [i] > 0)
      PP.hitpulse_thickness [0] [i] --;
     if (PP.hitpulse_thickness [1] [i] > 0)
      PP.hitpulse_thickness [1] [i] --;
    }


// player[p].range = (int) (hypot(hs[0].y - player[p].y, hs[0].x - player[p].x) - hs[0].range_minimum) >> 15;

  run_stars(p);

  PP.target_sight_visible = 0;

}


void run_stars(int p)
{

 int i;
 int star_dist;
 int angle2;
 int move_angle = ( - ANGLE_4 + radians_to_angle(atan2((player[p].y_speed), (player[p].x_speed))) - player[p].angle) & ANGLE_MASK;
 int window_x = player[p].window_centre_x << 10;
 int window_y = player[p].window_centre_y << 10;

 if (arena.camera_fix)
 {

  for (i = 0; i < NO_STARS; i ++)
  {
   star[p][i].old_x = star[p][i].x;
   star[p][i].old_y = star[p][i].y;

   star[p][i].x -= PP.x_speed >> 2;
   star[p][i].y -= PP.y_speed >> 2;

   if (star[p][i].x < -200<<10)
   {
    star[p][i].x = (PP.window_x+200)<<10;
    star[p][i].y = (-200 + grand(800))<<10;
    set_star(p, i);
   }
   if (star[p][i].x > (PP.window_x+200)<<10)
   {
    star[p][i].x = -200<<10;
    star[p][i].y = (-200 + grand(800))<<10;
    set_star(p, i);
   }
   if (star[p][i].y < -200<<10)
   {
    star[p][i].y = (PP.window_y+200)<<10;
    star[p][i].x = (-200 + grand(1000))<<10;
    set_star(p, i);
   }
   if (star[p][i].y > (PP.window_y+200)<<10)
   {
    star[p][i].y = -200<<10;
    star[p][i].x = (-200 + grand(1000))<<10;
    set_star(p, i);
   }

     star[p][i].old_x = star[p][i].x;
     star[p][i].old_y = star[p][i].y;

//     set_star(p, i);

/*
   star_dist = hypot(star[p][i].y - window_y, star[p][i].x - window_x);

   if (star_dist > STAR_DIST + 10)
   {
     angle2 = grand(ANGLE_2) - ANGLE_4 + move_angle;

     star[p][i].x = (player[p].window_centre_x << 10) + xpart(angle2, STAR_DIST);
     star[p][i].y = ((player[p].window_centre_y) << 10) + ypart(angle2, STAR_DIST);
     star[p][i].old_x = star[p][i].x;
     star[p][i].old_y = star[p][i].y;

     set_star(p, i);

    }*/
   }
  return;
 }

 int move_speed = hypot(player[p].y_speed, player[p].x_speed);

 int x_speed = xpart(move_angle, move_speed);
 int y_speed = ypart(move_angle, move_speed);

// float star_angle;

 float cos_angle = cos(angle_to_radians(player[p].turning));
 float sin_angle = sin(angle_to_radians(player[p].turning));
 float msin_angle = sin(angle_to_radians(player[p].turning)) * -1;
 int xd, yd;

 for (i = 0; i < NO_STARS; i ++)
 {
  star[p][i].old_x = star[p][i].x;
  star[p][i].old_y = star[p][i].y;

//  star_dist = hypot(star[p][i].y - (player[p].camera_y << 10), star[p][i].x - (player[p].camera_x << 10));
  star_dist = hypot(star[p][i].y - window_y, star[p][i].x - window_x);

  if (player[p].turning != 0)
  {
   xd = star[p][i].x - window_x;
   yd = star[p][i].y - window_y;
   star[p][i].x = (xd * cos_angle) + (yd * sin_angle) + window_x;
   star[p][i].y = (xd * msin_angle) + (yd * cos_angle) + window_y;
/*   star_angle = atan2(star[p][i].y - ((player[p].window_centre_y) << 10), star[p][i].x - (player[p].window_centre_x << 10));
   star_angle -= angle_to_radians(player[p].turning);
   star[p][i].x = (player[p].window_centre_x << 10) + cos(star_angle) * star_dist;
   star[p][i].y = ((player[p].window_centre_y) << 10) + sin(star_angle) * star_dist;*/
  }


//  star[p][i].x -= (x_speed >> 5) * 8;// * star[p][i].depth;
//  star[p][i].y -= (y_speed >> 5) * 8;// * star[p][i].depth;
  star[p][i].x -= (x_speed >> 2);// * star[p][i].depth;
  star[p][i].y -= (y_speed >> 2);// * star[p][i].depth;

//  if (star[p][i].x < (-200 << 10) || star[p][i].x > ((player[p].camera_x + 500) << 10)
//   || star[p][i].y < (-200 << 10) || star[p][i].y > ((player[p].camera_y + 500) << 10))
  if (star_dist > STAR_DIST + 10)
   {
/*    side_placement = grand(1000) - 500;
    star[p][i].x = (player[p].window_centre_x << 10) + xpart(move_angle, 500000) + xpart(move_angle + ANGLE_4, side_placement << 10);
    star[p][i].y = (player[p].window_centre_y << 10) + ypart(move_angle, 500000) + ypart(move_angle + ANGLE_4, side_placement << 10);
    star[p][i].old_x = star[p][i].x;
    star[p][i].old_y = star[p][i].y;*/
/*int side_placement = grand(STAR_DIST) - (STAR_DIST/2);
    star[p][i].x = (player[p].window_centre_x << 10) + xpart(move_angle, STAR_DIST) + xpart(move_angle + ANGLE_4, side_placement << 10);
    star[p][i].y = (player[p].window_centre_y << 10) + ypart(move_angle, STAR_DIST) + ypart(move_angle + ANGLE_4, side_placement << 10);
  */
    angle2 = grand(ANGLE_2) - ANGLE_4 + move_angle;// - ANGLE_2;
//    angle2 = ypart(grand(ANGLE_1), ANGLE_1) - ANGLE_2 + move_angle;// - ANGLE_2;



//    angle2 = xpart(grand(ANGLE_2) + ANGLE_4, ANGLE_2) + move_angle + ANGLE_4;// - ANGLE_2;
/*    if (coin())
    {
     angle2 = 0 + xpart(grand(ANGLE_4) - ANGLE_4, ANGLE_4) + move_angle;// - ANGLE_2;
    }
     else
      angle2 = 0 - xpart(grand(ANGLE_4) - ANGLE_4, ANGLE_4) + move_angle;// - ANGLE_2;*/


    star[p][i].x = (player[p].window_centre_x << 10) + xpart(angle2, STAR_DIST);
    star[p][i].y = ((player[p].window_centre_y) << 10) + ypart(angle2, STAR_DIST);
    star[p][i].old_x = star[p][i].x;
    star[p][i].old_y = star[p][i].y;


    //  star[p][i].depth = 4 + grand(28);
  //star[p][i].bright = 2 + star[p][i].depth / 2;
/*if (star[p][i].bright > 15)
 star[p][i].bright = 15;
 star[p][i].bright2 = star[p][i].bright / 3;*/

    set_star(p, i);

   }
 }
}



void init_player_new_level(int p)
{

    int i;

    PP.alive = 1;
    PP.respawning = 0;

    PP.max_shield = 1;
//    PP.weapon_type [0] = WPN_ROCKET;
//    PP.weapon_type [1] = WPN_AF_MISSILE;

    init_player_spawn(p);

//    arena.time_left = 50 * (60 + arena.stage * 5);
    PP.x = 0;
    PP.y = 0;
    PP.x = -150000; PP.y = 0400000;
    if (p == 1)
     PP.y += 100000;

    PP.start_x = PP.x;
    PP.start_y = PP.y;

    PP.priority_target = 2;
    PP.wing_orders = COMMAND_COVER;

    for (i = 0; i < WING_SIZE; i ++)
    {
     PP.wing [i] = -1;
    }

    PP.wing_size = 0;

}

void init_player_spawn(int p)
{
    PP.x = PP.start_x;
    PP.y = PP.start_y;
    PP.angle = 0;
    PP.turning = 0;
    PP.turn_speed = 0;
    PP.x_speed = 0;
    PP.y_speed = 0;

/*    PP.camera_x = 320;
    PP.camera_y = 360;
    PP.window_x = 640;
    PP.window_y = 480;
    PP.window_centre_x = 320;
    PP.window_centre_y = 360;*/

    PP.respawning = 0;
    PP.drive [0] = 0;
    PP.drive [1] = 0;
    PP.drive [2] = 0;
    PP.drive [3] = 0;
    PP.flap [0] = 0;
    PP.flap [1] = 0;


    player[p].turning = 0;
    player[p].turn_speed = 0;
    player[p].x_speed = 0;
    player[p].y_speed = 0;

    player[p].recycle = 0;
    player[p].over_recycle = 0;
    player[p].rocket_recycle = 0;
    player[p].rocket_burst = 0;
    player[p].charge = 0;
    player[p].rocket_angle = 0;
    PP.command_key = 0;

    player[p].camera_x = 400;
    player[p].camera_y = 460;
    player[p].window_x = 800;
    player[p].window_y = 600;
    player[p].window_centre_x = 400;
    player[p].window_centre_y = 460;

    if (arena.camera_fix)
    {
     player[p].camera_y = 300;
     player[p].window_y = 600;
     player[p].window_centre_y = 300;
    }

    if (arena.only_player == -1)
    {
     PP.camera_x = 200;
     PP.window_x = 400;
     PP.window_centre_x = 200;
    }
/*
    player[p].camera_x = 320;
    player[p].camera_y = 360;
    player[p].window_x = 640;
    player[p].window_y = 480;
    player[p].window_centre_x = 320;
    player[p].window_centre_y = 360;

    if (arena.players == 2)
    {
     PP.camera_x = 160;
     PP.window_x = 320;
     PP.window_centre_x = 160;
    }
*/
    player[p].drive [0] = 0;
    player[p].drive [1] = 0;
    player[p].drive [2] = 0;
    player[p].drive [3] = 0;
    player[p].flap [0] = 0;
    player[p].flap [1] = 0;
    PP.accelerating = 0;
    PP.commanding = 0;
    PP.just_commanded = 0;

    player[p].alive = 1;
    player[p].respawning = 0;

    PP.max_hp = 3000;
    PP.hp = PP.max_hp;
    PP.max_shield = 2000;
    PP.shield = PP.max_shield;
    PP.shield_up = 1;
    PP.shield_flash = 0;
    PP.shield_threshold = PP.shield / 4;
    PP.shield_just_up = 0;

    int i, j;

    for (i = 0; i < 2; i ++)
    {
     PP.weapon_charge [i] = 0;
     PP.weapon_block [i] = 0;
     PP.weapon_lock [i] = 0;
     PP.weapon_angle [i] = -ANGLE_4;
     PP.weapon_status [i] = 0;
     PP.weapon_status2 [i] = 0;
     PP.weapon_recycle [i] = 0;
     PP.weapon_firing [i] = 0;
     for (j = 0; j < WPN_TARGETS; j ++)
     {
       PP.weapon_target [i] [j] = TARGET_NONE;
       PP.weapon_sight_visible [i] [j] = 0;
     }
     switch(PP.weapon_type [i])
     {
      case WPN_AF_MISSILE:
       PP.weapon_status [i] = pos_or_neg(24);
       break;
     }
    }
    PP.mflash [0] = 0;
    PP.mflash [1] = 0;

    PP.target_a = -1;
    PP.target_new = 0;
    PP.target_key_hold = 0;
//    PP.target_auto = 0;

    for (i = 0; i < PULSE; i ++)
    {
     PP.spulse_time [i] = 0;
    }

    for (i = 0; i < HPULSE; i ++)
    {
     PP.hitpulse_thickness [0] [i] = 0;
     PP.hitpulse_thickness [1] [i] = 0;
    }

/*
    for (i = 0; i < SBARS; i ++)
    {
     player[p].sbar_width [i] = 0;
     player[p].sbar_high [i] = 0;
     player[p].sbar_y [i] = 0;
    }*/

}


void player_respawn(int p)
{
//    player[p].x = PP.start_x;
//    player[p].y = PP.start_y;
 if (arena.players == 2)
 {
  if (player[0].ships <= 0)
  {
   if (player[p^1].alive)
   {
    arena.only_player = p^1;
    player[p^1].camera_x = 400;
    player[p^1].camera_y = 460;
    player[p^1].window_centre_x = 400;
    return;
   }
    else
    {
     if (arena.all_wships_lost == 0)
      arena.game_over = 200;
     if (arena.mission_over > 0)
      arena.mission_over = 0;
     return;
    }
  }
  player[0].ships--;
 }

 if (arena.players == 1)
 {
  if (player[0].ships <= 0)
  {
   if (arena.all_wships_lost == 0)
    arena.game_over = 200;
   if (arena.mission_over > 0)
    arena.mission_over = 0;
   return;
  }
    else
     player[0].ships --;
 }

    init_stars(p);

    init_player_spawn(p);

// x and y are set in init_player_spawn. But we want to put them near the main (surviving) convoy:

 int a = TEAM_FRIEND;
 int e;
 int i;
 int angle;

 for (i = 1; i < 4; i ++)
 {
  for (e = 0; e < NO_SHIPS; e ++)
  {
   if (EE.type != SHIP_NONE
    && EE.spawn == i)
   {
    angle = grand(ANGLE_1);
    PP.x = EE.x + xpart(angle, 200000);
    PP.y = EE.y + ypart(angle, 200000);
    quick_cloud(CLOUD_SMALL_SHOCK, PP.x, PP.y, 0, 0, SMALL_SHOCK_TIME, 2, 0);
    return;
   }
  }
 }

// Note return above! To get down to here, player
// didn't find any friendly wships. Probably shouldn't happen, but even if it does we can get by
//  with the defaults set in init_player_spawn.

}


void init_stars(int p)
{

 int i, angle, dist;

 if (arena.camera_fix)
 {
  for (i = 0; i < NO_STARS; i ++)
  {

   star[p][i].x = (-200 + grand(1000))<<10;
   star[p][i].y = (-200 + grand(800))<<10;
   star[p][i].old_x = star[p][i].x;
   star[p][i].old_y = star[p][i].y;
   set_star(p, i);

  }


  return;
 }

 for (i = 0; i < NO_STARS; i ++)
 {

  angle = grand(ANGLE_1);
  dist = (grand(STAR_DIST) + grand(STAR_DIST)) / 2;
  star[p][i].x = (player[p].window_centre_x << 10) + xpart(angle, dist);
  star[p][i].y = (player[p].window_centre_y << 10) + ypart(angle, dist);

//  star[p][i].x = grand((player[p].window_x + 600) * 1024) - (300 << 10);
//  star[p][i].y = grand((player[p].window_y + 600) * 1024) - (300 << 10);

  star[p][i].old_x = star[p][i].x;
  star[p][i].old_y = star[p][i].y;
  star[p][i].depth = 3;//4 + grand(28);
//  star[p][i].bright = COL_STAR1 + grand(5);//2 + star[p][i].depth / 2;


  set_star(p, i);

/*if (star[p][i].bright > 15)
 star[p][i].bright = 15;
    star[p][i].bright2 = ((star[p][i].bright-COL_STAR1) / 3) + COL_STAR1;*/

 }


}

void set_star(int p, int i)
{
//    star[p][i].depth = 3+grand(6);//4 + grand(30);
//    star[p][i].bright = 15 - grand(13) + star[p][i].depth / 4;
/*    star[p][i].depth = 4 + grand(16);
    star[p][i].bright = ((star[p][i].depth - 4) / 2) + COL_6;
//    star[p][i].bright2 = ((star[p][i].bright-COL_STAR1) / 2) + COL_STAR1;
    if (star[p][i].bright > COL_12)
     star[p][i].bright = COL_12;
    star[p][i].bright2 = star[p][i].bright - 2;
    if (star[p][i].bright2 < COL_5)
     star[p][i].bright2 = COL_5;*/

     star[p][i].bright = COL_STAR1 + grand(5);
     star[p][i].bright2 = star[p][i].bright - 1;
     if (star[p][i].bright2 < COL_STAR1)
      star[p][i].bright2 = COL_STAR1;


}


