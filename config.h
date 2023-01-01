
#define ALLEGRO_STATICLINK


#define NO_KEYS CKEY_END
#define X_MIN 500
#define Y_MIN 500
#define X_MAX 79500
#define Y_MAX 59500

#define GRAIN 100

#define FINE_ANGLE_1 32768
#define FINE_ANGLE_MASK 32767
#define FINE_ANGLE_BITSHIFT 3
// bitshift is bitshifts to get from angle to fine_angle or back again

//#define ANGLEFULL_HEX 0x400
#define ANGLE_MASK 4095
#define ANGLE_1 4096
#define ANGLE_2 2048
#define ANGLE_3 1365
// 3 is not exact
#define ANGLE_4 1024
#define ANGLE_5 819
// not exact
#define ANGLE_6 683
#define ANGLE_7 585
// 6 is not exact
#define ANGLE_8 512
#define ANGLE_10 410
#define ANGLE_12 341
#define ANGLE_16 256
//#define ANGLE_8_3 384
//#define ANGLE_16_3 192
#define ANGLE_32 128
#define ANGLE_64 64
#define ANGLE_128 32
#define ANGLE_256 16

#ifndef PI
#define PI 3.141592
#endif

#define PI_2 (PI/2)
#define PI_4 (PI/4)
#define PI_8 (PI/8)
#define PI_16 (PI/16)
#define PI_32 (PI/32)

#define NO_SHIPS 100
#define NO_BULLETS 300
#define NO_CLOUDS 600
#define NO_PARTS 64
#define NO_WEAPONS 5

#define NO_STARS 500

#define D_SIZE 1000

#define ANGLE_FRONT 340
// this is the arc (to the left and the right) that counts as the front of the hs for the purposes of
//  working out whether to use a forward weapon or an all-around weapon.

#define PP player[p]
#define EE ship[a][e]
#define BL bullet[a][b]
#define CC cloud[c]

// number of images for wship
#define WSHIP_ROTATIONS 32

// wship_rotations - 1
#define WSHIP_ROTATION_MASK 31

// wship_rotations * 4 - 1
#define WSHIP_FULL_MASK 127

// bitshift required to get from ANGLE_4 to WSHIP_ROTATIONS
#define WSHIP_ROTATION_BITSHIFT 5



// number of images for fighter
#define FIGHTER_ROTATIONS 16

// fighter_rotations - 1
#define FIGHTER_ROTATION_MASK 15

// fighter_rotations * 4 - 1
#define FIGHTER_FULL_MASK 63

// bitshift required to get from ANGLE_4 to FIGHTER_ROTATIONS
#define FIGHTER_ROTATION_BITSHIFT 6




// number of images for turret
#define TURRET_ROTATIONS 64

// turret_rotations / 4 - 1
#define TURRET_ROTATION_MASK 15

// turret_rotations - 1
#define TURRET_FULL_MASK 63

// bitshift required to get from ANGLE_1 to TURRET_ROTATIONS
#define TURRET_ROTATION_BITSHIFT 6
/*
#define TURRET_ROTATIONS 128
#define TURRET_ROTATION_MASK 127
#define TURRET_ROTATION_BITSHIFT 3
*/

/*
// bitshift required to get from ANGLE_1 to WSHIP_ROTATIONS
#define WSHIP_ROTATION_BITSHIFT 7
*/


// number of images for player
#define PLAYER_ROTATIONS 16

// player_rotations - 1
#define PLAYER_ROTATION_MASK 15

// player_rotations * 4 - 1
#define PLAYER_FULL_MASK 63

// bitshift required to get from ANGLE_4 to PLAYER_ROTATIONS
#define PLAYER_ROTATION_BITSHIFT 6





// number of images for shield
#define SHIELD_ROTATIONS 32

// shield_rotations / 4 - 1
#define SHIELD_ROTATION_MASK 7

// shield_rotations - 1
#define SHIELD_FULL_MASK 31

// bitshift required to get from ANGLE_4 to SHIELD_ROTATIONS
#define SHIELD_ROTATION_BITSHIFT 7

#define SHIELD_FRAMES 5
#define PULSE 5


// number of images for missile
#define MISSILE_ROTATIONS 32

// missile_rotations / 4 - 1
#define MISSILE_ROTATION_MASK 7

// missile_rotations - 1
#define MISSILE_FULL_MASK 31

// bitshift required to get from ANGLE_1 to TURRET_ROTATIONS
#define MISSILE_ROTATION_BITSHIFT 7


#define NO_TEAMS 2

#define TEAM_NONE -1
#define TEAM_FRIEND 0
#define TEAM_ENEMY 1

#define MODS 3


enum {
    RP_OLD2_TURRET_1,
    RP_OLD2_TURRET_2,
    RP_OLD2_ENGINE_1,
    RP_OLD2_ENGINE_2,
    RP_OLD3_TURRET_1,
    RP_OLD3_TURRET_2,
    RP_OLD3_TURRET_3,
    RP_OLD3_ENGINE_1,
    RP_OLD3_ENGINE_2,
    RP_OLD3_ENGINE_3,
    RP_OLD3_ENGINE_4,
    RP_FRIEND3_TURRET_1,
    RP_FRIEND3_TURRET_2,
    RP_FRIEND3_TURRET_3,
    RP_FRIEND3_ENGINE_1,
    RP_FRIEND3_ENGINE_2,
    RP_SCOUT2_TURRET_1,
    RP_SCOUT2_TURRET_2,
    RP_SCOUT2_ENGINE_1,
    RP_SCOUT2_ENGINE_2,
    RP_SCOUT3_TURRET_1,
    RP_SCOUT3_TURRET_2,
    RP_SCOUT3_TURRET_3,
    RP_SCOUT3_ENGINE_1,
    RP_SCOUT3_ENGINE_2,
    RP_SCOUT3_ENGINE_3,
    RP_SCOUTCAR_TURRET_1,
    RP_SCOUTCAR_ENGINE_1,
    // must be in this order, i.e. all turrets for a ship then all engines
    RP_FREIGHT_TURRET_1,
    RP_FREIGHT_ENGINE_1,
    RP_FREIGHT_ENGINE_2,
    RP_FREIGHT_ENGINE_3,
    RP_DROM_ENGINE_1,
    RP_DROM_ENGINE_2,
    RP_LINER_ENGINE_1,
    RP_LINER_ENGINE_2,
    RP_EBASE_TURRET_1,
    RP_EBASE_TURRET_2,
    RP_EBASE_TURRET_3,
    RP_EBASE_TURRET_4,
    RP_EBASE_TURRET_5,
    RP_ECARRIER_TURRET_1,
    RP_ECARRIER_ENGINE_1,
    RP_ECARRIER_ENGINE_2,
    RP_ECARRIER_ENGINE_3,
    RP_ECARRIER_ENGINE_4,
    ROTATE_POS
};

enum {
    WSHIP_OLD2,
    WSHIP_OLD3,
    WSHIP_FRIEND3,
    WSHIP_SCOUT2,
    WSHIP_SCOUT3,
    WSHIP_SCOUTCAR,
    WSHIP_ECARRIER,
    WSHIP_FREIGHT,
    WSHIP_DROM,
    WSHIP_LINER,
    WSHIP_EBASE,
    WSHIP_TYPES
};

#define NO_TURRETS 6

enum {
    FIGHTER_BASIC,
    FIGHTER_BOMBER,
    FIGHTER_FRIEND,
    FIGHTER_ESCOUT,
    FIGHTER_EINT,
    FIGHTER_FSTRIKE,
    FIGHTER_IBEX,
    FIGHTER_AUROCHS,
    FIGHTER_LACEWING,
    FIGHTER_MONARCH,
    FIGHTER_TYPES
};

enum {
    TURRET_NONE,
    TURRET_BASIC,
    TURRET_HEAVY,
    TURRET_EBASIC,
    TURRET_EBEAM,
    TURRET_EHEAVY,
    TURRET_ELONG,
    TURRET_EANTI,
    TURRET_CGUN,
    TURRET_CLAUNCHER,
    TURRET_TYPES
    // when adding e-turrets remember to add a threat value to calculate_threat in level.c
};

enum {
    ATTACK_ONLY_WSHIP, // anti-wship turret
    ATTACK_ONLY_FIGHTER, // anti-fighter turret
    ATTACK_PREF_WSHIP, // prefers wships but will attack fighter
    ATTACK_PREF_FIGHTER, // vv
    ATTACK_ANY // attacks anything
};

enum {
    CKEY_LEFT,
    CKEY_RIGHT,
    CKEY_LEFT2,
    CKEY_RIGHT2,
    CKEY_UP,
    CKEY_DOWN,
    CKEY_FIRE1,
    CKEY_FIRE2,
    CKEY_FIRE3,
    CKEY_FIRE4,
    CKEY_COMMAND,
    CKEY_END
};

enum {
    JBUTTON_FIRE1,
    JBUTTON_FIRE2,
    JBUTTON_FIRE3,
    JBUTTON_FIRE4,
    JBUTTON_LEFT2,
    JBUTTON_RIGHT2,
    JBUTTON_COMMAND,
    JBUTTONS
    // any additions to this list need to be added manually
    //  to the get_config_int and set_config_int bits in main and menu.
};

enum {
    JAXIS_ACCEL,
    JAXIS_TURN,
    JAXIS_SLIDE,
    JAXES
};

enum {
    WEAPON_BASIC,
    WEAPON_MINE,
    WEAPON_ROCKET,
    WEAPON_WORM,
    WEAPON_MISSILE,
    WEAPON_SCATTER,
    WEAPONS
};


enum {
    CVAR_NONE, // standard version of all fighters
    CVAR_RAM_HEAVY,

    CVAR_LW_B,
    CVAR_LW_C,
    CVAR_MONARCH_B,
    CVAR_MONARCH_C,
    CVAR_IBEX_B,
    CVAR_IBEX_C,
    CVAR_AUROCHS_B,
    CVAR_AUROCHS_C,

    CVAR_END
};



enum {
    DIR_NONE,
    DIR_FORWARD,
    DIR_ALL
};

enum {
    WPN_AF_MISSILE,
    WPN_AWS_MISSILE,
    WPN_ROCKET,
    WPN_TORP,
    WPN_ROCKET2,
    WPN_DEFLECT,
    WPN_BLASTER,
    WPN_WROCKET,
    WPN_HROCKET,
    WPN_LW_MISSILE,
    WPN_ADV_LW_MISSILE,
    WPN_HVY_LW_MISSILE,
    WPN_RAIN,
    WPN_AUTOCANNON,

    WPN_E_AF_MISSILE,

    WPN_NONE,

    //WPN_SEEKERS,
    WPN_CANNON,
    WPN_HEATSEEKER,
    WPN_MINE,
    WPN_HOMING
};


#define SBARS 15
#define WPN_TARGETS 8

enum {
    CONTROL_KEY_A,
    CONTROL_KEY_B,
    CONTROL_JOY_A,
    CONTROL_JOY_B
};

enum {
    MOD_TORP,
    MOD_DRIVE,
    MOD_SPEED,
    NO_MODS
};

// needs to be 4 because the actual numbers are {0,1}{2,3} rather than {0,1}{0,1}
#define WING_SIZE 4
#define WINGS 2
#define HPULSE 8
#define WPNS 3

#define COVER_RANGE 200000

struct playerstruct {
    int x, y;
    int x_speed, y_speed;
    int target_distance;
    int angle;
    int turning;
    int turning_time;
    int turn_speed;
    int recycle;
    int over_recycle;
    int mflash [2];
    int weapon;
    int charge;
    int rocket_recycle;
    int rocket_burst;
    int rocket_angle;
    int range; // current distance to hs, in weapon range units
    int angle_to_hs;
    int priority_target; // are the enemies preferentially targetting this player?

    int control;

    int type;
    int variant;

    int alive;
    int ships;
    int starting_ships;
    int ships_lost;
    int respawning;
    int start_x, start_y;
    int drive [4]; // 0 = rear (more forwards), 1 = front, 2 = left, 3 = right
    int accelerating; // used to control squadron AI
    int flap [2];

    int weapon_type [WPNS];
    int weapon_charge [WPNS];
    int weapon_lock [WPNS];
    int weapon_target [WPNS] [WPN_TARGETS];
    int weapon_block [WPNS]; // prevents cannons or other weapon firing
    int weapon_sight_x [WPNS] [WPN_TARGETS];
    int weapon_sight_y [WPNS] [WPN_TARGETS];
    int weapon_sight_visible [WPNS] [WPN_TARGETS];
    int weapon_angle [WPNS];
    int weapon_status [WPNS];
    int weapon_status2 [WPNS];
    int weapon_recycle [WPNS];
    int weapon_firing [WPNS];

    int camera_x;
    int camera_y;
    int window_x;
    int window_y;
    int window_centre_x;
    int window_centre_y;
    // TO DO!!!: smaller clipping area (with different area for fighters and wships)

    int hp;
    int max_hp;
    int shield;
    int max_shield;
    int shield_recharge;
    int shield_up;
    int shield_flash;
    int shield_threshold;
    int shield_just_up;

    int target_a;
    int target_e;
    int target_new;
    int target_sight_x;
    int target_sight_y;
    int target_sight_visible;
    int target_key_hold;
    int target_auto;

    int debug;

    int spulse_angle [PULSE];
    int spulse_dist [PULSE];
    int spulse_time [PULSE];
    int spulse_time_delta [PULSE];
    int spulse_beam_e [PULSE]; // reserves an spulse for a particular attacker's beam. Doesn't need to be initialised as only relevant if being hit by beam
    int spulse_beam_t [PULSE]; // which turret of beam_e is firing the beam?

    int hitpulse_thickness [2] [HPULSE];
    int hitpulse_start [2] [HPULSE];
    int hitpulse_end [2] [HPULSE];
    char hitpulse_beam [2] [HPULSE];

    int wing_orders [WINGS];
    int wing_size [WINGS];
    int wing [WINGS] [WING_SIZE];
    int wing_type [WINGS]; // just used to set wing type during mission ship select screen then to make sure correct type is created. Values refer to escorts (0 or 1) rather than ship types
    int escort_type [2]; // two types of fighter in Cwlth fleet - only player 0's value is used for this
    int wings; // number of wings (1 or 2)
    int commanding;
    int wing_command; // which wing are you commanding?
    int just_commanded;
    char command_key; // this is the command key just pressed (if any). Stops you firing after issuing command
    int command_mode; //
    int wing_fire1; // if >0 and wing is in formation, this tells wing to fire best anti-fighter weapon
    int wing_fire2; // if >0 and wing is in formation, this tells wing to fire best anti-wship weapon

    int jump_safe; // is the player safe to jump out? (i.e. close enough to friendly wship?)

    int mod [NO_MODS];
};

enum {
    JUMP_SAFE,
    JUMP_RISK,
    JUMP_NOT_SAFE
};

enum {
    CMODE_TACTIC,
    CMODE_WING
};

#define MINE_BITS 15

struct bulletstruct {
    int x, y, x2, y2, x3, y3;
    int x_speed, y_speed;
    int type;
    int angle;
    int colour;
    int status;
    int status2;
    int status3;
    int status4;
    int status5;
    int turning;
    int timeout;
    int damage;
    int force;
    int size;
    int draw_size;
    int time;
    int owner; // who owns the bullet?
    int owner_t; // if a wship, which turret fired it?
    int target_e;
};

enum {
    BULLET_NONE,
    BULLET_SHOT,
    BULLET_BIGSHOT,
    BULLET_OLDSHOT,
    BULLET_ETORP1,
    BULLET_EBEAM1,
    BULLET_EBEAM2,
    BULLET_EBEAM3,
    BULLET_PTORP1,
    BULLET_AWS_MISSILE,
    BULLET_AF_MISSILE,
    BULLET_LW_MISSILE,
    BULLET_ADV_LW_MISSILE,
    BULLET_HVY_LW_MISSILE,
    BULLET_EAF_MISSILE, // enemy AF missile
    BULLET_HOMING,
    BULLET_EBIGSHOT,
    BULLET_ELONGSHOT,
    BULLET_ESHOT1,
    BULLET_ESHOT2,
    BULLET_ROCKET,
    BULLET_HROCKET,
    BULLET_ROCKET2,
    BULLET_PSEEKER,
    BULLET_BLAST,
    BULLET_FSHOT,
    BULLET_FROCK,
    BULLET_RAIN,

    BULLET_BASIC,
    BULLET_WORM,
    BULLET_MINE,
    BULLET_SCATTER,
    BULLET_MISSILE,
    BULLET_BOLT,
    BULLET_MINE_BLAST
};

enum {
    BEAM_HIT_NOTHING,
    BEAM_HIT_SHIELD,
    BEAM_HIT_HULL
};

enum {
    SREC_CREATED,
    SREC_DESTROYED,
    SREC_UNLAUNCHED, // number of fighters stored in destroyed ships of this type when they were destroyed
    SREC_P1_KILL,
    SREC_P2_KILL,
    SRECORDS
};


enum {
    SHIP_NONE,
    // This is the order they appear in in the battle report:
    SHIP_FRIEND3,
    SHIP_OLD3,
    SHIP_OLD2,
    SHIP_DROM,
    SHIP_AUROCHS,
    SHIP_IBEX,
    SHIP_FSTRIKE,
    SHIP_LACEWING,
    SHIP_MONARCH,
    SHIP_FIGHTER_FRIEND,
    SHIP_LINER,

    SHIP_EBASE,
    SHIP_ECARRIER,
    SHIP_SCOUTCAR,
    SHIP_FREIGHT,
    SHIP_SCOUT3,
    SHIP_SCOUT2,
    SHIP_BOMBER,
    SHIP_ESCOUT,
    SHIP_EINT,
    SHIP_FIGHTER,
    NO_SHIP_TYPES,

    SHIP_ESCORT1, // these are used in level.c to set up CTBR fighters as escort_type 1 or 2
    SHIP_ESCORT2
};

enum {
    FF_NONE,
    FF_SANDFLY,
    //FF_RAM,
    //FF_LACEWING,
    FF_MONARCH,
    FF_IBEX,
    FF_AUROCHS,
    NO_FF
};
// these must be in the same order as they are in fighter_ship in briefing.c

struct gamestruct {
    int fighter_available [NO_FF];
};

struct arenastruct {
    int max_x;
    int max_y;
    unsigned char counter; // assumes uc is 0-255
    int time; // seconds elapsed in mission
    unsigned char subtime; // 1/50 second
    int time_left;
    int game_over;
    int mission_over;
    int all_wships_lost; // also mission failed
    int send_messages; // if zero, no messages will be sent. Use e.g. if mission is over and no more messages needed (see RESULT_NO_MORE_MESSAGE in level.c)

    int end_stage;
    int stage;
    int jump_countdown; // this informs the player and triggers pickup - the actual jump is handled by the script
    int missed_jump;
    int jumped_out; // if the player (or either player in 2up) is near wship when it jumps out, and jump_countdown > -1, this is set to 1 and mission is over

    int players;
    int only_player; // is -1 if 2 players until 1 player dies with no lives left. Is 0 if 1 player.

    int threat;
    int wship_threat;
    int fighter_threat;

    int srecord [SRECORDS] [NO_TEAMS] [NO_SHIP_TYPES];

    char camera_fix;

    char debug_invulnerable;
};

struct starstruct {
    int x;
    int y;
    int old_x;
    int old_y;
    unsigned char bright;
    unsigned char bright2;
    unsigned char depth;
};

#define NO_CORE_CIRCLES 5

struct cloudstruct {
    int type;
    int x;
    int y;
    int x2, y2, x3, y3;
    int x_speed;
    int y_speed;
    int timeout;
    int time;
    int status;
    int status2;
    int colour;
    int drag;
    int angle;
    float fangle;
};

// these are used for setting time, but also for array dimensions:
#define SMALL_SHOCK_TIME 30
#define LARGE_SHOCK_TIME 50
#define HUGE_SHOCK_TIME 50

enum {
    CLOUD_NONE,
    CLOUD_BALL,
    CLOUD_DOUBLE_BALL,
    CLOUD_BALL_COL2,
    CLOUD_BALL_COL2_SLOW,
    CLOUD_BALL_COL3,
    CLOUD_BALL_COL3_SLOW,
    CLOUD_SMALL_SHOCK,
    CLOUD_LARGE_SHOCK,
    CLOUD_HUGE_SHOCK,
    CLOUD_FLARE,
    CLOUD_BIGFLARE,
    CLOUD_FLASH,
    CLOUD_MAX_DBALL,
    CLOUD_GROWBALL,
    CLOUD_FADEBALL,
    CLOUD_FADEFLARE,
    CLOUD_SYNCHBALL,
    CLOUD_SYNCHFLARE,
    CLOUD_MAX_DBALL_BRIGHT,
    CLOUD_BIGFADEFLARE,
    CLOUD_LINKLINE,
    CLOUD_EX_SHIELD,

    CLOUD_FIRE,
    CLOUD_FIRE2,
    CLOUD_FIRE3,

    CLOUD_SMALL,
    CLOUD_BANG,
    CLOUD_DRAG_BALL,
    CLOUD_SPARSE_FLARE,
    CLOUD_SPARSE_NARROW_FLARE,
    CLOUD_BURST,
    CLOUD_2BALL,
    CLOUD_SEEKER_TRAIL,
    CLOUD_SEEKER_TRAIL2,
    CLOUD_LINE_TRAIL,
    CLOUD_ROCKET_TRAIL,
    CLOUD_WORM_TRAIL,

    CLOUD_XBALL
};


struct optionstruct {
    int sound_init; // if 0, sound isn't initialised at all. Changed in config file only.
    int music; // if 0, music is off
    int sound_mode; // mono, stereo, reversed, off
    int run_vsync; // on or off
    int windowed;
    int sfx_volume; // sound fx volume; if 0 sound effects not played
    int ambience_volume; // if 0 ambience not played

    int highscore [3];

    int ckey [2] [CKEY_END];
    int joy_button [2] [JBUTTONS];
    int joy_stick [2] [JAXES];
    int joy_axis [2] [JAXES];
    int joystick_available [2];

    char joystick_dual;
    char fix_camera_angle;
};

#define SHOW_GRAPHS
// MAX_PARTS should probably be at least 1 more than the number of parts, as the extra one is
//  used for internal structure in some cases
#define MAX_PARTS 6
#define TARGET_P1 -1
#define TARGET_P2 -2
#define TARGET_NONE -3
#define MAX_ENGINES 5

enum {
    BURST_NONE, // turret etc has no burst fire - just fires whenever energy is full
    BURST_FIRING, // is firing a burst
    BURST_WAITING, // is waiting to start burst because no target in range
    BURST_CHARGING // is charging up
    // these need to be in this order or DISASTER!
};

#define CONVOY_ARRANGEMENTS 3

struct shipstruct {
    int type;
    int x;
    int y;
    int x_speed;
    int y_speed;
    int old_x, old_y; // probably only good for wships
    int recycle;
    int recycle2; // used for eg Lacewing's missiles (which have a separate recycle timer)
    int angle;
    int counter;

    int test_destroyed;

    int sprite;
    int sprite_count;

    int target_angle;
    int target_dist;
    int think_count; // for player squadron, this is just about turning
    int think_count2; // for player squadron, this deals with finding targets etc.

    int action;
    int turning;
    int turning_time;
    int turn_speed;
    int slide_dir; // -1, 0 or 1
    int slide_count;
    int engine [MAX_ENGINES];
    int engine_power;
    int wship_throttle;
    int burst;
    int target;
    int attack; // what kind of target will it attack?
    int base_target_range; // base value used to generate randomised target_range
    int target_range; // range it uses to find targets - will ignore targets outside this. randomised each time
    // fighter attacks so they can be a bit unpredictable
    int away_dist; // distance fighters will travel away from target after an attack run
    int attack_range; // start firing when within this range
    int frustration;
    int force_away; // fighter must stay in away action

    int hp [MAX_PARTS];
    int max_hp [MAX_PARTS];
    int structure; // structural integrity of multi-part wships - is reduced when a destroyed part is hit.
    int max_structure;
    int shield;
    int max_shield;
    int shield_recharge; // only used for fighters; wships do this through energy management
    int hit_pulse [MAX_PARTS];
    int hit_pulse_structure;
    int breakup;
    int breakup_turn;
    int jump;
    char shield_generator; // does it have a shield generator at all? only relevant for wships
    char just_hit;

    int carrier_launch; // set by scripts in level.c - a carrier's next launch
    int carrier_launch_time; // time until carrier's launch
    int can_launch; // if 1, ship's launching parts are still working

    int energy_production;
    int energy_steps_hp [4]; // how damaged the core has to be to reduce the ship's energy production for each step
    int energy_steps_production [4]; // how much energy is produced at each step
    int shield_energy_cost; // how much energy it takes to recharge 1 shield
    int shield_up; // turns off if shield goes to zero, and restarts when shield at 1/3 max
    int shield_threshhold; // 1/3 max

    int spulse_angle [PULSE];
    int spulse_dist [PULSE];
    int spulse_size [PULSE];
    int spulse_size2 [PULSE];
    int spulse_time [PULSE];
    int spulse_time_delta [PULSE];
    int spulse_beam_e [PULSE]; // reserves an spulse for a particular attacker's beam. Doesn't need to be initialised as only relevant if being hit by beam
    int spulse_beam_t [PULSE]; // which turret of beam_e is firing the beam?
    int shield_bar_pulse;

    int turret_type [NO_TURRETS];
    int turret_angle [NO_TURRETS];
    int turret_target_angle [NO_TURRETS];
    int turret_recycle [NO_TURRETS];
    int turret_burst [NO_TURRETS];
    int turret_x [NO_TURRETS];
    int turret_y [NO_TURRETS];
    int turret_turn [NO_TURRETS]; // turning speed
    int turret_slew [NO_TURRETS]; // which way it turned last
    int turret_target [NO_TURRETS];
    int turret_firing [NO_TURRETS];
    int turret_recoil [NO_TURRETS];
    int turret_class [NO_TURRETS];
    int turret_target_range [NO_TURRETS];
    int turret_firing_range [NO_TURRETS];
    int turret_rest_angle [NO_TURRETS];
    int turret_side [NO_TURRETS];

    int turret_recycle_time [NO_TURRETS];
    int turret_energy [NO_TURRETS];
    int turret_energy_max [NO_TURRETS]; // turret will absorb energy up to this point
    int turret_energy_use [NO_TURRETS]; // how much a single shot costs
    int turret_status [NO_TURRETS]; // used for turret unfolding etc
    int turret_status2 [NO_TURRETS]; // used for other misc things
    int turret_bullet_speed [NO_TURRETS]; // speed of bullet fired. May not be exact. Used for leading targets.

    int fighter_group; // special variable set in script according to certain flags
    int leader;
    int player_leader;
    int player_wing;
    int player_command;
    int formation_position;
    int available_formation_position; // for leaders of carrier-launched groups - this is the
    // next available formation_position for a new fighter to take on when launched.
    int mission;
    int formation_x;
    int formation_y;
    char leading_formation; // this is set each time a ship which has this ship as leader runs, and set to zero
    // each time this ship runs. It tells ship to reduce engine power by 1 to allow similar ships to catch up.

    int formation_size; // size of formation (for leaders). Altered by destruction. Doesn't include leader.
    // set at start and must be kept up to date.
    int formation_wait; // leader turns engines down when first entering formation to allow formation to form.
    // int chase; // how long fighter will leave formation to chase attacker

    int leader_awaiting_escort; // the first fighter/bomber launched has this set to a timeout.
    // while it's active, any new fighters or bombers launched in the nearby area will join
    // a squadron with this one as their leader.

    int convoy; // which convoy is it a member of?
    int convoy_angle [CONVOY_ARRANGEMENTS]; // for wships only.
    int convoy_dist [CONVOY_ARRANGEMENTS];
    int convoy_x;
    int convoy_y;

    int command; // used to tell which ships send messages
    // int command_rank; // ... and in which order if the commanding ships have been destroyed
    // rank var no longer used - it is assumed in message function that ships' command ranks will be in order wrt their e indices

    int spawn; // player will spawn nearby if set to 1+.

    int letter; // alpha, beta etc for wships
    int letter_rank;
    int scancol;
    int drive_colour;

    int stored_angle;
    int stored_dist; // stores these values so that if they're calculated for display they're kept for scanner
};

enum {
    MISSION_ESCORT, // escort fighter or bomber
    MISSION_GUARD, // guard wship
    MISSION_INTERCEPT,
    MISSION_ATTACK_WSHIP,
    MISSION_SCRAMBLE,
    MISSION_PLAYER_WING
};

enum {
    SCANCOL_OCSF,
    SCANCOL_FED,
    SCANCOL_CWLTH,
    SCANCOLS
};

enum {
    ECLASS_WSHIP,
    ECLASS_FIGHTER
};

enum {
    FCLASS_NONE,
    FCLASS_FIGHTER,
    FCLASS_BOMBER
};

enum {
    MOVE_NORMAL,
    MOVE_SLIDE,
    MOVE_ALL // slide capability is tested by > MOVE_SLIDE
};

struct eclass_struct {
    int ship_class;
    int fighter_class;
    int size;
    int parts;
    int turrets;
    int engines;
    int hp [MAX_PARTS];
    int energy_production [MAX_PARTS];
    int max_shield;
    int shield_recharge; // for wships, this becomes shield_energy_use; for fighters it becomes shield_recharge
    int dsprite [MAX_PARTS] [3];
    char fins;
    int generator_part;
    int generator_y;
    int engine_power [MAX_ENGINES];
    int engine_output;
    int move_mode;
    int turn;
    int drag;
    int base_turret_rp;
    int structure;
    int elongation;
    int blip_strength; // dot brightness in scanner
    int bullet_speed; // for fighters - speed of main bullet
};

enum {
    ACT_NONE,
    ACT_AWAY, // move away to make space for attack
    ACT_SEEK, // approach to attack
    ACT_ATTACK, // attacking!
    ACT_EVADE, // take evasive action
    ACT_TRANSIT, // travel longish distances - group leaders only. Group enters formation (ACT_FORM).
    ACT_FORM, // hold formation
    ACT_GUARD, // stay close to a wship
    ACT_WING_FORM,
    ACT_WING_AWAY, // move away to make space for attack
    ACT_WING_SEEK, // approach to attack
    ACT_WING_ATTACK, // attacking!
    ACT_WING_EVADE, // take evasive action

    ACT_WING_SEEK_HOVER, // approach to hover
    ACT_WING_HOVER // fighters/gunships with slide
};

enum {
    COMMAND_NONE,
    COMMAND_FORM,
    COMMAND_COVER,
    COMMAND_ATTACK,
    // TO DO: when specified target destroyed, send message to player and switch to engage
    COMMAND_ENGAGE,
    COMMAND_DEFEND
};

enum {
    PLAYER_SPRITE_1_1,
    PLAYER_SPRITE_1_2,
    PLAYER_SPRITE_1_3,
    PLAYER_SPRITE_2_1,
    PLAYER_SPRITE_2_2,
    PLAYER_SPRITE_2_3,
    PLAYER_SPRITE_3_1,
    PLAYER_SPRITE_3_2,
    PLAYER_SPRITE_3_3,
    PLAYER_SPRITES
};

enum {
    FIGHTER_SPRITE_BASIC_1,
    FIGHTER_SPRITE_BOMBER_1,
    FIGHTER_SPRITE_FRIEND_1,
    FIGHTER_SPRITE_FRIEND_2,
    FIGHTER_SPRITE_FRIEND_3,
    FIGHTER_SPRITE_FSTRIKE_1,
    FIGHTER_SPRITE_FSTRIKE_2,
    FIGHTER_SPRITE_FSTRIKE_3,
    FIGHTER_SPRITE_ESCOUT_1,
    FIGHTER_SPRITE_EINT_1,
    FIGHTER_SPRITE_BASIC_3,
    FIGHTER_SPRITE_BASIC_4,
    FIGHTER_SPRITE_IBEX_1,
    FIGHTER_SPRITE_IBEX_2,
    FIGHTER_SPRITE_IBEX_3,
    FIGHTER_SPRITE_AUROCHS_1,
    FIGHTER_SPRITE_AUROCHS_2,
    FIGHTER_SPRITE_AUROCHS_3,
    FIGHTER_SPRITE_LACEWING_1,
    FIGHTER_SPRITE_LACEWING_2,
    FIGHTER_SPRITE_LACEWING_3,
    FIGHTER_SPRITE_LACEWING_4,
    FIGHTER_SPRITE_MONARCH_1,
    FIGHTER_SPRITE_MONARCH_2,
    FIGHTER_SPRITE_MONARCH_3,
    FIGHTER_SPRITES
};

#define CONVOY_NONE -1

enum {
    CONVOY_0,
    CONVOY_1,
    CONVOY_2,
    CONVOY_3,
    CONVOY_4,
    CONVOY_5,
    CONVOY_6,
    CONVOY_7,
    CONVOY_8,
    CONVOY_9,
    CONVOY_10,
    CONVOY_11,
    CONVOY_12,
    CONVOY_13,
    CONVOY_14,
    CONVOY_15
    // if any are added, change NO_CONVOYS below as well
};

#define NO_CONVOYS 16
#define WSHIP_DRAG 18
// WSHIP_DRAG needs to be defined here because it's used for convoy movement as well as wships

struct convoystruct {
    char active; // 1 = active, 0 = inactive but was active, -1 = never active
    int ships; // number of wships in convoy
    int x, y;
    int x_speed, y_speed;
    int angle;
    int target_x, target_y;
    int target_offset_x, target_offset_y;
    int approach_convoy; // set to -1 if not approaching a convoy
    int arrangement;
    int throttle;
    int turn_speed;
    int drag;

    int fine_angle;

    int think_count;
    int target_angle;
    int turning;
    int turn_count;
    int can_turn;
    char approach_lead;
};

struct msgstruct {
    char text [2000];
    int val [5];
};

#define COMMS 6
// number of messages that can be displayed at once

struct commstruct {
    int exists;
    int ctext; // index in the ctext line structure
    int lines; // number of lines
    int flash, persist, fade;
    int y_offset; // when a gap is made above it, it moves up
    int from_type;
    int from_letter;
    int from_rank;
    int to;
    // any new variables need to be put in pack_messages in level.c
    int comm_col;
};

#define COMM_END_SPACE 7
#define COMM_BETWEEN 6

#define MESSAGE_LINE_SPACE 12

#define LINES 20
#define END_STRING '\0'
#define END_MESSAGE 1

struct linestruct {
    char text [250];
    int x, y;
    int col;
};

#define NO_TONES 57
#define BASE_TONE 200
// from sound.h

// used in the strategic map in briefing.c (initialised in display_init from starmap.bmp)
struct sstarstruct {
    char exists;
    int x;
    int y;
    int old_x;
    int old_y;
    int type;
    int settled;
    int side;
    int col;
};

#define SSTARS 500

enum {
    SS_WHITE,
    SS_YELLOW,
    SS_RED,
    SS_BLUE
};

enum {
    SSIDE_NONE,
    SSIDE_CWLTH,
    SSIDE_FED,
    SSIDE_IMP,
    SSIDE_OTHER
};
