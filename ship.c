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
#include "game.h"

#include "sound.h"


int create_ship(int type, int a, int srec);
void destroy_ship(int a, int e);
void move_ship(int a, int e);
void fighter_engine(int a, int e);
void move_wship(int a, int e);
void ship_fire(int a, int e, int burst_status, int fire_type);
int get_ship_sprite(int type);

void carrier_launches(int a, int carrier_e);

int attack_angle(int a, int e, int target);
int attack_angle_lead(int a, int e, int target, int b_speed);
int angle_to_formation_position(int a, int e);
void convoy_position(int a, int e);
int angle_to_convoy_position(int a, int e);
char add_fighter_to_formation(int a, int e, int e2);

void drag_ship(int a, int e, int dragged);
void ship_action(int a, int e);
void count_ship_sprite(int a, int e, int sprite1, int sprite2, int inc);
int edist(int a, int e, int target);
int edist_test_less(int a, int e, int target, int max);
int edist_test_more(int a, int e, int target, int min);
int efrienddist(int a, int e, int target);

int turret_track_target(int a, int e, int t, int target, int forbid);
int turret_track_target_lead(int a, int e, int t, int target, int forbid, int b_speed);

void turret_fire(int a, int e, int t);
void ship_explodes(int a, int e);
void big_explode(int x, int y, int x_speed, int y_speed, int flares, int flare_size, int flare_speed, int col);
void turret_find_target(int a, int e, int t);
int fighter_find_target(int a, int e, int x, int y);
void fighter_find_wship_to_guard(int a, int e, int x, int y);

int get_target_x(int a, int target, int x);
int get_target_y(int a, int target, int y);
void wship_part_explodes(int a, int e, int part);
int wship_breakup(int a, int e);
void wship_starts_breaking_up(int a, int e);
void wship_final_explosion(int a, int e);
void wship_jump_out(int a, int e);
void fighter_jump_out(int a, int e);
int wship_jumping_out(int a, int e);
void jump_clouds(int a, int e);
void record_ship_destroyed(int a, int e, int owner);

void fighter_explosion(int a, int e, int flare1_flares, int flare1_size, int flare1_speed,
                        int fadeballs, int fadeballs_size, int fadeballs_dist);

int next_ship;
void run_turrets(int a, int e);
void calculate_ship_energy(int a, int e);
void recharge_wship_shield(int a, int e, int energy);
void recharge_fighter_shield(int a, int e);
void ship_energy_use(int a, int e);
void formation_position(int a, int e);
void wing_formation_position(int a, int e, int p);

void fighter_carry_on_wing(int a, int e);
void fighter_carry_on(int a, int e);
void set_act_evade(int a, int e);
void set_wing_seek(int a, int e);

int target_ship_angle(int a, int target);
int target_ship_class(int a, int target);

// these sprite structs are in display.c
// they're used here because they contain some data about turret/weapon locations
struct BMP_STRUCT wship_sprite [WSHIP_SPRITES] [WSHIP_ROTATIONS];
struct BMP_STRUCT turret_sprite [TURRET_SPRITES] [TURRET_ROTATIONS];
struct BMP_STRUCT fighter_sprite [FIGHTER_SPRITES] [FIGHTER_ROTATIONS];

enum
{
BURST_NO,
BURST_START,
BURST_CONTINUE
};

void init_ships(void)
{

 int a, e;

 for (a = 0; a < NO_TEAMS; a ++)
 {
  for (e = 0; e < NO_SHIPS; e ++)
  {
   EE.type = SHIP_NONE;
  }
 }

 next_ship = 5;
}

/*
Tries to create an ship, and returns its index if successful (or -1 if not).
The calling function can use that index to set the ship's properties.
*/
int create_ship(int type, int a, int srec)
{

  int e, i;

  for (e = 0; e < NO_SHIPS + 1; e ++)
  {
    if (e == NO_SHIPS)
     return -1;
    if (EE.type == SHIP_NONE)
     break;
  }

 EE.type = type;
 EE.think_count = 1;
 EE.think_count2 = 1;
 EE.turning = 0;
 EE.turning_time = 0;
 EE.turn_speed = 0;
 EE.sprite_count = 0;
 EE.slide_dir = 0;
 EE.slide_count = 0;
 EE.burst = 0;
 EE.recycle = 0;
 EE.recycle2 = 0;
 EE.carrier_launch = SHIP_NONE;
 EE.leader_awaiting_escort = 0;
 EE.command = -1;
 EE.player_leader = -1;
 EE.player_command = COMMAND_NONE;
 EE.player_wing = -1;
 EE.letter = -1;
 EE.target = TARGET_NONE;
 EE.frustration = 0;
 EE.force_away = 0;
 EE.leader = -1;
 EE.fighter_group = -1;
 EE.stored_angle = -1;
 EE.convoy = CONVOY_NONE;
 EE.drive_colour = a;

 EE.breakup = 0;
 EE.jump = 0;

 if (srec == 1)
  arena.srecord [SREC_CREATED] [a] [type] ++;
// srecord can be wrong if the list of ships in level.c doesn't end with a blank entry for TEAM_NONE

 EE.scancol = SCANCOL_OCSF;
 if (a == TEAM_ENEMY)
  EE.scancol = SCANCOL_FED;
// this is also set for particular ship types (eg Cwlth ships) in the next switch

 EE.sprite = get_ship_sprite(EE.type);

 switch(type)
 {
  case SHIP_LINER:
   EE.drive_colour = TEAM_ENEMY;
   break;
  case SHIP_FRIEND3:
   EE.scancol = SCANCOL_CWLTH;
   break;
 }

 for (i = 0; i < MAX_PARTS; i ++)
 {
  EE.max_hp [i] = eclass[type].hp [i];
  EE.hp [i] = EE.max_hp [i];
  EE.hit_pulse [i] = 0;
 }

 for (i = 0; i < MAX_ENGINES; i ++)
 {
  EE.engine [i] = 0;
 }

 for (i = 0; i < NO_TURRETS; i ++)
 {
  EE.turret_firing [i] = 0;
  EE.turret_target [i] = TARGET_NONE;
  EE.turret_recoil [i] = 0;
  EE.turret_energy [i] = 0;
  EE.turret_recycle [i] = 0;
  EE.turret_burst [i] = BURST_NONE;
  EE.turret_status [i] = 0;
 }


 EE.structure = eclass[type].structure;
 EE.max_structure = eclass[type].structure;
 EE.hit_pulse_structure = 0;
 EE.shield = 2;
 EE.max_shield = EE.shield;
 EE.shield_bar_pulse = 0;
 EE.shield_up = 1;
 EE.shield_threshhold = EE.max_shield / 3;
 EE.shield_generator = 1;
 EE.engine_power = eclass[EE.type].engine_output;

 for (i = 0; i < PULSE; i ++)
 {
  EE.spulse_time [i] = 0;
 }

 EE.leading_formation = 0;
 EE.formation_size = 0;
 EE.formation_wait = 0;
 EE.just_hit = 0;

 if (eclass[type].ship_class == ECLASS_WSHIP)
 {
  EE.energy_steps_hp [0] = (EE.structure * 7) / 10;
  EE.energy_steps_hp [1] = (EE.structure * 3) / 10;
  EE.energy_steps_production [0] = (eclass[EE.type].energy_production [0] * 7) / 10;
  EE.energy_steps_production [1] = (eclass[EE.type].energy_production [0] * 4) / 10;
 }

 calculate_ship_energy(a, e);


 return e;

}


int get_ship_sprite(int type)
{

 switch(type)
 {
  case SHIP_FIGHTER:
   return FIGHTER_SPRITE_BASIC_1;
  case SHIP_OLD2:
   return WSHIP_SPRITE_OLD2;
  case SHIP_DROM:
   return WSHIP_SPRITE_DROM;
  case SHIP_LINER:
   return WSHIP_SPRITE_LINER;
  case SHIP_OLD3:
   return WSHIP_SPRITE_OLD3;
  case SHIP_FRIEND3:
   return WSHIP_SPRITE_FRIEND3;
  case SHIP_SCOUT2:
   return WSHIP_SPRITE_SCOUT2;
  case SHIP_SCOUT3:
   return WSHIP_SPRITE_SCOUT3;
  case SHIP_SCOUTCAR:
   return WSHIP_SPRITE_SCOUTCAR;
  case SHIP_ECARRIER:
   return WSHIP_SPRITE_ECARRIER;
  case SHIP_FREIGHT:
   return WSHIP_SPRITE_FREIGHT;
  case SHIP_BOMBER:
   return FIGHTER_SPRITE_BOMBER_1;
  case SHIP_FIGHTER_FRIEND:
   return FIGHTER_SPRITE_FRIEND_1;
  case SHIP_FSTRIKE:
   return FIGHTER_SPRITE_FSTRIKE_1;
  case SHIP_LACEWING:
   return FIGHTER_SPRITE_LACEWING_1;
  case SHIP_MONARCH:
   return FIGHTER_SPRITE_MONARCH_1;
  case SHIP_IBEX:
   return FIGHTER_SPRITE_IBEX_1;
  case SHIP_AUROCHS:
   return FIGHTER_SPRITE_AUROCHS_1;
  case SHIP_ESCOUT:
   return FIGHTER_SPRITE_ESCOUT_1;
  case SHIP_EINT:
   return FIGHTER_SPRITE_EINT_1;
  case SHIP_EBASE:
   return WSHIP_SPRITE_EBASE;

 }

 return 1;

}

void carrier_launches(int a, int carrier_e)
{

 int e = create_ship(ship[a][carrier_e].carrier_launch, a, 1);

 EE.convoy = CONVOY_NONE;

 setup_new_ship(a, e, 0); // no subtypes
 calculate_threat();

 EE.x = ship[a][carrier_e].x + xpart(ship[a][carrier_e].angle, 35000);
 EE.y = ship[a][carrier_e].y + ypart(ship[a][carrier_e].angle, 35000);
 EE.x_speed = ship[a][carrier_e].x_speed + xpart(ship[a][carrier_e].angle, 5000);
 EE.y_speed = ship[a][carrier_e].y_speed + ypart(ship[a][carrier_e].angle, 5000);
 EE.action = ACT_AWAY;


        int c = quick_cloud(CLOUD_FADEBALL,
             EE.x, EE.y,
             0, 0, 30 + grand(10), 2, 0);
        if (c != -1)
        {
         CC.status = 15 + grand(11);
        }

 int e2;

// if another fighter has recently launched nearby and is waiting for followers, form up with it:
 for (e2 = 0; e2 < NO_SHIPS; e2++)
 {
  if (ship[a][e2].type == SHIP_NONE
   || ship[a][e2].leader_awaiting_escort == 0
   || abs(EE.y - ship[a][e2].y) + abs(EE.x - ship[a][e2].x) > 1500000) // don't worry about precision here.
   continue;
  EE.leader = e2;
  EE.formation_position = ship[a][e2].available_formation_position;
  ship[a][e2].formation_size ++;
  ship[a][e2].available_formation_position ++;
  if (eclass[EE.type].fighter_class == FCLASS_BOMBER)
   EE.mission = MISSION_ATTACK_WSHIP;
    else
    {
//     EE.action = ACT_FORM;
     EE.mission = MISSION_ESCORT;
    }
  break;

 }

 if (EE.leader == -1) // no leader found - new ship becomes leader of a group (EE.leader is set to -1 in create_ship)
 {
  EE.leader_awaiting_escort = 500;
  EE.available_formation_position = 1;
  if (eclass[EE.type].fighter_class == FCLASS_BOMBER)
  {
   EE.mission = MISSION_ATTACK_WSHIP;
   EE.action = ACT_TRANSIT;
  }
    else
     EE.mission = MISSION_SCRAMBLE;
 }


}


void calculate_ship_energy(int a, int e)
{
// int t;

 EE.energy_production = eclass[EE.type].energy_production [0];

 if (EE.structure < EE.energy_steps_hp [0])
  EE.energy_production = EE.energy_steps_production [0];

 if (EE.structure < EE.energy_steps_hp [1])
  EE.energy_production = EE.energy_steps_production [1];

/*
 for (t = 0; t < MAX_PARTS; t ++)
 {
  if (eclass[EE.type].energy_production [t] == 0)
   break;
  if (EE.hp [t] > 0)
   EE.energy_production += eclass[EE.type].energy_production [t];

 }*/

}

// number of ticks that group leader travels slowly when starting a formation:
#define FORMATION_WAIT_TIME 500

void run_ships(void)
{


 int a, e, t, t2, angle;
 int angle_diff;
 int dist;
 int target_dist = 0;

 char drives_running;
 int drive_power;
 int slide_power;
 int base_power;


// int engine_power;

/* next_ship --;
 if (next_ship <= 0)
 {
  e = create_ship(SHIP_EXAMPLE);
  if (e != -1)
  {
   EE.x = grand(600000) + 20000;
   EE.y = grand(440000) + 20000;
   EE.x_speed = grand(5000) - 2500;
   EE.y_speed = grand(5000) - 2500;
   EE.recycle = 50;
  }
  // for now we'll rely on create_ship failing if there are too many ships.
  next_ship = 500;
 }*/



// now let's run through all ships and make them work:
 for (a = 0; a < NO_TEAMS; a ++)
 {
 for (e = 0; e < NO_SHIPS; e ++)
 {
  if (EE.type == SHIP_NONE)
   continue;

  EE.counter ++;
  if (EE.leader_awaiting_escort > 0)
   EE.leader_awaiting_escort --;

//  engine_power = 10;

/*
    Things general to all ships go here:
*/

 for (t = 0; t < eclass[EE.type].parts; t ++)
 {
//  textprintf_ex(screen, font, 40 + 180*t, 60 + 15*e, -1, -1, "a %i, e %i, type %i", a, e, EE.type);
  if (EE.hit_pulse [t] > 0)
   EE.hit_pulse [t] --;
//  textprintf_right_ex(screen, font, 35 + 180*t, 60 + 15*e, -1, -1, "done");
 }

 if (EE.hit_pulse_structure > 0)
  EE.hit_pulse_structure --;
 if (EE.shield_bar_pulse > 0)
  EE.shield_bar_pulse --;

/*
    Things specific to particular classes of ships go here:
*/

  switch(EE.type)
  {
   case SHIP_FIGHTER:
   case SHIP_ESCOUT:
   case SHIP_EINT:
   case SHIP_FIGHTER_FRIEND:
   case SHIP_FSTRIKE:
   case SHIP_LACEWING:
   case SHIP_MONARCH:
   case SHIP_IBEX:
   case SHIP_AUROCHS:
    recharge_fighter_shield(a, e);
      if (EE.burst > 0)
      {
        if (EE.recycle <= 0)
        {
         ship_fire(a, e, BURST_CONTINUE, -1); // don't specify a fire_type
         if (EE.burst <= 0)
         {
          if (EE.shield < (EE.max_shield >> 1))
          {
           EE.action = ACT_AWAY;
           if (EE.player_leader != -1)
            EE.action = ACT_WING_AWAY;
           if (EE.target >= 0
            && eclass[ship[a^1][EE.target].type].ship_class == ECLASS_WSHIP)
             EE.force_away = 50 + grand(50);
          }
         }
        }
        if (EE.target != TARGET_NONE)
        {

//            EE.target_angle = attack_angle(a, e, EE.target);
//            EE.turning = delta_turn_towards_angle(EE.angle, EE.target_angle, 1);


          EE.think_count --;
//          EE.turning = delta_turn_towards_angle(EE.angle, EE.target_angle, 1);
          if (EE.think_count <= 0)
          {
//           angle_diff = angle_difference(EE.angle, EE.target_angle);
//           if (angle_diff > ANGLE_64)
           {
            if (target_ship_class(a^1, EE.target) == ECLASS_WSHIP)
             EE.target_angle = attack_angle(a, e, EE.target);
              else
               EE.target_angle = attack_angle_lead(a, e, EE.target, 6000);
            EE.turning = delta_turn_towards_angle(EE.angle, EE.target_angle, 1);
            EE.turning_time = 2;
            EE.think_count = 2;//EE.turning_time;// * 2;
           }
          }
        }

      }
       else

    switch(EE.action)
    {


     case ACT_GUARD:
      formation_position(a, e);
      EE.think_count --;
      EE.turning_time --;
      if (EE.turning_time <= 0)
       EE.turning = 0;
/*      if (ship[a][EE.leader].action != ACT_TRANSIT)
      {
       EE.action = ACT_SEEK;
       EE.think_count = 0;
       EE.turning_time = 0;
       break;
      }
need to have some equivalent of this which makes fighters seek their own targets when wship engages
      */
      if (EE.just_hit)
      {
         if (EE.shield < (EE.max_shield >> 1))
         {
          set_act_evade(a, e);
          break;
         }
         EE.action = ACT_SEEK;
         EE.think_count = 0;
         EE.turning_time = 0;
         break;
      }

       angle = angle_to_formation_position(a, e);

       EE.x_speed += xpart(angle, 20);
       EE.y_speed += ypart(angle, 20);


//      ship[a][EE.leader].leading_formation = 1;
     if (EE.think_count <= 0)
     {
// first - if the fighter's mission is escorting bombers, occasionally check to see whether there
//  are fighters nearby to protect the bombers from.
         dist = fighter_find_target(a, e, ship[a][EE.leader].x, ship[a][EE.leader].y);

         if (dist > EE.target_range)
          EE.target = TARGET_NONE;
         if (EE.target != TARGET_NONE)
         {
          EE.action = ACT_SEEK;
          EE.think_count = 1;
          EE.target_range = EE.base_target_range + EE.base_target_range; //grand(EE.base_target_range);
//          break;
         }

        EE.think_count = 5;
        EE.engine_power = eclass[EE.type].engine_output;

        if (abs(EE.x - EE.formation_x) < 150000 && abs(EE.y - EE.formation_y) < 150000
         && (dist = xyedist(EE.formation_x, EE.formation_y, a, e)) < 150000)
         {
          if (abs(EE.x - EE.formation_x) < 30000 && abs(EE.y - EE.formation_y) < 30000
           && (dist = xyedist(EE.formation_x, EE.formation_y, a, e)) < 30000)
           {
            EE.engine_power = ship[a][EE.leader].engine_power;
            EE.target_angle = ship[a][EE.leader].angle;
            angle_diff = angle_difference(EE.angle, EE.target_angle);

            if (angle_diff < ANGLE_32)
             break;

            EE.turning = delta_turn_towards_angle(EE.angle, EE.target_angle, 1);
            EE.turning_time = angle_diff / eclass[EE.type].turn;
            EE.think_count = EE.turning_time;// * 2;
            break;
           }

           angle_diff = angle_difference(EE.angle, EE.target_angle);

           EE.engine_power = ship[a][EE.leader].engine_power; // match speed with leader


/*
              if (angle_diff > ANGLE_2 - ANGLE_8)
              {
               EE.turning = 0;
               EE.engine_power = 10;
               drag_ship(a, e, 100);
               EE.think_count = 20;
               break;
              }*/
         }
          else
          {
           EE.target_angle = angle;
           angle_diff = angle_difference(EE.angle, EE.target_angle);
          }

        if (angle_diff > ANGLE_32)
        {
         EE.turning = delta_turn_towards_angle(EE.angle, EE.target_angle, 1);
         EE.turning_time = angle_diff / eclass[EE.type].turn;
         EE.think_count = EE.turning_time;// * 2;
        }
     }
      break;


     case ACT_FORM:
      formation_position(a, e);
      EE.think_count --;
      EE.turning_time --;
      if (EE.turning_time <= 0)
       EE.turning = 0;
/*      if (ship[a][EE.leader].action != ACT_TRANSIT)
      {
       EE.action = ACT_SEEK;
       EE.think_count = 0;
       EE.turning_time = 0;
       break;
      }*/
      if (EE.just_hit)
      {
         if (EE.shield < (EE.max_shield >> 1))
         {
          set_act_evade(a, e);
          break;
         }
         EE.action = ACT_SEEK;
         EE.think_count = 0;
         EE.turning_time = 0;
         break;
      }
      ship[a][EE.leader].leading_formation = 1;

       angle = angle_to_formation_position(a, e);

       EE.x_speed += xpart(angle, 20);
       EE.y_speed += ypart(angle, 20);


     if (EE.think_count <= 0)
     {
// first - if the fighter's mission is escorting bombers, occasionally check to see whether there
//  are fighters nearby to protect the bombers from.
        if (EE.mission == MISSION_ESCORT)
        {
         dist = fighter_find_target(a, e, ship[a][EE.leader].x, ship[a][EE.leader].y);

         if (dist > EE.target_range)
          EE.target = TARGET_NONE;
         if (EE.target != TARGET_NONE)
         {
          EE.action = ACT_SEEK;
          EE.think_count = 1;
// if a fighter breaks formation to defend (or if it doesn't reach formation position because it's
//  defending), the leader shouldn't wait for it so it is counted as having reached formation:
          EE.target_range = EE.base_target_range + EE.base_target_range; //grand(EE.base_target_range);
          break;
         }
        }

        EE.think_count = 10;
        EE.engine_power = eclass[EE.type].engine_output;

        if (abs(EE.x - EE.formation_x) < 100000 && abs(EE.y - EE.formation_y) < 100000
         && (dist = xyedist(EE.formation_x, EE.formation_y, a, e)) < 100000)
         {
          if (abs(EE.x - EE.formation_x) < 30000 && abs(EE.y - EE.formation_y) < 30000
           && (dist = xyedist(EE.formation_x, EE.formation_y, a, e)) < 30000)
           {
            EE.engine_power = ship[a][EE.leader].engine_power;
            EE.target_angle = ship[a][EE.leader].angle;
            angle_diff = angle_difference(EE.angle, EE.target_angle);

            if (angle_diff < ANGLE_16)
             break;


            EE.turning = delta_turn_towards_angle(EE.angle, EE.target_angle, 1);
            EE.turning_time = angle_diff / eclass[EE.type].turn;
            EE.think_count = EE.turning_time;// * 2;
            break;
           }

           EE.target_angle = angle;
           angle_diff = angle_difference(EE.angle, EE.target_angle);

           EE.engine_power = ship[a][EE.leader].engine_power; // match speed with leader

              if (angle_diff > ANGLE_2 - ANGLE_8)
              {
               EE.turning = 0;
               EE.engine_power = 40;
//               drag_ship(a, e, 100);
               EE.think_count = 20;
               break;
              }
         }
          else
          {
           EE.target_angle = angle;
           angle_diff = angle_difference(EE.angle, EE.target_angle);
          }

        if (angle_diff > ANGLE_32)
        {
         EE.turning = delta_turn_towards_angle(EE.angle, EE.target_angle, 1);
         EE.turning_time = (angle_diff / eclass[EE.type].turn) + 2;
         EE.think_count = EE.turning_time;// * 2;
        }
     }
      break;

     case ACT_AWAY:
      EE.engine_power = eclass[EE.type].engine_output;
      EE.force_away --;
      EE.turning = 0;

      if (EE.force_away > 0)
       break;

      if (EE.leader != -1)
      {
       if (EE.mission == MISSION_ESCORT)
//        && ship[a][EE.leader].action == ACT_TRANSIT)
       {
        EE.action = ACT_FORM;
        break;
       }
       if (EE.mission == MISSION_GUARD)
       {
        EE.action = ACT_GUARD;
        break;
       }
      }

      EE.turning = 0;
//      count_ship_sprite(a, e, FIGHTER_SPRITE_BASIC_1, FIGHTER_SPRITE_BASIC_4, -20);
      if (EE.target == TARGET_NONE)
      {
       EE.think_count --;
       if (EE.think_count <= 0)
       {
         target_dist = fighter_find_target(a, e, EE.x, EE.y);
         if (EE.target != TARGET_NONE)
         {
          EE.action = ACT_SEEK;
          if (EE.formation_size > 0
           && target_dist > 1000000)
           {
            EE.action = ACT_TRANSIT; // group leaders enter transit mode if target distant. Followers form up.
            EE.formation_wait = FORMATION_WAIT_TIME;
           }
         }
           else
           {
            fighter_carry_on(a, e);
            EE.think_count = 30;
           }
       }
      }
       else
       {
        if (edist_test_more(a, e, EE.target, EE.away_dist))
        {
         fighter_find_target(a, e, EE.x, EE.y);
         if (EE.target != TARGET_NONE)
         {
          EE.action = ACT_SEEK;
          if (EE.formation_size > 0
           && target_dist > 1000000)
           {
            EE.action = ACT_TRANSIT; // group leaders enter transit mode if target distant. Followers form up.
            EE.formation_wait = FORMATION_WAIT_TIME;
           }
         }
         EE.think_count = 1;
        }
       }
      break;
     case ACT_ATTACK:
      EE.engine_power = eclass[EE.type].engine_output;

      if (EE.target == TARGET_NONE)
      {
       EE.action = ACT_AWAY;
       EE.think_count = 10;
       break;
      }
      EE.turning = 0;
//      count_ship_sprite(a, e, FIGHTER_SPRITE_BASIC_1, FIGHTER_SPRITE_BASIC_4, 40);
      if (EE.recycle <= 0)
      {
       ship_fire(a, e, BURST_START, -1);

      }
//      EE.engine_power = 5;
      if (EE.burst <= 0)
      {
       EE.action = ACT_AWAY;
       EE.think_count = 10;
/*       if (EE.target >= 0
        && eclass[ship[a^1][EE.target].type].ship_class == ECLASS_WSHIP)
        EE.force_away = 50 + grand(50);*/
       break;
      }


      EE.turning_time --;
      if (EE.turning_time <= 0)
       EE.turning = 0;
      EE.think_count --;
      EE.target_angle = attack_angle_lead(a, e, EE.target, eclass[EE.type].bullet_speed);
      if (EE.think_count <= 0)
      {
//       EE.target_angle = attack_angle(e);
       angle_diff = angle_difference(EE.angle, EE.target_angle);
       if (angle_diff > ANGLE_64)
       {
        EE.turning = delta_turn_towards_angle(EE.angle, EE.target_angle, 1);
        EE.turning_time = angle_diff / eclass[EE.type].turn;
        EE.think_count = EE.turning_time;// * 2;
       }
      }

      break;
     case ACT_SEEK:
      EE.engine_power = eclass[EE.type].engine_output;

      if (EE.leader != -1)
      {
        if (EE.mission == MISSION_ESCORT
         && ship[a][EE.leader].action == ACT_TRANSIT
         && efrienddist(a, e, EE.leader) > EE.target_range)
        {
         EE.action = ACT_FORM;
         break;
        }
        if (EE.mission == MISSION_GUARD
         && efrienddist(a, e, EE.leader) > EE.target_range)
        {
         EE.action = ACT_GUARD;
//         break;
        }
      }

      if (EE.target == TARGET_NONE)
      {
       EE.action = ACT_AWAY;
       EE.think_count = 10;
       break; // must break - following code assumes target value is valid
      }
//      count_ship_sprite(a, e, FIGHTER_SPRITE_BASIC_1, FIGHTER_SPRITE_BASIC_4, -20);
      EE.turning_time --;
      if (EE.turning_time <= 0)
       EE.turning = 0;
      EE.think_count --;
      EE.frustration ++;
      EE.target_angle = attack_angle_lead(a, e, EE.target, eclass[EE.type].bullet_speed);

      if (EE.think_count <= 0)
      {
//       EE.target_angle = attack_angle(e);
       angle_diff = angle_difference(EE.angle, EE.target_angle);
       if (angle_diff > ANGLE_256)
       {
        EE.turning = delta_turn_towards_angle(EE.angle, EE.target_angle, 1);
        EE.turning_time = angle_diff / eclass[EE.type].turn;
        EE.think_count = EE.turning_time;// * 2;
       }
      }
      dist = edist(a, e, EE.target);

// shouldn't hang around for too long directly in front of target
//  (otherwise you can just kill fighters with little risk by backing away while firing).
//  We need to work out whether target is facing the fighter:
//  (we can assume that target value is valid)
       if (target_ship_class(a^1, EE.target) == ECLASS_FIGHTER
        && EE.shield < (EE.max_shield >> 1)
        && dist < 450000
        && angle_difference(EE.target_angle + ANGLE_2, target_ship_angle(a^1, EE.target)) < ANGLE_16
        && grand(10) == 0)
        {
         EE.frustration = 0;
         set_act_evade(a, e);
        }

      if (dist < (EE.attack_range))
      {
       if (EE.recycle <= 0)
       {
        if (angle_difference(EE.angle, EE.target_angle) < 25 + (EE.frustration >> 4))
        {
         ship_fire(a, e, BURST_START, -1);
//         EE.recycle = 7;
//         EE.burst --;
         EE.frustration = 0;
         EE.think_count = 2;
         break;
        }
       }
       if (grand(EE.frustration) > 50)
       {
         ship_fire(a, e, BURST_START, -1);
         EE.frustration = 0;
         EE.think_count = 2;
         break;
/*        EE.action = ACT_EVADE;
        EE.frustration = 0;
        EE.think_count = 10 + grand(30);
        EE.turning = pos_or_neg(1);*/
       }

      }
      break;
     case ACT_EVADE:
      EE.engine_power = eclass[EE.type].engine_output;
      EE.think_count --;
      if (EE.think_count <= 0)
      {
       EE.action = ACT_SEEK;
      }
      break;
     case ACT_TRANSIT: // just like ACT_SEEK but for long distance and only for group leaders. Group enters formation.
      if (EE.target == TARGET_NONE)
      {
       EE.action = ACT_AWAY;
       EE.think_count = 30;
       break;
      }
      EE.engine_power = eclass[EE.type].engine_output;
      EE.turning_time --;
      if (EE.turning_time <= 0)
       EE.turning = 0;
      EE.think_count --;
      EE.frustration = 0;
      EE.target_angle = attack_angle(a, e, EE.target);
      if (EE.think_count <= 0)
      {
//       EE.target_angle = attack_angle(e);
       angle_diff = angle_difference(EE.angle, EE.target_angle);
       if (angle_diff > ANGLE_64)
       {
        EE.turning = delta_turn_towards_angle(EE.angle, EE.target_angle, 1);
        EE.turning_time = angle_diff / eclass[EE.type].turn;
        EE.think_count = EE.turning_time;// * 2;
       }
      }
      if (edist_test_less(a, e, EE.target, EE.attack_range) && angle_difference(EE.angle, EE.target_angle) < 25 + (EE.frustration >> 4))
      {
       EE.action = ACT_SEEK;
//       EE.burst = 4;
      }
      break;


/*
***************************************************************************************************8

START special player wing movement functions

Note - these are not all friendly fighters. ONLY in player wing.

***************************************************************************************************8
*/


     case ACT_WING_FORM:
      wing_formation_position(a, e, EE.player_leader);
      EE.think_count --;
      EE.think_count2 --;
      EE.turning_time --;

      if (EE.recycle <= 0)
      {
       if (player[EE.player_leader].wing_fire2)
        ship_fire(a, e, BURST_START, ECLASS_WSHIP);
         else
         {
           if (player[EE.player_leader].wing_fire1)
            ship_fire(a, e, BURST_START, ECLASS_FIGHTER);
         }
      }

      if (EE.turning_time <= 0)
      {
       EE.turning = 0;
       EE.think_count = 0;
       EE.turning_time = 0;
      }
      if (EE.just_hit && EE.player_command != COMMAND_FORM)
      {

         set_act_evade(a, e); // will set to ACT_WING_EVADE if a == TEAM_FRIEND;
         break;
      }
     if (EE.think_count2 <= 0)
     {
      EE.think_count2 = 10;
        if (EE.player_command == COMMAND_COVER)
        {
         dist = fighter_find_target(a, e, player[EE.player_leader].x, player[EE.player_leader].y);

         if (dist > EE.target_range)
          EE.target = TARGET_NONE;
         if (EE.target != TARGET_NONE)
         {
          set_wing_seek(a, e);
          EE.think_count = 1;
          EE.think_count2 = 50;
//          EE.target_range = COVER_RANGE;//EE.base_target_range;// + grand(EE.base_target_range);
//          if (EE.target_range < dist + 10000)
//           EE.target_range = dist + 10000;
          break;
         }
        }
     }

     if (EE.think_count <= 0)
     {

        EE.think_count = 10;
        EE.engine_power = eclass[EE.type].engine_output;
        EE.turning = 0;


        EE.wship_throttle = xyedist(EE.formation_x, EE.formation_y, a, e);
        // this is just being used as debugging display storage - see display.c

#define DIST1 250000
#define DIST2 200000


        if (abs(EE.x - EE.formation_x) < DIST1 && abs(EE.y - EE.formation_y) < DIST1
         && (dist = xyedist(EE.formation_x, EE.formation_y, a, e)) < DIST1)
         {
          EE.engine_power = eclass[EE.type].engine_output;
          t = angle_to_formation_position(a, e) & ANGLE_MASK;
          if (player[EE.player_leader].accelerating == 0
               && dist < 90000)
          {
            EE.engine_power = 0;
            drag_ship(a, e, 1010);
            EE.think_count = 20;
//            EE.x_speed += xpart(t, 15);
//            EE.y_speed += ypart(t, 15);
          }

           t2 = (t - EE.angle) & ANGLE_MASK;;//angle_difference(t, EE.angle) & ANGLE_MASK;
//           EE.engine [1] = t;
//           EE.engine [2] = t2;
/*
          if (eclass[EE.type].move_mode == MOVE_SLIDE
               && EE.slide_count < 2
               && dist > 20000
               && dist < 80000)
          {
           if (t2 > ANGLE_4 - ANGLE_16 && t2 < ANGLE_4 + ANGLE_16)
           {
            EE.slide_dir = 1;
            EE.slide_count = 3;
           }
           if (t2 > ANGLE_2 + ANGLE_4 - ANGLE_16 && t < ANGLE_2 + ANGLE_4 + ANGLE_16)
           {
            EE.slide_dir = -1;
            EE.slide_count = 3;
           }
          }
          */
          if (abs(EE.x - EE.formation_x) < DIST2 && abs(EE.y - EE.formation_y) < DIST2
           && dist < DIST2)
           {
            EE.target_angle = player[EE.player_leader].angle;
            angle_diff = angle_difference(EE.angle, EE.target_angle);

            if (player[EE.player_leader].accelerating == 0)
            {
//             EE.engine_power = 0;
//             EE.x_speed += xpart(t, 5);
//             EE.y_speed += ypart(t, 5);
//             drag_ship(a, e, 990);
            }
             else
             {
              EE.x_speed += xpart(t, 20);//20);//60);
              EE.y_speed += ypart(t, 20);//20);//60);
              EE.engine_power = eclass[EE.type].engine_output - 20;

//              drag_ship(a, e, 1015);
             }

            if (angle_diff < ANGLE_64)
             break;

            EE.turning = delta_turn_towards_angle(EE.angle, EE.target_angle, 1);
            EE.turning_time = angle_diff / eclass[EE.type].turn;
            EE.think_count = EE.turning_time;// * 2;
            break;
           }

           EE.target_angle = angle_to_formation_position(a, e);
           angle_diff = angle_difference(EE.angle, EE.target_angle);

//           EE.engine_power = 190; //FRIEND_EPOWER; // match speed with leader

              if (angle_diff > ANGLE_2 - ANGLE_8)
              {
               EE.turning = 0;
               EE.engine_power = 0;
//               drag_ship(a, e, 100);
               EE.think_count = 20;
               break;
              }
         }
          else
          {
           EE.target_angle = angle_to_formation_position(a, e);
           angle_diff = angle_difference(EE.angle, EE.target_angle);
//           EE.x_speed += xpart(EE.target_angle, 160);
//           EE.y_speed += ypart(EE.target_angle, 160);
          }

        if (angle_diff > ANGLE_32)
        {
         EE.turning = delta_turn_towards_angle(EE.angle, EE.target_angle, 1);
         EE.turning_time = angle_diff / eclass[EE.type].turn;
         EE.think_count = EE.turning_time;// * 2;
        }
     }
    break;

     case ACT_WING_EVADE:
      EE.engine_power = eclass[EE.type].engine_output;
      EE.think_count --;
      if (EE.think_count <= 0)
      {
          set_wing_seek(a, e);
      }
      break;

     case ACT_WING_SEEK:
      EE.engine_power = eclass[EE.type].engine_output;
      EE.think_count2 --;

        if ((EE.player_command == COMMAND_FORM
          || EE.player_command == COMMAND_COVER)
           && EE.think_count2 <= 0)
        {
         if (efrienddist(a, e, EE.leader) > EE.target_range)
         {
          EE.action = ACT_WING_FORM;
          EE.think_count = 1;
          EE.think_count2 = 2;
          break;
         }
        }

      if (EE.target == TARGET_NONE)
      {
       EE.action = ACT_WING_AWAY;
       EE.think_count = 10;
       break; // must break - following code assumes target value is valid
      }
      EE.turning_time --;
      if (EE.turning_time <= 0)
       EE.turning = 0;
      EE.think_count --;
      EE.frustration ++;
      EE.target_angle = attack_angle_lead(a, e, EE.target, 9000);

      if (EE.think_count <= 0)
      {
       angle_diff = angle_difference(EE.angle, EE.target_angle);
       if (angle_diff > ANGLE_64)
       {
        EE.turning = delta_turn_towards_angle(EE.angle, EE.target_angle, 1);
        EE.turning_time = angle_diff / eclass[EE.type].turn;
        EE.think_count = EE.turning_time;// * 2;
       }
      }
      dist = edist(a, e, EE.target);

// shouldn't hang around for too long directly in front of target
//  (otherwise you can just kill fighters with little risk by backing away while firing).
//  We need to work out whether target is facing the fighter:
//  (we can assume that target value is valid)
       if (EE.shield < (EE.max_shield >> 1)
        && target_ship_class(a^1, EE.target) == ECLASS_FIGHTER
        && dist < 450000
        && angle_difference(EE.target_angle + ANGLE_2, target_ship_angle(a^1, EE.target)) < ANGLE_16
        && grand(10) == 0)
        {
         set_act_evade(a, e); // will set action to ACT_WING_EVADE if a == TEAM_FRIEND
        }

      if (dist < (EE.attack_range))
      {
       if (EE.recycle <= 0)
       {
        if (angle_difference(EE.angle, EE.target_angle) < 35 + (EE.frustration >> 4))
        {
         ship_fire(a, e, BURST_START, -1);
//         EE.recycle = 7;
//         EE.burst --;
         EE.frustration = 0;
         EE.think_count = 2;
         EE.turning = 0;
         EE.turning_time = 0;
//         break;
        }
       if (dist < 100000
        && target_ship_class(a^1, EE.target) == ECLASS_WSHIP) // can assume target valid here
        {
         EE.action = ACT_WING_AWAY;
         EE.think_count = 10;
         break; // must break - following code assumes target value is valid
        }
       }

      }
      break;

     case ACT_WING_AWAY:
      EE.engine_power = eclass[EE.type].engine_output;
      EE.force_away --;
      EE.turning = 0;

      if (EE.force_away > 0)
       break;

      EE.think_count2 --;

      if ((EE.player_command == COMMAND_FORM
        || EE.player_command == COMMAND_COVER)
         && EE.think_count2 <= 0)
      {
       if (efrienddist(a, e, EE.leader) > EE.target_range)
       {
        EE.action = ACT_WING_FORM;
        EE.think_count = 1;
        EE.think_count2 = 2;
        break;
       }
      }

      EE.turning = 0;
      if (EE.target == TARGET_NONE)
      {
       EE.think_count --;
       if (EE.think_count <= 0)
       {
         target_dist = fighter_find_target(a, e, EE.x, EE.y);
         if (EE.target != TARGET_NONE)
         {
          set_wing_seek(a, e);
         }
           else
           {
            fighter_carry_on_wing(a, e);
            EE.think_count = 30;
           }
       }
      }
       else
       {
        if (edist_test_more(a, e, EE.target, EE.away_dist))
        {
         if (EE.player_command != COMMAND_ATTACK)
          fighter_find_target(a, e, EE.x, EE.y);
         if (EE.target != TARGET_NONE)
         {
          set_wing_seek(a, e);
         }
          else
            fighter_carry_on_wing(a, e);
         EE.think_count = 1;
        }
       }
      break;


// Hover actions:


     case ACT_WING_SEEK_HOVER:
      EE.engine_power = eclass[EE.type].engine_output;
      EE.think_count2 --;

        if ((EE.player_command == COMMAND_FORM
          || EE.player_command == COMMAND_COVER)
           && EE.think_count2 <= 0)
        {
         if (efrienddist(a, e, EE.leader) > EE.target_range)
         {
          EE.action = ACT_WING_FORM;
          EE.think_count = 1;
          EE.think_count2 = 2;
          break;
         }
        }

      if (EE.target == TARGET_NONE)
      {
       EE.action = ACT_WING_AWAY;
       EE.think_count = 10;
       break; // must break - following code assumes target value is valid
      }
      EE.turning_time --;
      if (EE.turning_time <= 0)
       EE.turning = 0;
      EE.think_count --;
      EE.frustration ++;
      EE.target_angle = attack_angle_lead(a, e, EE.target, 9000);

      if (EE.think_count <= 0)
      {
       angle_diff = angle_difference(EE.angle, EE.target_angle);
       if (angle_diff > ANGLE_64)
       {
        EE.turning = delta_turn_towards_angle(EE.angle, EE.target_angle, 1);
        EE.turning_time = angle_diff / eclass[EE.type].turn;
        EE.think_count = EE.turning_time;// * 2;
       }
      }
      dist = edist(a, e, EE.target);

// shouldn't hang around for too long directly in front of target
//  (otherwise you can just kill fighters with little risk by backing away while firing).
//  We need to work out whether target is facing the fighter:
//  (we can assume that target value is valid)
       if (EE.shield < (EE.max_shield >> 1)
        && target_ship_class(a^1, EE.target) == ECLASS_FIGHTER
        && dist < 450000
        && angle_difference(EE.target_angle + ANGLE_2, target_ship_angle(a^1, EE.target)) < ANGLE_16
        && grand(10) == 0)
        {
         set_act_evade(a, e); // will set action to ACT_WING_EVADE if a == TEAM_FRIEND
        }

      if (dist < (EE.attack_range))
      {
       if (EE.recycle <= 0)
       {
        if (angle_difference(EE.angle, EE.target_angle) < 35 + (EE.frustration >> 4))
        {
         ship_fire(a, e, BURST_START, -1);
//         EE.recycle = 7;
//         EE.burst --;
         EE.frustration = 0;
         EE.think_count = 2;
         EE.turning = 0;
         EE.turning_time = 0;
//         break;
        }
       if (dist < 100000
        && target_ship_class(a^1, EE.target) == ECLASS_WSHIP) // can assume target valid here
        {
         EE.action = ACT_WING_AWAY;
         EE.think_count = 10;
         break; // must break - following code assumes target value is valid
        }
       }
       if (target_ship_class(a^1, EE.target) == ECLASS_WSHIP)
       {
        if (dist > EE.attack_range - 250000)
        {
         EE.engine_power = 0;
         if (EE.slide_count == 0)
         {
          EE.slide_dir = pos_or_neg(1);
          EE.slide_count = 20 + grand(30);
         }
        }
         else
         {
//          EE.action = ACT_WING_AWAY;
//          EE.think_count = 10;
          set_act_evade(a, e);
          break; // must break - following code assumes target value is valid
         }
       }
      }
      break;



/*
***************************************************************************************************8

END special player wing movement functions

***************************************************************************************************8
*/



    } // END OF SWITCH
    if (EE.leading_formation)
    {
// this is set if ships with ACT_FORM have this ship as leader
     if (EE.action == ACT_TRANSIT)
      EE.engine_power = eclass[EE.type].engine_output - 20;
     EE.leading_formation = 0;
    }
    if (EE.turning == -1)
    {
      EE.turn_speed -= 5;
      if (EE.turn_speed < eclass[EE.type].turn)
       EE.turn_speed = eclass[EE.type].turn * -1;
    }
    if (EE.turning == 1)
    {
      EE.turn_speed += 5;
      if (EE.turn_speed > eclass[EE.type].turn)
       EE.turn_speed = eclass[EE.type].turn;
    }
    if (EE.turning == 0)
    {
     if (EE.turn_speed < 0)
      EE.turn_speed += 4;
     if (EE.turn_speed > 0)
      EE.turn_speed -= 4;
     if (EE.turn_speed < 4 && EE.turn_speed > -4)
      EE.turn_speed = 0;
    }


      EE.angle += EE.turn_speed;
      EE.angle &= ANGLE_MASK;

     if (eclass[EE.type].move_mode == MOVE_SLIDE && EE.slide_count > 0)
     {

       drives_running = (EE.engine_power > 0) + (EE.slide_count > 0);
       drive_power = 128;
       if (drives_running == 2)
        drive_power = 64;
       slide_power = (70 * drive_power) >> 7; // DRIVE_SLIDE is the power of slide drives. If changed, change it in ship.c as well.
       base_power = (EE.engine_power * drive_power) >> 7;

     }
      else
      {
       base_power = EE.engine_power;
      }


    if (EE.slide_count > 0)
    {
     if (EE.slide_dir == -1)
     {
      EE.x_speed -= xpart(EE.angle + ANGLE_4, slide_power);
      EE.y_speed -= ypart(EE.angle + ANGLE_4, slide_power);
     }
     if (EE.slide_dir == 1)
     {
      EE.x_speed += xpart(EE.angle + ANGLE_4, slide_power);
      EE.y_speed += ypart(EE.angle + ANGLE_4, slide_power);
     }
     EE.slide_count --;
    }
    EE.recycle --;
    EE.x_speed += xpart(EE.angle, base_power);
    EE.y_speed += ypart(EE.angle, base_power);
    fighter_engine(a, e);
    move_ship(a, e);
    drag_ship(a, e, 1023-eclass[EE.type].drag);
    break;


/*
// old fighter part:
    switch(EE.action)
    {
     case ACT_AWAY:
      EE.turning = 0;
//      count_ship_sprite(a, e, FIGHTER_SPRITE_BASIC_1, FIGHTER_SPRITE_BASIC_4, -20);
      if (EE.target == TARGET_NONE)
      {
       EE.think_count --;
       if (EE.think_count <= 0)
       {
         fighter_find_target(a, e);
         if (EE.target != TARGET_NONE)
          EE.action = ACT_SEEK;
           else
            EE.think_count = 30;
       }
      }
       else
       {
        if (edist(a, e, EE.target) > (EE.away_dist))
        {
         fighter_find_target(a, e);
         if (EE.target != TARGET_NONE)
          EE.action = ACT_SEEK;
         EE.think_count = 1;
        }
       }
      break;
     case ACT_ATTACK:
      if (EE.target == TARGET_NONE)
      {
       EE.action = ACT_AWAY;
       EE.think_count = 10;
       break;
      }
      EE.turning = 0;
//      count_ship_sprite(a, e, FIGHTER_SPRITE_BASIC_1, FIGHTER_SPRITE_BASIC_4, 40);
      if (EE.recycle <= 0)
      {
       ship_fire(a, e, BURST_NO);
       EE.recycle = 7;
       EE.burst --;
      }
      EE.engine_power = 5;
      if (EE.burst <= 0)
      {
       EE.action = ACT_AWAY;
       EE.think_count = 10;
       break;
      }


      EE.turning_time --;
      if (EE.turning_time <= 0)
       EE.turning = 0;
      EE.think_count --;
      EE.target_angle = attack_angle(a, e, EE.target);
      if (EE.think_count <= 0)
      {
//       EE.target_angle = attack_angle(e);
       angle_diff = angle_difference(EE.angle, EE.target_angle);
       if (angle_diff > ANGLE_64)
       {
        EE.turning = delta_turn_towards_angle(EE.angle, EE.target_angle, 1);
        EE.turning_time = angle_diff / EE.turn_speed;
        EE.think_count = EE.turning_time;// * 2;
       }
      }

      break;
     case ACT_SEEK:
      if (EE.target == TARGET_NONE)
      {
       EE.action = ACT_AWAY;
       EE.think_count = 10;
       break;
      }
//      count_ship_sprite(a, e, FIGHTER_SPRITE_BASIC_1, FIGHTER_SPRITE_BASIC_4, -20);
      EE.turning_time --;
      if (EE.turning_time <= 0)
       EE.turning = 0;
      EE.think_count --;
      EE.frustration ++;
      EE.target_angle = attack_angle(a, e, EE.target);

      if (EE.think_count <= 0)
      {
//       EE.target_angle = attack_angle(e);
       angle_diff = angle_difference(EE.angle, EE.target_angle);
       if (angle_diff > ANGLE_64)
       {
        EE.turning = delta_turn_towards_angle(EE.angle, EE.target_angle, 1);
        EE.turning_time = angle_diff / EE.turn_speed;
        EE.think_count = EE.turning_time;// * 2;
       }
      }
      if (edist(a, e, EE.target) < (EE.attack_range) && angle_difference(EE.angle, EE.target_angle) < 25 + (EE.frustration >> 4))
      {
       EE.action = ACT_ATTACK;
       EE.burst = 4;
       EE.frustration = 0;
      }
      break;
    }
//    if (EE.turning == 0)
    {
      EE.engine [0] = 10;
      EE.engine [1] = 10;
    }
    if (EE.turning == -1)
    {
//      EE.engine [0] = 0;
      EE.engine [1] = 10;
      EE.angle -= EE.turn_speed;
      EE.angle &= ANGLE_MASK;
    }

    if (EE.turning == 1)
    {
      EE.engine [0] = 10;
//      EE.engine [1] = 0;
      EE.angle += EE.turn_speed;
      EE.angle &= ANGLE_MASK;
    }
    EE.recycle --;
    EE.engine [0] *= EE.engine_power;
    EE.engine [1] *= EE.engine_power;
    EE.x_speed += xpart(EE.angle, ((EE.engine [0] + EE.engine [1]) * EE.engine_power) / 10);
    EE.y_speed += ypart(EE.angle, ((EE.engine [0] + EE.engine [1]) * EE.engine_power) / 10);
    move_ship(a, e);
    drag_ship(a, e, 1000);
    break;
*/

   case SHIP_BOMBER:
//    EE.engine_power = eclass[EE.type].engine_output;
    recharge_fighter_shield(a, e);
    switch(EE.action)
    {

     case ACT_FORM:
      formation_position(a, e);
      EE.think_count --;
      EE.turning_time --;
      if (EE.turning_time <= 0)
       EE.turning = 0;
      ship[a][EE.leader].leading_formation = 1;
      if (ship[a][EE.leader].action != ACT_TRANSIT)
      {
       EE.action = ACT_AWAY;
       EE.think_count = 0;
       EE.turning_time = 0;
       break;
      }
     if (EE.think_count <= 0)
     {
        EE.think_count = 10;
        EE.engine_power = eclass[EE.type].engine_output;

        if (abs(EE.x - EE.formation_x) < 60000 && abs(EE.y - EE.formation_y) < 60000
         && (dist = xyedist(EE.formation_x, EE.formation_y, a, e)) < 60000)
         {
          if (abs(EE.x - EE.formation_x) < 30000 && abs(EE.y - EE.formation_y) < 30000
           && dist < 30000)
           {
            EE.engine_power = ship[a][EE.leader].engine_power;
            EE.target_angle = ship[a][EE.leader].angle;
            angle_diff = angle_difference(EE.angle, EE.target_angle);

            if (angle_diff < ANGLE_16)
             break;

            EE.turning = delta_turn_towards_angle(EE.angle, EE.target_angle, 1);
            EE.turning_time = angle_diff / eclass[EE.type].turn;
            EE.think_count = EE.turning_time;// * 2;
            break;
           }

           EE.target_angle = angle_to_formation_position(a, e);
           angle_diff = angle_difference(EE.angle, EE.target_angle);

           EE.x_speed += xpart(EE.target_angle, 15);
           EE.y_speed += ypart(EE.target_angle, 15);

           EE.engine_power = ship[a][EE.leader].engine_power; // match speed with leader

              if (angle_diff > ANGLE_2 - ANGLE_8)
              {
               EE.turning = 0;
               EE.engine_power = 40;
//               drag_ship(a, e, 100);
               EE.think_count = 20;
               break;
              }
         }
          else
          {
           EE.target_angle = angle_to_formation_position(a, e);
           angle_diff = angle_difference(EE.angle, EE.target_angle);
          }

        if (angle_diff > ANGLE_32)
        {
         EE.turning = delta_turn_towards_angle(EE.angle, EE.target_angle, 1);
         EE.turning_time = angle_diff / eclass[EE.type].turn;
         EE.think_count = EE.turning_time;// * 2;
        }
     }
      break;


     case ACT_AWAY:
      EE.turning = 0;
//      count_ship_sprite(a, e, FIGHTER_SPRITE_BASIC_1, FIGHTER_SPRITE_BASIC_4, -20);

      if (EE.leader != -1
       && ship[a][EE.leader].action == ACT_TRANSIT)
      {
       EE.action = ACT_FORM;

       break;
      }

      if (EE.target == TARGET_NONE)
      {
       EE.think_count --;
       if (EE.think_count <= 0)
       {
         target_dist = fighter_find_target(a, e, EE.x, EE.y);
         if (EE.target != TARGET_NONE)
         {
          EE.action = ACT_SEEK;
          if (EE.formation_size > 0
           && target_dist > 1000000)
           {
            EE.action = ACT_TRANSIT; // group leaders enter transit mode if target distant. Followers form up.
            EE.formation_wait = FORMATION_WAIT_TIME;
           }
         }
           else
            EE.think_count = 30;
       }
      }
       else
       {
        if (edist_test_more(a, e, EE.target, EE.away_dist))
        {
         target_dist = fighter_find_target(a, e, EE.x, EE.y);
         if (EE.target != TARGET_NONE)
         {
          EE.action = ACT_SEEK;
          if (EE.formation_size > 0
           && target_dist > 1000000)
           {
            EE.action = ACT_TRANSIT; // group leaders enter transit mode if target distant. Followers form up.
            EE.formation_wait = FORMATION_WAIT_TIME;
           }
         }
           else
            EE.think_count = 30;
        }
       }
      break;
     case ACT_ATTACK:
      if (EE.target == TARGET_NONE)
      {
       EE.action = ACT_AWAY;
       EE.think_count = 30;
       break;
      }
      EE.turning = 0;
      if (EE.recycle <= 0)
      {
       ship_fire(a, e, BURST_NO, -1);
       EE.recycle = 70;
//       EE.action = ACT_AWAY;
//       EE.think_count = 30;
//       break;
      }

      target_dist = edist(a, e, EE.target);

      if (target_dist < 40000
       || target_dist > 400000)
      {
       EE.action = ACT_AWAY;
       EE.think_count = 80;
       break;
      }

      EE.turning_time --;
      if (EE.turning_time <= 0)
       EE.turning = 0;
      EE.think_count --;
      if (EE.think_count <= 0)
      {
       EE.think_count = 10;
       angle_diff = angle_difference(EE.angle, EE.target_angle);
       EE.target_angle = attack_angle(a, e, EE.target);
       if (angle_difference(EE.angle, EE.target_angle) > ANGLE_4)
       {
           EE.action = ACT_AWAY;
           EE.think_count = 80;
       }
      }


/*      EE.target_angle = attack_angle_lead(a, e, EE.target, eclass[EE.type].bullet_speed);
      if (EE.think_count <= 0)
      {
       angle_diff = angle_difference(EE.angle, EE.target_angle);
       if (angle_diff > ANGLE_64)
       {
        EE.turning = delta_turn_towards_angle(EE.angle, EE.target_angle, 1);
        EE.turning_time = angle_diff / eclass[EE.type].turn;
        EE.think_count = EE.turning_time;// * 2;
       }
      }*/

      break;

     case ACT_SEEK:
      EE.think_count --;
      if (EE.leader != -1
       && ship[a][EE.leader].action == ACT_TRANSIT)
      {

       EE.action = ACT_FORM;
       break;
      }
      if (EE.target == TARGET_NONE)
      {
       EE.action = ACT_AWAY;
       EE.think_count = 30;
       break;
      }
//      count_ship_sprite(a, e, FIGHTER_SPRITE_BASIC_1, FIGHTER_SPRITE_BASIC_4, -20);
      EE.turning_time --;
      if (EE.turning_time <= 0)
       EE.turning = 0;
      EE.frustration = 0;
      EE.target_angle = attack_angle_lead(a, e, EE.target, 9000);//eclass[EE.type].bullet_speed);
//      EE.target_angle = attack_angle(a, e, EE.target);
      if (EE.think_count <= 0)
      {
//       EE.target_angle = attack_angle(e);
       angle_diff = angle_difference(EE.angle, EE.target_angle);
       if (angle_diff > ANGLE_64)
       {
        EE.turning = delta_turn_towards_angle(EE.angle, EE.target_angle, 1);
        EE.turning_time = angle_diff / eclass[EE.type].turn;
        EE.think_count = EE.turning_time;// * 2;
       }
      }
      if (edist_test_less(a, e, EE.target, EE.attack_range))// && angle_difference(EE.angle, EE.target_angle) < 25 + (EE.frustration >> 4))
      {
       EE.action = ACT_ATTACK;
//       EE.burst = 4;
      }
      break;
     case ACT_TRANSIT: // just like ACT_SEEK but for long distance and only for group leaders. Group enters formation. Leader slows slightly
      if (EE.target == TARGET_NONE)
      {
       EE.action = ACT_AWAY;
       EE.think_count = 30;
       break;
      }
      EE.engine_power = eclass[EE.type].engine_output;// - 15;
      EE.turning_time --;
      if (EE.turning_time <= 0)
       EE.turning = 0;
      EE.think_count --;
      EE.frustration = 0;
      EE.target_angle = attack_angle(a, e, EE.target);
      if (EE.think_count <= 0)
      {
//       EE.target_angle = attack_angle(e);
       angle_diff = angle_difference(EE.angle, EE.target_angle);
       if (angle_diff > ANGLE_64)
       {
        EE.turning = delta_turn_towards_angle(EE.angle, EE.target_angle, 1);
        EE.turning_time = angle_diff / eclass[EE.type].turn;
        EE.think_count = EE.turning_time;// * 2;
       }
      }
      if (edist_test_less(a, e, EE.target, EE.attack_range) && angle_difference(EE.angle, EE.target_angle) < 25 + (EE.frustration >> 4))
      {
       EE.action = ACT_ATTACK;
//       EE.burst = 4;
      }
      break;

    }

    if (EE.formation_wait > 0)
    {
     EE.formation_wait --;
     EE.engine_power = eclass[EE.type].engine_output - 20;
    }

/*    if (EE.leading_formation)
    {
// leading_formation is set if ships with ACT_FORM have this ship as leader.

// if the rest of the group hasn't formed up yet, slow down so they can catch up:
     if (EE.formation_complete < EE.formation_size)
      EE.engine_power = eclass[EE.type].engine_output - 1;
     EE.leading_formation = 0;
    }
*/

    if (EE.turning == -1)
    {
      EE.turn_speed -= 5;
      if (EE.turn_speed < eclass[EE.type].turn * -1)
       EE.turn_speed = eclass[EE.type].turn * -1;
    }
    if (EE.turning == 1)
    {
      EE.turn_speed += 5;
      if (EE.turn_speed > eclass[EE.type].turn)
       EE.turn_speed = eclass[EE.type].turn;
    }
    if (EE.turning == 0)
    {
     if (EE.turn_speed < 0)
      EE.turn_speed += 4;
     if (EE.turn_speed > 0)
      EE.turn_speed -= 4;
     if (EE.turn_speed < 4 && EE.turn_speed > -4)
      EE.turn_speed = 0;
    }


      EE.angle += EE.turn_speed;
      EE.angle &= ANGLE_MASK;
    EE.recycle --;
/*    EE.engine [0] *= EE.engine_power;
    EE.engine [1] *= EE.engine_power;
    EE.x_speed += xpart(EE.angle, ((EE.engine [0] + EE.engine [1]) * EE.engine_power) / 10);
    EE.y_speed += ypart(EE.angle, ((EE.engine [0] + EE.engine [1]) * EE.engine_power) / 10);*/
    EE.x_speed += xpart(EE.angle, EE.engine_power);
    EE.y_speed += ypart(EE.angle, EE.engine_power);
    fighter_engine(a, e);
    move_ship(a, e);
    drag_ship(a, e, 1023-eclass[EE.type].drag);
    break;



   case SHIP_OLD2:
   case SHIP_OLD3:
   case SHIP_FRIEND3:
   case SHIP_DROM:
   case SHIP_LINER:
   case SHIP_SCOUT2:
   case SHIP_SCOUT3:
   case SHIP_SCOUTCAR:
   case SHIP_ECARRIER:
   case SHIP_FREIGHT:
   case SHIP_EBASE:
    if (EE.breakup > 0)
    {
     if (wship_breakup(a, e))
      break; // ship has exploded
    }
     else
     {
      if (EE.jump > 0)
      {
       if (wship_jumping_out(a, e))
        break; // ship has jumped out
      }
       else
       {
        if (EE.carrier_launch != SHIP_NONE)
        {
         EE.carrier_launch_time --;
         if (EE.carrier_launch_time <= 0)
         {
          carrier_launches(a, e);
          EE.carrier_launch = SHIP_NONE;
         }
        }
       }
     }
      EE.engine [0] = 10;
      EE.engine [1] = 10;
      EE.recycle --;
//    EE.engine1 *= engine_power;
//    EE.engine2 *= engine_power;
//    EE.x_speed += xpart(EE.angle, ((EE.engine1 + EE.engine2) * engine_power) / 10);
//    EE.y_speed += ypart(EE.angle, ((EE.engine1 + EE.engine2) * engine_power) / 10);
    move_wship(a, e);
    EE.angle &= ANGLE_MASK;
    if (EE.jump == 0)
     run_turrets(a, e);
    ship_energy_use(a, e); // must come after run_turrets
//    EE.angle += 5;
    break;

  }
    for (t = 0; t < PULSE; t ++)
    {
     if (EE.spulse_time [t] > 0)
     {
      EE.spulse_time [t] -= EE.spulse_time_delta [t];
      EE.spulse_size [t] -= EE.spulse_size2 [t];
     }
    }
  EE.just_hit = 0;

// the previous big switch thing deals with ships in classes. This deals with individual things:
  switch(EE.type)
  {
   case SHIP_ESCOUT:
    if (EE.recycle2 > 0)
     EE.recycle2 --;
    break;
   case SHIP_FIGHTER_FRIEND:
    if (EE.recycle2 > 0)
     EE.recycle2 --;
    if (EE.engine_power == 0)
     EE.sprite_count --;
      else
       EE.sprite_count ++;
    if (EE.sprite_count < 0)
    {
     EE.sprite = FIGHTER_SPRITE_FRIEND_1;
     EE.sprite_count = 0;
     break;
    }
    if (EE.sprite_count < 4)
    {
     EE.sprite = FIGHTER_SPRITE_FRIEND_2;
     break;
    }
    EE.sprite = FIGHTER_SPRITE_FRIEND_3;
    EE.sprite_count = 4;
    break;
   case SHIP_FSTRIKE:
    if (EE.engine_power == 0)
     EE.sprite_count --;
      else
       EE.sprite_count ++;
    if (EE.sprite_count < 0)
    {
     EE.sprite = FIGHTER_SPRITE_FSTRIKE_1;
     EE.sprite_count = 0;
     break;
    }
    if (EE.sprite_count < 5)
    {
     EE.sprite = FIGHTER_SPRITE_FSTRIKE_2;
     break;
    }
    EE.sprite = FIGHTER_SPRITE_FSTRIKE_3;
    EE.sprite_count = 5;
    break;
   case SHIP_LACEWING:
    if (EE.recycle2 > 0)
     EE.recycle2 --;
    if (EE.engine_power < 90)
     EE.sprite_count --;
      else
       EE.sprite_count ++;
    if (EE.sprite_count < 0)
    {
     EE.sprite = FIGHTER_SPRITE_LACEWING_1;
     EE.sprite_count = 0;
     break;
    }
    if (EE.sprite_count < 3)
    {
     EE.sprite = FIGHTER_SPRITE_LACEWING_2;
     break;
    }
    if (EE.sprite_count < 6)
    {
     EE.sprite = FIGHTER_SPRITE_LACEWING_3;
     break;
    }
    EE.sprite = FIGHTER_SPRITE_LACEWING_4;
    EE.sprite_count = 6;
    break;

   case SHIP_MONARCH:
    if (EE.recycle2 > 0)
     EE.recycle2 --;
    if (EE.engine_power == 0)
     EE.sprite_count --;
      else
       EE.sprite_count ++;
    if (EE.sprite_count < 0)
    {
     EE.sprite = FIGHTER_SPRITE_MONARCH_1;
     EE.sprite_count = 0;
     break;
    }
    if (EE.sprite_count < 4)
    {
     EE.sprite = FIGHTER_SPRITE_MONARCH_2;
     break;
    }
    EE.sprite = FIGHTER_SPRITE_MONARCH_3;
    EE.sprite_count = 4;
    break;
  }

 }
 }



}

// this function makes a fighter do whatever is appropriate for its mission. Only works for some missions!
//  call if whatever the fighter is doing runs out or can't be continued and it needs to go back to defending etc.
void fighter_carry_on_wing(int a, int e)
{
 switch(EE.mission)
 {
/*
  case MISSION_GUARD:
   if (EE.leader == -1)
   {
    fighter_find_wship_to_guard(a, e, EE.x, EE.y);
   }
  if (EE.leader != -1)
      EE.action = ACT_WING_FORM;
   break;*/
 }

}

void fighter_carry_on(int a, int e)
{
 switch(EE.mission)
 {
  case MISSION_GUARD:
   if (EE.leader == -1)
   {
    fighter_find_wship_to_guard(a, e, EE.x, EE.y);
    if (EE.leader != -1)
      EE.action = ACT_GUARD;
   }
   break;
 }

}


void set_act_evade(int a, int e)
{
          EE.action = ACT_EVADE;
          if (a == TEAM_FRIEND)
           EE.action = ACT_WING_EVADE;
          EE.think_count = 10 + grand(30);
          EE.turning = pos_or_neg(1);
          EE.frustration = 0;
          if (eclass[EE.type].move_mode == MOVE_SLIDE)
          {
            if (grand(2))
            {
             EE.slide_dir = pos_or_neg(1);
             EE.think_count = 10 + grand(20);
            }
          }

}

void set_wing_seek(int a, int e)
{
    if (eclass[EE.type].move_mode == MOVE_SLIDE)
     EE.action = ACT_WING_SEEK_HOVER;
      else
       EE.action = ACT_WING_SEEK;
}


int target_ship_class(int a, int target)
{
// assumes target is valid (although -1 target will just get fighter returned)
 if (target < 0)
  return ECLASS_FIGHTER; // must be player

 return eclass[ship[a][target].type].ship_class;

}

int target_ship_angle(int a, int target)
{
 if (target == TARGET_P1)
  return player[0].angle;
 if (target == TARGET_P2)
  return player[1].angle;

 return ship[a][target].angle;
}

void ship_energy_use(int a, int e)
{

 int demand = 0;

 if (EE.shield < EE.max_shield
  && EE.shield_generator)
  demand ++;

 int t;

 for (t = 0; t < eclass[EE.type].parts; t ++)
 {
  if (EE.turret_type [t] != TURRET_NONE)
  {
   if (EE.turret_energy [t] < EE.turret_energy_max [t])
    demand ++;
     else EE.turret_energy [t] = EE.turret_energy_max [t];
  }
 }

 if (demand == 0)
  return;

 int energy_share = EE.energy_production / demand;

 if (EE.shield < EE.max_shield
  && EE.shield_generator)
   recharge_wship_shield(a, e, energy_share);

 for (t = 0; t < eclass[EE.type].parts; t ++)
 {
  if (EE.turret_type [t] != TURRET_NONE)
  {
   if (EE.turret_energy [t] < EE.turret_energy_max [t])
   {
     EE.turret_energy [t] += energy_share;
// don't cap it at max here - this will allow any leftover to be saved if the turret fires immediately.
     if (EE.turret_energy [t] >= EE.turret_energy_max [t]
      && EE.turret_burst [t] == BURST_CHARGING)
      {
       EE.turret_burst [t] = BURST_WAITING;
       if (EE.turret_type [t] == TURRET_EBASIC
        || EE.turret_type [t] == TURRET_EANTI)
       {
        EE.turret_status [t] = 6;
        EE.turret_recoil [t] = 18;
       }
      }
   }
  }
 }

}



void recharge_wship_shield(int a, int e, int energy)
{

 EE.shield += energy / EE.shield_energy_cost;
 if (EE.shield > EE.max_shield)
  EE.shield = EE.max_shield;

 if (EE.shield_up == 0 && EE.shield >= EE.shield_threshhold)
  EE.shield_up = 1;

}


void recharge_fighter_shield(int a, int e)
{

 EE.shield += EE.shield_recharge;
 if (EE.shield > EE.max_shield)
  EE.shield = EE.max_shield;

 if (EE.shield_up == 0 && EE.shield >= EE.shield_threshhold)
  EE.shield_up = 1;

}

void run_turrets(int a, int e)
{

  int t;

    for (t = 0; t < eclass[EE.type].turrets; t ++)
    {
     if (EE.turret_type [t] == TURRET_NONE)
      continue;
     EE.angle &= ANGLE_MASK;
     EE.turret_x [t] = EE.x + rotate_pos_x [eclass[EE.type].base_turret_rp + t] [EE.angle];
     EE.turret_y [t] = EE.y + rotate_pos_y [eclass[EE.type].base_turret_rp + t] [EE.angle];
     EE.turret_recycle [t] --;
     if (EE.turret_recoil [t] > 0)
      EE.turret_recoil [t] --;

     if (EE.turret_type [t] == TURRET_EBEAM)
     {

       if (EE.turret_target [t] != TARGET_NONE
        || EE.turret_firing [t] > 0)
       {
        if (EE.turret_target [t] != TARGET_NONE
         && EE.turret_firing [t] == 0)
         EE.turret_angle [t] = turret_track_target(a, e, t, EE.turret_target [t], EE.turret_slew [t]);
        if (EE.turret_firing [t] > 0)
        {
          EE.turret_status [t] ++;
          if (EE.turret_status [t] >= 12)
          {
           turret_fire(a, e, t);
           EE.turret_firing [t] --;
           EE.turret_status [t] = 12;
          }
        }
         else
         {
          if ((EE.counter >> 3) & 1)
           EE.turret_slew [t] = 0;
          if (EE.turret_status [t] > 0)
           EE.turret_status [t] --;
         }

        if (EE.turret_recycle [t] <= 0
         && EE.turret_energy [t] >= EE.turret_energy_use [t]
         && angle_difference(EE.turret_angle [t], EE.turret_target_angle [t]) < 25)
         {
          EE.turret_recycle [t] = EE.turret_recycle_time [t];
          EE.turret_energy [t] -= EE.turret_energy_use [t];
          EE.turret_firing [t] = 70;
         }
           else
            {
             if (EE.turret_firing [t] == 0 && (EE.counter & 31) == 31)
              turret_find_target(a, e, t);
            }

       }
        else
        {
         if ((EE.counter & 31) == 31)
          turret_find_target(a, e, t);
         if (EE.turret_recycle [t] <= 0)
          EE.turret_recycle [t] = 1;
        }

/*
       if (EE.turret_target [t] != TARGET_NONE
        || EE.turret_firing [t] > 0)
       {
        if (EE.turret_target [t] != TARGET_NONE)
         EE.turret_angle [t] = turret_track_target(a, e, t, EE.turret_target [t], EE.turret_slew [t]);
        if (EE.turret_firing [t] > 0)
        {
          turret_fire(a, e, t);
          EE.turret_firing [t] --;
        }
         else
         {
          if ((EE.counter >> 3) & 1)
           EE.turret_slew [t] = 0;
         }

        if (EE.turret_recycle [t] <= 0
         && EE.turret_energy [t] >= EE.turret_energy_use [t]
         && angle_difference(EE.turret_angle [t], EE.turret_target_angle [t]) < 25)
         {
          EE.turret_recycle [t] = EE.turret_recycle_time [t];
          EE.turret_energy [t] -= EE.turret_energy_use [t];
          EE.turret_firing [t] = 70;
         }
           else
            {
             if (EE.turret_firing [t] == 0 && (EE.counter & 31) == 31)
              turret_find_target(a, e, t);
            }

       }
        else
        {
         if ((EE.counter & 31) == 31)
          turret_find_target(a, e, t);
         if (EE.turret_recycle [t] <= 0)
          EE.turret_recycle [t] = 1;
        }

*/
     }
      else // turrets other than beam turrets
      {
       if (EE.turret_target [t] != TARGET_NONE
        || EE.turret_burst [t] == BURST_FIRING)
       {
//      EE.turret_angle [t] = turret_track_target(a, e, t, EE.turret_target [t], EE.turret_slew [t]);
        if (EE.turret_target [t] != TARGET_NONE)
         EE.turret_angle [t] = turret_track_target_lead(a, e, t, EE.turret_target [t], EE.turret_slew [t], EE.turret_bullet_speed [t]);
        if ((EE.counter >> 3) & 1)
          EE.turret_slew [t] = 0;
        if (EE.turret_recycle [t] <= 0
         && EE.turret_energy [t] >= EE.turret_energy_use [t])
         {
          if (EE.turret_burst [t] == BURST_FIRING
           ||
            (EE.turret_burst [t] == BURST_NONE // only turrets with no burst fire mode
             && angle_difference(EE.turret_angle [t], EE.turret_target_angle [t]) < 145))
//             && edist_test_less(a, e, EE.turret_target [t], EE.turret_firing_range [t])))
// turret_target_angle is set in turret_track_target_lead
          {
//            EE.turret_burst [t] = BURST_FIRING;
            turret_fire(a, e, t);
          }
            else
            {
//             if (EE.turret_burst [t] == BURST_WAITING && (EE.counter & 31) == 31)
             if ((EE.counter & 31) == 31)
              turret_find_target(a, e, t);
            }
         }
       }
        else
        {
         if (angle_difference(EE.turret_angle [t], (EE.angle + EE.turret_rest_angle [t]) & ANGLE_MASK) <= EE.turret_turn [t])
          EE.turret_angle [t] = (EE.angle + EE.turret_rest_angle [t]) & ANGLE_MASK;
           else
            EE.turret_angle [t] = turn_towards_angle(EE.turret_angle [t], (EE.angle + EE.turret_rest_angle [t]) & ANGLE_MASK, EE.turret_turn [t]);
         if ((EE.counter & 31) == 31)
          turret_find_target(a, e, t);
         if (EE.turret_recycle [t] <= 0)
          EE.turret_recycle [t] = 1;
        }
       }

 switch(EE.turret_type [t])
 {
  case TURRET_EHEAVY:
   if (EE.turret_status [t] < 12)
    EE.turret_status [t] ++;
   break;
  case TURRET_ELONG:
   if (EE.turret_status [t] < 12)
    EE.turret_status [t] ++;
   break;

 }

    }

}


/*
void run_turrets(int a, int e)
{

  int t;

    for (t = 0; t < eclass[EE.type].turrets; t ++)
    {
     if (EE.turret_type [t] == TURRET_NONE)
      continue;
     EE.turret_x [t] = EE.x + rotate_pos_x [eclass[EE.type].base_turret_rp + t] [EE.angle];
     EE.turret_y [t] = EE.y + rotate_pos_y [eclass[EE.type].base_turret_rp + t] [EE.angle];
     EE.turret_recycle [t] --;
     if (EE.turret_recoil [t] > 0)
      EE.turret_recoil [t] --;

     if (EE.turret_type [t] == TURRET_EBEAM)
     {
       if (EE.turret_target [t] != TARGET_NONE
        || EE.turret_burst [t] == BURST_FIRING)
       {
        if (EE.turret_target [t] != TARGET_NONE)
         EE.turret_angle [t] = turret_track_target(a, e, t, EE.turret_target [t], EE.turret_slew [t]);
        if (EE.turret_firing [t] > 0)
        {
          turret_fire(a, e, t);
          EE.turret_firing [t] --;
        }
         else
         {
          if ((EE.counter >> 3) & 1)
           EE.turret_slew [t] = 0;
         }

        if (EE.turret_recycle [t] <= 0
         && EE.turret_energy [t] >= EE.turret_energy_use [t])
         {
          EE.turret_recycle [t] = EE.turret_recycle_time [t];
          EE.turret_energy [t] -= EE.turret_energy_use [t];
          EE.turret_firing [t] = 70;
         }
           else
            {
             if (EE.turret_firing [t] == 0 && (EE.counter & 31) == 31)
              turret_find_target(a, e, t);
            }

       }
        else
        {
         if ((EE.counter & 31) == 31)
          turret_find_target(a, e, t);
         if (EE.turret_recycle [t] <= 0)
          EE.turret_recycle [t] = 1;
        }


     }
      else // turrets other than beam turrets
      {
       if (EE.turret_target [t] != TARGET_NONE
        || EE.turret_burst [t] == BURST_FIRING)
       {
//      EE.turret_angle [t] = turret_track_target(a, e, t, EE.turret_target [t], EE.turret_slew [t]);
        if (EE.turret_target [t] != TARGET_NONE)
         EE.turret_angle [t] = turret_track_target_lead(a, e, t, EE.turret_target [t], EE.turret_slew [t], EE.turret_bullet_speed [t]);
        if ((EE.counter >> 3) & 1)
          EE.turret_slew [t] = 0;
        if (EE.turret_recycle [t] <= 0
         && EE.turret_energy [t] >= EE.turret_energy_use [t])
         {
          if (EE.turret_burst [t] < BURST_WAITING) // either BURST_NONE or BURST_FIRING
          {
//           if (EE.turret_burst [t] == BURST_FIRING
//            || angle_difference(EE.angle, EE.target_angle) < 25)
//           {
            turret_fire(a, e, t);
//           }
          }
            else
            {
             if (EE.turret_burst [t] == BURST_WAITING && (EE.counter & 31) == 31)
              turret_find_target(a, e, t);
            }
         }
       }
        else
        {
         if ((EE.counter & 31) == 31)
          turret_find_target(a, e, t);
         if (EE.turret_recycle [t] <= 0)
          EE.turret_recycle [t] = 1;
        }
       }

    }

}
*/



void turret_find_target(int a, int e, int t)
{

 int a2 = a ^ 1;
 int e2;

 int dist;
 int dist2, actual_dist_fighter = 0, actual_dist_wship = 0; // not modified for priority
 int closest_dist_wship = 9999999;
 int closest_e2_wship = TARGET_NONE;
 int closest_dist_fighter = 9999999;
 int closest_e2_fighter = TARGET_NONE;
 int x = EE.turret_x [t];
 int y = EE.turret_y [t];

 for (e2 = 0; e2 < NO_SHIPS; e2 ++)
 {
  if (ship[a2][e2].type == SHIP_NONE)
//   || ship[a2][e2].hp <= 0)
   continue;
  if (EE.turret_class [t] == ATTACK_ONLY_FIGHTER
   && eclass[ship[a2][e2].type].ship_class == ECLASS_WSHIP)
    continue;
  if (EE.turret_class [t] == ATTACK_ONLY_WSHIP
   && eclass[ship[a2][e2].type].ship_class == ECLASS_FIGHTER)
    continue;
 // do a quick and rough test first before we go into hypot:
  dist = abs(y - ship[a2][e2].y) + abs(x - ship[a2][e2].x);

  if (eclass[ship[a2][e2].type].ship_class == ECLASS_FIGHTER)
  {
   if (dist > closest_dist_fighter)
    continue;
   dist = hypot(y - ship[a2][e2].y, x - ship[a2][e2].x);
   if (dist < closest_dist_fighter)
   {
     closest_dist_fighter = dist;
     closest_e2_fighter = e2;
     actual_dist_fighter = dist;
    }
  }
   else
   {
    if (dist > closest_dist_wship)
     continue; // do a quick and rough test first before we go into hypot:
    dist = hypot(y - ship[a2][e2].y, x - ship[a2][e2].x);
    if (dist < closest_dist_wship)
    {
      closest_dist_wship = dist;
      closest_e2_wship = e2;
      actual_dist_wship = dist;
    }
   }
 }

 if (a == TEAM_ENEMY && EE.turret_class [t] != ATTACK_ONLY_WSHIP)
 {
  int p;

  for (p = 0; p < arena.players; p ++)
  {
   if (PP.alive == 0)
    continue;
   dist = abs(y - PP.y) + abs(x - PP.x);
//   if (PP.priority_target > 1)
//    dist /= PP.priority_target;
   if (dist > closest_dist_fighter)
    continue; // do a quick and rough test first before we go into hypot:
   dist = hypot(y - PP.y, x - PP.x);
   dist2 = dist;
   if (PP.priority_target > 1)
    dist /= PP.priority_target;
   if (dist < closest_dist_fighter)
   {
    closest_dist_fighter = dist;
    actual_dist_fighter = dist2;
    if (p == 0)
     closest_e2_fighter = TARGET_P1;
      else
       closest_e2_fighter = TARGET_P2;
   }
  }


 }

 int target_dist = 99999999;

 switch(EE.turret_class [t])
 {
  case ATTACK_ONLY_WSHIP:
   if (closest_dist_wship <= EE.turret_target_range [t])
   {
    EE.turret_target [t] = closest_e2_wship;
    target_dist = actual_dist_wship;
   }
    break;
  case ATTACK_ONLY_FIGHTER:
   if (closest_dist_fighter <= EE.turret_target_range [t])
   {
    EE.turret_target [t] = closest_e2_fighter;
    target_dist = actual_dist_fighter;
   }
    break;
  case ATTACK_PREF_FIGHTER:
   if (closest_dist_fighter <= EE.turret_target_range [t])
   {
    EE.turret_target [t] = closest_e2_fighter;
    target_dist = actual_dist_fighter;
   }
     else
     {
      if (closest_dist_wship <= EE.turret_target_range [t])
      {
       EE.turret_target [t] = closest_e2_wship;
       target_dist = actual_dist_wship;
      }
     }
   break;
  case ATTACK_PREF_WSHIP:
   if (closest_dist_wship <= EE.turret_target_range [t])
   {
    EE.turret_target [t] = closest_e2_wship;
    target_dist = actual_dist_wship;
   }
     else
     {
      if (closest_dist_fighter <= EE.turret_target_range [t])
      {
       EE.turret_target [t] = closest_e2_fighter;
       target_dist = actual_dist_fighter;
      }
     }
   break;
  case ATTACK_ANY:
   if (closest_dist_wship <= EE.turret_target_range [t])
   {
    EE.turret_target [t] = closest_e2_wship;
    target_dist = actual_dist_wship;
   }
   if (closest_dist_fighter <= EE.turret_target_range [t]
    && closest_dist_fighter <= closest_dist_wship)
   {
    EE.turret_target [t] = closest_e2_fighter;
    target_dist = actual_dist_fighter;
   }
    break;
 }

 if (EE.turret_burst [t] == BURST_WAITING
  && target_dist < EE.turret_firing_range [t]
  && EE.turret_target [t] != TARGET_NONE)
   EE.turret_burst [t] = BURST_FIRING;

}


// uses variable x and y values because if e.g. the fighter is escorting a bomber, it attacks the fighter
//  closest to the bomber.
int fighter_find_target(int a, int e, int x, int y)
{

 int a2 = a ^ 1;
 int e2;


 if (eclass[EE.type].fighter_class == FCLASS_BOMBER
  && EE.leader != -1)
 {
  EE.target = ship[a][EE.leader].target;
  return 10000;
 }

 int dist;
 int closest_dist_wship = 99999999;
 int closest_e2_wship = TARGET_NONE;
 int closest_dist_fighter = 99999999;
 int closest_e2_fighter = TARGET_NONE;

 for (e2 = 0; e2 < NO_SHIPS; e2 ++)
 {
  if (ship[a2][e2].type == SHIP_NONE)
//   || ship[a2][e2].hp <= 0)
   continue;
  if (EE.attack == ATTACK_ONLY_FIGHTER
   && eclass[ship[a2][e2].type].ship_class == ECLASS_WSHIP)
    continue;
  if (EE.attack == ATTACK_ONLY_WSHIP
   && eclass[ship[a2][e2].type].ship_class == ECLASS_FIGHTER)
    continue;
  dist = abs(y - ship[a2][e2].y) + abs(x - ship[a2][e2].x);

  if (eclass[ship[a2][e2].type].ship_class == ECLASS_FIGHTER)
  {
   if (dist > closest_dist_fighter)
    continue; // do a quick and rough test first before we go into hypot:
   dist = hypot(y - ship[a2][e2].y, x - ship[a2][e2].x);
   if (dist < closest_dist_fighter)
   {
     closest_dist_fighter = dist;
     closest_e2_fighter = e2;
    }
  }
   else
   {
    if (dist > closest_dist_wship)
     continue; // do a quick and rough test first before we go into hypot:
    dist = hypot(y - ship[a2][e2].y, x - ship[a2][e2].x);
    if (dist < closest_dist_wship)
    {
      closest_dist_wship = dist;
      closest_e2_wship = e2;
    }
   }
 }

 if (a == TEAM_ENEMY && EE.attack != ATTACK_ONLY_WSHIP)
 {
  int p;

  for (p = 0; p < arena.players; p ++)
  {
   if (PP.alive == 0)
    continue;
   dist = abs(y - PP.y) + abs(x - PP.x);
   if (PP.priority_target > 1)
    dist /= PP.priority_target;
   if (dist > closest_dist_fighter)
    continue; // do a quick and rough test first before we go into hypot:
   dist = hypot(y - PP.y, x - PP.x);
   if (PP.priority_target > 1)
    dist /= PP.priority_target;
   if (dist < closest_dist_fighter)
   {
    closest_dist_fighter = dist;
    if (p == 0)
     closest_e2_fighter = TARGET_P1;
      else
       closest_e2_fighter = TARGET_P2;
   }
  }


 }

 EE.target = TARGET_NONE;

 switch(EE.attack)
 {
  case ATTACK_ONLY_WSHIP:
//   if (closest_dist_wship <= EE.target_range)
    EE.target = closest_e2_wship;
    break;
  case ATTACK_ONLY_FIGHTER:
//   if (closest_dist_fighter <= EE.target_range)
    EE.target = closest_e2_fighter;
    break;
  case ATTACK_PREF_FIGHTER:
   EE.target = closest_e2_fighter;
   if (closest_e2_fighter == TARGET_NONE
    || closest_dist_fighter > EE.target_range)
   {
    if (closest_dist_wship <= EE.target_range)
     EE.target = closest_e2_wship;
   }
   break;
  case ATTACK_PREF_WSHIP:
   EE.target = closest_e2_wship;
   if (closest_dist_wship == TARGET_NONE
    || closest_dist_wship > EE.target_range)
    {
     if (closest_dist_fighter <= EE.target_range)
      EE.target = closest_e2_fighter;
    }
   break;
  case ATTACK_ANY:
   EE.target = closest_e2_wship;
   if (closest_dist_fighter <= closest_dist_wship)
    EE.target = closest_e2_fighter;
/*   if (closest_dist_wship <= EE.target_range)
    EE.target = closest_e2_wship;
   if (closest_dist_fighter <= EE.target_range
    && closest_dist_fighter <= closest_dist_wship)
    EE.target = closest_e2_fighter;*/
    break;
 }
/*
// for testing - makes player invisible
 if (EE.target == TARGET_P1
  || EE.target == TARGET_P2)
 EE.target = TARGET_NONE;*/

 if (EE.target == TARGET_NONE)
  return 9999999;
 if (EE.target == TARGET_P1
  || EE.target == TARGET_P2)
   return closest_dist_fighter;
 if (eclass[ship[a^1][EE.target].type].ship_class == ECLASS_FIGHTER)
   return closest_dist_fighter;

 return closest_dist_wship;

}




void fighter_find_wship_to_guard(int a, int e, int x, int y)
{

 int e2;


 int dist;
 int closest_dist_wship = 99999999;
 int closest_e2_wship = -1;

 for (e2 = 0; e2 < NO_SHIPS; e2 ++)
 {
  if (ship[a][e2].type == SHIP_NONE
   || eclass[ship[a][e2].type].ship_class != ECLASS_WSHIP
   || ship[a][e2].formation_size > 5)
   continue;

  dist = abs(y - ship[a][e2].y) + abs(x - ship[a][e2].x);

  if (dist > closest_dist_wship)
   continue; // do a quick and rough test first before we go into hypot:
  dist = hypot(y - ship[a][e2].y, x - ship[a][e2].x);
  if (dist < closest_dist_wship)
  {
    closest_dist_wship = dist;
    closest_e2_wship = e2;
  }
 }

 EE.leader = closest_e2_wship;

 if (closest_e2_wship != -1)
 {
  EE.target = TARGET_NONE;
  EE.action = ACT_GUARD;
  EE.mission = MISSION_GUARD;
  if (add_fighter_to_formation(a, e, closest_e2_wship))
   return;
 }

// can't find anything for you to do, sorry
 EE.mission = MISSION_SCRAMBLE;
 EE.action = ACT_AWAY;

}

// returns 1 on success, 0 on failure
// e is fighter, e2 is group leader, e3 is the ship being checked to see if it's part of e2's group
char add_fighter_to_formation(int a, int e, int e2)
{
  int existing_formation [6] = {1,0,0,0,0,0};

  int e3;

  if (ship[a][e2].formation_size == 0)
  {
   EE.leader = e2;
   EE.formation_position = 1;
   ship[a][e2].formation_size ++;
   return 1;
  }

  for (e3 = 0; e3 < NO_SHIPS; e3 ++)
  {
   if (ship[a][e3].leader == e2
    && ship[a][e3].formation_position > 0
    && ship[a][e3].formation_position < 6)
   {
    existing_formation [ship[a][e3].formation_position] = 1;
   }
  }

  int f;

// note: f starts at 1
  for (f = 1; f < 6; f ++)
  {
   if (existing_formation [f] == 0)
   {
    EE.leader = e2;
    EE.formation_position = f;
    ship[a][e2].formation_size ++;
    return 1;
   }
  }

 return 0;

}

/*
void fighter_find_target(int a, int e)
{

 int a2 = a ^ 1;
 int e2;

 int dist;
 int closest_dist = 9999999;
 int closest_e2 = TARGET_NONE;

 for (e2 = 0; e2 < NO_SHIPS; e2 ++)
 {
  if (ship[a2][e2].type == SHIP_NONE
   || ship[a2][e2].hp <= 0)
   continue;
  dist = abs(EE.y - ship[a2][e2].y) + abs(EE.x - ship[a2][e2].x);
  if (dist > closest_dist)
   continue; // do a quick and rough test first before we go into hypot:
  dist = hypot(EE.y - ship[a2][e2].y, EE.x - ship[a2][e2].x);
  if (dist < closest_dist)
  {
   closest_dist = dist;
   closest_e2 = e2;
  }
 }

 if (a == TEAM_ENEMY)
 {
  int p = 0;

  for (p = 0; p < arena.players; p ++)
  {
   dist = abs(EE.y - PP.y) + abs(EE.x - PP.x);
   if (dist > closest_dist)
    continue; // do a quick and rough test first before we go into hypot:
   dist = hypot(EE.y - PP.y, EE.x - PP.x);
   if (dist < closest_dist)
   {
    closest_dist = dist;
    if (p == 0)
     closest_e2 = TARGET_P1;
      else
       closest_e2 = TARGET_P2;
   }
  }


 }

 EE.target = closest_e2;


}

*/
/*
void ship_action(int a, int e)
{

 switch(EE.type)
 {
  case SHIP_FIGHTER:
   switch(EE.action)
   {
    case ACT_ATTACK:
     EE.action = ACT_AWAY;
     EE.think_count = 100;
     break;
    case ACT_SEEK:

     break;
   }

   EE.think_count = 10;
   break;


 }

}

*/

void move_ship(int a, int e)
{
//    return;
     EE.x += EE.x_speed;
     EE.y += EE.y_speed;

}

void fighter_engine(int a, int e)
{
 if (EE.engine_power > 0)
 {
  EE.engine [0] ++;
  if (EE.engine [0] > eclass[EE.type].engine_power [0])
   EE.engine [0] = eclass[EE.type].engine_power [0];
 }
  else
  {
      EE.engine [0] --;
      if (EE.engine [0] < 0)
       EE.engine [0] = 0;
  }
}

void move_wship(int a, int e)
{


/*      convoy_position(a, e);
      EE.old_x = EE.x;
      EE.old_y = EE.y;

            EE.target_angle = convoy[EE.convoy].angle;
            int angle_diff = angle_difference(EE.angle, EE.target_angle);

            if (angle_diff > ANGLE_64)
            {
             EE.turning = delta_turn_towards_angle(EE.angle, EE.target_angle, 1);
             EE.turning_time = angle_diff / EE.turn_speed;
             if (EE.turning_time > 20)
              EE.turning_time = 20;
             EE.think_count = EE.turning_time;// * 2;
//             EE.wship_throttle = convoy[EE.convoy].throttle >> 1;
            }
             else
             {
              EE.turning = 0;
              EE.turning_time = 20;
              EE.think_count = 20;
             }

 int tangle = angle_to_convoy_position(a, e);

 EE.x_speed += xpart(tangle, EE.wship_throttle<<2);
 EE.y_speed += ypart(tangle, EE.wship_throttle<<2);

 EE.x += EE.x_speed;
 EE.y += EE.y_speed;

// EE.x_speed += xpart(EE.angle, EE.wship_throttle);
// EE.y_speed += ypart(EE.angle, EE.wship_throttle);

 EE.x_speed *= WSHIP_DRAG;
 EE.x_speed >>= 10;
 EE.y_speed *= WSHIP_DRAG;
 EE.y_speed >>= 10;



    if (EE.turning == -1)
    {
//      EE.engine [1] = 10;
      EE.angle -= EE.turn_speed;
      EE.angle &= ANGLE_MASK;
    }

    if (EE.turning == 1)
    {
//      EE.engine [0] = 10;
      EE.angle += EE.turn_speed;
      EE.angle &= ANGLE_MASK;
    }


    return;

      convoy_position(a, e);
      EE.old_x = EE.x;
      EE.old_y = EE.y;
      EE.x = EE.convoy_x;
      EE.y = EE.convoy_y;
      EE.angle = convoy[EE.convoy].angle;

      EE.x_speed = EE.x - EE.old_x;
      EE.y_speed = EE.y - EE.old_y;
      return;

      EE.think_count --;
      EE.turning_time --;


      convoy_position(a, e);

      if (EE.turning_time <= 0)
      {

              EE.turning_time = 20;


            EE.target_angle = convoy[EE.convoy].angle;
            int angle_diff = angle_difference(EE.angle, EE.target_angle);

            if (angle_diff > ANGLE_64)
            {
             EE.turning = delta_turn_towards_angle(EE.angle, EE.target_angle, 1);
             EE.turning_time = angle_diff / EE.turn_speed;
             if (EE.turning_time > 20)
              EE.turning_time = 20;
             EE.think_count = EE.turning_time;// * 2;
             EE.wship_throttle = convoy[EE.convoy].throttle >> 1;
            }
             else
             {
              EE.turning = 0;
              EE.turning_time = 20;
              EE.think_count = 20;
             }
      }

 int cangle = angle_to_convoy_position(a, e);

 EE.x_speed += xpart(cangle, EE.wship_throttle*2);
 EE.y_speed += ypart(cangle, EE.wship_throttle*2);



    if (EE.turning == -1)
    {
//      EE.engine [1] = 10;
      EE.angle -= EE.turn_speed;
      EE.angle &= ANGLE_MASK;
    }

    if (EE.turning == 1)
    {
//      EE.engine [0] = 10;
      EE.angle += EE.turn_speed;
      EE.angle &= ANGLE_MASK;
    }

 EE.x += EE.x_speed;
 EE.y += EE.y_speed;

 EE.x_speed += xpart(EE.angle, EE.wship_throttle);
 EE.y_speed += ypart(EE.angle, EE.wship_throttle);

 EE.x_speed *= WSHIP_DRAG;
 EE.x_speed >>= 10;
 EE.y_speed *= WSHIP_DRAG;
 EE.y_speed >>= 10;



return;*/


 if (eclass[EE.type].engine_output == 0) // probably a base
 {
  EE.angle += EE.turning;
  return;
 }

 int dist, angle_diff;

      EE.think_count --;
      EE.turning_time --;
      if (EE.turning_time <= 0)
       EE.turning = 0;
     if (EE.think_count <= 0)
     {
      convoy_position(a, e);

        EE.think_count = 10;
        EE.wship_throttle = convoy[EE.convoy].throttle;
        EE.engine_power = convoy[EE.convoy].throttle;

        if (abs(EE.x - EE.convoy_x) < 100000 && abs(EE.y - EE.convoy_y) < 100000
         && (dist = xyedist(EE.convoy_x, EE.convoy_y, a, e)) < 100000)
         {
          if (abs(EE.x - EE.convoy_x) < 60000 && abs(EE.y - EE.convoy_y) < 60000
           && (dist = xyedist(EE.convoy_x, EE.convoy_y, a, e)) < 60000)
           {
            if (convoy[EE.convoy].throttle > 0)
             EE.wship_throttle = convoy[EE.convoy].throttle - 1;
            EE.target_angle = convoy[EE.convoy].angle;
            angle_diff = angle_difference(EE.angle, EE.target_angle);

            if (angle_diff > ANGLE_64)
            {
             EE.turning = delta_turn_towards_angle(EE.angle, EE.target_angle, 1);
             EE.turning_time = angle_diff / eclass[EE.type].turn;
             if (EE.turning_time > 20)
              EE.turning_time = 20;
             EE.think_count = EE.turning_time;// * 2;
             EE.wship_throttle = convoy[EE.convoy].throttle >> 1;
            }
             else
             {
              EE.turning = 0;
              EE.turning_time = 20;
              EE.think_count = 20;
             }

            if (angle_diff > ANGLE_2 - ANGLE_8)
            {
              EE.turning = 0;
              EE.wship_throttle = 0;
              EE.think_count = 20;
              EE.turning_time = 20;
            }

           }
            else
            {

             EE.target_angle = angle_to_convoy_position(a, e);
             angle_diff = angle_difference(EE.angle, EE.target_angle);

             EE.wship_throttle = convoy[EE.convoy].throttle; // match speed with leader

             if (angle_diff > ANGLE_2 - ANGLE_8)
             {
               EE.turning = 0;
               EE.wship_throttle = 0;
               EE.think_count = 20;
               EE.turning_time = 20;
             }
            }

         }
          else
          {
           EE.wship_throttle = convoy[EE.convoy].throttle;
           EE.target_angle = angle_to_convoy_position(a, e);

           angle_diff = angle_difference(EE.angle, EE.target_angle);
           if (angle_diff > ANGLE_32)
           {
            EE.turning = delta_turn_towards_angle(EE.angle, EE.target_angle, 1);
            EE.turning_time = angle_diff / eclass[EE.type].turn;
            if (EE.turning_time > 20)
             EE.turning_time = 20;
            EE.think_count = EE.turning_time;// * 2;
           }

             if (angle_diff > ANGLE_2 - ANGLE_8)
             {
               EE.turning = 0;
               EE.wship_throttle = 0;
               EE.think_count = 20;
               EE.turning_time = 20;
             }


          }
     }


 int cangle = angle_to_convoy_position(a, e);

// EE.x_speed += xpart(cangle, EE.wship_throttle);
// EE.y_speed += ypart(cangle, EE.wship_throttle);
 EE.x_speed += xpart(cangle, 5);
 EE.y_speed += ypart(cangle, 5);



    if (EE.turning == -1)
    {
//      EE.engine [1] = 10;
      EE.angle -= eclass[EE.type].turn;
      EE.angle &= ANGLE_MASK;
    }

    if (EE.turning == 1)
    {
//      EE.engine [0] = 10;
      EE.angle += eclass[EE.type].turn;
      EE.angle &= ANGLE_MASK;
    }
/*
 if (EE.wship_throttle == 0)
 {
  EE.x_speed = 0;
  EE.y_speed = 0;
 }*/

 EE.x += EE.x_speed;
 EE.y += EE.y_speed;

 EE.x_speed += xpart(EE.angle, EE.wship_throttle);
 EE.y_speed += ypart(EE.angle, EE.wship_throttle);

 EE.x_speed *= (1023-WSHIP_DRAG);
 EE.x_speed >>= 10;
 EE.y_speed *= (1023-WSHIP_DRAG);
 EE.y_speed >>= 10;


}

void drag_ship(int a, int e, int dragged)
{
     EE.x_speed *= dragged;
     EE.x_speed >>= 10;
     EE.y_speed *= dragged;
     EE.y_speed >>= 10;
}


void count_ship_sprite(int a, int e, int sprite1, int sprite2, int inc)
{

 EE.sprite_count += inc;

 if (EE.sprite_count < 0)
 {
  if (EE.sprite > sprite1)
  {
   EE.sprite --;
   EE.sprite_count = 100;
  }
   else
    EE.sprite_count = 0;
 }

 if (EE.sprite_count > 100)
 {
  if (EE.sprite < sprite2)
  {
   EE.sprite ++;
   EE.sprite_count = 0;
  }
   else
    EE.sprite_count = 100;
 }


}


void ship_fire(int a, int e, int burst_status, int fire_type)
{
 int angle = EE.angle;
 int b, c, i;

 switch(EE.type)
 {
  case SHIP_FIGHTER:
  case SHIP_EINT:
  b = create_bullet(BULLET_ESHOT1, a);
  if (b != -1)
  {
   BL.x = EE.x + xpart(angle, 10000);
   BL.y = EE.y + ypart(angle, 10000);
   BL.angle = angle;
   BL.x_speed = EE.x_speed + xpart(angle, eclass[EE.type].bullet_speed);
   BL.y_speed = EE.y_speed + ypart(angle, eclass[EE.type].bullet_speed);
   BL.damage = 300;
   BL.timeout = 40;
   BL.colour = 2;
   BL.draw_size = 2;
   BL.status = 3;
   BL.owner = e;
   BL.owner_t = 0;
 //  play_effectwfvx(WAV_FIRE, 500 + grand(1000), 200, bullet[a][b].x);
//   EE.x_speed -= xpart(angle, 400);
//   EE.y_speed -= ypart(angle, 400); // a little bit of recoil

   c = simple_cloud(CLOUD_BALL_COL2, BL.x, BL.y, 2, 25);
   if (c != -1)
   {
    cloud[c].x_speed = EE.x_speed;
    cloud[c].y_speed = EE.y_speed;
   }


   EE.recycle = 9;

   if (burst_status == BURST_START)
   {
     EE.burst = 4;
   }
    else
    {
     EE.burst --;
     if (EE.burst <= 0)
     {
        EE.recycle = 40;
        set_act_evade(a, e);
     }
    }
  play_effectwfvxy_xs_ys(WAV_EBASIC, SPRIORITY_LOW, 1500, 140, BL.x, BL.y, BL.x_speed, BL.y_speed);

  }
  break;
  case SHIP_ESCOUT:
  for (i = 0; i < 2; i ++)
  {
  b = create_bullet(BULLET_ESHOT1, a);
//  angle += grand(ANGLE_16) - ANGLE_32;
  if (b != -1)
  {
   if (i == 0)//EE.burst & 1)
   {
    BL.x = EE.x + xpart(angle, 7000) + xpart(angle - ANGLE_4, 5000);
    BL.y = EE.y + ypart(angle, 7000) + ypart(angle - ANGLE_4, 5000);
   }
    else
    {
     BL.x = EE.x + xpart(angle, 7000) + xpart(angle + ANGLE_4, 5000);
     BL.y = EE.y + ypart(angle, 7000) + ypart(angle + ANGLE_4, 5000);
    }
   BL.angle = angle;
   BL.x_speed = EE.x_speed + xpart(angle, eclass[EE.type].bullet_speed);
   BL.y_speed = EE.y_speed + ypart(angle, eclass[EE.type].bullet_speed);
   BL.damage = 300;
   BL.timeout = 40;
   BL.colour = 2;
   BL.draw_size = 2;
   BL.status = 3;
   BL.owner = e;
   BL.owner_t = 0;
 //  play_effectwfvx(WAV_FIRE, 500 + grand(1000), 200, bullet[a][b].x);
//   EE.x_speed -= xpart(angle, 400);
//   EE.y_speed -= ypart(angle, 400); // a little bit of recoil

   c = simple_cloud(CLOUD_BALL_COL2, BL.x, BL.y, 2, 25);
   if (c != -1)
   {
    cloud[c].x_speed = EE.x_speed;
    cloud[c].y_speed = EE.y_speed;
   }

   if (i == 0)
    play_effectwfvxy_xs_ys(WAV_EBASIC, SPRIORITY_LOW, 1500, 140, BL.x, BL.y, BL.x_speed, BL.y_speed);

  } // end if b != -1
  } // end i(0-1) loop
   EE.recycle = 11;

   if (burst_status == BURST_START)
   {
     EE.burst = 4 + grand(4);
   }
    else
    {
     EE.burst --;
     if (EE.burst <= 0)
      EE.recycle = 40;
    }

    if (EE.recycle2 == 0
       && (EE.target != TARGET_NONE && target_ship_class(a^1, EE.target) == ECLASS_FIGHTER))
   {
    EE.recycle2 = fighter_fire(a, e, -1, WPN_E_AF_MISSILE,60,EE.target);


   }

  break;
  case SHIP_BOMBER:
  b = create_bullet(BULLET_ETORP1, a);
  if (b != -1)
  {
   BL.x = EE.x + xpart(angle, 7000);
   BL.y = EE.y + ypart(angle, 7000);
   BL.angle = angle;
   BL.x_speed = EE.x_speed + xpart(angle, eclass[EE.type].bullet_speed);
   BL.y_speed = EE.y_speed + ypart(angle, eclass[EE.type].bullet_speed);
   BL.damage = 5000;
   BL.timeout = 40;
   BL.colour = 2;
   BL.draw_size = 3;
   BL.status = 2;
   BL.owner = e;
   BL.owner_t = 0;
 //  play_effectwfvx(WAV_FIRE, 500 + grand(1000), 200, bullet[a][b].x);
//   EE.x_speed -= xpart(angle, 400);
//   EE.y_speed -= ypart(angle, 400); // a little bit of recoil

   c = simple_cloud(CLOUD_BALL_COL2, BL.x, BL.y, 2, 25);
   if (c != -1)
   {
    cloud[c].x_speed = EE.x_speed;
    cloud[c].y_speed = EE.y_speed;
   }
   play_effectwfvxy_xs_ys(WAV_WHOOSH2, SPRIORITY_LOW, 1500, 200, BL.x, BL.y, BL.x_speed, BL.y_speed);
  }
  break;
  case SHIP_FIGHTER_FRIEND:

    EE.recycle = fighter_fire(a, e, -1, WPN_AUTOCANNON,0,0);

   if (burst_status == BURST_START)
   {
     EE.burst = 2;
   }
    else
    {
     EE.burst --;
    }

  if (EE.recycle2 == 0
       && (EE.target != TARGET_NONE && target_ship_class(a^1, EE.target) == ECLASS_FIGHTER))
   {
    EE.recycle2 = fighter_fire(a, e, -1, WPN_LW_MISSILE,60,EE.target);
// recycle2 is dealt with in the special case for SANDFLY fighters.
   }
  break;

  case SHIP_LACEWING:

    EE.recycle = fighter_fire(a, e, -1, WPN_AUTOCANNON,0,0);

   if (burst_status == BURST_START)
   {
     EE.burst = 2;
   }
    else
    {
     EE.burst --;
    }

  if (EE.recycle2 == 0
       && (EE.target != TARGET_NONE && target_ship_class(a^1, EE.target) == ECLASS_FIGHTER))
   {
    EE.recycle2 = fighter_fire(a, e, -1, WPN_LW_MISSILE,60,EE.target);
// recycle2 is dealt with in the special case for Lacewing fighters.
   }

  break;
  case SHIP_MONARCH:

    EE.recycle = fighter_fire(a, e, -1, WPN_AUTOCANNON,0,0);

   if (burst_status == BURST_START)
   {
     EE.burst = 2;
   }
    else
    {
     EE.burst --;
    }

  if (EE.recycle2 == 0
       && (EE.target != TARGET_NONE && target_ship_class(a^1, EE.target) == ECLASS_FIGHTER))
   {
    EE.recycle2 = fighter_fire(a, e, -1, WPN_LW_MISSILE,60,EE.target);
// recycle2 is dealt with in the special case for Monarch fighters.
   }

  break;
  case SHIP_FSTRIKE:
    if (fire_type == ECLASS_WSHIP
     || (EE.target != TARGET_NONE && target_ship_class(a^1, EE.target) == ECLASS_WSHIP))
    {
     fighter_fire(a, e, -1, WPN_WROCKET,0,0);
         EE.recycle = 80;
/*
     for (i = 0; i < 2; i ++)
     {

        b = create_bullet(BULLET_ROCKET, a);
        angle = EE.angle + ANGLE_4;
        if (i == 1)
        angle = EE.angle - ANGLE_4;

        if (b != -1)
        {
         BL.x = EE.x + xpart(angle, 8000);
         BL.y = EE.y + ypart(angle, 8000);
         BL.x_speed = EE.x_speed + xpart(angle, 200);
         BL.y_speed = EE.y_speed + ypart(angle, 200);
         BL.x2 = BL.x;
         BL.y2 = BL.y;
         BL.x3 = EE.x_speed;
         BL.y3 = EE.y_speed;
         BL.timeout = 90+grand(20);
         BL.colour = 0;
         BL.angle = EE.angle;
         BL.size = 5000;
         BL.damage = 700; // is *6 after priming
         BL.force = 100; // is *6 after priming
         BL.turning = 0;
         BL.status = 30;
         BL.status2 = 0;
         BL.status3 = -1;
         BL.owner = e;
         BL.owner_t = 0;
         EE.recycle = 80;


        }
       }*/

       /*
        b = create_bullet(BULLET_ROCKET2, a);

        if (b != -1)
        {
         BL.x = EE.x + xpart(EE.angle, 8000);
         BL.y = EE.y + ypart(EE.angle, 8000);
         BL.x_speed = EE.x_speed + xpart(EE.angle, 100);
         BL.y_speed = EE.y_speed + ypart(EE.angle, 100);
         BL.timeout = 100;
         BL.colour = 0;
         BL.angle = EE.angle;
         BL.size = 5000;
         BL.damage = 2500;
         BL.force = 400;
         BL.status = -1; // link to first cloud
         BL.status3 = -1;
         BL.owner = e;
         BL.owner_t = 0;
         EE.recycle = 40;
         if (b != -1)
           play_effectwfvxy_xs_ys(WAV_WHOOSH2, SPRIORITY_LOW, 1500, 80, BL.x, BL.y, EE.x_speed, EE.y_speed);
        }
        */
        break;
      }
// must be attacking a fighter, so use cannons (maybe should use rockets vs bombers?)
    EE.recycle = fighter_fire(a, e, -1, WPN_AUTOCANNON,0,0);
//    EE.recycle = 12;

/*
  for (i = 0; i < 2; i ++)
  {
   b = create_bullet(BULLET_SHOT, a);
   if (b != -1)
   {
    if (i == 0)
    {
     BL.x = EE.x + xpart(angle, 12000) + xpart(angle + ANGLE_4, 5000);
     BL.y = EE.y + ypart(angle, 12000) + ypart(angle + ANGLE_4, 5000);
    }
     else
     {
      BL.x = EE.x + xpart(angle, 12000) + xpart(angle - ANGLE_4, 5000);
      BL.y = EE.y + ypart(angle, 12000) + ypart(angle - ANGLE_4, 5000);
     }
    BL.angle = angle;
    BL.x_speed = EE.x_speed + xpart(angle, eclass[EE.type].bullet_speed);
    BL.y_speed = EE.y_speed + ypart(angle, eclass[EE.type].bullet_speed);
    BL.damage = 300;
    BL.timeout = 35;
    BL.colour = 0;
    BL.draw_size = 3;
    BL.status = 3;
    BL.owner = e;
    BL.owner_t = 0;
 //  play_effectwfvx(WAV_FIRE, 500 + grand(1000), 200, bullet[a][b].x);
//    EE.x_speed -= xpart(angle, 200);
//    EE.y_speed -= ypart(angle, 200); // a little bit of recoil

    c = simple_cloud(CLOUD_BALL_COL2, BL.x, BL.y, 0, 25);
    if (c != -1)
    {
     cloud[c].x_speed = EE.x_speed;
     cloud[c].y_speed = EE.y_speed;
    }
   }
  }*/


/*
   if (burst_status == BURST_START)
   {
     EE.burst = 3;
   }
    else
    {
     EE.burst --;
    }*/

//  play_effectwfv(WAV_FIRE, 3000, 100);
//  if (b != -1)
//   play_effectwfvxy_xs_ys(WAV_FIRE, SPRIORITY_LOW, 1500, 200, BL.x, BL.y, EE.x_speed, EE.y_speed);
  break;
  case SHIP_IBEX:
   if (EE.player_leader != -1 && EE.action == ACT_WING_FORM)
   {
    if (player[EE.player_leader].weapon_target [0] [0] != TARGET_NONE)
    {
     EE.target = player[EE.player_leader].weapon_target [0] [0];
    }
   }
    if (fire_type == ECLASS_WSHIP
     || (EE.target != TARGET_NONE && target_ship_class(a^1, EE.target) == ECLASS_WSHIP))
    {
         EE.recycle = fighter_fire(a, e, -1, WPN_AWS_MISSILE,4500, EE.target);

        break;
      }
    EE.recycle = fighter_fire(a, e, -1, WPN_AUTOCANNON,0,0);

  break;
  case SHIP_AUROCHS:
   if (EE.player_leader != -1 && EE.action == ACT_WING_FORM)
   {
    if (player[EE.player_leader].weapon_target [0] [0] != TARGET_NONE)
    {
     EE.target = player[EE.player_leader].weapon_target [0] [0];
    }
   }
    if (fire_type == ECLASS_WSHIP
     || (EE.target != TARGET_NONE && target_ship_class(a^1, EE.target) == ECLASS_WSHIP))
    {
         EE.recycle = fighter_fire(a, e, -1, WPN_HROCKET,0, 0);

        break;
      }
    EE.recycle = fighter_fire(a, e, -1, WPN_AUTOCANNON,0,0);

  break;
 }
//      play_effectwfvx(w, NWAV_ZAP, 1800 + grand(50), 30, ship[a][w][e].x);

}

void turret_fire(int a, int e, int t)
{
 int angle = EE.turret_angle [t];
 int b, c;
 int i;
 int range = EE.turret_firing_range [t]; // fix!!!

// note - some turrets can return without firing

 switch(EE.turret_type [t])
 {
 case TURRET_EBASIC:
 if (EE.turret_status [t] > 0)
 {
  EE.turret_status [t] --;
  return;
 }
 angle += grand(ANGLE_16) - grand(ANGLE_16);

 for (i = 0; i < 2; i ++)
 {

 b = create_bullet(BULLET_ESHOT1, a);
 if (b != -1)
 {
  if (i == 0)
  {
   BL.x = EE.turret_x [t] + xpart(angle, 6000) + xpart(angle + ANGLE_4, 3000);
   BL.y = EE.turret_y [t] + ypart(angle, 6000) + ypart(angle + ANGLE_4, 3000);
  }
   else
   {
    BL.x = EE.turret_x [t] + xpart(angle, 6000) + xpart(angle - ANGLE_4, 3000);
    BL.y = EE.turret_y [t] + ypart(angle, 6000) + ypart(angle - ANGLE_4, 3000);
   }
  BL.angle = angle;
  BL.x_speed = EE.x_speed + xpart(angle, EE.turret_bullet_speed [t]); // if speed changes, must change in the call to turret_track_target_lead
  BL.y_speed = EE.y_speed + ypart(angle, EE.turret_bullet_speed [t]);
  BL.damage = 300;
  BL.timeout = 90;
  BL.colour = 2;
  if (a == TEAM_FRIEND)
   BL.colour = 0;
  BL.draw_size = 2;
  BL.status = 3;
  BL.owner = e;
  BL.owner_t = t;
//  play_effectwfvx(WAV_FIRE, 500 + grand(1000), 200, bullet[a][b].x);
//  EE.x_speed -= xpart(angle, 400);
  //EE.y_speed -= ypart(angle, 400); // a little bit of recoil

  c = simple_cloud(CLOUD_BALL_COL2, BL.x, BL.y, 2, 20);
  if (c != -1)
  {
   cloud[c].x_speed = EE.x_speed;
   cloud[c].y_speed = EE.y_speed;
  }

 }
 if (i == 0)
  play_effectwfvxy_xs_ys(WAV_EBASIC, SPRIORITY_LOW, 1400, 100, BL.x, BL.y, BL.x_speed, BL.y_speed);

 }


 range = EE.turret_target_range [t];

 if (EE.turret_target [t] != TARGET_NONE
  && EE.turret_energy [t] < (EE.turret_energy_use [t] << 1)) // only enough energy for this shot
 {
  int x = get_target_x(a, EE.turret_target [t], EE.x);
  int y = get_target_y(a, EE.turret_target [t], EE.y);

  int dist;
  if (abs(EE.y - y) < range && abs(EE.x - x) < range) // do a quick and rough test first before we go into hypot:
  {
   dist = hypot(EE.y - y, EE.x - x);
   if (dist > range)
    EE.turret_target [t] = TARGET_NONE;
  }
   else
    EE.turret_target [t] = TARGET_NONE;
 }

   EE.turret_recoil [t] = 4;


//      play_effectwfvx(w, NWAV_ZAP, 1800 + grand(50), 30, ship[a][w][e].x);
 EE.turret_recycle [t] = EE.turret_recycle_time [t];
 EE.turret_energy [t] -= EE.turret_energy_use [t];

   if (EE.turret_energy [t] <= EE.turret_energy_use [t]
    || EE.turret_target [t] == TARGET_NONE)
   {
    EE.turret_burst [t] = BURST_CHARGING;
    EE.turret_recoil [t] = 15;
   }

 break;

 case TURRET_EANTI:
 if (EE.turret_status [t] > 0)
 {
  EE.turret_status [t] --;
  return;
 }

 i = EE.turret_status2 [t];
 EE.turret_status2 [t] ^= 1;
 angle += grand(ANGLE_16) - grand(ANGLE_16);

 b = create_bullet(BULLET_ESHOT2, a);
 if (b != -1)
 {
  if (i == 0)
  {
   BL.x = EE.turret_x [t] + xpart(angle, 9000) + xpart(angle + ANGLE_4, 4000);
   BL.y = EE.turret_y [t] + ypart(angle, 9000) + ypart(angle + ANGLE_4, 4000);
  }
   else
   {
    BL.x = EE.turret_x [t] + xpart(angle, 9000) + xpart(angle - ANGLE_4, 4000);
    BL.y = EE.turret_y [t] + ypart(angle, 9000) + ypart(angle - ANGLE_4, 4000);
   }
  BL.angle = angle;
  BL.x_speed = EE.x_speed + xpart(angle, EE.turret_bullet_speed [t]); // if speed changes, must change in the call to turret_track_target_lead
  BL.y_speed = EE.y_speed + ypart(angle, EE.turret_bullet_speed [t]);
  BL.damage = 450;
  BL.timeout = 70;
  BL.colour = 2;
  if (a == TEAM_FRIEND)
   BL.colour = 0;
  BL.draw_size = 2;
  BL.status = 3;
  BL.owner = e;
  BL.owner_t = t;
//  play_effectwfvx(WAV_FIRE, 500 + grand(1000), 200, bullet[a][b].x);
//  EE.x_speed -= xpart(angle, 400);
  //EE.y_speed -= ypart(angle, 400); // a little bit of recoil

  c = simple_cloud(CLOUD_BALL_COL3, BL.x, BL.y, 2, 20);
  if (c != -1)
  {
   cloud[c].x_speed = EE.x_speed;
   cloud[c].y_speed = EE.y_speed;
   cloud[c].x += EE.x_speed;
   cloud[c].y += EE.y_speed;
  }

  play_effectwfvxy_xs_ys(WAV_EBASIC, SPRIORITY_LOW, 1000, 140, BL.x, BL.y, BL.x_speed, BL.y_speed);

 }


 range = EE.turret_target_range [t];

 if (EE.turret_target [t] != TARGET_NONE)
 {
  int x = get_target_x(a, EE.turret_target [t], EE.x);
  int y = get_target_y(a, EE.turret_target [t], EE.y);

  int dist;
  if (abs(EE.y - y) < range && abs(EE.x - x) < range) // do a quick and rough test first before we go into hypot:
  {
   dist = hypot(EE.y - y, EE.x - x);
   if (dist > range)
    EE.turret_target [t] = TARGET_NONE;
  }
   else
    EE.turret_target [t] = TARGET_NONE;
 }

   EE.turret_recoil [t] = 4;


//      play_effectwfvx(w, NWAV_ZAP, 1800 + grand(50), 30, ship[a][w][e].x);
 EE.turret_recycle [t] = EE.turret_recycle_time [t];
 EE.turret_energy [t] -= EE.turret_energy_use [t];

   if (EE.turret_energy [t] <= EE.turret_energy_use [t]
    || EE.turret_target [t] == TARGET_NONE)
   {
    EE.turret_burst [t] = BURST_CHARGING;
    EE.turret_recoil [t] = 15;
   }

 break;



 case TURRET_EHEAVY:
 if (EE.turret_status [t] > 0)
 {
  EE.turret_status [t] -= 2;
  return;
 }
 angle += grand(140) - grand(140);
 b = create_bullet(BULLET_EBIGSHOT, a);
 if (b != -1)
 {
  BL.x = EE.turret_x [t] + xpart(angle, 9000);
  BL.y = EE.turret_y [t] + ypart(angle, 9000);
  BL.angle = angle;
  BL.x_speed = EE.x_speed + xpart(angle, EE.turret_bullet_speed [t]);
  BL.y_speed = EE.y_speed + ypart(angle, EE.turret_bullet_speed [t]);
  BL.x2 = EE.x_speed;
  BL.y2 = EE.y_speed;
  BL.damage = 11000;
  BL.timeout = 260;//140;
  BL.colour = 2;
  if (a == TEAM_FRIEND)
   BL.colour = 0;
  BL.draw_size = 4;
  BL.status = 5;
  BL.owner = e;
  BL.owner_t = t;
//  play_effectwfvx(WAV_FIRE, 500 + grand(1000), 200, bullet[a][b].x);
//  EE.x_speed -= xpart(angle, 400);
  //EE.y_speed -= ypart(angle, 400); // a little bit of recoil


  quick_fire(BL.x, BL.y, EE.x_speed, EE.y_speed, 80, BL.colour);
/*
  c = simple_cloud(CLOUD_DOUBLE_BALL, BL.x, BL.y, BL.colour, 80);
  if (c != -1)
  {
   cloud[c].status = 2 + grand(3);
   cloud[c].x_speed = EE.x_speed;
   cloud[c].y_speed = EE.y_speed;
  }*/
  play_effectwfvxy_xs_ys(WAV_HEAVY, SPRIORITY_LOW, 600, 140, BL.x, BL.y, BL.x_speed, BL.y_speed);

 }

 range = EE.turret_target_range [t];

 if (EE.turret_target [t] != TARGET_NONE)
 {
  int x = get_target_x(a, EE.turret_target [t], EE.x);
  int y = get_target_y(a, EE.turret_target [t], EE.y);

  int dist;
  if (abs(EE.y - y) < range && abs(EE.x - x) < range) // do a quick and rough test first before we go into hypot:
  {
   dist = hypot(EE.y - y, EE.x - x);
   if (dist > range)
    EE.turret_target [t] = TARGET_NONE;
  }
   else
    EE.turret_target [t] = TARGET_NONE;
 }

 EE.turret_recoil [t] = 20;

 EE.turret_recycle [t] = EE.turret_recycle_time [t];
 EE.turret_energy [t] -= EE.turret_energy_use [t];


 break;

 case TURRET_ELONG:
 if (EE.turret_status [t] > 0)
 {
  EE.turret_status [t] -= 2;
  return;
 }
 b = create_bullet(BULLET_ELONGSHOT, a);
 if (b != -1)
 {
  BL.x = EE.turret_x [t] + xpart(angle, 9000);
  BL.y = EE.turret_y [t] + ypart(angle, 9000);
  BL.angle = grand(ANGLE_1);
  BL.x_speed = EE.x_speed + xpart(angle, EE.turret_bullet_speed [t]);
  BL.y_speed = EE.y_speed + ypart(angle, EE.turret_bullet_speed [t]);
  BL.x2 = EE.x_speed;
  BL.y2 = EE.y_speed;
  BL.damage = 1200;
  BL.timeout = 440;
  BL.colour = 2;
  if (a == TEAM_FRIEND)
   BL.colour = 0;
  BL.draw_size = 4;
  BL.status = pos_or_neg(20 + grand(20));
  BL.owner = e;
  BL.owner_t = t;
//  play_effectwfvx(WAV_FIRE, 500 + grand(1000), 200, bullet[a][b].x);
//  EE.x_speed -= xpart(angle, 400);
  //EE.y_speed -= ypart(angle, 400); // a little bit of recoil

  c = simple_cloud(CLOUD_DOUBLE_BALL, BL.x, BL.y, BL.colour, 60);
  if (c != -1)
  {
   cloud[c].status = 2 + grand(3);
   cloud[c].x_speed = EE.x_speed;
   cloud[c].y_speed = EE.y_speed;
  }
  play_effectwfvxy_xs_ys(WAV_HEAVY, SPRIORITY_LOW, 2500, 100, BL.x, BL.y, BL.x_speed, BL.y_speed);

 }

 range = EE.turret_target_range [t];

 if (EE.turret_target [t] != TARGET_NONE)
 {
  int x = get_target_x(a, EE.turret_target [t], EE.x);
  int y = get_target_y(a, EE.turret_target [t], EE.y);

  int dist;
  if (abs(EE.y - y) < range && abs(EE.x - x) < range) // do a quick and rough test first before we go into hypot:
  {
   dist = hypot(EE.y - y, EE.x - x);
   if (dist > range)
    EE.turret_target [t] = TARGET_NONE;
  }
   else
    EE.turret_target [t] = TARGET_NONE;
 }

 EE.turret_recoil [t] = 20;

 EE.turret_recycle [t] = EE.turret_recycle_time [t];
 EE.turret_energy [t] -= EE.turret_energy_use [t];


 break;







 case TURRET_BASIC:
 i = EE.turret_side [t];
 EE.turret_side [t] ^= 1;

 b = create_bullet(BULLET_SHOT, a);
 if (b != -1)
 {
  if (i == 0)
  {
   BL.x = EE.turret_x [t] + xpart(angle, 9000) + xpart(angle + ANGLE_4, 3000);
   BL.y = EE.turret_y [t] + ypart(angle, 9000) + ypart(angle + ANGLE_4, 3000);
  }
   else
   {
    BL.x = EE.turret_x [t] + xpart(angle, 9000) + xpart(angle - ANGLE_4, 3000);
    BL.y = EE.turret_y [t] + ypart(angle, 9000) + ypart(angle - ANGLE_4, 3000);
   }
  BL.angle = angle;
  BL.x_speed = EE.x_speed + xpart(angle, EE.turret_bullet_speed [t]);
  BL.y_speed = EE.y_speed + ypart(angle, EE.turret_bullet_speed [t]);
  BL.damage = 200;
  BL.timeout = 35;
  BL.colour = 2;
  if (a == TEAM_FRIEND)
   BL.colour = 0;
  BL.draw_size = 2;
  BL.status = 3;
  BL.owner = e;
  BL.owner_t = t;
//  play_effectwfvx(WAV_FIRE, 500 + grand(1000), 200, bullet[a][b].x);
//  EE.x_speed -= xpart(angle, 400);
  //EE.y_speed -= ypart(angle, 400); // a little bit of recoil

  c = simple_cloud(CLOUD_BALL_COL3, BL.x, BL.y, 0, 15);
  if (c != -1)
  {
   cloud[c].x_speed = EE.x_speed;
   cloud[c].y_speed = EE.y_speed;
  }

 play_effectwfvxy_xs_ys(WAV_FIRE, SPRIORITY_LOW, 1100, 150, BL.x, BL.y, EE.x_speed, EE.y_speed);


 }


 range = EE.turret_target_range [t];

 if (EE.turret_target [t] != TARGET_NONE)
 {
  int x = get_target_x(a, EE.turret_target [t], EE.x);
  int y = get_target_y(a, EE.turret_target [t], EE.y);

  int dist;
  if (abs(EE.y - y) < range && abs(EE.x - x) < range) // do a quick and rough test first before we go into hypot:
  {
   dist = hypot(EE.y - y, EE.x - x);
   if (dist > range)
    EE.turret_target [t] = TARGET_NONE;
  }
   else
    EE.turret_target [t] = TARGET_NONE;
 }


   EE.turret_recoil [t] = 4;
//      play_effectwfvx(w, NWAV_ZAP, 1800 + grand(50), 30, ship[a][w][e].x);
 EE.turret_recycle [t] = EE.turret_recycle_time [t];
 EE.turret_energy [t] -= EE.turret_energy_use [t];

 break;

 case TURRET_HEAVY:
// for (i = 0; i < 2; i ++)
// {
 i = EE.turret_side [t];
 EE.turret_side [t] ^= 1;
 b = create_bullet(BULLET_OLDSHOT, a);
 angle += grand(120) - grand(120);
 if (b != -1)
 {
  BL.x = EE.turret_x [t] + xpart(angle, 15000);
  BL.y = EE.turret_y [t] + ypart(angle, 15000);
  if (i == 0)
  {
   BL.x += xpart(angle + ANGLE_4, 4000);
   BL.y += ypart(angle + ANGLE_4, 4000);
  }
   else
   {
    BL.x += xpart(angle - ANGLE_4, 4000);
    BL.y += ypart(angle - ANGLE_4, 4000);
   }
  BL.angle = angle;
  BL.x_speed = EE.x_speed + xpart(angle, EE.turret_bullet_speed [t]);
  BL.y_speed = EE.y_speed + ypart(angle, EE.turret_bullet_speed [t]);
  BL.x2 = EE.x_speed;
  BL.y2 = EE.y_speed;
  BL.damage = 12000;
  BL.timeout = 200;//100;
  BL.colour = 2;
  if (a == TEAM_FRIEND)
   BL.colour = 0;
  BL.draw_size = 4;
  BL.status = 4;
  BL.owner = e;
  BL.owner_t = t;
  play_effectwfvxy_xs_ys(WAV_BLAT, SPRIORITY_LOW, 600 + grand(200), 150, BL.x, BL.y, EE.x_speed, EE.y_speed);

//  play_effectwfvx(WAV_FIRE, 500 + grand(1000), 200, bullet[a][b].x);
  EE.x_speed -= xpart(angle, 100);
  EE.y_speed -= ypart(angle, 100); // a little bit of recoil

 c = simple_cloud(CLOUD_FADEFLARE, BL.x, BL.y, BL.colour, 10 + grand(5));
 if (c != -1)
 {
  cloud[c].x_speed = EE.x_speed + xpart(angle, 2000);
  cloud[c].y_speed = EE.y_speed + ypart(angle, 2000);
  cloud[c].x2 = EE.x_speed;
  cloud[c].y2 = EE.y_speed;
 }

  quick_fire(BL.x, BL.y, EE.x_speed, EE.y_speed, 40 + grand(10), BL.colour);
/*

        c = quick_cloud(CLOUD_FADEBALL,
             BL.x,
             BL.y,
             0, 0, 30 + grand(10), BL.colour, 0);
        if (c != -1)
        {
         cloud[c].x_speed = EE.x_speed + xpart(angle, 2000);
         cloud[c].y_speed = EE.y_speed + ypart(angle, 2000);;
         CC.status = 30 + grand(13);
        }*/

/*
  c = simple_cloud(CLOUD_DOUBLE_BALL, BL.x, BL.y, BL.colour, 80);
  if (c != -1)
  {
   cloud[c].status = 2 + grand(3);
   cloud[c].x_speed = EE.x_speed;
   cloud[c].y_speed = EE.y_speed;
  }*/

 }
// }
 range = EE.turret_target_range [t];

 if (EE.turret_target [t] != TARGET_NONE)
 {
  int x = get_target_x(a, EE.turret_target [t], EE.x);
  int y = get_target_y(a, EE.turret_target [t], EE.y);

  int dist;
  if (abs(EE.y - y) < range && abs(EE.x - x) < range) // do a quick and rough test first before we go into hypot:
  {
   dist = hypot(EE.y - y, EE.x - x);
   if (dist > range)
    EE.turret_target [t] = TARGET_NONE;
  }
   else
    EE.turret_target [t] = TARGET_NONE;
 }

 EE.turret_recoil [t] = 25;

 EE.turret_recycle [t] = EE.turret_recycle_time [t];
 EE.turret_energy [t] -= EE.turret_energy_use [t];


 break;




 case TURRET_CGUN:
 b = create_bullet(BULLET_FSHOT, a);
 if (b != -1)
 {
  angle += grand(100) - grand(100);
  BL.x = EE.turret_x [t] + xpart(angle, 11000);
  BL.y = EE.turret_y [t] + ypart(angle, 11000);
  BL.angle = angle;
  BL.x_speed = EE.x_speed + xpart(angle, EE.turret_bullet_speed [t]);
  BL.y_speed = EE.y_speed + ypart(angle, EE.turret_bullet_speed [t]);
  BL.damage = 3000;
  BL.timeout = 105;
  BL.colour = 2;
  if (a == TEAM_FRIEND)
   BL.colour = 0;
  BL.draw_size = 3;
  BL.status = 7;
  BL.owner = e;
  BL.owner_t = t;
//  play_effectwfvx(WAV_FIRE, 500 + grand(1000), 200, bullet[a][b].x);
//  EE.x_speed -= xpart(angle, 400);
  //EE.y_speed -= ypart(angle, 400); // a little bit of recoil

  c = simple_cloud(CLOUD_BALL_COL3, BL.x, BL.y, 0, 35);
  if (c != -1)
  {
   cloud[c].x_speed = EE.x_speed;
   cloud[c].y_speed = EE.y_speed;
  }

  play_effectwfvxy_xs_ys(WAV_BLAT, SPRIORITY_LOW, 600 + grand(200), 150, BL.x, BL.y, EE.x_speed, EE.y_speed);


 }


 range = EE.turret_target_range [t];

 if (EE.turret_target [t] != TARGET_NONE)
 {
  int x = get_target_x(a, EE.turret_target [t], EE.x);
  int y = get_target_y(a, EE.turret_target [t], EE.y);

  int dist;
  if (abs(EE.y - y) < range && abs(EE.x - x) < range) // do a quick and rough test first before we go into hypot:
  {
   dist = hypot(EE.y - y, EE.x - x);
   if (dist > range)
    EE.turret_target [t] = TARGET_NONE;
  }
   else
    EE.turret_target [t] = TARGET_NONE;
 }


   EE.turret_recoil [t] = 8;
//      play_effectwfvx(w, NWAV_ZAP, 1800 + grand(50), 30, ship[a][w][e].x);
 EE.turret_recycle [t] = EE.turret_recycle_time [t];
 EE.turret_energy [t] -= EE.turret_energy_use [t];

 break;

 case TURRET_CLAUNCHER:
 b = create_bullet(BULLET_FROCK, a);
 EE.turret_side [t] ++;
 if (EE.turret_side [t] == 3)
  EE.turret_side [t] = 0;
 if (b != -1)
 {
  BL.x = EE.turret_x [t] + xpart(angle, 13000);
  BL.y = EE.turret_y [t] + ypart(angle, 13000);
  if (EE.turret_side [t] == 0)
  {
   BL.x += xpart(angle + ANGLE_4, 5000);
   BL.y += ypart(angle + ANGLE_4, 5000);
  }
  if (EE.turret_side [t] == 2)
  {
   BL.x += xpart(angle - ANGLE_4, 5000);
   BL.y += ypart(angle - ANGLE_4, 5000);
  }
  BL.angle = angle;
  BL.x_speed = EE.x_speed + xpart(angle, EE.turret_bullet_speed [t] - 2000);
  BL.y_speed = EE.y_speed + ypart(angle, EE.turret_bullet_speed [t] - 2000);
  BL.damage = 8000;
  BL.timeout = 205;
  BL.colour = 2;
  if (a == TEAM_FRIEND)
   BL.colour = 0;
  BL.owner = e;
  BL.owner_t = t;
  BL.status = -1; // link to first cloud

  c = simple_cloud(CLOUD_BALL_COL3, BL.x, BL.y, 0, 25);
  if (c != -1)
  {
   cloud[c].x_speed = EE.x_speed;
   cloud[c].y_speed = EE.y_speed;
  }
  c = simple_cloud(CLOUD_BALL_COL2, BL.x, BL.y, 0, 32);
  if (c != -1)
  {
   cloud[c].x_speed = EE.x_speed;
   cloud[c].y_speed = EE.y_speed;
  }

  play_effectwfvxy_xs_ys(WAV_WHOOSH2, SPRIORITY_LOW, 800 + grand(200), 150, BL.x, BL.y, EE.x_speed, EE.y_speed);


 }


 range = EE.turret_target_range [t];

 if (EE.turret_target [t] != TARGET_NONE)
 {
  int x = get_target_x(a, EE.turret_target [t], EE.x);
  int y = get_target_y(a, EE.turret_target [t], EE.y);

  int dist;
  if (abs(EE.y - y) < range && abs(EE.x - x) < range) // do a quick and rough test first before we go into hypot:
  {
   dist = hypot(EE.y - y, EE.x - x);
   if (dist > range)
    EE.turret_target [t] = TARGET_NONE;
  }
   else
    EE.turret_target [t] = TARGET_NONE;
 }


//   EE.turret_recoil [t] = 8;

 EE.turret_recycle [t] = EE.turret_recycle_time [t];
 EE.turret_energy [t] -= EE.turret_energy_use [t];

 break;


 case TURRET_EBEAM:
 b = create_bullet(BULLET_EBEAM1, a);
 if (b != -1)
 {
  BL.x = EE.turret_x [t] + xpart(angle, 9000);
  BL.y = EE.turret_y [t] + ypart(angle, 9000);
  BL.angle = angle;
  BL.x_speed = 0;
  BL.y_speed = 0;
//  BL.x2 = EE.x_speed;
//  BL.y2 = EE.y_speed;
  BL.timeout = 2;
  BL.colour = 2;
  if (a == TEAM_FRIEND)
   BL.colour = 0;
  BL.owner = e;
  BL.owner_t = t;
  BL.draw_size = 4;
  BL.status2 = 5; // pixels between checks
  BL.size = EE.turret_firing [t];
  BL.status = 150 - BL.size; // length of beam, in units of (status2 * pixels)
  BL.status3 = BEAM_HIT_NOTHING;
  if (EE.turret_firing [t] > 20)
  {
   BL.size = 20;
  }
  if (EE.turret_firing [t] > 50)
  {
   BL.size = 70 - EE.turret_firing [t];
  }
  BL.damage = BL.size * 20;
//  play_effectwfvx(WAV_FIRE, 500 + grand(1000), 200, bullet[a][b].x);
//  EE.x_speed -= xpart(angle, 400);
  //EE.y_speed -= ypart(angle, 400); // a little bit of recoil
  play_effectwfvxy_xs_ys(WAV_BEAM, SPRIORITY_HIGH, 2000, 220, BL.x, BL.y, EE.x_speed, EE.y_speed);
 }

 range = EE.turret_firing_range [t];

 if (EE.turret_target [t] != TARGET_NONE)
 {
  int x = get_target_x(a, EE.turret_target [t], EE.x);
  int y = get_target_y(a, EE.turret_target [t], EE.y);

  int dist;
  if (abs(EE.y - y) < range && abs(EE.x - x) < range) // do a quick and rough test first before we go into hypot:
  {
   dist = hypot(EE.y - y, EE.x - x);
   if (dist > range)
    EE.turret_target [t] = TARGET_NONE;
  }
   else
    EE.turret_target [t] = TARGET_NONE;
 }

 EE.turret_recoil [t] = 20;

// deals with energy and recycle in run_turrets
 break;


 }

}

enum
{
FORM_BASIC,
FORMS

};

#define FORM_SIZE 25


int formations [FORMS] [FORM_SIZE] [2] =
{
 {
  {0, 0}, // leader
  {ANGLE_4, 40000},
  {-ANGLE_4, 40000},
  {ANGLE_4 + ANGLE_32, 80000},
  {-ANGLE_4 - ANGLE_32, 80000},
  {ANGLE_4 + ANGLE_16, 120000},
  {-ANGLE_4 - ANGLE_16, 120000},
  {ANGLE_4 + ANGLE_16, 160000},
  {-ANGLE_4 - ANGLE_16, 160000},
  {ANGLE_4 + ANGLE_16, 200000},
  {-ANGLE_4 - ANGLE_16, 200000},
  {ANGLE_4 + ANGLE_16, 240000},
  {-ANGLE_4 - ANGLE_16, 240000},
  {ANGLE_4 + ANGLE_16, 280000},
  {-ANGLE_4 - ANGLE_16, 280000},
  {ANGLE_4 + ANGLE_16, 320000},
  {-ANGLE_4 - ANGLE_16, 320000},
  {ANGLE_4 + ANGLE_16, 360000},
  {-ANGLE_4 - ANGLE_16, 360000},

 } // FORM_BASIC

};


void formation_position(int a, int e)
{
 int pos = EE.formation_position;

 EE.formation_x = ship[a][EE.leader].x + xpart(ship[a][EE.leader].angle + formations [FORM_BASIC] [pos] [0], formations [FORM_BASIC] [pos] [1]);
 EE.formation_y = ship[a][EE.leader].y + ypart(ship[a][EE.leader].angle + formations [FORM_BASIC] [pos] [0], formations [FORM_BASIC] [pos] [1]);

}

void convoy_position(int a, int e)
{
 EE.convoy_x = convoy[EE.convoy].x + xpart(EE.convoy_angle [convoy[EE.convoy].arrangement] + convoy[EE.convoy].angle, EE.convoy_dist [convoy[EE.convoy].arrangement]);
 EE.convoy_y = convoy[EE.convoy].y + ypart(EE.convoy_angle [convoy[EE.convoy].arrangement] + convoy[EE.convoy].angle, EE.convoy_dist [convoy[EE.convoy].arrangement]);
}

void wing_formation_position(int a, int e, int p)
{

 if (p == -1)
  return;

 int pos = EE.formation_position;

 EE.formation_x = PP.x + xpart(PP.angle + formations [FORM_BASIC] [pos] [0], formations [FORM_BASIC] [pos] [1]);
 EE.formation_y = PP.y + ypart(PP.angle + formations [FORM_BASIC] [pos] [0], formations [FORM_BASIC] [pos] [1]);

}


int get_target_x(int a, int target, int x)
{

  if (target >= 0)
   return ship[a^1][target].x;

  if (target == TARGET_P1)
   return player[0].x;

  if (target == TARGET_P2)
   return player[1].x;

// TARGET_NONE
  return x;

}

int get_target_y(int a, int target, int y)
{

  if (target >= 0)
   return ship[a^1][target].y;

  if (target == TARGET_P1)
   return player[0].y;

  if (target == TARGET_P2)
   return player[1].y;

// TARGET_NONE
  return y;

}


// returns 0 if all damage absorbed by shield
// returns 1 if some or all damage taken by hull
//  - may not be totally precise for beams, but this shouldn't matter
int hurt_ship_fighter(int ea, int e, int dam, int ba, int b, int beam, int bullet_x, int bullet_y)
{

 struct shipstruct* ee;

 ee = &ship[ea][e];
 ee->just_hit = 1;

 if (ee->shield_up)
 {
     int dam2;
     dam2 = dam - ee->shield;

     ee->shield -= dam;
     if (ee->shield <= 0)
     {
      ee->shield = 0;
      ee->shield_up = 0;

     int i, c;
     int angle = grand(ANGLE_1);
     int dist;
     int speed;

     for (i = 0; i < 8; i ++)
     {
//       angle = grand(ANGLE_1);
       angle += ANGLE_8 + grand(ANGLE_16);
       dist = (eclass[ship[ea][e].type].size) + grand(eclass[ship[ea][e].type].size>>1);
//       dist = grand(eclass[ship[ea][e].type].size);
       speed = dist>>2;
       c = quick_cloud(CLOUD_EX_SHIELD,
            ship[ea][e].x + xpart(angle, dist),
            ship[ea][e].y + ypart(angle, dist),
            ship[ea][e].x_speed + xpart(angle, speed),
            ship[ea][e].y_speed + ypart(angle, speed),
            8 + grand(5), 2, 0);
       if (c != -1)
       {
        CC.status = 3 + grand(3);
        CC.angle = angle;
        CC.drag = 980;
       }
     }
     quick_cloud(CLOUD_SMALL_SHOCK, bullet[ba][b].x, bullet[ba][b].y, 0, 0, SMALL_SHOCK_TIME, 2, 0);


     }

     int pl;

     int dist = eclass[ee->type].size;//xyedist(bullet[ba][b].x, bullet[ba][b].y, ea, e) >> 10;
     int angle = radians_to_angle(atan2(bullet_y - ee->y, bullet_x - ee->x));
     int angle_diff = angle - ee->angle - ANGLE_4;


           ee->shield_bar_pulse = 5;
           if (beam)
           {
// first check for an spulse reserved for this beam:
            for (pl = 0; pl < PULSE; pl ++)
            {
             if (ee->spulse_time [pl] >= 0
              && ee->spulse_beam_e [pl] == bullet[ba][b].owner
              && ee->spulse_beam_t [pl] == bullet[ba][b].owner_t)
             {
              ee->spulse_angle [pl] = angle_diff;
              ee->spulse_dist [pl] = dist >> 10;
              ee->spulse_time [pl] = 70 + grand(50);
              ee->spulse_time_delta [pl] = 10;
              ee->spulse_beam_e [pl] = bullet[ba][b].owner;
              ee->spulse_beam_t [pl] = bullet[ba][b].owner_t;
              if (ee->shield_up == 0) // must have been knocked out by this hit:
               play_effectwfvxy_xs_ys(WAV_SHIELD, SPRIORITY_LOW, 800, 90, ee->x, ee->y, ee->x_speed, ee->y_speed);
//              if (ee->spulse_size [pl] < ee->spulse_time [pl] * ee->spulse_size2 [pl])
//               ee->spulse_size [pl] = ee->spulse_time [pl] * ee->spulse_size2 [pl];
              return 0;
             }
            }
// if none found, reserve a new one:
            for (pl = 0; pl < PULSE; pl ++)
            {
             if (ee->spulse_time [pl] <= 0 || pl == PULSE - 1)
             {
              ee->spulse_size [pl] = 3;// + grand(4);
              ee->spulse_size2 [pl] = 1;// + grand(3);
              ee->spulse_angle [pl] = angle_diff;
              ee->spulse_dist [pl] = dist >> 10;
              ee->spulse_time [pl] = 100;// + grand(12);
              ee->spulse_time_delta [pl] = 10;
              ee->spulse_beam_e [pl] = bullet[ba][b].owner;
              ee->spulse_beam_t [pl] = bullet[ba][b].owner_t;
              if (ee->shield_up == 0) // must have been knocked out by this hit:
               play_effectwfvxy_xs_ys(WAV_SHIELD, SPRIORITY_LOW, 800, 90, ee->x, ee->y, ee->x_speed, ee->y_speed);

//              if (ee->spulse_size [pl] < ee->spulse_time [pl] * ee->spulse_size2 [pl])
//               ee->spulse_size [pl] = ee->spulse_time [pl] * ee->spulse_size2 [pl];
              return 0;
             }
            }
           }
              else // not a beam
              {
                for (pl = 0; pl < PULSE; pl ++)
                {
                 if (ee->spulse_time [pl] <= 0 || pl == PULSE - 1)
                 {
                  ee->spulse_size [pl] = 3;// + grand(4);
                  ee->spulse_size2 [pl] = 1;// + grand(3);
                  ee->spulse_angle [pl] = angle_diff;
                  ee->spulse_dist [pl] = dist >> 10;
                  ee->spulse_time [pl] = 120;// + grand(12);
                  ee->spulse_time_delta [pl] = 10;
                  if (ee->shield_up == 0) // must have been knocked out by this hit:
                   play_effectwfvxy_xs_ys(WAV_SHIELD, SPRIORITY_LOW, 800, 90, bullet_x, bullet_y, ee->x_speed, ee->y_speed);
                    else
                     play_effectwfvxy_xs_ys(WAV_SHIELD, SPRIORITY_LOW, 1500, 50, bullet_x, bullet_y, ee->x_speed, ee->y_speed);

//       if (ee->spulse_size [pl] < ee->spulse_time [pl] * ee->spulse_size2 [pl])
//        ee->spulse_size [pl] = ee->spulse_time [pl] * ee->spulse_size2 [pl];
                  break;
                 }
                }

              }
  dam = dam2;

//  if (dam <= 0)
   return 0; // all damage absorbed by shield
// Damage used to always flow through for fighters. Now it doesn't. Maybe this will be changed so that
//  really powerful hits (eg warship guns) flow through?

 }

// may have returned by now, especially if it's a beam

 ee->hp [0] -= dam;

 if (ee->hp [0] <= 0)
 {
  record_ship_destroyed(ea, e, bullet[ba][b].owner);
  ship_explodes(ea, e);
  return 2; // ship destroyed
 }

 ee->hit_pulse [0] = 5;
 ee->shield_bar_pulse = 5;

 return 1; // damage to hull

}

void ship_explodes(int a, int e)
{
// int c;
// int x = EE.x;
// int y = EE.y;

 if (eclass[EE.type].ship_class == ECLASS_WSHIP)
 {
  wship_starts_breaking_up(a, e);
  return;
 }

 switch(EE.type)
 {
  case SHIP_FIGHTER:
  case SHIP_ESCOUT:
  case SHIP_EINT:
  play_effectwfvxy_xs_ys(WAV_BASIC, SPRIORITY_LOW, 900 + grand(200), 220, EE.x, EE.y, EE.x_speed, EE.y_speed);
   fighter_explosion(a, e, 3, 10, 4000,
                        5, 25, 15000);
                        break;

  case SHIP_BOMBER:
  case SHIP_FIGHTER_FRIEND:
  case SHIP_FSTRIKE:
  case SHIP_LACEWING:
  case SHIP_IBEX:
  case SHIP_MONARCH:
  case SHIP_AUROCHS:
  play_effectwfvxy_xs_ys(WAV_BASIC, SPRIORITY_LOW, 600 + grand(200), 220, EE.x, EE.y, EE.x_speed, EE.y_speed);
   fighter_explosion(a, e, 5, 15, 5000,
                        7, 30, 19000);
                        break;
//   quick_cloud(CLOUD_BALL, x, y, 0, 0, 50 + grand(20), 0, 0);
//   quick_cloud(CLOUD_LARGE_SHOCK, x, y, 0, 0, LARGE_SHOCK_TIME, 0, 0);
//   break;


 }

// only fighters get this far (wships call these functions in their own function)

 if (a == TEAM_ENEMY)
  condition_eship_destroyed(e); // in level.c
   else
    condition_fship_destroyed(e); // in level.c

 destroy_ship(a, e);

}

void fighter_explosion(int a, int e, int flare1_flares, int flare1_size, int flare1_speed,
                        int fadeballs, int fadeballs_size, int fadeballs_dist)
{

 int x = EE.x;
 int y = EE.y;
 int c, i, dist;
 int col = a;

 quick_cloud(CLOUD_LARGE_SHOCK, x, y, 0, 0, LARGE_SHOCK_TIME, col, 0);

 int angle = grand(ANGLE_1);

 for (i = 0; i < flare1_flares; i ++)
 {
    c = simple_cloud(CLOUD_SYNCHFLARE, x, y, col, flare1_size + grand(5));
    if (c != -1)
    {
     cloud[c].x_speed = EE.x_speed + xpart(angle, flare1_speed + grand(flare1_speed));
     cloud[c].y_speed = EE.y_speed + ypart(angle, flare1_speed + grand(flare1_speed));
     CC.x2 = EE.x_speed;
     CC.y2 = EE.y_speed;
     cloud[c].drag = 940;
     CC.x3 = 1;
    }
    angle += ANGLE_8 + grand(ANGLE_3);
 }
/*
 c = simple_cloud(CLOUD_BIGFADEFLARE, x, y, 0, 60 + grand(20));
 if (c != -1)
 {
  cloud[c].x_speed = 0;//EE.x_speed;
  cloud[c].y_speed = 0;//EE.y_speed;
 }
*/

 dist = 0;

 for (i = 0; i < fadeballs; i ++)
 {

   quick_fire(x + xpart(angle, dist),
             y + ypart(angle, dist),
             0, 0, fadeballs_size + grand(10), col);

/*        c = quick_cloud(CLOUD_FADEBALL,
             x + xpart(angle, dist),
             y + ypart(angle, dist),
             0, 0, fadeballs_size + grand(10), col, 0);
        if (c != -1)
        {
         CC.status = 10 + grand(8);
        }*/

 angle = grand(ANGLE_1);
 dist = fadeballs_dist + grand(fadeballs_dist);


 }


}


void wship_starts_breaking_up(int a, int e)
{

// int x = EE.x;
// int y = EE.y;
 int t;

 EE.jump = 0;

 if (EE.breakup > 0)
  return;

 for (t = 0; t < eclass[EE.type].parts; t ++)
 {
   if (EE.hp [t] > 0)
   {
       EE.hp [t] = 0;
       wship_part_explodes(a, e, t);
   }
 }

 play_effectwfvxy_xs_ys(WAV_BANG3, SPRIORITY_LOW, 1000, 250, EE.x, EE.y, EE.x_speed, EE.y_speed);

 EE.breakup = 5 + grand (50) + grand(50);
 EE.breakup_turn = grand(8);
 if (coin())
  EE.breakup_turn *= -1;




// destroy_ship(a, e);
 // not yet...

}

int wship_breakup(int a, int e)
{

 EE.breakup --;
 EE.jump = 0;

 if (EE.breakup <= 0)
 {
  wship_final_explosion(a, e);
  return 1;
 }

 if ((EE.breakup >> 3) & 1)
 {
  int c, angle, dist;
  int col = EE.drive_colour;

  angle = grand(ANGLE_1);
  dist = grand(eclass[EE.type].size);

  quick_fire(EE.x + xpart(angle, dist),
             EE.y + ypart(angle, dist),
             EE.x_speed, EE.y_speed, 5 + grand(25), col);
/*
          c = quick_cloud(CLOUD_FADEBALL,
             EE.x + xpart(angle, dist),
             EE.y + ypart(angle, dist),
             EE.x_speed, EE.y_speed, 5 + grand(25), col, 0);
        if (c != -1)
        {
         CC.status = 10 + grand(13);
        }
*/

 }

 EE.angle += EE.breakup_turn;

 EE.x += EE.x_speed;
 EE.y += EE.y_speed;

 EE.x_speed += xpart(EE.angle, EE.wship_throttle);
 EE.y_speed += ypart(EE.angle, EE.wship_throttle);

 drag_ship(a, e, 1023-eclass[EE.type].drag);

/* if (EE.breakup_turn > 0)
 {
  EE.breakup_turn += grand(1);
 }
  else
   EE.breakup_turn -= grand(1);*/

// EE.speed += grand(50);

// EE.x_speed = xpart(EE.angle, EE.speed);
// EE.y_speed = ypart(EE.angle, EE.speed);

 return 0;
}

// wship_explodes
void wship_final_explosion(int a, int e)
{

 int x = EE.x;
 int y = EE.y;
 int c, i;
 int col = EE.drive_colour;

 quick_cloud(CLOUD_HUGE_SHOCK, x, y, 0, 0, HUGE_SHOCK_TIME, col, 0);

 int flares = 4;
 int flare_speed = 7000;
 int flare_size = 30;

 int angle = grand(ANGLE_1);

 switch(EE.type)
 {
  case SHIP_FRIEND3:
  case SHIP_SCOUT3:
   flares = 7;
   flare_speed = 11000;
   flare_size = 50;
   play_effectwfvxy_xs_ys(WAV_BANG4, SPRIORITY_LOW, 600, 250, EE.x, EE.y, EE.x_speed, EE.y_speed);
   quick_fire(x + xpart(EE.angle, 70000),
             y + ypart(EE.angle, 70000),
             0, 0, 60 + grand(40), col);
   quick_fire(x - xpart(EE.angle, 70000),
             y - ypart(EE.angle, 70000),
             0, 0, 60 + grand(40), col);
   quick_fire(
             x + xpart(EE.angle+ANGLE_4, 40000),
             y + ypart(EE.angle+ANGLE_4, 40000),
             0, 0, 60 + grand(40), col);
   quick_fire(
             x + xpart(EE.angle-ANGLE_4, 40000),
             y + ypart(EE.angle-ANGLE_4, 40000),
             0, 0, 60 + grand(40), col);
              /*
        c = quick_cloud(CLOUD_FADEBALL,
             x + xpart(EE.angle, 70000),
             y + ypart(EE.angle, 70000),
             0, 0, 60 + grand(40), col, 0);
        if (c != -1)
        {
         CC.status = 25 + grand(13);
        }
        c = quick_cloud(CLOUD_FADEBALL,
             x - xpart(EE.angle, 70000),
             y - ypart(EE.angle, 70000),
             0, 0, 60 + grand(40), col, 0);
        if (c != -1)
        {
         CC.status = 25 + grand(13);
        }
        c = quick_cloud(CLOUD_FADEBALL,
             x + xpart(EE.angle+ANGLE_4, 40000),
             y + ypart(EE.angle+ANGLE_4, 40000),
             0, 0, 60 + grand(40), col, 0);
        if (c != -1)
        {
         CC.status = 25 + grand(13);
        }
        c = quick_cloud(CLOUD_FADEBALL,
             x + xpart(EE.angle-ANGLE_4, 40000),
             y + ypart(EE.angle-ANGLE_4, 40000),
             0, 0, 60 + grand(40), col, 0);
        if (c != -1)
        {
         CC.status = 25 + grand(13);
        }*/
   break;
  case SHIP_EBASE:
   flares = 11;
   flare_speed = 11000;
   flare_size = 50;
   big_explode(x + xpart(EE.angle, 80000), y + ypart(EE.angle, 80000), 0,0, 3, 30, 8000, 1);
   big_explode(x + xpart(EE.angle + ANGLE_2, 80000), y + ypart(EE.angle + ANGLE_2, 80000), 0,0, 3, 30, 8000, 1);
   play_effectwfvxy_xs_ys(WAV_BANG4, SPRIORITY_HIGH, 500, 250, EE.x, EE.y, EE.x_speed, EE.y_speed);
   break;
  default:
   play_effectwfvxy_xs_ys(WAV_BANG4, SPRIORITY_LOW, 700, 250, EE.x, EE.y, EE.x_speed, EE.y_speed);
   break;
 }

 for (i = 0; i < flares; i ++)
 {
    c = simple_cloud(CLOUD_SYNCHFLARE, x, y, col, flare_size + grand(5));
    if (c != -1)
    {
     cloud[c].x_speed = EE.x_speed + xpart(angle, flare_speed + grand(flare_speed));
     cloud[c].y_speed = EE.y_speed + ypart(angle, flare_speed + grand(flare_speed));
     CC.x2 = EE.x_speed;
     CC.y2 = EE.y_speed;
     cloud[c].drag = 940;
     CC.x3 = 2;

    }
    angle += ANGLE_8 + grand(ANGLE_3);
 }



 c = simple_cloud(CLOUD_BIGFADEFLARE, x, y, col, 60 + grand(20));
 if (c != -1)
 {
  cloud[c].x_speed = 0;//EE.x_speed;
  cloud[c].y_speed = 0;//EE.y_speed;
 }

 quick_fire(x,
             y,
             0, 0, 80 + grand(40), col);
 quick_fire(x + xpart(EE.angle, 30000),
             y + ypart(EE.angle, 30000),
             0, 0, 60 + grand(40), col);
             quick_fire(
             x - xpart(EE.angle, 30000),
             y - ypart(EE.angle, 30000),
             0, 0, 60 + grand(40), col);
/*
        c = quick_cloud(CLOUD_FADEBALL,
             x,
             y,
             0, 0, 80 + grand(40), col, 0);
        if (c != -1)
        {
         CC.status = 30 + grand(13);
        }


        c = quick_cloud(CLOUD_FADEBALL,
             x + xpart(EE.angle, 30000),
             y + ypart(EE.angle, 30000),
             0, 0, 60 + grand(40), col, 0);
        if (c != -1)
        {
         CC.status = 25 + grand(13);
        }

        c = quick_cloud(CLOUD_FADEBALL,
             x - xpart(EE.angle, 30000),
             y - ypart(EE.angle, 30000),
             0, 0, 60 + grand(40), col, 0);
        if (c != -1)
        {
         CC.status = 25 + grand(13);
        }
*/

/* for (i = 0; i < eclass[EE.type].parts; i ++)
 {
        c = quick_cloud(CLOUD_FADEBALL,
             x + rotate_pos_x [eclass[EE.type].base_turret_rp + i] [EE.angle],
             y + rotate_pos_y [eclass[EE.type].base_turret_rp + i] [EE.angle],
             0, 0, 60 + grand(40), 0, 0);
        if (c != -1)
        {
         CC.status = 25 + grand(13);
        }


 }*/

 if (a == TEAM_ENEMY)
  condition_eship_destroyed(e); // in level.c
   else
    condition_fship_destroyed(e); // in level.c

 destroy_ship(a, e);

}


void big_explode(int x, int y, int x_speed, int y_speed, int flares, int flare_size, int flare_speed, int col)
{

 int i;
 int angle = grand(ANGLE_1);
 int c;

 for (i = 0; i < flares; i ++)
 {
    c = simple_cloud(CLOUD_SYNCHFLARE, x, y, col, flare_size + grand(5));
    if (c != -1)
    {
     cloud[c].x_speed = x_speed + xpart(angle, flare_speed + grand(flare_speed));
     cloud[c].y_speed = y_speed + ypart(angle, flare_speed + grand(flare_speed));
     CC.x2 = x_speed;
     CC.y2 = y_speed;
     cloud[c].drag = 940;
     CC.x3 = 3;

    }
    angle += ANGLE_8 + grand(ANGLE_3);
 }



 c = simple_cloud(CLOUD_BIGFADEFLARE, x, y, col, 60 + grand(20));
 if (c != -1)
 {
  cloud[c].x_speed = 0;//EE.x_speed;
  cloud[c].y_speed = 0;//EE.y_speed;
 }


 quick_fire(x,
             y,
             0, 0, 80 + grand(40), col);

/*        c = quick_cloud(CLOUD_FADEBALL,
             x,
             y,
             0, 0, 80 + grand(40), col, 0);
        if (c != -1)
        {
         CC.status = 30 + grand(13);
        }
*/

}


int wship_jumping_out(int a, int e)
{

  EE.jump --;

  if (EE.jump > 15)
   return 0; // still charging up

  EE.turning = 0;
//  EE.turn_count = 0;

  if (EE.jump == 14)
  {

      int t, x, y, k;
      EE.angle &= ANGLE_MASK;

      for (t = 0; t < eclass[EE.type].engines; t ++)
      {
       k = t + 1;
       x = EE.x + rotate_pos_x [eclass[EE.type].base_turret_rp + eclass[EE.type].turrets + t] [EE.angle];
       y = EE.y + rotate_pos_y [eclass[EE.type].base_turret_rp + eclass[EE.type].turrets + t] [EE.angle];
// the indices in the rotate_pos arrays have turrets added on because engines come after turrets
//  in those arrays.
       quick_cloud(CLOUD_LARGE_SHOCK, x, y, 0, 0, LARGE_SHOCK_TIME, 2, 0);
      }

  }

  EE.x_speed += xpart(EE.angle, 300);
  EE.y_speed += ypart(EE.angle, 300);

  if (EE.jump == 0)
  {
   wship_jump_out(a, e);
   return 1;
  }

  return 0;

}

void wship_jump_out(int a, int e)
{

// if (a == TEAM_ENEMY)
//  condition_eship_destroyed(e); // in level.c

 if (arena.jump_countdown > -1
  && arena.game_over == 0
  && a == TEAM_FRIEND)
 {
  int p;
  for (p = 0; p < 2; p++)
  {
   if (arena.only_player != -1
    && arena.only_player != p)
     continue;

   if (check_player_jump_safe(p, e))
    arena.jumped_out = 1;
  }
 }

// see also game.c in check_player_jump_safe:
#define SJUMP_PICKUP_RANGE 200000
#define SJUMP_PICKUP_SAFE 100000

// now check to see if the wship takes any other fighters with it:
 int e2;

 for (e2 = 0; e2 < NO_SHIPS; e2++)
 {
  if (ship[a][e2].type == SHIP_NONE
   || eclass[ship[a][e2].type].ship_class != ECLASS_FIGHTER)
   continue;

   if (abs(EE.y - ship[a][e2].y) > SJUMP_PICKUP_RANGE || abs(EE.x - ship[a][e2].x) > SJUMP_PICKUP_RANGE)
    continue;

   if (hypot(EE.y - ship[a][e2].y, EE.x - ship[a][e2].x) > SJUMP_PICKUP_RANGE)
    continue;

  fighter_jump_out(a, e2);

 }

 jump_clouds(a, e);

 destroy_ship(a, e);

// if it was friendly, and countdown is on, and there are no other ships left to pick the player up, the player has missed the jump:
 if (arena.jump_countdown > -1
  && arena.game_over == 0
  && a == TEAM_FRIEND)
 {
  char anyone_left = 0;

  for (e2 = 0; e2 < NO_SHIPS; e2++)
  {
   if (ship[a][e2].type != SHIP_NONE
    && eclass[ship[a][e2].type].ship_class == ECLASS_WSHIP)
     anyone_left = 1;
  }

 if (anyone_left == 0
  && arena.game_over == 0)
  {
   arena.jump_countdown = -1;
   arena.missed_jump = 200;
  }

 }
// the situation where all friendly ships have jumped out except one, and the player is near that one, and it's destroyed before it can jump,
//  is handled by scripts - if there is a ccon for this it will set arena.all_wships_lost (which counts as game over)


}

void fighter_jump_out(int a, int e)
{

 jump_clouds(a, e);

 destroy_ship(a, e);

}

void jump_clouds(int a, int e)
{

 if (!nearby(EE.x, EE.y))
  return;

 int x = EE.x;
 int y = EE.y;
 int c;
 int col = 2;

 if (eclass[EE.type].ship_class == ECLASS_FIGHTER)
 {

 quick_cloud(CLOUD_SMALL_SHOCK, x, y, 0, 0, HUGE_SHOCK_TIME, col, 0);



        c = quick_cloud(CLOUD_FADEBALL,
             x,
             y,
             0, 0, 30 + grand(20), col, 0);
        if (c != -1)
        {
         CC.status = 10 + grand(8);
        }

       return;

 }

 quick_cloud(CLOUD_HUGE_SHOCK, x, y, 0, 0, HUGE_SHOCK_TIME, col, 0);



 c = simple_cloud(CLOUD_BIGFADEFLARE, x, y, col, 60 + grand(20));
 if (c != -1)
 {
  cloud[c].x_speed = 0;
  cloud[c].y_speed = 0;
 }

        c = quick_cloud(CLOUD_FADEBALL,
             x,
             y,
             0, 0, 80 + grand(40), col, 0);
        if (c != -1)
        {
         CC.status = 30 + grand(13);
        }


        c = quick_cloud(CLOUD_FADEBALL,
             x + xpart(EE.angle, 30000),
             y + ypart(EE.angle, 30000),
             0, 0, 60 + grand(40), col, 0);
        if (c != -1)
        {
         CC.status = 25 + grand(13);
        }

        c = quick_cloud(CLOUD_FADEBALL,
             x - xpart(EE.angle, 30000),
             y - ypart(EE.angle, 30000),
             0, 0, 60 + grand(40), col, 0);
        if (c != -1)
        {
         CC.status = 25 + grand(13);
        }

// will have already returned if fighter

}


// owner is -1 if not a player
void hurt_wship_shield(int ea, int e, int dam, int owner, int ba, int b)
{

 /*if (ea == TEAM_FRIEND)
  return;*/

    ship[ea][e].shield -= dam;

    if (ship[ea][e].shield <= 0)
    {
     ship[ea][e].shield = 0;
     ship[ea][e].shield_up = 0;

     int i, c;
     int angle = grand(ANGLE_1);
     int dist;
     int speed;

     for (i = 0; i < 20; i ++)
     {
//       angle = grand(ANGLE_1);
       angle += ANGLE_8 + grand(ANGLE_8);
       dist = (eclass[ship[ea][e].type].size>>1) + grand(eclass[ship[ea][e].type].size>>1);
//       dist = grand(eclass[ship[ea][e].type].size);
       speed = dist>>2;
       c = quick_cloud(CLOUD_EX_SHIELD,
            ship[ea][e].x + xpart(angle, dist),
            ship[ea][e].y + ypart(angle, dist),
            ship[ea][e].x_speed + xpart(angle, speed),
            ship[ea][e].y_speed + ypart(angle, speed),
            20 + grand(10), 2, 0);
       if (c != -1)
       {
        CC.status = 6 + grand(2);
        CC.angle = angle;
        CC.drag = 940;
       }
     }
     quick_cloud(CLOUD_LARGE_SHOCK, bullet[ba][b].x, bullet[ba][b].y, 0, 0, LARGE_SHOCK_TIME, 2, 0);
    }

//    if (EE.shield <= 0)

}

// owner is -1 if not a player
void hurt_wship_part(int ea, int e, int dam, int owner, int ba, int b, int part)
{

    if (ship[ea][e].hp [part] > 0)
    {
      if (dam >= ship[ea][e].hp [part])
      {
       dam -= ship[ea][e].hp [part];
       ship[ea][e].hp [part] = 0;
       wship_part_explodes(ea, e, part);
       play_effectwfvxy_xs_ys(WAV_BANG3, SPRIORITY_LOW, 1000, 250, ship[ea][e].x, ship[ea][e].y, ship[ea][e].x_speed, ship[ea][e].y_speed);
       if (dam <= 0)
        return;
      }
       else
       {
        ship[ea][e].hp [part] -= dam;
        ship[ea][e].hit_pulse [part] = 5;
        return;
       }
// the damage has gone through to internal structure:
    }

        ship[ea][e].structure -= dam;
        ship[ea][e].hit_pulse_structure = 5;
        if (ship[ea][e].structure <= 0)
        {
         if (ship[ea][e].breakup <= 0)
          record_ship_destroyed(ea, e, bullet[ba][b].owner);
         ship_explodes(ea, e);
        }


}

void record_ship_destroyed(int a, int e, int owner)
{
 arena.srecord [SREC_DESTROYED] [a] [EE.type] ++;
 if (owner == -1)
  arena.srecord [SREC_P1_KILL] [a] [EE.type] ++;
 if (owner == -2)
  arena.srecord [SREC_P2_KILL] [a] [EE.type] ++;
}


void wship_part_explodes(int a, int e, int part)
{
 int c, x = 0, y = 0, angle, i;

 int dcol = a;

 if (part == eclass[EE.type].generator_part)
 {
  EE.shield_generator = 0;
 }

 if (EE.turret_type [part] == TURRET_NONE) // e.g. carrier's or freighter's front part
 {
  switch(EE.type)
  {
   case SHIP_ECARRIER:
    x = EE.x + xpart(EE.angle, 12000);
    y = EE.y + ypart(EE.angle, 12000);
    EE.can_launch = 0;
    if (a == TEAM_ENEMY)
     condition_ecarrier_disabled(e);
    break;
   case SHIP_FREIGHT:
    x = EE.x + xpart(EE.angle, 12000);
    y = EE.y + ypart(EE.angle, 12000);
    break;
  }
  // these return later in this function
 }
   else
   {
     x = EE.turret_x [part];
     y = EE.turret_y [part];
   }



 if (nearby(x, y))
 {
  quick_cloud(CLOUD_LARGE_SHOCK, x, y, 0, 0, LARGE_SHOCK_TIME, dcol, 0);

  angle = grand(ANGLE_1);

  for (i = 0; i < 4; i ++)
  {
    c = simple_cloud(CLOUD_SYNCHFLARE, x, y, dcol, 20 + grand(5));
    if (c != -1)
    {
     cloud[c].x_speed = EE.x_speed + xpart(angle, 5000 + grand(7000));
     cloud[c].y_speed = EE.y_speed + ypart(angle, 5000 + grand(7000));
     CC.x2 = EE.x_speed;
     CC.y2 = EE.y_speed;
     cloud[c].drag = 900;
     CC.x3 = 4;
    }
    angle += ANGLE_8 + grand(ANGLE_3);
  }

  c = simple_cloud(CLOUD_FLARE, x, y, dcol, 30 + grand(20));
  if (c != -1)
  {
   cloud[c].x_speed = EE.x_speed;
   cloud[c].y_speed = EE.y_speed;
  }
 }

/*
 for (i = 0; i < 4; i ++)
 {
    c = simple_cloud(CLOUD_SYNCHFLARE, x, y, 0, 15 + grand(5));
    if (c != -1)
    {
     cloud[c].x_speed = EE.x_speed + xpart(angle, 3000 + grand(5000));
     cloud[c].y_speed = EE.y_speed + ypart(angle, 3000 + grand(5000));
     CC.x2 = EE.x_speed;
     CC.y2 = EE.y_speed;
     cloud[c].drag = 880;
    }
    angle += ANGLE_8 + grand(ANGLE_3);
 }
*/

 if (EE.turret_type [part] == TURRET_NONE) // e.g. carrier
 {
  calculate_threat();
  return;
 }

 EE.turret_type [part] = TURRET_NONE;

 calculate_threat();


// Losing that turret may have left the wship without any anti-fighter or anti-wship weapons.
// If so, it should be willing to e.g. use an anti-fighter weapon on a wship if no fighter's in range.

 char anti_fighter = 0;
 char anti_wship = 0;

 for (i = 0; i < eclass[EE.type].parts; i ++)
 {
  if (EE.turret_type [i] == TURRET_NONE)
   continue;
  if (EE.turret_class [i] == ATTACK_ONLY_WSHIP
   || EE.turret_class [i] == ATTACK_PREF_WSHIP
   || EE.turret_class [i] == ATTACK_ANY)
   {
    anti_wship ++;
   }
  if (EE.turret_class [i] == ATTACK_ONLY_FIGHTER
   || EE.turret_class [i] == ATTACK_PREF_FIGHTER
   || EE.turret_class [i] == ATTACK_ANY)
   {
    anti_fighter ++;
   }
 }

// no guns willing to target wships. Change ATTACK_ONLY_FIGHTER guns to ATTACK_PREF_FIGHTER so
//  that they'll attack wships if no fighter in range.
 if (anti_wship == 0)
 {
  for (i = 0; i < eclass[EE.type].parts; i ++)
  {
   if (EE.turret_type [i] == TURRET_NONE)
    continue;
   if (EE.turret_class [i] == ATTACK_ONLY_FIGHTER)
    EE.turret_class [i] = ATTACK_PREF_FIGHTER;
  }
 }

// now the other way...
 if (anti_fighter == 0)
 {
  for (i = 0; i < eclass[EE.type].parts; i ++)
  {
   if (EE.turret_type [i] == TURRET_NONE)
    continue;
   if (EE.turret_class [i] == ATTACK_ONLY_WSHIP)
    EE.turret_class [i] = ATTACK_PREF_WSHIP;
  }
 }

// may have returned before now if e.g. a carrier

}


int turret_track_target(int a, int e, int t, int target, int forbid)
{
 int x = EE.x;
 int y = EE.y;

 if (target >= 0)
 {
  x = ship[a^1][target].x;
  y = ship[a^1][target].y;
 }

 if (target == TARGET_P1)
 {
  x = player[0].x;
  y = player[0].y;
 }
 if (target == TARGET_P2)
 {
  x = player[1].x;
  y = player[1].y;
 }

  EE.turret_target_angle [t] =
    radians_to_angle(atan2((y - EE.turret_y [t]), (x - EE.turret_x [t])));
   if (EE.turret_target_angle [t]  < 0)
    EE.turret_target_angle [t]  += ANGLE_1;
   if (EE.turret_target_angle [t]  > ANGLE_1)
    EE.turret_target_angle [t]  -= ANGLE_1;

 int retval = turn_towards_angle_forbid(EE.turret_angle [t], EE.turret_target_angle [t], EE.turret_turn [t], EE.turret_slew [t]);

 if (retval < EE.turret_angle [t] || (EE.turret_angle [t] < ANGLE_8 && retval > (ANGLE_1 - ANGLE_8)))
  EE.turret_slew [t] = -1;
   else
    EE.turret_slew [t] = 1;

  return retval;

}

// this is a crude and inaccurate leading function, but probably enough for now.
int turret_track_target_lead(int a, int e, int t, int target, int forbid, int b_speed)
{

 if (target == TARGET_NONE)
  return EE.turret_angle [t];

 int x = EE.x;
 int y = EE.y;
 int xs = 0, ys = 0;

 if (target >= 0)
 {
  x = ship[a^1][target].x;
  y = ship[a^1][target].y;
  xs = ship[a^1][target].x_speed;
  ys = ship[a^1][target].y_speed;
 }
  else
  {
   if (target == TARGET_P1)
   {
    x = player[0].x;
    y = player[0].y;
    xs = player[0].x_speed;
    ys = player[0].y_speed;
   }
     else
     {
      if (target == TARGET_P2)
      {
       x = player[1].x;
       y = player[1].y;
       xs = player[1].x_speed;
       ys = player[1].y_speed;
      }
     }
  }

 int dist = hypot(EE.y - y, EE.x - x);
 int time = dist / b_speed;
 x += time * xs;
 y += time * ys;

  EE.turret_target_angle [t] =
    radians_to_angle(atan2(y - (EE.turret_y [t] + (EE.y_speed*time)), x - (EE.turret_x [t] + (EE.x_speed*time))));
   if (EE.turret_target_angle [t]  < 0)
    EE.turret_target_angle [t]  += ANGLE_1;
   if (EE.turret_target_angle [t]  > ANGLE_1)
    EE.turret_target_angle [t]  -= ANGLE_1;

 int retval = turn_towards_angle_forbid(EE.turret_angle [t], EE.turret_target_angle [t], EE.turret_turn [t], EE.turret_slew [t]);

 if (retval < EE.turret_angle [t] || (EE.turret_angle [t] < ANGLE_8 && retval > (ANGLE_1 - ANGLE_8)))
  EE.turret_slew [t] = -1;
   else
    EE.turret_slew [t] = 1;

  return retval;

}



// atan2 is slow, so don't use it too much.
// see stuff.c for radians_to_angle etc.
int attack_angle(int a, int e, int target)
{
 if (target >= 0)
  return radians_to_angle(atan2(ship[a^1][target].y - EE.y, ship[a^1][target].x - EE.x));
 if (target == TARGET_P1)
  return radians_to_angle(atan2(player[0].y - EE.y, player[0].x - EE.x));

 if (target == TARGET_P2)
  return radians_to_angle(atan2(player[1].y - EE.y, player[1].x - EE.x));

// must be TARGET_NONE
  return EE.angle;
}

int angle_to_formation_position(int a, int e)
{
  return radians_to_angle(atan2(EE.formation_y - EE.y, EE.formation_x - EE.x));
}

int angle_to_convoy_position(int a, int e)
{
  return radians_to_angle(atan2(EE.convoy_y - EE.y, EE.convoy_x - EE.x));
}



int attack_angle_lead(int a, int e, int target, int b_speed)
{

 int x, y, xs, ys;

 if (target >= 0)
 {
  x = ship[a^1][target].x;
  y = ship[a^1][target].y;
  xs = ship[a^1][target].x_speed;
  ys = ship[a^1][target].y_speed;
 }
  else
  {
   if (target == TARGET_P1)
   {
    x = player[0].x;
    y = player[0].y;
    xs = player[0].x_speed;
    ys = player[0].y_speed;
   }
     else
     {
// must be P2:
      x = player[1].x;
      y = player[1].y;
      xs = player[1].x_speed;
      ys = player[1].y_speed;
     }
  }

 int dist = hypot(EE.y - y, EE.x - x);
 int time = dist / b_speed;
 x += time * xs;
 y += time * ys;

 return radians_to_angle(
  atan2(y - (EE.y + EE.y_speed * time), x - (EE.x + EE.x_speed * time)));


}



void destroy_ship(int a, int e)
{

 EE.type = SHIP_NONE;
 if (player[0].target_a == a && player[0].target_e == e)
  player[0].target_a = -1;
 if (player[1].target_a == a && player[1].target_e == e)
  player[1].target_a = -1;

 if (EE.convoy != CONVOY_NONE)
  convoy[EE.convoy].ships --;

 int e2, t;
 int a2 = a ^ 1;

 for (e2 = 0; e2 < NO_SHIPS; e2 ++)
 {
  if (ship[a2][e2].target == e)
  {
   ship[a2][e2].target = TARGET_NONE;
   if (ship[a2][e2].player_command == COMMAND_ATTACK)
   {
    ship[a2][e2].player_command = COMMAND_ENGAGE;
    if (ship[a2][e2].player_leader != -1
     && ship[a2][e2].player_wing != -1) // probably impossible if the ship has a player_command value
    {
     player[ship[a2][e2].player_leader].wing_orders [ship[a2][e2].player_wing] = COMMAND_ENGAGE;
    }
   }
  }
  if (eclass[ship[a2][e2].type].turrets > 0)
  {
   for (t = 0; t < eclass[ship[a2][e2].type].turrets; t ++)
   {
    if (ship[a2][e2].turret_target [t] == e)
      ship[a2][e2].turret_target [t] = TARGET_NONE; // doesn't matter if we clear target for destroyed turret
   }
  }
 }

 if (EE.leader != -1)
  ship[a][EE.leader].formation_size --;

// if the destroyed ship was leading a group (including a wship with guards), the second member of the group
//  becomes the new leader.
// If the new leader is a fighter on ESCORT or GUARD mission, its mission will be changed to SCRAMBLE.
// if (eclass[EE.type].ship_class == ECLASS_FIGHTER)
 {
  if (EE.formation_size > 0) // is a leader
  {
   int new_leader = -1;
   if (EE.formation_size > 1) // has more than 1 follower, so make the next ship the new leader
   {
    int lowest_formation = 1000;
 // let's find the ship which is following the destroyed ship with the lowest formation_position:
    for (e2 = 0; e2 < NO_SHIPS; e2 ++)
    {
     if (ship[a][e2].type == SHIP_NONE)
      continue;
     if (ship[a][e2].leader == e)
     {
      if (ship[a][e2].formation_position < lowest_formation)
      {
       lowest_formation = ship[a][e2].formation_position;
       new_leader = e2;
      }
     }
    }
   }
// now assign all followers of the destroyed ship to the new leader:
   for (e2 = 0; e2 < NO_SHIPS; e2 ++)
   {
    if (ship[a][e2].type == SHIP_NONE)
     continue;
    if (ship[a][e2].leader == e)
    {
     if (e2 == new_leader) // new leader found!
     {
      ship[a][e2].leader = -1;
      ship[a][e2].formation_size = EE.formation_size - 1;
      ship[a][e2].leading_formation = 0;
      ship[a][e2].formation_position = 0;
      ship[a][e2].mission = EE.mission;
      ship[a][e2].target = EE.target;
      ship[a][e2].action = ACT_AWAY;
/*      if (ship[a][e2].mission == MISSION_ESCORT)
       ship[a][e2].mission = MISSION_SCRAMBLE;
      if (ship[a][e2].mission == MISSION_ESCORT
       || ship[a][e2].mission == MISSION_GUARD)
       ship[a][e2].mission = MISSION_SCRAMBLE;*/
     }
       else
       {
        ship[a][e2].leader = new_leader; // could be -1 if no new leader.
        if (new_leader == -1) // if fighter is on its own, it will return to the nearest wship
        { // actually this is probably impossible - if there is at least one fighter left, it will be leader. So this code may be pointless.
         if (ship[a][e2].mission == MISSION_ESCORT
          || ship[a][e2].mission == MISSION_GUARD)
//           ship[a][e2].mission = MISSION_SCRAMBLE;
            fighter_find_wship_to_guard(a, e2, ship[a][e2].x, ship[a][e2].y);
        }
         else
         {
//          if (ship[a][e2].mission == MISSION_GUARD)
//           ship[a][e2].mission = MISSION_ESCORT;
           // mission shouldn't change, I think - new leader, same mission
         }
       }
     if (ship[a][e2].action == ACT_FORM
      || ship[a][e2].action == ACT_GUARD)
      ship[a][e2].action = ACT_AWAY;
      // will reform anyway if new leader enters transit mode
    }
   }
  }
 }

 int b;

 for (b = 0; b < NO_BULLETS; b ++)
 {
  if (bullet[a2][b].type != BULLET_NONE
   && bullet[a2][b].target_e == e)
    bullet[a2][b].target_e = TARGET_NONE;
 }

 if (a == TEAM_ENEMY)
 {
  calculate_threat();
  int p, w;

  for (p = 0; p < 2; p ++)
  {
   for (w = 0; w < 2; w ++)
   {
    switch (PP.weapon_type [w])
    {
     case WPN_AWS_MISSILE:
     case WPN_AF_MISSILE:
      if (PP.weapon_target [w] [0] == e)
      {
       PP.weapon_target [w] [0] = TARGET_NONE;
//       PP.weapon_charge [w] = 0;
       PP.weapon_lock [w] = 0;
      }
      break;
     case WPN_DEFLECT:
      if (PP.weapon_target [w] [0] == e)
      {
       PP.weapon_target [w] [0] = TARGET_NONE;
      }
      break;
    }
   }
  }


 }
  else // must be a friend
  {
   int i, w;
   for (w = 0; w < WINGS; w ++)
   {
    for (i = 0; i < WING_SIZE; i ++)
    {
     if (player[0].wing [w] [i] == e)
     {
      player[0].wing [w] [i] = -1;
      player[0].wing_size [w] --;
      if (player[0].wing_size [w] == 0)
       player[0].wing_command = -1;
     }
     if (player[1].wing [w] [i] == e)
     {
      player[1].wing [w] [i] = -1;
      player[1].wing_size [w] --;
      if (player[1].wing_size [w] == 0)
       player[1].wing_command = -1;
     }
    }
   }
  }

}

/*
int edist(int a, int e, int p)
{

 return (int) hypot(EE.y - PP.y, EE.x - PP.x);

}*/

int edist(int a, int e, int target)
{

 if (target == TARGET_NONE)
  return 1;

 int x, y;

 if (target >= 0)
 {
  x = ship[a^1][target].x;
  y = ship[a^1][target].y;
 }
  else
  {
   if (target == TARGET_P1)
   {
    x = player[0].x;
    y = player[0].y;
   }
     else
     {
// must be P2:
      x = player[1].x;
      y = player[1].y;
     }
  }

 int dist = hypot(EE.y - y, EE.x - x);

 return dist;

}

int edist_test_less(int a, int e, int target, int max)
{

 if (target == TARGET_NONE)
  return 0;

 int x, y;

 if (target >= 0)
 {
  x = ship[a^1][target].x;
  y = ship[a^1][target].y;
 }
  else
  {
   if (target == TARGET_P1)
   {
    x = player[0].x;
    y = player[0].y;
   }
     else
     {
// must be P2:
      x = player[1].x;
      y = player[1].y;
     }
  }

  if (abs(EE.y - y) > max || abs(EE.x - x) > max)
   return 0;

 if (hypot(EE.y - y, EE.x - x) > max)
  return 0;

 return 1;

}

int edist_test_more(int a, int e, int target, int min)
{

 if (target == TARGET_NONE)
  return 0;

 int x, y;

 if (target >= 0)
 {
  x = ship[a^1][target].x;
  y = ship[a^1][target].y;
 }
  else
  {
   if (target == TARGET_P1)
   {
    x = player[0].x;
    y = player[0].y;
   }
     else
     {
// must be P2:
      x = player[1].x;
      y = player[1].y;
     }
  }

  if (abs(EE.y - y) + abs(EE.x - x) < min)
   return 0;

 if (hypot(EE.y - y, EE.x - x) < min)
  return 0;

 return 1;

}



int efrienddist(int a, int e, int target)
{

 if (target == TARGET_NONE)
  return 1;

 int x, y;

 if (target >= 0)
 {
  x = ship[a][target].x;
  y = ship[a][target].y;
 }
  else
  {
   if (target == TARGET_P1)
   {
    x = player[0].x;
    y = player[0].y;
   }
     else
     {
// must be P2:
      x = player[1].x;
      y = player[1].y;
     }
  }

 int dist = hypot(EE.y - y, EE.x - x);

 return dist;

}
