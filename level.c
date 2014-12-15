
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

#include "display_init.h"

extern int comm_col [COMM_COLS] [COMM_COL_MAX+1];


#define CV convoy[cv]


void set_turret(int a, int e, int t, int type, int rest_angle);
void modify_ship(int a, int e);
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
int new_message_text(int m, int comm, int col);
void pack_messages(int deleted);
void setup_new_ship(int a, int e, int subtype);
int activate_carrier_launch(int a, int cv, int type);
int send_fighters(int group, int cv, int a);
void set_approach_target(int cv1, int cv2, int x_offset, int y_offset, char lead);
void about_to_end_script(int scr);
int get_convoy_speed(int cv);
int convoy_jump_out(int a, int cv);
void setup_player_wing(int p, int w, int type, int number);
void calculate_threat(void);
int check_for_enemies(void);
int get_special_stype(int stype);

void add_ship_to_command(int a, int e, int command);
void add_ship_to_letter(int a, int e, int letter);
void display_team(void);
void ship_from_level_data(int s, int a, int e);
void convoy_jumps_in(int cv, int flag);
void setup_special_stage_conditions(void);
void special_convoy(int cv);
void special_convoy_move(int cv);


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
 int flags2; // various settings e.g. command. Data type needs to contain enough bits to store all flags
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
#define RESULT_VALUES 3

enum
{
PCON_NONE,
PCON_SETUP_CONVOY,
PCON_XY, // vals: x, y
PCON_TIME, // vals: the time that running_script_time must equal to trigger PCON (in seconds)
PCON_LEVEL_TIME, // vals: the time that arena.time must equal to trigger (in seconds)
PCON_CONVOY_DIST, // vals: the max distance that the convoy must be from the other convoy to trigger
PCON_CONVOY_DIST_APART, // vals: like convoy_dist but min dist for moving away
PCON_LEVEL_TIME_SPECIAL, // uses an scon value for the time waited. vals: index of the scon.
PCON_ALWAYS_PASS, // easy
PCON_EVENT,
PCON_NO_ENEMIES_LEFT
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
CCON_CONVOY_DESTROYED, // can be friendly or enemy
CCON_CONVOY_REDUCED, // called if the convoy is reduced to a certain number. Can be friendly or enemy
CCON_SHIPTYPE_DESTROYED // called if a certain number of ships of a particular type have been destroyed (uses srecord data) val 0 = type, val 1 = number
};

struct conditionstruct
{
 int type;
 int val [CON_VALUES];
 int result_type;
 int result [RESULT_VALUES];
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
S4SCON_X,
S4SCON_Y
};


enum
{
RESULT_NONE,
RESULT_NEXT_SCRIPT,
RESULT_START_SCRIPT, // starts new script without killing current one
RESULT_END_SCRIPT,
RESULT_LAUNCH,
RESULT_JOIN_CONVOY, // also kills script even if joined convoy no longer exists
RESULT_START_JUMP_COUNTDOWN, // sets the countdown for player to get back to a wship. Doesn't actually cause jump
RESULT_CONVOY_JUMP_OUT, // does not kill script.
RESULT_MISSION_OVER, // mission finished (success, or at least survival) - can proceed to next mission
RESULT_MISSION_FAIL, // only happens if all friendly wships destroyed - this needs to be explicitly set as sometimes it is unwanted
RESULT_CHANGE_ARRANGE, // change arrangement without changing anything else
RESULT_SEND_FIGHTERS, // sends fighters in a particular group to attack a particular convoy
RESULT_EVENT, // sets event[result[0]] to result[1]. Other scripts can check this value and use it with PCON_EVENT
RESULT_NO_MORE_MESSAGES
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
#define FLAG_E_COMMAND_1 (1<<13)
#define FLAG_E_COMMAND_2 (1<<14)
#define FLAG_E_COMMAND_3 (1<<15)
// if adding jumps, add to init_level as well or they won't be skipped at start.

#define FLAGS2 0
#define FLAG2_FIGHTER_GROUP_1 1
#define FLAG2_MODIFIED (1<<1)
// modified: see modify_ship; is dealt with as special case for each level, e.g. a ship can be created with damage.
#define FLAG2_NO_SRECORD (1<<2)
// only affects SREC_CREATED. For now just use it for ships that jump out before anything happens

#define SCRIPT_VALS 5

struct scriptstruct
{
 int team; // may or may not be relevant
 int convoy;
 char name [40];
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
SCRIPT_L1C0_START,
SCRIPT_L1C0_MOVE1,
SCRIPT_L1C0_RETREAT,
SCRIPT_L1_CWLTH_WAIT,
SCRIPT_L1C1_JUMP,
SCRIPT_L1C1_MOVE1,
SCRIPT_L1C2_JUMP,
SCRIPT_L1C2_MOVE1,
SCRIPT_L1_ARRIVE1,
SCRIPT_L1_ARRIVE2,
SCRIPT_L1_ARRIVE3,
SCRIPT_L1_ARRIVE4,
SCRIPT_L1_FED_JUMP,
SCRIPT_L1C3_JUMP,
SCRIPT_L1C3_MOVE1,
SCRIPT_L1C3_ATTACK,
SCRIPT_L1C3_ATTACK2,
SCRIPT_L1C3_JOB_DONE,
SCRIPT_L1C3_RETREAT,
SCRIPT_L1C4_JUMP,
SCRIPT_L1C4_MOVE1,
SCRIPT_L1C4_ATTACK,
SCRIPT_L1C4_ATTACK2,
SCRIPT_L1C4_JOB_DONE,
SCRIPT_L1C4_RETREAT,

SCRIPT_L1_DESTROYED,
SCRIPT_L1_TRIREME_DESTROYED,
SCRIPT_L1_TRIREME_DESTROYED2


};

enum
{
SCRIPT_L2_NONE,
SCRIPT_L2C0_START,
SCRIPT_L2C0_MOVE1,
SCRIPT_L2C0_MOVE2,
SCRIPT_L2C1_START,
SCRIPT_L2C1_MOVE1,
SCRIPT_L2C2_START,
SCRIPT_L2C2_MOVE1,
SCRIPT_L2C2_LAUNCH2,
SCRIPT_L2C2_LAUNCH3,
SCRIPT_L2C2_LEAVE,
SCRIPT_L2C2_LEFT,
SCRIPT_L2_CHECK_COMPLETION,
SCRIPT_L2_LOSS
/*
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
*/
};

enum
{
SCRIPT_L3_EMPTY,
SCRIPT_L3C0_START,
SCRIPT_L3C0_MOVE1,
SCRIPT_L3C0_MOVE2,
SCRIPT_L3C0_MOVE3,
SCRIPT_L3C0_RETREAT,
SCRIPT_L3_START,
SCRIPT_L3_START2,
SCRIPT_L3C1_START,
SCRIPT_L3C1_MOVE,
SCRIPT_L3C2_START,
SCRIPT_L3C2_MOVE,
SCRIPT_L3C3_START,
SCRIPT_L3C3_MOVE,
SCRIPT_L3C3_MOVE2,
SCRIPT_L3C3_MOVE3,
SCRIPT_L3_MESSAGES,
SCRIPT_L3_CLEANUP,
SCRIPT_L3_JUST_FIGHTERS,
SCRIPT_L3_FINISH,
SCRIPT_L3_LOSS,
SCRIPT_L3_DESTROY

    /*
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
SCRIPT_L3C8_FOLLOW*/
};

enum
{
SCRIPT_L4_EMPTY,
SCRIPT_L4C0_START,
SCRIPT_L4C0_MOVE1,
SCRIPT_L4C0_MOVE2,
SCRIPT_L4C1_START,
SCRIPT_L4C1_MOVE1,
SCRIPT_L4C1_MOVE2,
SCRIPT_L4_OCSF_JUMP_ETC,
SCRIPT_L4_START,
SCRIPT_L4_START2,
SCRIPT_L4_FSF_WAIT,
SCRIPT_L4C2_JUMP,
SCRIPT_L4C2_MOVE1,
SCRIPT_L4C2_SLOW,
SCRIPT_L4C2_FAST,
SCRIPT_L4C3_JUMP,
SCRIPT_L4C3_MOVE1,
SCRIPT_L4C3_SLOW,
SCRIPT_L4C3_FAST,
SCRIPT_L4C4_JUMP,
SCRIPT_L4C4_MOVE1,
SCRIPT_L4C4_LAUNCH2,
SCRIPT_L4C4_LAUNCH3,
SCRIPT_L4C4_DISABLED,

SCRIPT_L4_LOSS,
SCRIPT_L4_LOSS2
};


enum
{
SCRIPT_L5_NONE,
SCRIPT_L5_START,
SCRIPT_L5_START2,
SCRIPT_L5_START3,
SCRIPT_L5_MISSION,
SCRIPT_L5_OCSF_JUMP,
SCRIPT_L5_SAFE,
SCRIPT_L5C0_START,
SCRIPT_L5C0_MOVE1,
SCRIPT_L5C0_MOVE2,
SCRIPT_L5_JUMPS,
SCRIPT_L5C1_JUMP,
SCRIPT_L5C1_MOVE1,
SCRIPT_L5C2_JUMP,
SCRIPT_L5C2_MOVE1
};

enum
{
SCRIPT_L6_NONE,
SCRIPT_L6C0_START,
SCRIPT_L6C0_MOVE1,
SCRIPT_L6C1_START,
SCRIPT_L6C1_MOVE1,
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
STYPE_MOVE_SPECIAL, // calls special_convoy_move when script starts; that function sets target x and y

// remember - when adding movement scripts, can_turn must be 1
};

enum
{
EVENT_L1C0_RETREAT,
NO_EVENTS
};

enum
{
APPROACH_LEAD,
APPROACH_NO_LEAD
};

int event [NO_EVENTS];
// there could've been separate event indices for each mission, but I couldn't be bothered doing this for such a small gain.

#define MSG_NONE 0
#define LEVEL_MSGS 30

enum
{
MSG_L1_NONE,
MSG_L1_START1,
MSG_L1_ARRIVE1,
MSG_L1_ARRIVE2,
MSG_L1_ARRIVE3,
MSG_L1_ARRIVE4,
MSG_L1_ARRIVE5,
MSG_L1_ARRIVE6,
MSG_L1_ARRIVE7,
MSG_L1_ARRIVE8,
MSG_L1_ARRIVE9,
MSG_L1_ARRIVE10,
MSG_L1_ARRIVE11,
MSG_L1_ARRIVE12,
MSG_L1_ARRIVE13,
MSG_L1_ARRIVE14,
MSG_L1_ARRIVE15,
MSG_L1_ARRIVE16,
MSG_L1_SPLIT,
MSG_L1_FIGHTERS,
MSG_L1_TRIREME_DESTROYED,
MSG_L1_BOTH_TRIREMES_DESTROYED,
MSG_L1_LEAVE,
MSG_L1_ENEMY_RETREAT,
MSG_L1_OVER,
MSG_L1_OVER2,
MSG_L1_FSF_DESTROYED,

// not used yet:
MSG_L1_ENGAGE,
MSG_L1_EWSHIPS_DESTROYED,
MSG_L1_EFIGHTERS_DESTROYED,
MSG_L1_MISSION_COMPLETE

};

enum
{
MSG_L2_NONE,
MSG_L2_START1,
MSG_L2_START2,
MSG_L2_START3,
MSG_L2_START4,
MSG_L2_START5,
MSG_L2_START6,
MSG_L2_START7,
MSG_L2_DAMAGE,
MSG_L2_WAVE2,
MSG_L2_WAVE3,
MSG_L2_EJUMP,
MSG_L2_FINISH,
MSG_L2_FSF_DESTROYED,
MSG_L2_EF_DESTROYED,
MSG_L2_EWS_DESTROYED,
MSG_L2_LOST1,
MSG_L2_LOST2,
MSG_L2_LOST3

};

enum
{
MSG_L3_NONE,
MSG_L3_START1,
MSG_L3_START2,
MSG_L3_START3,
MSG_L3_START4_A2,
MSG_L3_START5,
MSG_L3_START6_A2,
MSG_L3_TURN1_A2,
MSG_L3_TURN2,
MSG_L3_TURN3,
MSG_L3_ATTACK,
MSG_L3_CHANGE_FORMATION,
MSG_L3_CHANGE_FORMATION2,
MSG_L3_LOSS1,
MSG_L3_LOSS2,
MSG_L3_LOSS3,
MSG_L3_TARGET1,
MSG_L3_TARGET2,
MSG_L3_TARGET3,
MSG_L3_ETOJUMP,
MSG_L3_EJUMP,
MSG_L3_CLEANUP,
MSG_L3_OVER



    /*
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
MSG_L3M17,*/
};

enum
{
MSG_L4_NONE,
MSG_L4_START1_FR,
MSG_L4_START2,
MSG_L4_START3_FR,
MSG_L4_START4,
MSG_L4_START5_FR,

MSG_L4_START6_A2,
MSG_L4_START7,
MSG_L4_START8_FR,
MSG_L4_START9,

MSG_L4_LAUNCH1,
MSG_L4_LAUNCH2,
MSG_L4_LAUNCH3,

MSG_L4_C2_ATTACKS,
MSG_L4_C3_ATTACKS,

MSG_L4_JUMP,
MSG_L4_C2_DESTROYED,
MSG_L4_C3_DESTROYED,
MSG_L4_CARRIERS_DISABLED,

MSG_L4_C0_LOSS1,
MSG_L4_C0_LOSS2,
MSG_L4_C0_LOSS3_FR,
MSG_L4_C1_LOSS1,
MSG_L4_C1_LOSS2,
MSG_L4_C1_LOSS3
};


enum
{
MSG_L5_NONE,
MSG_L5_START1,
MSG_L5_START2,
MSG_L5_START3,
MSG_L5_START4,
MSG_L5_START5,
MSG_L5_START6,
MSG_L5_START7,
MSG_L5_START8,
MSG_L5_START9,
MSG_L5_START10,

MSG_L5_ARRIVE1,
MSG_L5_ARRIVE2,
MSG_L5_ARRIVE3,

MSG_L5_ARRIVE4,
MSG_L5_ARRIVE5,

MSG_L5_AGAIN1,

MSG_L5_LOSS1,
MSG_L5_LOSS2,
MSG_L5_LOSS3,

MSG_L5_SAFE1,
MSG_L5_SAFE2


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

#define SPEED_F_LIGHT 15
#define SPEED_F_HEAVY 8
#define SPEED_E_LIGHT 13
#define SPEED_E_HEAVY 9
#define SPEED_SLOW 3

/*
#define COL_OCSF COL_BOX4
#define COL_FED COL_EBOX4
#define COL_CWLTH COL_BOX4
#define COL_E_CWLTH COL_EBOX4*/

struct overlevelstruct ol [NO_LEVELS] =
{
 {
 },
 {
// Stage 1 - recon unidentified fleet
  {
   {""},
   {" This is $BGazer 1$C  to $BGazer group$C. Those ships should get here any second now. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L1_START1
   {" Two ships just jumped in. We'll get a scan as soon as possible. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L1_ARRIVE1
// let's skip this one:
   {" $BGazer 1$C, do you have anything on that scan yet? ", {WAV_SELECT0, NOTE_2A, COMM_COL_CWLTH}}, // MSG_L1_ARRIVE2
   {" Scan completed: the ships are Imperial heavy cruisers. One with signs of severe damage. No fighters or other craft. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L1_ARRIVE3
   {" $BAngry Moth$C, you should be picking the ships up on your scanner. Head over there while we establish a comms channel, but DO NOT fire on them. ", {WAV_SELECT1, NOTE_2G, COMM_COL_TO_AM}}, // MSG_L1_ARRIVE4
   {" Is that the CTBR? Thank the Emperor it's you! ", {WAV_SELECT0, NOTE_1G, COMM_COL_IMP}}, // MSG_L1_ARRIVE5
   {" Imperial warships, we have no records of an official request for passage. Please drop shields and power down all weapons. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L1_ARRIVE6
   {" Negative - they're right behind us! We surrender and request your assistance! ", {WAV_SELECT0, NOTE_1G, COMM_COL_IMP}}, // MSG_L1_ARRIVE7
   {" What? Imperial ships don't - ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L1_ARRIVE8
   {" $BGazer 1$C, several unidentified craft just jumped in. ", {WAV_SELECT0, NOTE_2A, COMM_COL_CWLTH}}, // MSG_L1_ARRIVE9
   {" Are those Federation ships? ", {WAV_SELECT0, NOTE_2B, COMM_COL_CWLTH}}, // MSG_L1_ARRIVE10
   {" Commonwealth captain, I greet you on behalf of the Federation Starfleet. We come in peace and friendship. ", {WAV_SELECT0, NOTE_3C, COMM_COL_FED}}, // MSG_L1_ARRIVE11
   {" Thank you, and welcome to Commonwealth space. Please drop shields and power down all weapons. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L1_ARRIVE12
   {" We are in pursuit of war criminals escaping from a brutal attack on our capital. Do not try to protect them, or you will be destroyed as well. ", {WAV_SELECT0, NOTE_1G, COMM_COL_FED}}, // MSG_L1_ARRIVE13
   {" What the hell? Do they want to start a war? Imperial ships, we accept your surrender. Hold tight, we've sent some fighters your way. ", {WAV_SELECT0, NOTE_1G, COMM_COL_CWLTH}}, // MSG_L1_ARRIVE14
   {" Thank you, CTBR captain. ", {WAV_SELECT0, NOTE_1G, COMM_COL_IMP}}, // MSG_L1_ARRIVE15
   {" $BAngry Moth$C: see if you can protect those cruisers, but pull out if it gets too hot - our first priority is to report back! Permission given to return fire. ", {WAV_SELECT1, NOTE_1G, COMM_COL_TO_AM}}, // MSG_L1_ARRIVE16

   {" The Federation group is splitting up; looks like they're going to try to engage the Imperial cruisers separately. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L1_SPLIT
   {" Squadron of hostile fighters incoming - our shields are out, and without our escort vessels they'll tear us apart! ", {WAV_SELECT0, NOTE_1G, COMM_COL_IMP}}, // MSG_L1_FIGHTERS

   {" One of the Imperial cruisers is down! Concentrate defences on the surviving ship. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L1_TRIREME_DESTROYED
   {" Both of the Imperial cruisers have been destroyed. There wasn't much we could have done against those FSF ships. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L1_BOTH_TRIREMES_DESTROYED
   {" We've done what we came here to do. All fighters, return to the main battle group for jump pickup. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L1_LEAVE
   {" The Federation ships are turning around! ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L1_ENEMY_RETREAT
// OVER 1 and 2 not used
   {" Good work, everyone. We'll provide cover to the Imperial ships while they recharge for the jump out. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L1_OVER
   {" Thank you, CTBR captain. Send us destination coordinates and we'll follow you home. ", {WAV_SELECT0, NOTE_1G, COMM_COL_IMP}}, // MSG_L1_OVER2
   {" All Federation ships destroyed! Well, they started it. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L1_FSF_DESTROYED


/*
   {" This is $BAlpha 1$C  to $BAlpha group$C. Those ships should get here any second now. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L1_START1
   {" Two ships just jumped in. We'll get a scan as soon as possible. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L1_ARRIVE1
   {" $BAlpha 1$C, do you have anything on that scan yet? ", {WAV_SELECT0, NOTE_2A, COMM_COL_CWLTH}}, // MSG_L1_ARRIVE2
   {" Confirmed, the ships are Commonwealth heavy cruisers. One of them shows signs of heavy damage. No fighters or other craft. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L1_ARRIVE3
   {" $BAngry Moth$C, you should be picking the ships up on your scanner. Head over there while we establish a comms channel, but DO NOT open fire. ", {WAV_SELECT1, NOTE_2G, COMM_COL_TO_AM}}, // MSG_L1_ARRIVE4
   {" Is that the OCSF? Thank god it's you! ", {WAV_SELECT0, NOTE_1G, COMM_COL_CWLTH}}, // MSG_L1_ARRIVE5
   {" Commonwealth warships, your presence is a violation of OC territorial sovereignty. Drop your shields, power down all weapons and prepare to be taken into custody. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L1_ARRIVE6
   {" Negative - they're right behind us! We surrender and request your assistance! ", {WAV_SELECT0, NOTE_1G, COMM_COL_CWLTH}}, // MSG_L1_ARRIVE7
   {" We will accept your surrender when you - ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L1_ARRIVE8
   {" $BAlpha 1$C, several unidentified craft just jumped in. ", {WAV_SELECT0, NOTE_2A, COMM_COL_CWLTH}}, // MSG_L1_ARRIVE9
   {" Are those Federation ships? I've never seen - ", {WAV_SELECT0, NOTE_2B, COMM_COL_CWLTH}}, // MSG_L1_ARRIVE10
   {" This is Federation Starfleet pursuit group 6. OCSF ships, we are tracking escaped war criminals. Please stand down and let us do our job. ", {WAV_SELECT0, NOTE_3C, COMM_COL_FED}}, // MSG_L1_ARRIVE11
   {" That's crazy talk. These ships' crew have requested our protection. Turn back now. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L1_ARRIVE12
   {" These crippled dogs are ours. Stand down or we will destroy you as well. No further communication. ", {WAV_SELECT0, NOTE_1G, COMM_COL_FED}}, // MSG_L1_ARRIVE13
   {" What the hell? Do they want to start a war? Commonwealth ships, we accept your surrender. Hold tight, we're sending some fighters your way. ", {WAV_SELECT0, NOTE_1G, COMM_COL_CWLTH}}, // MSG_L1_ARRIVE14
   {" Thank you, OCSF captain. ", {WAV_SELECT0, NOTE_1G, COMM_COL_CWLTH}}, // MSG_L1_ARRIVE15
   {" $BAngry Moth$C: see if you can protect those cruisers, but pull out if it gets too hot - our first priority is to report back! Permission given to fire on all Federation craft. ", {WAV_SELECT1, NOTE_1G, COMM_COL_TO_AM}}, // MSG_L1_ARRIVE16

   {" The Federation group is splitting up; looks like they're going to try to engage the Commonwealth cruisers separately. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L1_SPLIT
   {" Squadron of FSF fighters incoming - without our shields or escort vessels they'll tear us apart! ", {WAV_SELECT0, NOTE_1G, COMM_COL_CWLTH}}, // MSG_L1_FIGHTERS

   {" One of the CTBF cruisers is down! Concentrate defences on the surviving ship. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L1_TRIREME_DESTROYED
   {" Both of the CTBF cruisers have been destroyed. There wasn't much we could have done against those FSF ships. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L1_BOTH_TRIREMES_DESTROYED
   {" We've done what we came here to do. All fighters, return to the main battle group for jump pickup. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L1_LEAVE
   {" The Federation ships are turning around. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L1_ENEMY_RETREAT
   {" Good work, everyone. We'll provide cover to the CTBF ships while they recharge for the jump out. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L1_OVER
   {" Thank you, OCSF captain. Send us destination coordinates and we'll follow you home. ", {WAV_SELECT0, NOTE_1G, COMM_COL_CWLTH}}, // MSG_L1_OVER2
   {" All Federation ships destroyed! Well, they started it. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L1_ENEMY_RETREAT


*/
// MSG_L1_APPROACH:
//   {"All right, Alpha group is approaching . ", {WAV_SELECT0, NOTE_2G}},

  },

  {

// this is the team of LCs you jump in with:

   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 1, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{120, 0}, {-ANGLE_8, 150}}},
   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{0, 120}, {-ANGLE_4-ANGLE_8, 150}}},
   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{-120,0}, {ANGLE_8, 150}}},
   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{0, -120}, {ANGLE_4 + ANGLE_8, 150}}},

/*
   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 1, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{0, 0}, {-ANGLE_8, 120}}},
   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{-ANGLE_4-ANGLE_8, 150}, {-ANGLE_4-ANGLE_8, 120}}},
   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{ANGLE_4+ANGLE_8, 150}, {ANGLE_8, 120}}},
   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{ANGLE_2, 180}, {ANGLE_4 + ANGLE_8, 120}}},
*/
   {TEAM_FRIEND, SHIP_ESCORT1, VAR_NONE, MISSION_GUARD, ACT_AWAY, 1, 1, 0,0,
    CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
   {TEAM_FRIEND, SHIP_ESCORT1, VAR_NONE, MISSION_GUARD, ACT_AWAY, 1, 2, 0,0,
    CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},

//
   {TEAM_FRIEND, SHIP_FRIEND3, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
    CONVOY_1, FLAG_COMMAND_2|FLAG_JUMP_1, FLAGS2,
    {{0,0}}},
   {TEAM_FRIEND, SHIP_FRIEND3, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
    CONVOY_2, FLAG_COMMAND_2|FLAG_JUMP_2, FLAG2_MODIFIED, // modified flag: calls a special function. This ship is damaged.
    {{0,0}}},
/*
 3  2  1
  5   4
    6

     3
    2
  1  x
    4
     6
*/

// first enemy convoy:
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 3, 0, 0,0,
   CONVOY_3, FLAG_JUMP_3|FLAG_E_COMMAND_1, FLAGS2,
    {{-10,0}, {-10,0}, {-10, 0}}}, // 1
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_3, FLAG_JUMP_3|FLAG_E_COMMAND_1, FLAGS2,
    {{-180,0}, {-80,-180}, {-20,-360}}}, // 2
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 4, 0, 0,0,
   CONVOY_3, FLAG_JUMP_3|FLAG_E_COMMAND_1, FLAGS2,
    {{-400,0}, {-180,-120}, {-150, -300}}}, // 3
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_3, FLAG_JUMP_3|FLAG_E_COMMAND_1, FLAGS2, // 4
    {{-60,100}, {-180,120}, {-150, 300}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_3, FLAG_JUMP_3|FLAG_E_COMMAND_1, FLAGS2, // 5
    {{-300,100}, {-80,180}, {-20, 360}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_3, FLAG_JUMP_3|FLAG_E_COMMAND_1, FLAGS2, // 6
    {{-180,110}, {-300,0}, {-300, 0}}},

  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 3, 1, 0,0,
   CONVOY_NONE, FLAG_JUMP_3, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 3, 2, 0,0,
   CONVOY_NONE, FLAG_JUMP_3, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 4, 1, 0,0,
   CONVOY_NONE, FLAG_JUMP_3, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 4, 2, 0,0,
   CONVOY_NONE, FLAG_JUMP_3, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 5, 1, 0,0,
   CONVOY_NONE, FLAG_JUMP_3, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 5, 2, 0,0,
   CONVOY_NONE, FLAG_JUMP_3, FLAGS2, {{0,0}}},


// second enemy convoy. Sometimes this convoy has the same position as CV 3, so these all need to be offset a bit
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 2, 0, 0,0,
   CONVOY_4, FLAG_JUMP_4|FLAG_E_COMMAND_1, FLAGS2,
    {{-60,-100}, {-180, 0}, {-180,0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_4, FLAG_JUMP_4|FLAG_E_COMMAND_1, FLAGS2,
    {{-300,-100}, {0, -180}, {0,-300}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_4, FLAG_JUMP_4|FLAG_E_COMMAND_1, FLAGS2,
    {{-180,-110}, {0, 180}, {0,300}}},


  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 2, 1, 0,0,
   CONVOY_NONE, FLAG_JUMP_4, FLAG2_FIGHTER_GROUP_1, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 2, 2, 0,0,
   CONVOY_NONE, FLAG_JUMP_4, FLAG2_FIGHTER_GROUP_1, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 2, 3, 0,0,
   CONVOY_NONE, FLAG_JUMP_4, FLAG2_FIGHTER_GROUP_1, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 2, 4, 0,0,
   CONVOY_NONE, FLAG_JUMP_4, FLAG2_FIGHTER_GROUP_1, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 2, 5, 0,0,
   CONVOY_NONE, FLAG_JUMP_4, FLAG2_FIGHTER_GROUP_1, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 2, 6, 0,0,
   CONVOY_NONE, FLAG_JUMP_4, FLAG2_FIGHTER_GROUP_1, {{0,0}}},

  {TEAM_NONE} // this is necessary or srecord and other things break
  },

  {
  {

  },

  {TEAM_FRIEND, CONVOY_0, "SCRIPT_L1C0_START",
   STYPE_SETUP_CONVOY, -11500, 0, SPEED_F_LIGHT, 0, {0},
    {{PCON_SETUP_CONVOY, {19500, 0, 500}, RESULT_NEXT_SCRIPT, {SCRIPT_L1C0_MOVE1}}, // setup scripts can't have messages
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_FRIEND, CONVOY_0, "SCRIPT_L1C0_MOVE1",
   STYPE_MOVE_XY, 19500, 0, SPEED_F_LIGHT, 0, {0},
   {{PCON_LEVEL_TIME, {3}, RESULT_NONE, {0}, MSG_L1_START1, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
//     {PCON_CONVOY_DIST, {CONVOY_1, 500}, RESULT_NEXT_SCRIPT, SCRIPT_L1C0_MOVE2, MSG_L1_APPROACH, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {PCON_EVENT, {EVENT_L1C0_RETREAT, 1}, RESULT_NEXT_SCRIPT, {SCRIPT_L1C0_RETREAT}},
//    {PCON_LEVEL_TIME, {10}, RESULT_MISSION_OVER},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_FRIEND, CONVOY_0, "SCRIPT_L1C0_RETREAT", // - another script later deals with jumping out
   STYPE_MOVE_XY, -10000, 10000, SPEED_F_LIGHT, 0, {0},
   {{PCON_NONE},
    },
    {{CCON_NONE},
    }
   },


  {TEAM_FRIEND, CONVOY_NONE, "SCRIPT_L1_CWLTH_WAIT",
   STYPE_START_WAIT, 0, 0, 0, 0, {0},
    {{PCON_LEVEL_TIME, {5}, RESULT_START_SCRIPT, {SCRIPT_L1C1_JUMP}},
     {PCON_LEVEL_TIME, {5}, RESULT_START_SCRIPT, {SCRIPT_L1_ARRIVE1}},
     {PCON_LEVEL_TIME, {5}, RESULT_NEXT_SCRIPT, {SCRIPT_L1C2_JUMP}},
//     {PCON_LEVEL_TIME, {10}, RESULT_START_JUMP_COUNTDOWN, {500}},
//     {PCON_LEVEL_TIME, {20}, RESULT_CONVOY_JUMP_OUT, {TEAM_FRIEND, CONVOY_0}},
     {PCON_NONE},
    },
   {{CCON_NONE},
   }
   },
  {FLAG_JUMP_1, CONVOY_1, "SCRIPT_L1C1_JUMP",
   STYPE_CONVOY_JUMP, 10000, -3400, SPEED_F_HEAVY, 0, {0},
    {{PCON_SETUP_CONVOY, {0, 0, 200}, RESULT_NONE},
     {PCON_ALWAYS_PASS, {0}, RESULT_NEXT_SCRIPT, {SCRIPT_L1C1_MOVE1}},
     {PCON_NONE},
    },
   {{CCON_NONE},
   }
   },
  {TEAM_FRIEND, CONVOY_1, "SCRIPT_L1C1_MOVE1",
   STYPE_MOVE_XY, 0, 0, SPEED_F_HEAVY, 0, {0},
   {{PCON_NONE},
    },
    {{CCON_NONE},
    }
   },

  {FLAG_JUMP_2, CONVOY_2, "SCRIPT_L1C2_JUMP",
   STYPE_CONVOY_JUMP, 11000, 5200, SPEED_F_HEAVY, 0, {0},
    {{PCON_SETUP_CONVOY, {0, 0, 200}, RESULT_NONE},
     {PCON_ALWAYS_PASS, {0}, RESULT_NEXT_SCRIPT, {SCRIPT_L1C2_MOVE1}},
     {PCON_NONE},
    },
   {{CCON_NONE},
   }
   },
  {TEAM_FRIEND, CONVOY_2, "SCRIPT_L1C2_MOVE1",
   STYPE_MOVE_XY, 0, 300, SPEED_F_HEAVY, 0, {0},
   {{PCON_NONE},
    },
    {{CCON_NONE},
    }
   },


  {TEAM_FRIEND, CONVOY_NONE, "SCRIPT_L1_ARRIVE1",
   STYPE_WAIT, 0, 0, 0, 0, {0},
    {{PCON_TIME, {3}, RESULT_NONE, {0}, MSG_L1_ARRIVE1, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
//     {PCON_TIME, {9}, RESULT_NONE, {0}, MSG_L1_ARRIVE2, MSG_FROM_COMMAND_1, 2, MSG_TO_ALL},
     {PCON_TIME, {6}, RESULT_NONE, {0}, MSG_L1_ARRIVE3, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {PCON_TIME, {9}, RESULT_NONE, {0}, MSG_L1_ARRIVE4, MSG_FROM_COMMAND_1, 1, MSG_TO_AM},
     {PCON_TIME, {14}, RESULT_NEXT_SCRIPT, {SCRIPT_L1_ARRIVE2}, MSG_L1_ARRIVE5, MSG_FROM_COMMAND_2, 1, MSG_TO_ALL}
    },
    {{CCON_NONE},
    }
   },
  {TEAM_FRIEND, CONVOY_NONE, "SCRIPT_L1_ARRIVE2",
   STYPE_WAIT, 0, 0, 0, 0, {0},
    {{PCON_TIME, {3}, RESULT_NONE, {0}, MSG_L1_ARRIVE6, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {PCON_TIME, {6}, RESULT_NONE, {0}, MSG_L1_ARRIVE7, MSG_FROM_COMMAND_2, 1, MSG_TO_ALL},
     {PCON_TIME, {12}, RESULT_START_SCRIPT, {SCRIPT_L1_FED_JUMP}, MSG_L1_ARRIVE8, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {PCON_TIME, {13}, RESULT_NONE, {0}, MSG_L1_ARRIVE9, MSG_FROM_COMMAND_1, 2, MSG_TO_ALL},
     {PCON_TIME, {16}, RESULT_NEXT_SCRIPT, {SCRIPT_L1_ARRIVE3}, MSG_L1_ARRIVE10, MSG_FROM_COMMAND_1, 3, MSG_TO_ALL}
    },
    {{CCON_NONE},
    }
   },
  {TEAM_FRIEND, CONVOY_NONE, "SCRIPT_L1_ARRIVE3",
   STYPE_WAIT, 0, 0, 0, 0, {0},
    {{PCON_TIME, {1}, RESULT_NONE, {0}, MSG_L1_ARRIVE11, MSG_FROM_E_COMMAND_1, 1, MSG_TO_ALL},
     {PCON_TIME, {6}, RESULT_NONE, {0}, MSG_L1_ARRIVE12, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {PCON_TIME, {10}, RESULT_NONE, {0}, MSG_L1_ARRIVE13, MSG_FROM_E_COMMAND_1, 1, MSG_TO_ALL},
     {PCON_TIME, {14}, RESULT_NONE, {0}, MSG_L1_ARRIVE14, MSG_FROM_COMMAND_1, 2, MSG_TO_ALL},
     {PCON_TIME, {18}, RESULT_NEXT_SCRIPT, {SCRIPT_L1_ARRIVE4}, MSG_L1_ARRIVE15, MSG_FROM_COMMAND_2, 1, MSG_TO_ALL}
    },
    {{CCON_NONE},
    }
   },
  {TEAM_FRIEND, CONVOY_NONE, "SCRIPT_L1_ARRIVE4",
   STYPE_WAIT, 0, 0, 0, 0, {0},
    {{PCON_TIME, {3}, RESULT_NONE, {0}, MSG_L1_ARRIVE16, MSG_FROM_COMMAND_1, 1, MSG_TO_AM},
     {PCON_TIME, {4}, RESULT_END_SCRIPT},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },


  {TEAM_ENEMY, CONVOY_NONE, "SCRIPT_L1_FED_JUMP",
   STYPE_WAIT, 0, 0, 0, 0, {0},
    {{PCON_TIME, {1}, RESULT_START_SCRIPT, {SCRIPT_L1C3_JUMP}},
     {PCON_TIME, {1}, RESULT_NEXT_SCRIPT, {SCRIPT_L1C4_JUMP}},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },


  {FLAG_JUMP_3, CONVOY_3, "SCRIPT_L1C3_JUMP",
   STYPE_CONVOY_JUMP, 12000, 800, SPEED_E_LIGHT, 0, {0},
    {{PCON_SETUP_CONVOY, {0, 0, 200}, RESULT_NONE},
     {PCON_ALWAYS_PASS, {0}, RESULT_NEXT_SCRIPT, {SCRIPT_L1C3_MOVE1}},
     {PCON_NONE},
    },
   {{CCON_NONE},
   }
   },
  {TEAM_ENEMY, CONVOY_3, "SCRIPT_L1C3_MOVE1",
   STYPE_APPROACH_CONVOY_OFFSET, CONVOY_1, APPROACH_LEAD, SPEED_E_LIGHT, 1, {0},
    {{PCON_CONVOY_DIST, {CONVOY_1, 1500}, RESULT_NEXT_SCRIPT, {SCRIPT_L1C3_ATTACK}, MSG_NONE},
     {PCON_NONE},
    },
    {{CCON_CONVOY_DESTROYED, {CONVOY_1}, RESULT_NEXT_SCRIPT, {SCRIPT_L1C3_JOB_DONE}, MSG_NONE},
     {CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_3, "SCRIPT_L1C3_ATTACK1",
   STYPE_APPROACH_CONVOY, CONVOY_1, APPROACH_LEAD, SPEED_E_LIGHT, 2, {0},
    {{PCON_CONVOY_DIST, {CONVOY_1, 300}, RESULT_NEXT_SCRIPT, {SCRIPT_L1C3_ATTACK2}, MSG_NONE},
     {PCON_NONE},
    },
    {{CCON_CONVOY_DESTROYED, {CONVOY_1}, RESULT_NEXT_SCRIPT, {SCRIPT_L1C3_JOB_DONE}, MSG_NONE},
     {CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_3, "SCRIPT_L1C3_ATTACK2",
   STYPE_APPROACH_CONVOY, CONVOY_1, APPROACH_LEAD, SPEED_F_HEAVY, 2, {0}, // match speed with target
    {
     {PCON_LEVEL_TIME, {290}, RESULT_NEXT_SCRIPT, {SCRIPT_L1C3_RETREAT}},
     {PCON_NONE},
    },
    {{CCON_CONVOY_DESTROYED, {CONVOY_1}, RESULT_NEXT_SCRIPT, {SCRIPT_L1C3_JOB_DONE}, MSG_NONE},
     {CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_3, "SCRIPT_L1C3_JOB_DONE",
   STYPE_APPROACH_CONVOY, CONVOY_2, APPROACH_LEAD, SPEED_E_LIGHT, 1, {0}, // match speed with target
    {
     {PCON_LEVEL_TIME, {290}, RESULT_NEXT_SCRIPT, {SCRIPT_L1C3_RETREAT}},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },


  {TEAM_ENEMY, CONVOY_3, "SCRIPT_L1C3_RETREAT",
   STYPE_MOVE_XY, 20000, 0, SPEED_E_LIGHT, 0, {0},
   {{PCON_TIME, {3}, RESULT_NONE, {0}, MSG_L1_ENEMY_RETREAT, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {PCON_TIME, {8}, RESULT_MISSION_OVER},
//    {PCON_LEVEL_TIME, {295}, RESULT_NONE, {0}, MSG_L1_OVER, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
//    {PCON_LEVEL_TIME, {299}, RESULT_NONE, {0}, MSG_L1_OVER2, MSG_FROM_COMMAND_2, 1, MSG_TO_ALL},
//    {PCON_LEVEL_TIME, {302}, RESULT_MISSION_OVER},
    {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },

  {FLAG_JUMP_4, CONVOY_4, "SCRIPT_L1C4_JUMP",
   STYPE_CONVOY_JUMP, 12000, 800, SPEED_E_LIGHT, 0, {0},
    {{PCON_SETUP_CONVOY, {0, 0, 200}, RESULT_NONE},
     {PCON_ALWAYS_PASS, {0}, RESULT_NEXT_SCRIPT, {SCRIPT_L1C4_MOVE1}},
     {PCON_NONE},
    },
   {{CCON_NONE},
   }
   },
  {TEAM_ENEMY, CONVOY_4, "SCRIPT_L1C4_MOVE1",
   STYPE_APPROACH_CONVOY, CONVOY_2, APPROACH_LEAD, SPEED_E_LIGHT, 1, {0},
   {{PCON_TIME, {40}, RESULT_SEND_FIGHTERS, {1, CONVOY_2, TEAM_ENEMY}, MSG_L1_FIGHTERS, MSG_FROM_COMMAND_2, 1, MSG_TO_ALL},
    {PCON_CONVOY_DIST, {CONVOY_2, 1500}, RESULT_NEXT_SCRIPT, {SCRIPT_L1C4_ATTACK}, MSG_NONE},
    {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_4, "SCRIPT_L1C4_ATTACK",
   STYPE_APPROACH_CONVOY, CONVOY_2, APPROACH_LEAD, SPEED_E_LIGHT, 2, {0},
    {{PCON_CONVOY_DIST, {CONVOY_2, 200}, RESULT_NEXT_SCRIPT, {SCRIPT_L1C4_ATTACK2}, MSG_NONE},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_4, "SCRIPT_L1C4_ATTACK2",
   STYPE_APPROACH_CONVOY, CONVOY_2, APPROACH_LEAD, SPEED_F_HEAVY, 2, {0}, // match speed with target
    {
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_4, "SCRIPT_L1C4_JOB_DONE",
   STYPE_APPROACH_CONVOY, CONVOY_0, APPROACH_LEAD, SPEED_E_LIGHT, 1, {0},
    {
     {PCON_LEVEL_TIME, {240}, RESULT_NEXT_SCRIPT, {SCRIPT_L1C4_RETREAT}},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_4, "SCRIPT_L1C4_RETREAT",
   STYPE_MOVE_XY, 20000, 2000, SPEED_E_LIGHT, 0, {0},
   {{PCON_NONE},
    },
    {{CCON_NONE},
    }
   },



  {TEAM_FRIEND, CONVOY_NONE, "SCRIPT_L1_DESTROYED",
   STYPE_START_WAIT, 0, 0, 0, 0, {0},
   {{PCON_NONE},
    },
   {{CCON_CONVOY_DESTROYED, {CONVOY_0}, RESULT_MISSION_FAIL, {0}},
     {CCON_CONVOY_DESTROYED, {CONVOY_1}, RESULT_NEXT_SCRIPT, {SCRIPT_L1_TRIREME_DESTROYED}, MSG_L1_TRIREME_DESTROYED, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {CCON_CONVOY_DESTROYED, {CONVOY_2}, RESULT_NEXT_SCRIPT, {SCRIPT_L1_TRIREME_DESTROYED}, MSG_L1_TRIREME_DESTROYED, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {CCON_NONE},
   }
   },
  {TEAM_FRIEND, CONVOY_NONE, "SCRIPT_L1_TRIREME_DESTROYED",
   STYPE_WAIT, 0, 0, 0, 0, {0},
   {{PCON_NONE},
    },
   {{CCON_CONVOY_DESTROYED, {CONVOY_0}, RESULT_MISSION_FAIL, {0}},
     {CCON_CONVOY_DESTROYED, {CONVOY_1}, RESULT_NEXT_SCRIPT, {SCRIPT_L1_TRIREME_DESTROYED2}, MSG_L1_BOTH_TRIREMES_DESTROYED, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {CCON_CONVOY_DESTROYED, {CONVOY_2}, RESULT_NEXT_SCRIPT, {SCRIPT_L1_TRIREME_DESTROYED2}, MSG_L1_BOTH_TRIREMES_DESTROYED, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {CCON_NONE},
   }
   },

  {TEAM_FRIEND, CONVOY_NONE, "SCRIPT_L1_TRIREME_DESTROYED2",
   STYPE_WAIT, 0, 0, 0, 0, {0},
   {{PCON_TIME, {2}, RESULT_EVENT, {EVENT_L1C0_RETREAT, 1}, MSG_L1_LEAVE, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {PCON_TIME, {4}, RESULT_START_JUMP_COUNTDOWN, {45}, MSG_NONE},
    {PCON_TIME, {49}, RESULT_CONVOY_JUMP_OUT, {TEAM_FRIEND, CONVOY_0}, MSG_NONE},
    },
   {{CCON_CONVOY_DESTROYED, {CONVOY_0}, RESULT_MISSION_FAIL, {0}},
     {CCON_NONE},
   }
   },



  {TEAM_FRIEND, CONVOY_NONE, "SCRIPT_L1_ENEMY_DESTROYED_CHECK",
   STYPE_START_WAIT, 0, 0, 0, 0, {0},
   {{PCON_NONE},
    },
   {{CCON_EALL_DESTROYED, {0}, RESULT_MISSION_OVER, {0}, MSG_L1_FSF_DESTROYED, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {CCON_EALL_DESTROYED, {0}, RESULT_NO_MORE_MESSAGES},
     {CCON_NONE},
   }
   },



  }

/*
Plan for mission 1:

- Start with AM alongside CV 0. After a while CV 1 and 2 jump in and a conversation ensues while AM approaches CF 1 and 2
- CV 1 is at full strength and ahead. CV 2 is badly damaged (no shield) and behind.
- Then Fed ships jump in as CV 3 and 4 which look like just one formation.
- CV 3 (large) and 4 (small) split up and seek the two triremes. Each also sends a fighter group.
 MSG announces splitup
- If CV 2 is destroyed, CV 4 rejoins CV 3 and fighters scramble
- If CV 1 is destroyed, CV 4 still rejoins it and takes over from it
- If CV 1 and 2 destroyed, CV 0 recalls fighters and jumps out
- If CV 1 and/or 2 survive they converge and are joined by CV 0. Then CV 3 catches up and a battle happenss.
- If all enemies destroyed, mission ends
- If all OCSF warships destroyed but Triremes survive - what then? tricky. I think it'll have to be game over. This is pretty unlikely.

Misc MSGs:
- CV 1 loses shield
- CV 1 is destroyed
- CV 2 is destroyed
- CV 3 is destroyed
- CV 4 is destroyed
- CV 0 about to engage CV 3

*/



// S1M
/*
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
  {TEAM_FRIEND, SHIP_FRIEND3, VAR_NONE, 0, ACT_NONE, 1, 0, 0,0,
   CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS_2,
    {{0, 0}, {-ANGLE_8, 120}}},
  {TEAM_FRIEND, SHIP_DROM, VAR_NONE, 0, ACT_NONE, 2, 0, 0,0,
   CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS_2,
    {{-ANGLE_4-ANGLE_8, 150}, {-ANGLE_4-ANGLE_8, 120}}},
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 3, 0, 0,0,
   CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS_2,
    {{ANGLE_4+ANGLE_8, 150}, {ANGLE_8, 120}}},
  {TEAM_FRIEND, SHIP_OLD3, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS_2,
    {{ANGLE_2, 180}, {ANGLE_4 + ANGLE_8, 120}}},

  {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 1, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 1, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},

  {TEAM_ENEMY, SHIP_SCOUT3, VAR_NONE, 0, ACT_NONE, 5, 0, 0,0,
   CONVOY_1, FLAGS, FLAGS_2,
    {{0, 0}, {0, 0}}},

  {TEAM_ENEMY, SHIP_FREIGHT, VAR_NONE, 0, ACT_NONE, 6, 0, 0,0,
   CONVOY_2, FLAGS, FLAGS_2,
    {{ANGLE_4 + ANGLE_16, 250}, {0, 0}}},
  {TEAM_ENEMY, SHIP_FREIGHT, VAR_NONE, 0, ACT_NONE, 7, 0, 0,0,
   CONVOY_3, FLAGS, FLAGS_2,
    {{ANGLE_2, 200}, {0, 0}}},
  {TEAM_ENEMY, SHIP_FREIGHT, VAR_LONG, 0, ACT_NONE, 8, 0, 0,0,
   CONVOY_4, FLAGS, FLAGS_2,
    {{-ANGLE_4-ANGLE_16, 250}, {0, 0}}},
  {TEAM_ENEMY, SHIP_FREIGHT, VAR_ANTI, 0, ACT_NONE, 9, 0, 0,0,
   CONVOY_5, FLAGS, FLAGS_2,
    {{ANGLE_2, 400}, {0, 0}}},
  {TEAM_ENEMY, SHIP_FREIGHT, VAR_ANTI, 0, ACT_NONE, 10, 0, 0,0,
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


  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 5, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 5, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 5, 3, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 5, 4, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 5, 5, 0,0,
   CONVOY_NONE, FLAGS, FLAGS_2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 5, 6, 0,0,
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
*/

// THINK ABOUT: making PREF_FIGHTER (e.g. EANTI) turrets FIGHTER_ONLY if there is a WSHIP_ONLY turret on
//  the same ship.
// - also: for escorting or guarding fighters, randomising the distance that they'll follow something each time
//    they start following.
 },

/*

Plan for mission 2:

This is a recon mission.
AM jumps in with small group.
Jumps in too close to large fleet. Turns around immediately.
Fleet jumps out in direction of Commonwealth
But leaves several carriers with small wship escort - number of carriers and wships depends on performance
Carriers attack group with bombers.
Carriers jump out when all FSF bombers/fighters destroyed.
Mission ends when:
 - carriers jump out
 - friendly wships destroyed
 - all enemy ships destroyed

Performance: based on surviving friendly and destroyed enemy ships.


*/


 {
// Stage 2 - messenger
  {
   {""},
   {" This is $BGazer 1$C  to $BGazer group$C. Looks like our jump route may not have been indirect enough. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L2_START1
   {" Scanners are picking up a large convoy of FSF ships! Taking evasive action! ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L2_START2
   {" Energy readings indicate that they are preparing to jump out. ", {WAV_SELECT0, NOTE_2A, COMM_COL_CWLTH}}, // MSG_L2_START3
   {" Looks like some of them decided to stick around. $BGazer 2$C, can you get a scan on those ships? ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L2_START4
   {" There are about six small warships and four larger warships. Several fighters. At their present speed we should be able to outrun them. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L2_START5
   {" Wait - we're picking up some new signals... large fighters, splitting off and heading our way. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L2_START6
   {" $BAngry Moth$C, set course to intercept! ", {WAV_SELECT0, NOTE_2A, COMM_COL_CWLTH}}, // MSG_L2_START7
   {" We're taking heavy damage! All fighters, take those bombers down! ", {WAV_SELECT0, NOTE_2A, COMM_COL_CWLTH}}, // MSG_L2_DAMAGE
   {" Another set of new signals detected. They must be using carriers! ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L2_WAVE2
   {" More bombers detected! How many times can those carriers launch? ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L2_WAVE3

   {" The carrier group is jumping out! ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L2_EJUMP
   {" Looks like we're clear - but that fleet was headed the same way we are. Our orders are to head back to Anenome. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L2_FINISH
   {" All remaining FSF ships destroyed! Good work. Looks like we're clear - but that fleet was headed the same way we are. Our orders are to head back to Anenome. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L2_EJUMP
   {" All FSF fighters shot down! ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L2_EF_DESTROYED
   {" Goodbye, FSF carrier group! Excellent work, pilots. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L2_EWS_DESTROYED

   {" Light cruiser down! ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L2_LOST1
   {" We just lost another cruiser! Those bombers are hitting us hard! ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L2_LOST2
   {" $BGazer group$C  taking heavy losses! Where is our fighter cover?! ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L2_LOST3


  },

  {
   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 1, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{140, 0}, {0,0}}},
   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{0, 100}, {0,0}}},
   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{-140,0}, {0,0}}},
   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 2, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{0, -100}, {0,0}}},
   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{-100, -180}, {0,0}}},
   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{-100, 180}, {0,0}}},

   {TEAM_FRIEND, SHIP_ESCORT1, VAR_NONE, MISSION_GUARD, ACT_AWAY, 1, 1, 0,0,
    CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
   {TEAM_FRIEND, SHIP_ESCORT1, VAR_NONE, MISSION_GUARD, ACT_AWAY, 1, 2, 0,0,
    CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
//   {TEAM_FRIEND, SHIP_ESCORT1, VAR_NONE, MISSION_GUARD, ACT_AWAY, 2, 1, 0,0,
//    CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
//   {TEAM_FRIEND, SHIP_ESCORT1, VAR_NONE, MISSION_GUARD, ACT_AWAY, 2, 2, 0,0,
//    CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},

// this is the carrier group that stays around:
  {TEAM_ENEMY, SHIP_ECARRIER, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_2, FLAGS, FLAGS2,
    {{250,0}, {0, 0}}},
  {TEAM_ENEMY, SHIP_ECARRIER, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_2, FLAGS, FLAGS2,
    {{-250,0}, {0, 0}}},
  {TEAM_ENEMY, SHIP_ECARRIER, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_2, FLAGS, FLAGS2,
    {{0,130}, {0, 0}}},
  {TEAM_ENEMY, SHIP_ECARRIER, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_2, FLAGS, FLAGS2,
    {{0,-130}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 3, 0, 0,0,
   CONVOY_2, FLAGS, FLAGS2,
    {{190,140}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 4, 0, 0,0,
   CONVOY_2, FLAGS, FLAGS2,
    {{190,-140}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 5, 0, 0,0,
   CONVOY_2, FLAGS, FLAGS2,
    {{-190,140}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 6, 0, 0,0,
   CONVOY_2, FLAGS, FLAGS2,
    {{-190,-140}, {0, 0}}},
// the battle report in briefing.c assumes that there will be this number of ships. Update if necessary!

  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 3, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 3, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 4, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 4, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 5, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 5, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 6, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 6, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},

// this is the convoy that jumps out:
  {TEAM_ENEMY, SHIP_SCOUT3, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAGS, FLAG2_NO_SRECORD,
    {{0,0}, {0, 0}}}, // 1
  {TEAM_ENEMY, SHIP_SCOUT3, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAGS, FLAG2_NO_SRECORD,
    {{350,0}, {0, 0}}}, // 1
  {TEAM_ENEMY, SHIP_SCOUT3, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAGS, FLAG2_NO_SRECORD,
    {{-350,0}, {0, 0}}}, // 1
  {TEAM_ENEMY, SHIP_SCOUT3, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAGS, FLAG2_NO_SRECORD,
    {{0,350}, {0, 0}}}, // 1
  {TEAM_ENEMY, SHIP_SCOUT3, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAGS, FLAG2_NO_SRECORD,
    {{0,-350}, {0, 0}}}, // 1
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAGS, FLAG2_NO_SRECORD,
    {{280,280}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAGS, FLAG2_NO_SRECORD,
    {{280,-280}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAGS, FLAG2_NO_SRECORD,
    {{-280,280}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAGS, FLAG2_NO_SRECORD,
    {{-280,-280}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAGS, FLAG2_NO_SRECORD,
    {{450,280}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAGS, FLAG2_NO_SRECORD,
    {{450,-280}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAGS, FLAG2_NO_SRECORD,
    {{-450,280}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAGS, FLAG2_NO_SRECORD,
    {{-450,-280}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAGS, FLAG2_NO_SRECORD,
    {{530,190}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAGS, FLAG2_NO_SRECORD,
    {{530,-190}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAGS, FLAG2_NO_SRECORD,
    {{-530,190}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAGS, FLAG2_NO_SRECORD,
    {{-530,-190}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAGS, FLAG2_NO_SRECORD,
    {{620,0}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAGS, FLAG2_NO_SRECORD,
    {{-620,0}, {0, 0}}},

  {TEAM_NONE} // this is necessary or srecord and other things break

  },

// },


  {
  {

  },

  {TEAM_FRIEND, CONVOY_0, "SCRIPT_L2C0_START",
   STYPE_SETUP_CONVOY, 0, 0, SPEED_F_LIGHT, 0, {0},
    {{PCON_SETUP_CONVOY, {19500, 0, 500}, RESULT_NEXT_SCRIPT, {SCRIPT_L2C0_MOVE1}}, // setup scripts can't have messages
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_FRIEND, CONVOY_0, "SCRIPT_L2C0_MOVE1",
   STYPE_MOVE_XY, 19500, 0, SPEED_F_LIGHT, 0, {0},
   {{PCON_LEVEL_TIME, {3}, RESULT_NONE, {0}, MSG_L2_START1, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {PCON_LEVEL_TIME, {5}, RESULT_NEXT_SCRIPT, {SCRIPT_L2C0_MOVE2}, MSG_L2_START2, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_FRIEND, CONVOY_0, "SCRIPT_L2C0_MOVE2",
   STYPE_MOVE_XY, 2000, 10000, SPEED_F_LIGHT, 0, {0},
   {{PCON_LEVEL_TIME, {8}, RESULT_NONE, {0}, MSG_L2_START3, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {PCON_LEVEL_TIME, {13}, RESULT_NONE, {0}, MSG_L2_START4, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {PCON_LEVEL_TIME, {16}, RESULT_NONE, {0}, MSG_L2_START5, MSG_FROM_COMMAND_1, 2, MSG_TO_ALL},
     {PCON_LEVEL_TIME, {19}, RESULT_NONE, {0}, MSG_L2_START6, MSG_FROM_COMMAND_1, 2, MSG_TO_ALL},
     {PCON_LEVEL_TIME, {22}, RESULT_NONE, {0}, MSG_L2_START7, MSG_FROM_COMMAND_1, 1, MSG_TO_AM}
    },
    {{CCON_NONE},
    }
   },

  {TEAM_ENEMY, CONVOY_1, "SCRIPT_L2C1_START",
   STYPE_SETUP_CONVOY, 9500, -5500, SPEED_E_HEAVY, 0, {0},
    {{PCON_SETUP_CONVOY, {19500, 8000, 200}, RESULT_NEXT_SCRIPT, {SCRIPT_L2C1_MOVE1}}, // setup scripts can't have messages
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_1, "SCRIPT_L2C1_MOVE1",
   STYPE_MOVE_XY, 19500, 8000, SPEED_E_HEAVY, 0, {0},
   {{PCON_LEVEL_TIME, {10}, RESULT_CONVOY_JUMP_OUT, {TEAM_ENEMY, CONVOY_1}, MSG_L2_NONE},
     {PCON_LEVEL_TIME, {11}, RESULT_END_SCRIPT},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },

  {TEAM_ENEMY, CONVOY_2, "SCRIPT_L2C2_START",
   STYPE_SETUP_CONVOY, 8200, -6800, SPEED_E_HEAVY, 0, {0},
    {{PCON_SETUP_CONVOY, {19500, 8000, 200}, RESULT_NEXT_SCRIPT, {SCRIPT_L2C2_MOVE1}}, // setup scripts can't have messages
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_2, "SCRIPT_L2C2_MOVE1",
   STYPE_MOVE_XY, 19500, 8000, SPEED_E_HEAVY, 0, {0},
   {{PCON_LEVEL_TIME, {17}, RESULT_LAUNCH, {SHIP_BOMBER}, MSG_L2_NONE},
    {PCON_LEVEL_TIME, {18}, RESULT_LAUNCH, {SHIP_FIGHTER}, MSG_L2_NONE},
    {PCON_LEVEL_TIME, {19}, RESULT_LAUNCH, {SHIP_BOMBER}, MSG_L2_NONE},
    {PCON_LEVEL_TIME, {20}, RESULT_LAUNCH, {SHIP_FIGHTER}, MSG_L2_NONE},
    {PCON_LEVEL_TIME, {20}, RESULT_NEXT_SCRIPT, {SCRIPT_L2C2_LAUNCH2}, MSG_L2_NONE},
//     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },

  {TEAM_ENEMY, CONVOY_2, "SCRIPT_L2C2_LAUNCH2",
   STYPE_MOVE_XY, 19500, 8000, SPEED_E_HEAVY, 0, {0},
   {{PCON_TIME, {57}, RESULT_LAUNCH, {SHIP_BOMBER}, MSG_L2_NONE},
    {PCON_TIME, {58}, RESULT_LAUNCH, {SHIP_FIGHTER}, MSG_L2_NONE},
    {PCON_TIME, {59}, RESULT_LAUNCH, {SHIP_BOMBER}, MSG_L2_WAVE2, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {PCON_TIME, {60}, RESULT_LAUNCH, {SHIP_FIGHTER}, MSG_L2_NONE},
    {PCON_TIME, {60}, RESULT_NEXT_SCRIPT, {SCRIPT_L2C2_LAUNCH3}, MSG_L2_NONE}
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_2, "SCRIPT_L2C2_LAUNCH3",
   STYPE_MOVE_XY, 19500, 8000, SPEED_E_HEAVY, 0, {0},
   {{PCON_TIME, {57}, RESULT_LAUNCH, {SHIP_BOMBER}, MSG_L2_NONE},
    {PCON_TIME, {58}, RESULT_LAUNCH, {SHIP_FIGHTER}, MSG_L2_NONE},
    {PCON_TIME, {59}, RESULT_LAUNCH, {SHIP_BOMBER}, MSG_L2_WAVE3, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {PCON_TIME, {60}, RESULT_LAUNCH, {SHIP_FIGHTER}, MSG_L2_NONE},
    {PCON_TIME, {60}, RESULT_NEXT_SCRIPT, {SCRIPT_L2C2_LEAVE}, MSG_L2_NONE}
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_2, "SCRIPT_L2C2_LEAVE",
   STYPE_MOVE_XY, 19500, 8000, SPEED_E_HEAVY, 0, {0},
   {{PCON_TIME, {80}, RESULT_CONVOY_JUMP_OUT, {TEAM_ENEMY, CONVOY_2}, MSG_L2_EJUMP, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {PCON_TIME, {82}, RESULT_NEXT_SCRIPT, {SCRIPT_L2C2_LEFT}, MSG_L2_NONE},
    {PCON_NONE}
    },
    {{CCON_NONE},
    }
   },
// This script runs once to see whether anything is left when the carriers jumped out (not if they were destroyed), and is then killed:
// If when the carriers leave there are no enemy ships left, mission is over.
  {TEAM_ENEMY, CONVOY_2, "SCRIPT_L2C2_LEFT",
   STYPE_MOVE_XY, 19500, 8000, SPEED_E_HEAVY, 0, {0},
   {{PCON_NO_ENEMIES_LEFT, {0}, RESULT_MISSION_OVER, {0}, MSG_L2_FINISH, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {PCON_NO_ENEMIES_LEFT, {0}, RESULT_NO_MORE_MESSAGES, {0}, MSG_L2_NONE},
    {PCON_ALWAYS_PASS, {0}, RESULT_END_SCRIPT, {0}},
    },
    {{CCON_NONE},
    }
   },
// This script runs the whole time:
// Note that CCON_EFIGHTERS_DESTROYED_REMAIN can occur multiple times, if all EFs are destroyed then carriers launch again
  {TEAM_FRIEND, CONVOY_NONE, "SCRIPT_L2_CHECK_COMPLETION",
   STYPE_START_WAIT, 0, 0, 0, 0, {0},
    {{PCON_NONE},
    },
   {{CCON_EALL_DESTROYED, {0}, RESULT_MISSION_OVER, {0}, MSG_L2_FSF_DESTROYED, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {CCON_EALL_DESTROYED, {0}, RESULT_NO_MORE_MESSAGES},
    {CCON_EFIGHTERS_DESTROYED_REMAIN, {0}, RESULT_NONE, {0}, MSG_L2_EF_DESTROYED, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {CCON_EWSHIPS_DESTROYED_FREMAIN, {0}, RESULT_NONE, {0}, MSG_L2_EWS_DESTROYED, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {CCON_CONVOY_DESTROYED, {CONVOY_0}, RESULT_MISSION_FAIL, {0}},
   }
   },
// This script runs the whole time:
  {TEAM_FRIEND, CONVOY_NONE, "SCRIPT_L2_LOSS",
   STYPE_START_WAIT, 0, 0, 0, 0, {0},
    {{PCON_NONE},
    },
   {{CCON_CONVOY_REDUCED, {CONVOY_0, 5}, RESULT_NONE, {0}, MSG_L2_LOST1, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {CCON_CONVOY_REDUCED, {CONVOY_0, 4}, RESULT_NONE, {0}, MSG_L2_LOST2, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {CCON_CONVOY_REDUCED, {CONVOY_0, 2}, RESULT_NONE, {0}, MSG_L2_LOST3, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {CCON_NONE}
   }
   },


/*
   {""},
   {" This is $BAlpha 1$C  to $BAlpha group$C. Looks like our jump route may not have been indirect enough. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L2_START1
   {" Scanners are picking up a large convoy of FSF ships. Taking evasive action! ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L2_START2
   {" Energy readings indicate that they are preparing to jump out. ", {WAV_SELECT0, NOTE_2A, COMM_COL_CWLTH}}, // MSG_L2_START3
   {" Looks like some of them decided to stick around. $BAlpha 2$C, can you get a scan on those ships? ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L2_START4
   {" There are about six small warships similar to the ones we met at (system), and four larger warships. Several fighters. At their present speed we should be able to outrun them. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L2_START5
   {" Wait - we're picking up some new signals... large fighters, splitting off from the warships. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L2_START6
   {" Those fighters are headed our way. $BAngry Moth$C, set course to intercept! ", {WAV_SELECT0, NOTE_2A, COMM_COL_CWLTH}}, // MSG_L2_START7
   {" We're taking heavy damage! All fighters, take those bombers down! ", {WAV_SELECT0, NOTE_2A, COMM_COL_CWLTH}}, // MSG_L2_DAMAGE
   {" Another set of new signals detected. Those warships must be some kind of carrier. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L2_WAVE2
   {" More bombers detected! How many times can those carriers launch? ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L2_WAVE3

   {" The carrier group has jumped out. Looks like we're clear - but those ships were headed the same way we are. Our orders are to return to Anenome. ", {WAV_SELECT0, NOTE_2A, COMM_COL_CWLTH}}, // MSG_L2_FINISH
   {" The carrier group has jumped out. Looks like we're clear - but those ships were headed the same way we are. Our orders are to head back to Anenome. ", {WAV_SELECT0, NOTE_2A, COMM_COL_CWLTH}}, // MSG_L2_START7

*/


  },

 },


 {
// Stage 3 - convoy raid
  {
   {""},
   {" Looks like our intel was on the money this time. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L3_START1
   {" $BGazer group$C, adjust course to intercept. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L3_START2
   {" $BAngry Moth$C, head out and see if you can draw some fighters away from the transport convoy. ", {WAV_SELECT1, NOTE_2G, COMM_COL_TO_AM}}, // MSG_L3_START3
   {" $BGazer 1$C, are there any carriers guarding the transports? ", {WAV_SELECT0, NOTE_2A, COMM_COL_CWLTH}}, // MSG_L3_START4_A2
   {" Negative, $BGazer 2$C. We're only picking up transports, escort fighters and light warships. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L3_START5
   {" Awesome. I hate those things. ", {WAV_SELECT0, NOTE_2A, COMM_COL_CWLTH}}, // MSG_L3_START6_A2
   {" Several ships just split off from the main convoy. ", {WAV_SELECT0, NOTE_2A, COMM_COL_CWLTH}}, // MSG_L3_TURN1_A2
   {" They're turning around! ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L3_TURN2
   {" $BGazer group$C, prepare to engage! Fighters, keep their tailguns busy! ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L3_TURN3

   {" That's most of their defence out of the way. Full speed towards the transports! ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L3_ATTACK
   {" $BGazer group$C, move to broadside formation and prepare to engage at close range! ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L3_CHANGE_FORMATION
   {" $BGazer group$C, prepare to engage with the freighter group! ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L3_CHANGE_FORMATION2

   {" Light cruiser down! ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L3_LOSS1
   {" We just lost another cruiser! ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L3_LOSS2
   {" We're suffering heavy losses! $BGazer group$C, pull out! ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L3_LOSS3

   {" There goes the first transport! ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L3_TARGET1
   {" Another transport down. Keep up the good work. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L3_TARGET2
   {" That's all of the transports! We've done a lot of damage out here today. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L3_TARGET3
// need to deal with what happens when freighters jumping out and last one is destroyed but some have escaped.

   {" Energy readings indicate that the transports will jump out any second. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L3_ETOJUMP
   {" The FSF ships are jumping out! ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L3_EJUMP
   {" There are still some fighters left. Angry Moth, clean them up and we can head home. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L3_CLEANUP
   {" Mission's over. Gazer group charging up to head back to base. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L3_OVER




  },

  {
// arrange 1 is spread-out battle formation. arrange 2 is crescent formation (no longer used):
   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 1, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{140, 0}, {20,0}, {0, 90}}},
   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 2, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{-140,0}, {-180,0}, {0, -90}}},

   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{0, 100}, {180,120}, {90, 180}}},
   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{0, -100}, {180,-120}, {90, -180}}},

   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{-100, -180}, {-380,-120}, {190, 280}}},
   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{-100, 180}, {-380,120}, {190, -280}}},

   {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 1, 1, 0,0,
    CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
   {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 1, 2, 0,0,
    CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
   {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 2, 1, 0,0,
    CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
   {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 2, 2, 0,0,
    CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},

// this is the freighter convoy:
  {TEAM_ENEMY, SHIP_FREIGHT, VAR_NONE, 0, ACT_NONE, 3, 0, 0,0,
   CONVOY_1, FLAGS, FLAGS2,
    {{250,0}, {0, 0}}},
  {TEAM_ENEMY, SHIP_FREIGHT, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAGS, FLAGS2,
    {{0,0}, {0, 0}}},
  {TEAM_ENEMY, SHIP_FREIGHT, VAR_NONE, 0, ACT_NONE, 4, 0, 0,0,
   CONVOY_1, FLAGS, FLAGS2,
    {{-250,0}, {0, 0}}},
  {TEAM_ENEMY, SHIP_FREIGHT, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAGS, FLAGS2,
    {{110,200}, {0, 0}}},
  {TEAM_ENEMY, SHIP_FREIGHT, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAGS, FLAGS2,
    {{-110,200}, {0, 0}}},
  {TEAM_ENEMY, SHIP_FREIGHT, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAGS, FLAGS2,
    {{110,-200}, {0, 0}}},
  {TEAM_ENEMY, SHIP_FREIGHT, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAGS, FLAGS2,
    {{-110,-200}, {0, 0}}},

// some of the freighters have fighter guards:
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 3, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 3, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 4, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 4, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},

// Convoy 2 is warships which stay with the freighters. Convoy centre follows that of convoy_1
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 5, 0, 0,0,
   CONVOY_2, FLAGS, FLAGS2,
    {{-420,0}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_2, FLAGS, FLAGS2,
    {{-390,120}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_2, FLAGS, FLAGS2,
    {{-390,-120}, {0, 0}}},
/*  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 6, 0, 0,0,
   CONVOY_2, FLAGS, FLAGS2,
    {{-330,210}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 7, 0, 0,0,
   CONVOY_2, FLAGS, FLAGS2,
    {{-330,-210}, {0, 0}}},*/
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 8, 0, 0,0,
   CONVOY_2, FLAGS, FLAGS2,
    {{-150,290}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 9, 0, 0,0,
   CONVOY_2, FLAGS, FLAGS2,
    {{-150,-290}, {0, 0}}},
// these ones are in front of the freighters:
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_2, FLAGS, FLAGS2,
    {{290,130}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_2, FLAGS, FLAGS2,
    {{290,-130}, {0, 0}}},


  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 5, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 5, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 6, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 6, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 7, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
/*  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 7, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 8, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 8, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 9, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 9, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},*/


// Convoy 3 is warships which turn to fight Alpha group. Spreads out to battle formation

  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_3, FLAGS, FLAGS2,
    {{150,100}, {210, 100}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_3, FLAGS, FLAGS2,
    {{150,-100}, {210, -100}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 10, 0, 0,0,
   CONVOY_3, FLAGS, FLAGS2,
    {{0,180}, {0, 120}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 11, 0, 0,0,
   CONVOY_3, FLAGS, FLAGS2,
    {{0,-180}, {0, -120}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_3, FLAGS, FLAGS2,
    {{-150,100}, {-210, 100}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_3, FLAGS, FLAGS2,
    {{-150,-100}, {-210, -100}}},
//  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 12, 0, 0,0,
//   CONVOY_3, FLAGS, FLAGS2,
//    {{0,0}, {0, 0}}},
/*  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_3, FLAGS, FLAGS2,
    {{-220,160}, {-380, 80}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 12, 0, 0,0,
   CONVOY_3, FLAGS, FLAGS2,
    {{-220,-160}, {-380, -80}}},
*/
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 10, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 10, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 11, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 11, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 12, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 12, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},

  {TEAM_NONE} // this is necessary or srecord and other things break

  },

// },


  {
  {

  },

  {TEAM_FRIEND, CONVOY_0, "SCRIPT_L3C0_START",
   STYPE_SETUP_CONVOY, -4000, 2500, SPEED_F_LIGHT, 0, {0},
    {{PCON_SETUP_CONVOY, {19500, 0, 500}, RESULT_NEXT_SCRIPT, {SCRIPT_L3C0_MOVE1}}, // setup scripts can't have messages
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_FRIEND, CONVOY_0, "SCRIPT_L3C0_MOVE1",
   STYPE_APPROACH_CONVOY, CONVOY_2, APPROACH_LEAD, SPEED_F_LIGHT, 0, {0},
   {
    {PCON_CONVOY_DIST, {CONVOY_3, 2800}, RESULT_NEXT_SCRIPT, {SCRIPT_L3C0_MOVE2}, MSG_L3_CHANGE_FORMATION, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {PCON_NONE},
    },
    {{CCON_CONVOY_DESTROYED, {CONVOY_0}, RESULT_MISSION_FAIL, {0}},
     {CCON_CONVOY_REDUCED, {CONVOY_0, 2}, RESULT_NEXT_SCRIPT, {SCRIPT_L3C0_RETREAT}, MSG_L3_LOSS3, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    }
   },
// this is just like the previous script, but with arrange=1 (wider spread for wship/wship battle)
  {TEAM_FRIEND, CONVOY_0, "SCRIPT_L3C0_MOVE2",
   STYPE_APPROACH_CONVOY, CONVOY_2, APPROACH_LEAD, SPEED_F_LIGHT, 1, {0},
   {
    {PCON_CONVOY_DIST, {CONVOY_2, 2800}, RESULT_NEXT_SCRIPT, {SCRIPT_L3C0_MOVE3}, MSG_L3_CHANGE_FORMATION2, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {PCON_NONE},
    },
    {{CCON_CONVOY_DESTROYED, {CONVOY_0}, RESULT_MISSION_FAIL, {0}},
     {CCON_CONVOY_REDUCED, {CONVOY_0, 2}, RESULT_NEXT_SCRIPT, {SCRIPT_L3C0_RETREAT}, MSG_L3_LOSS3, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    }
   },
// just like move2 but now approaching at an offset
  {TEAM_FRIEND, CONVOY_0, "SCRIPT_L3C0_MOVE3",
   STYPE_APPROACH_CONVOY_OFFSET, CONVOY_2, APPROACH_LEAD, SPEED_F_LIGHT, 1, {0, -800},
   {
     {PCON_NONE},
    },
    {{CCON_CONVOY_DESTROYED, {CONVOY_0}, RESULT_MISSION_FAIL, {0}},
     {CCON_CONVOY_REDUCED, {CONVOY_0, 2}, RESULT_NEXT_SCRIPT, {SCRIPT_L3C0_RETREAT}, MSG_L3_LOSS3, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    }
   },
// Losses too heavy
  {TEAM_FRIEND, CONVOY_0, "SCRIPT_L3C0_RETREAT",
   STYPE_MOVE_XY, -10000, -100000, SPEED_F_LIGHT, 0, {0},
   {
     {PCON_NONE},
    },
    {{CCON_CONVOY_DESTROYED, {CONVOY_0}, RESULT_MISSION_FAIL, {0}},
    }
   },

  {TEAM_NONE, CONVOY_NONE, "SCRIPT_L3_START",
   STYPE_START_WAIT, 0, 0, 0, 0, {0},
   {{PCON_TIME, {3}, RESULT_NONE, {0}, MSG_L3_START1, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {PCON_TIME, {5}, RESULT_NONE, {0}, MSG_L3_START2, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {PCON_TIME, {7}, RESULT_NONE, {0}, MSG_L3_START3, MSG_FROM_COMMAND_1, 1, MSG_TO_AM},
    {PCON_TIME, {10}, RESULT_NONE, {0}, MSG_L3_START4_A2, MSG_FROM_COMMAND_1, 2, MSG_TO_ALL},
    {PCON_TIME, {13}, RESULT_NEXT_SCRIPT, {SCRIPT_L3_START2}, MSG_L3_START5, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_NONE, CONVOY_NONE, "SCRIPT_L3_START2",
   STYPE_WAIT, 0, 0, 0, 0, {0},
   {{PCON_TIME, {3}, RESULT_END_SCRIPT, {0}, MSG_L3_START6_A2, MSG_FROM_COMMAND_1, 2, MSG_TO_ALL},
    {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },

// Convoy 1 is the freight convoy. It doesn't really do anything except jump out eventually:
// (although we might make the freighters scatter when they're under attack?)
  {TEAM_ENEMY, CONVOY_1, "SCRIPT_L3C1_START",
   STYPE_SETUP_CONVOY, 2000, -1500, SPEED_F_HEAVY, 0, {0},
    {{PCON_SETUP_CONVOY, {12000, -19000, 200}, RESULT_NEXT_SCRIPT, {SCRIPT_L3C1_MOVE}}, // setup scripts can't have messages
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_1, "SCRIPT_L3C1_MOVE",
   STYPE_MOVE_XY, 12000, -19000, SPEED_F_HEAVY, 0, {0},
   {{PCON_LEVEL_TIME, {400}, RESULT_CONVOY_JUMP_OUT, {TEAM_ENEMY, CONVOY_1}, MSG_NONE},
    {PCON_LEVEL_TIME, {401}, RESULT_END_SCRIPT, {}, MSG_NONE},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },

// Convoy 2 is the wships guarding the freight convoy. It doesn't really do anything except jump out eventually:
// It should follow Convoy 1 exactly
  {TEAM_ENEMY, CONVOY_2, "SCRIPT_L3C2_START",
   STYPE_SETUP_CONVOY, 2000, -1500, SPEED_F_HEAVY, 0, {0},
    {{PCON_SETUP_CONVOY, {12000, -19000, 200}, RESULT_NEXT_SCRIPT, {SCRIPT_L3C2_MOVE}}, // setup scripts can't have messages
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_2, "SCRIPT_L3C2_MOVE",
   STYPE_MOVE_XY, 12000, -19000, SPEED_F_HEAVY, 0, {0},
   {{PCON_LEVEL_TIME, {400}, RESULT_CONVOY_JUMP_OUT, {TEAM_ENEMY, CONVOY_2}, MSG_NONE},
    {PCON_LEVEL_TIME, {401}, RESULT_END_SCRIPT, {}, MSG_NONE},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },

// Convoy 3 is the wships that turn back and accelerate to engage Alpha group.

  {TEAM_ENEMY, CONVOY_3, "SCRIPT_L3C3_START",
   STYPE_SETUP_CONVOY, 1500, -1000, SPEED_F_HEAVY, 0, {0},
    {{PCON_SETUP_CONVOY, {12000, -19000, 200}, RESULT_NEXT_SCRIPT, {SCRIPT_L3C3_MOVE}}, // setup scripts can't have messages
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_3, "SCRIPT_L3C3_MOVE",
   STYPE_MOVE_XY, 12000, -19000, SPEED_F_HEAVY, 0, {0},
   {{PCON_LEVEL_TIME, {20}, RESULT_NEXT_SCRIPT, {SCRIPT_L3C3_MOVE2}, MSG_NONE},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_3, "SCRIPT_L3C3_MOVE2",
   STYPE_APPROACH_CONVOY_OFFSET, CONVOY_0, APPROACH_NO_LEAD, SPEED_F_LIGHT, 0, {380, 480},
   {{PCON_LEVEL_TIME, {25}, RESULT_NONE, {0}, MSG_L3_TURN1_A2, MSG_FROM_COMMAND_1, 2, MSG_TO_ALL},
    {PCON_LEVEL_TIME, {29}, RESULT_NONE, {0}, MSG_L3_TURN2, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {PCON_CONVOY_DIST, {CONVOY_0, 2800}, RESULT_NEXT_SCRIPT, {SCRIPT_L3C3_MOVE3}, MSG_NONE},
    {PCON_LEVEL_TIME, {400}, RESULT_CONVOY_JUMP_OUT, {TEAM_ENEMY, CONVOY_3}, MSG_NONE},
    {PCON_LEVEL_TIME, {401}, RESULT_END_SCRIPT, {0}, MSG_NONE},
    },
    {{CCON_CONVOY_DESTROYED, {CONVOY_3}, RESULT_END_SCRIPT, {0}, MSG_L3_ATTACK, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {CCON_NONE},
    }
   },
// change formation when approaching
  {TEAM_ENEMY, CONVOY_3, "SCRIPT_L3C3_MOVE3",
   STYPE_MOVE_FORWARDS, 0, 0, 3, 1, {0, 0},
   {{PCON_CONVOY_DIST_APART, {CONVOY_0, 1750}, RESULT_NEXT_SCRIPT, {SCRIPT_L3C3_MOVE2}, MSG_NONE},
    {PCON_LEVEL_TIME, {400}, RESULT_CONVOY_JUMP_OUT, {TEAM_ENEMY, CONVOY_3}, MSG_NONE},
    {PCON_LEVEL_TIME, {401}, RESULT_END_SCRIPT, {CONVOY_3}, MSG_NONE},
     {PCON_NONE},
    },
    {{CCON_CONVOY_DESTROYED, {CONVOY_3}, RESULT_END_SCRIPT, {0}, MSG_L3_ATTACK, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {CCON_NONE},
    }
   },


  {TEAM_NONE, CONVOY_NONE, "SCRIPT_L3_MESSAGES",
   STYPE_START_WAIT, 0, 0, 0, 0, {0},
   {{PCON_LEVEL_TIME, {390}, RESULT_NONE, {0}, MSG_L3_ETOJUMP, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
//    {PCON_LEVEL_TIME, {401}, RESULT_NEXT_SCRIPT, {SCRIPT_L3_CLEANUP}, MSG_L3_EJUMP, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {PCON_LEVEL_TIME, {401}, RESULT_END_SCRIPT, {0}, MSG_L3_EJUMP, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {PCON_NONE},
    },
    {{CCON_NONE}
    }
   },

  {TEAM_NONE, CONVOY_NONE, "SCRIPT_L3_CLEANUP",
   STYPE_START_WAIT, 0, 0, 0, 0, {0},
   {{PCON_NO_ENEMIES_LEFT, {}, RESULT_NEXT_SCRIPT, {SCRIPT_L3_FINISH}, MSG_NONE},
// PCON_NO_ENEMIES_LEFT could be triggered either by killing all enemies or by them all jumping out.
     {PCON_LEVEL_TIME, {404}, RESULT_NONE, {0}, MSG_L3_CLEANUP, MSG_FROM_COMMAND_1, 1, MSG_TO_AM},
// If we get to 404 without PCON_NO_ENEMIES_LEFT being triggered, there must be FSF fighters left who didn't get picked up.
// Get rid of them before we go to FINISH
     {PCON_NONE},
    },
    {{CCON_EWSHIPS_DESTROYED_FREMAIN, {}, RESULT_NEXT_SCRIPT, {SCRIPT_L3_JUST_FIGHTERS}, MSG_L3_CLEANUP, MSG_FROM_COMMAND_1, 1, MSG_TO_AM},
     {CCON_NONE}
    }
   },

  {TEAM_NONE, CONVOY_NONE, "SCRIPT_L3_JUST_FIGHTERS",
   STYPE_WAIT, 0, 0, 0, 0, {0},
   {{PCON_NO_ENEMIES_LEFT, {}, RESULT_NEXT_SCRIPT, {SCRIPT_L3_FINISH}, MSG_NONE},
     {PCON_NONE},
    },
    {{CCON_NONE}
    }
   },


  {TEAM_NONE, CONVOY_NONE, "SCRIPT_L3_FINISH",
   STYPE_WAIT, 0, 0, 0, 0, {0},
   {{PCON_TIME, {1}, RESULT_MISSION_OVER, {0}, MSG_L3_OVER, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {PCON_NONE},
    },
    {{CCON_NONE}
    }
   },

  {TEAM_NONE, CONVOY_NONE, "SCRIPT_L3_LOSS",
   STYPE_START_WAIT, 0, 0, 0, 0, {0},
   {{PCON_NONE},
    },
    {{CCON_CONVOY_REDUCED, {CONVOY_0, 5}, RESULT_NONE, {0}, MSG_L3_LOSS1, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {CCON_CONVOY_REDUCED, {CONVOY_0, 3}, RESULT_NONE, {0}, MSG_L3_LOSS2, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
// MSG_L3_LOSS3 dealt with above (losing 3 ships makes convoy retreat)
     {CCON_NONE}
    }
   },

  {TEAM_NONE, CONVOY_NONE, "SCRIPT_L3_DESTROY",
   STYPE_START_WAIT, 0, 0, 0, 0, {0},
   {{PCON_NONE},
    },
    {{CCON_CONVOY_REDUCED, {CONVOY_1, 6}, RESULT_NONE, {0}, MSG_L3_TARGET1, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {CCON_CONVOY_REDUCED, {CONVOY_1, 3}, RESULT_NONE, {0}, MSG_L3_TARGET2, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {CCON_CONVOY_DESTROYED, {CONVOY_1}, RESULT_END_SCRIPT, {0}, MSG_L3_TARGET3, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {CCON_NONE}
    }
   },

   },



 },


 {
// Stage 4 - convoy defence
  {
   {""},
   {" Hello $BGazer Group$C, this is $BCaravan 3$C. Do you read me? ", {WAV_SELECT0, NOTE_2E, COMM_COL_CWLTH}}, // MSG_L4_START1_FR
   {" Clear as a crisp Spring morning, $BCaravan 3$C. We have you on our scanners. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L4_START2
   {" Good to hear your voice. What's your jump status? We're looking at about seven minutes before our drives are ready to go. ", {WAV_SELECT0, NOTE_2E, COMM_COL_CWLTH}}, // MSG_L4_START3_FR
   {" We'll be ready in five. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L4_START4
   {" Good, good. ", {WAV_SELECT0, NOTE_2E, COMM_COL_CWLTH}}, // MSG_L4_START5_FR

   {" $BGazer 1$C, we're picking up incoming jump signatures. Profiles indicate FSF warships. ", {WAV_SELECT0, NOTE_2A, COMM_COL_CWLTH}}, // MSG_L4_START6_A2
   {" Damn. Sorry $BCaravan 3$C, it looks like we're in for a fight today. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L4_START7
   {" I was afraid you were going to say that. ", {WAV_SELECT0, NOTE_2E, COMM_COL_CWLTH}}, // MSG_L4_START8_FR
   {" $BAngry Moth$C, get ready to scramble. Protecting those freighters is our absolute priority. ", {WAV_SELECT1, NOTE_2G, COMM_COL_TO_AM}}, // MSG_L4_START9

   {" Bomber launch detected! ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L4_LAUNCH1
   {" More bombers incoming! Can someone take out those carriers? ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L4_LAUNCH2
   {" More bombers on their way! ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L4_LAUNCH3

   {" $BGazer Group$C, prepare to engage FSF warships. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L4_C2_ATTACKS
   {" $BCaravan 3$C  under attack! ", {WAV_SELECT0, NOTE_2E, COMM_COL_CWLTH}}, // MSG_L4_C3_ATTACKS

   {" Time to jump home. All fighters report in. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L4_JUMP
   {" FSF warship formation down! ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L4_C2_DESTROYED
   {" FSF warship formation down! ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L4_C3_DESTROYED
   {" All FSF carriers out of action. No more bombers! ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L4_CARRIERS_DISABLED

   {" $BGazer Group$C  needs fighter support! ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L4_C0_LOSS1
   {" $BGazer Group$C  taking heavy damage! ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L4_C0_LOSS2
// this one is said by a freighter:
   {" Oh no - we've lost $BGazer Group$C! ", {WAV_SELECT0, NOTE_2E, COMM_COL_CWLTH}}, // MSG_L4_C0_LOSS3_FR

   {" Freighter down! ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L4_C1_LOSS1
   {" We're losing too many freighters! ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L4_C1_LOSS2
   {" That's the last of the freighters. Damn, Anenome needed those supplies. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L4_C1_LOSS3

  },

  {
// Alpha Group:
   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 1, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{140, 0}, {0,0}}},
   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{0, 100}, {0,0}}},
   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{-140,0}, {0,0}}},
   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 2, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{0, -100}, {0,0}}},
   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{-100, -180}, {0,0}}},
   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{-100, 180}, {0,0}}},
// if adding or removing, remember to change values in CONVOY_REDUCED scripts

   {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 1, 1, 0,0,
    CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
   {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 1, 2, 0,0,
    CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
   {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 2, 1, 0,0,
    CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
   {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 2, 2, 0,0,
    CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},

// Convoy 1 is the OCSF freighter convoy:
  {TEAM_FRIEND, SHIP_DROM, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAG_COMMAND_2|FLAG_BETA|FLAG_SPAWN_2, FLAGS2,
    {{180,120}, {0, 0}}},
  {TEAM_FRIEND, SHIP_DROM, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAG_COMMAND_2|FLAG_BETA|FLAG_SPAWN_2, FLAGS2,
    {{0,120}, {0, 0}}},
  {TEAM_FRIEND, SHIP_DROM, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAG_COMMAND_2|FLAG_BETA|FLAG_SPAWN_2, FLAGS2,
    {{-180,120}, {0, 0}}},
  {TEAM_FRIEND, SHIP_DROM, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAG_COMMAND_2|FLAG_BETA|FLAG_SPAWN_2, FLAGS2,
    {{180,-120}, {0, 0}}},
  {TEAM_FRIEND, SHIP_DROM, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAG_COMMAND_2|FLAG_BETA|FLAG_SPAWN_2, FLAGS2,
    {{0,-120}, {0, 0}}},
  {TEAM_FRIEND, SHIP_DROM, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAG_COMMAND_2|FLAG_BETA|FLAG_SPAWN_2, FLAGS2,
    {{-180,-120}, {0, 0}}},
// with a small and pretty inadequate escort:
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 3, 0, 0,0,
   CONVOY_1, FLAG_COMMAND_2|FLAG_BETA|FLAG_SPAWN_2, FLAGS2,
    {{80,220}, {0, 0}}},
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 4, 0, 0,0,
   CONVOY_1, FLAG_COMMAND_2|FLAG_BETA|FLAG_SPAWN_2, FLAGS2,
    {{80,-220}, {0, 0}}},
  {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAG_COMMAND_2|FLAG_BETA|FLAG_SPAWN_2, FLAGS2,
    {{-230,0}, {0, 0}}},
// if adding or removing, remember to change values in CONVOY_REDUCED scripts

   {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 3, 1, 0,0,
    CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
   {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 3, 2, 0,0,
    CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
   {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 4, 1, 0,0,
    CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
   {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 4, 2, 0,0,
    CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},

// Convoy 2 is the large FSF wship convoy that goes for Alpha group:
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_2, FLAG_JUMP_1, FLAGS2,
    {{180,0}, {280, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_2, FLAG_JUMP_1, FLAGS2,
    {{150,150}, {180, 100}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_2, FLAG_JUMP_1, FLAGS2,
    {{150,-150}, {180, -100}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 5, 0, 0,0,
   CONVOY_2, FLAG_JUMP_1, FLAGS2,
    {{30,0}, {30, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 6, 0, 0,0,
   CONVOY_2, FLAG_JUMP_1, FLAGS2,
    {{0,150}, {-150, 130}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 7, 0, 0,0,
   CONVOY_2, FLAG_JUMP_1, FLAGS2,
    {{0,-150}, {-150, -130}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_LONG, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_2, FLAG_JUMP_1, FLAGS2,
    {{-180,0}, {-380, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_LONG, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_2, FLAG_JUMP_1, FLAGS2,
    {{-150,150}, {-250, 120}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_LONG, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_2, FLAG_JUMP_1, FLAGS2,
    {{-150,-150}, {-250, -120}}},

  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 5, 1, 0,0,
   CONVOY_NONE, FLAG_JUMP_1, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 5, 2, 0,0,
   CONVOY_NONE, FLAG_JUMP_1, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 6, 1, 0,0,
   CONVOY_NONE, FLAG_JUMP_1, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 6, 2, 0,0,
   CONVOY_NONE, FLAG_JUMP_1, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 7, 1, 0,0,
   CONVOY_NONE, FLAG_JUMP_1, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 7, 2, 0,0,
   CONVOY_NONE, FLAG_JUMP_1, FLAGS2, {{0,0}}},

// Convoy 3 is the small (maybe no longer small) FSF wship convoy that goes for Caravan 3:
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_3, FLAG_JUMP_2, FLAGS2,
    {{180,0}, {280, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_3, FLAG_JUMP_2, FLAGS2,
    {{150,150}, {120, 120}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_3, FLAG_JUMP_2, FLAGS2,
    {{150,-150}, {120, -120}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 7, 0, 0,0,
   CONVOY_3, FLAG_JUMP_2, FLAGS2,
    {{0,80}, {0, 130}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 8, 0, 0,0,
   CONVOY_3, FLAG_JUMP_2, FLAGS2,
    {{0,-80}, {0, -130}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_LONG, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_3, FLAG_JUMP_2, FLAGS2,
    {{-180,0}, {-280, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_3, FLAG_JUMP_2, FLAGS2,
    {{-150,150}, {-200, 150}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_LONG, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_3, FLAG_JUMP_2, FLAGS2,
    {{-150,-150}, {-200, 150}}},

  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 7, 1, 0,0,
   CONVOY_NONE, FLAG_JUMP_2, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 7, 2, 0,0,
   CONVOY_NONE, FLAG_JUMP_2, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 8, 1, 0,0,
   CONVOY_NONE, FLAG_JUMP_2, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 8, 2, 0,0,
   CONVOY_NONE, FLAG_JUMP_2, FLAGS2, {{0,0}}},

// Convoy 4 is the carrier group that just sits there and launches a few bombers at whichever OCSF group is closest:
  {TEAM_ENEMY, SHIP_ECARRIER, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_4, FLAG_JUMP_3, FLAGS2,
    {{0,0}, {0, 0}}},
  {TEAM_ENEMY, SHIP_ECARRIER, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_4, FLAG_JUMP_3, FLAGS2,
    {{90,170}, {0, 0}}},
  {TEAM_ENEMY, SHIP_ECARRIER, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_4, FLAG_JUMP_3, FLAGS2,
    {{90,-170}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 9, 0, 0,0,
   CONVOY_4, FLAG_JUMP_3, FLAGS2,
    {{-120,160}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 10, 0, 0,0,
   CONVOY_4, FLAG_JUMP_3, FLAGS2,
    {{-120,-160}, {0, 0}}},

  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 9, 1, 0,0,
   CONVOY_NONE, FLAG_JUMP_3, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 9, 2, 0,0,
   CONVOY_NONE, FLAG_JUMP_3, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 10, 1, 0,0,
   CONVOY_NONE, FLAG_JUMP_3, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 10, 2, 0,0,
   CONVOY_NONE, FLAG_JUMP_3, FLAGS2, {{0,0}}},

  {TEAM_NONE} // this is necessary or srecord and other things break

  },

// },


  {
  {

  },
/*
   {""},
   {" Hello $BAlpha Group$C, this is $BCaravan 3$C. Do you read me? ", {WAV_SELECT0, NOTE_2E, COMM_COL_CWLTH}}, // MSG_L4_START1_FR
   {" Clear as a crisp Spring morning, $BCaravan 3$C. We have you on our scanners. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L4_START2
   {" Good to hear your voice. What's your jump status? We're looking at about seven minutes before our drives are ready to go. ", {WAV_SELECT0, NOTE_2E, COMM_COL_CWLTH}}, // MSG_L4_START3_FR
   {" We'll be ready in five. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L4_START4
   {" Good, good. ", {WAV_SELECT0, NOTE_2E, COMM_COL_CWLTH}}, // MSG_L4_START5_FR

   {" $BAlpha 1$C, we're picking up incoming jump signatures. Emanation profiles indicate FSF warships. ", {WAV_SELECT0, NOTE_2A, COMM_COL_CWLTH}}, // MSG_L4_START6_A2
   {" Damn. Sorry $BCaravan 3$C, it looks like we're in for a fight today. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L4_START7
   {" I was afraid you were going to say that. ", {WAV_SELECT0, NOTE_2E, COMM_COL_CWLTH}}, // MSG_L4_START8_FR
   {" $BAngry Moth$C, get ready to scramble. Protecting those freighters is our number one priority. ", {WAV_SELECT1, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L4_START9
*/
  {TEAM_FRIEND, CONVOY_0, "SCRIPT_L4C0_START",
   STYPE_SETUP_CONVOY, -8000, -2500, SPEED_F_LIGHT, 0, {0},
    {{PCON_SETUP_CONVOY, {9000, 2500, 500}, RESULT_NEXT_SCRIPT, {SCRIPT_L4C0_MOVE1}}, // setup scripts can't have messages
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_FRIEND, CONVOY_0, "SCRIPT_L4C0_MOVE1",
   STYPE_APPROACH_CONVOY, CONVOY_1, APPROACH_NO_LEAD, SPEED_F_LIGHT, 0, {0},
   {{PCON_CONVOY_DIST, {CONVOY_1, 1700}, RESULT_NEXT_SCRIPT, {SCRIPT_L4C0_MOVE2}, MSG_NONE},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_FRIEND, CONVOY_0, "SCRIPT_L4C0_MOVE2",
   STYPE_MOVE_SPECIAL, 0, 0, SPEED_F_HEAVY, 0, {0},
// note change in throttle
   {{PCON_NONE},
    },
    {{CCON_NONE},
    }
   },

  {TEAM_FRIEND, CONVOY_1, "SCRIPT_L4C1_START",
   STYPE_SETUP_CONVOY, 9000, 2500, SPEED_F_HEAVY, 0, {0},
    {{PCON_SETUP_CONVOY, {-8000, -2500, 500}, RESULT_NEXT_SCRIPT, {SCRIPT_L4C1_MOVE1}}, // setup scripts can't have messages
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_FRIEND, CONVOY_1, "SCRIPT_L4C1_MOVE1",
   STYPE_APPROACH_CONVOY, CONVOY_0, APPROACH_NO_LEAD, SPEED_F_HEAVY, 0, {0},
   {{PCON_CONVOY_DIST, {CONVOY_0, 2000}, RESULT_NEXT_SCRIPT, {SCRIPT_L4C1_MOVE2}, MSG_NONE},
    {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_FRIEND, CONVOY_1, "SCRIPT_L4C1_MOVE2",
   STYPE_MOVE_SPECIAL, 0, 0, SPEED_F_HEAVY, 0, {0},
   {{PCON_NONE},
    },
    {{CCON_NONE},
    }
   },



  {TEAM_FRIEND, CONVOY_NONE, "SCRIPT_L4_OCSF_JUMP_ETC",
   STYPE_START_WAIT, 0, 0, 0, 0, {0},
   {{PCON_LEVEL_TIME, {390}, RESULT_START_JUMP_COUNTDOWN, {30}, MSG_L4_JUMP, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {PCON_LEVEL_TIME, {420}, RESULT_CONVOY_JUMP_OUT, {TEAM_FRIEND, CONVOY_0}, MSG_NONE},
    {PCON_LEVEL_TIME, {420}, RESULT_CONVOY_JUMP_OUT, {TEAM_FRIEND, CONVOY_1}, MSG_NONE},
    },
   {{CCON_FWSHIPS_DESTROYED, {}, RESULT_MISSION_FAIL, {0}},
     {CCON_NONE},
   }
   },


  {TEAM_NONE, CONVOY_NONE, "SCRIPT_L4_START",
   STYPE_START_WAIT, 0, 0, 0, 0, {0},
   {{PCON_TIME, {3}, RESULT_NONE, {0}, MSG_L4_START1_FR, MSG_FROM_COMMAND_2, 1, MSG_TO_ALL},
    {PCON_TIME, {5}, RESULT_NONE, {0}, MSG_L4_START2, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {PCON_TIME, {7}, RESULT_NONE, {0}, MSG_L4_START3_FR, MSG_FROM_COMMAND_2, 1, MSG_TO_ALL},
    {PCON_TIME, {10}, RESULT_NONE, {0}, MSG_L4_START4, MSG_FROM_COMMAND_1, 2, MSG_TO_ALL},
    {PCON_TIME, {13}, RESULT_NEXT_SCRIPT, {SCRIPT_L4_START2}, MSG_L4_START5_FR, MSG_FROM_COMMAND_2, 1, MSG_TO_ALL},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_NONE, CONVOY_NONE, "SCRIPT_L4_START2",
   STYPE_WAIT, 0, 0, 0, 0, {0},
   {{PCON_TIME, {3}, RESULT_NONE, {0}, MSG_L4_START6_A2, MSG_FROM_COMMAND_1, 2, MSG_TO_ALL},
    {PCON_TIME, {6}, RESULT_NONE, {0}, MSG_L4_START7, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {PCON_TIME, {9}, RESULT_NONE, {0}, MSG_L4_START8_FR, MSG_FROM_COMMAND_2, 1, MSG_TO_ALL},
    {PCON_TIME, {12}, RESULT_END_SCRIPT, {0}, MSG_L4_START9, MSG_FROM_COMMAND_1, 1, MSG_TO_AM},
    {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },


// The FSF convoys jump in at a random location.
  {TEAM_ENEMY, CONVOY_NONE, "SCRIPT_L4_FSF_WAIT",
   STYPE_START_WAIT, 0, 0, 0, 0, {0},
    {{PCON_LEVEL_TIME, {23}, RESULT_START_SCRIPT, {SCRIPT_L4C2_JUMP}},
     {PCON_LEVEL_TIME, {23}, RESULT_START_SCRIPT, {SCRIPT_L4C3_JUMP}},
     {PCON_LEVEL_TIME, {23}, RESULT_NEXT_SCRIPT, {SCRIPT_L4C4_JUMP}},
     {PCON_NONE},
    },
   {{CCON_NONE},
   }
   },

// Cv 2 attacks Alpha group
  {FLAG_JUMP_1, CONVOY_2, "SCRIPT_L4C2_JUMP",
   STYPE_CONVOY_JUMP_SPECIAL, 0, 0, SPEED_E_LIGHT, 0, {0},
    {{PCON_SETUP_CONVOY, {0, 0, 0}, RESULT_NONE},
     {PCON_ALWAYS_PASS, {0}, RESULT_NEXT_SCRIPT, {SCRIPT_L4C2_MOVE1}},
     {PCON_NONE},
    },
   {{CCON_NONE},
   }
   },
  {TEAM_ENEMY, CONVOY_2, "SCRIPT_L4C2_MOVE1",
   STYPE_APPROACH_CONVOY_OFFSET, CONVOY_0, APPROACH_NO_LEAD, SPEED_E_LIGHT, 0, {900},
   {{PCON_CONVOY_DIST, {CONVOY_0, 1400}, RESULT_NEXT_SCRIPT, {SCRIPT_L4C2_SLOW}, MSG_L4_C2_ATTACKS, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {PCON_LEVEL_TIME, {120}, RESULT_CHANGE_ARRANGE, {1}, MSG_NONE}
    },
    {{CCON_CONVOY_DESTROYED, {CONVOY_2}, RESULT_END_SCRIPT, {0}, MSG_L4_C2_DESTROYED, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_2, "SCRIPT_L4C2_SLOW", // no longer slow
//   STYPE_APPROACH_CONVOY_OFFSET, CONVOY_0, APPROACH_NO_LEAD, SPEED_E_HEAVY, 0, {800},
   STYPE_MOVE_FORWARDS, 0, 0, SPEED_E_LIGHT, 1, {0},
   {{PCON_CONVOY_DIST_APART, {CONVOY_0, 2050}, RESULT_NEXT_SCRIPT, {SCRIPT_L4C2_FAST}, MSG_NONE},
    },
    {{CCON_CONVOY_DESTROYED, {CONVOY_2}, RESULT_END_SCRIPT, {0}, MSG_L4_C2_DESTROYED, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_2, "SCRIPT_L4C2_FAST", // no longer fast!
   STYPE_APPROACH_CONVOY_OFFSET, CONVOY_0, APPROACH_NO_LEAD, SPEED_E_HEAVY, 1, {-800},
   {{PCON_CONVOY_DIST, {CONVOY_0, 2400}, RESULT_NEXT_SCRIPT, {SCRIPT_L4C2_SLOW}, MSG_NONE},
    },
    {{CCON_CONVOY_DESTROYED, {CONVOY_2}, RESULT_END_SCRIPT, {0}, MSG_L4_C2_DESTROYED, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {CCON_NONE},
    }
   },

// Cv 3 attacks Caravan 3
  {FLAG_JUMP_2, CONVOY_3, "SCRIPT_L4C3_JUMP",
   STYPE_CONVOY_JUMP_SPECIAL, 0, 0, SPEED_E_LIGHT, 0, {0},
    {{PCON_SETUP_CONVOY, {0, 0, 0}, RESULT_NONE},
     {PCON_ALWAYS_PASS, {0}, RESULT_NEXT_SCRIPT, {SCRIPT_L4C3_MOVE1}},
     {PCON_NONE},
    },
   {{CCON_NONE},
   }
   },
  {TEAM_ENEMY, CONVOY_3, "SCRIPT_L4C3_MOVE1",
   STYPE_APPROACH_CONVOY_OFFSET, CONVOY_1, APPROACH_NO_LEAD, SPEED_E_LIGHT, 0, {-900},
   {{PCON_CONVOY_DIST, {CONVOY_0, 1800}, RESULT_NEXT_SCRIPT, {SCRIPT_L4C3_SLOW}, MSG_L4_C3_ATTACKS, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {PCON_LEVEL_TIME, {125}, RESULT_CHANGE_ARRANGE, {1}, MSG_NONE}
    },
    {{CCON_CONVOY_DESTROYED, {CONVOY_3}, RESULT_END_SCRIPT, {0}, MSG_L4_C3_DESTROYED, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_3, "SCRIPT_L4C3_SLOW", // no longer slow
//   STYPE_APPROACH_CONVOY_OFFSET, CONVOY_1, APPROACH_NO_LEAD, SPEED_SLOW, 0, {-800},
   STYPE_MOVE_FORWARDS, 0, 0, SPEED_E_LIGHT, 1, {0},
   {{PCON_CONVOY_DIST_APART, {CONVOY_1, 2050}, RESULT_NEXT_SCRIPT, {SCRIPT_L4C3_FAST}, MSG_NONE},
    },
    {{CCON_CONVOY_DESTROYED, {CONVOY_3}, RESULT_END_SCRIPT, {0}, MSG_L4_C3_DESTROYED, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_3, "SCRIPT_L4C3_FAST", // no longer fast!
   STYPE_APPROACH_CONVOY_OFFSET, CONVOY_1, APPROACH_NO_LEAD, SPEED_E_HEAVY, 1, {800},
   {{PCON_CONVOY_DIST, {CONVOY_1, 2400}, RESULT_NEXT_SCRIPT, {SCRIPT_L4C3_SLOW}, MSG_NONE},
    },
    {{CCON_CONVOY_DESTROYED, {CONVOY_3}, RESULT_END_SCRIPT, {0}, MSG_L4_C3_DESTROYED, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {CCON_NONE},
    }
   },


// Cv 4 is the carrier group; it just moves slowly along, launching occasionally
  {FLAG_JUMP_3, CONVOY_4, "SCRIPT_L4C4_JUMP",
   STYPE_CONVOY_JUMP_SPECIAL, 0, 0, SPEED_E_HEAVY, 0, {0},
    {{PCON_SETUP_CONVOY, {0, 0, 0}, RESULT_NONE},
     {PCON_ALWAYS_PASS, {0}, RESULT_NEXT_SCRIPT, {SCRIPT_L4C4_MOVE1}},
     {PCON_NONE},
    },
   {{CCON_ECARRIERS_DISABLED, {}, RESULT_NEXT_SCRIPT, {SCRIPT_L4C4_DISABLED}, MSG_L4_CARRIERS_DISABLED, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
       {CCON_NONE},
   }
   },
  {TEAM_ENEMY, CONVOY_4, "SCRIPT_L4C4_MOVE1",
   STYPE_HOLD, 0, 0, SPEED_E_HEAVY, 0, {0},
   {{PCON_TIME, {92}, RESULT_LAUNCH, {SHIP_BOMBER}, MSG_L4_NONE},
    {PCON_TIME, {93}, RESULT_LAUNCH, {SHIP_FIGHTER}, MSG_L4_NONE},
    {PCON_TIME, {94}, RESULT_LAUNCH, {SHIP_BOMBER}, MSG_L4_NONE},
    {PCON_TIME, {95}, RESULT_LAUNCH, {SHIP_FIGHTER}, MSG_L4_LAUNCH1, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {PCON_TIME, {96}, RESULT_NEXT_SCRIPT, {SCRIPT_L4C4_LAUNCH2}, MSG_L4_NONE},
    },
   {{CCON_ECARRIERS_DISABLED, {}, RESULT_NEXT_SCRIPT, {SCRIPT_L4C4_DISABLED}, MSG_L4_CARRIERS_DISABLED, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
       {CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_4, "SCRIPT_L4C4_LAUNCH2",
   STYPE_HOLD, 0, 0, SPEED_E_HEAVY, 0, {0},
   {{PCON_TIME, {82}, RESULT_LAUNCH, {SHIP_BOMBER}, MSG_L4_NONE},
    {PCON_TIME, {83}, RESULT_LAUNCH, {SHIP_FIGHTER}, MSG_L4_NONE},
    {PCON_TIME, {84}, RESULT_LAUNCH, {SHIP_BOMBER}, MSG_L4_NONE},
    {PCON_TIME, {85}, RESULT_LAUNCH, {SHIP_FIGHTER}, MSG_L4_LAUNCH2, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {PCON_TIME, {86}, RESULT_NEXT_SCRIPT, {SCRIPT_L4C4_LAUNCH3}, MSG_L4_NONE},
    },
   {{CCON_ECARRIERS_DISABLED, {}, RESULT_NEXT_SCRIPT, {SCRIPT_L4C4_DISABLED}, MSG_L4_CARRIERS_DISABLED, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
       {CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_4, "SCRIPT_L4C4_LAUNCH3",
   STYPE_HOLD, 0, 0, SPEED_E_HEAVY, 0, {0},
   {{PCON_TIME, {82}, RESULT_LAUNCH, {SHIP_BOMBER}, MSG_L4_NONE},
    {PCON_TIME, {83}, RESULT_LAUNCH, {SHIP_FIGHTER}, MSG_L4_NONE},
    {PCON_TIME, {84}, RESULT_LAUNCH, {SHIP_BOMBER}, MSG_L4_NONE},
    {PCON_TIME, {85}, RESULT_LAUNCH, {SHIP_FIGHTER}, MSG_L4_LAUNCH3, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {PCON_TIME, {86}, RESULT_END_SCRIPT, {0}, MSG_L4_NONE},
    },
   {{CCON_ECARRIERS_DISABLED, {}, RESULT_NEXT_SCRIPT, {SCRIPT_L4C4_DISABLED}, MSG_L4_CARRIERS_DISABLED, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
       {CCON_NONE},
    }
   },
// this DISABLED script probably isn't needed, but I'm sleepy and at least it doesn't break anything
  {TEAM_ENEMY, CONVOY_4, "SCRIPT_L4C4_DISABLED",
   STYPE_HOLD, 0, 0, SPEED_E_HEAVY, 0, {0},
    {{PCON_NONE},
    },
   {{CCON_NONE},
    }
   },


  {TEAM_NONE, CONVOY_NONE, "SCRIPT_L4_LOSS",
   STYPE_START_WAIT, 0, 0, 0, 0, {0},
    {{PCON_NONE},
    },
   {{CCON_CONVOY_REDUCED, {CONVOY_0, 5}, RESULT_NONE, {0}, MSG_L4_C0_LOSS1, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {CCON_CONVOY_REDUCED, {CONVOY_0, 3}, RESULT_NONE, {0}, MSG_L4_C0_LOSS2, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {CCON_CONVOY_DESTROYED, {CONVOY_0}, RESULT_END_SCRIPT, {0}, MSG_L4_C0_LOSS3_FR, MSG_FROM_COMMAND_2, 1, MSG_TO_ALL},
    }
   },
  {TEAM_NONE, CONVOY_NONE, "SCRIPT_L4_LOSS2",
   STYPE_START_WAIT, 0, 0, 0, 0, {0},
    {{PCON_NONE},
    },
   {{CCON_SHIPTYPE_DESTROYED, {SHIP_DROM, 1}, RESULT_NONE, {0}, MSG_L4_C1_LOSS1, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {CCON_SHIPTYPE_DESTROYED, {SHIP_DROM, 3}, RESULT_NONE, {0}, MSG_L4_C1_LOSS2, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {CCON_SHIPTYPE_DESTROYED, {SHIP_DROM, 6}, RESULT_END_SCRIPT, {0}, MSG_L4_C1_LOSS3, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    }
   },


  } // end stage 4 scripts
  }, // end stage 4






 {
// Stage 5 - defection
  {
   {""},
   {" A single ship just jumped in. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L5_START1
   {" Scans indicate an unarmed passenger vessel. $BAngry Moth$C, fly over and check it out while we open a channel. ", {WAV_SELECT0, NOTE_2G, COMM_COL_TO_AM}}, // MSG_L5_START2
   {" ... you read me? Outer Colonies starships, do you read me? ", {WAV_SELECT0, NOTE_3C, COMM_COL_FSF}}, // MSG_L5_START3
   {" We read you. What's the story? ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L5_START4
   {" Excellent! You got my message. ", {WAV_SELECT0, NOTE_3C, COMM_COL_FSF}}, // MSG_L5_START5
   {" My name is Xavier Olaf Mrrhrn, former Second Assistant Secretary of the Federation Department of Defence. I seek asylum in the Outer Colonies, in exchange for everything I know about the Federation's fleet and its battle plans. ", {WAV_SELECT0, NOTE_3C, COMM_COL_FSF}}, // MSG_L5_START6
   {" Interesting. I don't have the authority to accept your offer, but I'm happy to take you in. We'll stay in this system until your ship is ready to jump; transmitting coordinates now. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L5_START7
   {" I appreciate that. Standing by. ", {WAV_SELECT0, NOTE_3C, COMM_COL_FSF}}, // MSG_L5_START8
   {" $BGazer 1$C, incoming jump signatures. ", {WAV_SELECT0, NOTE_2A, COMM_COL_CWLTH}}, // MSG_L5_START9
   {" What is it now? ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L5_START10

   {" Picking up a number of large warships, a type we haven't seen before. Several escorts. ", {WAV_SELECT0, NOTE_2A, COMM_COL_CWLTH}}, // MSG_L5_ARRIVE1
   {" Ach, they followed me! Captain, can you send assistance? This staryacht is fast, but it can't outfly their fighters. ", {WAV_SELECT0, NOTE_3C, COMM_COL_FSF}}, // MSG_L5_ARRIVE2
   {" Cover that ship, $BAngry Moth$C! ", {WAV_SELECT0, NOTE_2G, COMM_COL_TO_AM}}, // MSG_L5_ARRIVE3

   {" Stay at a safe distance, OCSF idiots. This stolen toy is coming back with us, in several pieces. ", {WAV_SELECT0, NOTE_2C, COMM_COL_FSF}}, // MSG_L5_ARRIVE4
   {" Watch out, a squadron of FSF fighters heading your way. Unknown class. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L5_ARRIVE5

   {" More warships jumping in. ", {WAV_SELECT0, NOTE_2A, COMM_COL_CWLTH}}, // MSG_L5_AGAIN1

   {" Hull breached in all sectors! I'm breaking up! ", {WAV_SELECT0, NOTE_3C, COMM_COL_FSF}}, // MSG_L5_LOSS1
   {" Mrrhrn's ship is down. ", {WAV_SELECT0, NOTE_2A, COMM_COL_CWLTH}}, // MSG_L5_LOSS2
   {" Too bad. We'd better get out of here. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L5_LOSS3

   {" You're safe with us now. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L5_SAFE1
   {" Thank you, captain. Your fighters are unstoppable. ", {WAV_SELECT0, NOTE_3C, COMM_COL_FSF}}, // MSG_L5_SAFE2

  },

  {
// Alpha Group:
   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 1, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{140, 0}, {0,0}}},
   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{0, 100}, {0,0}}},
   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{-140,0}, {0,0}}},
   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 2, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{0, -100}, {0,0}}},
   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{-100, -180}, {0,0}}},
   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{-100, 180}, {0,0}}},

   {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 1, 1, 0,0,
    CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
   {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 1, 2, 0,0,
    CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
   {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 2, 1, 0,0,
    CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
   {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 2, 2, 0,0,
    CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},

// Convoy 1 is the defector's ship:
  {TEAM_FRIEND, SHIP_LINER, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAG_COMMAND_2|FLAG_JUMP_1, FLAGS2,
    {{-190,0}, {0, 0}}},

// Convoy 2 is the first set of wships that jumps in:
  {TEAM_ENEMY, SHIP_SCOUT3, VAR_NONE, 0, ACT_NONE, 3, 0, 0,0,
   CONVOY_2, FLAG_E_COMMAND_1|FLAG_JUMP_2, FLAGS2,
    {{0,100}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT3, VAR_NONE, 0, ACT_NONE, 4, 0, 0,0,
   CONVOY_2, FLAG_E_COMMAND_1|FLAG_JUMP_2, FLAGS2,
    {{0,100}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 5, 0, 0,0,
   CONVOY_2, FLAG_E_COMMAND_1|FLAG_JUMP_2, FLAGS2,
    {{-150,100}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 6, 0, 0,0,
   CONVOY_2, FLAG_E_COMMAND_1|FLAG_JUMP_2, FLAGS2,
    {{-150,-100}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 7, 0, 0,0,
   CONVOY_2, FLAG_E_COMMAND_1|FLAG_JUMP_2, FLAGS2,
    {{150, 0}, {0, 0}}},


  {TEAM_ENEMY, SHIP_EINT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 3, 1, 0,0,
   CONVOY_NONE, FLAG_JUMP_2, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_EINT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 3, 2, 0,0,
   CONVOY_NONE, FLAG_JUMP_2, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_EINT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 3, 3, 0,0,
   CONVOY_NONE, FLAG_JUMP_2, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_EINT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 3, 4, 0,0,
   CONVOY_NONE, FLAG_JUMP_2, FLAGS2, {{0,0}}},

  {TEAM_ENEMY, SHIP_EINT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 4, 1, 0,0,
   CONVOY_NONE, FLAG_JUMP_2, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_EINT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 4, 2, 0,0,
   CONVOY_NONE, FLAG_JUMP_2, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_EINT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 4, 3, 0,0,
   CONVOY_NONE, FLAG_JUMP_2, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_EINT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 4, 4, 0,0,
   CONVOY_NONE, FLAG_JUMP_2, FLAGS2, {{0,0}}},

  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 5, 1, 0,0,
   CONVOY_NONE, FLAG_JUMP_2, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 5, 2, 0,0,
   CONVOY_NONE, FLAG_JUMP_2, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 6, 1, 0,0,
   CONVOY_NONE, FLAG_JUMP_2, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 6, 2, 0,0,
   CONVOY_NONE, FLAG_JUMP_2, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 7, 1, 0,0,
   CONVOY_NONE, FLAG_JUMP_2, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 7, 2, 0,0,
   CONVOY_NONE, FLAG_JUMP_2, FLAGS2, {{0,0}}},

  {TEAM_NONE} // this is necessary or srecord and other things break

  },

// },


  {
  {
// SCRIPT_L4_NONE
  },

  {TEAM_NONE, CONVOY_NONE, "SCRIPT_L5_START",
   STYPE_START_WAIT, 0, 0, 0, 0, {0},
   {{PCON_TIME, {6}, RESULT_NONE, {0}, MSG_L5_START1, MSG_FROM_COMMAND_1, 2, MSG_TO_ALL},
    {PCON_TIME, {8}, RESULT_NONE, {0}, MSG_L5_START2, MSG_FROM_COMMAND_1, 1, MSG_TO_AM},
    {PCON_TIME, {10}, RESULT_NONE, {0}, MSG_L5_START3, MSG_FROM_COMMAND_2, 1, MSG_TO_ALL},
    {PCON_TIME, {13}, RESULT_NONE, {0}, MSG_L5_START4, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {PCON_TIME, {16}, RESULT_NEXT_SCRIPT, {SCRIPT_L5_START2}, MSG_L5_START5, MSG_FROM_COMMAND_2, 1, MSG_TO_ALL},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_NONE, CONVOY_NONE, "SCRIPT_L5_START2",
   STYPE_WAIT, 0, 0, 0, 0, {0},
   {{PCON_TIME, {3}, RESULT_NONE, {0}, MSG_L5_START6, MSG_FROM_COMMAND_2, 1, MSG_TO_ALL},
    {PCON_TIME, {6}, RESULT_NONE, {0}, MSG_L5_START7, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {PCON_TIME, {9}, RESULT_NONE, {0}, MSG_L5_START8, MSG_FROM_COMMAND_2, 1, MSG_TO_ALL},
    {PCON_TIME, {12}, RESULT_NONE, {0}, MSG_L5_START9, MSG_FROM_COMMAND_1, 2, MSG_TO_ALL},
    {PCON_TIME, {15}, RESULT_NEXT_SCRIPT, {SCRIPT_L5_START3}, MSG_L5_START10, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_NONE, CONVOY_NONE, "SCRIPT_L5_START3",
   STYPE_WAIT, 0, 0, 0, 0, {0},
   {{PCON_TIME, {3}, RESULT_NONE, {0}, MSG_L5_ARRIVE1, MSG_FROM_COMMAND_1, 2, MSG_TO_ALL},
    {PCON_TIME, {6}, RESULT_NONE, {0}, MSG_L5_ARRIVE2, MSG_FROM_COMMAND_2, 1, MSG_TO_ALL},
    {PCON_TIME, {9}, RESULT_NONE, {0}, MSG_L5_ARRIVE3, MSG_FROM_COMMAND_1, 1, MSG_TO_AM},
    {PCON_TIME, {12}, RESULT_NONE, {0}, MSG_L5_ARRIVE4, MSG_FROM_E_COMMAND_1, 1, MSG_TO_ALL}, // FSF commander
    {PCON_TIME, {15}, RESULT_SEND_FIGHTERS, {3, CONVOY_1, TEAM_ENEMY}, MSG_L5_ARRIVE5, MSG_FROM_COMMAND_1, 1, MSG_TO_AM},
    },
    {{CCON_NONE},
    }
   },

  {TEAM_NONE, CONVOY_NONE, "SCRIPT_L5_MISSION",
   STYPE_START_WAIT, 0, 0, 0, 0, {0},
   {{PCON_NONE},
    },
    {{CCON_SHIPTYPE_DESTROYED, {SHIP_LINER, 1}, RESULT_START_SCRIPT, {SCRIPT_L5_OCSF_JUMP}, MSG_L5_LOSS1, MSG_FROM_COMMAND_2, 1, MSG_TO_ALL},
     {CCON_CONVOY_DESTROYED, {CONVOY_0}, RESULT_MISSION_FAIL, {0}}, // very unlikely
    }
   },
  {TEAM_NONE, CONVOY_NONE, "SCRIPT_L5_OCSF_JUMP",
   STYPE_WAIT, 0, 0, 0, 0, {0},
   {{PCON_TIME, {3}, RESULT_NONE, {0}, MSG_L5_LOSS2, MSG_FROM_COMMAND_1, 2, MSG_TO_ALL},
    {PCON_TIME, {6}, RESULT_NONE, {0}, MSG_L5_LOSS3, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {PCON_TIME, {9}, RESULT_START_JUMP_COUNTDOWN, {45}, MSG_NONE},
    {PCON_TIME, {54}, RESULT_CONVOY_JUMP_OUT, {CONVOY_0}, MSG_NONE},
    },
    {{CCON_NONE},
    }
   },

  {TEAM_NONE, CONVOY_NONE, "SCRIPT_L5_SAFE",
   STYPE_WAIT, 0, 0, 0, 0, {0},
   {{PCON_TIME, {3}, RESULT_NONE, {0}, MSG_L5_SAFE1, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
    {PCON_TIME, {6}, RESULT_NONE, {0}, MSG_L5_SAFE2, MSG_FROM_COMMAND_2, 1, MSG_TO_ALL},
    {PCON_TIME, {9}, RESULT_MISSION_OVER, {0}, MSG_NONE},
    },
    {{CCON_NONE},
    }
   },

/*

   {" More warships jumping in. ", {WAV_SELECT0, NOTE_2A, COMM_COL_CWLTH}}, // MSG_L5_AGAIN1

   {" Hull breached in all sectors! I'm breaking up! ", {WAV_SELECT0, NOTE_3C, COMM_COL_FSF}}, // MSG_L5_LOSS1
   {" Mrrhrn's ship is down. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L5_LOSS2
   {" Too bad. We'd better get out of here. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L5_LOSS3

   {" You're safe with us now. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L5_SAFE1
   {" Thank you, captain. Your fighters are unstoppable. ", {WAV_SELECT0, NOTE_3C, COMM_COL_FSF}}, // MSG_L5_SAFE2
*/

  {TEAM_FRIEND, CONVOY_0, "SCRIPT_L5C0_START",
   STYPE_SETUP_CONVOY, -8000, 2500, SPEED_F_LIGHT, 0, {0},
    {{PCON_SETUP_CONVOY, {9000, 2500, 500}, RESULT_NEXT_SCRIPT, {SCRIPT_L5C0_MOVE1}}, // setup scripts can't have messages
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_FRIEND, CONVOY_0, "SCRIPT_L5C0_MOVE1",
   STYPE_HOLD, 0, 0, SPEED_F_LIGHT, 0, {0},
   {{PCON_LEVEL_TIME, {59}, RESULT_NEXT_SCRIPT, {SCRIPT_L5C0_MOVE2}, MSG_NONE},
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
// evasive action:
  {TEAM_FRIEND, CONVOY_0, "SCRIPT_L5C0_MOVE2",
   STYPE_MOVE_XY, -2000, 19000, SPEED_F_LIGHT, 0, {0},
   {{PCON_NONE},
    },
    {{CCON_NONE},
    }
   },

  {TEAM_NONE, CONVOY_NONE, "SCRIPT_L5_JUMPS",
   STYPE_START_WAIT, 0, 0, 0, 0, {0},
   {{PCON_TIME, {4}, RESULT_START_SCRIPT, {SCRIPT_L5C1_JUMP}, MSG_NONE},
    {PCON_TIME, {30}, RESULT_START_SCRIPT, {SCRIPT_L5C2_JUMP}, MSG_NONE},
    {PCON_TIME, {31}, RESULT_END_SCRIPT, {0}, MSG_NONE},
    },
    {{CCON_NONE},
    }
   },


// Cv 1 is the defector's starliner:
  {FLAG_JUMP_1, CONVOY_1, "SCRIPT_L5C1_JUMP",
   STYPE_CONVOY_JUMP, 2000, -4000, 20, 0, {0},
    {{PCON_SETUP_CONVOY, {0, 0, 0}, RESULT_NONE},
     {PCON_ALWAYS_PASS, {0}, RESULT_NEXT_SCRIPT, {SCRIPT_L5C1_MOVE1}},
     {PCON_NONE},
    },
   {{CCON_NONE},
   }
   },
  {TEAM_FRIEND, CONVOY_1, "SCRIPT_L5C1_MOVE1",
   STYPE_APPROACH_CONVOY, CONVOY_0, APPROACH_LEAD, 20, 0, {0},
   {{PCON_CONVOY_DIST, {CONVOY_0, 300}, RESULT_JOIN_CONVOY, {CONVOY_0}}, // this ends the script
    },
    {{CCON_NONE},
    }
   },

// Cv 2 is the enemy convoy that jumps in:
  {FLAG_JUMP_2, CONVOY_2, "SCRIPT_L5C2_JUMP",
   STYPE_CONVOY_JUMP, -2000, -18000, SPEED_E_HEAVY, 0, {0},
    {{PCON_SETUP_CONVOY, {0, 0, 0}, RESULT_NONE},
     {PCON_ALWAYS_PASS, {0}, RESULT_NEXT_SCRIPT, {SCRIPT_L5C2_MOVE1}},
     {PCON_NONE},
    },
   {{CCON_NONE},
   }
   },
  {TEAM_ENEMY, CONVOY_2, "SCRIPT_L5C2_MOVE1",
   STYPE_APPROACH_CONVOY, CONVOY_0, APPROACH_LEAD, SPEED_E_HEAVY, 0, {0},
   {{PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  } // end stage 5 scripts
  }, // end stage 5



 {
// Stage 6 - large-scale battle
  {
   {""},
   {" A single ship just jumped in. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L6_START1
   {" Scans indicate an unarmed passenger vessel. $BAngry Moth$C, fly over and check it out while we open a channel. ", {WAV_SELECT0, NOTE_2G, COMM_COL_TO_AM}}, // MSG_L5_START2
   {" ... you read me? Outer Colonies starships, do you read me? ", {WAV_SELECT0, NOTE_3C, COMM_COL_FSF}}, // MSG_L5_START3
   {" We read you. What's the story? ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L5_START4
   {" Excellent! You got my message. ", {WAV_SELECT0, NOTE_3C, COMM_COL_FSF}}, // MSG_L5_START5
   {" My name is Xavier Olaf Mrrhrn, former Second Assistant Secretary of the Federation Department of Defence. I seek asylum in the Outer Colonies, in exchange for everything I know about the Federation's fleet and its battle plans. ", {WAV_SELECT0, NOTE_3C, COMM_COL_FSF}}, // MSG_L5_START6
   {" Interesting. I don't have the authority to accept your offer, but I'm happy to take you in. We'll stay in this system until your ship is ready to jump; transmitting coordinates now. ", {WAV_SELECT0, NOTE_2G, COMM_COL_CWLTH}}, // MSG_L5_START7





  },

  {
   {TEAM_FRIEND, SHIP_OLD3, VAR_NONE, 0, ACT_NONE, 1, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{0, 0}, {0,0}}},
   {TEAM_FRIEND, SHIP_OLD3, VAR_NONE, 0, ACT_NONE, 2, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{450, 0}, {0,0}}},
   {TEAM_FRIEND, SHIP_OLD3, VAR_NONE, 0, ACT_NONE, 3, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{-450, 0}, {0,0}}},

   {TEAM_FRIEND, SHIP_DROM, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{200, 250}, {0,0}}},
   {TEAM_FRIEND, SHIP_DROM, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{200, -250}, {0,0}}},
   {TEAM_FRIEND, SHIP_DROM, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{-200, 250}, {0,0}}},
   {TEAM_FRIEND, SHIP_DROM, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{-200, -250}, {0,0}}},

   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{540, 180}, {0,0}}},
   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{540, -180}, {0,0}}},
   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{-540, 180}, {0,0}}},
   {TEAM_FRIEND, SHIP_OLD2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
    CONVOY_0, FLAG_COMMAND_1|FLAG_ALPHA|FLAG_SPAWN_1, FLAGS2,
    {{-540, -180}, {0,0}}},



   {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 1, 1, 0,0,
    CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
   {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 1, 2, 0,0,
    CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
   {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 2, 1, 0,0,
    CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
   {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 2, 2, 0,0,
    CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
   {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 3, 1, 0,0,
    CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
   {TEAM_FRIEND, SHIP_FIGHTER_FRIEND, VAR_NONE, MISSION_GUARD, ACT_AWAY, 3, 2, 0,0,
    CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},





// Convoy 3 is warships which turn to fight Alpha group.

  {TEAM_ENEMY, SHIP_SCOUT3, VAR_NONE, 0, ACT_NONE, 4, 0, 0,0,
   CONVOY_1, FLAGS, FLAGS2,
    {{0,0}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT3, VAR_NONE, 0, ACT_NONE, 5, 0, 0,0,
   CONVOY_1, FLAGS, FLAGS2,
    {{420,0}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT3, VAR_NONE, 0, ACT_NONE, 6, 0, 0,0,
   CONVOY_1, FLAGS, FLAGS2,
    {{-420,0}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT3, VAR_NONE, 0, ACT_NONE, 7, 0, 0,0,
   CONVOY_1, FLAGS, FLAGS2,
    {{-840,0}, {0, 0}}},

  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAGS, FLAGS2,
    {{450,-200}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAGS, FLAGS2,
    {{450,200}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAGS, FLAGS2,
    {{-450,-200}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_ANTI, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAGS, FLAGS2,
    {{-450,200}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAGS, FLAGS2,
    {{-700,200}, {0, 0}}},
  {TEAM_ENEMY, SHIP_SCOUT2, VAR_NONE, 0, ACT_NONE, 0, 0, 0,0,
   CONVOY_1, FLAGS, FLAGS2,
    {{-700,-200}, {0, 0}}},


  {TEAM_ENEMY, SHIP_EINT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 4, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_EINT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 4, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_EINT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 5, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_EINT, VAR_NONE, MISSION_GUARD, ACT_AWAY, 5, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 6, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 6, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 7, 1, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},
  {TEAM_ENEMY, SHIP_FIGHTER, VAR_NONE, MISSION_GUARD, ACT_AWAY, 7, 2, 0,0,
   CONVOY_NONE, FLAGS, FLAGS2, {{0,0}}},

  {TEAM_NONE} // this is necessary or srecord and other things break

  },

// },


  {
  {

  },

  {TEAM_FRIEND, CONVOY_0, "SCRIPT_L6C0_START",
   STYPE_SETUP_CONVOY, -2000, 0, SPEED_F_HEAVY, 0, {0},
    {{PCON_SETUP_CONVOY, {19500, 0, 500}, RESULT_NEXT_SCRIPT, {SCRIPT_L6C0_MOVE1}}, // setup scripts can't have messages
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_FRIEND, CONVOY_0, "SCRIPT_L6C0_MOVE1",
   STYPE_MOVE_XY, 19500, 0, SPEED_F_HEAVY, 0, {0},
   {
     {PCON_NONE},
    },
    {{CCON_CONVOY_DESTROYED, {CONVOY_0}, RESULT_MISSION_FAIL, {0}},
    }
   },


  {TEAM_ENEMY, CONVOY_1, "SCRIPT_L6C1_START",
   STYPE_SETUP_CONVOY, 2000, 700, SPEED_E_HEAVY, 0, {0},
    {{PCON_SETUP_CONVOY, {-19500, 1000, 500}, RESULT_NEXT_SCRIPT, {SCRIPT_L6C1_MOVE1}}, // setup scripts can't have messages
     {PCON_NONE},
    },
    {{CCON_NONE},
    }
   },
  {TEAM_ENEMY, CONVOY_1, "SCRIPT_L6C1_MOVE1",
   STYPE_MOVE_XY, -19500, 700, SPEED_E_HEAVY, 0, {0},
   {
     {PCON_NONE},
    },
    {{CCON_CONVOY_DESTROYED, {CONVOY_1}, RESULT_MISSION_OVER, {0}},
    }
   },

   },



 },




/*

  {TEAM_NONE, CONVOY_NONE, "SCRIPT_L3_MESSAGES",
   STYPE_START_WAIT, 0, 0, 0, 0, {0},
   {{PCON_LEVEL_TIME, {390}, RESULT_NONE, {0}, MSG_L3_ETOJUMP, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
//    {PCON_LEVEL_TIME, {401}, RESULT_NEXT_SCRIPT, {SCRIPT_L3_CLEANUP}, MSG_L3_EJUMP, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {PCON_LEVEL_TIME, {401}, RESULT_END_SCRIPT, {0}, MSG_L3_EJUMP, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {PCON_NONE},
    },
    {{CCON_NONE}
    }
   },

  {TEAM_NONE, CONVOY_NONE, "SCRIPT_L3_CLEANUP",
   STYPE_START_WAIT, 0, 0, 0, 0, {0},
   {{PCON_NO_ENEMIES_LEFT, {}, RESULT_NEXT_SCRIPT, {SCRIPT_L3_FINISH}, MSG_NONE},
// PCON_NO_ENEMIES_LEFT could be triggered either by killing all enemies or by them all jumping out.
     {PCON_LEVEL_TIME, {404}, RESULT_NONE, {0}, MSG_L3_CLEANUP, MSG_FROM_COMMAND_1, 1, MSG_TO_AM},
// If we get to 404 without PCON_NO_ENEMIES_LEFT being triggered, there must be FSF fighters left who didn't get picked up.
// Get rid of them before we go to FINISH
     {PCON_NONE},
    },
    {{CCON_EWSHIPS_DESTROYED_FREMAIN, {}, RESULT_NEXT_SCRIPT, {SCRIPT_L3_JUST_FIGHTERS}, MSG_L3_CLEANUP, MSG_FROM_COMMAND_1, 1, MSG_TO_AM},
     {CCON_NONE}
    }
   },

  {TEAM_NONE, CONVOY_NONE, "SCRIPT_L3_JUST_FIGHTERS",
   STYPE_WAIT, 0, 0, 0, 0, {0},
   {{PCON_NO_ENEMIES_LEFT, {}, RESULT_NEXT_SCRIPT, {SCRIPT_L3_FINISH}, MSG_NONE},
     {PCON_NONE},
    },
    {{CCON_NONE}
    }
   },


  {TEAM_NONE, CONVOY_NONE, "SCRIPT_L3_FINISH",
   STYPE_WAIT, 0, 0, 0, 0, {0},
   {{PCON_TIME, {1}, RESULT_MISSION_OVER, {0}, MSG_L3_OVER, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {PCON_NONE},
    },
    {{CCON_NONE}
    }
   },

  {TEAM_NONE, CONVOY_NONE, "SCRIPT_L3_LOSS",
   STYPE_START_WAIT, 0, 0, 0, 0, {0},
   {{PCON_NONE},
    },
    {{CCON_CONVOY_REDUCED, {CONVOY_0, 5}, RESULT_NONE, {0}, MSG_L3_LOSS1, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {CCON_CONVOY_REDUCED, {CONVOY_0, 3}, RESULT_NONE, {0}, MSG_L3_LOSS2, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {CCON_CONVOY_REDUCED, {CONVOY_0, 2}, RESULT_NONE, {0}, MSG_L3_LOSS3, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {CCON_NONE}
    }
   },

  {TEAM_NONE, CONVOY_NONE, "SCRIPT_L3_DESTROY",
   STYPE_START_WAIT, 0, 0, 0, 0, {0},
   {{PCON_NONE},
    },
    {{CCON_CONVOY_REDUCED, {CONVOY_1, 6}, RESULT_NONE, {0}, MSG_L3_TARGET1, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {CCON_CONVOY_REDUCED, {CONVOY_1, 3}, RESULT_NONE, {0}, MSG_L3_TARGET2, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {CCON_CONVOY_DESTROYED, {CONVOY_1}, RESULT_END_SCRIPT, {0}, MSG_L3_TARGET3, MSG_FROM_COMMAND_1, 1, MSG_TO_ALL},
     {CCON_NONE}
    }
   },
*/




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
#define LD ol[arena.stage].level_data[s]

int pre_briefing_wship_sprite_check(void)
{
 int s;
 int processes = 0;

 for (s = 0; s < LEVEL_SHIPS; s ++)
 {
  if (LD.team == TEAM_NONE)
   break;
  processes += set_wss_status(LD.type);
 }

 return processes; // the number of times we'll need to call wship_process - used to calc progress bar length
}

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
 arena.jump_countdown = -1;
 arena.missed_jump = 0;
 arena.jumped_out = 0;

 int a, e, i, j;
 int scr = 0;

 for (i = 0; i < COMMS; i ++)
 {
//  comm [i].message = -1;
  comm [i].exists = 0;
  ctext[i][0].text[0] = END_MESSAGE;
 }

 for (i = 0; i < NO_EVENTS; i ++)
 {
     event [i] = 0;
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
  convoy[i].approach_lead = APPROACH_LEAD;
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
    running_script [scr] = ol[arena.stage].script_data[i].pcon[0].result [0]; // result_type must always be RESULT_NEXT_SCRIPT
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

#define FAST_SLEW 50
#define SLOW_SLEW 10
#define LOW_TURN 25

 int s;
 int stype;

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
  stype = LD.type;
  if (stype > NO_SHIP_TYPES) // special type
  {
   stype = get_special_stype(stype);
   if (stype == SHIP_NONE)
    continue;
  }
  e = create_ship(stype, LD.team, (LD.flags2 & FLAG2_NO_SRECORD) ^ 1);
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
 int stype;

//player[0].ships ++;
 for (s = 0; s < LEVEL_SHIPS; s ++)
 {
  if (LD.team == TEAM_NONE)
   break;
  if (!(LD.flags & flag))
   continue;
  a = LD.team;
  stype = LD.type;
  if (stype > NO_SHIP_TYPES) // special type
  {
   stype = get_special_stype(stype);
   if (stype == SHIP_NONE)
    continue;
  }
  e = create_ship(stype, LD.team, (LD.flags2 & FLAG2_NO_SRECORD) ^ 1);
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


int get_special_stype(int stype)
{

   switch(stype)
   {
    case SHIP_ESCORT1: return player[0].escort_type [0];
    case SHIP_ESCORT2: return  player[0].escort_type [1];
   }

 return SHIP_NONE; // will not create anything for this LD entry
}
void ship_from_level_data(int s, int a, int e)
{

 int j;


  EE.convoy = LD.convoy;
  if (EE.convoy != CONVOY_NONE)
  {
   for (j = 0; j < CONVOY_ARRANGEMENTS; j ++)
   {
//    EE.convoy_angle [j] = LD.convoy_position [j] [0]; //LD.convoy_angle [j];
//    EE.convoy_dist [j] = LD.convoy_position [j] [1]<<10;//LD.convoy_dist [j] << 10;
    EE.convoy_angle [j] = radians_to_angle(atan2(LD.convoy_position [j] [1], LD.convoy_position [j] [0]));
    EE.convoy_dist [j] = ((int)hypot(LD.convoy_position [j] [1], LD.convoy_position [j] [0]))<<10;
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
  EE.letter = -1;
  EE.letter_rank = -1;

  if (LD.flags != FLAGS)
  {
   if (LD.flags & FLAG_COMMAND_1
    || LD.flags & FLAG_E_COMMAND_1)
    add_ship_to_command(a, e, 1); // must be 1
   if (LD.flags & FLAG_COMMAND_2
    || LD.flags & FLAG_E_COMMAND_2)
    add_ship_to_command(a, e, 2); // must be 2
   if (LD.flags & FLAG_COMMAND_3
    || LD.flags & FLAG_E_COMMAND_3)
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

  if (LD.flags2 != FLAGS2)
  {
   if (LD.flags2 & FLAG2_FIGHTER_GROUP_1)
    EE.fighter_group = 1;
// FLAG2_MODIFIED is checked below.
  }


  EE.old_x = EE.x;
  EE.old_y = EE.y;


  setup_new_ship(a, e, LD.subtype);

   if (LD.flags2 & FLAG2_MODIFIED)
    modify_ship(a, e);
}

void modify_ship(int a, int e)
{

 switch(arena.stage)
 {
  case 1: // this is the damaged trireme that jumps in:
   EE.turret_type [1] = TURRET_NONE;
   EE.hp [0] *= 0.6;
   EE.hp [1] = 0;
   EE.hp [2] *= 0.3;
   EE.structure *= 0.9;
   EE.shield_generator = 0;
   EE.shield = 0;
   EE.shield_up = 0;
   break;
 }
}


void setup_player_wing(int p, int w, int type, int number)
{

 int e;
 int a = TEAM_FRIEND;

 e = create_ship(type, TEAM_FRIEND, 1);
 if (e == -1)
  return; // shouldn't happen

 EE.player_leader = p;
 EE.player_wing = w;
 EE.leader = -1;
/* if (p == 0)
  EE.leader = TARGET_P1;
   else
    EE.leader = TARGET_P2;*/
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


 EE.formation_position = number+1;
 PP.wing [w] [number] = e;
 PP.wing_size [w] ++;

 EE.x_speed = PP.x_speed;
 EE.y_speed = PP.y_speed;

 EE.old_x = EE.x;
 EE.old_y = EE.y;

 setup_new_ship(TEAM_FRIEND, e, VAR_NONE);
// EE.target_range = 550000;
 EE.target_range = COVER_RANGE;
 EE.base_target_range = 450000;

}


// This function is called at the start of each stage. It sets variables that are to be used by
//  special_convoy.
void setup_special_stage_conditions(void)
{

 switch(arena.stage)
 {
   case 4:
    switch(grand(4))
    {
     case 0:
      scon [S4SCON_X] = 2000;
      scon [S4SCON_Y] = -5000;
      break;
     case 1:
      scon [S4SCON_X] = -3000;
      scon [S4SCON_Y] = 5000;
      break;
     case 2:
      scon [S4SCON_X] = -1500;
      scon [S4SCON_Y] = 4000;
      break;
     case 3:
      scon [S4SCON_X] = 300;
      scon [S4SCON_Y] = -6000;
      break;
    }
/*     case 0:
      scon [S4SCON_X] = 2000 + grand(3000);
      scon [S4SCON_Y] = -3000 - grand(3000);
      break;
     case 1:
      scon [S4SCON_X] = -2000 - grand(3000);
      scon [S4SCON_Y] = 3000 + grand(3000);
      break;
    }*/

    break;
// stage 1 has no scons
/*  case 2:
   switch(grand(3))
   {

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
   break;*/


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
//   convoy[cv].y = - 3000000 + grand(6000000);
   break;
  case 4:
// Stage 4: Convoys 2, 3 and 4 jump in at a single random location.
   switch(cv)
   {
    case CONVOY_2:
     convoy[cv].x = (scon [S4SCON_X] - 500)<<10;
     convoy[cv].y = scon [S4SCON_Y]<<10;
     convoy[cv].angle = radians_to_angle(atan2(scon [S4SCON_X], scon [S4SCON_Y]));
     convoy[cv].fine_angle = CV.angle << FINE_ANGLE_BITSHIFT;
     break;
    case CONVOY_3:
     convoy[cv].x = (scon [S4SCON_X] + 500)<<10;
     convoy[cv].y = scon [S4SCON_Y]<<10;
     convoy[cv].angle = radians_to_angle(atan2(scon [S4SCON_X], scon [S4SCON_Y]));
     convoy[cv].fine_angle = CV.angle << FINE_ANGLE_BITSHIFT;
     break;
    case CONVOY_4:
     convoy[cv].x = (scon [S4SCON_X])<<10;
     convoy[cv].y = scon [S4SCON_Y]<<10;
     if (convoy[cv].y < 0)
      convoy[cv].angle = ANGLE_1-ANGLE_4;
       else
        convoy[cv].angle = ANGLE_4;
     convoy[cv].fine_angle = CV.angle << FINE_ANGLE_BITSHIFT;
     break;
   }
   break;
   /*
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
   break;*/
 }




}

void special_convoy_move(int cv)
{

 switch(arena.stage)
 {
  case 4:
// When they meet, both OCSF groups head away from the direction the FSF ships came from (and the carriers may still be there)
     convoy[cv].target_x = convoy[cv].x;
     if (scon [S4SCON_Y] > 0)
      convoy[cv].target_y = -9900000;
       else
        convoy[cv].target_y = 9900000;
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

  EE.test_destroyed = 0;
  EE.wship_throttle = 4;
  EE.engine_power = eclass[EE.type].engine_output;
//  EE.turn_speed = 0;//eclass[EE.type].turn;
//  EE.drag = eclass[EE.type].drag;

  EE.max_shield = eclass[EE.type].max_shield;
  if (eclass[EE.type].ship_class == ECLASS_WSHIP)
   EE.shield_energy_cost = eclass[EE.type].shield_recharge;
    else
     EE.shield_recharge = eclass[EE.type].shield_recharge;

  EE.shield = EE.max_shield;
  EE.shield_threshhold = EE.max_shield / 3;

  int tur;

  switch(EE.type)
  {
   case SHIP_OLD3:
    set_turret(a, e, 0, TURRET_HEAVY, 0);
    set_turret(a, e, 1, TURRET_HEAVY, 0);
    set_turret(a, e, 2, TURRET_BASIC, ANGLE_2);
    break;
   case SHIP_FRIEND3:
    set_turret(a, e, 0, TURRET_CGUN, 0);
    set_turret(a, e, 1, TURRET_CLAUNCHER, 0);
    set_turret(a, e, 2, TURRET_CGUN, ANGLE_2);
    break;
   case SHIP_OLD2:
    tur = TURRET_HEAVY;
    if (subtype == VAR_ANTI)
     tur = TURRET_BASIC;
    set_turret(a, e, 0, tur, 0);
    set_turret(a, e, 1, TURRET_BASIC, ANGLE_2);
    break;
   case SHIP_DROM:
    break;
   case SHIP_LINER:
    break;
   case SHIP_SCOUT2:
    tur = TURRET_EHEAVY;
    if (subtype == VAR_LONG)
     tur = TURRET_ELONG;
    if (subtype == VAR_ANTI)
     tur = TURRET_EANTI;
    set_turret(a, e, 0, tur, 0);
    set_turret(a, e, 1, TURRET_EBASIC, ANGLE_2);
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
    break;
   case SHIP_SCOUTCAR:
    set_turret(a, e, 0, TURRET_EBASIC, ANGLE_2);
//    EE.can_launch = 1;
    break;
   case SHIP_ECARRIER:
    set_turret(a, e, 0, TURRET_EANTI, ANGLE_2);
    EE.can_launch = 1;
    break;
   case SHIP_FREIGHT:
    set_turret(a, e, 0, TURRET_EBASIC, ANGLE_2);
    break;
   case SHIP_FIGHTER:
    EE.attack = ATTACK_PREF_FIGHTER;
    EE.target_range = 350000;
    EE.base_target_range = 350000;
    EE.away_dist = 200000;
    EE.attack_range = 300000;
    EE.shield_up = 1;
    break;
   case SHIP_ESCOUT:
    EE.attack = ATTACK_PREF_FIGHTER;
    EE.target_range = 550000;
    EE.base_target_range = 550000;
    EE.away_dist = 100000;
    EE.attack_range = 400000;
    EE.shield_up = 1;
    break;
   case SHIP_EINT:
    EE.attack = ATTACK_PREF_FIGHTER;
    EE.target_range = 750000;
    EE.base_target_range = 750000;
    EE.away_dist = 100000;
    EE.attack_range = 400000;
    EE.shield_up = 1;
    break;
   case SHIP_BOMBER:
    EE.attack = ATTACK_ONLY_WSHIP;
    EE.target_range = 99000000;
    EE.base_target_range = 99000000;
    EE.away_dist = 500000;
    EE.attack_range = 300000;
    EE.shield_up = 1;
    break;
   case SHIP_FIGHTER_FRIEND:
   case SHIP_LACEWING:
   case SHIP_MONARCH:
    EE.attack = ATTACK_PREF_FIGHTER;
    EE.target_range = 550000;
    EE.base_target_range = 450000; // this is also set in setup_player_wing
    EE.away_dist = 100000;
    EE.attack_range = 400000;
    EE.shield_up = 1;
    break;
   case SHIP_FSTRIKE:
   case SHIP_AUROCHS:
   case SHIP_IBEX:
    EE.attack = ATTACK_PREF_WSHIP;
    EE.target_range = 650000;
    EE.base_target_range = 450000; // this is also set in setup_player_wing
    EE.away_dist = 100000;
    EE.attack_range = 600000;
    EE.shield_up = 1;
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
    EE.turning = pos_or_neg(1);
    break;
  }





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
   EE.turret_bullet_speed [t] = 8000;
   break;
  case TURRET_HEAVY:
   EE.turret_turn [t] = SLOW_SLEW;
   EE.turret_recycle_time [t] = 30;
   EE.turret_energy_max [t] = 40000; // turret will absorb energy up to this point
   EE.turret_energy_use [t] = EE.turret_energy_max [t]; // how much a single shot costs
   EE.turret_class [t] = ATTACK_ONLY_WSHIP; // anti_wship
   EE.turret_target_range [t] = 1550000;//550000;
   EE.turret_firing_range [t] = 1530000;//530000;
   EE.turret_bullet_speed [t] = 7400;//5000;
   break;
  case TURRET_CGUN:
   EE.turret_turn [t] = SLOW_SLEW;
   EE.turret_recycle_time [t] = 30;
   EE.turret_energy_max [t] = 5000; // turret will absorb energy up to this point
   EE.turret_energy_use [t] = EE.turret_energy_max [t]; // how much a single shot costs
   EE.turret_class [t] = ATTACK_PREF_WSHIP; // anti_wship
   EE.turret_target_range [t] = 890000;
   EE.turret_firing_range [t] = 870000;
   EE.turret_bullet_speed [t] = 6300;
   break;
  case TURRET_CLAUNCHER:
   EE.turret_turn [t] = SLOW_SLEW - 5;
   EE.turret_recycle_time [t] = 30;
   EE.turret_energy_max [t] = 10000; // turret will absorb energy up to this point
   EE.turret_energy_use [t] = EE.turret_energy_max [t]; // how much a single shot costs
   EE.turret_class [t] = ATTACK_ONLY_WSHIP; // anti_wship
   EE.turret_target_range [t] = 1650000;
   EE.turret_firing_range [t] = 1630000;
   EE.turret_bullet_speed [t] = 4000; // actual speed is slower - but this is the speed used for targetting
   break;
  case TURRET_EBASIC:
   EE.turret_turn [t] = FAST_SLEW;
   EE.turret_recycle_time [t] = 20;
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
   EE.turret_energy_max [t] = 42000; // turret will absorb energy up to this point
   EE.turret_energy_use [t] = EE.turret_energy_max [t]; // how much a single shot costs
   EE.turret_class [t] = ATTACK_ONLY_WSHIP; // anti_wship
   EE.turret_target_range [t] = 1550000;//550000;
   EE.turret_firing_range [t] = 1500000;//500000;
   EE.turret_bullet_speed [t] = 6500;//3000;
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
   EE.turret_recycle_time [t] = 14;
   EE.turret_energy_max [t] = 10000; // turret will absorb energy up to this point then start firing
   EE.turret_energy_use [t] = 1800; // how much a single shot costs
   EE.turret_class [t] = ATTACK_PREF_FIGHTER;
   EE.turret_target_range [t] = 650000;
   EE.turret_firing_range [t] = 600000;
   EE.turret_burst [t] = BURST_CHARGING;
   EE.turret_status [t] = 6;
   EE.turret_bullet_speed [t] = 9000;
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
     case PCON_EVENT:
      if (event [SD->pcon[j].val[0]] == SD->pcon[j].val[1])
       pcon_met(i, j);
      break;
     case PCON_NO_ENEMIES_LEFT:
      if (check_for_enemies() == 0)
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
    running_script [scr] = SD->pcon[pcon].result [0];
    start_script(scr);
    break;
   case RESULT_START_SCRIPT:
    start_new_script(ol[arena.stage].script_data[running_script [scr]].pcon[pcon].result [0]);
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
     SD->pcon[pcon].result [0]))
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
      ship[SD->team][s].convoy = SD->pcon[pcon].result [0];
     }
    }
    about_to_end_script(scr);
    running_script [scr] = -1;
// note that SD pointer is still valid even after the script has been stopped. This
    //  means that we can use it below to deal with messages.
    break;
   case RESULT_START_JUMP_COUNTDOWN:
    if (arena.all_wships_lost != 0)
     break;
    arena.jump_countdown = SD->pcon[pcon].result [0] * 50;
    break;
   case RESULT_CONVOY_JUMP_OUT:
    if (convoy_jump_out(SD->pcon[pcon].result [0], SD->pcon[pcon].result [1]) == 0)
     check_message = 0;
//    running_script [scr] = -1;
    break;
   case RESULT_CHANGE_ARRANGE:
    convoy[SD->convoy].arrangement = SD->pcon[pcon].result [0];
    break;
   case RESULT_SEND_FIGHTERS:
    check_message = 0;
    if (send_fighters(SD->pcon[pcon].result [0], SD->pcon[pcon].result [1], SD->pcon[pcon].result [2]) == 1)
         check_message = 1;
    break;
   case RESULT_EVENT:
    event [SD->pcon[pcon].result [0]] = SD->pcon[pcon].result [1];
    break;
   case RESULT_MISSION_OVER:
    if (arena.game_over > 0)
     break;
    if (arena.all_wships_lost > 0)
     break;
    if (arena.missed_jump > 0)
     break;
    arena.mission_over = 200;
    break;
   case RESULT_NONE:
    break;
   case RESULT_NO_MORE_MESSAGES:
    break; // this is dealt with after the message for the condition (if any) is sent - see later in this function.
   default: exit(999);
  }

 if (check_message == 1 && arena.send_messages == 1)
 {
      if (SD->pcon[pcon].msg != MSG_NONE)
       new_message(SD->pcon[pcon].msg,
        SD->pcon[pcon].msg_from,
        SD->pcon[pcon].msg_from2,
        SD->pcon[pcon].msg_to);
 }

// NOTE: any changes in pcon_met may need to be replicated in ccon_met! It's not good
//  but I can't be bothered fixing it

 if (SD->pcon[pcon].result_type == RESULT_NO_MORE_MESSAGES)
  arena.send_messages = 0;

}

// returns 1 if any enemies still around
int check_for_enemies(void)
{
 int a = TEAM_ENEMY;
 int e;

 for (e = 0; e < NO_SHIPS; e ++)
 {
  if (EE.type != SHIP_NONE)
   return 1;
 }

 return 0;

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
   case STYPE_MOVE_SPECIAL:
    special_convoy_move(cv);
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
    set_approach_target(SD->convoy, SD->x, 0, 0, SD->y);
    CV.throttle = SD->throttle;
    CV.arrangement = SD->arrange;
    CV.approach_convoy = SD->x;
    CV.can_turn = 1;
    CV.approach_lead = SD->y;
    set_convoy_turning(cv);
    break;
   case STYPE_APPROACH_CONVOY_OFFSET:
    CV.target_offset_x = SD->val[0]<<10;
    CV.target_offset_y = SD->val[1]<<10;
    set_approach_target(SD->convoy, SD->x, CV.target_offset_x, CV.target_offset_y, SD->y);
    CV.throttle = SD->throttle;
    CV.arrangement = SD->arrange;
    CV.approach_convoy = SD->x;
    CV.can_turn = 1;
    CV.approach_lead = SD->y;
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


// sends fighters in group to attack a ship in convoy cv
int send_fighters(int group, int cv, int a)
{

// first we find a target:
 int e2;
 int a2 = a^1;
 int target = -1;

 for (e2 = 0; e2 < NO_SHIPS; e2++)
 {
  if (ship[a2][e2].convoy == cv)
  {
   target = e2;
   break;
  }
 }

 if (target == -1)
  return 0; // no target to attack

 int e;
 int leader = -1;
 int position = 1;

// next we find a leader:
 for (e = 0; e < NO_SHIPS; e ++)
 {
  if (EE.fighter_group == group)
  {
    leader = e;
    EE.formation_size = 0;
    EE.formation_position = 0;
    EE.action = ACT_TRANSIT;
    EE.mission = MISSION_ATTACK_WSHIP;
    EE.target = target;
    break;
  }
 }

 if (leader == -1)
  return 0; // no fighters from group survive

// now we assign every other member of the group:
 for (e = 0; e < NO_SHIPS; e ++)
 {
  if (EE.fighter_group == group
   && e != leader)
   {
     EE.leader = leader;
     EE.formation_position = position;
     position ++;
     ship[a][leader].formation_size ++;
     EE.mission = MISSION_ESCORT;
     EE.action = ACT_AWAY;
   }
 }

 return 1;

}


// This is called before destroy_ship, so the destroyed ship still technically exists - but is disregarded for the e2 loop below
void condition_eship_destroyed(int e)
{
// need to have some kind of flag system for particular ships being destroyed
 int a = TEAM_ENEMY;
 int e2;

 char other_ships_exist = 0;
 char convoy_still_exists = 0;
 char other_wships_exist = 0;
 char other_fighters_exist = 0;
 int convoy_size = 0;
 int ship_class = eclass[EE.type].ship_class;

 for (e2 = 0; e2 < NO_SHIPS; e2++)
 {
  if (e2 == e)
   continue;
  if (ship[a][e2].type == SHIP_NONE)
   continue;
  other_ships_exist = 1;
  if (ship[a][e2].convoy == EE.convoy)
  {
   convoy_size ++;
   convoy_still_exists = 1;
  }
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
      case CCON_CONVOY_REDUCED: // also in condition_fship_destroyed
       if (EE.convoy != CONVOY_NONE
        && EE.convoy == ol[arena.stage].script_data[running_script [i]].ccon[j].val [0]
        && convoy_size == ol[arena.stage].script_data[running_script [i]].ccon[j].val [1])
         ccon_met(i, j);
       break;
      case CCON_SHIPTYPE_DESTROYED: // also in condition_fship_destroyed
       if (EE.type == ol[arena.stage].script_data[running_script [i]].ccon[j].val [0]
        && arena.srecord [SREC_DESTROYED] [TEAM_ENEMY] [EE.type]
             == ol[arena.stage].script_data[running_script [i]].ccon[j].val [1])
         ccon_met(i, j);
// note that the ship whose destruction resulted in this function being called will already be recorded in srecord as destroyed
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
  if (ship[a][e2].type != SHIP_ECARRIER)
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
 int convoy_size = 0;

 for (e2 = 0; e2 < NO_SHIPS; e2++)
 {
  if (e2 == e)
   continue;
  if (ship[a][e2].type == SHIP_NONE)
   continue;
  other_ships_exist = 1;
  if (ship[a][e2].convoy == EE.convoy)
  {
    convoy_size++;
    convoy_still_exists = 1;
  }
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
      case CCON_CONVOY_REDUCED: // also in condition_eship_destroyed
       if (EE.convoy != CONVOY_NONE
        && EE.convoy == ol[arena.stage].script_data[running_script [i]].ccon[j].val [0]
        && convoy_size == ol[arena.stage].script_data[running_script [i]].ccon[j].val [1])
         ccon_met(i, j);
       break;
      case CCON_SHIPTYPE_DESTROYED: // also in condition_eship_destroyed
       if (EE.type == ol[arena.stage].script_data[running_script [i]].ccon[j].val [0]
        && arena.srecord [SREC_DESTROYED] [TEAM_FRIEND] [EE.type]
             == ol[arena.stage].script_data[running_script [i]].ccon[j].val [1])
         ccon_met(i, j);
// note that the ship whose destruction resulted in this function being called will already be recorded in srecord as destroyed
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
    running_script [scr] = ol[arena.stage].script_data[running_script [scr]].ccon[ccon].result [0];
    start_script(scr);
    break;
   case RESULT_START_SCRIPT:
    start_new_script(ol[arena.stage].script_data[running_script [scr]].ccon[ccon].result [0]);
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
     ol[arena.stage].script_data[running_script [scr]].ccon[ccon].result [0]))
     {
       check_message = 1;
//      if (ol[arena.stage].script_data[running_script [scr]].ccon[ccon].msg != MSG_NONE)
//       new_message(ol[arena.stage].script_data[running_script [scr]].ccon[ccon].msg);
// only display message if the carrier actually launched something.
     }
    break;
   case RESULT_START_JUMP_COUNTDOWN:
    if (arena.all_wships_lost != 0)
     break;
    arena.jump_countdown = SD->ccon[ccon].result [0];
    break;
   case RESULT_CONVOY_JUMP_OUT:
//    convoy_jump_out(ol[arena.stage].script_data[running_script [scr]].team, ol[arena.stage].script_data[running_script [scr]].convoy);
    if (convoy_jump_out(SD->ccon[ccon].result [0], SD->ccon[ccon].result [1]) == 0)
     check_message = 0;
//    running_script [scr] = -1;
    break;
   case RESULT_SEND_FIGHTERS:
    check_message = 0;
    if (send_fighters(SD->ccon[ccon].result [0], SD->ccon[ccon].result [1], SD->ccon[ccon].result [2]) == 1)
         check_message = 1;
    break;
   case RESULT_EVENT:
    event [SD->ccon[ccon].result [0]] = SD->ccon[ccon].result [1];
    break;
   case RESULT_MISSION_OVER:
//    if (ol[arena.stage].script_data[running_script [scr]].ccon[ccon].msg != MSG_NONE)
//     new_message(ol[arena.stage].script_data[running_script [scr]].ccon[ccon].msg);
    if (arena.game_over > 0)
     break;
    if (arena.all_wships_lost > 0)
     break;
    if (arena.missed_jump > 0)
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
   case RESULT_NONE:
    break;
   case RESULT_NO_MORE_MESSAGES:
    break; // this is dealt with after the message for the condition (if any) is sent - see later in this function.
   default: exit(999);
  }

 if (check_message == 1 && arena.send_messages == 1)
 {
    if (SD->ccon[ccon].msg != MSG_NONE)
     new_message(SD->ccon[ccon].msg,
      SD->ccon[ccon].msg_from,
      SD->ccon[ccon].msg_from2,
      SD->ccon[ccon].msg_to);
 }


// NOTE: any changes in ccon_met may need to be replicated in pcon_met! It's not good
//  but I can't be bothered fixing it

  if (ol[arena.stage].script_data[running_script [scr]].ccon[ccon].result_type == RESULT_NO_MORE_MESSAGES)
   arena.send_messages = 0;

}

int convoy_jump_out(int a, int cv)
{
 int e;
 int found = 0;

 for (e = 0; e < NO_SHIPS; e++)
 {
  if (EE.type != SHIP_NONE
   && EE.convoy == cv)
  {
   EE.jump = 20 + grand(10);
   found = 1;
  }

 }

 return found;
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
   case SHIP_SCOUTCAR: arena.wship_threat += 5; break;
   case SHIP_ECARRIER: arena.wship_threat += 8;
    if (EE.can_launch)
     arena.wship_threat += 50;
    break;
   case SHIP_FREIGHT: arena.wship_threat += 1; break;
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
void set_approach_target(int cv1, int cv2, int x_offset, int y_offset, char lead)
{

 if (convoy[cv1].throttle == 0)
  return; // will divide by zero otherwise

 if (lead == APPROACH_NO_LEAD)
 {
  convoy[cv1].target_x = convoy[cv2].x + x_offset;
  convoy[cv1].target_y = convoy[cv2].y + y_offset;
  return;
 }

 int tx, ty;
 int cv1_speed = get_convoy_speed(cv1);//convoy[cv1].throttle;//*5;
 int cv2_speed = get_convoy_speed(cv2);//convoy[cv2].throttle;//*5;

 int dist = hypot(convoy[cv1].y - convoy[cv2].y, convoy[cv1].x - convoy[cv2].x);
 int time = dist / cv1_speed;

 tx = convoy[cv2].x + xpart(convoy[cv2].angle, time * cv2_speed);// + x_offset;
 ty = convoy[cv2].y + ypart(convoy[cv2].angle, time * cv2_speed);// + y_offset;

// let's do a second approximation to be just a little more accurate:

 dist = hypot(convoy[cv1].y - ty, convoy[cv1].x - tx);
 time = dist / cv1_speed;

 convoy[cv1].target_x = convoy[cv2].x + xpart(convoy[cv2].angle, time * cv2_speed) + x_offset;// + x_offset;
 convoy[cv1].target_y = convoy[cv2].y + ypart(convoy[cv2].angle, time * cv2_speed) + y_offset;// + y_offset;

}


int activate_carrier_launch(int a, int cv, int type)
{
 int e;
 char first = 1;

 for (e = 0; e < NO_SHIPS; e ++)
 {
  if (EE.convoy == cv
   && EE.type == SHIP_ECARRIER
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
   CV.x_speed *= (1023-WSHIP_DRAG);
   CV.x_speed >>= 10;
   CV.y_speed *= (1023-WSHIP_DRAG);
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
    set_approach_target(cv, convoy[cv].approach_convoy, convoy[cv].target_offset_x, convoy[cv].target_offset_y, convoy[cv].approach_lead);
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
      CV.turn_count = (angle_diff<<FINE_ANGLE_BITSHIFT) / 8;//CV.turn_speed;
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

 if (m_from >= MSG_FROM_E_COMMAND_1)
 {
  a = TEAM_ENEMY;
  m_from -= MSG_FROM_E_COMMAND_1;
  m_from ++;
 }

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

// remember that m_from will have been adjusted for enemy ships...
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
   comm [i].ctext = new_message_text(m, i, comm_col [ol[arena.stage].lmsg[m].val [2]] [4]);
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
 comm [i].comm_col = ol[arena.stage].lmsg[m].val [2];
/* comm [i].col_min = COL_BOX1;
 comm [i].col_max = COL_BOX4;
 if (m_to == MSG_TO_AM)
  comm[i].col_min = COL_BOX2;
 if (a == TEAM_ENEMY)
 {
  comm [i].col_min = COL_EBOX1;
  comm [i].col_max = COL_EBOX4;
 }*/

 indicator(sound, tone, 250, -1);

}

int new_message_text(int m, int cm, int col)
{
 int i;

 for (i = 0; i < COMMS; i ++)
 {
  if (ctext[i][0].text [0] == END_MESSAGE)
  {
   comm[cm].lines = generate_message(ol[arena.stage].lmsg[m], 245, MESSAGE_LINE_SPACE, col, ctext[i]);
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
  comm [i].comm_col = comm [i+1].comm_col;
//  comm [i].col_max = comm [i+1].col_max;
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
       case SHIP_ECARRIER: strcpy(str, "ecarrier"); break;
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
            EE.turning_time = angle_diff / eclass[EE.type].turn;
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
      EE.angle -= eclass[EE.type].turn;
      EE.angle &= ANGLE_MASK;
    }

    if (EE.turning == 1)
    {
//      EE.engine [0] = 10;
      EE.angle += eclass[EE.type].turn;
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

void get_script_name(int r, char sname [30])
{
 if (running_script [r] == -1)
 {
  strcpy(sname, "NONE");
  return;
 }

 strcpy(sname, ol[arena.stage].script_data[running_script [r]].name);

}

