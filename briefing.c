#include <string.h>
#include <math.h>

#include "config.h"

#include "allegro.h"

#include "globvars.h"

#include "palette.h"
#include "game.h"
#include "input.h"

#include "display.h"
#include "sound.h"
#include "menu.h"
#include "briefing.h"
#include "level.h"
#include "display_init.h"
#include "ship.h"

#include "stuff.h"
#include "text.h"
#include "debug_logger.h"

#define TRANS_MODE drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
#define END_TRANS_MODE drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
#define END_TRANS drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);

#define FREQ_BSELECT 1200
#define FREQ_BSELECT1 (FREQ_BSELECT * 1.05946 * 1.05946 * 1.05946 * 1.05946)
#define FREQ_BSELECT2 (FREQ_BSELECT * 1.05946 * 1.05946)

#define VOL_BSELECT 100
#define VOL_BSELECT1 100


extern RGB palet [256];
extern volatile unsigned char ticked;
extern BITMAP *display [3];
extern FONT* small_font;

BITMAP* bsprite [2] [BSPRITES] [2];
BITMAP* wisprite [WISPRITES];

unsigned char briefing_counter;
char bkey_wait [2];
int bkey_wait_pressed [2];

struct linestruct bline [LINES];

void bdisplay(void);
void run_bconvoys(void);
void run_bselects(void);
int check_settled(void);
void check_for_fire(void);
void run_bscripts(void);
int briefing_over_menu(void);
int briefing_loop(void);
void boverdisplay(void);
int pressing_a_key(int p, int ckeyp, int cbuttonp);
int get_bsprite(int type);
int get_bsprite_from_ship(int type);
char ask_bover_quit(void);
int choose_weapons(void);
int choose_valid(int p, int ch);
void choose_display(int finished1, int finished2);
void weapon_text(int x, int y, int w, int w2);
void draw_arrows(int x, int y, int w, int basecol, int counter);
void weapon_box(int p, int msg, int x, int y, int x1, int y1, int line_dir);
void display_wing_choices(int p, int x, int y);
void display_variant_choices(int p, int x, int y);

void run_tbox(void);
void draw_tbox(void);
void create_tbox(int msg, int x, int y, int w, int x2, int y2, int cv);
void report_text(int y);
int loss_proportion(int a);
int ship_value(int type);
int get_rank(int lost, int r2, int r3, int r4, int r5);

void init_mdata(void);
void add_mdata(int btype, int side, int number);
void display_mdata(int x, int y, int btype);

void ssdisplay(void);
void init_sstars(void);
void prepare_sstars_for_briefing(void);
void run_starmap(void);
void assign_star_side(int s, int x, int y, int rad, int side);
void assign_star_settled(int s, int x, int y, int rad);

int starzoom;
int starzoom_x;
int starzoom_y;
int zoom_target; // zooming in or out
int zoom_x_target;
int zoom_y_target;
int zoom_move_speed;
int starselect_side;
int starselect_settled;

#define BSHIPS 50
#define BSELECT 10

#define MENU_SENSE 100

#define BOX_W 160

#define SHIPS_LISTED 20
#define MODS_LISTED 10
#define VARIANTS_LISTED 5

int br_ship_choices;
int ship_list [SHIPS_LISTED];
int mod_list [MODS_LISTED];
int br_variant_choices;
int variant_list [2] [VARIANTS_LISTED];

enum {
    BSHIP_NONE,
    BSHIP_OLD2,
    BSHIP_OLD3,
    BSHIP_FRIEND3,
    BSHIP_DROM,
    BSHIP_LINER,
    BSHIP_FIGHTER_FRIEND,
    BSHIP_FIGHTER_FSTRIKE,
    BSHIP_IBEX,
    BSHIP_AUROCHS,
    BSHIP_LACEWING,
    BSHIP_MONARCH,
    BSHIP_MOTH,
    BSHIP_TRIREME,
    BSHIP_END_FRIENDS,

    BSHIP_FIGHTER,
    BSHIP_SCOUT2,
    BSHIP_BOMBER,
    BSHIP_SCOUT3,
    BSHIP_ESCOUT,
    BSHIP_ECARRIER,
    BSHIP_SCOUTCAR,
    BSHIP_FREIGHT,
    BSHIP_EBASE,
    BSHIP_EINT,
    BSHIP_UNKNOWN,
    NO_BSHIP_TYPES
};

enum {
    BCONVOY_0,
    BCONVOY_1,
    BCONVOY_2,
    BCONVOY_3,
    BCONVOY_4,
    BCONVOY_5,
    BCONVOY_6,
    BCONVOY_7,
    BCONVOY_8,
    BCONVOYS
};

struct bshipstruct {
    int type;
    int side;
    int convoy;
    int x, y;
};

struct bconvoystruct {
    int exists;
    int x;
    int y;
    int face;
    int x_speed;
    int y_speed;
    int goal_x;
    int goal_y;
    int move_settle;
};

struct bselectstruct {
    int exists;
    int x;
    int y;
    int x1, y1, x2, y2;
    int x_offset, y_offset;
    int count;
};

struct bshipstruct bship[BSHIPS];
struct bconvoystruct bconvoy[BCONVOYS];
struct bselectstruct bselect[BSELECT];

int map_mode;

enum {
    MM_STARMAP,
    MM_TACTICAL
};

enum {
    BSCRIPT_HEADER,
    BSCRIPT_STARMAP,
    BSCRIPT_TACTICAL,
    BSCRIPT_NEW_SHIP,
    BSCRIPT_NEW_CONVOY,
    BSCRIPT_MOVE,
    BSCRIPT_WAIT,
    BSCRIPT_HIT_FIRE,
    BSCRIPT_BMESSAGE, // creates a tbox. Can point to a convoy, or set cv to -1 for x/y value or -2 for no line.
    // BMESSAGE can be used for starmap
    BSCRIPT_SETTLE,
    BSCRIPT_SELECT, // tactical view only
    BSCRIPT_SELECT_SM, // tac or starmap - just uses x/y values and size, not a convoy
    BSCRIPT_CLEAR_MESSAGE,
    BSCRIPT_STARMAP_ZOOM,
    BSCRIPT_STARSELECT_SIDE,
    BSCRIPT_STARSELECT_SETTLED,
    BSCRIPT_MDATA, // adds a ship to mission data
    BSCRIPT_END
};

#define BVARS 8

#define LEFT 0
#define RIGHT 1

enum {
    BNEWSHIP_BCONVOY,
    BNEWSHIP_TYPE,
    BNEWSHIP_X,
    BNEWSHIP_Y
};

enum {
    BNEWCONVOY_CONVOY,
    BNEWCONVOY_X,
    BNEWCONVOY_Y,
    BNEWCONVOY_FACE
};

enum {
    BMOVE_CONVOY,
    BMOVE_WAIT,
    BMOVE_X,
    BMOVE_Y,
    BMOVE_SPEED,
    BMOVE_FACE,
    BMOVE_SETTLE
};

enum {
    BSELECT_CONVOY,
    BSELECT_X_OFFSET,
    BSELECT_Y_OFFSET
};

enum {
    BSELECT_SM_X,
    BSELECT_SM_Y,
    BSELECT_SM_SIZE
};

enum {
    BWAIT_TIME
};

enum {
    BMESSAGE_BMESSAGE,
    BMESSAGE_X,
    BMESSAGE_Y,
    BMESSAGE_W,
    BMESSAGE_X2,
    BMESSAGE_Y2,
    BMESSAGE_CONVOY
};

enum {
    BSTARMAPZOOM_X,
    BSTARMAPZOOM_Y,
    BSTARMAPZOOM_ZOOM,
};

enum {
    BSTARSELECT_SIDE
};

enum {
    BMDATA_BTYPE,
    BMDATA_SIDE,
    BMDATA_NUMBER
};

int tbox_exists;
int tbox_x;
int tbox_y;
int tbox_x2;
int tbox_y2;
int tbox_w;
int tbox_h;
int tbox_in;
int tbox_flash_in;
int tbox_out;
int tbox_flash_out;

struct mdatastruct {
    int side;
    int number;
};

struct mdatastruct mdata[NO_BSHIP_TYPES];

struct briefingscript {
    int type;
    int var[BVARS];
};

enum {
    BMSG_CANNON,
    BMSG_FIGHTER,
    BMSG_WPN1,
    BMSG_WPN2,
    BMSG_WPN3,
    BMSG_WPN4,
    BMSG_WPN5,
    BMSG_WPN6,
    BMSG_WPN7,

    BMSG_INTRO1,
    BMSG_INTRO2,
    BMSG_INTRO3,
    BMSG_INTRO4,
    BMSG_INTRO5,
    BMSG_INTRO6,
    BMSG_INTRO7,

    BMSG_M1_1,
    BMSG_M1_2,
    BMSG_M1_3,
    BMSG_M1_4,
    BMSG_M1_5,
    BMSG_M1_6,

    BMSG_M2_1,
    BMSG_M2_2,
    BMSG_M2_3,
    BMSG_M2_4,
    BMSG_M2_5,
    BMSG_M2_6,

    BMSG_M3_1,
    BMSG_M3_2,
    BMSG_M3_3,
    BMSG_M3_4,
    BMSG_M3_5,
    BMSG_M3_6,
    BMSG_M3_7,

    BMSGS
};


struct msgstruct bmsg[BMSGS] = {
    {" >> double autocannon $P $BPower: $C3.0 (x2) $PA basic fighter weapon. ", {0}},
    {" >> fighter $PStandard OCSF fighter. Has good armour and shielding and a powerful engine, but lacks the agility or firepower of the Angry Moth. Armed with double autocannon. ", {0}},
    {" >> anti-fighter missile $P $BPower: $C40.0 $PHigh-performance seeking missile with a sensor system capable of targetting small craft in a forward arc. Full lock-on maximises tracking capability. Low rate of fire. ", {0}},
    {" >> anti-warship missile $P $BPower: $C50.0 $PSlow missile with limited homing capability. Capable of targetting large ships only. Warhead is ineffective until missile drive fully engaged. ", {0}},
    {" >> multi-rockets $P $BPower: $C32.0 $PRocket launcher capable of loading and unleashing a stream of small rockets. ", {0}},
    {" >> torpedo $P $BPower: $C180.0 $PPowerful, but difficult to aim. Warhead is unstable and must prime both before and after launch - the torpedo will cause little harm if it explodes too early. ", {0}},
    {" >> single rocket $P $BPower: $C32.0 $PFires a single rocket. Lacks the rapid fire of the multiple rocket launcher but does not require pre-loading. ", {0}},
    {" >> tracking cannon $P $BPower: $C3.0 $PModifies the standard autocannons with electromagnets capable of deflecting fire towards targets within a narrow forward arc. ", {0}},
    {" >> heavy cannon $P $BPower: $C10.0 (x2) $PIncreases the power of the standard autocannons, at the cost of speed and rate of fire. ", {0}},

    {" In the 4th Millenium, humanity has spread to the stars. ", {0}}, // BMSG_INTRO1
    {" Trillions of people sprawl across hundreds of worlds. ", {0}}, // BMSG_INTRO2
    {" At the heart of this expanding sphere sits the ancient Sol Empire. For hundreds of years, its fleets have kept the squabbling provinces at peace. ", {0}}, // BMSG_INTRO3
    {" Far away in a distant arm of the Empire, the Dalat Commonwealth is a small but prosperous nation. ", {0}}, // BMSG_INTRO4
    {" Overshadowed in size by its more powerful neighbour, the Outer Colonies Federation, the Commonwealth preserves its independence with diplomacy and a professional, technologically advanced military. Its warships are valued members of any Imperial peacekeeping force. ", {0}}, // BMSG_INTRO5
    {" The Commonwealth had just celebrated the founding of its thirtieth settlement when the Void Gazers reconnaisance group, on patrol through uninhabited border systems, picked up unexpected activity on a long-range scan. ", {0}}, // BMSG_INTRO6
    {" The Void Gazers send a small light cruiser detachment, supported by fighters from Angry Moth Squadron, to investigate. ", {0}}, // BMSG_INTRO7

    {" Listen up, Void Gazers. We are investigating a signal indicating the imminent arrival of several starships in the system SK-02. ", {0}}, // BMSG_M1_1
    {" The signal is confused, but suggests Imperial warships. No-one has any idea why the Empire would send a fleet our way, and it's your job to find out. ", {0}}, // BMSG_M1_2
    {" All we have been able to mobilise for this mission is four light cruisers and a small fighter escort. Don't worry, we don't expect you to do any fighting. Just get in, wait for the unidentified ships to show up, then get back out while they recharge their jump drives. ", {0}}, // BMSG_M1_3
    {" Gazer 1 captain has the discretion to engage if absolutely necessary, but the last thing we want is a war. ", {0}}, // BMSG_M1_4
    {" Angry Moth, you will support Gazer 1. ", {0}}, // BMSG_M1_5
    {" Safe flying, everyone! $P $P>> $BBriefing end$C ", {0}}, // BMSG_M1_6

    {" The communications relays linking the Commonwealth to Federation space, and the neighbouring Sunrise League, appear to have been cut. ", {0}}, // BMSG_M2_1
    {" We can only assume that the Federation is behind this as well. ", {0}}, // BMSG_M2_2
    {" We need to contact the Sunrise League; since they share borders with the Federation, they may have some idea what is going on. But without our relays, we will need to send starships as messengers. ", {0}}, // BMSG_M2_3
    {" We have plotted an indirect course to the League outpost at MK-5. ", {0}}, // BMSG_M2_4
    {" A small team of ships will make several jumps to carry messages to the League. This mission is too dangerous for civilian ships, so we are sending Void Gazers with support from Angry Moth. ", {0}}, // BMSG_M2_5
    {" The team is set up for speed and range, not combat. If you meet resistance, you have orders to retreat. $P $P>> $BBriefing end$C ", {0}}, // BMSG_M2_6

    {" Our long range tracking station at Uqbar III has picked up large amounts of activity in nearside Federation space. ", {0}}, // BMSG_M3_1
    {" The Federation embassy at Dalat Prime claims to know nothing, and all attempts to establish direct diplomatic communication have failed. ", {0}}, // BMSG_M3_2
    {" We can only assume that they are preparing to attack. ", {0}}, // BMSG_M3_3
    {" So we must strike first. ", {0}}, // BMSG_M3_4
    {" We have located what appears to be a convoy of military transports carrying supplies to one of the main fleet concentrations. ", {0}}, // BMSG_M3_5
    {" A small group of Void Gazer warships, supported by Angry Moth squadron, will try to intercept the convoy between jumps. You will likely have only a few minutes, so make them count. ", {0}}, // BMSG_M3_6
    {" The AUROCHS strike bomber will be available for this mission, and the SANDFLY interceptor's weapon systems have been upgraded. $P $P>> $BBriefing end$C ", {0}}, // BMSG_M3_7

    // remember trailing space!!
};

#define BSCRIPTS 300
#define POINT_TO_NOTHING -2
#define POINT_TO_XY -1

struct briefingscript bscript[BSCRIPTS] = {

    {BSCRIPT_HEADER, {1}},

    {BSCRIPT_STARMAP},
    {BSCRIPT_WAIT, {30}},
    {BSCRIPT_BMESSAGE, {BMSG_INTRO1, 315, 285, BOX_W + 10, 0, 0, POINT_TO_NOTHING}},
    {BSCRIPT_SETTLE},
    {BSCRIPT_HIT_FIRE},

    {BSCRIPT_STARSELECT_SETTLED, {0}},
    {BSCRIPT_BMESSAGE, {BMSG_INTRO2, 315, 385, BOX_W + 10, 0, 0, POINT_TO_NOTHING}},
    {BSCRIPT_SETTLE},
    {BSCRIPT_HIT_FIRE},

    {BSCRIPT_STARSELECT_SIDE, {SSIDE_IMP}},
    {BSCRIPT_BMESSAGE, {BMSG_INTRO3, 415, 155, BOX_W + 10, 0, 0, POINT_TO_NOTHING}},
    {BSCRIPT_SETTLE},
    {BSCRIPT_HIT_FIRE},

    {BSCRIPT_STARSELECT_SIDE, {SSIDE_CWLTH}},
    {BSCRIPT_BMESSAGE, {BMSG_INTRO4, 115, 155, BOX_W + 10, 0, 0, POINT_TO_NOTHING}},
    {BSCRIPT_SETTLE},
    {BSCRIPT_HIT_FIRE},

    {BSCRIPT_STARSELECT_SIDE, {SSIDE_FED}},
    {BSCRIPT_BMESSAGE, {BMSG_INTRO5, 315, 285, BOX_W + 10, 0, 0, POINT_TO_NOTHING}},
    {BSCRIPT_SETTLE},
    {BSCRIPT_HIT_FIRE},

    {BSCRIPT_STARSELECT_SIDE, {SSIDE_CWLTH}},
    {BSCRIPT_BMESSAGE, {BMSG_INTRO6, 115, 155, BOX_W + 10, 0, 0, POINT_TO_NOTHING}},
    {BSCRIPT_SETTLE},
    {BSCRIPT_HIT_FIRE},

    {BSCRIPT_STARMAP_ZOOM, {450, 190, 3000}},
    {BSCRIPT_BMESSAGE, {BMSG_INTRO7, 115, 115, BOX_W + 10, 0, 0, POINT_TO_NOTHING}},
    {BSCRIPT_SETTLE},
    {BSCRIPT_HIT_FIRE},

    {BSCRIPT_TACTICAL},
    {BSCRIPT_BMESSAGE, {BMSG_M1_1, 315, 285, BOX_W + 10, 0, 0, POINT_TO_NOTHING}},
    {BSCRIPT_SETTLE},
    {BSCRIPT_HIT_FIRE},

    {BSCRIPT_BMESSAGE, {BMSG_M1_2, 315, 285, BOX_W + 10, 0, 0, POINT_TO_NOTHING}},
    {BSCRIPT_SETTLE},
    {BSCRIPT_HIT_FIRE},

    {BSCRIPT_NEW_CONVOY, {BCONVOY_0, 50, 250, RIGHT}},
    {BSCRIPT_BMESSAGE, {BMSG_M1_3, 150, 450, BOX_W + 10, 0, 29, BCONVOY_0}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_OLD2, 0, 0}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_OLD2, -10, -7}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_OLD2, -10, 7}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_OLD2, -20, 0}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_FIGHTER_FRIEND, -2, -6}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_FIGHTER_FRIEND, -2, 6}},
    {BSCRIPT_MDATA, {BSHIP_OLD2, TEAM_FRIEND, 4}},
    {BSCRIPT_MDATA, {BSHIP_FIGHTER_FRIEND, TEAM_FRIEND, 2}},
    {BSCRIPT_SELECT, {BCONVOY_0, 0, 0}},

    {BSCRIPT_NEW_CONVOY, {BCONVOY_2, 350, 250, LEFT}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_2, BSHIP_UNKNOWN, 0, 0}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_2, BSHIP_UNKNOWN, -35, 20}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_2, BSHIP_UNKNOWN, -35, -20}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_2, BSHIP_UNKNOWN, -70, 40}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_2, BSHIP_UNKNOWN, -70, -40}},

    {BSCRIPT_NEW_CONVOY, {BCONVOY_1, 70, 210, RIGHT}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_MOTH, 0, 0}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_FIGHTER_FRIEND, -5, -5}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_FIGHTER_FRIEND, -5, 5}},
    {BSCRIPT_MDATA, {BSHIP_MOTH, TEAM_FRIEND, 1}},
    {BSCRIPT_MDATA, {BSHIP_FIGHTER_FRIEND, TEAM_FRIEND, 2}},
    {BSCRIPT_SELECT, {BCONVOY_1, 0, 0}},
    {BSCRIPT_SETTLE},
    {BSCRIPT_HIT_FIRE},

    {BSCRIPT_SELECT, {BCONVOY_2, -80, 0}},
    {BSCRIPT_BMESSAGE, {BMSG_M1_4, 150, 150, BOX_W + 10, 0, -29, BCONVOY_2}},
    {BSCRIPT_SETTLE},
    {BSCRIPT_HIT_FIRE},

    {BSCRIPT_SELECT, {BCONVOY_1, 0, 0}},
    {BSCRIPT_BMESSAGE, {BMSG_M1_5, 250, 350, BOX_W + 10, 20, 11, BCONVOY_1}},
    {BSCRIPT_SETTLE},
    {BSCRIPT_HIT_FIRE},

    {BSCRIPT_BMESSAGE, {BMSG_M1_6, 315, 355, BOX_W + 10, 0, 0, POINT_TO_NOTHING}},
    {BSCRIPT_SETTLE},
    {BSCRIPT_HIT_FIRE},

    {BSCRIPT_WAIT, {50}},
    {BSCRIPT_END},

/* ********************************************************************************

Stage 2!

************************************************************************************ */

    {BSCRIPT_HEADER, {2}},

    {BSCRIPT_STARMAP},
    {BSCRIPT_WAIT, {30}},
    {BSCRIPT_BMESSAGE, {BMSG_M2_1, 315, 285, BOX_W + 10, 0, 0, POINT_TO_NOTHING}},
    {BSCRIPT_SETTLE},
    {BSCRIPT_HIT_FIRE},

    {BSCRIPT_BMESSAGE, {BMSG_M2_2, 315, 385, BOX_W + 10, 0, 0, POINT_TO_NOTHING}},
    {BSCRIPT_SETTLE},
    {BSCRIPT_HIT_FIRE},

    {BSCRIPT_BMESSAGE, {BMSG_M2_3, 415, 155, BOX_W + 10, 0, 0, POINT_TO_NOTHING}},
    {BSCRIPT_SETTLE},
    {BSCRIPT_HIT_FIRE},
    {BSCRIPT_BMESSAGE, {BMSG_M2_4, 315, 285, BOX_W + 10, 0, 0, POINT_TO_NOTHING}},
    {BSCRIPT_SETTLE},
    {BSCRIPT_HIT_FIRE},

    {BSCRIPT_TACTICAL},

    {BSCRIPT_NEW_CONVOY, {BCONVOY_0, 315, 285, RIGHT}},
    {BSCRIPT_BMESSAGE, {BMSG_M2_5, 150, 150, BOX_W + 10, -32, 0, BCONVOY_0}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_OLD2, 0, 0}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_OLD2, -10, -7}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_OLD2, -10, 7}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_OLD2, -24, 0}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_OLD2, -17, -12}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_OLD2, -17, 12}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_FIGHTER_FRIEND, -2, -6}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_FIGHTER_FRIEND, -2, 6}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_FIGHTER_FRIEND, -26, -6}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_FIGHTER_FRIEND, -26, 6}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_FIGHTER_FRIEND, 15, -26}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_FIGHTER_FRIEND, 15, -14}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_MOTH, 19, -20}},

    {BSCRIPT_SELECT, {BCONVOY_0, 0, 0}},
    {BSCRIPT_SETTLE},
    {BSCRIPT_HIT_FIRE},

    {BSCRIPT_BMESSAGE, {BMSG_M2_6, 350, 150, BOX_W + 10, 0, 0, POINT_TO_NOTHING}},
    {BSCRIPT_SETTLE},
    {BSCRIPT_HIT_FIRE},

    {BSCRIPT_WAIT, {50}},
    {BSCRIPT_END},

/* ********************************************************************************

Stage 3!

************************************************************************************ */

    {BSCRIPT_HEADER, {3}},

    {BSCRIPT_STARMAP},
    {BSCRIPT_WAIT, {30}},
    {BSCRIPT_BMESSAGE, {BMSG_M3_1, 315, 285, BOX_W + 10, 0, 0, POINT_TO_NOTHING}},
    {BSCRIPT_SETTLE},
    {BSCRIPT_HIT_FIRE},

    {BSCRIPT_BMESSAGE, {BMSG_M3_2, 315, 385, BOX_W + 10, 0, 0, POINT_TO_NOTHING}},
    {BSCRIPT_SETTLE},
    {BSCRIPT_HIT_FIRE},

    {BSCRIPT_BMESSAGE, {BMSG_M3_3, 415, 155, BOX_W + 10, 0, 0, POINT_TO_NOTHING}},
    {BSCRIPT_SETTLE},
    {BSCRIPT_HIT_FIRE},

    {BSCRIPT_BMESSAGE, {BMSG_M3_4, 315, 155, BOX_W + 10, 0, 0, POINT_TO_NOTHING}},
    {BSCRIPT_SETTLE},
    {BSCRIPT_HIT_FIRE},

    {BSCRIPT_TACTICAL},

    {BSCRIPT_NEW_CONVOY, {BCONVOY_0, 335, 285, RIGHT}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_FREIGHT, 0, 0}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_FREIGHT, 20, 0}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_FREIGHT, -20, 0}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_FREIGHT, 11, 11}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_FREIGHT, -11, 11}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_FREIGHT, 11, -11}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_FREIGHT, -11, -11}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_SCOUT2, -37, 0}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_SCOUT2, -32, -9}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_SCOUT2, -32, 9}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_SCOUT2, -22, -16}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_SCOUT2, -22, 16}},

    {BSCRIPT_SELECT, {BCONVOY_0, 0, 0}},
    {BSCRIPT_BMESSAGE, {BMSG_M3_5, 350, 120, BOX_W + 10, 0, -25, BCONVOY_0}},
    {BSCRIPT_SETTLE},
    {BSCRIPT_HIT_FIRE},

    {BSCRIPT_NEW_CONVOY, {BCONVOY_1, 200, 285, RIGHT}},
    {BSCRIPT_BMESSAGE, {BMSG_M3_6, 130, 150, BOX_W + 10, -12, -20, BCONVOY_1}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_OLD2, 0, 0}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_OLD2, -10, -7}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_OLD2, -10, 7}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_OLD2, -24, 0}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_OLD2, -17, -12}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_OLD2, -17, 12}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_FIGHTER_FRIEND, -2, -6}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_FIGHTER_FRIEND, -2, 6}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_FIGHTER_FRIEND, -26, -6}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_FIGHTER_FRIEND, -26, 6}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_FIGHTER_FRIEND, 15, -26}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_FIGHTER_FRIEND, 15, -14}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_MOTH, 19, -20}},

    {BSCRIPT_SELECT, {BCONVOY_1, 0, 0}},
    {BSCRIPT_SETTLE},
    {BSCRIPT_HIT_FIRE},

    {BSCRIPT_MOVE, {BCONVOY_1, 0, 300, 285, 3000, RIGHT, 1}},
    {BSCRIPT_MOVE, {BCONVOY_0, 0, 400, 285, 1200, RIGHT, 0}},

    {BSCRIPT_SETTLE},
    {BSCRIPT_HIT_FIRE},

    {BSCRIPT_BMESSAGE, {BMSG_M3_7, 350, 150, BOX_W + 10, 0, 0, POINT_TO_NOTHING}},
    {BSCRIPT_SETTLE},
    {BSCRIPT_HIT_FIRE},

    {BSCRIPT_WAIT, {50}},
    {BSCRIPT_END},

// 4th stage I guess?

    {BSCRIPT_HEADER, {2}},

    {BSCRIPT_NEW_CONVOY, {BCONVOY_3, 305, 280, RIGHT}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_3, BSHIP_EBASE, 0, 0}},
    {BSCRIPT_BMESSAGE, {BMSG_M3_1, 500, 420, BOX_W + 10, 20, 11, BCONVOY_3}},
    {BSCRIPT_SELECT, {BCONVOY_3, 0, 0}},
    {BSCRIPT_SETTLE},
    {BSCRIPT_HIT_FIRE},
    {BSCRIPT_CLEAR_MESSAGE},

    {BSCRIPT_BMESSAGE, {BMSG_M3_2, 120, 420, BOX_W + 10, 20, 11, -2}},
    {BSCRIPT_SETTLE},
    {BSCRIPT_WAIT, {20}},
    {BSCRIPT_HIT_FIRE},
    {BSCRIPT_CLEAR_MESSAGE},

    {BSCRIPT_NEW_CONVOY, {BCONVOY_4, 305, 280, RIGHT}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_4, BSHIP_SCOUT2, 20, -20}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_4, BSHIP_SCOUT2, -20, -20}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_4, BSHIP_SCOUT3, -30, 0}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_4, BSHIP_SCOUT3, 30, 0}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_4, BSHIP_SCOUT2, -20, 20}},
    {BSCRIPT_NEW_SHIP, {BCONVOY_4, BSHIP_SCOUT2, 20, 20}},
    {BSCRIPT_SELECT, {BCONVOY_4, 0, 0}},
    {BSCRIPT_BMESSAGE, {BMSG_M3_4, 500, 420, BOX_W + 10, 50, 11, BCONVOY_4}},
    {BSCRIPT_SETTLE},
    {BSCRIPT_HIT_FIRE},
    {BSCRIPT_CLEAR_MESSAGE},
};

int waiting;
int waiting_for_fire;
int briefing_pos;
int selecting [2];
char finished_wship_process;

void finish_wship_process(void);

enum {
    BOVER_MENU_START,
    BOVER_MENU_REPLAY,
    BOVER_MENU_DATA,
    BOVER_MENU_QUIT,
    BOVER_MENU_END
};

#define BCOL_BACK (COL_BOX1)
#define BCOL_SCREEN (COL_BOX1)
#define BCOL_DARK (COL_BOX0)
#define BCOL_EDGE (COL_BOX2)
#define BCOL_TEXT (COL_BOX4)
#define EDGE 1

enum {
    CHOOSE_SHIP,
    CHOOSE_VARIANT,
    CHOOSE_MOD1,
    CHOOSE_MOD2,
    CHOOSE_MOD3,
    CHOOSE_WEAPON1,
    CHOOSE_WEAPON2,
    CHOOSE_WEAPON3,
    CHOOSE_ESCORT1,
    CHOOSE_ESCORT2,
    CHOOSE_WING1,
    CHOOSE_WING2,

    CHOOSE_LAUNCH,
    CHOOSE_REPLAY,
    CHOOSE_END
};

int wship_process_indicator;
int wship_process_indicator_max;
void wship_process_progress(void);
void set_player_ship_type(int p, int type, int variant);

// order must match order of FFs. So I don't know why I bothered to have the first entry here.
int fighter_ship[NO_FF][2] = {
    {FF_NONE, SHIP_NONE},
    {FF_SANDFLY, SHIP_FIGHTER_FRIEND},
    {FF_MONARCH, SHIP_MONARCH},
    {FF_IBEX, SHIP_IBEX},
    {FF_AUROCHS, SHIP_AUROCHS},
};

void init_mission_briefing(void)
{
    // note: this function is called every time the briefing is restarted as well as the first time it starts.
    prepare_sstars_for_briefing();

    int c, s, i;

    for (c = 0; c < BCONVOYS; c ++) {
        bconvoy[c].exists = 0;
    }
    for (s = 0; s < BSHIPS; s ++) {
        bship[s].type = BSHIP_NONE;
    }
    for (s = 0; s < BSELECT; s ++) {
        bselect[s].exists = 0;
    }

    briefing_pos = 0;

    for (i = 0; i < BSCRIPTS; i ++) {
        if (bscript [i].type == BSCRIPT_HEADER
            && bscript [i].var [0] == arena.stage
        ) {
            briefing_pos = i;
            break;
        }
    }

    bline[0].text [0] = END_MESSAGE;
    tbox_exists = 0;

    init_mdata();

    waiting = 0;
    waiting_for_fire = 0;

    int p;

    player[0].escort_type[0] = SHIP_FIGHTER_FRIEND; // escort_type only valid for player 0
    player[0].escort_type[1] = SHIP_FIGHTER_FRIEND;

    for (p = 0; p < 2; p++) {
        PP.wings = 1;
        PP.wing_type[0] = 0;
        PP.wing_type[1] = -1;
        set_player_ship_type(p, SHIP_FIGHTER_FRIEND, CVAR_NONE);
    }

    int count = 0;

    for (i = 0; i < NO_FF; i ++) {
        if (game.fighter_available[i] == 1) {
            ship_list[count] = fighter_ship[i][1];
            count ++;
        }
    }

    ship_list[count] = SHIP_NONE;
    br_ship_choices = count;
}

// this is used both to set ship types and to set variant types when ship type is already set.
void set_player_ship_type(int p, int type, int variant)
{
    PP.type = type;
    PP.weapon_type[0] = WPN_NONE;
    PP.weapon_type[1] = WPN_NONE;
    PP.shield_recharge = eclass[type].shield_recharge;
    // some other values get set in init_player_spawn in game.c

    int v = 0;

    switch(type) {
        case SHIP_FIGHTER_FRIEND:
            if (arena.stage > 2) {
                PP.weapon_type[0] = WPN_LW_MISSILE;
            }
            variant_list[p][v++] = CVAR_NONE;
            variant_list[p][v] = CVAR_END;
            break;
        case SHIP_FSTRIKE:
            PP.weapon_type[0] = WPN_WROCKET;
            variant_list[p][v++] = CVAR_NONE;
            variant_list[p][v++] = CVAR_RAM_HEAVY;
            variant_list[p][v] = CVAR_END;
            break;
        case SHIP_LACEWING:
            PP.weapon_type[0] = WPN_LW_MISSILE;
            variant_list[p][v++] = CVAR_NONE;
            variant_list[p][v++] = CVAR_LW_B;
            variant_list[p][v++] = CVAR_LW_C;
            variant_list[p][v] = CVAR_END;
            break;
        case SHIP_MONARCH:
            PP.weapon_type[0] = WPN_LW_MISSILE;
            variant_list[p][v++] = CVAR_NONE;
            variant_list[p][v++] = CVAR_MONARCH_B;
            variant_list[p][v++] = CVAR_MONARCH_C;
            variant_list[p][v] = CVAR_END;
            break;
        case SHIP_IBEX:
            PP.weapon_type[0] = WPN_AWS_MISSILE;
            variant_list[p][v++] = CVAR_NONE;
            variant_list[p][v++] = CVAR_IBEX_B;
            variant_list[p][v++] = CVAR_IBEX_C;
            variant_list[p][v] = CVAR_END;
            break;
        case SHIP_AUROCHS:
            PP.weapon_type[0] = WPN_HROCKET;
            variant_list[p][v++] = CVAR_NONE;
            variant_list[p][v++] = CVAR_AUROCHS_B;
            variant_list[p][v++] = CVAR_AUROCHS_C;
            variant_list[p][v] = CVAR_END;
            break;
    }

    PP.variant = variant;
    br_variant_choices = v;
}

void finish_wship_process(void)
{
    #define FX 400
    #define FY 300

    int pl = wship_process_indicator >> 2;
    int pl2 = wship_process_indicator_max >> 2;

    hline(display[0], 795 - pl2 - 1, 595, 795 - pl2 + pl, COL_BOX3);

    if (finished_wship_process == 0) {
        vsync();
        clear_to_color(screen, COL_BOX0);
        textprintf_right_ex(screen, small_font, FX - 5, FY - 9, COL_BOX4, -1, "Loading");
        vline(screen, FX, FY - 2, FY + 2, COL_BOX3);
        vline(screen, FX + pl2 + 2, FY - 2, FY + 2, COL_BOX4);
        hline(screen, FX, FY, FX + pl, COL_BOX4);
    }

    while (finished_wship_process == 0) {
        finished_wship_process = wship_process();
        wship_process_indicator ++;
        pl = wship_process_indicator >> 2;
        hline(screen, FX, FY, FX + pl, COL_BOX4);
    };

    vsync();
    clear_bitmap(display[0]);
    clear_bitmap(screen);
}

void wship_process_progress(void)
{
    if (finished_wship_process == 1) {
        return;
    }

    int pl = wship_process_indicator >> 2;
    int pl2 = wship_process_indicator_max >> 2;

    vline(display[0], 795, 591, 595, COL_BOX3);
    vline(display[0], 795 - pl2 - 2, 591, 595, COL_BOX3);
    hline(display[0], 795 - pl2 - 1, 593, 795 - pl2 + pl, COL_BOX3);

    textprintf_right_ex(display[0], small_font, 795 - pl2 - 5, 586, COL_BOX3, -1, "Loading");
}

int mission_briefing(void)
{

 init_mission_briefing();

 wship_process_indicator_max = pre_briefing_wship_sprite_check();
 wship_process_indicator = 0;
// pre_briefing_wship_sprite_check();
 start_making_new_wships();
 finished_wship_process = 0;

 if (wship_process_indicator_max == 0)
  finished_wship_process = 1; // nothing to process

 int menu_return = -1;
 char replay = 0;
 char finished = 0;

 do
 {

  replay = 0;

  if (briefing_loop() == 1)
  {
   finish_wship_process();
   return 0; // must have quit
  }

  if (bscript[briefing_pos].type == BSCRIPT_END)
   finished = 1;
    else
     if (pressing_a_key(0, CKEY_FIRE2, JBUTTON_FIRE2))
     {
      finished = 1;
      while (bscript[briefing_pos].type != BSCRIPT_END)
      {
       if (bscript[briefing_pos].type == BSCRIPT_MDATA)
        add_mdata(bscript[briefing_pos].var[BMDATA_BTYPE], bscript[briefing_pos].var[BMDATA_SIDE], bscript[briefing_pos].var[BMDATA_NUMBER]);
       briefing_pos ++;
      };
     }

    if (finished == 1)
    {
     finished = 0;
     bkey_wait [0] = 10;
     play_basicwfv(WAV_SELECT0, FREQ_BSELECT, VOL_BSELECT1);
//   || options.joystick_available [0] && joy[0].button[options.joy_button [0] [1]].b))
    do
    {
     menu_return = briefing_over_menu();
     switch(menu_return)
     {
      case BOVER_MENU_START:
       play_basicwfv(WAV_SELECT1, FREQ_BSELECT1, VOL_BSELECT1);
       menu_return = choose_weapons();
       if (menu_return == 0)
       {
        init_mission_briefing();
        replay = 1;
        break;
       }
       play_basicwfv(WAV_SELECT1, FREQ_BSELECT1, VOL_BSELECT1);
       finish_wship_process();
       return 1;
      case BOVER_MENU_REPLAY:
       init_mission_briefing();
       replay = 1;
       play_basicwfv(WAV_SELECT1, FREQ_BSELECT1, VOL_BSELECT1);
       break;
      case BOVER_MENU_DATA:
// data
       break;
      case BOVER_MENU_QUIT:
       if (ask_bover_quit())
       {
        play_basicwfv(WAV_SELECT0, FREQ_BSELECT2, VOL_BSELECT1);
        finish_wship_process();
        return 0;
       }
       break;

     }
    } while (replay == 0);
   }


 } while (TRUE);

 finish_wship_process();
 return 1;

}

int briefing_loop(void)
{
    if (key[KEY_ESC]) {
        return ask_bover_quit();
    }
    briefing_counter ++;

    if (map_mode == MM_TACTICAL) {
        run_bconvoys();
    }

    run_bselects();
    run_starmap();

    if (tbox_exists) {
        run_tbox();
    }

    if (bscript[briefing_pos].type == BSCRIPT_SETTLE) {
        if (check_settled() == 0) {
            waiting = 1;
        }
    } // wait for all convoys to stop moving

    if (waiting > 0) {
        waiting--;
    } else {
        if (waiting_for_fire > 0) {
            check_for_fire();
        } else {
            run_bscripts();
        }
    }

    bdisplay();
    wship_process_progress();

    blit(display[0], screen, 0, 0, 0, 0, 800, 600);

    if (finished_wship_process == 0) {
        finished_wship_process = wship_process();
        wship_process_indicator++;
    }

    while (ticked == 0) {
        rest(1);
    };
    ticked = 0;
    return 0;
}

#define MAP_X 10
#define MAP_Y 10
#define MAP_W 600
#define MAP_H 550

#define BOX_X 630
#define BOX_Y 10
#define BOX_H 550

void bdisplay(void)
{
    int s, c;

    if (map_mode == MM_TACTICAL) {
        clear_to_color(display [0], 0);

        rectfill(display[0], MAP_X-EDGE, MAP_Y-EDGE, MAP_X + MAP_W+EDGE, MAP_Y + MAP_H+EDGE, BCOL_EDGE);
        rectfill(display[0], MAP_X, MAP_Y, MAP_X + MAP_W, MAP_Y + MAP_H, BCOL_DARK);

        rectfill(display[0], MAP_X, MAP_Y + 5, MAP_X + MAP_W, MAP_Y + 22, BCOL_EDGE);
        textprintf_centre_ex(display[0], small_font, MAP_X + (MAP_W>>1), MAP_Y+7, BCOL_TEXT, -1, "Mission Briefing");

        for (s = 0; s < BSHIPS; s ++) {
            if (bship[s].type == BSHIP_NONE) {
                continue;
            }
            if (bconvoy[bship[s].convoy].face == RIGHT) {
                draw_sprite(
                    display[0],
                    bsprite[bship[s].side][get_bsprite(bship[s].type)][0],
                    MAP_X + (bconvoy[bship[s].convoy].x >> 10) + bship[s].x - 6,
                    MAP_Y + (bconvoy[bship[s].convoy].y >> 10) + bship[s].y - 6
                );
            } else {
                draw_sprite_h_flip(
                    display[0],
                    bsprite[bship[s].side][get_bsprite(bship[s].type)][0],
                    MAP_X + (bconvoy[bship[s].convoy].x >> 10) - bship[s].x - 6,
                    MAP_Y + (bconvoy[bship[s].convoy].y >> 10) - bship[s].y - 6
                );
            }
        }
    } else {
        ssdisplay(); // map_mode == MM_STARMAP
    }

    for (s = 0; s < BSELECT; s ++) {
        if (bselect[s].exists == 0
            || bselect[s].count <= 0
        ) {
            continue;
        }
        c = bselect[s].count*3 - 10 - 20;
        if (c < 10) {
            c = 10;
        }
        vline(display[0], bselect[s].x + bselect[s].x1 - c, bselect[s].y + bselect[s].y1 - c, bselect[s].y + bselect[s].y1 - c + 4, COL_F5 + TRANS_BLUE3);
        hline(display[0], bselect[s].x + bselect[s].x1 - c, bselect[s].y + bselect[s].y1 - c, bselect[s].x + bselect[s].x1 - c + 4, COL_F5 + TRANS_BLUE3);

        vline(display[0], bselect[s].x + bselect[s].x2 + c, bselect[s].y + bselect[s].y1 - c, bselect[s].y + bselect[s].y1 - c + 4, COL_F5 + TRANS_BLUE3);
        hline(display[0], bselect[s].x + bselect[s].x2 + c, bselect[s].y + bselect[s].y1 - c, bselect[s].x + bselect[s].x2 + c - 4, COL_F5 + TRANS_BLUE3);

        vline(display[0], bselect[s].x + bselect[s].x1 - c, bselect[s].y + bselect[s].y2 + c, bselect[s].y + bselect[s].y2 + c - 4, COL_F5 + TRANS_BLUE3);
        hline(display[0], bselect[s].x + bselect[s].x1 - c, bselect[s].y + bselect[s].y2 + c, bselect[s].x + bselect[s].x1 - c + 4, COL_F5 + TRANS_BLUE3);

        vline(display[0], bselect[s].x + bselect[s].x2 + c, bselect[s].y + bselect[s].y2 + c, bselect[s].y + bselect[s].y2 + c - 4, COL_F5 + TRANS_BLUE3);
        hline(display[0], bselect[s].x + bselect[s].x2 + c, bselect[s].y + bselect[s].y2 + c, bselect[s].x + bselect[s].x2 + c - 4, COL_F5 + TRANS_BLUE3);
    }

    rectfill(display[0], 250, 575, 550, 595, BCOL_EDGE);

    vline(display[0], MAP_X + MAP_W - 80, MAP_Y + MAP_H + 2, MAP_Y + MAP_H + 8, BCOL_EDGE);
    hline(display[0], 320, MAP_Y + MAP_H + 8, MAP_X + MAP_W - 80, BCOL_EDGE);
    vline(display[0], 320, MAP_Y + MAP_H + 8, 595 - EDGE, BCOL_EDGE);

    if (tbox_exists) {
        draw_tbox();
    }

    if (waiting_for_fire) {
        rectfill(display[0], 250+EDGE, 575+EDGE, 550-EDGE, 595-EDGE, COL_BOX2);
        textprintf_centre_ex(display[0], small_font, 400, 580, BCOL_TEXT, -1, "Press fire 1 to continue or fire 2 to skip");
        draw_arrows(400, 580, 115, 2, briefing_counter);
    } else {
        rectfill(display[0], 250+EDGE, 575+EDGE, 550-EDGE, 595-EDGE, BCOL_DARK);
    }
}

int get_bsprite(int type)
{
    switch(type) {
        case BSHIP_OLD2: return BSF_OLD2;
        case BSHIP_OLD3: return BSF_OLD3;
        case BSHIP_FIGHTER_FRIEND: return BSF_FIGHTER;
        case BSHIP_FIGHTER_FSTRIKE: return BSF_FIGHTER;
        case BSHIP_MOTH: return BSF_MOTH;
        case BSHIP_DROM: return BSF_DROM;
        case BSHIP_LINER: return BSF_DROM;
        case BSHIP_TRIREME: return BSF_TRIREME;

        case BSHIP_FIGHTER: return BSE_FIGHTER;
        case BSHIP_BOMBER: return BSE_FIGHTER;
        case BSHIP_EINT: return BSE_FIGHTER;
        case BSHIP_ESCOUT: return BSE_FIGHTER;
        case BSHIP_SCOUT2: return BSE_SCOUT2;
        case BSHIP_SCOUT3: return BSE_SCOUT3;
        case BSHIP_ECARRIER: return BSE_ECARRIER;
        case BSHIP_FREIGHT: return BSE_FREIGHT;
        case BSHIP_EBASE: return BSE_EBASE;
        case BSHIP_UNKNOWN: return BSE_UNKNOWN;

        default: return BSE_EBASE; // this is BSF_MOTH for friendly, I think
    }
}

int get_bsprite_from_ship(int type)
{
    switch(type) {
        case SHIP_OLD2: return BSF_OLD2;
        case SHIP_OLD3: return BSF_OLD3;
        case SHIP_DROM: return BSF_DROM;
        case SHIP_LINER: return BSF_LINER;
        case SHIP_FIGHTER_FRIEND: return BSF_FIGHTER;
        case SHIP_LACEWING:
        case SHIP_MONARCH:
        case SHIP_IBEX:
        case SHIP_AUROCHS:
        case SHIP_FSTRIKE: return BSF_FIGHTER;
        case SHIP_FRIEND3: return BSF_TRIREME;

        case SHIP_FIGHTER: return BSE_FIGHTER;
        case SHIP_BOMBER: return BSE_FIGHTER;
        case SHIP_ESCOUT: return BSE_FIGHTER;
        case SHIP_EINT: return BSE_FIGHTER;
        case SHIP_SCOUT2: return BSE_SCOUT2;
        case SHIP_SCOUT3: return BSE_SCOUT3;
        case SHIP_ECARRIER: return BSE_ECARRIER;
        case SHIP_FREIGHT: return BSE_ECARRIER;
        case SHIP_EBASE: return BSE_EBASE;

        default: return BSE_EBASE;
    }
}

void run_bscripts(void)
{
    int c, s;
    float f_angle;

    do {
        briefing_pos++;

        switch(bscript[briefing_pos].type) {
            case BSCRIPT_STARMAP:
                map_mode = MM_STARMAP;
                break;
            case BSCRIPT_TACTICAL:
                map_mode = MM_TACTICAL;
                break;
            case BSCRIPT_NEW_CONVOY:
                c = bscript[briefing_pos].var[BNEWCONVOY_CONVOY];
                bconvoy[c].x = bscript[briefing_pos].var[BNEWCONVOY_X]<<10;
                bconvoy[c].y = bscript[briefing_pos].var[BNEWCONVOY_Y]<<10;
                bconvoy[c].exists = 1;
                bconvoy[c].x_speed = 0;
                bconvoy[c].y_speed = 0;
                bconvoy[c].face = bscript[briefing_pos].var[BNEWCONVOY_FACE];
                break;
            case BSCRIPT_NEW_SHIP:
                c = bscript[briefing_pos].var[BNEWSHIP_BCONVOY];
                for (s = 0; s < BSHIPS; s ++) {
                    if (bship[s].type == BSHIP_NONE) {
                        bship[s].type = bscript[briefing_pos].var[BNEWSHIP_TYPE];
                        bship[s].convoy = bscript[briefing_pos].var[BNEWSHIP_BCONVOY];
                        bship[s].x = bscript[briefing_pos].var[BNEWSHIP_X];
                        bship[s].y = bscript[briefing_pos].var[BNEWSHIP_Y];
                        if (bship[s].type < BSHIP_END_FRIENDS) {
                            bship[s].side = TEAM_FRIEND;
                        } else {
                            bship[s].side = TEAM_ENEMY;
                        }
                       break;
                    }
                }
                break;
            case BSCRIPT_MOVE:
                c = bscript[briefing_pos].var[BMOVE_CONVOY];
                f_angle = atan2(
                    (bscript[briefing_pos].var[BMOVE_Y] << 10) - bconvoy[c].y,
                    (bscript[briefing_pos].var[BMOVE_X] << 10) - bconvoy[c].x
                );
                bconvoy[c].x_speed = fxpart(f_angle, bscript[briefing_pos].var[BMOVE_SPEED]);
                bconvoy[c].y_speed = fypart(f_angle, bscript[briefing_pos].var[BMOVE_SPEED]);
                bconvoy[c].goal_x = bscript[briefing_pos].var[BMOVE_X] << 10;
                bconvoy[c].goal_y = bscript[briefing_pos].var[BMOVE_Y] << 10;
                bconvoy[c].face = bscript[briefing_pos].var[BMOVE_FACE];
                bconvoy[c].move_settle = bscript[briefing_pos].var[BMOVE_SETTLE];
                break;
            case BSCRIPT_WAIT:
                waiting = bscript[briefing_pos].var[BWAIT_TIME];
                break;
            case BSCRIPT_HIT_FIRE:
                waiting_for_fire = 2;
                break;
            case BSCRIPT_BMESSAGE:
                create_tbox(
                    bscript[briefing_pos].var[BMESSAGE_BMESSAGE],
                    bscript[briefing_pos].var[BMESSAGE_X], bscript[briefing_pos].var[BMESSAGE_Y],
                    bscript[briefing_pos].var[BMESSAGE_W], bscript[briefing_pos].var[BMESSAGE_X2],
                    bscript[briefing_pos].var[BMESSAGE_Y2], bscript[briefing_pos].var[BMESSAGE_CONVOY]
                );
                break;
            case BSCRIPT_CLEAR_MESSAGE:
                tbox_exists = 0;
                break;
            case BSCRIPT_SELECT:
                for (s = 0; s < BSELECT; s ++) {
                    if (bselect[s].exists == 0) {
                        bselect[s].x = (bconvoy[bscript[briefing_pos].var[BSELECT_CONVOY]].x >> 10) + bscript[briefing_pos].var[BSELECT_X_OFFSET] + MAP_X;
                        bselect[s].y = (bconvoy[bscript[briefing_pos].var[BSELECT_CONVOY]].y >> 10) + bscript[briefing_pos].var[BSELECT_Y_OFFSET] + MAP_Y;
                        if (bconvoy[bscript[briefing_pos].var[BSELECT_CONVOY]].face == LEFT) {
                            bselect[s].x = (bconvoy[bscript[briefing_pos].var[BSELECT_CONVOY]].x >> 10) + MAP_X - bscript[briefing_pos].var[BSELECT_X_OFFSET];
                            bselect[s].y = (bconvoy[bscript[briefing_pos].var[BSELECT_CONVOY]].y >> 10) + MAP_Y - bscript[briefing_pos].var[BSELECT_Y_OFFSET];
                        }
                        bselect[s].x1 = -5;
                        bselect[s].y1 = -5;
                        bselect[s].x2 = 5;
                        bselect[s].y2 = 5;
                        for (c = 0; c < BSHIPS; c ++) {
                            if (bship[c].type != BSHIP_NONE
                                && bship[c].convoy == bscript[briefing_pos].var[BSELECT_CONVOY]
                            ) {
                                if (bship[c].x < bselect[s].x1 - 0) {
                                    bselect[s].x1 = bship[c].x - 0;
                                }
                                if (bship[c].y < bselect[s].y1 - 0) {
                                    bselect[s].y1 = bship[c].y - 0;
                                }
                                if (bship[c].x > bselect[s].x2 + 0) {
                                    bselect[s].x2 = bship[c].x + 0;
                                }
                                if (bship[c].y > bselect[s].y2 + 0) {
                                    bselect[s].y2 = bship[c].y + 0;
                                }
                            }
                        }
                        bselect[s].exists = 1;
                        bselect[s].count = 25;
                        break;
                    }
                }
                break;
            case BSCRIPT_SELECT_SM:
                for (s = 0; s < BSELECT; s ++) {
                    if (bselect[s].exists == 0) {
                        bselect[s].x = bscript[briefing_pos].var[BSELECT_SM_X] + MAP_X;
                        bselect[s].y = bscript[briefing_pos].var[BSELECT_SM_Y] + MAP_Y;
                        bselect[s].x1 = bscript[briefing_pos].var[BSELECT_SM_SIZE] * -1;
                        bselect[s].y1 = bscript[briefing_pos].var[BSELECT_SM_SIZE] * -1;
                        bselect[s].x2 = bscript[briefing_pos].var[BSELECT_SM_SIZE];
                        bselect[s].y2 = bscript[briefing_pos].var[BSELECT_SM_SIZE];
                        bselect[s].exists = 1;
                        bselect[s].count = 25;
                        break;
                    }
                }
                break;
            case BSCRIPT_STARMAP_ZOOM:
                zoom_x_target = bscript[briefing_pos].var[BSTARMAPZOOM_X];
                zoom_y_target = bscript[briefing_pos].var[BSTARMAPZOOM_Y];
                zoom_target = bscript[briefing_pos].var[BSTARMAPZOOM_ZOOM];
                if (zoom_target == starzoom) {
                    zoom_move_speed = 3;
                } else {
                    zoom_move_speed = hypot(zoom_y_target - starzoom_y, zoom_x_target - starzoom_x) / (abs(zoom_target - starzoom)/40) + 1;
                }
                break;
            case BSCRIPT_STARSELECT_SIDE:
                starselect_side = bscript[briefing_pos].var[BSTARSELECT_SIDE];
                starselect_settled = 0;
                break;
            case BSCRIPT_STARSELECT_SETTLED:
                starselect_settled = 1;
                break;
            case BSCRIPT_MDATA:
                add_mdata(
                    bscript[briefing_pos].var[BMDATA_BTYPE],
                    bscript[briefing_pos].var[BMDATA_SIDE],
                    bscript[briefing_pos].var[BMDATA_NUMBER]
                );
                break;
        }
    } while (bscript[briefing_pos].type != BSCRIPT_WAIT
       && bscript[briefing_pos].type != BSCRIPT_HIT_FIRE
       && bscript[briefing_pos].type != BSCRIPT_SETTLE
       && bscript[briefing_pos].type != BSCRIPT_END
    );
}

void init_mdata(void)
{
    int i;

    for (i = 0; i < NO_BSHIP_TYPES; i ++) {
        mdata[i].number = 0;
    }
}

void add_mdata(int btype, int side, int number)
{
    mdata[btype].side = side;
    mdata[btype].number += number;
}

void create_tbox(int msg, int x, int y, int w, int x2, int y2, int cv)
{
    tbox_exists = 1;
    tbox_out = 0;
    tbox_w = w + 12;

    int lines = generate_message(bmsg[msg], w, 15, BCOL_TEXT, bline);
    tbox_h = (lines * 15) + 12;

    tbox_w >>= 1;
    tbox_h >>= 1;

    tbox_in = tbox_h;
    if (tbox_w > tbox_h) {
        tbox_in = tbox_w;
    }

    tbox_x = x;
    tbox_y = y;
    tbox_x2 = x2;
    tbox_y2 = y2;

    if (cv == -2) {
        tbox_x2 = x;
        tbox_y2 = y;
    }

    if (cv >= 0) {
        tbox_x2 += (bconvoy[cv].x>>10) + MAP_X;
        tbox_y2 += (bconvoy[cv].y>>10) + MAP_Y;
    }
}


void draw_tbox(void)
{
    int w, h;
    int col = COL_BOX4;

    if (tbox_x2 != tbox_x || tbox_y2 != tbox_y) {
        if (tbox_x2 < tbox_x - tbox_w - 10) {
            hline(display[0], tbox_x, tbox_y, tbox_x - tbox_w - 10, col);
            vline(display[0], tbox_x - tbox_w - 10, tbox_y, tbox_y2, col);
            hline(display[0], tbox_x - tbox_w - 10, tbox_y2, tbox_x2, col);
        } else {
            if (tbox_x2 > tbox_x + tbox_w + 10) {
                hline(display[0], tbox_x, tbox_y, tbox_x + tbox_w + 10, col);
                vline(display[0], tbox_x + tbox_w + 10, tbox_y, tbox_y2, col);
                hline(display[0], tbox_x + tbox_w + 10, tbox_y2, tbox_x2, col);
            } else {
                if (tbox_y2 < tbox_y) {
                    vline(display[0], tbox_x, tbox_y, tbox_y - tbox_h - 10, col);
                    hline(display[0], tbox_x, tbox_y - tbox_h - 10, tbox_x2, col);
                    vline(display[0], tbox_x2, tbox_y - tbox_h - 10, tbox_y2, col);
                } else {
                    vline(display[0], tbox_x, tbox_y, tbox_y + tbox_h + 10, col);
                    hline(display[0], tbox_x, tbox_y + tbox_h + 10, tbox_x2, col);
                    vline(display[0], tbox_x2, tbox_y + tbox_h + 10, tbox_y2, col);
                }
            }
        }
    }

    if (tbox_in > 0) {
        w = tbox_w - tbox_in;
        if (w < 0) {
            w = 0;
        }
        h = tbox_h - tbox_in;
        if (h < 0) {
            h = 0;
        }
        rectfill(display[0], tbox_x - w, tbox_y - h, tbox_x + w, tbox_y + h, COL_BOX4);
        return;
    }

    if (tbox_out > 0) {
        w = tbox_out;
        if (w > tbox_w) {
            w = tbox_w;
        }
        h = tbox_out;
        if (h > tbox_h) {
            h = tbox_h;
        }
        rectfill(display[0], tbox_x - w, tbox_y - h, tbox_x + w, tbox_y + h, COL_BOX4);
        return;
    }

    col = COL_BOX1;

    if (tbox_flash_in > 0) {
        col = COL_BOX1 + tbox_flash_in / 5;
        if (col > COL_BOX4) {
            col = COL_BOX4;
        }
    }

    if (tbox_flash_out > 0) {
        col = COL_BOX4 - (tbox_flash_out / 5);
        if (col < COL_BOX1) {
            col = COL_BOX1;
        }
    }

    rectfill(display[0], tbox_x - tbox_w, tbox_y - tbox_h, tbox_x + tbox_w, tbox_y + tbox_h, col);
    col++;

    if (col > COL_BOX4) {
        col = COL_BOX4;
    }

    rect(display[0], tbox_x - tbox_w, tbox_y - tbox_h, tbox_x + tbox_w, tbox_y + tbox_h, col);
    display_message(display[0], bline, tbox_x - tbox_w + 6, tbox_y - tbox_h + 6);
}

void run_tbox(void)
{
    if (tbox_in > 0) {
        tbox_in -= 10;
        if (tbox_in <= 0) {
            tbox_flash_in = 20;
        }
        return;
    }

    if (tbox_flash_in > 0) {
        tbox_flash_in -= 2;
    }

    if (tbox_flash_out > 0) {
        tbox_flash_out--;
        if (tbox_flash_out <= 0) {
            tbox_out = tbox_w;
        }
        if (tbox_h > tbox_out) {
            tbox_out = tbox_h;
        }
    }

    if (tbox_out > 0) {
        tbox_out -= 3;
        if (tbox_out <= 0) {
            tbox_exists = 0;
        }
        return;
    }
}


void run_bconvoys(void)
{
 int c;

 for (c = 0; c < BCONVOYS; c ++)
 {
  if (bconvoy[c].exists == 0)
   continue;
  bconvoy[c].x += bconvoy[c].x_speed;
  bconvoy[c].y += bconvoy[c].y_speed;
  if ((bconvoy[c].x>>13) == (bconvoy[c].goal_x>>13)
   && (bconvoy[c].y>>13) == (bconvoy[c].goal_y>>13))
  {
   bconvoy[c].x_speed = 0;
   bconvoy[c].y_speed = 0;
  }
 }

}

void run_bselects(void)
{

 int c;

 for (c = 0; c < BSELECT; c ++)
 {
  if (bselect[c].exists == 0)
   continue;
  bselect[c].count --;
  if (bselect[c].count < 1)
   bselect[c].exists = 0;
 }

}


int check_settled(void)
{
 int c;

 for (c = 0; c < BCONVOYS; c ++)
 {
  if (bconvoy[c].exists == 1
   && bconvoy[c].move_settle == 1
   && (bconvoy[c].x_speed != 0
    || bconvoy[c].y_speed != 0))
    return 0; // not settled yet
 }

 for (c = 0; c < BSELECT; c++)
 {
  if (bselect[c].exists == 1
   && bselect[c].count > 0)
    return 0; // still zooming select
 }

 if (tbox_flash_out > 0
  || tbox_out > 0)
   return 0;

 if (map_mode == MM_STARMAP && starzoom != zoom_target)
  return 0;

// okay, everything's settled so let's clear speed values for all
//  bconvoys that have move_settle == 0:
 for (c = 0; c < BCONVOYS; c ++)
 {
  if (bconvoy[c].exists == 1
   && bconvoy[c].move_settle == 0)
   {
    bconvoy[c].x_speed = 0;
    bconvoy[c].y_speed = 0;
   }
 }

 return 1;
}


void check_for_fire(void)
{

 int p, js, kb;

  if (waiting_for_fire == 1)
  {
   for (p = 0; p < 1; p ++)
   {
    if (p == 1 && arena.players == 1)
     continue;
    if (PP.control == CONTROL_KEY_A
     || PP.control == CONTROL_KEY_B)
    {
     if (key [options.ckey [PP.control] [CKEY_FIRE1]])
     {
      waiting_for_fire = 0;
      play_basicwfv(WAV_SELECT0, FREQ_BSELECT, VOL_BSELECT1);
     }
    }
     else
     {
       poll_joystick();
       js = PP.control - CONTROL_JOY_A;
       if (joy[js].button[options.joy_button [js] [JBUTTON_FIRE1]].b)
       {
        waiting_for_fire = 0;
        play_basicwfv(WAV_SELECT0, FREQ_BSELECT, VOL_BSELECT1);
       }
     }
   }
/*   if (key [KEY_SPACE] || key [KEY_ENTER] || key [KEY_ENTER_PAD] || key [KEY_Z] || key [options.ckey [0] [CKEY_FIRE1]])
    waiting_for_fire = 0;
   if (options.joystick_available [0] && joy[0].button[options.joy_button [0] [0]].b)
    waiting_for_fire = 0;*/
  }
   else
   {
    for (p = 0; p < 1; p ++)
    {
     if (p == 1 && arena.players == 1)
      continue;
     if (PP.control == CONTROL_KEY_A
      || PP.control == CONTROL_KEY_B)
     {
      kb = PP.control;
      if (!key [options.ckey [kb] [CKEY_FIRE1]])
       waiting_for_fire = 1;
     }
      else
      {
        poll_joystick();
        js = PP.control - CONTROL_JOY_A;
        if (!joy[js].button[options.joy_button [js] [JBUTTON_FIRE1]].b)
         waiting_for_fire = 1;
      }
    }
/*    if (!key [KEY_SPACE] && !key [KEY_ENTER] && !key [KEY_ENTER_PAD] && !key [KEY_Z] && !key [options.ckey [0] [CKEY_FIRE1]]
     && (!options.joystick_available [0] || !joy[0].button[options.joy_button [0] [0]].b))
      waiting_for_fire = 1;*/
   }

}


int bover_menu_select [2];
int bover_menu_select2 [2];

int get_bover_input(int p, int min, int max, int select2, int min2, int max2, int choosing);

int bover_flash_in;

int briefing_over_menu(void)
{



 bover_menu_select [0] = BOVER_MENU_START;
 bkey_wait [0] = 30;
 bkey_wait_pressed [0] = -1;
 int input_value = 0;
 bover_flash_in = 0;

 while (TRUE)
 {

  briefing_counter ++;
  if (bover_flash_in != -1)
   bover_flash_in ++;


  bdisplay();
  rectfill(display[0], 250+EDGE, 575+EDGE, 550-EDGE, 595-EDGE, BCOL_DARK);
//  rectfill(display[0], 0, 579, 800, 600, BCOL_BACK); // to remove "press fire 1 etc" text
  boverdisplay();
//  if (bover_flash_in == -1)
  input_value = get_bover_input(0, BOVER_MENU_START, BOVER_MENU_END - 1, 0, 0, 0, 0);
  if (input_value != -1)
   return bover_menu_select [0];

/*   if (key [options.ckey [0] [CKEY_FIRE2]]
    || (options.joystick_available [0] && joy[0].button[options.joy_button [0] [1]].b))
    break;*/

  if (finished_wship_process == 0)
  {
   finished_wship_process = wship_process();
   wship_process_indicator ++;
  }

    while (ticked == 0)
    {
        rest(1);
    };
    ticked = 0;

  wship_process_progress();

//  vsync();
  blit(display[0], screen, 0, 0, 0, 0, 800, 600);



 };

// briefing_over_menu();

}

// "skip" is only used for the main selection, and currently only used
//  to skip third weapon selection in 2-p games.
int get_bover_input(int p, int min, int max, int select2, int min2, int max2, int choosing)
{
 int move = 0;
 int hmove = 0;
 int select = 0;
 int js;
 int kb;

  char anykey = 0;

 if (PP.control == CONTROL_KEY_A
  || PP.control == CONTROL_KEY_B)
 {
  kb = PP.control;
  if (bkey_wait_pressed [p] != -1
   && key [options.ckey [kb] [bkey_wait_pressed [p]]])
   anykey = 1;

/*  for (i = CKEY_LEFT; i < CKEY_FIRE2 + 1; i ++)
  {
   if (key [options.ckey [kb] [i]])
   {
    anykey = 1;
   }
  }*/
 }


 if (anykey == 0 && player_joystick_input(p) == 0)
  bkey_wait [p] = 0;

 if (bkey_wait [p] > 0)
 {
  bkey_wait [p] --;
  if (bkey_wait [p] == 0)
   bkey_wait_pressed [p] = -1;
  return -1;
 }



     if (PP.control == CONTROL_KEY_A
      || PP.control == CONTROL_KEY_B)
     {
      kb = PP.control;
      if (key [options.ckey [kb] [CKEY_UP]])
      {
       move = -1;
       bkey_wait [p] = 10;
       bkey_wait_pressed [p] = CKEY_UP;
      }
      if (key [options.ckey [kb] [CKEY_DOWN]])
      {
       move = 1;
       bkey_wait [p] = 10;
       bkey_wait_pressed [p] = CKEY_DOWN;
      }
      if (key [options.ckey [kb] [CKEY_LEFT]])
      {
       hmove = -1;
       bkey_wait [p] = 10;
       bkey_wait_pressed [p] = CKEY_LEFT;
      }
      if (key [options.ckey [kb] [CKEY_RIGHT]])
      {
       hmove = 1;
       bkey_wait [p] = 10;
       bkey_wait_pressed [p] = CKEY_RIGHT;
      }
      if (key [options.ckey [kb] [CKEY_FIRE1]])
      {
       move = 0;
       select = 1;
       bkey_wait [p] = 10;
       bkey_wait_pressed [p] = CKEY_FIRE1;
      }

     }
      else
      {
        poll_joystick();
        js = PP.control - CONTROL_JOY_A;
        if (joy[js].button[options.joy_button [js] [JBUTTON_FIRE1]].b)
        {
         select = 1;
         move = 0;
         bkey_wait [p] = 10;
         bkey_wait_pressed [p] = -1;
        }
        if (joy[js].stick[0].axis[1].pos < -MENU_SENSE)
        {
         move = -1;
         bkey_wait [p] = 10;
         bkey_wait_pressed [p] = -1;
        }
        if (joy[js].stick[0].axis[1].pos > MENU_SENSE)
        {
         move = 1;
         bkey_wait [p] = 10;
         bkey_wait_pressed [p] = -1;
        }
        if (joy[js].stick[0].axis[0].pos < -MENU_SENSE)
        {
         hmove = -1;
         bkey_wait [p] = 10;
         bkey_wait_pressed [p] = -1;
        }
        if (joy[js].stick[0].axis[0].pos > MENU_SENSE)
        {
         hmove = 1;
         bkey_wait [p] = 10;
         bkey_wait_pressed [p] = -1;
        }
      }


 if (!select2)
 {
  if (move == -1)
  {
   do
   {
    bover_menu_select [p] --;
    if (bover_menu_select [p] < min)
     bover_menu_select [p] = max;
   }
    while (choosing == 1 && choose_valid(p, bover_menu_select [p]) != 2);
   play_basicwfv(WAV_SELECT0, FREQ_BSELECT, VOL_BSELECT1);
  }

  if (move == 1)
  {
   do
   {
   bover_menu_select [p] ++;
   if (bover_menu_select [p] > max)
    bover_menu_select [p] = min;
   }
    while (choosing == 1 && choose_valid(p, bover_menu_select [p]) != 2);
   play_basicwfv(WAV_SELECT0, FREQ_BSELECT, VOL_BSELECT1);
  }
 }
  else
  {
   if (move == -1)
   {
    bover_menu_select2 [p] --;
    if (bover_menu_select2 [p] < min2)
     bover_menu_select2 [p] = max2;
    play_basicwfv(WAV_SELECT0, FREQ_BSELECT, VOL_BSELECT1);
   }

   if (move == 1)
   {
    bover_menu_select2 [p] ++;
    if (bover_menu_select2 [p] > max2)
     bover_menu_select2 [p] = min2;
    play_basicwfv(WAV_SELECT0, FREQ_BSELECT, VOL_BSELECT1);
   }
  }
/*
 if (horiz)
 {
  if (hmove == -1)
  {
   bover_menu_select2 [p] --;
   if (bover_menu_select2 [p] < hmin)
    bover_menu_select2 [p] = hmax;
  }

  if (hmove == 1)
  {
   bover_menu_select2 [p] ++;
   if (bover_menu_select2 [p] > hmax)
    bover_menu_select2 [p] = hmin;
  }

  if (select)
   return 1;// bover_menu_select [p] + (bover_menu_select2 [p] * 100);

 }
*/
 if (select)
  return 1;//bover_menu_select [p];

 return -1;

}

void boverdisplay(void)
{

// clear_to_color(display [0], COL_STAR1);
//  leave map part of screen untouched

 int col = COL_BOX4;
 col -= (bover_flash_in >> 1);
 if (col < COL_BOX0)
 {
  col = COL_BOX0;
  bover_flash_in = -1;
 }

 if (bover_flash_in == -1)
  col = COL_BOX0;

 int col2 = col + 1;
 if (col2 > COL_BOX4)
  col2 = COL_BOX4;

 int col3 = col + 2;
 if (col3 > COL_BOX4)
  col3 = COL_BOX4;


 rectfill(display[0], BOX_X-EDGE, BOX_Y-EDGE, BOX_X + BOX_W+EDGE, BOX_Y + BOX_H+EDGE, col2);
 rectfill(display[0], BOX_X, BOX_Y, BOX_X + BOX_W, BOX_Y + BOX_H, col);

// rectfill(display[0], BOX_X + 5, BOX_Y + (BOX_H>>1) - 30 + (30*bover_menu_select [0]), BOX_X + BOX_W - 5, BOX_Y + (BOX_H>>1) - 5 + (30*bover_menu_select [0]), COL_F1 + TRANS_BLUE1);
// rect(display[0], BOX_X + 5, BOX_Y + (BOX_H>>1) - 30 + (30*bover_menu_select [0]), BOX_X + BOX_W - 5, BOX_Y + (BOX_H>>1) - 5 + (30*bover_menu_select [0]), COL_F2 + TRANS_BLUE2);

 int y = BOX_Y + 10;

 int i;
 int side = TEAM_FRIEND;

 for (i = 0; i < NO_BSHIP_TYPES; i ++)
 {
  if (mdata[i].number > 0)
  {
   if (mdata[i].side != side) // side change!
   {
    y += 30;
   }
   display_mdata(BOX_X + 5, y, i);
   y += 20;
  }
 }

 if (col < COL_BOX4)
 {

 y = 400;

 rectfill(display[0], BOX_X + 5, y - 5 + (30*bover_menu_select [0]), BOX_X + BOX_W - 5, y + 20 + (30*bover_menu_select [0]), col2);
 rect(display[0], BOX_X + 5, y - 5 + (30*bover_menu_select [0]), BOX_X + BOX_W - 5, y + 20 + (30*bover_menu_select [0]), col3);

 textprintf_centre_ex(display[0], small_font, BOX_X + (BOX_W>>1), y, BCOL_TEXT, -1, "prepare to launch >>>");
 textprintf_centre_ex(display[0], small_font, BOX_X + (BOX_W>>1), y + 30, BCOL_TEXT, -1, "replay briefing");
 textprintf_centre_ex(display[0], small_font, BOX_X + (BOX_W>>1), y + 60, BCOL_TEXT, -1, "mission data");
 textprintf_centre_ex(display[0], small_font, BOX_X + (BOX_W>>1), y + 90, BCOL_TEXT, -1, "quit");
 }

}

void display_mdata(int x, int y, int btype)
{
    if (mdata[btype].side == TEAM_FRIEND) {
        draw_sprite(display[0], bsprite [mdata[btype].side] [get_bsprite(btype)] [0], x, y);
    } else {
        draw_sprite_h_flip(display[0], bsprite [mdata[btype].side] [get_bsprite(btype)] [0], x, y);
    }

    textprintf_ex(display[0], small_font, x + 15, y, BCOL_TEXT, -1, "(%i)", mdata[btype].number);

    char sname [30];

    switch(btype) {
        case BSHIP_OLD2: strcpy(sname, "CTBR-LC SUNSHARK"); break;
        case BSHIP_OLD3: strcpy(sname, "CTBR-BC STARWHALE"); break;
        case BSHIP_TRIREME:
        case BSHIP_FRIEND3: strcpy(sname, "IF-HC TRIREME"); break;
        case BSHIP_DROM: strcpy(sname, "CTBR-T DROMEDARY"); break;
        case BSHIP_LINER: strcpy(sname, "STARLINER"); break;
        case BSHIP_FIGHTER_FRIEND: strcpy(sname, "CTBR-SANDFLY"); break;
        case BSHIP_FIGHTER_FSTRIKE: strcpy(sname, "CTBR-RAM"); break;
        case BSHIP_MOTH: strcpy(sname, "CTBR-ANGRY MOTH"); break;

        case BSHIP_FIGHTER: strcpy(sname, "FSF-CALLED"); break;
        case BSHIP_SCOUT2: strcpy(sname, "FSF-RIGHTEOUS"); break;
        case BSHIP_SCOUT3: strcpy(sname, "FSF-MERCIFUL"); break;
        case BSHIP_SCOUTCAR: strcpy(sname, "FSF-GRACEFUL"); break;
        case BSHIP_ECARRIER: strcpy(sname, "FSF-INFINITE"); break;
        case BSHIP_FREIGHT: strcpy(sname, "FSF-BENEVOLENT"); break;
        case BSHIP_BOMBER: strcpy(sname, "FSF-MESSENGER"); break;
        case BSHIP_EBASE: strcpy(sname, "FSF ORBITAL BASE"); break;
        case BSHIP_EINT: strcpy(sname, "FSF-CHOSEN"); break;
        case BSHIP_ESCOUT: strcpy(sname, "FSF-DISCIPLE"); break;
        default:
        case BSHIP_UNKNOWN: strcpy(sname, "UNKNOWN SHIP"); break;
    }

    textprintf_ex(display[0], small_font, x + 35, y, BCOL_TEXT, -1, "%s", sname);
}


int pressing_a_key(int p, int ckeyp, int cbuttonp)
{

    int kb;

    if (PP.control == CONTROL_KEY_A
     || PP.control == CONTROL_KEY_B)
    {
        kb = PP.control;
        if (key [options.ckey [kb] [ckeyp]])
         return 1;
          else
           return 0;
    }

    int js = PP.control - CONTROL_JOY_A;

    poll_joystick();

    if (joy[js].button[options.joy_button [js] [cbuttonp]].b)
     return 1;

    return 0;

}

char ask_bover_quit(void)
{
 vsync();
 rectfill(display[0], 250, 200, 550, 310, TRANS_RED1);
 rect(display[0], 250, 200, 550, 310, COL_EBOX3);
 textprintf_centre_ex(display[0], small_font, 400, 230, COL_EBOX4, -1, "QUIT GAME?");
 textprintf_centre_ex(display[0], small_font, 400, 260, COL_EBOX3, -1, "press 'Y' to quit to main menu");
 textprintf_centre_ex(display[0], small_font, 400, 275, COL_EBOX3, -1, "or 'N' to play on");

 wship_process_progress();

 blit(display[0], screen, 0, 0, 0, 0, 800, 600);

 bkey_wait [0] = 10; // for when it returns

 unsigned char pc = 0;

 do
 {
    pc ++;
    pc &= 255;

    if (key [KEY_Y])
     return 1;
    if (key [KEY_N])
    {
     play_basicwfv(WAV_SELECT0, FREQ_BSELECT2, VOL_BSELECT1);
     return 0;
    }

  if (finished_wship_process == 0)
  {
   finished_wship_process = wship_process();
   wship_process_indicator ++;
  }

    while (ticked == 0)
    {
        rest(1);
    };

    ticked = 0;

/*
  for (i = 0; i < 4; i ++)
  {
   col = ((pc + i * 8)/8)%4;
   switch(col)
   {
    case 3: col = COL_E1 + TRANS_RED1; break;
    case 2: col = COL_E1 + TRANS_RED2; break;
    case 1: col = COL_E2 + TRANS_RED3; break;
    case 0: col = COL_E3 + TRANS_RED4; break;
   }
   textprintf_centre_ex(display[0], small_font, 400 - 40 - i*3, 230, col, -1, ">");
   textprintf_centre_ex(display[0], small_font, 400 + 40 + i*3, 230, col, -1, "<");
  }
*/
  draw_arrows(400, 230, 40, 0, pc);

  wship_process_progress();

  blit(display[0], screen, 0, 0, 0, 0, 800, 600);

 } while (TRUE);

}

void draw_arrows(int x, int y, int w, int basecol, int counter)
{

 int i, col;

  for (i = 0; i < 4; i ++)
  {
   col = ((counter + i * 8)/8)%4;
   switch(basecol)
   {
    case 0:
     switch(col)
     {
      case 3: col = COL_EBOX1; break;
      case 2: col = COL_EBOX2; break;
      case 1: col = COL_EBOX3; break;
      case 0: col = COL_EBOX4; break;
     }
     break;
    case 2:
     switch(col)
     {
      case 3: col = COL_BOX1; break;
      case 2: col = COL_BOX2; break;
      case 1: col = COL_BOX3; break;
      case 0: col = COL_BOX4; break;
     }
     break;
   }
   textprintf_centre_ex(display[0], small_font, x - w - i*3, y, col, -1, ">");
   textprintf_centre_ex(display[0], small_font, x + w + i*3, y, col, -1, "<");
  }


}

//#define WPNS 7


void bover_box(int x, int y);

int choose_weapons(void)
{

// char ckey_wait [2] = {10, 10};
/* player[0].weapon_type [0] = WPN_AF_MISSILE;
 player[0].weapon_type [1] = WPN_AWS_MISSILE;
 player[1].weapon_type [0] = WPN_AF_MISSILE;
 player[1].weapon_type [1] = WPN_AWS_MISSILE;

 if (arena.players == 2)
 {
  player[0].weapon_type [1] = WPN_NONE;
  player[1].weapon_type [0] = WPN_AWS_MISSILE;
  player[1].weapon_type [1] = WPN_NONE;
 }
*/
 char finished [2] = {0,0};
 int ret1;
// int wpn_swap;
 int w, p;

 bover_menu_select [0] = CHOOSE_LAUNCH;
 bover_menu_select2 [0] = 0;
 bover_menu_select [1] = CHOOSE_LAUNCH;
 bover_menu_select2 [1] = 0;
 selecting [0] = 0;
 selecting [1] = 0;

 int choices2 = br_ship_choices - 1; // This is updated if not choosing ships - see the switch just below

 do
 {

  briefing_counter ++;


    choose_display(finished [0], finished [1]);


   for (p = 0; p < 2; p ++)
   {

    if (finished [p] != 0)
     continue;

    if (p == 1 && arena.players != 2)
     continue;
//    {
//     if (arena.players == 1)
//        )

       switch(bover_menu_select [p])
       {
           case CHOOSE_SHIP:
           case CHOOSE_ESCORT1:
           case CHOOSE_ESCORT2:
            choices2 = br_ship_choices - 1;
            break;
           case CHOOSE_WING1:
           case CHOOSE_WING2:
            choices2 = 1; // actually means 2
            break;
           case CHOOSE_VARIANT:
            choices2 = br_variant_choices - 1;
            break;
       }

      ret1 = get_bover_input(p, 0, CHOOSE_REPLAY, selecting [p], 0, choices2, 1);
// was this one

//       else
//        ret1 = get_bover_input(0, 0, CHOOSE_REPLAY, selecting [0], 0, WPNS - 1, CHOOSE_WEAPON2);
     if (ret1 != -1)
     {
      if (bover_menu_select [p] == CHOOSE_REPLAY)
       return 0;
      if (bover_menu_select [p] == CHOOSE_LAUNCH)
      {
       finished [p] = 1;
       play_basicwfv(WAV_SELECT1, FREQ_BSELECT1, VOL_BSELECT1);
      }
       else
       {
        if (bover_menu_select [p] == CHOOSE_SHIP)
         {
          if (selecting [p])
          {
           PP.type = ship_list [bover_menu_select2 [p]];
           set_player_ship_type(p, PP.type, CVAR_NONE);
           selecting [p] = 0;
           play_basicwfv(WAV_SELECT1, FREQ_BSELECT1, VOL_BSELECT1);
          }
           else
            {
             selecting [p] = 1;
             bover_menu_select2 [p] = 0; //player[0].weapon_type [bover_menu_select [0] - CHOOSE_WEAPON1];
             while (PP.type != ship_list [bover_menu_select2 [p]])
             {
              bover_menu_select2 [p]++;
             }
             play_basicwfv(WAV_SELECT1, FREQ_BSELECT1, VOL_BSELECT1);
            }
         }

        if (bover_menu_select [p] == CHOOSE_VARIANT)
         {
          if (selecting [p])
          {
           PP.variant = variant_list [p] [bover_menu_select2 [p]];
           set_player_ship_type(p, PP.type, PP.variant);
           selecting [p] = 0;
           play_basicwfv(WAV_SELECT1, FREQ_BSELECT1, VOL_BSELECT1);
          }
           else
            {
             selecting [p] = 1;
             bover_menu_select2 [p] = 0; //player[0].weapon_type [bover_menu_select [0] - CHOOSE_WEAPON1];
             while (PP.variant != variant_list [p] [bover_menu_select2 [p]])
             {
              bover_menu_select2 [p]++;
             }
             play_basicwfv(WAV_SELECT1, FREQ_BSELECT1, VOL_BSELECT1);
            }
         }


// Only player 0 has CHOOSE_ESCORT entries: (they aren't valid for p2)
        if (bover_menu_select [p] == CHOOSE_ESCORT1
         || bover_menu_select [p] == CHOOSE_ESCORT2)
         {
          w = bover_menu_select [p] - CHOOSE_ESCORT1; // will be 0 or 1
          if (selecting [p])
          {
           PP.escort_type [w] = ship_list [bover_menu_select2 [p]];
           selecting [p] = 0;
           play_basicwfv(WAV_SELECT1, FREQ_BSELECT1, VOL_BSELECT1);
          }
           else
            {
             selecting [p] = 1;
             bover_menu_select2 [p] = 0; //player[0].weapon_type [bover_menu_select [0] - CHOOSE_WEAPON1];
             while (PP.escort_type [w] != ship_list [bover_menu_select2 [p]])
             {
              bover_menu_select2 [p]++;
             }
             play_basicwfv(WAV_SELECT1, FREQ_BSELECT1, VOL_BSELECT1);
            }
         }

        if (bover_menu_select [p] == CHOOSE_WING1
         || bover_menu_select [p] == CHOOSE_WING2)
         {
          w = bover_menu_select [p] - CHOOSE_WING1; // will be 0 or 1
          if (selecting [p])
          {
           PP.wing_type [w] = bover_menu_select2 [p];
           selecting [p] = 0;
           play_basicwfv(WAV_SELECT1, FREQ_BSELECT1, VOL_BSELECT1);
          }
           else
            {
             selecting [p] = 1;
             bover_menu_select2 [p] = PP.wing_type [w];
             play_basicwfv(WAV_SELECT1, FREQ_BSELECT1, VOL_BSELECT1);
            }
         }

       }
     }

 }

/*
    if (finished [0] == 0)
    {
//     if (arena.players == 1)


      ret1 = get_bover_input(0, 0, CHOOSE_REPLAY, selecting [0], 0, br_ship_choices-1, 1);
// was this one

//       else
//        ret1 = get_bover_input(0, 0, CHOOSE_REPLAY, selecting [0], 0, WPNS - 1, CHOOSE_WEAPON2);
     if (ret1 != -1)
     {
      if (bover_menu_select [0] == CHOOSE_REPLAY)
       return 0;
      if (bover_menu_select [0] == CHOOSE_LAUNCH)
      {
       finished [0] = 1;
       play_basicwfv(WAV_SELECT1, FREQ_BSELECT1, VOL_BSELECT1);
      }
       else
       {
        if (bover_menu_select [0] == CHOOSE_SHIP)
         {
          if (selecting [0])
          {
           player[0].type = ship_list [bover_menu_select2 [0]];
           set_player_ship_type(0, player[0].type);
           selecting [0] = 0;
           play_basicwfv(WAV_SELECT1, FREQ_BSELECT1, VOL_BSELECT1);
          }
           else
            {
             selecting [0] = 1;
             bover_menu_select2 [0] = 0; //player[0].weapon_type [bover_menu_select [0] - CHOOSE_WEAPON1];
             while (player[0].type != ship_list [bover_menu_select2 [0]])
             {
              bover_menu_select2 [0]++;
             }
             play_basicwfv(WAV_SELECT1, FREQ_BSELECT1, VOL_BSELECT1);
            }
         }

// Escorts only need to be done for p == 0 as only player 1 can set them:
        if (bover_menu_select [0] == CHOOSE_ESCORT1
         || bover_menu_select [0] == CHOOSE_ESCORT2)
         {
          w = bover_menu_select [0] - CHOOSE_ESCORT1; // will be 0 or 1
          if (selecting [0])
          {
           player[0].escort_type [w] = ship_list [bover_menu_select2 [0]];
           selecting [0] = 0;
           play_basicwfv(WAV_SELECT1, FREQ_BSELECT1, VOL_BSELECT1);
          }
           else
            {
             selecting [0] = 1;
             bover_menu_select2 [0] = 0; //player[0].weapon_type [bover_menu_select [0] - CHOOSE_WEAPON1];
             while (player[0].escort_type [w] != ship_list [bover_menu_select2 [0]])
             {
              bover_menu_select2 [0]++;
             }
             play_basicwfv(WAV_SELECT1, FREQ_BSELECT1, VOL_BSELECT1);
            }
         }


       }
     }
    }

    if (arena.players == 2 && finished [1] == 0)
    {
     ret1 = get_bover_input(1, 0, CHOOSE_REPLAY, selecting [1], 0, WPNS - 1, 1);
//     ret1 = get_bover_input(1, 0, CHOOSE_REPLAY, selecting [1], 0, WPNS - 1, -1);
     if (ret1 != -1)
     {
      if (bover_menu_select [1] == CHOOSE_REPLAY)
       return 0;
      if (bover_menu_select [1] == CHOOSE_LAUNCH)
      {
       finished [1] = 1;
      }
       else
       {
        if (bover_menu_select [1] == CHOOSE_WEAPON1
         || bover_menu_select [1] == CHOOSE_WEAPON2)
         {
          if (selecting [1])
          {
           w = bover_menu_select [1] - CHOOSE_WEAPON1;
           if (player[1].weapon_type [w^1] == bover_menu_select2 [1])
           {
            player[1].weapon_type [w^1] = player[1].weapon_type [w];
           }
           player[1].weapon_type [w] = bover_menu_select2 [1];
           selecting [1] = 0;
           play_basicwfv(WAV_SELECT1, FREQ_BSELECT1, VOL_BSELECT1);
          }
           else
            {
             selecting [1] = 1;
             bover_menu_select2 [1] = player[1].weapon_type [bover_menu_select [1] - CHOOSE_WEAPON1];
             play_basicwfv(WAV_SELECT1, FREQ_BSELECT1, VOL_BSELECT1);
            }
         }

       }
     }
    }
*/
    if (finished [0] == 1 && (arena.players == 1 || finished [1] == 1))
     return 1;



  if (finished_wship_process == 0)
  {
   finished_wship_process = wship_process();
   wship_process_indicator ++;
  }

    while (ticked == 0)
    {
        rest(1);
    };

    ticked = 0;

  wship_process_progress();

//  vsync();
  blit(display[0], screen, 0, 0, 0, 0, 800, 600);

 } while (TRUE);



}


#define LSPACE1 30
#define LSPACE2 17
#define X_INDENT 15
#define WBOX_Y 480

extern struct BMP_STRUCT fighter_sprite [FIGHTER_SPRITES] [FIGHTER_ROTATIONS];
extern struct BMP_STRUCT player_sprite [PLAYER_SPRITES] [PLAYER_ROTATIONS];
void print_weapon_name(int x, int y, int col, int w, int number);
void print_ship_name(int x, int y, int col, int type);
void display_ship_choices(int p, int x, int y);
void print_variant_name(int x, int y, int col, int type, int variant);


enum
{
BOX_NONE,
BOX_SHIP,
BOX_VARIANT,
BOX_WING
};


void choose_display(int finished1, int finished2)
{

 clear_to_color(display[0], BCOL_DARK);

 int p;

 int x, y, i;
// int w;
// int w2;
 int sprite;

 int box_display;
 int box_y;


 clear_to_color(display[0], COL_BOX0);


 for (p = 0; p < arena.players; p ++)
 {

  box_display = BOX_NONE;

  y = 130;

  if (arena.players == 1)
   x = 300;
    else
     x = 40 + p * 400;

 sprite = get_ship_sprite(PP.type);
 draw_sprite(display[0], fighter_sprite [sprite] [0].sprite, x + 100 - (fighter_sprite [sprite] [0].sprite->w / 2), 50);
 sprite = get_ship_sprite(player[0].escort_type [PP.wing_type [0]]);
 draw_sprite(display[0], fighter_sprite [sprite] [0].sprite, x + 100 - 40 - (fighter_sprite [sprite] [0].sprite->w / 2), 80);
 draw_sprite(display[0], fighter_sprite [sprite] [0].sprite, x + 100 + 40 - (fighter_sprite [sprite] [0].sprite->w / 2), 80);
// draw second wing if relevant:

   if ((p == 0 && finished1)
    || (p == 1 && finished2))
   {
    rectfill(display[0], x, y, x + 200, y + 420, BCOL_SCREEN);
    rect(display[0], x, y, x + 200, y + 420, BCOL_EDGE);
//    rectfill(display[0], x + 3, y + 3, x + 200 - 3, y + 420 - 3, BCOL_SCREEN);
    textprintf_centre_ex(display[0], small_font, x + 100, y + 100, BCOL_TEXT, -1, "waiting...");
    continue;
   }




  textprintf_ex(display[0], small_font, x, y, BCOL_TEXT, -1, "Angry Moth %i", p+1);
/*
  y += LSPACE1;
  textprintf_ex(display[0], small_font, x, y, BCOL_TEXT, -1, "Command ship");
  y += LSPACE2;
  if (bover_menu_select [p] == CHOOSE_SHIP)
  {
   bover_box(x, y);
//   weapon_box(p, BMSG_CANNON, x, WBOX_Y, x - 5, y + 6, -1);
  }
  print_ship_name(x + X_INDENT, y, COL_BOX3, PP.type);
*/





   y += LSPACE2;
   if (bover_menu_select [p] == CHOOSE_SHIP)
   {
    display_ship_choices(p, x, y);
   }
   print_ship_name(x + X_INDENT, y, COL_BOX3, PP.type);
 // }










  y += LSPACE1;
  textprintf_ex(display[0], small_font, x, y, BCOL_TEXT, -1, "Variant");
  y += LSPACE2;
  if (bover_menu_select [p] == CHOOSE_VARIANT)
  {
   bover_box(x, y);
   print_variant_name(x + X_INDENT, y, COL_BOX3, PP.type, PP.variant);
   display_variant_choices(p, x, y);
  }
   else
    print_variant_name(x + X_INDENT, y, COL_BOX3, PP.type, PP.variant);



  y += LSPACE1;
  textprintf_ex(display[0], small_font, x, y, BCOL_TEXT, -1, "Weapons");
  y += LSPACE2;
  if (bover_menu_select [p] == CHOOSE_WEAPON1)
  {
   bover_box(x, y);
  }
  textprintf_ex(display[0], small_font, x + X_INDENT, y, COL_BOX3, -1, "1 - Autocannon");


  for (i = 0; i < 2; i ++)
  {
   y += LSPACE2;
   if (choose_valid(p, CHOOSE_WEAPON2 + i) < 1)
   {
    textprintf_ex(display[0], small_font, x + X_INDENT, y, COL_BOX1, -1, "%i - None", i + 2);
    continue;
   }
   if (bover_menu_select [p] == CHOOSE_WEAPON2 + i)
   {
     bover_box(x, y);
//   weapon_box(p, BMSG_CANNON, x, WBOX_Y, x - 5, y + 6, -1);
   }
   if (PP.weapon_type [i] == WPN_NONE)
    textprintf_ex(display[0], small_font, x + X_INDENT, y, COL_BOX1, -1, "%i - None", i + 2);
     else
      print_weapon_name(x + X_INDENT, y, COL_BOX3, PP.weapon_type [i], i + 2);

  }

  if (p == 0)
  {
   y += LSPACE1;
   textprintf_ex(display[0], small_font, x, y, BCOL_TEXT, -1, "Fleet fighter escort");
   for (i = 0; i < 2; i ++)
   {
    y += LSPACE2;
    if (choose_valid(p, CHOOSE_ESCORT1 + i) == 0)
     continue;
    if (bover_menu_select [p] == CHOOSE_ESCORT1 + i)
    {
     bover_box(x, y);
     display_ship_choices(p, x, y);
//   weapon_box(p, BMSG_CANNON, x, WBOX_Y, x - 5, y + 6, -1);
    }
    print_ship_name(x + X_INDENT, y, COL_BOX3, player[0].escort_type [i]);
   }
  }
   else
    y += LSPACE1 + LSPACE2 + LSPACE2;

  y += LSPACE1;
  textprintf_ex(display[0], small_font, x, y, BCOL_TEXT, -1, "Command wing");
  for (i = 0; i < 2; i ++)
  {
   y += LSPACE2;
   if (choose_valid(p, CHOOSE_WING1 + i) == 0)
    continue;
   if (bover_menu_select [p] == CHOOSE_WING1 + i)
   {
     bover_box(x, y);
     display_wing_choices(p, x, y);
//   weapon_box(p, BMSG_CANNON, x, WBOX_Y, x - 5, y + 6, -1);
   }
//   textprintf_ex(display[0], small_font, x + X_INDENT, y, COL_BOX1, -1, "None");
    print_ship_name(x + X_INDENT, y, COL_BOX3, player[0].escort_type [PP.wing_type [i]]);
//   textprintf_ex(display[0], small_font, x + X_INDENT, y, COL_BOX1, -1, "None");
  }



/*
  y += LSPACE1;
  textprintf_ex(display[0], small_font, x, y, COL_BOX2, -1, "weapons");
  for (w = 0; w < 2; w ++)
  {
   if (arena.players == 2 && w == 1)
    break;
   y += LSPACE2;
   if (bover_menu_select [p] == CHOOSE_WEAPON1 + w)
   {
    bover_box(x, y);
    rectfill(display[0], x + 140, y - 5, x + 300, y + 10 + WPNS * 20, COL_STAR1);
    rect(display[0], x + 140, y - 5, x + 300, y + 10 + WPNS * 20, COL_BOX2);
    for (w2 = 0; w2 < WPNS; w2 ++)
    {
//     if (bover_menu_select2 [p] == w2)//PP.weapon_type [w] == w2)
     if ((selecting [p] == 0 && PP.weapon_type [w] == w2)
     || (selecting [p] == 1 && bover_menu_select2 [p] == w2))
     {
      col = COL_BOX0;
      if (selecting [p])
       col = COL_BOX1;
      rectfill(display[0], x + 140, y + 3 + (w2 * 20), x + 300, y + 20 + (w2 * 20), col);
      rect(display[0], x + 140, y + 3 + (w2 * 20), x + 300, y + 20 + (w2 * 20), col + 1);
      weapon_box(p, BMSG_WPN1 + w2, x, WBOX_Y, x + 301, y + 12 + (w2 * 20), 1);
//      textprintf_ex(display[0], small_font, x + 145, y + 5 + (w2 * 20), COL_BOX3, -1, "<");
//      textprintf_right_ex(display[0], small_font, x + 295, y + 5 + (w2 * 20), COL_BOX3, -1, ">");
     }
     print_weapon_name(x + 155, y + 5 + (w2 * 20), COL_BOX4, w2);
    }
   }
   print_weapon_name(x + 20, y, COL_BOX4, PP.weapon_type [w]);
  }
*/
  y += LSPACE1;
  y += LSPACE1;
  if (bover_menu_select [p] == CHOOSE_LAUNCH)
   bover_box(x, y);
  textprintf_ex(display[0], small_font, x + X_INDENT, y, COL_BOX4, -1, "launch >>");

  y += LSPACE1;
  if (bover_menu_select [p] == CHOOSE_REPLAY)
   bover_box(x, y);
  textprintf_ex(display[0], small_font, x + X_INDENT - text_length(small_font, "<< "), y, COL_BOX4, -1, "<< replay briefing");

 }


}

// returns: 0 = not valid at all, 1 = can be seen but not selected, 2 = can be seen and selected
int choose_valid(int p, int ch)
{

 switch(ch)
 {
  case CHOOSE_SHIP:
   return 2;
  case CHOOSE_VARIANT:
   return 2;
  case CHOOSE_MOD1:
  case CHOOSE_MOD2:
  case CHOOSE_MOD3:
   return 0;
  case CHOOSE_WEAPON1:
   return 2;
  case CHOOSE_WEAPON2:
   if (PP.weapon_type [0] == WPN_NONE)
    return 1;
   return 2;
  case CHOOSE_WEAPON3:
   if (PP.weapon_type [1] == WPN_NONE)
    return 1;
   return 2;
  case CHOOSE_ESCORT1:
  case CHOOSE_ESCORT2:
   if (p == 1)
    return 0;
   return 2;
  case CHOOSE_WING1:
   return 2;
  case CHOOSE_WING2:
   if (PP.wings == 2)
    return 2;
   return 0;
  default: return 2;
 }



}

void display_ship_choices(int p, int x, int y)
{

    int w2, col;

    bover_box(x, y);
    rectfill(display[0], x + 140, y - 5, x + 300, y + 10 + br_ship_choices * 20, COL_STAR1);
    rect(display[0], x + 140, y - 5, x + 300, y + 10 + br_ship_choices * 20, COL_BOX2);
    for (w2 = 0; w2 < br_ship_choices; w2 ++)
    {
     if ((selecting [p] == 0 && PP.type == w2)
     || (selecting [p] == 1 && bover_menu_select2 [p] == w2))
     {
      col = COL_BOX0;
      if (selecting [p])
       col = COL_BOX1;
      rectfill(display[0], x + 140, y + 3 + (w2 * 20), x + 300, y + 20 + (w2 * 20), col);
      rect(display[0], x + 140, y + 3 + (w2 * 20), x + 300, y + 20 + (w2 * 20), col + 1);
//      weapon_box(p, BMSG_WPN1 + w2, x, WBOX_Y, x + 301, y + 12 + (w2 * 20), 1);
//      textprintf_ex(display[0], small_font, x + 145, y + 5 + (w2 * 20), COL_BOX3, -1, "<");
//      textprintf_right_ex(display[0], small_font, x + 295, y + 5 + (w2 * 20), COL_BOX3, -1, ">");
     }
     print_ship_name(x + 155, y + 5 + (w2 * 20), COL_BOX3, ship_list [w2]);
    }

}


#define WING_CHOICES 2

void display_wing_choices(int p, int x, int y)
{

    int w2, col;

    bover_box(x, y);
    rectfill(display[0], x + 140, y - 5, x + 300, y + 10 + WING_CHOICES * 20, COL_STAR1);
    rect(display[0], x + 140, y - 5, x + 300, y + 10 + WING_CHOICES * 20, COL_BOX2);
    for (w2 = 0; w2 < WING_CHOICES; w2 ++)
    {
     if ((selecting [p] == 0 && PP.type == w2)
     || (selecting [p] == 1 && bover_menu_select2 [p] == w2))
     {
      col = COL_BOX0;
      if (selecting [p])
       col = COL_BOX1;
      rectfill(display[0], x + 140, y + 3 + (w2 * 20), x + 300, y + 20 + (w2 * 20), col);
      rect(display[0], x + 140, y + 3 + (w2 * 20), x + 300, y + 20 + (w2 * 20), col + 1);
     }
     print_ship_name(x + 155, y + 5 + (w2 * 20), COL_BOX3, player [0].escort_type [w2]);
    }

}



void print_ship_name(int x, int y, int col, int type)
{

 switch(type)
 {
  case SHIP_FIGHTER_FRIEND:
   textprintf_ex(display[0], small_font, x, y, col, -1, "Sandfly");
   break;
  case SHIP_FSTRIKE:
   textprintf_ex(display[0], small_font, x, y, col, -1, "Ram");
   break;
  case SHIP_LACEWING:
   textprintf_ex(display[0], small_font, x, y, col, -1, "Lacewing");
   break;
  case SHIP_MONARCH:
   textprintf_ex(display[0], small_font, x, y, col, -1, "Monarch");
   break;
  case SHIP_IBEX:
   textprintf_ex(display[0], small_font, x, y, col, -1, "Ibex");
   break;
  case SHIP_AUROCHS:
   textprintf_ex(display[0], small_font, x, y, col, -1, "Aurochs");
   break;
  default:
   textprintf_ex(display[0], small_font, x, y, col, -1, "Unknown");
   break;
 }



}



void display_variant_choices(int p, int x, int y)
{

    int w2, col;

//    bover_box(x, y);
    rectfill(display[0], x + 140, y - 5, x + 300, y + 10 + br_variant_choices * 20, COL_STAR1);
    rect(display[0], x + 140, y - 5, x + 300, y + 10 + br_variant_choices * 20, COL_BOX2);
    for (w2 = 0; w2 < br_variant_choices; w2 ++)
    {
     if ((selecting [p] == 0 && PP.type == w2)
     || (selecting [p] == 1 && bover_menu_select2 [p] == w2))
     {
      col = COL_BOX0;
      if (selecting [p])
       col = COL_BOX1;
      rectfill(display[0], x + 140, y + 3 + (w2 * 20), x + 300, y + 20 + (w2 * 20), col);
      rect(display[0], x + 140, y + 3 + (w2 * 20), x + 300, y + 20 + (w2 * 20), col + 1);
//      weapon_box(p, BMSG_WPN1 + w2, x, WBOX_Y, x + 301, y + 12 + (w2 * 20), 1);
//      textprintf_ex(display[0], small_font, x + 145, y + 5 + (w2 * 20), COL_BOX3, -1, "<");
//      textprintf_right_ex(display[0], small_font, x + 295, y + 5 + (w2 * 20), COL_BOX3, -1, ">");
     }
     print_variant_name(x + 155, y + 5 + (w2 * 20), COL_BOX3, PP.type, variant_list [p] [w2]);
    }

}


void print_variant_name(int x, int y, int col, int type, int variant)
{
    char sname [30] = "error";

    switch(variant) {
        case CVAR_NONE: // basic type
            switch(type) {
                case SHIP_FIGHTER_FRIEND: strcpy(sname, "Sandfly"); break;
                case SHIP_FSTRIKE: strcpy(sname, "Ram"); break;
                case SHIP_LACEWING: strcpy(sname, "Lacewing"); break;
                case SHIP_MONARCH: strcpy(sname, "Monarch"); break;
                case SHIP_IBEX: strcpy(sname, "Ibex"); break;
                case SHIP_AUROCHS: strcpy(sname, "Aurochs"); break;
                default: strcpy(sname, "Unknown"); break;
            }
            strcat(sname, "   ");
            textprintf_ex(display[0], small_font, x, y, col, -1, "%s", sname);
            textprintf_ex(display[0], small_font, x + text_length(small_font, sname), y, col-1, -1, "<standard>");
            break;
        case CVAR_RAM_HEAVY:
            strcpy(sname, "HellRam   ");
            textprintf_ex(display[0], small_font, x, y, col, -1, "%s", sname);
            break;
        case CVAR_LW_B:
            strcpy(sname, "Hungry Lacewing   ");
            textprintf_ex(display[0], small_font, x, y, col, -1, "%s", sname);
            break;
        case CVAR_LW_C:
            strcpy(sname, "Furious Lacewing   ");
            textprintf_ex(display[0], small_font, x, y, col, -1, "%s", sname);
            break;
        case CVAR_MONARCH_B:
            strcpy(sname, "Hungry Monarch   ");
            textprintf_ex(display[0], small_font, x, y, col, -1, "%s", sname);
            break;
        case CVAR_MONARCH_C:
            strcpy(sname, "Wise Monarch   ");
            textprintf_ex(display[0], small_font, x, y, col, -1, "%s", sname);
            break;
        case CVAR_IBEX_B:
            strcpy(sname, "Golden Ibex   ");
            textprintf_ex(display[0], small_font, x, y, col, -1, "%s", sname);
            break;
        case CVAR_IBEX_C:
            strcpy(sname, "Black Ibex   ");
            textprintf_ex(display[0], small_font, x, y, col, -1, "%s", sname);
            break;
        default:
            textprintf_ex(display[0], small_font, x, y, col, -1, "%s", "Unknown");
            break;
    }
}


/*


void choose_display(int finished1, int finished2)
{

 clear_to_color(display[0], BCOL_DARK);

 int p;

 int x, y, col;
 int w;
 int w2;

 clear_to_color(display[0], COL_BOX0);


 for (p = 0; p < arena.players; p ++)
 {

  y = 130;

  if (arena.players == 1)
   x = 300;
    else
     x = 40 + p * 400;

 draw_sprite(display[0], player_sprite [0] [0].sprite, x + 100 - (player_sprite [0] [0].sprite->w / 2), 50);
 draw_sprite(display[0], fighter_sprite [FIGHTER_SPRITE_FRIEND_1] [0].sprite, x + 100 - 40 - (fighter_sprite [FIGHTER_SPRITE_FRIEND_1] [0].sprite->w / 2), 80);
 draw_sprite(display[0], fighter_sprite [FIGHTER_SPRITE_FRIEND_1] [0].sprite, x + 100 + 40 - (fighter_sprite [FIGHTER_SPRITE_FRIEND_1] [0].sprite->w / 2), 80);


   if ((p == 0 && finished1)
    || (p == 1 && finished2))
   {
    rectfill(display[0], x, y, x + 200, y + 420, BCOL_SCREEN);
    rect(display[0], x, y, x + 200, y + 420, BCOL_EDGE);
//    rectfill(display[0], x + 3, y + 3, x + 200 - 3, y + 420 - 3, BCOL_SCREEN);
    textprintf_centre_ex(display[0], small_font, x + 100, y + 100, BCOL_TEXT, -1, "waiting...");
    continue;
   }


  textprintf_ex(display[0], small_font, x, y, BCOL_TEXT, -1, "Angry Moth %i", p+1);

  y += LSPACE1;
  textprintf_ex(display[0], small_font, x, y, COL_BOX2, -1, "basic weapon");
  y += LSPACE2;
  if (bover_menu_select [p] == CHOOSE_CANNON)
  {
   bover_box(x, y);
   weapon_box(p, BMSG_CANNON, x, WBOX_Y, x - 5, y + 6, -1);
  }
  textprintf_ex(display[0], small_font, x + 20, y, COL_BOX4, -1, "double autocannon");

  y += LSPACE1;
  textprintf_ex(display[0], small_font, x, y, COL_BOX2, -1, "squadron");
  y += LSPACE2;
  if (bover_menu_select [p] == CHOOSE_WING1)
  {
   bover_box(x, y);
   weapon_box(p, BMSG_FIGHTER, x, WBOX_Y, x - 5, y + 6, -1);
  }
  textprintf_ex(display[0], small_font, x + 20, y, COL_BOX4, -1, "fighter");

  y += LSPACE1;
  textprintf_ex(display[0], small_font, x, y, COL_BOX2, -1, "weapons");
  for (w = 0; w < 2; w ++)
  {
   if (arena.players == 2 && w == 1)
    break;
   y += LSPACE2;
   if (bover_menu_select [p] == CHOOSE_WEAPON1 + w)
   {
    bover_box(x, y);
    rectfill(display[0], x + 140, y - 5, x + 300, y + 10 + WPNS * 20, COL_STAR1);
    rect(display[0], x + 140, y - 5, x + 300, y + 10 + WPNS * 20, COL_BOX2);
    for (w2 = 0; w2 < WPNS; w2 ++)
    {
//     if (bover_menu_select2 [p] == w2)//PP.weapon_type [w] == w2)
     if ((selecting [p] == 0 && PP.weapon_type [w] == w2)
     || (selecting [p] == 1 && bover_menu_select2 [p] == w2))
     {
      col = COL_BOX0;
      if (selecting [p])
       col = COL_BOX1;
      rectfill(display[0], x + 140, y + 3 + (w2 * 20), x + 300, y + 20 + (w2 * 20), col);
      rect(display[0], x + 140, y + 3 + (w2 * 20), x + 300, y + 20 + (w2 * 20), col + 1);
      weapon_box(p, BMSG_WPN1 + w2, x, WBOX_Y, x + 301, y + 12 + (w2 * 20), 1);
//      textprintf_ex(display[0], small_font, x + 145, y + 5 + (w2 * 20), COL_BOX3, -1, "<");
//      textprintf_right_ex(display[0], small_font, x + 295, y + 5 + (w2 * 20), COL_BOX3, -1, ">");
     }
     print_weapon_name(x + 155, y + 5 + (w2 * 20), COL_BOX4, w2);
    }
   }
   print_weapon_name(x + 20, y, COL_BOX4, PP.weapon_type [w]);
  }

  y += LSPACE1;
  if (bover_menu_select [p] == CHOOSE_LAUNCH)
   bover_box(x, y);
  textprintf_ex(display[0], small_font, x + 20, y, COL_BOX4, -1, "launch >>");

  y += LSPACE1;
  if (bover_menu_select [p] == CHOOSE_REPLAY)
   bover_box(x, y);
  textprintf_ex(display[0], small_font, x + 20 - text_length(small_font, "<< "), y, COL_BOX4, -1, "<< replay briefing");

 }


}

*/

void bover_box(int x, int y)
{
    rectfill(display[0], x - 5, y - 3, x + 150, y + 15, COL_BOX1);
    rect(display[0], x - 5, y - 3, x + 150, y + 15, COL_BOX2);
}

void print_weapon_name(int x, int y, int col, int w, int number)
{
   switch(w)
   {
    case WPN_AF_MISSILE:
     textprintf_ex(display[0], small_font, x, y, col, -1, "%i - anti-fighter missile", number); break;
    case WPN_AWS_MISSILE:
     textprintf_ex(display[0], small_font, x, y, col, -1, "%i - anti-warship missile", number); break;
//    case WPN_ROCKET1:
//     textprintf_ex(display[0], small_font, x, y, col, -1, "rocket"); break;
    case WPN_ROCKET:
     textprintf_ex(display[0], small_font, x, y, col, -1, "%i - multi-rockets", number); break;
    case WPN_TORP:
     textprintf_ex(display[0], small_font, x, y, col, -1, "%i - torpedo", number); break;
    case WPN_ROCKET2:
     textprintf_ex(display[0], small_font, x, y, col, -1, "%i - rocket", number); break;
    case WPN_DEFLECT:
     textprintf_ex(display[0], small_font, x, y, col, -1, "%i - tracking cannon", number); break;
    case WPN_WROCKET:
     textprintf_ex(display[0], small_font, x, y, col, -1, "%i - Wing rockets", number); break;
    case WPN_HROCKET:
     textprintf_ex(display[0], small_font, x, y, col, -1, "%i - Heavy rockets", number); break;
    case WPN_BLASTER:
     textprintf_ex(display[0], small_font, x, y, col, -1, "%i - heavy cannon", number); break;
    case WPN_LW_MISSILE:
     textprintf_ex(display[0], small_font, x, y, col, -1, "%i - Devourer", number); break;
    case WPN_ADV_LW_MISSILE:
     textprintf_ex(display[0], small_font, x, y, col, -1, "%i - Devourer-Adv", number); break;
    case WPN_HVY_LW_MISSILE:
     textprintf_ex(display[0], small_font, x, y, col, -1, "%i - Devourer-Hvy", number); break;
    case WPN_RAIN:
     textprintf_ex(display[0], small_font, x, y, col, -1, "%i - Burning rain", number); break;
   }
}

void weapon_box(int p, int msg, int x, int y, int x1, int y1, int line_dir)
{

 rectfill(display[0], x, y, x + 300, y + 110, COL_STAR1);
 rect(display[0], x, y, x + 300, y + 110, COL_BOX1);

 if (line_dir == -1)
 {
  hline(display[0], x1, y1, x1 - 10, COL_BOX1);
  vline(display[0], x1 - 10, y1, y + 20, COL_BOX1);
  hline(display[0], x1 - 10, y + 20, x, COL_BOX1);
 }
  else
  {
   hline(display[0], x1, y1, x1 + 10, COL_BOX1);
   vline(display[0], x1 + 10, y1, y + 20, COL_BOX1);
   hline(display[0], x1 + 10, y + 20, x1, COL_BOX1);
  }

  generate_message(bmsg[msg], 280, 15, BCOL_TEXT, bline);

  display_message(display[0], bline, x + 10, y + 10);

}


#define WTBOX_X 150

void weapon_text(int x, int y, int w, int w2)
{

  int message_lines;

//  message_lines = generate_message(BMSG_WPN1 + w2, WTBOX_X, BCOL_TEXT, bline);
  message_lines = generate_message(bmsg[BMSG_WPN1 + w2], WTBOX_X, 15, BCOL_TEXT, bline);

//  if (w == 1)
   x += 60;
//    else
//     x -= WTBOX_X + 10;

  rectfill(display[0], x - 6, y - 6, x + WTBOX_X + 6, y + (message_lines*16) + 6, BCOL_EDGE);
  rectfill(display[0], x - 3, y - 3, x + WTBOX_X + 3, y + (message_lines*16) + 3, BCOL_SCREEN);

  display_message(display[0], bline, x, y);

}


#define BR_ROWS 30

enum
{
BR_ROW_OURS_TITLE,
BR_ROW_OUR_SHIP,
BR_ROW_AM,
BR_ROW_ENEMY_TITLE,
BR_ROW_ENEMY_SHIP,
BR_ROW_END

};

int br_row [BR_ROWS];
int br_row_val [BR_ROWS];

void battle_report_display(int row, int flash, int space);
void assemble_br_rows(void);


void battle_report(void)
{

 int row = 0;
 int flash;
 int finished = 0;

 assemble_br_rows();
 flash = 10;

 while (TRUE)
 {
  if (flash > 0)
   flash --;
  if (finished == 0 && flash <= 0)
  {
   flash = 10;
   row ++;
   if (br_row [row] == BR_ROW_END)
   {
    flash = 0;
    finished = 1;
   }
  }

  battle_report_display(row, flash, finished);

  vsync();
  blit(display[0], screen, 0, 0, 0, 0, 800, 600);

  while (ticked == 0)
  {
      rest(1);
  };

  ticked = 0;



  if (finished && key [KEY_SPACE])
   break;
 };

}


void battle_report_display(int row, int flash, int space)
{
    clear_to_color(display [0], 0);

    rectfill(display[0], 100, 50, 700, 550, BCOL_DARK);
    rect(display[0], 100, 50, 700, 550, BCOL_EDGE);

    rectfill(display[0], 100, 55, 700, 72, BCOL_EDGE);
    textprintf_centre_ex(display[0], small_font, 400, 58, BCOL_TEXT, -1, "%s", "Battle Report");

    int r = 0;
    int mincol = COL_BOX1;
    int maxcol = COL_BOX4;
    int textcol = COL_BOX4;
    int y = 100;
    int col;
    // int x1 = 0, x2 = 13;
    char str [350];
    int i;
    int col2;

    for (r = 0; r < row + 1; r ++) {

        if (br_row [r] == BR_ROW_ENEMY_TITLE) {
            mincol = COL_EBOX1;
            maxcol = COL_EBOX4;
            textcol = COL_EBOX4;
        }
        if (r == row && flash > 0) {
            col = mincol + (flash >> 1);
            if (col > maxcol) {
                col = maxcol;
            }
            rectfill(display[0], 100, y + 5, 700, y + 22, col);
        }

        switch(br_row [r])
        {
            case BR_ROW_OURS_TITLE:
                if (r != row) {
                    rectfill(display[0], 101, y + 5, 699, y + 22, COL_BOX1);
                }
                textprintf_ex(display[0], small_font, 150, y + 7, textcol, -1, "%s", "Friendly Forces");
                y += 5;
                break;
            case BR_ROW_OUR_SHIP:
                switch(br_row_val[r]) {
                    case SHIP_OLD2: strcpy(str, "CTBR-SUNSHARK"); break;
                    case SHIP_OLD3: strcpy(str, "CTBR-STARWHALE"); break;
                    case SHIP_FRIEND3: strcpy(str, "IF-TRIREME"); break;
                    case SHIP_FIGHTER_FRIEND: strcpy(str, "CTBR-SANDFLY"); break;
                    case SHIP_FSTRIKE: strcpy(str, "CTBR-RAM"); break;
                    case SHIP_LACEWING: strcpy(str, "CTBR-LACEWING"); break;
                    case SHIP_MONARCH: strcpy(str, "CTBR-MONARCH"); break;
                    case SHIP_IBEX: strcpy(str, "CTBR-IBEX"); break;
                    case SHIP_AUROCHS: strcpy(str, "CTBR-AUROCHS"); break;
                    case SHIP_DROM: strcpy(str, "CTBR-DROMEDARY"); break;
                    case SHIP_LINER: strcpy(str, "STARLINER"); break;
                    default:
                        strcpy(str, "Unknown ship type");
                        textprintf_right_ex(display[0], small_font, 80, y + 7, textcol, -1, "%i %i %i", br_row_val [r], arena.srecord [SREC_CREATED] [TEAM_FRIEND] [br_row_val [r]], arena.srecord [SREC_DESTROYED] [TEAM_FRIEND] [br_row_val [r]]);
                        break;
                }
                textprintf_right_ex(display[0], small_font, 230, y + 7, textcol, -1, "%s", str);
                col2 = 0;
                for (i = 0; i < arena.srecord[SREC_CREATED][TEAM_FRIEND][br_row_val[r]]; i ++) {
                    if (i >= arena.srecord[SREC_CREATED][TEAM_FRIEND][br_row_val[r]] - arena.srecord[SREC_DESTROYED][TEAM_FRIEND][br_row_val[r]]) {
                        col2 = 1; // destroyed
                    }
                    draw_sprite(display[0], bsprite[TEAM_FRIEND][get_bsprite_from_ship(br_row_val[r])][col2], 235 + (i * 13), y + 7);
                }
                break;
            case BR_ROW_AM:
                textprintf_right_ex(display[0], small_font, 230, y + 7, textcol, -1, "%s", "Angry Moth");
                col2 = 0;
                for (i = 0; i < player[0].starting_ships; i ++) {
                    if (i >= player[0].starting_ships - player[0].ships_lost) {
                        col2 = 1; // destroyed
                    }
                    draw_sprite(display[0], bsprite[TEAM_FRIEND][BSF_FIGHTER][col2], 235 + (i * 13), y + 7);
                }
                y += 10;
                break;
            case BR_ROW_ENEMY_TITLE:
                if (r != row) {
                    rectfill(display[0], 101, y + 5, 699, y + 22, COL_EBOX1);
                }
                textprintf_ex(display[0], small_font, 150, y + 7, textcol, -1, "%s", "Enemy Forces");
                y += 5;
                break;
            case BR_ROW_ENEMY_SHIP:
                switch(br_row_val [r]) {
                    case SHIP_SCOUT2: strcpy(str, "FSF-RIGHTEOUS"); break;
                    case SHIP_SCOUT3: strcpy(str, "FSF-MERCIFUL"); break;
                    case SHIP_BOMBER: strcpy(str, "FSF-MESSENGER"); break;
                    case SHIP_FIGHTER: strcpy(str, "FSF-CALLED"); break;
                    case SHIP_ESCOUT: strcpy(str, "FSF-DISCIPLE"); break;
                    case SHIP_EINT: strcpy(str, "FSF-CHOSEN"); break;
                    case SHIP_SCOUTCAR: strcpy(str, "FSF-GRACEFUL"); break;
                    case SHIP_ECARRIER: strcpy(str, "FSF-INFINITE"); break;
                    case SHIP_FREIGHT: strcpy(str, "FSF-BENEVOLENT"); break;
                    case SHIP_EBASE: strcpy(str, "Base"); break;
                    default: strcpy(str, "Unknown ship type"); break;
                }
                textprintf_right_ex(display[0], small_font, 230, y + 7, textcol, -1, "%s", str);
                col2 = 0;
                for (i = 0; i < arena.srecord[SREC_CREATED][TEAM_ENEMY][br_row_val[r]]; i ++) {
                    if (i >= arena.srecord[SREC_CREATED][TEAM_ENEMY][br_row_val[r]] - arena.srecord[SREC_DESTROYED][TEAM_ENEMY][br_row_val[r]]) {
                        col2 = 1; // destroyed
                    }
                    draw_sprite(display[0], bsprite[TEAM_ENEMY][get_bsprite_from_ship(br_row_val[r])][col2], 235 + (i * 13), y + 7);
                }
                break;
        }
        y += 20;
    }

    y += 20;
    if (space) {
        report_text(y);
        textprintf_centre_ex(display[0], small_font, 400, 530, BCOL_TEXT, -1, "%s", "Press space to continue");
    }
}

int get_rank(int lost, int r2, int r3, int r4, int r5)
{

 if (lost <= 0)
  return 0;
 if (lost < r2)
  return 1;
 if (lost < r3)
  return 2;
 if (lost < r4)
  return 3;
 if (lost < r5)
  return 4;

 return 5;

}


// returns % of forces lost (each ship is given a value)
int loss_proportion(int a)
{

 int i;
 int total = 0;
 int lost = 0;

    for (i = 0; i < NO_SHIP_TYPES; i ++)
    {
     if (arena.srecord [SREC_CREATED] [a] [i] > 0)
     {
      total += arena.srecord [SREC_CREATED] [a] [i] * ship_value(i);
      lost += arena.srecord [SREC_DESTROYED] [a] [i] * ship_value(i);
     }

    }

    if (a == TEAM_FRIEND)
    {
     total += player[0].starting_ships * 6;
     lost += player[0].ships_lost * 6;
    }

 return (lost * 100) / total;

}

int ship_value(int type)
{

 switch(type)
 {
  case SHIP_OLD2: return 10;
  case SHIP_DROM: return 20;
  case SHIP_LINER: return 6;
  case SHIP_OLD3: return 35;
  case SHIP_FIGHTER: return 2;
  case SHIP_FRIEND3: return 45;
  case SHIP_SCOUT2: return 7;
  case SHIP_BOMBER: return 5;
  case SHIP_LACEWING:
  case SHIP_MONARCH:
  case SHIP_IBEX:
  case SHIP_AUROCHS:
  case SHIP_FIGHTER_FRIEND: return 3;
  case SHIP_FSTRIKE: return 3;
  case SHIP_SCOUT3: return 35;
  case SHIP_ESCOUT: return 4;
  case SHIP_SCOUTCAR: return 45;
  case SHIP_ECARRIER: return 60;
  case SHIP_FREIGHT: return 45;
  case SHIP_EBASE: return 100;
  case SHIP_EINT: return 4;

 }

 return 0;
}

void report_text(int y)
{

 struct msgstruct rmsg;

 strcpy(rmsg.text, "");

 int flost;
// int elost;
 int lost_rank;
// int dest_rank;
 int var;

 switch(arena.stage)
 {

  case 1:
   strcpy(rmsg.text, " This does not look good. ");
   if (arena.srecord [SREC_DESTROYED] [TEAM_FRIEND] [SHIP_FRIEND3] < 2)
   {
     strcat(rmsg.text, " $P $P According to the surrendering Imperial officers, they were part of a task force sent to Federation space to investigate the activities of a mysterious cult known as the 'Book of Love'. ");
     strcat(rmsg.text, " The investigation had only just started when the task force was almost destroyed in a surprise attack by the Federation Starfleet. ");
     strcat(rmsg.text, " $P $P Whatever happened, the Federation's attack on starships under our protection, in one of our systems, was an act of war. We must prepare for further aggression. ");

//     strcat(rmsg.text, " $P $P Gazer 1 Captain made the difficult decision to engage with the aggressive force of FSF ships, but that was all that could have been done in the circumstances. ");

     if (arena.srecord [SREC_DESTROYED] [TEAM_FRIEND] [SHIP_FRIEND3] == 0)
      strcat(rmsg.text, " $P $P The Imperial warships (TRIREME heavy cruisers) will fly alongside the CTBR until their mission is complete. We are working on repairing the damage they have sustained. ");
       else
        strcat(rmsg.text, " $P $P The surviving Imperial warship (a TRIREME heavy cruiser) will fly alongside the CTBR until its mission is complete. We are working on integrating it into our fleet. ");

//     strcat(rmsg.text, " $P $P We may be in for a wild ride here. I only hope that we are ready for what happens next. ");
   }
    else
    {
     strcat(rmsg.text, " $P $P The Federation's attack on starships under our protection, in one of our systems, was an act of war. We must prepare for further aggression. ");
     strcat(rmsg.text, " $P $P It is unfortunate that the Imperial cruisers could not have been saved, but lightly armed interceptors are no match for warships. ");
     strcat(rmsg.text, " $P $P We may be in for the long haul here. I only hope that we are ready for what happens next. ");
    }

   break;

/*
  case 1:
   strcpy(rmsg.text, " This does not look good. ");
   if (arena.srecord [SREC_DESTROYED] [TEAM_FRIEND] [SHIP_FRIEND3] < 2)
   {
     strcat(rmsg.text, " $P $P We have had a chance to debrief the surrendering CTBR officers. They claim to be the last survivors of a massive, unexplained surprise attack that destroyed the rest of the Commonwealth's 2nd Fleet. ");
     strcat(rmsg.text, " We have been unable to open communications with either side and their diplomatic representatives claim to know nothing, but based on what we have seen we may not be able to avoid being drawn into this war. ");
     strcat(rmsg.text, " $P $P Alpha Captain made the difficult decision to engage with the aggressive force of FSF ships, but that was all that could have been done in the circumstances. It is a testament to the skills of our fighter pilots that the FSF ships were unable to complete their mission. ");

     if (arena.srecord [SREC_DESTROYED] [TEAM_FRIEND] [SHIP_FRIEND3] == 0)
      strcat(rmsg.text, " $P $P The crews of the CTBR ships (TRIREME heavy cruisers) have agreed to fly alongside the OCSF until they can return to Commonwealth space. We are working on repairing their ships. ");
       else
        strcat(rmsg.text, " $P $P The crew of the surviving CTBR ship (a TRIREME heavy cruiser) has agreed to fly alongside the OCSF until they can return to Commonwealth space. We are working on repairing their ship. ");

     strcat(rmsg.text, " $P $P We may be in for the long haul here. I only hope that we are ready for what happens next. ");
   }
    else
    {
     strcat(rmsg.text, " $P $P It seems that the Earth Federation has declared war on the ... Commonwealth, although we have no idea why. We have been unable to open communications with either side and their diplomatic representatives claim to know nothing. ");
     strcat(rmsg.text, " It looks like the Outer Colonies will be drawn into this war as well.");
     strcat(rmsg.text, " $P $P Alpha Captain made the difficult decision to engage with an aggressive force of FSF ships in one of our systems, but that was all that could have been done in the circumstances. It is unfortunate that the CTBR cruisers could not be saved, but the Federation forces were just too strong. ");
     strcat(rmsg.text, " $P $P We may be in for the long haul here. I only hope that we are ready for what happens next. ");
    }
   break;*/
  case 2:
   strcpy(rmsg.text, " It's unfortunate that our attempt to contact Commonwealth forces apparently coincided with a major Federation offensive. $P $P");
   flost = loss_proportion(TEAM_FRIEND);
   lost_rank = get_rank(flost, 30, 50, 70, 80);
   switch(lost_rank)
   {
    case 0:
     strcat(rmsg.text, " Alpha group returned to base under orders and, thanks to excellent fighter support, intact. "); break;
    case 1:
     strcat(rmsg.text, " Alpha group returned to base under orders, suffering only minor losses thanks to excellent fighter support. "); break;
    case 2:
     strcat(rmsg.text, " Alpha group escaped a furious assault by carrier-launched FSF bombers, suffering moderate losses. "); break;
    case 3:
     strcat(rmsg.text, " Alpha group escaped a furious assault by carrier-launched FSF bombers, losing several ships. "); break;
    case 4:
     strcat(rmsg.text, " Alpha group barely escaped a furious assault by carrier-launched FSF bombers, losing several ships. "); break;
    case 5:
     strcat(rmsg.text, " Alpha group was almost destroyed by carrier-launched FSF bombers, but some of our ships managed to get out. "); break;
   }
   var = arena.srecord [SREC_DESTROYED] [TEAM_ENEMY] [SHIP_ECARRIER] + arena.srecord [SREC_DESTROYED] [TEAM_ENEMY] [SHIP_SCOUT2];
   switch(var)
   {
// no case zero
    case 1:
      strcat(rmsg.text, " $P $P Our fighters managed to take one of their warships down as well. "); break;
    case 2:
      strcat(rmsg.text, " $P $P Our fighters managed to take two of their warships down as well. "); break;
    case 3:
    case 4:
    case 5:
      strcat(rmsg.text, " $P $P In addition to fighting off the bombers, Angry Moth's wing managed to destroy several FSF warships. Good work! "); break;
    case 6:
    case 7:
      strcat(rmsg.text, " $P $P In addition to fighting off the bombers, Angry Moth's wing managed to inflict severe damage on the FSF carrier group. Excellent work! "); break;
    case 8:
      strcat(rmsg.text, " $P $P In addition to fighting off the bombers, Angry Moth's wing managed to completely destroy the FSF carrier group. Fabulous work! "); break;
   }
   break;

  case 3:
  switch (arena.srecord [SREC_DESTROYED] [TEAM_ENEMY] [SHIP_FREIGHT])
  {
   default:
   case 0: strcpy(rmsg.text, " . "); break;
  }
  /*
   strcpy(rmsg.text, " This does not look good. ");
   if (arena.srecord [SREC_DESTROYED] [TEAM_ENEMY] [SHIP_FREIGHT] < 2)
   {
     strcat(rmsg.text, " $P $P We have had a chance to debrief the surrendering CTBR officers. They claim to be the last survivors of a massive, unexplained surprise attack that destroyed the rest of the Commonwealth's 2nd Fleet. ");
     strcat(rmsg.text, " We have been unable to open communications with either side and their diplomatic representatives claim to know nothing, but based on what we have seen we may not be able to avoid being drawn into this war. ");
     strcat(rmsg.text, " $P $P Alpha Captain made the difficult decision to engage with the aggressive force of FSF ships, but that was all that could have been done in the circumstances. It is a testament to the skills of our fighter pilots that the FSF ships were unable to complete their mission. ");

     if (arena.srecord [SREC_DESTROYED] [TEAM_FRIEND] [SHIP_FRIEND3] == 0)
      strcat(rmsg.text, " $P $P The crews of the CTBR ships (TRIREME heavy cruisers) have agreed to fly alongside the OCSF until they can return to Commonwealth space. We are working on repairing their ships. ");
       else
        strcat(rmsg.text, " $P $P The crew of the surviving CTBR ship (a TRIREME heavy cruiser) has agreed to fly alongside the OCSF until they can return to Commonwealth space. We are working on repairing their ship. ");

     strcat(rmsg.text, " $P $P We may be in for the long haul here. I only hope that we are ready for what happens next. ");
   }
    else
    {
     strcat(rmsg.text, " $P $P It seems that the Earth Federation has declared war on the ... Commonwealth, although we have no idea why. We have been unable to open communications with either side and their diplomatic representatives claim to know nothing. ");
     strcat(rmsg.text, " It looks like the Outer Colonies will be drawn into this war as well.");
     strcat(rmsg.text, " $P $P Alpha Captain made the difficult decision to engage with an aggressive force of FSF ships in one of our systems, but that was all that could have been done in the circumstances. It is unfortunate that the CTBR cruisers could not be saved, but the Federation forces were just too strong. ");
     strcat(rmsg.text, " $P $P We may be in for the long haul here. I only hope that we are ready for what happens next. ");
    }*/
   break;
/*  case 1:
// only relevant consideration here is flost as all enemies must be destroyed to finish mission
   flost = loss_proportion(TEAM_FRIEND);
   lost_rank = get_rank(flost, 30, 50, 70, 80);
   switch(lost_rank)
   {

    case 0:
     strcpy(rmsg.text, " The mission was an absolute success. Enemy forces were annihilated, with no losses on our side. Well done! "); break;
    case 1:
     strcpy(rmsg.text, " The mission was a great success. Enemy forces were annihilated, with only minor losses on our side. Well done! "); break;
    case 2:
     strcpy(rmsg.text, " The mission was successful. Although we lost a number of ships, the enemy lost far more. "); break;
    case 3:
     strcpy(rmsg.text, " The mission was successful. Although we lost several ships, the enemy force was destroyed. "); break;
    case 4:
     strcpy(rmsg.text, " We lost a number of our ships, but the enemy force was destroyed. Overall, a success. "); break;
    case 5:
     strcpy(rmsg.text, " Most of our ships were destroyed, but at least we stopped the enemy force getting through. "); break;
   }

   break;*/
   /*
  case 3:
   strcpy(rmsg.text, "The primary objective - destroying the enemy base - was achieved");
   flost = loss_proportion(TEAM_FRIEND);
   lost_rank = get_rank(flost, 30, 50, 65, 80);
   var = 1; // is the first message good or bad? (ie how bad were friendly losses?)
   switch(lost_rank)
   {
    case 0:
     strcat(rmsg.text, " with no losses at all on our side - incredible! "); break; // This is very unlikely to ever happen
    case 1:
     strcat(rmsg.text, " with only light losses - excellent work! "); break;
    case 2:
     strcat(rmsg.text, " with moderate losses - not bad for such a risky operation. "); break;
    case 3:
     strcat(rmsg.text, ". Several OCSF ships were lost - but we expected worse. ");
     var = -1;
     break;
    case 4:
     strcat(rmsg.text, ". Our losses were severe - but we got the job done. ");
     var = -1;
     break;
    case 5:
     strcat(rmsg.text, ", although at a heavy cost in OCSF ships. ");
     var = -1;
     break;
   }
   flost = loss_proportion(TEAM_ENEMY);
   lost_rank = get_rank(flost, 40, 60, 80, 100);
   var = 1; // is the first message good or bad? (ie how bad were friendly losses?)
   switch(lost_rank)
   {
    case 2:
     strcat(rmsg.text, "We also took out a reasonable number of enemy ships. "); break;
    case 3:
     strcat(rmsg.text, "We also inflicted heavy losses on the enemy. ");
     break;
    case 4:
     strcat(rmsg.text, "Almost the entire enemy fleet was destroyed as well. ");
     break;
    case 5:
     strcat(rmsg.text, "All enemy ships were destroyed - this sector should be safe for now. ");
     break;
   }
   break;
*/

 }

  generate_message(rmsg, 500, 15, BCOL_TEXT, bline);

  display_message(display[0], bline, 130, y);

}





void assemble_br_rows(void)
{

 int r = 0;
 int a, t;

 br_row [r] = BR_ROW_OURS_TITLE;
 r ++;

 a = TEAM_FRIEND;

 for (t = 0; t < NO_SHIP_TYPES; t ++)
 {
  if (arena.srecord [SREC_CREATED] [a] [t] > 0)
  {
   br_row [r] = BR_ROW_OUR_SHIP;
   br_row_val [r] = t;
   r ++;
  }
 }

 br_row [r] = BR_ROW_AM;
 r ++;

 br_row [r] = BR_ROW_ENEMY_TITLE;
 r++;

 a = TEAM_ENEMY;

 for (t = 0; t < NO_SHIP_TYPES; t ++)
 {
  if (arena.srecord [SREC_CREATED] [a] [t] > 0)
  {
   br_row [r] = BR_ROW_ENEMY_SHIP;
   br_row_val [r] = t;
   r ++;
  }
 }

 br_row [r] = BR_ROW_END;
// r ++;

// return r;

}



/*

*************************************************************************************************

Strategic map

*************************************************************************************************

*/


struct sstarstruct sstar_template [SSTARS];
struct sstarstruct sstar [SSTARS];

// called at start of game after sstar_template is filled - runs once only
void init_sstars(void)
{

}

// run each time a briefing starts or is reset
// sstar_template is read from starmap.bmp in display_init.c and never changes.
void prepare_sstars_for_briefing(void)
{

 int s;

 for (s = 0; s < SSTARS; s ++)
 {
  sstar [s].exists = sstar_template [s].exists;
  sstar [s].x = sstar_template [s].x;
  sstar [s].y = sstar_template [s].y;
  sstar [s].old_x = -1000;
  sstar [s].old_y = -1000;
  sstar [s].type = sstar_template [s].type;
  sstar [s].side = SSIDE_NONE; //sstar_template [s].side;
  sstar [s].settled = 0;
  sstar [s].col = sstar_template [s].col;

  assign_star_settled(s, 300, 280, 240);
  assign_star_side(s, 300, 280, 70, SSIDE_IMP);
  assign_star_side(s, 450, 170, 80, SSIDE_FED);
  assign_star_side(s, 450, 210, 50, SSIDE_CWLTH);


 }

 starzoom = 1000;
 zoom_target = 1000;
 starzoom_x = MAP_X + (MAP_W/2);
 starzoom_y = MAP_Y + (MAP_H/2);
 zoom_x_target = starzoom_x;
 zoom_y_target = starzoom_y;
 starselect_side = -1;
 starselect_settled = 0;

}

void assign_star_side(int s, int x, int y, int rad, int side)
{

 int dist = 0;
 dist = hypot(sstar[s].y - y, sstar[s].x - x);

 if (dist < rad)
  sstar[s].side = side;

}

void assign_star_settled(int s, int x, int y, int rad)
{

 int dist = 0;
 dist = hypot(sstar[s].y - y, sstar[s].x - x);

 if (dist < rad)
  sstar[s].settled = 1;

}




void ssdisplay(void)
{

 clear_to_color(display [0], 0);

 rectfill(display[0], MAP_X-EDGE, MAP_Y-EDGE, MAP_X + MAP_W+EDGE, MAP_Y + MAP_H+EDGE, BCOL_EDGE);
 rectfill(display[0], MAP_X, MAP_Y, MAP_X + MAP_W, MAP_Y + MAP_H, 0);//BCOL_DARK);

 int s, x, y;
 int col;

 for (s = 0; s < SSTARS; s ++)
 {
  if (sstar[s].exists == 0)
   continue;
  col = COL_WHITE;
  switch(sstar[s].col)
  {
   case SS_RED: col = COL_EBOX4; break;
   case SS_BLUE: col = COL_EBOX4; break;
   case SS_YELLOW: col = COL_E6 + TRANS_RED4; break;
  }
// to work out zoom, first we work out how far the star is from the zoom focus:
//  x = starzoom_x - sstar[s].x;
//  y = starzoom_y - sstar[s].y;
  x = sstar[s].x - starzoom_x;
  y = sstar[s].y - starzoom_y;
// then multiply that by the zoom level:
  x *= starzoom;
  x /= 1000;
  y *= starzoom;
  y /= 1000;
// then add on the focus:
  x += starzoom_x + MAP_X;
  y += starzoom_y + MAP_Y;
// don't display if not visible
  if (x <= MAP_X || x >= MAP_X + MAP_W || y <= MAP_Y || y >= MAP_Y + MAP_H)
   continue;

 col = sstar[s].col;

  if (sstar[s].old_x != -1000)
   line(display[0], x, y, sstar[s].old_x, sstar[s].old_y, col + COL_STAR1);
  sstar[s].old_x = x;
  sstar[s].old_y = y;

 putpixel(display[0], x, y, col + COL_STAR1);

  if (starselect_side == sstar[s].side)
  {
   col = COL_BOX2;

   putpixel(display[0], x-2, y-2, col);
   putpixel(display[0], x-1, y-2, col);
   putpixel(display[0], x-2, y-1, col);

   putpixel(display[0], x+2, y+2, col);
   putpixel(display[0], x+1, y+2, col);
   putpixel(display[0], x+2, y+1, col);

//   vline(display[0], x-3,y-2,y+2, col);
//   vline(display[0], x+3,y-2,y+2, col);
  }


  if (starselect_settled == 1 && sstar[s].settled == 1)
  {
   col = COL_BOX2;

   putpixel(display[0], x-2, y-2, col);
   putpixel(display[0], x-1, y-2, col);
   putpixel(display[0], x-2, y-1, col);

   putpixel(display[0], x+2, y+2, col);
   putpixel(display[0], x+1, y+2, col);
   putpixel(display[0], x+2, y+1, col);

  }

// textprintf_centre_ex(display[0], small_font, x, y, BCOL_TEXT, -1, "%i", col);//" %i %i  (%i, %i)  %i", starzoom, zoom_target, starzoom_x, starzoom_y, zoom_move_speed);
 continue;
/*
  switch(sstar[s].side)
  {
   case SSIDE_OC: col = COL_BOX4; col2 = COL_BOX3; break;
   case SSIDE_FED: col = COL_E1 + TRANS_RED2; col2 = COL_E1 + TRANS_RED1; break;
   case SSIDE_IND: col = TRANS_YELLOW4; col2 = TRANS_RED3; break;
   case SSIDE_CWLTH: col = COL_E1 + TRANS_BLUE2; col2 = COL_E1 + TRANS_BLUE1; break;
  }

//  if (starzoom > )
//  circle(display[0], x, y, 2, col);
  putpixel(display[0], x-1, y, col2);
  putpixel(display[0], x+1, y, col2);
  putpixel(display[0], x, y-1, col2);
  putpixel(display[0], x, y+1, col2);

  if (sstar[s].old_x != -1000)
   line(display[0], x, y, sstar[s].old_x, sstar[s].old_y, col);
  sstar[s].old_x = x;
  sstar[s].old_y = y;
  putpixel(display[0], x, y, col);
  if (starselect_side == sstar[s].side)
  {
   vline(display[0], x-3,y-2,y+2, col);
   vline(display[0], x+3,y-2,y+2, col);
  }*/
 }

 rectfill(display[0], MAP_X, MAP_Y + 5, MAP_X + MAP_W, MAP_Y + 22, BCOL_EDGE);
 textprintf_centre_ex(display[0], small_font, MAP_X + (MAP_W>>1), MAP_Y+7, BCOL_TEXT, -1, "Human Space");//" %i %i  (%i, %i)  %i", starzoom, zoom_target, starzoom_x, starzoom_y, zoom_move_speed);

// vsync();
// blit(display[0], screen, 0, 0, 0, 0, 800, 600);

}



void run_starmap(void)
{

 if (starzoom < zoom_target + 41 && starzoom > zoom_target - 41)
  starzoom = zoom_target;

 if (starzoom > zoom_target)
  starzoom -= 40;
 if (starzoom < zoom_target)
  starzoom += 40;

 if (starzoom_x < zoom_x_target + (zoom_move_speed*2) && starzoom_x > zoom_x_target - (zoom_move_speed*2))
 {
  starzoom_x = zoom_x_target;
  starzoom_y = zoom_y_target;
 }
  else
  {
   float angle = atan2(starzoom_y - zoom_y_target, starzoom_x - zoom_x_target);
   starzoom_x -= fxpart(angle, zoom_move_speed);
   starzoom_y -= fypart(angle, zoom_move_speed);
  }

/*
 if (starzoom_x < zoom_x_target + 4 && starzoom_x > zoom_x_target - 4)
  starzoom_x = zoom_x_target;
 if (starzoom_x > zoom_x_target)
  starzoom_x -= 3;
 if (starzoom_x < zoom_x_target)
  starzoom_x += 3;

 if (starzoom_y < zoom_y_target + 4 && starzoom_y > zoom_y_target - 4)
  starzoom_y = zoom_y_target;
 if (starzoom_y > zoom_y_target)
  starzoom_y -= 3;
 if (starzoom_y < zoom_y_target)
  starzoom_y += 3;
*/

}


