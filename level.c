
#include "config.h"

#include "allegro.h"

#include <math.h>

#include "globvars.h"

#include "stuff.h"

#include "bullet.h"
#include "bullet.h"
#include "ship.h"
#include "palette.h"
#include "text.h"
#include "message.h"
#include "sound.h"

#define CV convoy[cv]


void set_turret(int a, int e, int t, int type, int rest_angle);
void init_level(void);
void run_convoys(void);
void run_level(void);
void start_script(int scr);
void start_new_script(int news);
void pcon_met(int scr, int pcon);
void call_condition(int ccon_type, int val1, int val2, int val3);
void condition_eship_destroyed(int e);
void condition_ecarrier_disabled(int e);
void condition_fship_destroyed(int e);
void ccon_met(int scr, int ccon);
void set_convoy_turning(int cv);
void run_messages(void);
void new_message(int m, int m_from, int m_from2, int m_to);
int new_message_text(int m, int comm);
void pack_messages(int deleted);
void setup_new_ship(int a, int e, int subtype);
int activate_carrier_launch(int a, int cv, int type);
void set_approach_target(int cv1, int cv2, int x_offset, int y_offset);
void about_to_end_script(int scr);
int get_convoy_speed(int cv);
void convoy_jump_out(int a, int cv);
void setup_player_wing(int p, int type, int number);
void calculate_threat(void);

void add_ship_to_command(int a, int e, int command);
void add_ship_to_letter(int a, int e, int letter);
void display_team(void);
void ship_from_level_data(int s, int a, int e);
void convoy_jumps_in(int cv, int flag);
void setup_special_stage_conditions(void);
void special_convoy(int cv);


char nearby(int x, int y);

//void init_stars(int p);

struct levelstruct
{
 int team;
 int type;
 int subtype; // doesn't affect basic type of enemy, but can affect turrets and a few details.
 int mission; // irrelevant for wships
 int action;
 int group; //  groups are for fighters and for wships that are being guarded (a wship can only be leader)
 int group_position;
 int x, y; // for fighters (group leaders only)
 int convoy; // convoys are for wships
 int flags; // various settings e.g. command. Data type needs to contain enough bits to store all flags
 int flags_2; // various settings e.g. command. Data type needs to contain enough bits to store all flags
  // ships should be in order of command
 int convoy_position [CONVOY_ARRANGEMENTS] [2]; // angle, distance
// int convoy_angle [CONVOY_ARRANGEMENTS]; // angle of the wship's displacement from the convoy centre, in different arrangements
// int convoy_dist [CONVOY_ARRANGEMENTS]; // distance from convoy centre

};

#define NO_LEVELS 10
#define LEVEL_SHIPS 100
#define LEVEL_SCRIPTS 50
#define GROUPS 28

int group_leader [GROUPS];

enum
{
VAR_NONE,
VAR_HEAVY,
VAR_ANTI,
VAR_LONG
};

#define POLLED_CONDITIONS 5
// polled conditions are tested for each tick (or each <x> ticks as they might be slow)

#define CALLED_CONDITIONS 5
// called conditions are tested for when particular events occur, e.g. a particular ship is destroyed

#define CON_VALUES 4

enum
{
PCON_NONE,
PCON_SETUP_CONVOY,
PCON_XY, // vals: x, y
PCON_TIME, // vals: the time that running_script_time must equal to trigger PCON (in seconds)
PCON_LEVEL_TIME, // vals: the time that arena.time must equal to trigger (in seconds)
PCON_CONVOY_DIST, // vals: the distance that the convoy must be from the other convoy to trigger
PCON_CONVOY_DIST_APART, // vals: like convoy_dist but for moving away
PCON_LEVEL_TIME_SPECIAL, // uses an scon value for the time waited. vals: index of the scon.
PCON_ALWAYS_PASS, // easy
};

enum
{
CCON_NONE,
CCON_SHIP_DESTROYED, // not yet implemented
CCON_ECONVOY_DESTROYED_REMAIN, // convoy destroyed! But some wships (+fighters maybe) remain
CCON_ECONVOY_DESTROYED_FREMAIN, // convoy and all ewships destroyed! But some fighters remain
CCON_ECONVOY_DESTROYED_ALL, // called if this was the last convoy
// For the convoy_destroyed ones, the relevant convoy is set in the CCON data; it's not the script convoy.
CCON_EFIGHTERS_DESTROYED_REMAIN, // called if all efighters (inc bombers) destroyed, but ewships remain
CCON_EWSHIPS_DESTROYED_FREMAIN, // called if all efighters (inc bombers) destroyed, but ewships remain
CCON_THREAT_FALLS, // called if arena.threat falls below a certain level
CCON_FTHREAT_FALLS, // called if threat value for fighters only falls below a certain level
CCON_WSTHREAT_FALLS, // called if threat value for wships only falls below a certain level
CCON_EALL_DESTROYED,
CCON_FWSHIPS_DESTROYED,
CCON_FCONVOY_DESTROYED_REMAIN,
CCON_FFIGHTERS_DESTROYED_REMAIN,
CCON_ECARRIERS_DISABLED, // no more enemy carriers with can_launch == 1 (through either damage or destruction)
CCON_CONVOY_DESTROYED // can be friendly or enemy
};

struct conditionstruct
{
 int type;
 int val [CON_VALUES];
 int result_type;
 int result;
 int msg;
// int msg_source;
 int msg_from;
 int msg_from2; // usually will contain command rank number - but if some members of a command have
  // beean destroyed, the message source will count from zero and skip them.
 int msg_to;
 int test; // if 1, condition only passed if convoy of script still has ships in it
};

enum
{
TEST_NONE,
TEST_CONVOY_EXISTS
};

// special conditions:
#define SCON 10
int scon [SCON];

enum
{
S2SCON_C4WAIT,
S2SCON_C5WAIT,
S2SCON_C6WAIT,
S2SCON_C7ANGLE, // angle from base it appears at
S2SCON_C7WAIT,
S2SCON_C8ANGLE, // angle from base it appears at
S2SCON_C8WAIT,
};


enum
{
RESULT_NONE,
RESULT_NEXT_SCRIPT,
RESULT_START_SCRIPT, // starts new script without killing current one
RESULT_END_SCRIPT,
RESULT_LAUNCH,
RESULT_JOIN_CONVOY, // also kills script even if joined convoy no longer exists
RESULT_CONVOY_JUMP_OUT, // also kills script
RESULT_MISSION_OVER,
RESULT_MISSION_FAIL, // only happens if all friendly wships destroyed
RESULT_CHANGE_ARRANGE // change arrangement without changing anything else
};

enum
{
MSOURCE_NONE,
MSOURCE_COMMAND
};

#define FLAGS 0
#define FLAG_COMMAND_1 1
#define FLAG_COMMAND_2 (1<<1)
#define FLAG_COMMAND_3 (1<<2)
#define FLAG_ALPHA (1<<3)
#define FLAG_BETA (1<<4)
#define FLAG_GAMMA (1<<5)
#define FLAG_SPAWN_1 (1<<6)
#define FLAG_SPAWN_2 (1<<7)
#define FLAG_SPAWN_3 (1<<8)
#define FLAG_JUMP_1 (1<<9)
#define FLAG_JUMP_2 (1<<10)
#define FLAG_JUMP_3 (1<<11)
#define FLAG_JUMP_4 (1<<12)
// if adding jumps, add to init_level as well or they won't be skipped at start.

#define FLAGS_2 0

#define SCRIPT_VALS 5

struct scriptstruct
{
 int team; // may or may not be relevant
 int convoy;
 int type;
 int x, y; // means:
 //  starting position for setup
 //  target for move_xy
 //  convoy for approach_convoy
 int throttle;
 int arrange;
 int val [SCRIPT_VALS];
// int pcons;
 struct conditionstruct pcon [POLLED_CONDITIONS];
// int ccons;
 struct conditionstruct ccon [CALLED_CONDITIONS];
};

enum
{
SCRIPT_L1_EMPTY,
SCRIPT_L1C0_SETUP,
SCRIPT_L1C0_MOVE,
SCRIPT_L1C0_MOVE2,
SCRIPT_L1C0_MOVE3,
SCRIPT_L1C0_MESSAGE1,
SCRIPT_L1C0_MESSAGE2,
//SCRIPT_L1C0_MOVE2,
SCRIPT_L1C1_SETUP,
SCRIPT_L1C1_MOVE,
SCRIPT_L1C2_SETUP,
SCRIPT_L1C2_MOVE,
SCRIPT_L1C3_SETUP,
SCRIPT_L1C3_MOVE,
SCRIPT_L1C4_SETUP,
SCRIPT_L1C4_MOVE,
SCRIPT_L1C5_SETUP,
SCRIPT_L1C5_MOVE,
SCRIPT_L1C6_SETUP,
SCRIPT_L1C6_MOVE,
SCRIPT_L1C7_SETUP,
SCRIPT_L1C7_MOVE,
SCRIPT_L1C8_SETUP,
SCRIPT_L1C8_MOVE,
SCRIPT_L1C9_SETUP,
SCRIPT_L1C9_MOVE,
SCRIPT_L1C10_SETUP,
SCRIPT_L1C10_MOVE,
//SCRIPT_L1C1_MOVE2,
SCRIPT_L1_MESSAGE
};

enum
{
SCRIPT_L2_EMPTY,
SCRIPT_L2_BASIC,
SCRIPT_L2C0_SETUP,
SCRIPT_L2C0_MOVE,
SCRIPT_L2C0_MOVE2,
SCRIPT_L2C0_ATTACK,
SCRIPT_L2C0_ESCAPE,
SCRIPT_L2C1_SETUP,
SCRIPT_L2C1_MOVE,
SCRIPT_L2C1_MOVE2,
SCRIPT_L2C1_ATTACK,
SCRIPT_L2C1_ESCAPE,
SCRIPT_L2C2_SETUP,
SCRIPT_L2C2_MOVE,
SCRIPT_L2C2_MOVE2,
SCRIPT_L2C2_ATTACK,
SCRIPT_L2C2_ESCAPE,
SCRIPT_L2C3_SETUP,
SCRIPT_L2C3_RUN,
SCRIPT_L2C4_SETUP,
SCRIPT_L2C4_MOVE,
SCRIPT_L2C4_MOVE2,
SCRIPT_L2C4_MOVE3,
SCRIPT_L2C4_DEFEND,
SCRIPT_L2C4_DEFEND2,
SCRIPT_L2C5_SETUP,
SCRIPT_L2C5_MOVE,
SCRIPT_L2C5_MOVE2,
SCRIPT_L2C5_MOVE3,
SCRIPT_L2C5_DEFEND,
SCRIPT_L2C5_DEFEND2,
SCRIPT_L2C6_SETUP,
SCRIPT_L2C6_MOVE,
SCRIPT_L2C6_MOVE2,
SCRIPT_L2C6_MOVE3,
SCRIPT_L2C6_DEFEND,
SCRIPT_L2C6_DEFEND2,
SCRIPT_L2C7_WAIT,
SCRIPT_L2C7_JUMP,
SCRIPT_L2C7_LAUNCH,
SCRIPT_L2C7_LAUNCH2,
SCRIPT_L2C7_LAUNCH3,
SCRIPT_L2C8_WAIT,
SCRIPT_L2C8_JUMP,
SCRIPT_L2C8_MOVE,
SCRIPT_L2C8_MOVE2,
SCRIPT_L2C8_MOVE3,
SCRIPT_L2C8_MOVE4,
SCRIPT_L2C8_MOVE5,

};

enum
{
SCRIPT_L3_EMPTY,
SCRIPT_L3_BASIC,
SCRIPT_L3C0_SETUP,
SCRIPT_L3C0_MOVE,
SCRIPT_L3CO_TURN,
SCRIPT_L3CO_CARRIERS,
SCRIPT_L3C0_ATTACK,
SCRIPT_L3C0_JOIN,
SCRIPT_L3C0_HOLD,
SCRIPT_L3C0_ESCAPE,
SCRIPT_L3C1_WAIT,
SCRIPT_L3C1_JUMP,
SCRIPT_L3C1_MOVE,
SCRIPT_L3C1_HOLD,
SCRIPT_L3C1_ESCAPE,
SCRIPT_L3C2_SETUP,
SCRIPT_L3C2_RUN,
SCRIPT_L3C3_START,
SCRIPT_L3C3_WAIT,
SCRIPT_L3C3_MOVE,
SCRIPT_L3C3_DEFEND,
SCRIPT_L3C3_DEFEND2,
SCRIPT_L3C4_START,
SCRIPT_L3C4_WAIT,
SCRIPT_L3C4_ATTACK,
SCRIPT_L3C4_FIGHT,
SCRIPT_L3C4_ATTACK2,
SCRIPT_L3C4_FIGHT2,
SCRIPT_L3C4_FOLLOW,
SCRIPT_L3C5_WAIT,
SCRIPT_L3C5_JUMP,
SCRIPT_L3C5_ATTACK_C0,
SCRIPT_L3C5_ATTACK_C0_2,
SCRIPT_L3C5_MOVE,
SCRIPT_L3C5_DEFEND,
SCRIPT_L3C7_WAIT,
SCRIPT_L3C7_JUMP,
SCRIPT_L3C7_RUN,
SCRIPT_L3C7_LAUNCH,
SCRIPT_L3C7_MSG,
SCRIPT_L3C7_NOTHING,
SCRIPT_L3C8_WAIT,
SCRIPT_L3C8_JUMP,
SCRIPT_L3C8_MOVE,
SCRIPT_L3C8_ATTACK,
SCRIPT_L3C8_FOLLOW
};


enum
{
STYPE_EMPTY,
STYPE_SETUP_CONVOY, // sets a convoy up using the settings given in the script
STYPE_SPECIAL_CONVOY, // sets a convoy up and calls special_convoy() to place it etc - for randomness etc
STYPE_MOVE_XY,
STYPE_MOVE_FORWARDS, // just moves forwards
STYPE_APPROACH_CONVOY, // convoy heads for another convoy (leads target, sort of). Other convoy must exist
 // when this script starts.
STYPE_APPROACH_CONVOY_OFFSET, // vals are x/y offsets from the target convoy
STYPE_HOLD, // hold course/position: like move_xy but doesn't reset target_x/y. Can change throttle/arrange.
STYPE_START_WAIT, // like wait, but runs from start of mission
STYPE_WAIT, // doesn't do anything by itself, but conditions can do things like spawn other scripts
STYPE_STOP, // sets convoy's throttle to zero, leaves it facing same direction. Can change arrangement.
STYPE_CONVOY_JUMP, // convoy jumps in. team value of script is the jump flag of ships that arrive.
// Remember - jump scripts don't automatically go to the next script like setup_convoy!
STYPE_CONVOY_JUMP_SPECIAL, // like the last one but also calls special_convoy


// remember - when adding movement scripts, can_turn must be 1
};

#define MSG_NONE 0
#define LEVEL_MSGS 25

enum
{
MSG_L1_NONE,
MSG_L1_START1,
MSG_L1_START2,
MSG_L1_START3,
MSG_L1_START4,
MSG_L1_START5,
MSG_L1_START6,
MSG_L1_START7,
MSG_L1_START8,
MSG_L1_ENGAGE,
MSG_L1_EWSHIPS_DESTROYED,
MSG_L1_EFIGHTERS_DESTROYED,
MSG_L1_MISSION_COMPLETE

};

enum
{
MSG_L3_NONE,
MSG_L3K1,
MSG_L3K2,
MSG_L3K3,
MSG_L3K4,
MSG_L3K5,
MSG_L3K6,
MSG_L3K7,
MSG_L3M1,
MSG_L3M2,
MSG_L3M3,
MSG_L3M4,
MSG_L3M5,
MSG_L3M6,
MSG_L3M7,
MSG_L3M8,
MSG_L3M9,
MSG_L3M10,
MSG_L3M11,
MSG_L3M12,
MSG_L3M13,
MSG_L3M14,
MSG_L3M15,
MSG_L3M16,
MSG_L3M17,
};

#define RUNNING_SCRIPTS 16
int running_script [RUNNING_SCRIPTS];
int running_script_time [RUNNING_SCRIPTS];
int running_script_subtime [RUNNING_SCRIPTS];

struct overlevelstruct
{
 struct msgstruct lmsg [LEVEL_MSGS];
 struct levelstruct level_data [LEVEL_SHIPS];
 struct scriptstruct script_data [LEVEL_SCRIPTS];

};

struct overlevelstruct ol [NO_LEVELS] =
{
 {
 },
 {
// S1M
  {
   {""},
   {"This is $BAlpha 1$C  to $BAlpha group$C. We're picking up a large number of hostile ships. ", {WAV_SELECT0, NOTE_2G}}, // MSG_L1_START1
   {"How many of them are there? ", {WAV_SELECT0, NOTE_2A}}, // MSG_L1_START2
   {"Our scanners count nine... no, ten warships. At least twice that many fighters. ", {WAV_SELECT0, NOTE_2G}}, // MSG_L1_START3
   {"Ten?? We won't even slow them down. Request permission for evasive manoeuvres. ", {WAV_SELECT0, NOTE_2A}}, // MSG_L1_START4
   {"Negative. Our orders are clear, $BAlpha 2$C. We can't let these ships through. Hold course and prepare to engage. ", {WAV_SELECT0, NOTE_2G}}, // MSG_L1_START5
   {"The enemy convoy is spread out over a wide area; they must have come in on a bad jump pattern. ", {WAV_SELECT0, NOTE_2G}}, // MSG_L1_START6
   {" $BAngry Moth$C, fly out and do as much damage as you can before they're able to regroup. ", {WAV_SELECT1, NOTE_2G}}, // MSG_L1_START7
   {"Concentrate on the ships with heavy weaponry. Ships with anti-fighter weapons are low priority. ", {WAV_SELECT1, NOTE_2G}}, // MSG_L1_START8
// MSG_L1_CHANGE
   {" $BAlpha group$C, prepare to engage enemy at close range. ", {WAV_SELECT0, NOTE_2G}},
   {"All enemy warships destroyed! Now we just need to clean up the remaining fighters. ", {WAV_SELECT0, NOTE_2G}},
   {"All enemy fighters destroyed. Concentrate fire on the remaining warships. ", {WAV_SELECT0, NOTE_2G}},
   {"All enemy ships destroyed! Prepare to return to base. ", {WAV_SELECT1, NOTE_3C}},
  },
  {
// group position values must not be duplicated but can appear in any order
// except for group leader (0), which must be first.
// convoy value must be -1 for fighters.
//  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 0, 0, 0,0,
//   0, {0, 0, 0}, {0, 0, 0}},

// S1L
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 1, 0, 0,0,
   CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS_2,
    {{0, 0}, {-ANGLE_8, 120}}},
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 2, 0, 0,0,
   CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS_2,
    {{-ANGLE_4-ANGLE_8, 150}, {-ANGLE_4-ANGLE_8, 120}}},
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 3, 0, 0,0,
   CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS_2,
    {{ANGLE_4+ANGLE_8, 150}, {ANGLE_8, 120}}},
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS_2,
    {{ANGLE_2, 180}, {ANGLE_4 + ANGLE_8, 120}}},

  {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 1, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 1, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},

  {TEAM_ENEMY, SHIP_SCOUT3, VAR_NONE, 0, ACT_NONE, 5, 0, 0,0,
   CONVOY_1, FLAGS, FLAGS_2,
    {{0, 0}, {0, 0}}},

  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 6, 0, 0,0,
   CONVOY_2, FLAGS, FLAGS_2,
    {{ANGLE_4 + ANGLE_16, 250}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 7, 0, 0,0,
   CONVOY_3, FLAGS, FLAGS_2,
    {{ANGLE_2, 200}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_LONG, 0, ACT_NONE, 8, 0, 0,0,
   CONVOY_4, FLAGS, FLAGS_2,
    {{-ANGLE_4-ANGLE_16, 250}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 9, 0, 0,0,
   CONVOY_5, FLAGS, FLAGS_2,
    {{ANGLE_2, 400}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 10, 0, 0,0,
   CONVOY_6, FLAGS, FLAGS_2,
    {{-ANGLE_4 - ANGLE_8, 400}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 11, 0, 0,0,
   CONVOY_7, FLAGS, FLAGS_2,
    {{ANGLE_2 - ANGLE_16, 400}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 12, 0, 0,0,
   CONVOY_8, FLAGS, FLAGS_2,
    {{ANGLE_2, 600}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_LONG, 0, ACT_NONE, 13, 0, 0,0,
   CONVOY_9, FLAGS, FLAGS_2,
    {{ANGLE_2 - ANGLE_16, 500}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 14, 0, 0,0,
   CONVOY_10, FLAGS, FLAGS_2,
    {{ANGLE_2 + ANGLE_16, 500}, {0, 0}}},


  {TEAM_ENEMY, SHIP_EINT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 5, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_EINT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 5, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_EINT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 5, 3, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_EINT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 5, 4, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_EINT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 5, 5, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_EINT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 5, 6, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},

  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 6, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 6, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 7, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 7, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 8, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 8, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 9, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 9, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 10, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 10, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 11, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 11, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 12, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 12, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 13, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 13, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 14, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 14, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},

  {TEAM_NONE}


// THINK ABOUT: making PREF_FIGHTER (e.g. EANTI) turrets FIGHTER_ONLY if there is a WSHIP_ONLY turret on
//  the same ship.
// - also: for escorting or guarding fighters, randomising the distance that they'll follow something each time
//    they start following.
 },

 {
  {

  },
// S1S
  {TEAM_FRIEND, CONVOY_0, // SCRIPT_L1C0_START
   STYPE_SETUP_CONVOY, -3500, 0, 2, 0, {0},
    {{PCON_SETUP_CONVOY, {19500, 0, 10}, RESULT_NEXT_SCRIPT, SCRIPT_L1C0_MOVE}, // setup scripts can't have messages
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_FRIEND, CONVOY_0, // SCRIPT_L1C0_MOVE
   STYPE_MOVE_XY, 19500, 0, 17, 0, {0},
//    {{PCON_LEVEL_TIME, {4}, RESULT_CONVOY_JUMP_OUT},
   {{PCON_CONVOY_DIST, {CONVOY_1, 700}, RESULT_NEXT_SCRIPT, SCRIPT_L1C0_MOVE2, MSG_L1_ENGAGE, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },

  {TEAM_FRIEND, CONVOY_0, // SCRIPT_L1C0_MOVE2
   STYPE_MOVE_XY, 19500, 0, 17, 1, {0},
   {{PCON_CONVOY_DIST, {CONVOY_1, 50}, RESULT_NEXT_SCRIPT, SCRIPT_L1C0_MOVE3, MSG_NONE},
    {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },

  {TEAM_FRIEND, CONVOY_0, // SCRIPT_L1C0_MOVE3
   STYPE_MOVE_XY, 19500, 0, 9, 1, {0}, // note lower throttle!
   {{PCON_NONE},
    },
   {{CCON_NONE},
    }
   },

  {TEAM_FRIEND, CONVOY_NONE, // SCRIPT_L1C0_MESSAGE1
   STYPE_START_WAIT, 19500, 0, 17, 0, {0},
    {{PCON_LEVEL_TIME, {2}, RESULT_NONE, 0, MSG_L1_START1, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {PCON_LEVEL_TIME, {7}, RESULT_NONE, 0, MSG_L1_START2, MSG_FROM_COMMAND_1, 2, MSG_TO_ALL},
     {PCON_LEVEL_TIME, {13}, RESULT_NONE, 0, MSG_L1_START3, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {PCON_LEVEL_TIME, {20}, RESULT_NONE, 0, MSG_L1_START4, MSG_FROM_COMMAND_1, 2, MSG_TO_ALL},
     {PCON_LEVEL_TIME, {26}, RESULT_NEXT_SCRIPT, SCRIPT_L1C0_MESSAGE2, MSG_L1_START5, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL}
    },
    {{CCON_NONE},
    }
   },

  {TEAM_FRIEND, CONVOY_NONE, // SCRIPT_L1C0_MESSAGE2
   STYPE_WAIT, 19500, 0, 17, 0, {0},
    {{PCON_LEVEL_TIME, {34}, RESULT_NONE, 0, MSG_L1_START6, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {PCON_LEVEL_TIME, {40}, RESULT_NONE, 0, MSG_L1_START7, MSG_FROM_COMMAND_1, 1, MSG_TO_AM},
     {PCON_LEVEL_TIME, {47}, RESULT_NONE, 0, MSG_L1_START8, MSG_FROM_COMMAND_1, 1, MSG_TO_AM},
     {PCON_NONE},
    },

   {{CCON_EWSHIPS_DESTROYED_FREMAIN, {0}, RESULT_NONE, 0, MSG_L1_EWSHIPS_DESTROYED, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {CCON_EFIGHTERS_DESTROYED_REMAIN, {0}, RESULT_NONE, 0, MSG_L1_EFIGHTERS_DESTROYED, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {CCON_EALL_DESTROYED, {0}, RESULT_MISSION_OVER, 0, MSG_L1_MISSION_COMPLETE, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {CCON_FWSHIPS_DESTROYED, {0}, RESULT_MISSION_FAIL, MSG_NONE},
    {CCON_NONE},
    }
   },

/*
  {TEAM_ENEMY, CONVOY_1, // SCRIPT_L1C1_START
   STYPE_SETUP_CONVOY, -5500, 0, 13, 0,
    {{PCON_SETUP_CONVOY, {5500, 0, 180}, RESULT_NEXT_SCRIPT, SCRIPT_L1C1_MOVE},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },*/

  {TEAM_ENEMY, CONVOY_1, // SCRIPT_L1C1_START
   STYPE_SETUP_CONVOY, 2000, 0, 9, 0, {0},
//   STYPE_SETUP_CONVOY, -4100, 100, 9, 0,
    {{PCON_SETUP_CONVOY, {5500, 0, 180}, RESULT_NEXT_SCRIPT, SCRIPT_L1C1_MOVE},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_1, // SCRIPT_L1C1_MOVE
   STYPE_MOVE_XY, 19500, 0, 9, 0, {0},
    {{PCON_NONE},
    },
    {{CCON_NONE},
    }
   },

  {TEAM_ENEMY, CONVOY_2, // SCRIPT_L1C1_START
   STYPE_SPECIAL_CONVOY, 500, 0, 15, 0, {0},
    {{PCON_SETUP_CONVOY, {1500, 0, 200}, RESULT_NEXT_SCRIPT, SCRIPT_L1C2_MOVE},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_2, // SCRIPT_L1C1_MOVE
   STYPE_APPROACH_CONVOY, CONVOY_1, 0, 15, 0, {0},
    {{PCON_CONVOY_DIST, {CONVOY_1, 100}, RESULT_JOIN_CONVOY, CONVOY_1},
    },
    {{CCON_NONE},
    }
   },

  {TEAM_ENEMY, CONVOY_3, // SCRIPT_L1C1_START
   STYPE_SPECIAL_CONVOY, 200, 0, 15, 0, {0},
    {{PCON_SETUP_CONVOY, {1500, 0, 200}, RESULT_NEXT_SCRIPT, SCRIPT_L1C3_MOVE},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_3, // SCRIPT_L1C1_MOVE
   STYPE_APPROACH_CONVOY, CONVOY_1, 0, 15, 0, {0},
    {{PCON_CONVOY_DIST, {CONVOY_1, 100}, RESULT_JOIN_CONVOY, CONVOY_1},
    },
    {{CCON_NONE},
    }
   },

  {TEAM_ENEMY, CONVOY_4, // SCRIPT_L1C1_START
   STYPE_SPECIAL_CONVOY, -100, 0, 15, 0, {0},
    {{PCON_SETUP_CONVOY, {1500, 0, 200}, RESULT_NEXT_SCRIPT, SCRIPT_L1C4_MOVE},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_4, // SCRIPT_L1C1_MOVE
   STYPE_APPROACH_CONVOY, CONVOY_1, 0, 15, 0, {0},
    {{PCON_CONVOY_DIST, {CONVOY_1, 100}, RESULT_JOIN_CONVOY, CONVOY_1},
    },
    {{CCON_NONE},
    }
   },

  {TEAM_ENEMY, CONVOY_5, // SCRIPT_L1C1_START
   STYPE_SPECIAL_CONVOY, -400, 0, 15, 0, {0},
    {{PCON_SETUP_CONVOY, {2500, 0, 200}, RESULT_NEXT_SCRIPT, SCRIPT_L1C5_MOVE},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_5, // SCRIPT_L1C1_MOVE
   STYPE_APPROACH_CONVOY, CONVOY_1, 0, 15, 0, {0},
    {{PCON_CONVOY_DIST, {CONVOY_1, 100}, RESULT_JOIN_CONVOY, CONVOY_1},
    },
    {{CCON_NONE},
    }
   },

  {TEAM_ENEMY, CONVOY_6, // SCRIPT_L1C1_START
   STYPE_SPECIAL_CONVOY, -700, 00, 15, 0, {0},
    {{PCON_SETUP_CONVOY, {2500, 0, 200}, RESULT_NEXT_SCRIPT, SCRIPT_L1C6_MOVE},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_6, // SCRIPT_L1C1_MOVE
   STYPE_APPROACH_CONVOY, CONVOY_1, 0, 15, 0, {0},
    {{PCON_CONVOY_DIST, {CONVOY_1, 100}, RESULT_JOIN_CONVOY, CONVOY_1},
    },
    {{CCON_NONE},
    }
   },

  {TEAM_ENEMY, CONVOY_7, // SCRIPT_L1C1_START
   STYPE_SPECIAL_CONVOY, -1000, 0, 15, 0, {0},
    {{PCON_SETUP_CONVOY, {2500, 0, 200}, RESULT_NEXT_SCRIPT, SCRIPT_L1C7_MOVE},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_7, // SCRIPT_L1C1_MOVE
   STYPE_APPROACH_CONVOY, CONVOY_1, 0, 15, 0, {0},
    {{PCON_CONVOY_DIST, {CONVOY_1, 100}, RESULT_JOIN_CONVOY, CONVOY_1},
    },
    {{CCON_NONE},
    }
   },

  {TEAM_ENEMY, CONVOY_8, // SCRIPT_L1C1_START
   STYPE_SPECIAL_CONVOY, -1300, 0, 15, 0, {0},
    {{PCON_SETUP_CONVOY, {2500, 0, 200}, RESULT_NEXT_SCRIPT, SCRIPT_L1C8_MOVE},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_8, // SCRIPT_L1C1_MOVE
   STYPE_APPROACH_CONVOY, CONVOY_1, 0, 15, 0, {0},
    {{PCON_CONVOY_DIST, {CONVOY_1, 200}, RESULT_JOIN_CONVOY, CONVOY_1},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },


  {TEAM_ENEMY, CONVOY_9, // SCRIPT_L1C1_START
   STYPE_SPECIAL_CONVOY, -1600, 0, 15, 0, {0},
    {{PCON_SETUP_CONVOY, {2500, 0, 200}, RESULT_NEXT_SCRIPT, SCRIPT_L1C9_MOVE},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_9, // SCRIPT_L1C1_MOVE
   STYPE_APPROACH_CONVOY, CONVOY_1, 0, 15, 0, {0},
    {{PCON_CONVOY_DIST, {CONVOY_1, 200}, RESULT_JOIN_CONVOY, CONVOY_1},
    },
    {{CCON_NONE},
    }
   },

  {TEAM_ENEMY, CONVOY_10, // SCRIPT_L1C1_START
   STYPE_SPECIAL_CONVOY, -1900, 0, 15, 0, {0},
    {{PCON_SETUP_CONVOY, {2500, 0, 200}, RESULT_NEXT_SCRIPT, SCRIPT_L1C10_MOVE},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_10, // SCRIPT_L1C1_MOVE
   STYPE_APPROACH_CONVOY, CONVOY_1, 0, 15, 0, {0},
    {{PCON_CONVOY_DIST, {CONVOY_1, 200}, RESULT_JOIN_CONVOY, CONVOY_1},
    },
    {{CCON_NONE},
    }
   },

  },

 }, // end stage 1


 { // Stage 2!
// S2M
  {
   {""},
//   {"This is $BAlpha 1$C  to $BAlpha group$C. We're picking up a large number of hostile ships. ", {wav, freq}}, // MSG_L1_START1
  },

/*
************************************************************************************************

 LEVEL 2!!!

************************************************************************************************

Convoys:
 Friendly:
  0 - bottom left
  1 - bottom right
  2 - top

Each convoy consists of 1 BC and 5 LCs. BC and rearmost LC each have 2 fighter escorts.

 Enemy:
  3 - base
  4 - loose defensive convoy
  5 - loose defensive convoy
  6 - loose defensive convoy
  7 - carriers - jump in after a few minutes, to a random location
  8 - heavy - jump in later, to a random location, then approach base - possibly swirl around it?

- the defensive convoys surround the base at a different distance on each side.
- pull together to attack friendly convoys, each encounter at a slightly different time


*/

// FIRST friendly convoy
// S2L
 {
  {TEAM_FRIEND, SHIP_OLD3, VAR_NONE, 0, ACT_NONE, 1, 0, 0,0,
   CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS_2,
    {{0, 0}, {0, 0}}},
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS_2,
    {{-ANGLE_8, 150}, {-ANGLE_4, 150}}},
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS_2,
    {{ANGLE_8, 150}, {ANGLE_4, 150}}},
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS_2,
    {{-ANGLE_4-ANGLE_16, 180}, {-ANGLE_4 + ANGLE_16, 250}}},
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS_2,
    {{ANGLE_4+ANGLE_16, 180}, {ANGLE_4-ANGLE_16, 250}}},
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 2, 0, 0,0,
   CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS_2,
    {{ANGLE_2, 170}, {ANGLE_2, 150}}},

  {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 1, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 1, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 2, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 2, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},

// SECOND friendly convoy

  {TEAM_FRIEND, SHIP_OLD3, VAR_NONE, 0, ACT_NONE, 3, 0, 0,0,
   CONVOY_1, FLAG_COMMAND_2|FLAG_BETA|FLAG_SPAWN_2, FLAGS_2,
    {{0, 0}, {0, 0}}},
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAG_COMMAND_2|FLAG_BETA|FLAG_SPAWN_2, FLAGS_2,
    {{-ANGLE_8, 150}, {-ANGLE_4, 150}}},
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAG_COMMAND_2|FLAG_BETA|FLAG_SPAWN_2, FLAGS_2,
    {{ANGLE_8, 150}, {ANGLE_4, 150}}},
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAG_COMMAND_2|FLAG_BETA|FLAG_SPAWN_2, FLAGS_2,
    {{-ANGLE_4-ANGLE_8, 180}, {-ANGLE_4 + ANGLE_16, 250}}},
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAG_COMMAND_2|FLAG_BETA|FLAG_SPAWN_2, FLAGS_2,
    {{ANGLE_4+ANGLE_8, 180}, {ANGLE_4-ANGLE_16, 250}}},
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 4, 0, 0,0,
   CONVOY_1, FLAG_COMMAND_2|FLAG_BETA|FLAG_SPAWN_2, FLAGS_2,
    {{ANGLE_2, 170}, {ANGLE_2, 150}}},

  {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 3, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 3, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 4, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 4, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},

// THIRD friendly convoy

  {TEAM_FRIEND, SHIP_OLD3, VAR_NONE, 0, ACT_NONE, 5, 0, 0,0,
   CONVOY_2, FLAG_COMMAND_3|FLAG_GAMMA|FLAG_SPAWN_2, FLAGS_2,
    {{0, 0}, {0, 0}}},
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_2, FLAG_COMMAND_3|FLAG_GAMMA|FLAG_SPAWN_2, FLAGS_2,
    {{-ANGLE_8, 150}, {-ANGLE_4, 150}}},
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_2, FLAG_COMMAND_3|FLAG_GAMMA|FLAG_SPAWN_2, FLAGS_2,
    {{ANGLE_8, 150}, {ANGLE_4, 150}}},
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_2, FLAG_COMMAND_3|FLAG_GAMMA|FLAG_SPAWN_2, FLAGS_2,
    {{-ANGLE_4-ANGLE_8, 180}, {-ANGLE_4 + ANGLE_16, 250}}},
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_2, FLAG_COMMAND_3|FLAG_GAMMA|FLAG_SPAWN_2, FLAGS_2,
    {{ANGLE_4+ANGLE_8, 180}, {ANGLE_4-ANGLE_16, 250}}},
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 6, 0, 0,0,
   CONVOY_2, FLAG_COMMAND_3|FLAG_GAMMA|FLAG_SPAWN_2, FLAGS_2,
    {{ANGLE_2, 170}, {ANGLE_2, 150}}},

  {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 5, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 5, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 6, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 6, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},

// ENEMY BASE

  {TEAM_ENEMY, SHIP_EBASE, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_3, FLAGS, FLAGS_2,
    {{0, 0}, {0, 0}}},

// Convoy 4 - defends the base, present from start
  {TEAM_ENEMY, SHIP_SCOUT3, VAR_NONE, 0, ACT_NONE, 7, 0, 0,0,
   CONVOY_4, FLAGS, FLAGS_2,
    {{ANGLE_4, 250}, {ANGLE_8, 350}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_4, FLAGS, FLAGS_2,
    {{ANGLE_4 + ANGLE_32, 750}, {ANGLE_4, 300}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_4, FLAGS, FLAGS_2,
    {{ANGLE_4 + ANGLE_16, 1250}, {ANGLE_4 + ANGLE_8, 350}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 8, 0, 0,0,
   CONVOY_4, FLAGS, FLAGS_2,
    {{-ANGLE_4, 250}, {-ANGLE_8, 350}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_4, FLAGS, FLAGS_2,
    {{-ANGLE_4 - ANGLE_32, 750}, {-ANGLE_4, 300}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_4, FLAGS, FLAGS_2,
    {{-ANGLE_4 - ANGLE_16, 1250}, {-ANGLE_4 - ANGLE_8, 350}}},

  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 7, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 7, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 8, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 8, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},


// Convoy 5 - defends the base, present from start
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 9, 0, 0,0,
   CONVOY_5, FLAGS, FLAGS_2,
    {{ANGLE_4, 250}, {ANGLE_8, 350}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_5, FLAGS, FLAGS_2,
    {{ANGLE_4 + ANGLE_32, 750}, {ANGLE_4, 300}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_5, FLAGS, FLAGS_2,
    {{ANGLE_4 + ANGLE_16, 1250}, {ANGLE_4 + ANGLE_8, 350}}},
  {TEAM_ENEMY, SHIP_SCOUT3, VAR_NONE, 0, ACT_NONE, 10, 0, 0,0,
   CONVOY_5, FLAGS, FLAGS_2,
    {{-ANGLE_4, 250}, {-ANGLE_8, 350}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_5, FLAGS, FLAGS_2,
    {{-ANGLE_4 - ANGLE_32, 750}, {-ANGLE_4, 300}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_5, FLAGS, FLAGS_2,
    {{-ANGLE_4 - ANGLE_16, 1250}, {-ANGLE_4 - ANGLE_8, 350}}},

  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 9, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 9, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 10, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 10, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},

// Convoy 6 - defends the base, present from start
  {TEAM_ENEMY, SHIP_SCOUT3, VAR_NONE, 0, ACT_NONE, 11, 0, 0,0,
   CONVOY_6, FLAGS, FLAGS_2,
    {{ANGLE_4, 250}, {ANGLE_8, 350}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_6, FLAGS, FLAGS_2,
    {{ANGLE_4 + ANGLE_32, 750}, {ANGLE_4, 300}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_6, FLAGS, FLAGS_2,
    {{ANGLE_4 + ANGLE_16, 1250}, {ANGLE_4 + ANGLE_8, 350}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 12, 0, 0,0,
   CONVOY_6, FLAGS, FLAGS_2,
    {{-ANGLE_4, 250}, {-ANGLE_8, 350}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_6, FLAGS, FLAGS_2,
    {{-ANGLE_4 - ANGLE_32, 750}, {-ANGLE_4, 300}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_6, FLAGS, FLAGS_2,
    {{-ANGLE_4 - ANGLE_16, 125}, {-ANGLE_4 - ANGLE_8, 350}}},

  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 11, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 11, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 12, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 12, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},

// Carrier convoy 7 - jumps in
  {TEAM_ENEMY, SHIP_SCOUTCAR, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_7, FLAG_JUMP_1, FLAGS_2,
    {{0, 200}}},
  {TEAM_ENEMY, SHIP_SCOUTCAR, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_7, FLAG_JUMP_1, FLAGS_2,
    {{ANGLE_3, 200}}},
  {TEAM_ENEMY, SHIP_SCOUTCAR, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_7, FLAG_JUMP_1, FLAGS_2,
    {{-ANGLE_3, 200}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 13, 0, 0,0,
   CONVOY_7, FLAG_JUMP_1, FLAGS_2,
    {{ANGLE_6, 300}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 14, 0, 0,0,
   CONVOY_7, FLAG_JUMP_1, FLAGS_2,
    {{-ANGLE_6, 300}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 15, 0, 0,0,
   CONVOY_7, FLAG_JUMP_1, FLAGS_2,
    {{ANGLE_2, 300}}},

  {TEAM_ENEMY, SHIP_ESCOUT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 13, 1, 0,0,
   CONVOY_NONE, FLAG_JUMP_1, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_ESCOUT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 13, 2, 0,0,
   CONVOY_NONE, FLAG_JUMP_1, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_ESCOUT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 14, 1, 0,0,
   CONVOY_NONE, FLAG_JUMP_1, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_ESCOUT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 14, 2, 0,0,
   CONVOY_NONE, FLAG_JUMP_1, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_ESCOUT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 15, 1, 0,0,
   CONVOY_NONE, FLAG_JUMP_1, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_ESCOUT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 15, 2, 0,0,
   CONVOY_NONE, FLAG_JUMP_1, FLAGS_2, {{0,0}}},

// Convoy 8 - second set of reinforcements - jumps in

  {TEAM_ENEMY, SHIP_SCOUT3, VAR_NONE, 0, ACT_NONE, 16, 0, 0,0,
   CONVOY_8, FLAG_JUMP_2, FLAGS_2,
    {{ANGLE_8, 120}}},
  {TEAM_ENEMY, SHIP_SCOUT3, VAR_NONE, 0, ACT_NONE, 17, 0, 0,0,
   CONVOY_8, FLAG_JUMP_2, FLAGS_2,
    {{-ANGLE_8, 120}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_8, FLAG_JUMP_2, FLAGS_2,
    {{ANGLE_4 + ANGLE_8, 120}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_LONG, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_8, FLAG_JUMP_2, FLAGS_2,
    {{-ANGLE_4 - ANGLE_8, 120}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_8, FLAG_JUMP_2, FLAGS_2,
    {{ANGLE_4, 160}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_8, FLAG_JUMP_2, FLAGS_2,
    {{-ANGLE_4, 160}}},

  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 16, 1, 0,0,
   CONVOY_NONE, FLAG_JUMP_2, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 16, 2, 0,0,
   CONVOY_NONE, FLAG_JUMP_2, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 17, 1, 0,0,
   CONVOY_NONE, FLAG_JUMP_2, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 17, 2, 0,0,
   CONVOY_NONE, FLAG_JUMP_2, FLAGS_2, {{0,0}}},

 },

 {
// S2S
  {
  },

  {TEAM_FRIEND, CONVOY_NONE, // SCRIPT_L2_BASIC
   STYPE_START_WAIT, 0, 0, 0, 0, {0},
    {{PCON_NONE},
    },
    {{CCON_FWSHIPS_DESTROYED, {0}, RESULT_MISSION_FAIL, MSG_NONE},
    {CCON_NONE},
    }
   },


  {TEAM_FRIEND, CONVOY_0, // SCRIPT_L2C0_SETUP
   STYPE_SETUP_CONVOY, -8500, 6300, 0, 0, {0}, // 12200 distance
//   STYPE_SETUP_CONVOY, -2000, 2000, 0, 0,
    {{PCON_SETUP_CONVOY, {0, 0, 10}, RESULT_NEXT_SCRIPT, SCRIPT_L2C0_MOVE}, // setup scripts can't have messages
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_FRIEND, CONVOY_0, // SCRIPT_L2C0_MOVE
   STYPE_MOVE_XY, 0, 0, 12, 0, {0},
   {{PCON_CONVOY_DIST, {CONVOY_3, 1500}, RESULT_NEXT_SCRIPT, SCRIPT_L2C0_MOVE2},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_FRIEND, CONVOY_0, // SCRIPT_L2C0_MOVE2
   STYPE_MOVE_XY, 0, 0, 6, 1, {0},
   {{PCON_CONVOY_DIST, {CONVOY_3, 480}, RESULT_NEXT_SCRIPT, SCRIPT_L2C0_ATTACK},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_FRIEND, CONVOY_0, // SCRIPT_L2C0_ATTACK
   STYPE_STOP, 0, 0, 0, 1, {0},
   {{PCON_NONE},
    },
   {{CCON_CONVOY_DESTROYED, {CONVOY_3}, RESULT_NEXT_SCRIPT, SCRIPT_L2C0_ESCAPE},
    {CCON_NONE},
    }
   },
  {TEAM_FRIEND, CONVOY_0, // SCRIPT_L2C0_ESCAPE
   STYPE_MOVE_XY, 4000, -1000, 12, 0, {0},
   {{PCON_XY, {4000, -1000}, RESULT_MISSION_OVER},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },

  {TEAM_FRIEND, CONVOY_1, // SCRIPT_L2C1_SETUP
   STYPE_SETUP_CONVOY, 2000, -10000, 0, 0, {0}, // 11000
//   STYPE_SETUP_CONVOY, 2000, -2000, 0, 0,
    {{PCON_SETUP_CONVOY, {0, 0, 10}, RESULT_NEXT_SCRIPT, SCRIPT_L2C1_MOVE}, // setup scripts can't have messages
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_FRIEND, CONVOY_1, // SCRIPT_L2C1_MOVE
   STYPE_MOVE_XY, 0, 0, 12, 0, {0},
   {{PCON_CONVOY_DIST, {CONVOY_3, 1500}, RESULT_NEXT_SCRIPT, SCRIPT_L2C1_MOVE2},
     {PCON_NONE},
    },
    {{CCON_NONE},
    },
   },
  {TEAM_FRIEND, CONVOY_1, // SCRIPT_L2C1_MOVE2
   STYPE_MOVE_XY, 0, 0, 6, 1, {0},
   {{PCON_CONVOY_DIST, {CONVOY_3, 430}, RESULT_NEXT_SCRIPT, SCRIPT_L2C1_ATTACK},
     {PCON_NONE},
    },
    {{CCON_NONE},
    },
   },
  {TEAM_FRIEND, CONVOY_1, // SCRIPT_L2C1_ATTACK
   STYPE_STOP, 0, 0, 0, 1, {0},
   {{CCON_CONVOY_DESTROYED, {CONVOY_3}, RESULT_NEXT_SCRIPT, SCRIPT_L2C1_ESCAPE},
    },
    {{CCON_NONE},
    },
   },
  {TEAM_FRIEND, CONVOY_1, // SCRIPT_L2C1_ESCAPE
   STYPE_MOVE_XY, 4000, -1500, 12, 0, {0},
   {{PCON_XY, {4000, -1500}, RESULT_MISSION_OVER},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },

  {TEAM_FRIEND, CONVOY_2, // SCRIPT_L2C2_SETUP
   STYPE_SETUP_CONVOY, 8500, 7500, 0, 0, {0}, // 10600
//   STYPE_SETUP_CONVOY, 2000, 2000, 0, 0,
    {{PCON_SETUP_CONVOY, {0, 0, 10}, RESULT_NEXT_SCRIPT, SCRIPT_L2C2_MOVE}, // setup scripts can't have messages
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_FRIEND, CONVOY_2, // SCRIPT_L2C2_MOVE
   STYPE_MOVE_XY, 0, 0, 12, 0, {0},
   {{PCON_CONVOY_DIST, {CONVOY_3, 1500}, RESULT_NEXT_SCRIPT, SCRIPT_L2C2_MOVE2},
     {PCON_NONE},
    },
    {{CCON_NONE},
    },
  },
  {TEAM_FRIEND, CONVOY_2, // SCRIPT_L2C2_MOVE2
   STYPE_MOVE_XY, 0, 0, 6, 1, {0},
   {{PCON_CONVOY_DIST, {CONVOY_3, 480}, RESULT_NEXT_SCRIPT, SCRIPT_L2C2_ATTACK},
     {PCON_NONE},
    },
    {{CCON_NONE},
    },
  },
  {TEAM_FRIEND, CONVOY_2, // SCRIPT_L2C1_ATTACK
   STYPE_STOP, 0, 0, 0, 1, {0},
   {{CCON_CONVOY_DESTROYED, {CONVOY_3}, RESULT_NEXT_SCRIPT, SCRIPT_L2C2_ESCAPE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_FRIEND, CONVOY_2, // SCRIPT_L2C2_ESCAPE
   STYPE_MOVE_XY, 4000, -500, 12, 0, {0},
   {{PCON_XY, {4000, -500}, RESULT_MISSION_OVER},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },


  {TEAM_ENEMY, CONVOY_3, // SCRIPT_L2C3_SETUP
   STYPE_SETUP_CONVOY, 0, 0, 0, 0, {0},
    {{PCON_SETUP_CONVOY, {0, 0, 0}, RESULT_NEXT_SCRIPT, SCRIPT_L2C3_RUN},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_3, // SCRIPT_L2C3_RUN
   STYPE_STOP, 0, 0, 0, 0, {0},
   {{PCON_NONE},
    },
    {{CCON_NONE},
    }
   },



  {TEAM_ENEMY, CONVOY_4, // SCRIPT_L2C4_START
   STYPE_SPECIAL_CONVOY, 0, 0, 0, 0, {0},
    {{PCON_SETUP_CONVOY, {0, 0, 0}, RESULT_NEXT_SCRIPT, SCRIPT_L2C4_MOVE},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_4, // SCRIPT_L2C4_MOVE
   STYPE_HOLD, 0, 0, 0, 0, {0},
    {{PCON_LEVEL_TIME, {10}, RESULT_CHANGE_ARRANGE, 1},
     {PCON_LEVEL_TIME_SPECIAL, {S2SCON_C4WAIT}, RESULT_NEXT_SCRIPT, SCRIPT_L2C4_MOVE2},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_4, // SCRIPT_L2C4_MOVE2
   STYPE_APPROACH_CONVOY, CONVOY_1, 0, 12, 1, {0},
   {{PCON_CONVOY_DIST, {CONVOY_1, 700}, RESULT_NEXT_SCRIPT, SCRIPT_L2C4_MOVE3},
    {PCON_NONE},
    },
    {{CCON_FCONVOY_DESTROYED_REMAIN, {CONVOY_1}, RESULT_NEXT_SCRIPT, SCRIPT_L2C4_DEFEND},
     {CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_4, // SCRIPT_L2C4_MOVE3
   STYPE_HOLD, 0, 0, 12, 1, {0},
   {{PCON_CONVOY_DIST_APART, {CONVOY_1, 800}, RESULT_NEXT_SCRIPT, SCRIPT_L2C4_MOVE2}, // cycle back
    {PCON_NONE},
    },
   {{CCON_FCONVOY_DESTROYED_REMAIN, {CONVOY_1}, RESULT_NEXT_SCRIPT, SCRIPT_L2C4_DEFEND},
     {CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_4, // SCRIPT_L2C4_DEFEND
   STYPE_MOVE_XY, 500, -300, 12, 1, {0},
   {{PCON_XY, {500, -300}, RESULT_NEXT_SCRIPT, SCRIPT_L2C4_DEFEND2},
    {PCON_NONE},
    },
   {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_4, // SCRIPT_L2C4_DEFEND2
   STYPE_STOP, 0, 0, 0, 1, {0},
   {{PCON_NONE},
   },
   {{CCON_NONE},
   }
   },

  {TEAM_ENEMY, CONVOY_5, // SCRIPT_L2C5_START
   STYPE_SPECIAL_CONVOY, 0, 0, 0, 0, {0},
    {{PCON_SETUP_CONVOY, {0, 0, 0}, RESULT_NEXT_SCRIPT, SCRIPT_L2C5_MOVE},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_5, // SCRIPT_L2C5_MOVE
   STYPE_HOLD, 0, 0, 0, 0, {0},
    {{PCON_LEVEL_TIME, {10}, RESULT_CHANGE_ARRANGE, 1},
     {PCON_LEVEL_TIME_SPECIAL, {S2SCON_C5WAIT}, RESULT_NEXT_SCRIPT, SCRIPT_L2C5_MOVE2},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_5, // SCRIPT_L2C5_MOVE2
   STYPE_APPROACH_CONVOY, CONVOY_2, 0, 12, 1, {0},
   {{PCON_CONVOY_DIST, {CONVOY_2, 700}, RESULT_NEXT_SCRIPT, SCRIPT_L2C5_MOVE3},
    {PCON_NONE},
    },
    {{CCON_FCONVOY_DESTROYED_REMAIN, {CONVOY_2}, RESULT_NEXT_SCRIPT, SCRIPT_L2C5_DEFEND},
     {CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_5, // SCRIPT_L2C5_MOVE3
   STYPE_HOLD, 0, 0, 12, 1, {0},
   {{PCON_CONVOY_DIST_APART, {CONVOY_2, 800}, RESULT_NEXT_SCRIPT, SCRIPT_L2C5_MOVE2}, // cycle back
    {PCON_NONE},
    },
   {{CCON_FCONVOY_DESTROYED_REMAIN, {CONVOY_2}, RESULT_NEXT_SCRIPT, SCRIPT_L2C5_DEFEND},
     {CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_5, // SCRIPT_L2C5_DEFEND
   STYPE_MOVE_XY, 400, 250, 12, 1, {0},
   {{PCON_XY, {400, 250}, RESULT_NEXT_SCRIPT, SCRIPT_L2C5_DEFEND2},
    {PCON_NONE},
    },
   {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_5, // SCRIPT_L2C5_DEFEND2
   STYPE_STOP, 0, 0, 0, 1, {0},
   {{PCON_NONE},
   },
   {{CCON_NONE},
   }
   },




  {TEAM_ENEMY, CONVOY_6, // SCRIPT_L2C6_START
   STYPE_SPECIAL_CONVOY, 0, 0, 12, 0, {0},
    {{PCON_SETUP_CONVOY, {0, 0, 0}, RESULT_NEXT_SCRIPT, SCRIPT_L2C6_MOVE},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_6, // SCRIPT_L2C6_MOVE
   STYPE_HOLD, 0, 0, 0, 0, {0},
    {{PCON_LEVEL_TIME, {10}, RESULT_CHANGE_ARRANGE, 1},
     {PCON_LEVEL_TIME_SPECIAL, {S2SCON_C6WAIT}, RESULT_NEXT_SCRIPT, SCRIPT_L2C6_MOVE2},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_6, // SCRIPT_L2C6_MOVE2
   STYPE_APPROACH_CONVOY, CONVOY_0, 0, 12, 1, {0},
   {{PCON_CONVOY_DIST, {CONVOY_0, 700}, RESULT_NEXT_SCRIPT, SCRIPT_L2C6_MOVE3},
    {PCON_NONE},
    },
    {{CCON_FCONVOY_DESTROYED_REMAIN, {CONVOY_0}, RESULT_NEXT_SCRIPT, SCRIPT_L2C6_DEFEND},
     {CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_6, // SCRIPT_L2C6_MOVE3
   STYPE_HOLD, 0, 0, 12, 1, {0},
   {{PCON_CONVOY_DIST_APART, {CONVOY_0, 800}, RESULT_NEXT_SCRIPT, SCRIPT_L2C6_MOVE2}, // cycle back
    {PCON_NONE},
    },
   {{CCON_FCONVOY_DESTROYED_REMAIN, {CONVOY_0}, RESULT_NEXT_SCRIPT, SCRIPT_L2C6_DEFEND},
     {CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_6, // SCRIPT_L2C6_DEFEND
   STYPE_MOVE_XY, -500, 200, 12, 1, {0},
   {{PCON_XY, {-500, 200}, RESULT_NEXT_SCRIPT, SCRIPT_L2C6_DEFEND2},
    {PCON_NONE},
    },
   {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_6, // SCRIPT_L2C6_DEFEND2
   STYPE_STOP, 0, 0, 0, 1, {0},
   {{PCON_NONE},
   },
   {{CCON_NONE},
   }
   },

// Carrier convoy 7 - jumps in:

  {TEAM_ENEMY, CONVOY_NONE, // SCRIPT_L2C7_WAIT
   STYPE_START_WAIT, 0, 0, 0, 0, {0},
    {{PCON_LEVEL_TIME_SPECIAL, {S2SCON_C7WAIT}, RESULT_NEXT_SCRIPT, SCRIPT_L2C7_JUMP},
     {PCON_NONE},
    },
   {{CCON_NONE},
   }
   },
  {FLAG_JUMP_1, CONVOY_7, // SCRIPT_L2C7_JUMP
   STYPE_CONVOY_JUMP_SPECIAL, 0, 0, 7, 0, {0},
    {{PCON_SETUP_CONVOY, {10, 10, 200}, RESULT_NONE},
     {PCON_ALWAYS_PASS, {0}, RESULT_NEXT_SCRIPT, SCRIPT_L2C7_LAUNCH},
     {PCON_NONE},
    },
   {{CCON_NONE},
   }
   },
  {TEAM_ENEMY, CONVOY_7, // SCRIPT_L2C7_LAUNCH
   STYPE_MOVE_FORWARDS, 0, 0, 7, 0, {0},
    {{PCON_TIME, {20}, RESULT_LAUNCH, SHIP_BOMBER},
     {PCON_TIME, {22}, RESULT_LAUNCH, SHIP_BOMBER},
     {PCON_TIME, {23}, RESULT_LAUNCH, SHIP_FIGHTER},
     {PCON_TIME, {150}, RESULT_NEXT_SCRIPT, SCRIPT_L2C7_LAUNCH2},
     {PCON_NONE},
    },
   {{CCON_NONE},
   }
   },
  {TEAM_ENEMY, CONVOY_7, // SCRIPT_L2C7_LAUNCH2
   STYPE_MOVE_FORWARDS, 0, 0, 7, 0, {0},
    {{PCON_TIME, {1}, RESULT_LAUNCH, SHIP_BOMBER},
     {PCON_TIME, {3}, RESULT_LAUNCH, SHIP_BOMBER},
     {PCON_TIME, {4}, RESULT_LAUNCH, SHIP_FIGHTER},
     {PCON_TIME, {100}, RESULT_NEXT_SCRIPT, SCRIPT_L2C7_LAUNCH3},
     {PCON_NONE},
    },
   {{CCON_NONE},
   }
   },
  {TEAM_ENEMY, CONVOY_7, // SCRIPT_L2C7_LAUNCH3
   STYPE_MOVE_FORWARDS, 0, 0, 7, 0, {0},
    {{PCON_TIME, {1}, RESULT_LAUNCH, SHIP_BOMBER},
     {PCON_TIME, {3}, RESULT_LAUNCH, SHIP_BOMBER},
     {PCON_TIME, {4}, RESULT_LAUNCH, SHIP_FIGHTER},
//     {PCON_TIME, {100}, RESULT_NEXT_SCRIPT, SCRIPT_L2C8_LAUNCH2},
     {PCON_NONE},
    },
   {{CCON_NONE},
   }
   },


// Carrier convoy 8 - jumps in:

  {TEAM_ENEMY, CONVOY_NONE, // SCRIPT_L2C8_WAIT
   STYPE_START_WAIT, 0, 0, 0, 0, {0},
    {{PCON_LEVEL_TIME_SPECIAL, {S2SCON_C8WAIT}, RESULT_NEXT_SCRIPT, SCRIPT_L2C8_JUMP},
     {PCON_NONE},
    },
   {{CCON_NONE},
   }
   },
/*  {FLAG_JUMP_2, CONVOY_8, // SCRIPT_L2C8_JUMP
   STYPE_CONVOY_JUMP_SPECIAL, 0, 0, 10, 0,
    {{PCON_SETUP_CONVOY, {10, 10, 200}, RESULT_NONE},
     {PCON_ALWAYS_PASS, {0}, RESULT_NEXT_SCRIPT, SCRIPT_L2C8_MOVE},
     {PCON_NONE},
    },
   {{CCON_NONE},
   }
   },
  {TEAM_ENEMY, CONVOY_8, // SCRIPT_L2C8_MOVE
   STYPE_MOVE_XY, 0, 0, 10, 0,
   {{PCON_CONVOY_DIST, {CONVOY_3, 800}, RESULT_NEXT_SCRIPT, SCRIPT_L2C8_ATTACK},
    {PCON_NONE},
    },
   {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_8, // SCRIPT_L2C8_ATTACK
   STYPE_STOP, 0, 0, 10, 0,
   {{PCON_TIME, {50}, RESULT_NEXT_SCRIPT, SCRIPT_L2C8_MOVE2},
    {PCON_NONE},
    },
   {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_8, // SCRIPT_L2C8_MOVE2
   STYPE_MOVE_XY, 0, 0, 10, 0,
   {{PCON_CONVOY_DIST, {CONVOY_3, 300}, RESULT_NEXT_SCRIPT, SCRIPT_L2C8_STOP},
    {PCON_NONE},
    },
   {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_8, // SCRIPT_L2C8_STOP
   STYPE_STOP, 0, 0, 0, 0,
   {{PCON_NONE},
    },
   {{CCON_NONE},
    }
   },

*/
  {FLAG_JUMP_2, CONVOY_8, // SCRIPT_L2C8_JUMP
   STYPE_CONVOY_JUMP_SPECIAL, 0, 0, 10, 0, {0},
    {{PCON_SETUP_CONVOY, {100, -1500, 200}, RESULT_NONE},
     {PCON_ALWAYS_PASS, {0}, RESULT_NEXT_SCRIPT, SCRIPT_L2C8_MOVE},
     {PCON_NONE},
    },
   {{CCON_NONE},
   }
   },
  {TEAM_ENEMY, CONVOY_8, // SCRIPT_L2C8_MOVE
   STYPE_MOVE_XY, 100, -1500, 10, 0, {0},
   {{PCON_XY, {100, -1500}, RESULT_NEXT_SCRIPT, SCRIPT_L2C8_MOVE2},
    {PCON_NONE},
    },
   {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_8, // SCRIPT_L2C8_MOVE2
   STYPE_MOVE_XY, 700, 0, 10, 0, {0},
   {{PCON_XY, {700, 0}, RESULT_NEXT_SCRIPT, SCRIPT_L2C8_MOVE3},
    {PCON_NONE},
    },
   {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_8, // SCRIPT_L2C8_MOVE3
   STYPE_MOVE_XY, 0, 700, 10, 0, {0},
   {{PCON_XY, {0, 700}, RESULT_NEXT_SCRIPT, SCRIPT_L2C8_MOVE4},
    {PCON_NONE},
    },
   {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_8, // SCRIPT_L2C8_MOVE4
   STYPE_MOVE_XY, -700, 0, 10, 0, {0},
   {{PCON_XY, {-700, 0}, RESULT_NEXT_SCRIPT, SCRIPT_L2C8_MOVE5},
    {PCON_NONE},
    },
   {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_8, // SCRIPT_L2C8_MOVE5
   STYPE_MOVE_XY, 0, -700, 10, 0, {0},
   {{PCON_XY, {0, -700}, RESULT_NEXT_SCRIPT, SCRIPT_L2C8_MOVE2}, // cycles back
    {PCON_NONE},
    },
   {{CCON_NONE},
    }
   },

  }


 }, // end Stage 2


 { // Stage 3!
// S2M
  {
   {""},
   {" $BAlpha group $Call present, good. ", {WAV_SELECT0, NOTE_2G}},
   {"There is a large group of ships around the target. $BAngry Moth, $Cfly in and see if you can do some damage to the outer defences - but be careful. ", {WAV_SELECT1, NOTE_2G}},
   {" $BAlpha group, $Cadjust course. ", {WAV_SELECT0, NOTE_2G}},
   {"Looks like several warships have broken away from the base and are giving chase. Careful, $BAlpha group.$C ", {WAV_SELECT0, NOTE_2G}},
   {"A small group of warships just jumped in at 12 o'clock. ", {WAV_SELECT0, NOTE_2G}},
   {"Engaging enemy. $BAngry Moth, $Cwe'll need fighter support for this! ", {WAV_SELECT1, NOTE_2G}},
   {"We're picking up jump signatures for another group of enemy ships. Looks like carriers - watch for bomber squadrons! ", {WAV_SELECT0, NOTE_2G}},

   {"This is $BBeta 1 $Cto $BAlpha group. $CDo you read me? ", {WAV_SELECT0, NOTE_2G}},
   {" $BBeta 1, $Cglad you could make it. We're meeting heavy resistance but the operation is proceeding as planned. ", {WAV_SELECT0, NOTE_2A}},
   {"Got that. Looks like we jumped in at the right time. $BBeta group $Ccourse set for target orbital. ", {WAV_SELECT0, NOTE_2G}},
   {" $BBeta group $Ccommencing attack on enemy base. Looks like we're up against some kind of beam weapon - hope it's not too nasty. ", {WAV_SELECT0, NOTE_2G}},
   {"Enemy base destroyed! The mission is accomplished - now we just have to get out of here. ", {WAV_SELECT0, NOTE_2G}},
   {"Looks like we're clear! Time to go home. ", {WAV_SELECT0, NOTE_2G}},
   {"The enemy carrier group just launched a large squadron of bombers. $BAngry Moth, $Ckeep them away from our cruisers! ", {WAV_SELECT1, NOTE_2A}},
   {"Another several carrier launches detected. Take care of those bombers! ", {WAV_SELECT1, NOTE_2A}},
   {"We're picking up another lot of bombers on our scans. How many times can those carriers launch? ", {WAV_SELECT0, NOTE_2G}},
   {"All enemy carriers disabled. Should make this a lot easier. ", {WAV_SELECT0, NOTE_2G}},
   {" $BAlpha group $Cjoining the attack on the enemy base. ", {WAV_SELECT0, NOTE_2A}},
   {" $BAlpha group $Cengaging with enemy carriers. Looks like they have heavy fighter escort - $BAngry Moth, $Cwe need support here. ", {WAV_SELECT1, NOTE_2A}},
   {" $BBeta group $Cis about to engage with base defence ships. ", {WAV_SELECT0, NOTE_2G}},
   {" $BAlpha group $Cadjusting course to engage with enemy carrier group. ", {WAV_SELECT1, NOTE_2A}},
   {"Another group of enemy warships just jumped in. ", {WAV_SELECT0, NOTE_2G}},
   {" $BAlpha group $Cis down! ", {WAV_SELECT0, NOTE_2G}},
   {"We just lost $BBeta group! $CThis is not looking good. ", {WAV_SELECT0, NOTE_2A}},
  },

 {
// Decoy group:
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 1, 0, 0,0,
   CONVOY_0, FLAG_COMMAND_2|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS_2,
    {{0, 0}, {0, 0}}},
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_0, FLAG_COMMAND_2|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS_2,
    {{-ANGLE_8, 150}, {-ANGLE_4, 150}}},
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_0, FLAG_COMMAND_2|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS_2,
    {{ANGLE_8, 150}, {ANGLE_4, 150}}},
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_0, FLAG_COMMAND_2|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS_2,
    {{-ANGLE_4-ANGLE_16, 180}, {-ANGLE_4 + ANGLE_16, 250}}},
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_0, FLAG_COMMAND_2|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS_2,
    {{ANGLE_4+ANGLE_16, 180}, {ANGLE_4-ANGLE_16, 250}}},
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 2, 0, 0,0,
   CONVOY_0, FLAG_COMMAND_2|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS_2,
    {{ANGLE_2, 170}, {ANGLE_2, 150}}},

  {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 1, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 1, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 2, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 2, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},


// Main attack group:
  {TEAM_FRIEND, SHIP_OLD3, VAR_NONE, 0, ACT_NONE, 3, 0, 0,0,
   CONVOY_1, FLAG_COMMAND_1|FLAG_BETA|FLAG_SPAWN_1|FLAG_JUMP_1, FLAGS_2,
    {{0, 0}, {0, 0}}},
  {TEAM_FRIEND, SHIP_OLD3, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAG_COMMAND_1|FLAG_BETA|FLAG_SPAWN_1|FLAG_JUMP_1, FLAGS_2,
    {{-ANGLE_8, 150}, {-ANGLE_4, 150}}},
  {TEAM_FRIEND, SHIP_OLD3, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAG_COMMAND_1|FLAG_BETA|FLAG_SPAWN_1|FLAG_JUMP_1, FLAGS_2,
    {{ANGLE_8, 150}, {ANGLE_4, 150}}},
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAG_COMMAND_1|FLAG_BETA|FLAG_SPAWN_1|FLAG_JUMP_1, FLAGS_2,
    {{-ANGLE_4-ANGLE_16, 180}, {-ANGLE_4 + ANGLE_16, 250}}},
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAG_COMMAND_1|FLAG_BETA|FLAG_SPAWN_1|FLAG_JUMP_1, FLAGS_2,
    {{ANGLE_4+ANGLE_16, 180}, {ANGLE_4-ANGLE_16, 250}}},
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAG_COMMAND_1|FLAG_BETA|FLAG_SPAWN_1|FLAG_JUMP_1, FLAGS_2,
    {{-ANGLE_4, 230}, {-ANGLE_4 + ANGLE_8, 290}}},
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAG_COMMAND_1|FLAG_BETA|FLAG_SPAWN_1|FLAG_JUMP_1, FLAGS_2,
    {{ANGLE_4, 230}, {ANGLE_4-ANGLE_8, 290}}},
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 4, 0, 0,0,
   CONVOY_1, FLAG_COMMAND_1|FLAG_BETA|FLAG_SPAWN_1|FLAG_JUMP_1, FLAGS_2,
    {{ANGLE_2, 170}, {ANGLE_2, 150}}},
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAG_COMMAND_1|FLAG_BETA|FLAG_SPAWN_1|FLAG_JUMP_1, FLAGS_2,
    {{0, 150}, {0, 150}}},

  {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 3, 1, 0,0,
   CONVOY_NONE, FLAGS|FLAG_JUMP_1, FLAGS_2, {{0,0}}},
  {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 3, 2, 0,0,
   CONVOY_NONE, FLAGS|FLAG_JUMP_1, FLAGS_2, {{0,0}}},
  {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 4, 1, 0,0,
   CONVOY_NONE, FLAGS|FLAG_JUMP_1, FLAGS_2, {{0,0}}},
  {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 4, 2, 0,0,
   CONVOY_NONE, FLAGS|FLAG_JUMP_1, FLAGS_2, {{0,0}}},

// fighter groups 5 and 6 reserved for friendly

// ENEMY BASE

  {TEAM_ENEMY, SHIP_EBASE, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_2, FLAGS, FLAGS_2,
    {{0, 0}, {0, 0}}},

// Convoy 3 - These ships start near base and go out to meet the decoy convoy
  {TEAM_ENEMY, SHIP_SCOUT3, VAR_NONE, 0, ACT_NONE, 7, 0, 0,0,
   CONVOY_3, FLAGS, FLAGS_2,
    {{ANGLE_4, 250}, {ANGLE_8, 150}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_3, FLAGS, FLAGS_2,
    {{ANGLE_4 + ANGLE_16, 750}, {ANGLE_4, 200}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_3, FLAGS, FLAGS_2,
    {{ANGLE_4 + ANGLE_8, 1250}, {ANGLE_4 + ANGLE_8, 150}}},
  {TEAM_ENEMY, SHIP_SCOUT3, VAR_NONE, 0, ACT_NONE, 8, 0, 0,0,
   CONVOY_3, FLAGS, FLAGS_2,
    {{-ANGLE_4, 250}, {-ANGLE_8, 150}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_3, FLAGS, FLAGS_2,
    {{-ANGLE_4 - ANGLE_16, 750}, {-ANGLE_4, 200}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_3, FLAGS, FLAGS_2,
    {{-ANGLE_4 - ANGLE_8, 1250}, {-ANGLE_4 - ANGLE_8, 150}}},

  {TEAM_ENEMY, SHIP_EINT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 7, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_EINT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 7, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 7, 3, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 7, 4, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_EINT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 8, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_EINT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 8, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 8, 3, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 8, 4, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},

// Convoy 4 - These ships start near base and engage with the main convoy when it arrives
  {TEAM_ENEMY, SHIP_SCOUT3, VAR_NONE, 0, ACT_NONE, 9, 0, 0,0,
   CONVOY_4, FLAGS, FLAGS_2,
    {{ANGLE_4, 250}, {ANGLE_8, 150}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_4, FLAGS, FLAGS_2,
    {{ANGLE_4 + ANGLE_16, 750}, {ANGLE_4, 200}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_4, FLAGS, FLAGS_2,
    {{ANGLE_4 + ANGLE_8, 1250}, {ANGLE_4 + ANGLE_8, 150}}},
  {TEAM_ENEMY, SHIP_SCOUT3, VAR_NONE, 0, ACT_NONE, 10, 0, 0,0,
   CONVOY_4, FLAGS, FLAGS_2,
    {{-ANGLE_4, 250}, {-ANGLE_8, 150}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_4, FLAGS, FLAGS_2,
    {{-ANGLE_4 - ANGLE_16, 750}, {-ANGLE_4, 200}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_4, FLAGS, FLAGS_2,
    {{-ANGLE_4 - ANGLE_8, 1250}, {-ANGLE_4 - ANGLE_8, 150}}},

  {TEAM_ENEMY, SHIP_EINT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 9, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_EINT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 9, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 9, 3, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 9, 4, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_EINT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 10, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_EINT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 10, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 10, 3, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 10, 4, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},

// Convoy 5 - These ships jump in and engage decoy group. Any survivors then move to base and defend it
  {TEAM_ENEMY, SHIP_SCOUT3, VAR_NONE, 0, ACT_NONE, 11, 0, 0,0,
   CONVOY_5, FLAG_JUMP_2, FLAGS_2,
    {{ANGLE_4, 250}, {ANGLE_8, 150}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_5, FLAG_JUMP_2, FLAGS_2,
    {{ANGLE_4 + ANGLE_32, 750}, {ANGLE_4, 220}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_5, FLAG_JUMP_2, FLAGS_2,
    {{ANGLE_4 + ANGLE_16, 1250}, {ANGLE_4 + ANGLE_8, 150}}},
  {TEAM_ENEMY, SHIP_SCOUT3, VAR_NONE, 0, ACT_NONE, 12, 0, 0,0,
   CONVOY_5, FLAG_JUMP_2, FLAGS_2,
    {{-ANGLE_4, 250}, {-ANGLE_8, 150}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_5, FLAG_JUMP_2, FLAGS_2,
    {{-ANGLE_4 - ANGLE_32, 750}, {-ANGLE_4, 220}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_5, FLAG_JUMP_2, FLAGS_2,
    {{-ANGLE_4 - ANGLE_16, 1250}, {-ANGLE_4 - ANGLE_8, 150}}},

  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 11, 1, 0,0,
   CONVOY_NONE, FLAG_JUMP_2, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 11, 2, 0,0,
   CONVOY_NONE, FLAG_JUMP_2, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 12, 1, 0,0,
   CONVOY_NONE, FLAG_JUMP_2, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 12, 2, 0,0,
   CONVOY_NONE, FLAG_JUMP_2, FLAGS_2, {{0,0}}},


// Carrier convoy 7 - jumps in quickly and launches bombers at decoy convoy.
  {TEAM_ENEMY, SHIP_SCOUTCAR, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_7, FLAG_JUMP_3, FLAGS_2,
    {{0, 200}}},
  {TEAM_ENEMY, SHIP_SCOUTCAR, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_7, FLAG_JUMP_3, FLAGS_2,
    {{ANGLE_2, 200}}},
  {TEAM_ENEMY, SHIP_SCOUTCAR, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_7, FLAG_JUMP_3, FLAGS_2,
    {{ANGLE_4, 120}}},
  {TEAM_ENEMY, SHIP_SCOUTCAR, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_7, FLAG_JUMP_3, FLAGS_2,
    {{-ANGLE_4, 120}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 13, 0, 0,0,
   CONVOY_7, FLAG_JUMP_3, FLAGS_2,
    {{ANGLE_8, 200}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 14, 0, 0,0,
   CONVOY_7, FLAG_JUMP_3, FLAGS_2,
    {{-ANGLE_8, 200}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 15, 0, 0,0,
   CONVOY_7, FLAG_JUMP_3, FLAGS_2,
    {{ANGLE_2+ANGLE_8, 200}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 16, 0, 0,0,
   CONVOY_7, FLAG_JUMP_3, FLAGS_2,
    {{ANGLE_2-ANGLE_8, 200}}},

/*
  {TEAM_ENEMY, SHIP_SCOUTCAR, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_7, FLAG_JUMP_1, FLAGS_2,
    {{0, 200}}},
  {TEAM_ENEMY, SHIP_SCOUTCAR, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_7, FLAG_JUMP_1, FLAGS_2,
    {{ANGLE_3, 200}}},
  {TEAM_ENEMY, SHIP_SCOUTCAR, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_7, FLAG_JUMP_1, FLAGS_2,
    {{-ANGLE_3, 200}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 13, 0, 0,0,
   CONVOY_7, FLAG_JUMP_1, FLAGS_2,
    {{ANGLE_6, 300}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 14, 0, 0,0,
   CONVOY_7, FLAG_JUMP_1, FLAGS_2,
    {{-ANGLE_6, 300}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 15, 0, 0,0,
   CONVOY_7, FLAG_JUMP_1, FLAGS_2,
    {{ANGLE_2, 300}}},*/

  {TEAM_ENEMY, SHIP_ESCOUT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 13, 1, 0,0,
   CONVOY_NONE, FLAG_JUMP_3, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_ESCOUT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 13, 2, 0,0,
   CONVOY_NONE, FLAG_JUMP_3, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_ESCOUT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 14, 1, 0,0,
   CONVOY_NONE, FLAG_JUMP_3, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_ESCOUT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 15, 2, 0,0,
   CONVOY_NONE, FLAG_JUMP_3, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_ESCOUT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 16, 1, 0,0,
   CONVOY_NONE, FLAG_JUMP_3, FLAGS_2, {{0,0}}},


// Convoy 8 - These ships jump in late and engage main group from behind
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 17, 0, 0,0,
   CONVOY_8, FLAG_JUMP_4, FLAGS_2,
    {{ANGLE_4, 250}, {ANGLE_8, 150}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_8, FLAG_JUMP_4, FLAGS_2,
    {{ANGLE_4 + ANGLE_32, 750}, {ANGLE_4, 220}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_8, FLAG_JUMP_4, FLAGS_2,
    {{ANGLE_4 + ANGLE_16, 1250}, {ANGLE_4 + ANGLE_8, 150}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 18, 0, 0,0,
   CONVOY_8, FLAG_JUMP_4, FLAGS_2,
    {{-ANGLE_4, 250}, {-ANGLE_8, 150}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_8, FLAG_JUMP_4, FLAGS_2,
    {{-ANGLE_4 - ANGLE_32, 750}, {-ANGLE_4, 220}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_8, FLAG_JUMP_4, FLAGS_2,
    {{-ANGLE_4 - ANGLE_16, 1250}, {-ANGLE_4 - ANGLE_8, 150}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_LONG, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_8, FLAG_JUMP_4, FLAGS_2,
    {{0, 1250}, {0, 180}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_LONG, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_8, FLAG_JUMP_4, FLAGS_2,
    {{0, 1250}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_LONG, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_8, FLAG_JUMP_4, FLAGS_2,
    {{0, 1250}, {ANGLE_2, 180}}},

  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 17, 1, 0,0,
   CONVOY_NONE, FLAG_JUMP_4, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 17, 2, 0,0,
   CONVOY_NONE, FLAG_JUMP_4, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 17, 3, 0,0,
   CONVOY_NONE, FLAG_JUMP_4, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 17, 4, 0,0,
   CONVOY_NONE, FLAG_JUMP_4, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 18, 1, 0,0,
   CONVOY_NONE, FLAG_JUMP_4, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 18, 2, 0,0,
   CONVOY_NONE, FLAG_JUMP_4, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 18, 3, 0,0,
   CONVOY_NONE, FLAG_JUMP_4, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 18, 4, 0,0,
   CONVOY_NONE, FLAG_JUMP_4, FLAGS_2, {{0,0}}},

  {TEAM_NONE} // MUST finish with this!



  },


// S3S
  {

   {
   },

// This script doesn't start until the main attack group jumps in
  {TEAM_FRIEND, CONVOY_NONE, // SCRIPT_L3_BASIC
   STYPE_WAIT, 0, 0, 0, 0, {0},
   {{PCON_NONE},
    },
    {{CCON_FWSHIPS_DESTROYED, {0}, RESULT_MISSION_FAIL, 0, MSG_NONE},
     {CCON_ECARRIERS_DISABLED, {0}, RESULT_NONE, 0, MSG_L3M10, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {CCON_CONVOY_DESTROYED, {CONVOY_0}, RESULT_NONE, 0, MSG_L3M16, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {CCON_CONVOY_DESTROYED, {CONVOY_1}, RESULT_NONE, 0, MSG_L3M17, MSG_FROM_COMMAND_2, 1, MSG_TO_ALL},
     {CCON_NONE},
    }
   },


  {TEAM_FRIEND, CONVOY_0, // SCRIPT_L3C0_SETUP
   STYPE_SETUP_CONVOY, -1500, -8300, 0, 0, {0},
    {{PCON_SETUP_CONVOY, {0, 0, 10}, RESULT_NEXT_SCRIPT, SCRIPT_L3C0_MOVE}, // setup scripts can't have messages
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_FRIEND, CONVOY_0, // SCRIPT_L3C0_MOVE
   STYPE_MOVE_XY, 0, 0, 16, 0, {0},
   {
     {PCON_LEVEL_TIME, {4}, RESULT_NONE, 0, MSG_L3K1, MSG_FROM_COMMAND_2, 1, MSG_TO_ALL},
     {PCON_LEVEL_TIME, {8}, RESULT_NONE, 0, MSG_L3K2, MSG_FROM_COMMAND_2, 1, MSG_TO_AM},
     {PCON_LEVEL_TIME, {60}, RESULT_NEXT_SCRIPT, SCRIPT_L3CO_TURN, MSG_L3K3, MSG_FROM_COMMAND_2, 1, MSG_TO_ALL},
     {PCON_NONE},
    },
    {{CCON_CONVOY_DESTROYED, {CONVOY_2}, RESULT_NEXT_SCRIPT, SCRIPT_L3C0_ESCAPE},
     {CCON_NONE},
    }
   },
  {TEAM_FRIEND, CONVOY_0, // SCRIPT_L3CO_TURN
   STYPE_MOVE_XY, -12000, 1500, 16, 0, {0},
   {{PCON_LEVEL_TIME, {255}, RESULT_NEXT_SCRIPT, SCRIPT_L3CO_CARRIERS},
    {PCON_NONE},
    },
    {{CCON_CONVOY_DESTROYED, {CONVOY_2}, RESULT_NEXT_SCRIPT, SCRIPT_L3C0_ESCAPE},
     {CCON_NONE},
    }
   },
  {TEAM_FRIEND, CONVOY_0, // SCRIPT_L3CO_CARRIERS
   STYPE_APPROACH_CONVOY, CONVOY_7, 0, 16, 0, {0},
   {{PCON_CONVOY_DIST, {CONVOY_7, 350}, RESULT_NEXT_SCRIPT, SCRIPT_L3C0_ATTACK, MSG_L3M12, MSG_FROM_COMMAND_2, 1, MSG_TO_AM},
    {PCON_LEVEL_TIME, {305}, RESULT_NONE, 0, MSG_L3M14, MSG_FROM_COMMAND_2, 1, MSG_TO_ALL},
    {PCON_NONE},
    },
    {{CCON_CONVOY_DESTROYED, {CONVOY_2}, RESULT_NEXT_SCRIPT, SCRIPT_L3C0_ESCAPE},
     {CCON_NONE},
    }
   },
  {TEAM_FRIEND, CONVOY_0, // SCRIPT_L3CO_ATTACK
   STYPE_APPROACH_CONVOY, CONVOY_7, 0, 6, 0, {0},
   {{PCON_NONE},
    },
    {{CCON_CONVOY_DESTROYED, {CONVOY_7}, RESULT_NEXT_SCRIPT, SCRIPT_L3C0_JOIN, MSG_L3M11, MSG_FROM_COMMAND_2, 1, MSG_TO_ALL},
     {CCON_CONVOY_DESTROYED, {CONVOY_2}, RESULT_NEXT_SCRIPT, SCRIPT_L3C0_ESCAPE},
     {CCON_NONE},
    }
   },
  {TEAM_FRIEND, CONVOY_0, // SCRIPT_L3CO_JOIN
   STYPE_APPROACH_CONVOY, CONVOY_2, 0, 16, 0, {0},
   {{PCON_CONVOY_DIST, {CONVOY_2, 350}, RESULT_NEXT_SCRIPT, SCRIPT_L3C0_HOLD},
    {PCON_NONE},
    },
    {{CCON_CONVOY_DESTROYED, {CONVOY_2}, RESULT_NEXT_SCRIPT, SCRIPT_L3C0_ESCAPE},
     {CCON_NONE},
    }
   },
  {TEAM_FRIEND, CONVOY_0, // SCRIPT_L3CO_HOLD
   STYPE_STOP, 0, 0, 0, 0, {0},
   {{PCON_NONE},
    },
    {{CCON_CONVOY_DESTROYED, {CONVOY_2}, RESULT_NEXT_SCRIPT, SCRIPT_L3C0_ESCAPE},
     {CCON_NONE},
    }
   },
  {TEAM_FRIEND, CONVOY_0, // SCRIPT_L3C0_ESCAPE
   STYPE_MOVE_XY, 5500, -6000, 12, 0, {0},
   {{PCON_TIME, {70}, RESULT_MISSION_OVER, 0},
    {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },

  {TEAM_FRIEND, CONVOY_NONE, // SCRIPT_L3C1_WAIT
   STYPE_START_WAIT, 0, 0, 0, 0, {0},
    {{PCON_LEVEL_TIME, {275}, RESULT_NEXT_SCRIPT, SCRIPT_L3C1_JUMP},
     {PCON_NONE},
    },
   {{CCON_NONE},
   }
   },
  {FLAG_JUMP_1, CONVOY_1, // SCRIPT_L3C1_JUMP
   STYPE_CONVOY_JUMP, 1000, 10400, 12, 0, {0},
    {{PCON_SETUP_CONVOY, {0, 0, 200}, RESULT_NONE},
     {PCON_ALWAYS_PASS, {0}, RESULT_START_SCRIPT, SCRIPT_L3_BASIC},
     {PCON_ALWAYS_PASS, {0}, RESULT_NEXT_SCRIPT, SCRIPT_L3C1_MOVE},
     {PCON_NONE},
    },
   {{CCON_NONE},
   }
   },
  {TEAM_FRIEND, CONVOY_1, // SCRIPT_L3C1_MOVE
   STYPE_MOVE_XY, 0, 0, 12, 0, {0},
   {{PCON_CONVOY_DIST, {CONVOY_2, 350}, RESULT_NEXT_SCRIPT, SCRIPT_L3C1_HOLD},
    {PCON_TIME, {3}, RESULT_NONE, 0, MSG_L3M1, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {PCON_TIME, {8}, RESULT_NONE, 0, MSG_L3M2, MSG_FROM_COMMAND_2, 1, MSG_TO_ALL},
    {PCON_TIME, {15}, RESULT_NONE, 0, MSG_L3M3, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {PCON_NONE},
    },
    {{CCON_CONVOY_DESTROYED, {CONVOY_2}, RESULT_NEXT_SCRIPT, SCRIPT_L3C1_ESCAPE, MSG_L3M5, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {CCON_NONE},
    }
   },
  {TEAM_FRIEND, CONVOY_1, // SCRIPT_L3C1_HOLD
   STYPE_STOP, 0, 0, 0, 0, {0},
   {{PCON_TIME, {1}, RESULT_NONE, 0, MSG_L3M4, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {PCON_NONE},
    },
    {{CCON_CONVOY_DESTROYED, {CONVOY_2}, RESULT_NEXT_SCRIPT, SCRIPT_L3C1_ESCAPE, MSG_L3M5, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {CCON_NONE},
    }
   },
  {TEAM_FRIEND, CONVOY_1, // SCRIPT_L3C1_ESCAPE
   STYPE_MOVE_XY, 6000, -6000, 12, 0, {0},
   {{PCON_TIME, {64}, RESULT_NONE, 0, MSG_L3M6, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {PCON_TIME, {70}, RESULT_MISSION_OVER, 0},
    {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
/*  {TEAM_FRIEND, CONVOY_0, // SCRIPT_L3C0_ATTACK
   STYPE_STOP, 0, 0, 0, 1,
   {{PCON_NONE},
    },
   {{CCON_CONVOY_DESTROYED, {CONVOY_3}, RESULT_NEXT_SCRIPT, SCRIPT_L3C0_ESCAPE},
    {CCON_NONE},
    }
   },
  {TEAM_FRIEND, CONVOY_0, // SCRIPT_L2C0_ESCAPE
   STYPE_MOVE_XY, 8000, -7000, 12, 0,
   {{PCON_XY, {4000, -1000}, RESULT_MISSION_OVER},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
*/


  {TEAM_ENEMY, CONVOY_2, // SCRIPT_L3C2_SETUP
   STYPE_SETUP_CONVOY, 0, 0, 0, 0, {0},
    {{PCON_SETUP_CONVOY, {0, 0, 0}, RESULT_NEXT_SCRIPT, SCRIPT_L3C2_RUN},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_2, // SCRIPT_L3C2_RUN
   STYPE_STOP, 0, 0, 0, 0, {0},
   {{PCON_NONE},
    },
    {{CCON_NONE},
    }
   },


  {TEAM_ENEMY, CONVOY_3, // SCRIPT_L3C3_START
   STYPE_SETUP_CONVOY, -700, -700, 0, 0, {0},
    {{PCON_SETUP_CONVOY, {-2000, -2000, 0}, RESULT_NEXT_SCRIPT, SCRIPT_L3C3_WAIT},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_3, // SCRIPT_L3C3_WAIT
   STYPE_HOLD, 0, 0, 0, 0, {0},
    {{PCON_LEVEL_TIME, {61}, RESULT_NEXT_SCRIPT, SCRIPT_L3C3_MOVE},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_3, // SCRIPT_L3C3_MOVE
   STYPE_APPROACH_CONVOY_OFFSET, CONVOY_0, 0, 12, 1, {300, 300},
   {{PCON_TIME, {5}, RESULT_NONE, 0, MSG_L3K4, MSG_FROM_COMMAND_2, 1, MSG_TO_ALL}, // defenders moving
    {PCON_NONE},
    },
    {{CCON_CONVOY_DESTROYED, {CONVOY_0}, RESULT_NEXT_SCRIPT, SCRIPT_L3C3_DEFEND},
     {CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_3, // SCRIPT_L3C3_DEFEND
   STYPE_MOVE_XY, -300, 200, 12, 1, {0},
   {{PCON_XY, {-300, 200}, RESULT_NEXT_SCRIPT, SCRIPT_L3C3_DEFEND2},
    {PCON_NONE},
    },
   {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_3, // SCRIPT_L3C3_DEFEND2
   STYPE_STOP, 0, 0, 0, 1, {0},
   {{PCON_NONE},
   },
   {{CCON_NONE},
   }
   },

  {TEAM_ENEMY, CONVOY_4, // SCRIPT_L3C4_START
   STYPE_SETUP_CONVOY, 700, 700, 0, 0, {0},
    {{PCON_SETUP_CONVOY, {2000, 2000, 0}, RESULT_NEXT_SCRIPT, SCRIPT_L3C4_WAIT},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_4, // SCRIPT_L3C4_WAIT
   STYPE_HOLD, 0, 0, 0, 0, {0},
    {{PCON_LEVEL_TIME, {290}, RESULT_NEXT_SCRIPT, SCRIPT_L3C4_ATTACK},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_4, // SCRIPT_L3C4_ATTACK
   STYPE_APPROACH_CONVOY_OFFSET, CONVOY_1, 0, 12, 1, {400, -100},
    {{PCON_CONVOY_DIST, {CONVOY_1, 600}, RESULT_NEXT_SCRIPT, SCRIPT_L3C4_FIGHT, MSG_L3M13, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL, TEST_CONVOY_EXISTS},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_4, // SCRIPT_L3C4_FIGHT
   STYPE_MOVE_FORWARDS, 0, 0, 12, 1, {0},
    {{PCON_CONVOY_DIST_APART, {CONVOY_1, 800}, RESULT_NEXT_SCRIPT, SCRIPT_L3C4_ATTACK2},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_4, // SCRIPT_L3C4_ATTACK2
   STYPE_APPROACH_CONVOY_OFFSET, CONVOY_2, 0, 12, 1, {-100, 300},
    {{PCON_CONVOY_DIST, {CONVOY_1, 400}, RESULT_NEXT_SCRIPT, SCRIPT_L3C4_FIGHT2},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_4, // SCRIPT_L3C4_FIGHT2
   STYPE_HOLD, 0, 0, 0, 1, {0},
    {{PCON_NONE},
    },
    {{CCON_CONVOY_DESTROYED, {CONVOY_2}, RESULT_NEXT_SCRIPT, SCRIPT_L3C4_FOLLOW},
    }
   },
  {TEAM_ENEMY, CONVOY_4, // SCRIPT_L3C4_FOLLOW
   STYPE_APPROACH_CONVOY, CONVOY_1, 0, 12, 1, {0},
    {{PCON_NONE},
    },
    {{CCON_NONE},
    }
   },


  {TEAM_ENEMY, CONVOY_NONE, // SCRIPT_L3C5_WAIT
   STYPE_START_WAIT, 0, 0, 0, 0, {0},
    {{PCON_LEVEL_TIME, {200}, RESULT_NEXT_SCRIPT, SCRIPT_L3C5_JUMP},
     {PCON_NONE},
    },
   {{CCON_NONE},
   }
   },
  {FLAG_JUMP_2, CONVOY_5, // SCRIPT_L3C5_JUMP
   STYPE_CONVOY_JUMP, -10000, -2000, 7, 1, {0},
    {{PCON_SETUP_CONVOY, {-10000, -2000, 200}, RESULT_NONE},
     {PCON_ALWAYS_PASS, {0}, RESULT_NEXT_SCRIPT, SCRIPT_L3C5_ATTACK_C0},
     {PCON_NONE},
    },
   {{CCON_NONE},
   }
   },
  {TEAM_ENEMY, CONVOY_5, // SCRIPT_L3C5_ATTACK_C0
   STYPE_APPROACH_CONVOY_OFFSET, CONVOY_0, 0, 12, 1, {-20, -330},
   {{PCON_TIME, {2}, RESULT_NONE, 0, MSG_L3K5, MSG_FROM_COMMAND_2, 1, MSG_TO_ALL},
    {PCON_CONVOY_DIST, {CONVOY_0, 500}, RESULT_NEXT_SCRIPT, SCRIPT_L3C5_ATTACK_C0_2, MSG_L3K6, MSG_FROM_COMMAND_2, 1, MSG_TO_AM},
    {PCON_NONE},
    },
   {{CCON_CONVOY_DESTROYED, {CONVOY_0}, RESULT_NEXT_SCRIPT, SCRIPT_L3C5_MOVE},
    {CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_5, // SCRIPT_L3C5_ATTACK_C0_2
   STYPE_MOVE_FORWARDS, 0, 0, 12, 1, {0},
   {{PCON_CONVOY_DIST_APART, {CONVOY_0, 800}, RESULT_NEXT_SCRIPT, SCRIPT_L3C5_MOVE},
    {PCON_NONE},
    },
   {{CCON_CONVOY_DESTROYED, {CONVOY_0}, RESULT_NEXT_SCRIPT, SCRIPT_L3C5_MOVE},
    {CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_5, // SCRIPT_L3C5_MOVE
   STYPE_MOVE_XY, 0, 400, 12, 1, {0},
   {{PCON_XY, {0, 400}, RESULT_NEXT_SCRIPT, SCRIPT_L3C5_DEFEND},
    {PCON_NONE},
    },
   {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_5, // SCRIPT_L3C5_DEFEND
   STYPE_HOLD, 0, 0, 0, 1, {0},
    {{PCON_NONE},
    },
    {{CCON_NONE},
    }
   },


  {TEAM_ENEMY, CONVOY_NONE, // SCRIPT_L3C7_WAIT
   STYPE_START_WAIT, 0, 0, 0, 0, {0},
    {{PCON_LEVEL_TIME, {250}, RESULT_NEXT_SCRIPT, SCRIPT_L3C7_JUMP},
     {PCON_NONE},
    },
   {{CCON_NONE},
   }
   },
  {FLAG_JUMP_3, CONVOY_7, // SCRIPT_L3C7_JUMP
   STYPE_CONVOY_JUMP, -7000, 6000, 5, 0, {0},
    {{PCON_SETUP_CONVOY, {8000, 7000, 200}, RESULT_NONE},
     {PCON_ALWAYS_PASS, {0}, RESULT_NEXT_SCRIPT, SCRIPT_L3C7_RUN},
     {PCON_NONE},
    },
   {{CCON_NONE},
   }
   },
  {TEAM_ENEMY, CONVOY_7, // SCRIPT_L3C7_RUN
   STYPE_MOVE_FORWARDS, 0, 0, 5, 0, {0},
   {{PCON_LEVEL_TIME, {252}, RESULT_NONE, 0, MSG_L3K7, MSG_FROM_COMMAND_2, 1, MSG_TO_ALL},
    {PCON_LEVEL_TIME, {305}, RESULT_NEXT_SCRIPT, SCRIPT_L3C7_LAUNCH},
    {PCON_LEVEL_TIME, {450}, RESULT_NEXT_SCRIPT, SCRIPT_L3C7_LAUNCH},
    {PCON_LEVEL_TIME, {550}, RESULT_NEXT_SCRIPT, SCRIPT_L3C7_LAUNCH},
// reflect any changes to times in L3C7_MSG below
    {PCON_NONE},
    },
   {{CCON_ECARRIERS_DISABLED, {0}, RESULT_NEXT_SCRIPT, SCRIPT_L3C7_NOTHING},
    {CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_7, // SCRIPT_L3C7_LAUNCH
   STYPE_MOVE_FORWARDS, 0, 0, 5, 0, {0},
    {{PCON_TIME, {1}, RESULT_LAUNCH, SHIP_BOMBER},
     {PCON_TIME, {2}, RESULT_LAUNCH, SHIP_BOMBER},
     {PCON_TIME, {3}, RESULT_LAUNCH, SHIP_FIGHTER},
     {PCON_TIME, {4}, RESULT_NEXT_SCRIPT, SCRIPT_L3C7_MSG},
     {PCON_NONE},
    },
   {{CCON_ECARRIERS_DISABLED, {0}, RESULT_NEXT_SCRIPT, SCRIPT_L3C7_NOTHING},
    {CCON_NONE},
   }
   },

  {TEAM_ENEMY, CONVOY_7, // SCRIPT_L3C7_MSG
   STYPE_MOVE_FORWARDS, 0, 0, 5, 0, {0},
    {{PCON_LEVEL_TIME, {326}, RESULT_NONE, 0, MSG_L3M7, MSG_FROM_COMMAND_2, 1, MSG_TO_AM},
     {PCON_LEVEL_TIME, {456}, RESULT_NONE, 0, MSG_L3M8, MSG_FROM_COMMAND_1, 1, MSG_TO_AM},
     {PCON_LEVEL_TIME, {556}, RESULT_NONE, 0, MSG_L3M9, MSG_FROM_COMMAND_1, 1, MSG_TO_AM},
     {PCON_TIME, {10}, RESULT_NEXT_SCRIPT, SCRIPT_L3C7_RUN},
     {PCON_NONE},
    },
   {{CCON_ECARRIERS_DISABLED, {0}, RESULT_NEXT_SCRIPT, SCRIPT_L3C7_NOTHING},
    {CCON_NONE},
   }
   },
  {TEAM_ENEMY, CONVOY_7, // SCRIPT_L3C7_NOTHING
   STYPE_MOVE_FORWARDS, 0, 0, 5, 0, {0},
    {{PCON_NONE},
    },
   {{CCON_NONE},
   }
   },

  {TEAM_ENEMY, CONVOY_NONE, // SCRIPT_L3C8_WAIT
   STYPE_START_WAIT, 0, 0, 0, 0, {0},
   {{PCON_LEVEL_TIME, {460}, RESULT_NEXT_SCRIPT, SCRIPT_L3C8_JUMP},
    {PCON_NONE},
    },
   {{CCON_NONE},
    }
   },
  {FLAG_JUMP_4, CONVOY_8, // SCRIPT_L3C8_JUMP
   STYPE_CONVOY_JUMP, -1000, 6000, 12, 1, {0},
    {{PCON_SETUP_CONVOY, {0, 0, 200}, RESULT_NONE},
     {PCON_ALWAYS_PASS, {0}, RESULT_NEXT_SCRIPT, SCRIPT_L3C8_MOVE, MSG_L3M15, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {PCON_NONE},
    },
   {{CCON_NONE},
   }
   },
  {TEAM_ENEMY, CONVOY_8, // SCRIPT_L3C8_MOVE
   STYPE_APPROACH_CONVOY_OFFSET, CONVOY_1, 0, 12, 1, {0, 250},
   {{PCON_CONVOY_DIST, {CONVOY_1, 300}, RESULT_NEXT_SCRIPT, SCRIPT_L3C8_ATTACK},
    {PCON_NONE},
    },
   {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_8, // SCRIPT_L3C8_ATTACK
   STYPE_STOP, 0, 0, 0, 1, {0},
   {{PCON_NONE},
    },
   {{CCON_CONVOY_DESTROYED, {CONVOY_2}, RESULT_NEXT_SCRIPT, SCRIPT_L3C8_FOLLOW},
    {CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_8, // SCRIPT_L3C8_FOLLOW
   STYPE_APPROACH_CONVOY_OFFSET, CONVOY_1, 0, 12, 1, {0, 250},
   {{PCON_NONE},
    },
   {{CCON_NONE},
    }
   },

  }

 }

};

/*
struct scriptstruct script_data [NO_LEVELS] [LEVEL_SCRIPTS] =
{
 {
  {0},
 },

// STAGE 1

// LEVEL 2!!





};

struct levelstruct level_data [NO_LEVELS] [LEVEL_SHIPS] =
{
 {
  {TEAM_NONE}
 }, // 0 value, because


};
*/

//void checkstars(int num);

void init_level(void)
{

// int i;

// init_bullets();
 init_bullets();
 init_ships();

 arena.end_stage = 0;
 arena.only_player = 0;
 if (arena.players == 2)
  arena.only_player = -1;
 arena.mission_over = 0;
 arena.all_wships_lost = 0;
 arena.game_over = 0;
 arena.time = 0;
 arena.subtime = 0;

 int a, e, i, j;
 int scr = 0;

 for (i = 0; i < COMMS; i ++)
 {
//  comm [i].message = -1;
  comm [i].exists = 0;
  ctext[i][0].text[0] = END_MESSAGE;
 }

 for (i = 0; i < RUNNING_SCRIPTS; i ++)
 {
  running_script [i] = -1;
 }

 for (i = 0; i < NO_CONVOYS; i ++)
 {
  convoy[i].active = 0;// NO: -1; // 1 = active, 0 = inactive but was active, -1 = never active
  convoy[i].approach_convoy = -1;
  convoy[i].target_offset_x = 0;
  convoy[i].target_offset_y = 0;
  convoy[i].ships = 0;
 }

 for (i = 0; i < SRECORDS; i ++)
 {
  for (j = 0; j < NO_SHIP_TYPES; j ++)
  {
   arena.srecord [i] [0] [j] = 0;
   arena.srecord [i] [1] [j] = 0;
  }
 }

 setup_special_stage_conditions();

 for (i = 0; i < LEVEL_SCRIPTS; i ++)
 {
  switch(ol[arena.stage].script_data[i].type)
  {
   case STYPE_SETUP_CONVOY:
   case STYPE_SPECIAL_CONVOY:
    running_script [scr] = i;
    start_script(scr);
    running_script [scr] = ol[arena.stage].script_data[i].pcon[0].result; // result_type must always be RESULT_NEXT_SCRIPT
    start_script(scr);
    scr++;
    break;
   case STYPE_START_WAIT:
    running_script [scr] = i;//ol[arena.stage].script_data[i].pcon[0].result; // result_type must always be RESULT_NEXT_SCRIPT
    start_script(scr);
    scr++;
    break;
  }
 }
/*
extern FONT* small_font;
  textprintf_ex(screen, small_font, 10, 20, -1, 1, "%i, %i", convoy[CONVOY_4].x, convoy[CONVOY_4].y);
  do
  {i++;} while(!key[KEY_1]);
*/

#define FAST_SLEW 55
#define SLOW_SLEW 15
#define TURN 40
#define LOW_TURN 25
#define WSHIP_TURN 2
#define LD ol[arena.stage].level_data[s]

 int s;

// init_stars(0);

 for (s = 0; s < LEVEL_SHIPS; s ++)
 {
  if (LD.team == TEAM_NONE)
   break;
  if ((LD.flags & FLAG_JUMP_1)
   || (LD.flags & FLAG_JUMP_2)
   || (LD.flags & FLAG_JUMP_3)
   || (LD.flags & FLAG_JUMP_4))
   continue; // will jump in later
  a = LD.team;
  e = create_ship(LD.type, LD.team);
  if (e == -1)
   exit(1); // this should not be possible. But just in case, exit rather than crash.
//  EE.x = PP.x + LD.x;
  //EE.y = PP.y + LD.y;
  ship_from_level_data(s, a, e);
 }

 calculate_threat();

/*
extern FONT* small_font;
  textprintf_ex(screen, small_font, 10, 20, -1, 1, "%i, %i", convoy[CONVOY_4].x, convoy[CONVOY_4].y);
  do
  {i++;} while(!key[KEY_2]);
//display_team();
*/

}
/*
extern struct starstruct star [2] [NO_STARS];

void checkstars(int num)
{
 if (star[0][0].x != star[0][0].old_x)
  exit(num);

}
*/

void convoy_jumps_in(int cv, int flag)
{

 int a, e, s;
//player[0].ships ++;
 for (s = 0; s < LEVEL_SHIPS; s ++)
 {
  if (LD.team == TEAM_NONE)
   break;
  if (!(LD.flags & flag))
   continue;
  a = LD.team;
  e = create_ship(LD.type, LD.team);
//  player[0].rocket_burst ++;
  if (e == -1)
   exit(1); // this should not be possible. But just in case, exit rather than crash.

  ship_from_level_data(s, a, e);
  EE.x_speed = 0;
  EE.y_speed = 0;

  jump_clouds(a, e);

 }

 calculate_threat();

}


void ship_from_level_data(int s, int a, int e)
{

 int j;


  EE.convoy = LD.convoy;
  if (EE.convoy != CONVOY_NONE)
  {
   for (j = 0; j < CONVOY_ARRANGEMENTS; j ++)
   {
    EE.convoy_angle [j] = LD.convoy_position [j] [0]; //LD.convoy_angle [j];
    EE.convoy_dist [j] = LD.convoy_position [j] [1]<<10;//LD.convoy_dist [j] << 10;
   }


   convoy_position(a, e);
   EE.x = EE.convoy_x;
   EE.y = EE.convoy_y;
   EE.angle = convoy[EE.convoy].angle;
   EE.x_speed = convoy[EE.convoy].x_speed;
   EE.y_speed = convoy[EE.convoy].y_speed;
   convoy[EE.convoy].ships ++;

  }
   else
   {
    EE.x = LD.x;
    EE.y = LD.y;
    EE.x_speed = 0;//LD.x_speed;
    EE.y_speed = 0;//LD.y_speed;
    EE.angle += ANGLE_4;
   }
//  EE.speed = hypot(EE.y_speed, EE.x_speed); // is relevant for wships only.
  EE.action = LD.action;
  EE.mission = LD.mission;

  if (LD.group != 0)
  {
   if (LD.group_position == 0)
   {
    group_leader [LD.group] = e; // don't worry about a; groups are all the same a.
// assumes leader is first in list
   }

   EE.formation_position = LD.group_position;
   if (LD.group_position != 0)
   {
    EE.leader = group_leader [LD.group];
    formation_position(a, e);
    EE.angle = ship[a][EE.leader].angle;
    ship[a][EE.leader].formation_size ++;
    EE.x = EE.formation_x - xpart(EE.angle, 25000);
    EE.y = EE.formation_y - ypart(EE.angle, 25000);
   }
  }

  EE.command = 0;
  EE.spawn = 0;

  if (LD.flags != FLAGS)
  {
   if (LD.flags & FLAG_COMMAND_1)
    add_ship_to_command(a, e, 1); // must be 1
   if (LD.flags & FLAG_COMMAND_2)
    add_ship_to_command(a, e, 2); // must be 2
   if (LD.flags & FLAG_COMMAND_3)
    add_ship_to_command(a, e, 3); // must be 3
   if (LD.flags & FLAG_ALPHA)
    add_ship_to_letter(a, e, 0);
   if (LD.flags & FLAG_BETA)
    add_ship_to_letter(a, e, 1);
   if (LD.flags & FLAG_GAMMA)
    add_ship_to_letter(a, e, 2);
   if (LD.flags & FLAG_SPAWN_1)
    EE.spawn = 1;
   if (LD.flags & FLAG_SPAWN_2)
    EE.spawn = 2;
   if (LD.flags & FLAG_SPAWN_3)
    EE.spawn = 3;
  }


  EE.old_x = EE.x;
  EE.old_y = EE.y;


  setup_new_ship(a, e, LD.subtype);

}


void setup_player_wing(int p, int type, int number)
{

 int e;
 int a = TEAM_FRIEND;

 e = create_ship(type, TEAM_FRIEND);
 if (e == -1)
  return; // shouldn't happen

 EE.player_leader = p;
 if (p == 0)
  EE.leader = TARGET_P1;
   else
    EE.leader = TARGET_P2;
 EE.mission = MISSION_PLAYER_WING;
 EE.action = ACT_WING_FORM;
 EE.player_command = COMMAND_COVER;

 int angle = 0, dist = 0;

 switch(number)
 {
  case 0: angle = PP.angle + ANGLE_4 + ANGLE_16; dist = 60000; break;
  case 1: angle = PP.angle - ANGLE_4 - ANGLE_16; dist = 60000; break;
  case 2: angle = PP.angle + ANGLE_4 + ANGLE_16; dist = 120000; break;
  case 3: angle = PP.angle - ANGLE_4 - ANGLE_16; dist = 120000; break;
 }

 EE.x = PP.x + xpart(angle, dist);
 EE.y = PP.y + ypart(angle, dist);

// EE.x = 0;
// EE.y = 0;

 EE.formation_position = number + 1;
 PP.wing [number] = e;
 PP.wing_size ++;

 EE.x_speed = PP.x_speed;
 EE.y_speed = PP.y_speed;

 EE.old_x = EE.x;
 EE.old_y = EE.y;

 setup_new_ship(TEAM_FRIEND, e, VAR_NONE);
 EE.target_range = 550000;
 EE.base_target_range = 450000;

}


// This function is called at the start of each stage. It sets variables that are to be used by
//  special_convoy.
void setup_special_stage_conditions(void)
{

 switch(arena.stage)
 {
// stage 1 has no scons
  case 2:
   switch(grand(3))
   {
/*
    case 0: scon [S2SCON_C4DIST] = 2000; scon [S2SCON_C5DIST] = 4000; scon [S2SCON_C6DIST] = 6000; break;
    case 1: scon [S2SCON_C5DIST] = 2000; scon [S2SCON_C6DIST] = 4000; scon [S2SCON_C4DIST] = 6000; break;
    case 2: scon [S2SCON_C6DIST] = 2000; scon [S2SCON_C4DIST] = 4000; scon [S2SCON_C5DIST] = 6000; break;*/
    case 0: scon [S2SCON_C4WAIT] = 15; scon [S2SCON_C5WAIT] = 30; scon [S2SCON_C6WAIT] = 60; break;
    case 1: scon [S2SCON_C5WAIT] = 15; scon [S2SCON_C6WAIT] = 30; scon [S2SCON_C4WAIT] = 60; break;
    case 2: scon [S2SCON_C6WAIT] = 15; scon [S2SCON_C4WAIT] = 30; scon [S2SCON_C5WAIT] = 60; break;
   }
   scon [S2SCON_C4WAIT] += grand(10);
   scon [S2SCON_C5WAIT] += grand(10);
   scon [S2SCON_C6WAIT] += grand(10);
   scon [S2SCON_C7ANGLE] = (-ANGLE_8 + ANGLE_16 + (ANGLE_3 * grand(3))) & ANGLE_MASK;
   scon [S2SCON_C7WAIT] = 50 + grand(20);
   scon [S2SCON_C8ANGLE] = (scon[S2SCON_C7ANGLE] + pos_or_neg(ANGLE_3)) & ANGLE_MASK;
   scon [S2SCON_C8WAIT] = 210 + grand(30);
   break;


 }


}


// This function takes care of convoys with special starting conditions.
//  Eg convoys with randomised positions
void special_convoy(int cv)
{
//return;
 switch(arena.stage)
 {


  case 1:
// Stage 1 has the following special conditions:
// - Enemy convoys (consisting of individual ships) start at specified x values but random y values.
//   These convoys then converge on convoy 1, which is the big ship and always starts in a fixed position.
//   We can assume that if this function is called, the convoy is one of these.
   convoy[cv].y = - 3000000 + grand(6000000);
//   convoy[cv].y += grand(2000000);
//   convoy[cv].y -= grand(2000000);
   break;
  case 2:
// - Convoys 4, 5 and 6 are small ships surrounding the base in loose clouds. They come together to engage
//   with the friendly convoys. One is close to base and one is far - this is random.
// - Convoy 7 consists of carriers and a few wship escorts. It jumps in at a semi-random location.
//   If it isn't dealt with, the nearest friendly convoy will send a few light cruisers to attack it.
// - Convoy 8 consists of 2 heavy wships and a 3 escorts with EANTI. They jump in at a semi-random location
//   different to Convoy 7's and proceeds to the base.
   switch(cv)
   {
    case CONVOY_4:
     convoy[cv].x = xpart(-ANGLE_8, 2000);//scon [S2SCON_C4DIST]<<10);
     convoy[cv].y = ypart(-ANGLE_8, 2000);//scon [S2SCON_C4DIST]<<10);
     convoy[cv].angle = -ANGLE_8;
     convoy[cv].fine_angle = CV.angle << FINE_ANGLE_BITSHIFT;
     break;
    case CONVOY_5:
     convoy[cv].x = xpart(-ANGLE_8 + ANGLE_3, 2000);//scon [S2SCON_C5DIST]<<10);
     convoy[cv].y = ypart(-ANGLE_8 + ANGLE_3, 2000);//scon [S2SCON_C5DIST]<<10);
     convoy[cv].angle = -ANGLE_8 + ANGLE_3;
     convoy[cv].fine_angle = CV.angle << FINE_ANGLE_BITSHIFT;
     break;
    case CONVOY_6:
     convoy[cv].x = xpart(-ANGLE_8 - ANGLE_3, 2000);//scon [S2SCON_C6DIST]<<10);
     convoy[cv].y = ypart(-ANGLE_8 - ANGLE_3, 2000);//scon [S2SCON_C6DIST]<<10);
     convoy[cv].angle = -ANGLE_8 - ANGLE_3;
     convoy[cv].fine_angle = CV.angle << FINE_ANGLE_BITSHIFT;
     break;
    case CONVOY_7:
     convoy[cv].x = xpart(scon [S2SCON_C7ANGLE], 11000<<10);
     convoy[cv].y = ypart(scon [S2SCON_C7ANGLE], 11000<<10);
     convoy[cv].angle = (scon [S2SCON_C7ANGLE] + ANGLE_4) & ANGLE_MASK;
     convoy[cv].fine_angle = CV.angle << FINE_ANGLE_BITSHIFT;
//     convoy[cv].x_speed = xpart(scon [S2SCON_C7ANGLE], 9000<<10);
     break;
    case CONVOY_8:
     convoy[cv].x = xpart(scon [S2SCON_C8ANGLE], 4000<<10);
     convoy[cv].y = ypart(scon [S2SCON_C8ANGLE], 4000<<10);
     break;
   }
   break;
 }




}


void add_ship_to_command(int a, int e, int command)
{
/*
 int e2;
 int highest_rank = 0;

 for (e2 = 0; e2 < NO_SHIPS; e2 ++)
 {
  if (ship[a][e2].type == SHIP_NONE
   || e2 == e)
   continue;
  if (ship[a][e2].command == command
   && ship[a][e2].command_rank > highest_rank)
    highest_rank = ship[a][e2].command_rank;
 }
*/
 EE.command = command;
// EE.command_rank = highest_rank + 1;

// It is assumed in message function that ships' command ranks will be in order wrt their e indices

}

void add_ship_to_letter(int a, int e, int letter)
{

 int e2;
 int highest_rank = 0;

 for (e2 = 0; e2 < NO_SHIPS; e2 ++)
 {
  if (ship[a][e2].type == SHIP_NONE
   || e2 == e)
   continue;
  if (ship[a][e2].letter == letter
   && ship[a][e2].letter_rank > highest_rank)
    highest_rank = ship[a][e2].letter_rank;
 }

 EE.letter = letter;
 EE.letter_rank = highest_rank + 1;


}

void setup_new_ship(int a, int e, int subtype)
{

  EE.turn_speed = 0;
  EE.test_destroyed = 0;
  EE.wship_throttle = 4;
  EE.engine_power = eclass[EE.type].engine_output;
  EE.turn_speed = WSHIP_TURN; // default - is changed later for fighters

  int tur;

  switch(EE.type)
  {
   case SHIP_OLD3:
    set_turret(a, e, 0, TURRET_HEAVY, 0);
    set_turret(a, e, 1, TURRET_HEAVY, 0);
    set_turret(a, e, 2, TURRET_BASIC, ANGLE_2);
    EE.shield_energy_cost = 15;
    EE.max_shield = 35000;
    break;
   case SHIP_FRIEND3:
    set_turret(a, e, 0, TURRET_BASIC, 0);
    set_turret(a, e, 1, TURRET_HEAVY, 0);
    set_turret(a, e, 2, TURRET_BASIC, ANGLE_2);
    EE.shield_energy_cost = 12;
    EE.max_shield = 30000;
    break;
   case SHIP_OLD2:
    tur = TURRET_HEAVY;
    if (subtype == VAR_ANTI)
     tur = TURRET_BASIC;
    set_turret(a, e, 0, tur, 0);
    set_turret(a, e, 1, TURRET_BASIC, ANGLE_2);
    EE.shield_energy_cost = 20;
    EE.max_shield = 20000;
    break;
   case SHIP_SCOUT2:
    tur = TURRET_EHEAVY;
    if (subtype == VAR_LONG)
     tur = TURRET_ELONG;
    if (subtype == VAR_ANTI)
     tur = TURRET_EANTI;
    set_turret(a, e, 0, tur, 0);
    set_turret(a, e, 1, TURRET_EBASIC, ANGLE_2);
    EE.shield_energy_cost = 25;
    EE.max_shield = 20000;
    break;
   case SHIP_SCOUT3:
    tur = TURRET_EHEAVY;
    if (subtype == VAR_LONG)
     tur = TURRET_ELONG;
    if (subtype == VAR_ANTI)
     tur = TURRET_EANTI;
    set_turret(a, e, 0, tur, 0);
    set_turret(a, e, 1, TURRET_EHEAVY, 0);
    set_turret(a, e, 2, TURRET_EBASIC, ANGLE_2);
    EE.shield_energy_cost = 12;
    EE.max_shield = 40000;
    break;
   case SHIP_SCOUTCAR:
    set_turret(a, e, 0, TURRET_EBASIC, ANGLE_2);
    EE.shield_energy_cost = 25;
    EE.max_shield = 15000;
    EE.can_launch = 1;
    break;
   case SHIP_FIGHTER:
    EE.turn_speed = 33;
    EE.attack = ATTACK_PREF_FIGHTER;
    EE.target_range = 350000;
    EE.base_target_range = 350000;
    EE.away_dist = 200000;
    EE.attack_range = 300000;
    EE.max_shield = 2200;
    EE.shield_up = 1;
    EE.shield_recharge = 4;
    break;
   case SHIP_ESCOUT:
    EE.turn_speed = 25;
    EE.attack = ATTACK_PREF_FIGHTER;
    EE.target_range = 550000;
    EE.base_target_range = 550000;
    EE.away_dist = 100000;
    EE.attack_range = 400000;
    EE.max_shield = 3500;
    EE.shield_up = 1;
    EE.shield_recharge = 6;
    break;
   case SHIP_EINT:
    EE.turn_speed = 36;
    EE.attack = ATTACK_PREF_FIGHTER;
    EE.target_range = 750000;
    EE.base_target_range = 750000;
    EE.away_dist = 100000;
    EE.attack_range = 400000;
    EE.max_shield = 2500;
    EE.shield_up = 1;
    EE.shield_recharge = 5;
    break;
   case SHIP_BOMBER:
    EE.turn_speed = 20;
    EE.attack = ATTACK_ONLY_WSHIP;
    EE.target_range = 99000000;
    EE.base_target_range = 99000000;
    EE.away_dist = 500000;
    EE.attack_range = 300000;
    EE.max_shield = 4000;
    EE.shield_up = 1;
    EE.shield_recharge = 12;
    break;
   case SHIP_FIGHTER_FRIEND:
    EE.turn_speed = TURN;
    EE.attack = ATTACK_PREF_FIGHTER;
    EE.target_range = 550000;
    EE.base_target_range = 450000; // this is also set in setup_player_wing
    EE.away_dist = 100000;
    EE.attack_range = 400000;
    EE.max_shield = 2000;
    EE.shield_up = 1;
    EE.shield_recharge = 3;
    break;
   case SHIP_EBASE:
    set_turret(a, e, 0, TURRET_EANTI, 0);
    set_turret(a, e, 1, TURRET_EANTI, ANGLE_2);
    set_turret(a, e, 2, TURRET_EHEAVY, 0);
    set_turret(a, e, 3, TURRET_EHEAVY, ANGLE_2);
/*    set_turret(a, e, 0, TURRET_ELONG, 0);
    set_turret(a, e, 1, TURRET_ELONG, ANGLE_2);
    set_turret(a, e, 2, TURRET_ELONG, 0);
    set_turret(a, e, 3, TURRET_ELONG, ANGLE_2);*/
    set_turret(a, e, 4, TURRET_EBEAM, 0);
    EE.shield_energy_cost = 6; // 13
    EE.max_shield = 500000; // 90000
    EE.turning = pos_or_neg(1);
    break;
  }
    EE.shield = EE.max_shield;
    EE.shield_threshhold = EE.max_shield / 3;



}


void set_turret(int a, int e, int t, int type, int rest_angle)
{

 EE.turret_type [t] = type;
 EE.turret_slew [t] = 1;
 EE.turret_recycle [t] = 0;
 EE.turret_rest_angle [t] = rest_angle;
 EE.turret_angle [t] = (EE.angle + rest_angle) & ANGLE_MASK;
 EE.turret_side [t] = (t+e) & 1;

 switch(type)
 {
  case TURRET_BASIC:
   EE.turret_turn [t] = FAST_SLEW;
   EE.turret_recycle_time [t] = 5;
   EE.turret_energy_max [t] = 500; // turret will absorb energy up to this point
   EE.turret_energy_use [t] = EE.turret_energy_max [t]; // how much a single shot costs
   EE.turret_class [t] = ATTACK_ONLY_FIGHTER; // anti-fighter
   EE.turret_target_range [t] = 500000;
   EE.turret_firing_range [t] = 400000;
   EE.turret_bullet_speed [t] = 9000;
   break;
  case TURRET_HEAVY:
   EE.turret_turn [t] = SLOW_SLEW;
   EE.turret_recycle_time [t] = 30;
   EE.turret_energy_max [t] = 3500; // turret will absorb energy up to this point
   EE.turret_energy_use [t] = EE.turret_energy_max [t]; // how much a single shot costs
   EE.turret_class [t] = ATTACK_PREF_WSHIP; // anti_wship
   EE.turret_target_range [t] = 550000;
   EE.turret_firing_range [t] = 530000;
   EE.turret_bullet_speed [t] = 5000;
   break;
  case TURRET_EBASIC:
   EE.turret_turn [t] = FAST_SLEW;
   EE.turret_recycle_time [t] = 7;
   EE.turret_energy_max [t] = 6000; // turret will absorb energy up to this point then start firing
   EE.turret_energy_use [t] = 2000; // how much a single shot costs
   EE.turret_class [t] = ATTACK_ONLY_FIGHTER; // anti-fighter
   EE.turret_target_range [t] = 600000;
   EE.turret_firing_range [t] = 550000;
   EE.turret_burst [t] = BURST_CHARGING;
   EE.turret_status [t] = 6;
   EE.turret_bullet_speed [t] = 5000;
   break;
  case TURRET_EBEAM:
   EE.turret_turn [t] = SLOW_SLEW;
   EE.turret_recycle_time [t] = 200;
   EE.turret_energy_max [t] = 30000; // turret will absorb energy up to this point then start firing
   EE.turret_energy_use [t] = 30000; // how much a single shot costs
   EE.turret_class [t] = ATTACK_PREF_WSHIP; // anti-wship
   EE.turret_target_range [t] = 750000;
   EE.turret_firing_range [t] = 700000;
   EE.turret_burst [t] = BURST_CHARGING;
   EE.turret_status [t] = 0;
   EE.turret_bullet_speed [t] = 5000;
   break;
  case TURRET_EHEAVY:
   EE.turret_turn [t] = SLOW_SLEW;
   EE.turret_recycle_time [t] = 60;
   EE.turret_energy_max [t] = 7000; // turret will absorb energy up to this point
   EE.turret_energy_use [t] = EE.turret_energy_max [t]; // how much a single shot costs
   EE.turret_class [t] = ATTACK_PREF_WSHIP; // anti_wship
   EE.turret_target_range [t] = 550000;
   EE.turret_firing_range [t] = 500000;
   EE.turret_bullet_speed [t] = 3000;
   break;
  case TURRET_ELONG:
   EE.turret_turn [t] = SLOW_SLEW;
   EE.turret_recycle_time [t] = 40;
   EE.turret_energy_max [t] = 2000; // turret will absorb energy up to this point
   EE.turret_energy_use [t] = EE.turret_energy_max [t]; // how much a single shot costs
   EE.turret_class [t] = ATTACK_PREF_FIGHTER;
   EE.turret_target_range [t] = 950000;
   EE.turret_firing_range [t] = 800000;
   EE.turret_bullet_speed [t] = 4000;
   break;
  case TURRET_EANTI:
   EE.turret_turn [t] = FAST_SLEW;
   EE.turret_recycle_time [t] = 6;
   EE.turret_energy_max [t] = 10000; // turret will absorb energy up to this point then start firing
   EE.turret_energy_use [t] = 1500; // how much a single shot costs
   EE.turret_class [t] = ATTACK_PREF_FIGHTER;
   EE.turret_target_range [t] = 650000;
   EE.turret_firing_range [t] = 600000;
   EE.turret_burst [t] = BURST_CHARGING;
   EE.turret_status [t] = 6;
   EE.turret_bullet_speed [t] = 7500;
   EE.turret_status2 [t] = grand(2); // which side fires first - might as well randomise it
   break;

 }

/*
 switch(type)
 {
  case TURRET_BASIC:
   EE.turret_turn [t] = FAST_SLEW;
   EE.turret_recycle_time [t] = 7;
   EE.turret_energy_max [t] = 700; // turret will absorb energy up to this point
   EE.turret_energy_use [t] = EE.turret_energy_max [t]; // how much a single shot costs
   EE.turret_class [t] = ATTACK_ONLY_FIGHTER; // anti-fighter
   EE.turret_target_range [t] = 500000;
   EE.turret_firing_range [t] = 400000;
   EE.turret_bullet_speed [t] = 9000;
   break;
  case TURRET_HEAVY:
   EE.turret_turn [t] = SLOW_SLEW;
   EE.turret_recycle_time [t] = 60;
   EE.turret_energy_max [t] = 7000; // turret will absorb energy up to this point
   EE.turret_energy_use [t] = EE.turret_energy_max [t]; // how much a single shot costs
   EE.turret_class [t] = ATTACK_ONLY_WSHIP; // anti_wship
   EE.turret_target_range [t] = 450000;
   EE.turret_firing_range [t] = 400000;
   EE.turret_bullet_speed [t] = 3000;
   break;
  case TURRET_EBASIC:
   EE.turret_turn [t] = FAST_SLEW;
   EE.turret_recycle_time [t] = 6;
   EE.turret_energy_max [t] = 10000; // turret will absorb energy up to this point then start firing
   EE.turret_energy_use [t] = 2000; // how much a single shot costs
   EE.turret_class [t] = ATTACK_ONLY_FIGHTER; // anti-fighter
   EE.turret_target_range [t] = 450000;
   EE.turret_firing_range [t] = 400000;
   EE.turret_burst [t] = BURST_CHARGING;
   EE.turret_status [t] = 6;
   EE.turret_bullet_speed [t] = 5000;
   break;
  case TURRET_EBEAM:
   EE.turret_turn [t] = FAST_SLEW;
   EE.turret_recycle_time [t] = 100;
   EE.turret_energy_max [t] = 30000; // turret will absorb energy up to this point then start firing
   EE.turret_energy_use [t] = 30000; // how much a single shot costs
   EE.turret_class [t] = ATTACK_ONLY_WSHIP; // anti-wship
   EE.turret_target_range [t] = 450000;
   EE.turret_firing_range [t] = 400000;
   EE.turret_burst [t] = BURST_CHARGING;
   EE.turret_status [t] = 6;
   EE.turret_bullet_speed [t] = 5000;
   break;
  case TURRET_EHEAVY:
   EE.turret_turn [t] = SLOW_SLEW;
   EE.turret_recycle_time [t] = 60;
   EE.turret_energy_max [t] = 7000; // turret will absorb energy up to this point
   EE.turret_energy_use [t] = EE.turret_energy_max [t]; // how much a single shot costs
   EE.turret_class [t] = ATTACK_ONLY_WSHIP; // anti_wship
   EE.turret_target_range [t] = 450000;
   EE.turret_firing_range [t] = 400000;
   EE.turret_bullet_speed [t] = 3000;
   break;
  case TURRET_ELONG:
   EE.turret_turn [t] = SLOW_SLEW;
   EE.turret_recycle_time [t] = 40;
   EE.turret_energy_max [t] = 2000; // turret will absorb energy up to this point
   EE.turret_energy_use [t] = EE.turret_energy_max [t]; // how much a single shot costs
   EE.turret_class [t] = ATTACK_PREF_FIGHTER; // anti_wship
   EE.turret_target_range [t] = 950000;
   EE.turret_firing_range [t] = 800000;
   EE.turret_bullet_speed [t] = 4000;
   break;
  case TURRET_EANTI:
   EE.turret_turn [t] = FAST_SLEW;
   EE.turret_recycle_time [t] = 5;
   EE.turret_energy_max [t] = 10000; // turret will absorb energy up to this point then start firing
   EE.turret_energy_use [t] = 1500; // how much a single shot costs
   EE.turret_class [t] = ATTACK_PREF_FIGHTER; // anti-fighter
   EE.turret_target_range [t] = 550000;
   EE.turret_firing_range [t] = 500000;
   EE.turret_burst [t] = BURST_CHARGING;
   EE.turret_status [t] = 6;
   EE.turret_bullet_speed [t] = 6000;
   EE.turret_status2 [t] = grand(2); // which side fires first - might as well randomise it
   break;

 }
*/
}

//#define TESTING_LEVEL

void run_level(void)
{

 int i, j, cv;
 struct scriptstruct* SD;

 for (i = 0; i < RUNNING_SCRIPTS; i ++)
 {
  if (running_script [i] == -1)
   continue;
//  running_script [scr] = ol[arena.stage].script_data[i].pcon[0].next_script;
  running_script_subtime [i] ++;
  if (running_script_subtime [i] == 50)
  {
   running_script_subtime [i] = 0;
   running_script_time [i] ++;
  }
  SD = &ol[arena.stage].script_data[running_script [i]];

//  if (ol[arena.stage].script_data[running_script [i]].pcons > 0)
  {
   for (j = 0; j < POLLED_CONDITIONS; j ++)
   {
    if (SD->pcon[j].type == PCON_NONE)
     break;
    switch(SD->pcon[j].type)
    {
     case PCON_XY:
      cv = SD->convoy;
      if (abs(CV.x - (SD->pcon[j].val[0]<<10)) + abs(CV.y - (SD->pcon[j].val[1]<<10)) < 50000)
       pcon_met(i, j);
      break;
     case PCON_TIME:
      if (running_script_subtime [i] == 0
       && running_script_time [i] == SD->pcon[j].val[0])
       pcon_met(i, j);
       break;
     case PCON_LEVEL_TIME:
      if (arena.subtime == 0
       && arena.time == SD->pcon[j].val[0])
       pcon_met(i, j);
       break;
     case PCON_LEVEL_TIME_SPECIAL:
      if (arena.subtime == 0
       && arena.time == scon [SD->pcon[j].val[0]])
        pcon_met(i, j);
       break;
     case PCON_CONVOY_DIST:
      if (abs(convoy[SD->convoy].x - convoy[SD->pcon[j].val[0]].x)
        + abs(convoy[SD->convoy].y - convoy[SD->pcon[j].val[0]].y)
         < (SD->pcon[j].val[1]<<10))
        pcon_met(i, j);
       break;
     case PCON_CONVOY_DIST_APART:
      if (abs(convoy[SD->convoy].x - convoy[SD->pcon[j].val[0]].x)
        + abs(convoy[SD->convoy].y - convoy[SD->pcon[j].val[0]].y)
         > (SD->pcon[j].val[1]<<10))
        pcon_met(i, j);
       break;
     case PCON_ALWAYS_PASS:
      pcon_met(i, j);
      break;
    }
   }
  }


 }

 run_convoys();
 run_messages();

}

void pcon_met(int scr, int pcon)
{

 char check_message = 1;
 int s;
 struct scriptstruct* SD = &ol[arena.stage].script_data[running_script [scr]];

  switch(SD->pcon[pcon].result_type)
  {
   case RESULT_NEXT_SCRIPT:
//    if (ol[arena.stage].script_data[running_script [scr]].pcon[pcon].msg != MSG_NONE)
//     new_message(ol[arena.stage].script_data[running_script [scr]].pcon[pcon].msg);
    about_to_end_script(scr);
    running_script [scr] = SD->pcon[pcon].result;
    start_script(scr);
    break;
   case RESULT_START_SCRIPT:
    start_new_script(ol[arena.stage].script_data[running_script [scr]].pcon[pcon].result);
    break;
   case RESULT_END_SCRIPT:
//    if (ol[arena.stage].script_data[running_script [scr]].pcon[pcon].msg != MSG_NONE)
//     new_message(ol[arena.stage].script_data[running_script [scr]].pcon[pcon].msg);
    about_to_end_script(scr);
    running_script [scr] = -1;
    // note that SD pointer is still valid even after the script has been stopped. This
    //  means that we can use it below to deal with messages.
    break;
   case RESULT_LAUNCH:
    check_message = 0;
    if (activate_carrier_launch(SD->team,
     SD->convoy,
     SD->pcon[pcon].result))
     {
         check_message = 1;
//      if (ol[arena.stage].script_data[running_script [scr]].pcon[pcon].msg != MSG_NONE)
//       new_message(ol[arena.stage].script_data[running_script [scr]].pcon[pcon].msg);
// only display message if the carrier actually launched something.
     }
    break;
   case RESULT_JOIN_CONVOY:
    for (s = 0; s < NO_SHIPS; s ++)
    {
     if (ship[SD->team][s].type != SHIP_NONE
      && ship[SD->team][s].convoy == SD->convoy)
     {
      ship[SD->team][s].convoy = SD->pcon[pcon].result;
     }
    }
    about_to_end_script(scr);
    running_script [scr] = -1;
    break;
   case RESULT_CONVOY_JUMP_OUT:
    convoy_jump_out(SD->team, SD->convoy);
    running_script [scr] = -1;
    break;
   case RESULT_CHANGE_ARRANGE:
    convoy[SD->convoy].arrangement = SD->pcon[pcon].result;
    break;
   case RESULT_MISSION_OVER:
    if (arena.game_over > 0)
     break;
    if (arena.all_wships_lost > 0)
     break;
    arena.mission_over = 200;
    break;
  }

 if (check_message == 1)
 {
      if (SD->pcon[pcon].msg != MSG_NONE)
       new_message(SD->pcon[pcon].msg,
        SD->pcon[pcon].msg_from,
        SD->pcon[pcon].msg_from2,
        SD->pcon[pcon].msg_to);
 }

// NOTE: any changes in pcon_met may need to be replicated in ccon_met! It's not good
//  but I can't be bothered fixing it

}

void about_to_end_script(int scr)
{

 struct scriptstruct* SD = &ol[arena.stage].script_data[running_script [scr]];

 switch(SD->type)
 {
   case STYPE_APPROACH_CONVOY:
   case STYPE_APPROACH_CONVOY_OFFSET:
    convoy[SD->convoy].approach_convoy = -1;
    convoy[SD->convoy].target_offset_x = 0;
    convoy[SD->convoy].target_offset_y = 0;
    break;
 }

}


void start_new_script(int news)
{

 int s;

 for (s = 0; s < RUNNING_SCRIPTS; s ++)
 {
  if (running_script [s] == -1)
   break;
// don't worry about a sanity check - this should never fail
 }

  running_script [s] = news;
  start_script(s);

}


void start_script(int scr)
{
  int cv = ol[arena.stage].script_data[running_script [scr]].convoy;
  running_script_time [scr] = 0;
  running_script_subtime [scr] = 0;

  struct scriptstruct* SD = &ol[arena.stage].script_data[running_script [scr]];

  switch(SD->type)
  {
   case STYPE_SETUP_CONVOY:
   case STYPE_SPECIAL_CONVOY:
   case STYPE_CONVOY_JUMP:
   case STYPE_CONVOY_JUMP_SPECIAL:
    cv = SD->convoy;
    CV.active = 1; // 1 = active, 0 = inactive but was active, -1 = never active
    CV.x = SD->x << 10;
    CV.y = SD->y << 10;
    CV.arrangement = SD->arrange;
    CV.throttle = SD->throttle;
    CV.angle = radians_to_angle(atan2(SD->pcon[0].val [1] - SD->y, SD->pcon[0].val [0] - SD->x));
    CV.fine_angle = CV.angle << FINE_ANGLE_BITSHIFT;
    CV.x_speed = 0;
    CV.y_speed = 0;
    if (SD->pcon[0].val [2] != 0)
    {
     CV.x_speed = xpart(CV.angle, SD->pcon[0].val [2]);
     CV.y_speed = ypart(CV.angle, SD->pcon[0].val [2]);
    }
    CV.think_count = 0;
    CV.target_angle = 0; // set in start_script
    CV.turning = 0;
    CV.turn_count = 0;
    CV.turn_speed = 8;
    CV.can_turn = 1;
    if (SD->type == STYPE_SPECIAL_CONVOY
     || SD->type == STYPE_CONVOY_JUMP_SPECIAL)
     special_convoy(cv);
    if (SD->type == STYPE_CONVOY_JUMP
     || SD->type == STYPE_CONVOY_JUMP_SPECIAL)
     convoy_jumps_in(cv, SD->team);
    break;
   case STYPE_MOVE_XY:
//    convoy[0].angle = ANGLE_8;
//    convoy[1].angle = grand(ANGLE_1);
//exit(ol[arena.stage].script_data[running_script [scr]].convoy);
    CV.target_x = SD->x<<10;
    CV.target_y = SD->y<<10;
    CV.throttle = SD->throttle;
    CV.arrangement = SD->arrange;
    CV.can_turn = 1;
    set_convoy_turning(cv);
    break;
   case STYPE_MOVE_FORWARDS:
//    CV.target_x = SD->x<<10;
//    CV.target_y = SD->y<<10;
    CV.throttle = SD->throttle;
    CV.arrangement = SD->arrange;
    CV.turning = 0;
    CV.turn_count = 0;
    CV.can_turn = 0;
//    set_convoy_turning(cv);
    break;
   case STYPE_HOLD:
//    CV.target_x = SD->x<<10;
//    CV.target_y = SD->y<<10;
    CV.throttle = SD->throttle;
    CV.arrangement = SD->arrange;
    CV.can_turn = 0;
    set_convoy_turning(cv);
    break;
   case STYPE_APPROACH_CONVOY:
    set_approach_target(SD->convoy, SD->x, 0, 0);
    CV.throttle = SD->throttle;
    CV.arrangement = SD->arrange;
    CV.approach_convoy = SD->x;
    CV.can_turn = 1;
    set_convoy_turning(cv);
    break;
   case STYPE_APPROACH_CONVOY_OFFSET:
    CV.target_offset_x = SD->val[0]<<10;
    CV.target_offset_y = SD->val[1]<<10;
    set_approach_target(SD->convoy, SD->x, CV.target_offset_x, CV.target_offset_y);
    CV.throttle = SD->throttle;
    CV.arrangement = SD->arrange;
    CV.approach_convoy = SD->x;
    CV.can_turn = 1;
    set_convoy_turning(cv);
    break;
   case STYPE_STOP:
    CV.target_x = CV.x + xpart(CV.angle, 100000); // just so that the convoy keeps facing forwards
    CV.target_y = CV.y + ypart(CV.angle, 100000); // just so that the convoy keeps facing forwards;
    CV.throttle = 0;
    CV.x_speed = 0;
    CV.y_speed = 0;
    CV.can_turn = 0;
    CV.arrangement = SD->arrange;
    set_convoy_turning(cv);
    break;
  }

}

// This is called before destroy_ship
void condition_eship_destroyed(int e)
{
// need to have some kind of flag system for particular ships being destroyed
 int a = TEAM_ENEMY;
 int e2;

 char other_ships_exist = 0;
 char convoy_still_exists = 0;
 char other_wships_exist = 0;
 char other_fighters_exist = 0;
 int ship_class = eclass[EE.type].ship_class;

 for (e2 = 0; e2 < NO_SHIPS; e2++)
 {
  if (e2 == e)
   continue;
  if (ship[a][e2].type == SHIP_NONE)
   continue;
  other_ships_exist = 1;
  if (ship[a][e2].convoy == EE.convoy)
   convoy_still_exists = 1;
  if (eclass[ship[a][e2].type].ship_class == ECLASS_FIGHTER)
   other_fighters_exist = 1;
    else
     other_wships_exist = 1;
 }

/*
 if (EE.type == SHIP_SCOUTCAR
  && EE.can_launch == 1)
   condition_ecarrier_disabled(e); // probably unnecessary as its launch part should already have
    // been destroyed by wship_starts_breaking_up.
*/

 if (EE.convoy == CONVOY_NONE)
  convoy_still_exists = 0;

 int i, j;

 for (i = 0; i < RUNNING_SCRIPTS; i ++)
 {
  if (running_script [i] == -1)
   continue;

//  if (ol[arena.stage].script_data[running_script [i]].ccons > 0)
  {
   for (j = 0; j < CALLED_CONDITIONS; j ++)
   {
//    if (ol[arena.stage].script_data[running_script [i]].ccon[j].type == ccon_type)
//    {
     if (ol[arena.stage].script_data[running_script [i]].ccon[j].type == CCON_NONE)
      break;
     if (ol[arena.stage].script_data[running_script [i]].ccon[j].test == TEST_CONVOY_EXISTS
      && convoy[ol[arena.stage].script_data[running_script [i]].convoy].ships <= 0)
       break;
     switch(ol[arena.stage].script_data[running_script [i]].ccon[j].type)
     {
      case CCON_EALL_DESTROYED:
//      exit(1);
       if (other_ships_exist == 0)
        ccon_met(i, j);
       break;
      case CCON_ECONVOY_DESTROYED_REMAIN:
       if (EE.convoy != CONVOY_NONE
        && EE.convoy == ol[arena.stage].script_data[running_script [i]].ccon[j].val [0]
        && convoy_still_exists == 0
        && other_wships_exist == 1)
         ccon_met(i, j);
       break;
      case CCON_ECONVOY_DESTROYED_FREMAIN:
       if (EE.convoy != CONVOY_NONE
        && EE.convoy == ol[arena.stage].script_data[running_script [i]].ccon[j].val [0]
        && convoy_still_exists == 0
        && other_wships_exist == 0
        && other_fighters_exist == 1)
         ccon_met(i, j);
       break;
      case CCON_EFIGHTERS_DESTROYED_REMAIN:
//      exit(2);
       if (ship_class == ECLASS_FIGHTER
        && other_wships_exist == 1
        && other_fighters_exist == 0)
         ccon_met(i, j);
       break;
      case CCON_EWSHIPS_DESTROYED_FREMAIN:
//      exit(3);
       if (ship_class == ECLASS_WSHIP
        && other_wships_exist == 0
        && other_fighters_exist == 1)
         ccon_met(i, j);
       break;
      case CCON_CONVOY_DESTROYED: // also in condition_fship_destroyed
       if (EE.convoy != CONVOY_NONE
        && EE.convoy == ol[arena.stage].script_data[running_script [i]].ccon[j].val [0]
        && convoy_still_exists == 0)
         ccon_met(i, j);
       break;

     }

//    }
   }
  }
 }
/*
 if (convoy_still_exists == 0)
  call_condition(CCON_CONVOY_DESTROYED, EE.convoy, other_ships_present, 0);

 if (other_ships_present == 0)
  call_condition(CCON_ALL_ENEMIES_DESTROYED, 0, 0, 0);

CCON_SHIP_DESTROYED,
CCON_ECONVOY_DESTROYED_REMAIN, // convoy destroyed! But some wships (+fighters maybe) remain
CCON_ECONVOY_DESTROYED_FREMAIN, // convoy and all ewships destroyed! But some fighters remain
CCON_ECONVOY_DESTROYED_ALL, // called if this was the last convoy
CCON_EFIGHTERS_DESTROYED_REMAIN, // called if all efighters (inc bombers) destroyed, but ewships remain
CCON_EALL_DESTROYED

*/
}

// This is only called when a carrier's launch part is destroyed, or if a carrier
//  that could launch is destroyed. It isn't called when a carrier with can_launch == 0
//  is destroyed.
void condition_ecarrier_disabled(int e)
{

 int e2;
 int a = TEAM_ENEMY;

 for (e2 = 0; e2 < NO_SHIPS; e2++)
 {
  if (e2 == e)
   continue;
  if (ship[a][e2].type != SHIP_SCOUTCAR)
   continue;
  if (ship[a][e2].can_launch == 1)
   return;  // fail
 }


 int i, j;

 for (i = 0; i < RUNNING_SCRIPTS; i ++)
 {
  if (running_script [i] == -1)
   continue;
  for (j = 0; j < CALLED_CONDITIONS; j ++)
  {
//     if (ol[arena.stage].script_data[running_script [i]].ccon[j].type == CCON_NONE)
//      break;
     if (ol[arena.stage].script_data[running_script [i]].ccon[j].type == CCON_ECARRIERS_DISABLED)
     {
      ccon_met(i, j);
     }
  }
 }

}

void condition_fship_destroyed(int e)
{


 int a = TEAM_FRIEND;
 int e2;

 char other_ships_exist = 0;
 char convoy_still_exists = 0;
 char other_wships_exist = 0;
 char other_fighters_exist = 0;
 int ship_class = eclass[EE.type].ship_class;

 for (e2 = 0; e2 < NO_SHIPS; e2++)
 {
  if (e2 == e)
   continue;
  if (ship[a][e2].type == SHIP_NONE)
   continue;
  other_ships_exist = 1;
  if (ship[a][e2].convoy == EE.convoy)
   convoy_still_exists = 1;
  if (eclass[ship[a][e2].type].ship_class == ECLASS_FIGHTER)
   other_fighters_exist = 1;
    else
     other_wships_exist = 1;
 }


 if (EE.convoy == CONVOY_NONE)
  convoy_still_exists = 0;

 int i, j;

 for (i = 0; i < RUNNING_SCRIPTS; i ++)
 {
  if (running_script [i] == -1)
   continue;

//  if (ol[arena.stage].script_data[running_script [i]].ccons > 0)
  {
   for (j = 0; j < CALLED_CONDITIONS; j ++)
   {
//    if (ol[arena.stage].script_data[running_script [i]].ccon[j].type == ccon_type)
//    {
     if (ol[arena.stage].script_data[running_script [i]].ccon[j].type == CCON_NONE)
      break;
     switch(ol[arena.stage].script_data[running_script [i]].ccon[j].type)
     {
      case CCON_FWSHIPS_DESTROYED:
       if (other_wships_exist == 0)
        ccon_met(i, j); // will usually (always?) end the mission with failure
       break;
      case CCON_FCONVOY_DESTROYED_REMAIN:
       if (EE.convoy != CONVOY_NONE
        && EE.convoy == ol[arena.stage].script_data[running_script [i]].ccon[j].val [0]
        && convoy_still_exists == 0
        && other_wships_exist == 1)
         ccon_met(i, j);
       break;
      case CCON_FFIGHTERS_DESTROYED_REMAIN:
       if (ship_class == ECLASS_FIGHTER
        && other_wships_exist == 1
        && other_fighters_exist == 0)
         ccon_met(i, j);
       break;
      case CCON_CONVOY_DESTROYED: // also in condition_eship_destroyed
       if (EE.convoy != CONVOY_NONE
        && EE.convoy == ol[arena.stage].script_data[running_script [i]].ccon[j].val [0]
        && convoy_still_exists == 0)
         ccon_met(i, j);
       break;


     }

   }
  }
 }



}


void ccon_met(int scr, int ccon)
{

 char check_message = 1;

  struct scriptstruct* SD = &ol[arena.stage].script_data[running_script [scr]];

  switch(ol[arena.stage].script_data[running_script [scr]].ccon[ccon].result_type)
  {
   case RESULT_NEXT_SCRIPT:
//    if (ol[arena.stage].script_data[running_script [scr]].ccon[ccon].msg != MSG_NONE)
//     new_message(ol[arena.stage].script_data[running_script [scr]].ccon[ccon].msg);
    about_to_end_script(scr);
    running_script [scr] = ol[arena.stage].script_data[running_script [scr]].ccon[ccon].result;
    start_script(scr);
    break;
   case RESULT_START_SCRIPT:
    start_new_script(ol[arena.stage].script_data[running_script [scr]].ccon[ccon].result);
    break;
   case RESULT_END_SCRIPT:
//    if (ol[arena.stage].script_data[running_script [scr]].ccon[ccon].msg != MSG_NONE)
//     new_message(ol[arena.stage].script_data[running_script [scr]].ccon[ccon].msg);
    about_to_end_script(scr);
    running_script [scr] = -1;
    break;
   case RESULT_LAUNCH:
    check_message = 0;
    if (activate_carrier_launch(ol[arena.stage].script_data[running_script [scr]].team,
     ol[arena.stage].script_data[running_script [scr]].convoy,
     ol[arena.stage].script_data[running_script [scr]].ccon[ccon].result))
     {
       check_message = 1;
//      if (ol[arena.stage].script_data[running_script [scr]].ccon[ccon].msg != MSG_NONE)
//       new_message(ol[arena.stage].script_data[running_script [scr]].ccon[ccon].msg);
// only display message if the carrier actually launched something.
     }
    break;
   case RESULT_CONVOY_JUMP_OUT:
    convoy_jump_out(ol[arena.stage].script_data[running_script [scr]].team, ol[arena.stage].script_data[running_script [scr]].convoy);
    running_script [scr] = -1;
    break;
   case RESULT_MISSION_OVER:
//    if (ol[arena.stage].script_data[running_script [scr]].ccon[ccon].msg != MSG_NONE)
//     new_message(ol[arena.stage].script_data[running_script [scr]].ccon[ccon].msg);
    if (arena.game_over > 0)
     break;
    if (arena.all_wships_lost > 0)
     break;
    arena.mission_over = 200;
    break;
   case RESULT_MISSION_FAIL:
    if (arena.game_over > 0)
     break;
    if (arena.mission_over > 0)
     arena.mission_over = 0;
    arena.all_wships_lost = 200;
    break;
  }

 if (check_message == 1)
 {
    if (SD->ccon[ccon].msg != MSG_NONE)
     new_message(SD->ccon[ccon].msg,
      SD->ccon[ccon].msg_from,
      SD->ccon[ccon].msg_from2,
      SD->ccon[ccon].msg_to);
 }


// NOTE: any changes in ccon_met may need to be replicated in pcon_met! It's not good
//  but I can't be bothered fixing it

}

void convoy_jump_out(int a, int cv)
{
 int e;

 for (e = 0; e < NO_SHIPS; e++)
 {
  if (EE.type != SHIP_NONE
   && EE.convoy == cv)
  {
   EE.jump = 70 + grand(40);
  }

 }

}

void calculate_threat(void)
{

 int a = TEAM_ENEMY;
 int e, t;

 int old_threat = arena.threat;
 int old_fighter_threat = arena.fighter_threat;
 int old_wship_threat = arena.wship_threat;

 arena.fighter_threat = 0;
 arena.wship_threat = 0;

 for (e = 0; e < NO_SHIPS; e ++)
 {
  if (EE.type == SHIP_NONE)
   continue;
  switch(EE.type)
  {
   case SHIP_FIGHTER: arena.fighter_threat += 3; break;
   case SHIP_BOMBER: arena.fighter_threat += 15; break;
   case SHIP_ESCOUT: arena.fighter_threat += 6; break;
   case SHIP_EINT: arena.fighter_threat += 6; break;
   case SHIP_SCOUT2: arena.wship_threat += 5; break;
   case SHIP_SCOUT3: arena.wship_threat += 10; break;
   case SHIP_SCOUTCAR: arena.wship_threat += 8;
    if (EE.can_launch)
     arena.wship_threat += 50;
    break;
   case SHIP_EBASE: arena.wship_threat += 50; break;
  }

  if (eclass[EE.type].ship_class == ECLASS_WSHIP)
  {
   for (t = 0; t < eclass[EE.type].parts; t ++)
   {
    switch(EE.turret_type [t])
    {
     case TURRET_EBASIC: arena.wship_threat += 5; break;
     case TURRET_EBEAM: arena.wship_threat += 35; break;
     case TURRET_EHEAVY: arena.wship_threat += 25; break;
     case TURRET_ELONG: arena.wship_threat += 10; break;
     case TURRET_EANTI: arena.wship_threat += 10; break;
    }
   }
   if (EE.shield_generator)
    arena.wship_threat += 10;
  }

 }

 arena.threat = arena.fighter_threat + arena.wship_threat;


 int i, j;

 for (i = 0; i < RUNNING_SCRIPTS; i ++)
 {
  if (running_script [i] == -1)
   continue;
  {
   for (j = 0; j < CALLED_CONDITIONS; j ++)
   {
     if (ol[arena.stage].script_data[running_script [i]].ccon[j].type == CCON_NONE)
      break;
     switch(ol[arena.stage].script_data[running_script [i]].ccon[j].type)
     {
      case CCON_THREAT_FALLS:
       if (arena.threat <= ol[arena.stage].script_data[running_script [i]].ccon[j].val [0]
        && old_threat > ol[arena.stage].script_data[running_script [i]].ccon[j].val [0])
         ccon_met(i, j);
       break;
      case CCON_FTHREAT_FALLS:
       if (arena.fighter_threat <= ol[arena.stage].script_data[running_script [i]].ccon[j].val [0]
        && old_fighter_threat > ol[arena.stage].script_data[running_script [i]].ccon[j].val [0])
         ccon_met(i, j);
       break;
      case CCON_WSTHREAT_FALLS:
       if (arena.wship_threat <= ol[arena.stage].script_data[running_script [i]].ccon[j].val [0]
        && old_wship_threat > ol[arena.stage].script_data[running_script [i]].ccon[j].val [0])
         ccon_met(i, j);
       break;
     }
   }
  }
 }


}


// these need to be tested by running the game with show_convoy
// some of them are obviously wrong but they don't need to be exact at all
int get_convoy_speed(int cv)
{

 switch(CV.throttle)
 {
  case 0: return 1;
  case 1: return 32;
  case 2: return 93;
  case 3: return 93;
  case 4: return 150;
  case 5: return 170;
  case 6: return 209;
  case 7: return 266;
  case 8: return 325; // ?
  case 9: return 325; // ?
  case 10: return 384;
  case 11: return 377; // ?
  case 12: return 471;
  case 13: return 530;
  case 14: return 530;
  case 15: return 589;
  case 16: return 648;
  case 17: return 650;
  case 18: return 709;
  case 19: return 736;
  case 20: return 796;
  case 21: return 855;
  case 22: return 883;
  case 23: return 890;
  case 24: return 950;
  case 25: return 1003;
  case 26: return 1062;
  case 27: return 1090;
  case 28: return 1120;
  case 29: return 1170;
  case 30: return 1236;

 }

 return 1400;

}


// sets target_x/target_y values for cv1 to intercept cv2
void set_approach_target(int cv1, int cv2, int x_offset, int y_offset)
{

 if (convoy[cv1].throttle == 0)
  return; // will divide by zero otherwise

 int tx, ty;
 int cv1_speed = get_convoy_speed(cv1);//convoy[cv1].throttle;//*5;
 int cv2_speed = get_convoy_speed(cv2);//convoy[cv2].throttle;//*5;

 int dist = hypot(convoy[cv1].y - convoy[cv2].y - x_offset, convoy[cv1].x - convoy[cv2].x + y_offset);
 int time = dist / cv1_speed;

 tx = convoy[cv2].x + xpart(convoy[cv2].angle, time * cv2_speed) + x_offset;
 ty = convoy[cv2].y + ypart(convoy[cv2].angle, time * cv2_speed) + y_offset;

// let's do a second approximation to be just a little more accurate:

 dist = hypot(convoy[cv1].y - ty, convoy[cv1].x - tx);
 time = dist / cv1_speed;

 convoy[cv1].target_x = convoy[cv2].x + xpart(convoy[cv2].angle, time * cv2_speed) + x_offset;
 convoy[cv1].target_y = convoy[cv2].y + ypart(convoy[cv2].angle, time * cv2_speed) + y_offset;

}


int activate_carrier_launch(int a, int cv, int type)
{
 int e;
 char first = 1;

 for (e = 0; e < NO_SHIPS; e ++)
 {
  if (EE.convoy == cv
   && EE.type == SHIP_SCOUTCAR
   && EE.can_launch == 1)
   {
    EE.carrier_launch = type;
    if (first)
    {
     EE.carrier_launch_time = 0;
     first = 0;
    }
      else
       EE.carrier_launch_time = grand(50);
   }

 }

 if (first == 1)
  return 0; // nothing launched

 return 1;

}


void run_convoys(void)
{
// this should run even if there are no ships in the convoy,
//  as others may join.
 int cv;

 for (cv = 0; cv < NO_CONVOYS; cv ++)
 {
//   if (CV.active == -1)
//    break; // must be last one
   if (CV.active == 0)
    continue; // there may still be some left

   CV.x += CV.x_speed;
   CV.y += CV.y_speed;

   CV.x_speed += xpart(CV.angle, CV.throttle);
   CV.y_speed += ypart(CV.angle, CV.throttle);
   CV.x_speed *= WSHIP_DRAG;
   CV.x_speed >>= 10;
   CV.y_speed *= WSHIP_DRAG;
   CV.y_speed >>= 10;

   if (CV.can_turn == 0)
   {
//    CV.turning = 0;
    continue;
   }

// can only have turning things from this point on:

   CV.turn_count --;

   if (CV.approach_convoy != -1 && (arena.counter & 31) == 31)
   {
    set_approach_target(cv, convoy[cv].approach_convoy, convoy[cv].target_offset_x, convoy[cv].target_offset_y);
    CV.turn_count = 0;
   }

   if (CV.turn_count <= 0)
    set_convoy_turning(cv);

   CV.fine_angle += CV.turning;
   CV.fine_angle &= FINE_ANGLE_MASK;
   CV.angle = CV.fine_angle >> FINE_ANGLE_BITSHIFT;

  }
}

void set_convoy_turning(int cv)
{

    CV.target_angle = radians_to_angle(atan2(CV.target_y - CV.y, CV.target_x - CV.x));
    int angle_diff = angle_difference(CV.angle, CV.target_angle);
    if (abs(angle_diff) < 4)
    {
     CV.turning = 0;
     CV.turn_count = 100;
    }
     else
     {
      CV.turning = delta_turn_towards_angle(CV.angle, CV.target_angle, CV.turn_speed);
      CV.turn_count = (angle_diff<<FINE_ANGLE_BITSHIFT) / CV.turn_speed;
     }


}


void run_messages(void)
{

 int i;

 for (i = 0; i < COMMS; i ++)
 {
  if (comm[i].exists == 0)
   continue;
  if (comm[i].y_offset > 0)
   comm[i].y_offset --;
  if (comm[i].flash > 0)
  {
   comm[i].flash -= 10;
   if (comm[i].flash <= 0)
   {
    comm[i].flash = 0;
    comm[i].persist = 250;
   }
   continue;
  }
  if (comm[i].persist > 0)
  {
   comm[i].persist --;
   continue;
  }
//  if (arena.counter % 4 == 0)
  comm[i].fade += 10;
  if ((comm[i].fade) > 250)
  {
   if (i + 1 < COMMS)
   {
//    comm[i+1].y_offset += comm[i].y_offset + msg [comm[i].message].lines * 12 + 14 + 10;
    comm[i+1].y_offset += comm[i].y_offset + comm[i].lines * MESSAGE_LINE_SPACE + COMM_END_SPACE + COMM_BETWEEN + 4;
    if (comm[i].from_type != SHIP_NONE)
     comm[i+1].y_offset += MESSAGE_LINE_SPACE;
    if (comm[i].to != MSG_TO_ALL)
     comm[i+1].y_offset += MESSAGE_LINE_SPACE;
   }
   comm[i].exists = 0;
   ctext[comm[i].ctext][0].text[0] = END_MESSAGE;
   pack_messages(i);
  }
/*  if (comm[i].fade > 0)
  {
   comm[i].fade --;
   if (comm[i].fade <= 0)
   {
    comm[i].message = -1;
   }
   continue;
  }*/
 }

}

void new_message(int m, int m_from, int m_from2, int m_to)
{

 int e;
 int a = TEAM_FRIEND;

/*
 int lowest_rank = 1000; // actually lowest ranks are more important
 int lowest_rank_e;

 for (e = 0; e < NO_SHIPS; e ++)
 {
  if (EE.type == SHIP_NONE)
   continue;
  if (EE.command == m_from
   && EE.command_rank < lowest_rank)
  {
   lowest_rank = EE.command_rank;
   lowest_rank_e = e;
   ranked ++;
  }
 }

 if (lowest_rank == 1000) // no ships left from this command
  return;
*/

 int command_number = 0;
 int tone = NOTE_2C;
 int sound = WAV_SELECT0;

 for (e = 0; e < NO_SHIPS; e ++)
 {
  if (EE.type == SHIP_NONE)
   continue;
  if (EE.command == m_from)
  {
   command_number ++;
   if (command_number == m_from2)
    break;
  }
 }

 if (command_number < m_from2)
  return; // not enough ships left in command

 int i;

 for (i = 0; i <= COMMS; i ++)
 {
  if (i >= COMMS-1) // last one needs to remain empty
   return;
  if (comm [i].exists == 0)
  {
   comm [i].ctext = new_message_text(m, i);
   sound = ol[arena.stage].lmsg[m].val [0];
   tone = ol[arena.stage].lmsg[m].val [1];
   if (comm[i].ctext == -1)
   {
    comm[i].exists = 0;
    return; // fail! shouldn't happen as there are the same numbers of ctexts as comms
   }
   break;
  }
 }

 comm [i].exists = 1;
 comm [i].flash = 250;
 comm [i].fade = 0;
 comm [i].persist = 0;
 comm [i].y_offset = 0;
 comm [i].from_type = ship[a][e].type;
 comm [i].from_letter = ship[a][e].letter;
 comm [i].from_rank = ship[a][e].letter_rank;
 comm [i].to = m_to;
 comm [i].col_min = COL_BOX1;
 comm [i].col_max = COL_BOX4;
 if (m_to == MSG_TO_AM)
  comm[i].col_min = COL_BOX2;

 indicator(sound, tone, 250, -1);

}

int new_message_text(int m, int cm)
{
 int i;

 for (i = 0; i < COMMS; i ++)
 {
  if (ctext[i][0].text [0] == END_MESSAGE)
  {
   comm[cm].lines = generate_message(ol[arena.stage].lmsg[m], 245, MESSAGE_LINE_SPACE, COL_BOX4, ctext[i]);
   return i;
  }
 }

 return -1; // unlikely!
}

void pack_messages(int deleted)
{
 int i = deleted;

 while (i < COMMS - 1)
 {
  comm [i].exists = comm [i+1].exists;
  comm [i].ctext = comm [i+1].ctext;
  comm [i].flash = comm [i+1].flash;
  comm [i].persist = comm [i+1].persist;
  comm [i].fade = comm [i+1].fade;
  comm [i].y_offset = comm [i+1].y_offset;
  comm [i].lines = comm [i+1].lines;
  comm [i].from_type = comm [i+1].from_type;
  comm [i].from_letter = comm [i+1].from_letter;
  comm [i].from_rank = comm [i+1].from_rank;
  comm [i].to = comm [i+1].to;
  comm [i].col_min = comm [i+1].col_min;
  comm [i].col_max = comm [i+1].col_max;
  i ++;
 };


}


char nearby(int x, int y)
{

 if (arena.only_player == -1)
 {
  if ((x > player[0].x - 900000
    && x < player[0].x + 900000
    && y > player[0].y - 900000
    && y < player[0].y + 900000)
    ||
     (x > player[1].x - 900000
   && x < player[1].x + 900000
   && y > player[1].y - 900000
   && y < player[1].y + 900000))
    return 1;

  return 0;
 }

  if (x > player[arena.only_player].x - 900000
    && x < player[arena.only_player].x + 900000
    && y > player[arena.only_player].y - 900000
    && y < player[arena.only_player].y + 900000)
     return 1;

  return 0;

}


#ifdef TESTING_LEVEL
#define TEST_SCALE

#include <string.h>

extern BITMAP *display [3];

void run_test_wship(int a, int e);
volatile unsigned char ticked;
void wship_list(void);

void draw_test_wships(int col0, int col1, int trace);
void test_wship_collision_check(int a, int e);
int angle_to_convoy_position(int a, int e);

int wselect_x;
int wselect_y;
int select_a;
int select_e;

BITMAP *ltbmp;

int no_ships [2];


int coll_a [2];
int coll_e [2];
int coll_x;
int coll_y;

void level_test(void)
{

 arena.stage = 1;

 init_level();

 int speed = 14;
 int a, e, i, thing;
 wselect_x = 0;
 wselect_y = 0;
 select_a = 0;
 select_e = 0;
 no_ships [0] = 0;
 no_ships [1] = 0;
 coll_a [0] = -1;

 ltbmp = create_bitmap(600, 600);
 clear_bitmap(display[0]);
 clear_bitmap(ltbmp);
 clear_bitmap(screen);

 int time_elapsed = 0;
 char paused = 0;
 int anykey = 0;
 int key_count = 0;


 do
 {

   clear_bitmap(display[0]);

   if (key [KEY_EQUALS])
    speed = 50; // FF
     else
      speed = 10;

  for (i = KEY_A; i < KEY_CAPSLOCK + 1; i ++)
  {
   if (key [i])
   {
    if (anykey == 1)
    {
     anykey = 2;
     break;
    }
      else
      {
       if (anykey == 0)
        anykey = 1;
         else
          key_count ++;
       break;
      }
   }
   if (i == KEY_CAPSLOCK)
    anykey = 0;
  }

  if (key_count >= 10 || anykey == 0)
  {
   key_count = 0;
   anykey = 0;
  }

  if (anykey == 1)
  {
   if (key [KEY_P])
    paused ^= 1;
   if (key [KEY_UP])
   {
    wselect_y--;
    if (wselect_y < 0)
     wselect_y = no_ships [wselect_x] - 1;
   }
   if (key [KEY_DOWN])
   {
    wselect_y++;
    if (wselect_y > no_ships [wselect_x] - 1)
     wselect_y = 0;
   }
   if (key [KEY_LEFT] || key [KEY_RIGHT])
   {
    wselect_x ^= 1;
    if (wselect_y > no_ships [wselect_x] - 1)
     wselect_y = no_ships [wselect_x] - 1;
   }
   if (key [KEY_K])
   {
    ship[select_a][select_e].type = SHIP_NONE;
    ship[select_a][select_e].test_destroyed = 1;
   }

  }


 if (paused == 0)
 {
   draw_test_wships(COL_F2, COL_E2, 1);


   for (i = 0; i < speed; i ++)
   {

    arena.counter ++;
    arena.counter &= 255;
    arena.subtime++;
    if (arena.subtime == 50)
    {
     arena.time ++;
     arena.subtime = 0;
    }


    for (a = 0; a < 2; a ++)
    {
     for (e = 0; e < NO_SHIPS; e++)
     {
      if (EE.type == SHIP_NONE)
       continue;
      if (eclass[EE.type].ship_class != ECLASS_WSHIP)
       continue;
      run_test_wship(a, e);
      test_wship_collision_check(a, e);
     }
    }
    run_level();
   }
   draw_test_wships(COL_F4, COL_E4, 0);
  }
  else
  {
   textprintf_ex(display[0], font, 600, 15, -1, -1, "paused");
  }

  wship_list();

   blit(ltbmp, display[0], 0, 0, 200, 0, 600, 600);

    do
    {
        thing ++;
    } while (ticked == 0);
    ticked = 0;

 vline(display[0], 199, 0, 600, COL_F3);

 vsync();
 blit(display[0], screen, 0, 0, 0, 0, 800, 600);


 if (key [KEY_ESC])
  exit(0);

 } while(TRUE);



}

void draw_test_wships(int col0, int col1, int trace)
{
 int a, e;

#define BSHIFT 15

    for (a = 0; a < 2; a ++)
    {
     for (e = 0; e < NO_SHIPS; e++)
     {
      if (EE.test_destroyed == 0)
      {
       if (EE.type == SHIP_NONE)
        continue;
       if (eclass[EE.type].ship_class != ECLASS_WSHIP)
        continue;
      }
// note: type may be SHIP_NONE from this point
      if (a == 0)
       putpixel(ltbmp, 300 + (EE.x >> BSHIFT), 300 + (EE.y >> BSHIFT), col0);
        else
         putpixel(ltbmp, 300 + (EE.x >> BSHIFT), 300 + (EE.y >> BSHIFT), col1);
      if (trace == 0 && select_a == a && select_e == e)
         putpixel(ltbmp, 300 + (EE.x >> BSHIFT), 300 + (EE.y >> BSHIFT), grand(256));

     }
    }

 int cv;

 for (cv = 0; cv < NO_CONVOYS; cv ++)
 {
  if (CV.active == 0)//-1)
   continue;
  putpixel(ltbmp, 300 + (CV.x >> BSHIFT), 300 + (CV.y >> BSHIFT), COL_F1);//grand(256));
//  textprintf_ex(ltbmp, font, 300 + (CV.x >> BSHIFT), 315 + (CV.y >> BSHIFT), -1, -1, "%i %i", cv, CV.angle);

 }

}

void wship_list(void)
{

    int a, e, s;
    char str [40];
    int back = -1;


    for (a = 0; a < 2; a ++)
    {
     s = 0;
     for (e = 0; e < NO_SHIPS; e++)
     {
      if (EE.test_destroyed == 0)
      {
       if (EE.type == SHIP_NONE)
        continue;
       if (eclass[EE.type].ship_class != ECLASS_WSHIP)
        continue;
      }
      if (wselect_x == a && wselect_y == s)
      {
       back = COL_F1 + TRANS_BLUE2;
       select_a = a;
       select_e = e;
      }
      textprintf_right_ex(display[0], font, 15 + a*100, 15 + s*15, -1, back, "%i ", e);
      switch(EE.type)
      {
       case SHIP_NONE: strcpy(str, "<x>"); break;
       case SHIP_OLD2: strcpy(str, "old2"); break;
       case SHIP_OLD3: strcpy(str, "old3"); break;
       case SHIP_FRIEND3: strcpy(str, "friend3"); break;
       case SHIP_SCOUT2: strcpy(str, "scout2"); break;
       case SHIP_SCOUT3: strcpy(str, "scout3"); break;
       case SHIP_SCOUTCAR: strcpy(str, "scoutcar"); break;
       case SHIP_SCOUT3: strcpy(str, "scoutbase"); break;
      }
      textprintf_ex(display[0], font, 15 + a*100, 15 + s*15, -1, back, str);
      textprintf_ex(display[0], font, 15 + a*100, 215 + s*15, -1, back, "%i, %i", EE.x_speed, EE.y_speed);
      s++;
      back = -1;

     }
     no_ships [a] = s;
    }

    if (coll_a [0] == -1)
     textprintf_ex(display[0], font, 5, 580, -1, -1, "no collisions");
      else
       textprintf_ex(display[0], font, 5, 580, -1, -1, "%i,%i hit %i,%i (%i,%i)",
        coll_a [0], coll_e [0], coll_a [1], coll_e [1], coll_x>>10, coll_y>>10);


     textprintf_ex(display[0], font, 5, 560, -1, -1, "%i %i %i %i %i", running_script [0], running_script [1], running_script [2], running_script [3], running_script [4]);


}

void level_test_display(void)
{



}


void run_test_wship(int a, int e)
{
/*

      convoy_position(a, e);
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
*/
/*
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

        if (abs(EE.x - EE.convoy_x) < 100000 && abs(EE.y - EE.convoy_y) < 100000
         && (dist = xyedist(EE.convoy_x, EE.convoy_y, a, e)) < 100000)
         {
          if (abs(EE.x - EE.convoy_x) < 60000 && abs(EE.y - EE.convoy_y) < 60000
           && (dist = xyedist(EE.convoy_x, EE.convoy_y, a, e)) < 60000)
           {
            EE.wship_throttle = convoy[EE.convoy].throttle - 1;
            EE.target_angle = convoy[EE.convoy].angle;
            angle_diff = angle_difference(EE.angle, EE.target_angle);

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

            if (angle_diff > ANGLE_2 - ANGLE_4)
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

             if (angle_diff > ANGLE_2 - ANGLE_4)
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
            EE.turning_time = angle_diff / EE.turn_speed;
            if (EE.turning_time > 20)
             EE.turning_time = 20;
            EE.think_count = EE.turning_time;// * 2;
           }
          }
     }


 int cangle = angle_to_convoy_position(a, e);

 EE.x_speed += xpart(cangle, EE.wship_throttle);
 EE.y_speed += ypart(cangle, EE.wship_throttle);



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

*/


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

        if (abs(EE.x - EE.convoy_x) < 100000 && abs(EE.y - EE.convoy_y) < 100000
         && (dist = xyedist(EE.convoy_x, EE.convoy_y, a, e)) < 100000)
         {
          if (abs(EE.x - EE.convoy_x) < 60000 && abs(EE.y - EE.convoy_y) < 60000
           && (dist = xyedist(EE.convoy_x, EE.convoy_y, a, e)) < 60000)
           {
            EE.wship_throttle = convoy[EE.convoy].throttle - 1;
            EE.target_angle = convoy[EE.convoy].angle;
            angle_diff = angle_difference(EE.angle, EE.target_angle);

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

            if (angle_diff > ANGLE_2 - ANGLE_4)
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

             if (angle_diff > ANGLE_2 - ANGLE_4)
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
            EE.turning_time = angle_diff / EE.turn_speed;
            if (EE.turning_time > 20)
             EE.turning_time = 20;
            EE.think_count = EE.turning_time;// * 2;
           }
          }
     }


 int cangle = angle_to_convoy_position(a, e);

 EE.x_speed += xpart(cangle, EE.wship_throttle);
 EE.y_speed += ypart(cangle, EE.wship_throttle);



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



}

void test_wship_collision_check(int a, int e)
{

// collision check:

 int a2, e2, size;

    for (a2 = 0; a2 < 2; a2 ++)
    {
     for (e2 = 0; e2 < NO_SHIPS; e2++)
     {
       if (ship[a2][e2].type == SHIP_NONE)
        continue;
       if (eclass[ship[a2][e2].type].ship_class != ECLASS_WSHIP)
        continue;
       if (a == a2 && e == e2)
        continue;
       size = eclass[ship[a2][e2].type].size + eclass[EE.type].size;
       if (abs(EE.x - ship[a2][e2].x) < size && abs(EE.y - ship[a2][e2].y) < size)
       {
        if (hypot(EE.y - ship[a2][e2].y, EE.x - ship[a2][e2].x) < size)
        {
         circlefill(ltbmp, 300 + (EE.x >> BSHIFT), 300 + (EE.y >> BSHIFT), 5, COL_E2 + TRANS_RED2);
         coll_a [0] = a;
         coll_e [0] = e;
         coll_a [1] = a2;
         coll_e [1] = e2;
         coll_x = ship[a][e].x; coll_y = ship[a][e].y;
        }
       }

     }
    }

}



#endif
// end of ifdef TESTING_LEVEL


void display_team(void)
{

 rectfill(screen, 100, 100, 300, 300, COL_F1);

 int e;
 int a = TEAM_FRIEND;

extern FONT* small_font;

 for (e = 0; e < NO_SHIPS; e ++)
 {
  textprintf_ex(screen, small_font, 120, 120 + e * 10, -1, -1, "%i, %i, x %i, y %i", e, EE.type, EE.x, EE.y);

 }


 do
 {
     rest(5);
 } while (key [KEY_SPACE]);

 do
 {
     rest(5);
 } while (!key [KEY_SPACE]);

}
