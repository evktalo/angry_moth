#include "config.h"

#include "allegro.h"

#include "display.h"

#define NO_ELONGATION 1024
/*
SHIP_FRIEND3,
SHIP_OLD3,
SHIP_OLD2,
SHIP_FIGHTER_FRIEND,

SHIP_EBASE,
SHIP_SCOUTCAR,
SHIP_SCOUT3,
SHIP_SCOUT2,
SHIP_BOMBER,
SHIP_ESCOUT,
SHIP_EINT,
SHIP_FIGHTER,
*/

#define WSHIP_TURN 2
//#define FIGHTER_DRAG 15


struct eclass_struct eclass [NO_SHIP_TYPES] =
{
 { // SHIP_NONE
 ECLASS_WSHIP,
 FCLASS_NONE,
 0,
 0, // parts
 0, // turrets
 0, // engines
 },


 { // SHIP_FRIEND3
 ECLASS_WSHIP,
 FCLASS_NONE,
 65000,
 3, // parts
 3, // turrets
 2, // engines
 {40000, 40000, 40000,0,0}, // hp
// {130, 130, 130,0,0}, // energy_production
 {390, 0, 0,0,0}, // energy_production
 30000, // max_shield
 12, // shield_recharge
 {
  {DSPRITE_WSHIP_FRIEND3_1, 24, 30},
  {DSPRITE_WSHIP_FRIEND3_2, 31, 6},
  {DSPRITE_WSHIP_FRIEND3_3, 21, -3},
  {DSPRITE_WSHIP_FRIEND3_4, 10, 18},
  {DSPRITE_WSHIP_FRIEND3_5, 29, -10},
 }, // indices and image offsets for dsprites
 1, // fins
 1, // generator_part (which part contains the shield generator) - centre
 -1, // generator_y (where to draw the line to on the targetting display)
 {7, 7}, // engine power
 2, // engine_output
 MOVE_NORMAL, // move_mode
 WSHIP_TURN, // turn
 WSHIP_DRAG, // drag
 RP_FRIEND3_TURRET_1,
 50000, // structure
 NO_ELONGATION, // elongation
 5, // blip_strength
 0 // bullet_speed (fighters only)
 },

 { // SHIP_OLD3
 ECLASS_WSHIP,
 FCLASS_NONE,
 65000,
 3, // parts
 3, // turrets
 4, // engines
 {40000, 40000, 40000,0,0}, // hp
// {130, 130, 130,0,0}, // energy_production
 {450, 150, 150,0,0}, // energy_production
 35000, // max_shield
 15, // shield_recharge
 {
  {DSPRITE_WSHIP_OLD3_1, 20, 26},
  {DSPRITE_WSHIP_OLD3_2, 20, 11},
  {DSPRITE_WSHIP_OLD3_3, 15, -7},
  {DSPRITE_WSHIP_OLD3_4, 7, 18},
  {DSPRITE_WSHIP_OLD3_5, 32, -1},
/*
  {DSPRITE_WSHIP_OLD3_1, 11, 24},
  {DSPRITE_WSHIP_OLD3_2, 14, 8},
  {DSPRITE_WSHIP_OLD3_3, 14, -7},
  {DSPRITE_WSHIP_OLD3_4, 7, 17},
  {DSPRITE_WSHIP_OLD3_5, 26, 15},
*/
 }, // indices and image offsets for dsprites
 1, // fins
 2, // generator_part (which part contains the shield generator)
 18, // generator_y (where to draw the line to on the targetting display)
 {12,12,20,20}, // engine power
 2, // engine_output
 MOVE_NORMAL, // move_mode
 WSHIP_TURN, // turn
 WSHIP_DRAG, // drag
 RP_OLD3_TURRET_1,
 50000, // structure
 NO_ELONGATION, // elongation
 5, // blip_strength
 0 // bullet_speed (fighters only)
 },

 { // SHIP_OLD2
 ECLASS_WSHIP,
 FCLASS_NONE,
 50000,
 2, // parts
 2, // turrets
 2, // engines
 {20000, 20000,0,0,0}, // hp
// {100, 100,0,0,0}, // energy_production
 {200, 100,0,0,0}, // energy_production
 20000, // max_shield
 20, // shield_recharge
 {
/*
  {DSPRITE_WSHIP_OLD2_1, 11, 20},
  {DSPRITE_WSHIP_OLD2_2, 13, 0},
  {DSPRITE_WSHIP_OLD2_3, 4, 12},
  {DSPRITE_WSHIP_OLD2_4, 24, -3},*/
  {DSPRITE_WSHIP_OLD2_1, 19, 23},
  {DSPRITE_WSHIP_OLD2_2, 19, 0},
  {DSPRITE_WSHIP_OLD2_3, 7, 16},
  {DSPRITE_WSHIP_OLD2_4, 29, -10},
 }, // indices and image offsets for dsprites
 1, // fins
 1, // generator_part (which part contains the shield generator)
 13, // generator_y (where to draw the line to on the targetting display)
 {6,6}, // engine power
 2, // engine_output
 MOVE_NORMAL, // move_mode
 WSHIP_TURN, // turn
 WSHIP_DRAG, // drag
 RP_OLD2_TURRET_1,
 20000, // structure
 NO_ELONGATION, // elongation
 4, // blip_strength
 0 // bullet_speed
 },

 { // SHIP_DROM
 ECLASS_WSHIP,
 FCLASS_NONE,
 70000,
 2, // parts
 0, // turrets
 2, // engines
 {20000, 20000,0,0,0}, // hp
// {100, 100,0,0,0}, // energy_production
 {200, 100,0,0,0}, // energy_production
 22000, // max_shield
 19, // shield_recharge
 {
  {DSPRITE_WSHIP_DROM_1, 19, 0},
  {DSPRITE_WSHIP_DROM_2, 19, 25},
  {DSPRITE_WSHIP_DROM_3, 6, 17},
 }, // indices and image offsets for dsprites
 0, // fins
 0, // generator_part (which part contains the shield generator)
 13, // generator_y (where to draw the line to on the targetting display)
 {5, 5}, // engine power
 3, // engine_output
 MOVE_NORMAL, // move_mode
 WSHIP_TURN, // turn
 WSHIP_DRAG, // drag
 RP_DROM_ENGINE_1,
 30000, // structure
 NO_ELONGATION, // elongation
 4, // blip_strength
 0 // bullet_speed
 },


 { // SHIP_AUROCHS
  ECLASS_FIGHTER,
  FCLASS_BOMBER,
  7000,
  1, // parts
  0, // turrets
  2, // engines
  {3500, 0,0,0,0,0},
  {0, 0, 0,0,0}, // energy_production
  3000, // max_shield
  5, // shield_recharge
  {
   {DSPRITE_AUROCHS_1, 14, 12}
  }, // indices and image offsets for dsprites
  0, // fins
  0, // generator_part (which part contains the shield generator) - not relevant for fighters
  0, // generator_y - not relevant for fighters
  {3, 3}, // engine power
  85, // engine_output
  MOVE_NORMAL, // move_mode
  26, // turn
  12, // drag
  0, // not used for fighters
  0, // not used for fighters
  NO_ELONGATION, // elongation
  2, // blip_strength
  9000 // bullet_speed (fighters only)
 },

 { // SHIP_IBEX
  ECLASS_FIGHTER,
  FCLASS_BOMBER,
  7000,
  1, // parts
  0, // turrets
  2, // engines
  {3000, 0,0,0,0,0},
  {0, 0, 0,0,0}, // energy_production
  2500, // max_shield
  4, // shield_recharge
  {
   {DSPRITE_IBEX_1, 14, 12}
  }, // indices and image offsets for dsprites
  0, // fins
  0, // generator_part (which part contains the shield generator) - not relevant for fighters
  0, // generator_y - not relevant for fighters
  {3, 3}, // engine power
  120, // engine_output
  MOVE_SLIDE, // move_mode
  33, // turn
  13, // drag
  0, // not used for fighters
  0, // not used for fighters
  NO_ELONGATION, // elongation
  2, // blip_strength
  9000 // bullet_speed (fighters only)
 },

 { // SHIP_FIGHTER_FSTRIKE
  ECLASS_FIGHTER,
  FCLASS_BOMBER,
  7000,
  1, // parts
  0, // turrets
  2, // engines
  {3000, 0,0,0,0,0},
  {0, 0, 0,0,0}, // energy_production
  2500, // max_shield
  4, // shield_recharge
  {
   {DSPRITE_FRIEND_1, 14, 12}
  }, // indices and image offsets for dsprites
  0, // fins
  0, // generator_part (which part contains the shield generator) - not relevant for fighters
  0, // generator_y - not relevant for fighters
  {2, 2}, // engine power
  88, // engine_output
  MOVE_NORMAL, // move_mode
  33, // turn
  12, // drag
  0, // not used for fighters
  0, // not used for fighters
  NO_ELONGATION, // elongation
  2, // blip_strength
  9000 // bullet_speed (fighters only)
 },


 { // SHIP_LACEWING
  ECLASS_FIGHTER,
  FCLASS_FIGHTER,
  6000,
  1, // parts
  0, // turrets
  1, // engines
  {2200, 0,0,0,0,0},
  {0, 0, 0,0,0}, // energy_production
  1500, // max_shield
  3, // shield_recharge
  {
   {DSPRITE_LACEWING_1, 14, 12}
  }, // indices and image offsets for dsprites
  0, // fins
  0, // generator_part (which part contains the shield generator) - not relevant for fighters
  0, // generator_y - not relevant for fighters
  {5}, // engine power
  110, //170, // engine_output
  MOVE_NORMAL, // move_mode
  43, // turn
  13, // drag
  0, // not used for fighters
  0, // not used for fighters
  NO_ELONGATION, // elongation
  2, // blip_strength
  9000 // bullet_speed (fighters only)
 },


 { // SHIP_MONARCH
  ECLASS_FIGHTER,
  FCLASS_FIGHTER,
  6000,
  1, // parts
  0, // turrets
  1, // engines
  {2300, 0,0,0,0,0},
  {0, 0, 0,0,0}, // energy_production
  1600, // max_shield
  3, // shield_recharge
  {
   {DSPRITE_MONARCH_1, 14, 12}
  }, // indices and image offsets for dsprites
  0, // fins
  0, // generator_part (which part contains the shield generator) - not relevant for fighters
  0, // generator_y - not relevant for fighters
  {4}, // engine power
  110, // engine_output
  MOVE_SLIDE, // move_mode
  46, // turn
  14, // drag - high, for manoeuvreability
  0, // not used for fighters
  0, // not used for fighters
  NO_ELONGATION, // elongation
  2, // blip_strength
  9000 // bullet_speed (fighters only)
 },

 { // SHIP_FIGHTER_FRIEND (SANDFLY)
  ECLASS_FIGHTER,
  FCLASS_FIGHTER,
  6000,
  1, // parts
  0, // turrets
  1, // engines
  {2500, 0,0,0,0,0},
  {0, 0, 0,0,0}, // energy_production
  2000, // max_shield
  3, // shield_recharge
  {
   {DSPRITE_FRIEND_1, 14, 12}
  }, // indices and image offsets for dsprites
  0, // fins
  0, // generator_part (which part contains the shield generator) - not relevant for fighters
  0, // generator_y - not relevant for fighters
  {4}, // engine power
  105, //170, // engine_output
  MOVE_NORMAL, // move_mode
  40, // turn
  13, // drag
  0, // not used for fighters
  0, // not used for fighters
  NO_ELONGATION, // elongation
  2, // blip_strength
  9000 // bullet_speed (fighters only)
 },

 { // SHIP_LINER
 ECLASS_WSHIP,
 FCLASS_NONE,
 30000,
 2, // parts
 0, // turrets
 2, // engines
 {15000, 15000,0,0,0}, // hp
// {100, 100,0,0,0}, // energy_production
 {100, 100,0,0,0}, // energy_production
 15000, // max_shield
 18, // shield_recharge
 {
  {DSPRITE_WSHIP_LINER_1, 10, 0},
  {DSPRITE_WSHIP_LINER_2, 10, 19},
  {DSPRITE_WSHIP_LINER_3, 6, 11},
 }, // indices and image offsets for dsprites
 0, // fins
 0, // generator_part (which part contains the shield generator)
 13, // generator_y (where to draw the line to on the targetting display)
 {4, 4}, // engine power
 3, // engine_output
 MOVE_NORMAL, // move_mode
 WSHIP_TURN, // turn
 WSHIP_DRAG, // drag
 RP_LINER_ENGINE_1,
 20000, // structure
 NO_ELONGATION, // elongation
 3, // blip_strength
 0 // bullet_speed
 },


 { // SHIP_EBASE
 ECLASS_WSHIP,
 FCLASS_NONE,
 150000,
 5, // parts
 5, // turrets
 0, // engines
 {60000, 60000,60000,60000,60000}, // hp
// {250, 250,250, 250, 250}, // energy_production
 {1250, 0,0, 0, 0}, // energy_production
 500000, // max_shield
 6, // shield_recharge
 {
  {DSPRITE_WSHIP_EBASE_1, 13, 33},
  {DSPRITE_WSHIP_EBASE_2, 13, -22},
  {DSPRITE_WSHIP_EBASE_3, 15, 20},
  {DSPRITE_WSHIP_EBASE_4, 15, -9},
  {DSPRITE_WSHIP_EBASE_5, 18, 9},
  {DSPRITE_WSHIP_EBASE_6, 4, 9},
 }, // indices and image offsets for dsprites
 0, // fins
 4, // generator_part (which part contains the shield generator)
 0, // generator_y (where to draw the line to on the targetting display)
 {0, 0, 0}, // engine power
 0, // engine_output
 MOVE_NORMAL, // move_mode
 WSHIP_TURN, // turn
 WSHIP_DRAG, // drag
 RP_EBASE_TURRET_1,
 200000, // structure
 NO_ELONGATION, //-1300, // elongation - negative value indicates both front and back elongated
 6, // blip_strength
 0 // bullet_speed (fighters only)
 },


 { // SHIP_ECARRIER
 ECLASS_WSHIP,
 FCLASS_NONE,
 110000,
 2, // parts
 1, // turrets
 4, // engines
 {40000, 30000,0,0,0}, // hp
// {50, 50,0,0,0}, // energy_production
 {100, 0,0,0,0}, // energy_production
 25000, // max_shield
 28, // shield_recharge
 {
  {DSPRITE_WSHIP_ECARRIER_1, 14, -5}, // note: parts are in opposite order because
  {DSPRITE_WSHIP_ECARRIER_2, 14, 24}, //  of carrier's single rear turret
  {DSPRITE_WSHIP_ECARRIER_3, 6, 4},
  {DSPRITE_WSHIP_ECARRIER_4, 15, -10},
 }, // indices and image offsets for dsprites
 0, // fins
 0, // generator_part (which part contains the shield generator)
 15, // generator_y (where to draw the line to on the targetting display)
 {7, 5, 5, 7}, // engine power
 2, // engine_output
 MOVE_NORMAL, // move_mode
 WSHIP_TURN, // turn
 WSHIP_DRAG, // drag
 RP_ECARRIER_TURRET_1,
 30000, // structure
 1200, // elongation - positive value indicates only front elongated
 6, // blip_strength
 0 // bullet_speed (fighters only)
 },


 { // SHIP_SCOUTCAR
 ECLASS_WSHIP,
 FCLASS_NONE,
 60000,
 2, // parts
 1, // turrets
 1, // engines
 {20000, 20000,0,0,0}, // hp
// {50, 50,0,0,0}, // energy_production
 {100, 0,0,0,0}, // energy_production
 15000, // max_shield
 25, // shield_recharge
 {
  {DSPRITE_WSHIP_SCOUTCAR_1, 14, -5}, // note: parts are in opposite order because
  {DSPRITE_WSHIP_SCOUTCAR_2, 14, 24}, //  of carrier's single rear turret
  {DSPRITE_WSHIP_SCOUTCAR_3, 6, 4},
  {DSPRITE_WSHIP_SCOUTCAR_4, 15, -10},
 }, // indices and image offsets for dsprites
 1, // fins
 0, // generator_part (which part contains the shield generator)
 15, // generator_y (where to draw the line to on the targetting display)
 {8}, // engine power
 2, // engine_output
 MOVE_NORMAL, // move_mode
 WSHIP_TURN, // turn
 WSHIP_DRAG, // drag
 RP_SCOUTCAR_TURRET_1,
 20000, // structure
 1200, // elongation - positive value indicates only front elongated
 6, // blip_strength
 0 // bullet_speed (fighters only)
 },

 { // SHIP_FREIGHT
 ECLASS_WSHIP,
 FCLASS_NONE,
 60000,
 2, // parts
 1, // turrets
 3, // engines
 {20000, 20000,0,0,0}, // hp
// {50, 50,0,0,0}, // energy_production
 {100, 0,0,0,0}, // energy_production
 15000, // max_shield
 25, // shield_recharge
 {
  {DSPRITE_WSHIP_FREIGHT_1, 17, -2}, // note: parts are in opposite order because
  {DSPRITE_WSHIP_FREIGHT_2, 18, 24}, //  of carrier's single rear turret
  {DSPRITE_WSHIP_FREIGHT_3, 6, 9},
  {DSPRITE_WSHIP_FREIGHT_4, 21, -3},
 }, // indices and image offsets for dsprites
 1, // fins
 0, // generator_part (which part contains the shield generator)
 15, // generator_y (where to draw the line to on the targetting display)
 {6, 6, 6}, // engine power
 2, // engine_output
 MOVE_NORMAL, // move_mode
 WSHIP_TURN, // turn
 WSHIP_DRAG, // drag
 RP_FREIGHT_TURRET_1,
 30000, // structure
 1100, // elongation - positive value indicates only front elongated
 5, // blip_strength
 0 // bullet_speed (fighters only)
 },

 { // SHIP_SCOUT3
 ECLASS_WSHIP,
 FCLASS_NONE,
 70000,
 3, // parts
 3, // turrets
 3, // engines
 {30000, 30000,30000,0,0}, // hp
// {150, 150,150,0,0}, // energy_production
 {450, 0,0,0,0}, // energy_production
 40000, // max_shield
 12, // shield_recharge
 {
  {DSPRITE_WSHIP_SCOUT3_1, 20, 23},
  {DSPRITE_WSHIP_SCOUT3_2, 22, 8},
  {DSPRITE_WSHIP_SCOUT3_3, 17, -4},
  {DSPRITE_WSHIP_SCOUT3_4, 10, 21},
  {DSPRITE_WSHIP_SCOUT3_5, 25, -7},
 }, // indices and image offsets for dsprites
 1, // fins
 2, // generator_part (which part contains the shield generator)
 15, // generator_y (where to draw the line to on the targetting display)
 {6, 6, 10}, // engine power
 2, // engine_output
 MOVE_NORMAL, // move_mode
 WSHIP_TURN, // turn
 WSHIP_DRAG, // drag
 RP_SCOUT3_TURRET_1,
 40000, // structure
 NO_ELONGATION, // elongation
 5, // blip_strength
 0 // bullet_speed (fighters only)
 },


 { // SHIP_SCOUT2
 ECLASS_WSHIP,
 FCLASS_NONE,
 50000,
 2, // parts
 2, // turrets
 2, // engines
 {20000, 20000,0,0,0}, // hp
// {100, 100,0,0,0}, // energy_production
 {200, 0,0,0,0}, // energy_production
 20000, // max_shield
 25, // shield_recharge
 {
  {DSPRITE_WSHIP_SCOUT2_1, 20, 22},
  {DSPRITE_WSHIP_SCOUT2_2, 19, 0},
  {DSPRITE_WSHIP_SCOUT2_3, 8, 18},
  {DSPRITE_WSHIP_SCOUT2_4, 20, -9},
 }, // indices and image offsets for dsprites
 1, // fins
 1, // generator_part (which part contains the shield generator)
 8, // generator_y (where to draw the line to on the targetting display)
 {6, 6}, // engine power
 2, // engine_output
 MOVE_NORMAL, // move_mode
 WSHIP_TURN, // turn
 WSHIP_DRAG, // drag
 RP_SCOUT2_TURRET_1,
 20000, // structure
 NO_ELONGATION, // elongation
 4, // blip_strength
 0 // bullet_speed (fighters only)
 },

 { // SHIP_BOMBER
  ECLASS_FIGHTER,
  FCLASS_BOMBER,
  10000,
  1, // parts
  0, // turrets
  2, // engines
  {4000, 0,0,0,0,0},
  {0, 0, 0,0,0}, // energy_production
  3000, // max_shield
  8, // shield_recharge
  {
   {DSPRITE_BOMBER_1, 16, 12}
  }, // indices and image offsets for dsprites
  0, // fins
  0, // generator_part (which part contains the shield generator) - not relevant for fighters
  0, // generator_y - not relevant for fighters
  {2, 2}, // engine power
  80, // engine_output
  MOVE_NORMAL, // move_mode
  20, // turn
  12, // drag
  0, // not used for fighters
  0, // not used for fighters
  NO_ELONGATION, // elongation
  3, // blip_strength
  2000 // bullet_speed (fighters only)
 },

 { // SHIP_ESCOUT
  ECLASS_FIGHTER,
  FCLASS_FIGHTER,
  10000,
  1, // parts
  0, // turrets
  2, // engines
  {5000, 0,0,0,0,0},
  {0, 0, 0,0,0}, // energy_production
  3500, // max_shield
  6, // shield_recharge
  {
   {DSPRITE_ESCOUT_1, 18, 17}
  }, // indices and image offsets for dsprites
  0, // fins
  0, // generator_part (which part contains the shield generator) - not relevant for fighters
  0, // generator_y - not relevant for fighters
  {3, 2}, // engine power
  95, // engine_output
  MOVE_SLIDE, // move_mode
  20, // turn
  12, // drag
  0, // not used for fighters
  0, // not used for fighters
  NO_ELONGATION, // elongation
  2, // blip_strength
  7000 // bullet_speed (fighters only)
 },


 { // SHIP_EINT
  ECLASS_FIGHTER,
  FCLASS_FIGHTER,
  9000,
  1, // parts
  0, // turrets
  1, // engines
  {4000, 0,0,0,0,0}, // hull
  {0, 0, 0,0,0}, // energy_production
  2500, // max_shield
  5, // shield_recharge
  {
   {DSPRITE_EINT_1, 10, 17}
  }, // indices and image offsets for dsprites
  0, // fins
  0, // generator_part (which part contains the shield generator) - not relevant for fighters
  0, // generator_y - not relevant for fighters
  {5}, // engine power
  110, //230, // engine_output
  MOVE_NORMAL, // move_mode
  30, // turn
  13, // drag
  0, // not used for fighters
  0, // not used for fighters
  NO_ELONGATION, // elongation
  2, // blip_strength
  7000 // bullet_speed (fighters only)
 },

 { // SHIP_FIGHTER
  ECLASS_FIGHTER,
  FCLASS_FIGHTER,
  6000,
  1, // parts
  0, // turrets
  1, // engines
  {2000, 0,0,0,0,0}, // hull
  {0, 0, 0,0,0}, // energy_production
  1500, // max_shield
  3, // shield_recharge
  {
   {DSPRITE_FIGHTER_1, 11, 11}
  }, // indices and image offsets for dsprites
  0, // fins
  0, // generator_part (which part contains the shield generator) - not relevant for fighters
  0, // generator_y - not relevant for fighters
  {4}, // engine power
  93, //165, // engine_output
  MOVE_NORMAL, // move_mode
  25, // turn
  13, // drag
  0, // not used for fighters
  0, // not used for fighters
  NO_ELONGATION, // elongation
  2, // blip_strength
  6000 // bullet_speed (fighters only)
 },

// remember to set threat in calculate threat in level.c if adding an enemy!

};

