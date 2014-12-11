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

#include "stuff.h"
#include "text.h"

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
//extern int message_lines;

BITMAP* bsprite [2] [BSPRITES] [2];
BITMAP* wisprite [WISPRITES];

unsigned char briefing_counter;
char bkey_wait [2];
int bkey_wait_pressed [2];

struct linestruct bline [LINES];

void bdisplay(void);
void run_bconvoys(void);
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
void choose_display(int finished1, int finished2);
void weapon_text(int x, int y, int w, int w2);
void draw_arrows(int x, int y, int w, int basecol, int counter);
void weapon_box(int p, int msg, int x, int y, int x1, int y1, int line_dir);

void run_tbox(void);
void draw_tbox(void);
void create_tbox(int msg, int x, int y, int w, int x2, int y2, int cv);
void report_text(int y);
int loss_proportion(int a);
int ship_value(int type);
int get_rank(int lost, int r2, int r3, int r4, int r5);


#define BSHIPS 50
#define BSELECT 10

#define MENU_SENSE 100

#define BOX_W 160

enum
{
BSHIP_NONE,
BSHIP_OLD2,
BSHIP_OLD3,
BSHIP_FRIEND3,
BSHIP_FIGHTER_FRIEND,
BSHIP_MOTH,

BSHIP_END_FRIENDS,

BSHIP_FIGHTER,
BSHIP_SCOUT2,
BSHIP_BOMBER,
BSHIP_SCOUT3,
BSHIP_ESCOUT,
BSHIP_SCOUTCAR,
BSHIP_EBASE,
BSHIP_EINT,
NO_BSHIP_TYPES

};

enum
{BCONVOY_0, BCONVOY_1, BCONVOY_2,BCONVOY_3,BCONVOY_4,BCONVOY_5,BCONVOY_6,BCONVOY_7,BCONVOY_8,BCONVOYS};

struct bshipstruct
{
 int type;
 int side;
 int convoy;
 int x, y;
};

struct bconvoystruct
{
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

struct bselectstruct
{
 int exists;
 int x;
 int y;
 int x1, y1, x2, y2;
 int x_offset, y_offset;
 int count;
};

struct bshipstruct bship [BSHIPS];
struct bconvoystruct bconvoy [BCONVOYS];
struct bselectstruct bselect [BSELECT];

enum
{
BSCRIPT_HEADER,
BSCRIPT_NEW_SHIP,
BSCRIPT_NEW_CONVOY,
BSCRIPT_MOVE,
BSCRIPT_WAIT,
BSCRIPT_HIT_FIRE,
BSCRIPT_BMESSAGE,
BSCRIPT_SETTLE,
BSCRIPT_SELECT,
BSCRIPT_CLEAR_MESSAGE,
BSCRIPT_END
};

#define BVARS 8

#define LEFT 0
#define RIGHT 1

enum
{
BNEWSHIP_BCONVOY,
BNEWSHIP_TYPE,
BNEWSHIP_X,
BNEWSHIP_Y
};
enum
{
BNEWCONVOY_CONVOY,
BNEWCONVOY_X,
BNEWCONVOY_Y,
BNEWCONVOY_FACE
};

enum
{
BMOVE_CONVOY,
BMOVE_WAIT,
BMOVE_X,
BMOVE_Y,
BMOVE_SPEED,
BMOVE_FACE,
BMOVE_SETTLE
};
enum
{
BSELECT_CONVOY,
BSELECT_X_OFFSET,
BSELECT_Y_OFFSET
};
enum
{
BWAIT_TIME
};
enum
{
BMESSAGE_BMESSAGE,
BMESSAGE_X,
BMESSAGE_Y,
BMESSAGE_W,
BMESSAGE_X2,
BMESSAGE_Y2,
BMESSAGE_CONVOY
};

// BOX_W - 10

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



struct briefingscript
{
 int type;
 int var [BVARS];
};

enum
{
BMSG_CANNON,
BMSG_FIGHTER,
BMSG_WPN1,
BMSG_WPN2,
BMSG_WPN3,
BMSG_WPN4,
BMSG_WPN5,
BMSG_WPN6,
BMSG_WPN7,
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
BMSG_M2_7,
BMSG_M2_8,

BMSG_M3_1,
BMSG_M3_2,
BMSG_M3_3,
BMSG_M3_4,
BMSG_M3_5,
BMSG_M3_6,
BMSG_M3_7,
BMSG_M3_8,

BMSGS
};


struct msgstruct bmsg [BMSGS] =
{
 {" >> double autocannon $P $BPower: $C3.0 (x2) $PA basic fighter weapon. ", {0}},
 {" >> fighter $PStandard OCSF fighter. Has good armour and shielding and a powerful engine, but lacks the agility or firepower of the Angry Moth. Armed with double autocannon. ", {0}},
 {" >> anti-fighter missile $P $BPower: $C40.0 $PHigh-performance seeking missile with a sensor system capable of targetting small craft in a forward arc. Full lock-on maximises tracking capability. Low rate of fire. ", {0}},
 {" >> anti-warship missile $P $BPower: $C50.0 $PSlow missile with limited homing capability. Capable of targetting large ships only. Warhead is ineffective until missile drive fully engaged. ", {0}},
 {" >> multi-rockets $P $BPower: $C32.0 $PRocket launcher capable of loading and unleashing a stream of small rockets. ", {0}},
 {" >> torpedo $P $BPower: $C180.0 $PPowerful, but difficult to aim. Warhead is unstable and must prime both before and after launch - the torpedo will cause little harm if it explodes too early. ", {0}},
 {" >> single rocket $P $BPower: $C32.0 $PFires a single rocket. Lacks the rapid fire of the multiple rocket launcher but does not require pre-loading. ", {0}},
 {" >> tracking cannon $P $BPower: $C3.0 $PModifies the standard autocannons with electromagnets capable of deflecting fire towards targets within a narrow forward arc. ", {0}},
 {" >> heavy cannon $P $BPower: $C10.0 (x2) $PIncreases the power of the standard autocannons, at the cost of speed and rate of fire. ", {0}},
// {" $BAngry Moth 1 $Cleads a strike squadron to engage with any stragglers. ", {0}},
 {"Our deep tracking array at SK-9 has picked up a small raiding party which seems to have slipped past our outer defences. $P $P Most likely it is on its way to strike at our internal freight networks. ", {0}},
 {"We are scrambling a small interdiction force to catch the raiders while they recharge in low orbit around RL-3. $P  $P $B Alpha group $C consists of four light cruisers with fighter escort. ", {0}},
 {" $BAngry Moth $Cleads a strike squadron to engage with any stragglers. ", {0}},
 {"The precise size and composition of the enemy force is unknown, but we expect only small warships and standard fighters. ", {0}},
 {" $BAlpha group $Ccaptain has command in this engagement. ", {0}},
 {"Get as many of them as you can before they jump out! $P $P>> $BBriefing end$C ", {0}},

 {""},
 {""},
 {""},
 {""},
 {""},
 {""},
 {""},
 {""},

 {"Analysis of enemy fleet movements suggests the presence of an enemy outpost in the RO system. $P $P Our interstellar sensors have trouble detecting it directly, but we believe we have located a base in orbit around RO-2. ", {0}},
 {"Destroying this base would severely hamper the enemy's offensive in this sector. ", {0}},
 {" ", {0}},
 {"We can expect the base to be very heavily armed and shielded. It is also likely to be defended by large numbers of smaller ships. ", {0}},
 {" $BAlpha group $Cwill jump in and approach the base in an attempt to draw some of the defenders away. ", {0}},
 {" $BAngry Moth $Cwill join $BAlpha group's $Cdecoy attack. ", {0}},
 {" $BBeta Group $Cwill jump in about five minutes after $BAlpha group$C. $BAngry Moth $Cis to assist in the attack on the base. Hopefully you will have a clear run! ", {0}},
 {"Look out for enemy reinforcements! $P $P>> $BBriefing end$C ", {0}},

// remember trailing space!!
};



#define BSCRIPTS 300

struct briefingscript bscript [BSCRIPTS] =
{

 {BSCRIPT_HEADER, {1}},
// {BSCRIPT_BMESSAGE, {BMSG_M1_1, }}, // must always have a message - preferably straight after header
// {BSCRIPT_HIT_FIRE},
// {BSCRIPT_WAIT, {50}},

 {BSCRIPT_NEW_CONVOY, {BCONVOY_2, 350, 250, RIGHT}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_2, BSHIP_SCOUT2, 0, 0}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_2, BSHIP_FIGHTER, -5, -5}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_2, BSHIP_FIGHTER, -5, 5}},

 {BSCRIPT_NEW_CONVOY, {BCONVOY_3, 350, 200, RIGHT}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_3, BSHIP_SCOUT2, 0, 0}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_3, BSHIP_FIGHTER, -5, -5}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_3, BSHIP_FIGHTER, -5, 5}},

 {BSCRIPT_NEW_CONVOY, {BCONVOY_4, 350, 300, RIGHT}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_4, BSHIP_SCOUT2, 0, 0}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_4, BSHIP_FIGHTER, -5, -5}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_4, BSHIP_FIGHTER, -5, 5}},

 {BSCRIPT_NEW_CONVOY, {BCONVOY_5, 300, 225, RIGHT}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_5, BSHIP_SCOUT2, 0, 0}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_5, BSHIP_FIGHTER, -5, -5}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_5, BSHIP_FIGHTER, -5, 5}},

 {BSCRIPT_NEW_CONVOY, {BCONVOY_6, 300, 275, RIGHT}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_6, BSHIP_SCOUT2, 0, 0}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_6, BSHIP_FIGHTER, -5, -5}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_6, BSHIP_FIGHTER, -5, 5}},

 {BSCRIPT_BMESSAGE, {BMSG_M1_1, 150, 150, BOX_W + 10, 330, 260, -1}},

 {BSCRIPT_SELECT, {BCONVOY_2, 0, 0}},
 {BSCRIPT_WAIT, {20}},
 {BSCRIPT_SELECT, {BCONVOY_3, 0, 0}},
 {BSCRIPT_WAIT, {20}},
 {BSCRIPT_SELECT, {BCONVOY_4, 0, 0}},
 {BSCRIPT_WAIT, {20}},
 {BSCRIPT_SELECT, {BCONVOY_5, 0, 0}},
 {BSCRIPT_WAIT, {20}},
 {BSCRIPT_SELECT, {BCONVOY_6, 0, 0}},
 {BSCRIPT_SETTLE},
 {BSCRIPT_HIT_FIRE},

 {BSCRIPT_NEW_CONVOY, {BCONVOY_0, 50, 250, RIGHT}},
 {BSCRIPT_BMESSAGE, {BMSG_M1_2, 150, 450, BOX_W + 10, 0, 29, BCONVOY_0}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_OLD2, 0, 0}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_OLD2, -10, -7}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_OLD2, -10, 7}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_OLD2, -20, 0}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_FIGHTER_FRIEND, -2, -6}},
// {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_FIGHTER_FRIEND, -20, -14}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_FIGHTER_FRIEND, -2, 6}},
// {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_FIGHTER_FRIEND, -20, 14}},
/* {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_OLD2, -15, -15}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_OLD2, -15, 15}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_FIGHTER_FRIEND, -5, -15}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_FIGHTER_FRIEND, -5, 15}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_FIGHTER_FRIEND, -25, -25}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_FIGHTER_FRIEND, -25, 25}},*/
 {BSCRIPT_SELECT, {BCONVOY_0, 0, 0}},
 {BSCRIPT_SETTLE},
 {BSCRIPT_HIT_FIRE},

 {BSCRIPT_NEW_CONVOY, {BCONVOY_1, 70, 210, RIGHT}},
 {BSCRIPT_BMESSAGE, {BMSG_M1_3, 150, 450, BOX_W + 10, 5, 22, BCONVOY_1}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_MOTH, 0, 0}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_FIGHTER_FRIEND, -5, -5}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_FIGHTER_FRIEND, -5, 5}},
 {BSCRIPT_SELECT, {BCONVOY_1, 0, 0}},
 {BSCRIPT_SETTLE},
 {BSCRIPT_HIT_FIRE},

 {BSCRIPT_BMESSAGE, {BMSG_M1_4, 150, 150, BOX_W + 10, 330, 250, -1}},
 {BSCRIPT_HIT_FIRE},

 {BSCRIPT_BMESSAGE, {BMSG_M1_5, 150, 150, BOX_W + 10, 0, 0, -2}},
 {BSCRIPT_MOVE, {BCONVOY_0, 0, 260, 250, 3000, RIGHT, 1}},
 {BSCRIPT_MOVE, {BCONVOY_1, 0, 280, 210, 3000, RIGHT, 0}},
 {BSCRIPT_MOVE, {BCONVOY_2, 0, 550, 250, 1500, RIGHT, 0}},
 {BSCRIPT_MOVE, {BCONVOY_3, 0, 550, 200, 1500, RIGHT, 0}},
 {BSCRIPT_MOVE, {BCONVOY_4, 0, 550, 300, 1500, RIGHT, 0}},
 {BSCRIPT_MOVE, {BCONVOY_5, 0, 550, 225, 1500, RIGHT, 0}},
 {BSCRIPT_MOVE, {BCONVOY_6, 0, 550, 275, 1500, RIGHT, 0}},

 {BSCRIPT_SETTLE},
 {BSCRIPT_HIT_FIRE},

 {BSCRIPT_BMESSAGE, {BMSG_M1_6, 150, 150, BOX_W + 10, 0, 0, -2}},
 {BSCRIPT_WAIT, {100}},
 {BSCRIPT_END},


 {BSCRIPT_HEADER, {2}},

 {BSCRIPT_NEW_CONVOY, {BCONVOY_3, 305, 280, RIGHT}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_3, BSHIP_EBASE, 0, 0}},
 {BSCRIPT_BMESSAGE, {BMSG_M2_1, 500, 420, BOX_W + 10, 20, 11, BCONVOY_3}},
 {BSCRIPT_SELECT, {BCONVOY_3, 0, 0}},
 {BSCRIPT_SETTLE},
 {BSCRIPT_HIT_FIRE},

 {BSCRIPT_BMESSAGE, {BMSG_M2_2, 120, 420, BOX_W + 10, 20, 11, -2}},
 {BSCRIPT_SETTLE},
 {BSCRIPT_WAIT, {20}},
 {BSCRIPT_HIT_FIRE},

 {BSCRIPT_BMESSAGE, {BMSG_M2_3, 500, 420, BOX_W + 10, 20, 11, BCONVOY_3}},
 {BSCRIPT_SETTLE},
 {BSCRIPT_WAIT, {20}},
 {BSCRIPT_HIT_FIRE},

 {BSCRIPT_NEW_SHIP, {BCONVOY_3, BSHIP_SCOUT2, 40, -40}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_3, BSHIP_SCOUT2, -40, -40}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_3, BSHIP_SCOUT2, -70, 0}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_3, BSHIP_SCOUT2, 70, 0}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_3, BSHIP_SCOUT2, -40, 40}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_3, BSHIP_SCOUT2, 40, 40}},
 {BSCRIPT_SELECT, {BCONVOY_3, 0, 0}},
 {BSCRIPT_BMESSAGE, {BMSG_M2_4, 120, 420, BOX_W + 10, -80, 11, BCONVOY_3}},
 {BSCRIPT_SETTLE},
 {BSCRIPT_HIT_FIRE},

 {BSCRIPT_BMESSAGE, {BMSG_M2_5, 500, 420, BOX_W + 10, 20, 11, -2}},
 {BSCRIPT_SETTLE},
 {BSCRIPT_WAIT, {20}},
 {BSCRIPT_HIT_FIRE},


 {BSCRIPT_BMESSAGE, {BMSG_M2_6, 120, 220, BOX_W + 10, 20, 11, -2}},

 {BSCRIPT_NEW_CONVOY, {BCONVOY_0, 130, 450, RIGHT}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_OLD3, 0, 0}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_OLD2, -10, -10}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_OLD2, -10, 10}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_OLD2, 10, -10}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_OLD2, 10, 10}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_OLD2, -17, 0}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_FIGHTER_FRIEND, -2, -6}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_FIGHTER_FRIEND, -2, 6}},

 {BSCRIPT_NEW_CONVOY, {BCONVOY_1, 210, 50, RIGHT}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_OLD3, 0, 0}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_OLD2, -10, -10}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_OLD2, -10, 10}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_OLD2, 10, -10}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_OLD2, 10, 10}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_OLD2, -17, 0}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_FIGHTER_FRIEND, -2, -6}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_FIGHTER_FRIEND, -2, 6}},

 {BSCRIPT_NEW_CONVOY, {BCONVOY_2, 540, 330, LEFT}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_2, BSHIP_OLD3, 0, 0}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_2, BSHIP_OLD2, -10, -10}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_2, BSHIP_OLD2, -10, 10}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_2, BSHIP_OLD2, 10, -10}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_2, BSHIP_OLD2, 10, 10}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_2, BSHIP_OLD2, -17, 0}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_2, BSHIP_FIGHTER_FRIEND, -2, -6}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_2, BSHIP_FIGHTER_FRIEND, -2, 6}},

 {BSCRIPT_SELECT, {BCONVOY_0, 0, 0}},
 {BSCRIPT_WAIT, {20}},
 {BSCRIPT_SELECT, {BCONVOY_1, 0, 0}},
 {BSCRIPT_WAIT, {20}},
 {BSCRIPT_SELECT, {BCONVOY_2, 0, 0}},
 {BSCRIPT_SETTLE},
 {BSCRIPT_HIT_FIRE},

 {BSCRIPT_BMESSAGE, {BMSG_M2_7, 330, 480, BOX_W + 10, 40, -1, BCONVOY_0}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_MOTH, 25, -10}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_FIGHTER_FRIEND, 20, -15}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_FIGHTER_FRIEND, 20, -5}},
 {BSCRIPT_SELECT, {BCONVOY_0, 0, 0}},
 {BSCRIPT_SETTLE},
 {BSCRIPT_HIT_FIRE},

 {BSCRIPT_BMESSAGE, {BMSG_M2_8, 500, 170, BOX_W + 10, 0, 29, -2}},
 {BSCRIPT_MOVE, {BCONVOY_0, 0, 250, 380, 3000, RIGHT, 1}},
 {BSCRIPT_MOVE, {BCONVOY_1, 0, 310, 190, 3000, RIGHT, 1}},
 {BSCRIPT_MOVE, {BCONVOY_2, 0, 410, 300, 3000, LEFT, 1}},

 {BSCRIPT_SETTLE},
 {BSCRIPT_END},





 {BSCRIPT_HEADER, {3}},

 {BSCRIPT_NEW_CONVOY, {BCONVOY_3, 305, 280, RIGHT}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_3, BSHIP_EBASE, 0, 0}},
 {BSCRIPT_BMESSAGE, {BMSG_M3_1, 500, 420, BOX_W + 10, 20, 11, BCONVOY_3}},
 {BSCRIPT_SELECT, {BCONVOY_3, 0, 0}},
 {BSCRIPT_SETTLE},
 {BSCRIPT_HIT_FIRE},
 {BSCRIPT_CLEAR_MESSAGE},

 {BSCRIPT_BMESSAGE, {BMSG_M3_2, 120, 420, BOX_W + 10, 20, 11, -2}},
// {BSCRIPT_BMESSAGE, {BMSG_M3_2, 500, 420, BOX_W + 10, 20, 11, -2}},
 {BSCRIPT_SETTLE},
 {BSCRIPT_WAIT, {20}},
 {BSCRIPT_HIT_FIRE},
 {BSCRIPT_CLEAR_MESSAGE},

/* {BSCRIPT_BMESSAGE, {BMSG_M3_3, 500, 420, BOX_W + 10, 20, 11, -2}},
 {BSCRIPT_SETTLE},
 {BSCRIPT_WAIT, {20}},
 {BSCRIPT_HIT_FIRE},
 {BSCRIPT_CLEAR_MESSAGE},*/

 {BSCRIPT_NEW_CONVOY, {BCONVOY_4, 305, 280, RIGHT}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_4, BSHIP_SCOUT2, 20, -20}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_4, BSHIP_SCOUT2, -20, -20}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_4, BSHIP_SCOUT3, -30, 0}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_4, BSHIP_SCOUT3, 30, 0}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_4, BSHIP_SCOUT2, -20, 20}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_4, BSHIP_SCOUT2, 20, 20}},
 {BSCRIPT_SELECT, {BCONVOY_4, 0, 0}},
// {BSCRIPT_BMESSAGE, {BMSG_M3_4, 120, 420, BOX_W + 10, -40, 11, BCONVOY_4}},
 {BSCRIPT_BMESSAGE, {BMSG_M3_4, 500, 420, BOX_W + 10, 50, 11, BCONVOY_4}},
 {BSCRIPT_SETTLE},
 {BSCRIPT_HIT_FIRE},
 {BSCRIPT_CLEAR_MESSAGE},
/*
 {BSCRIPT_BMESSAGE, {BMSG_M2_5, 500, 420, BOX_W + 10, 20, 11, -2}},
 {BSCRIPT_SETTLE},
 {BSCRIPT_WAIT, {20}},
 {BSCRIPT_HIT_FIRE},
 {BSCRIPT_CLEAR_MESSAGE},
*/
// {BSCRIPT_BMESSAGE, {BMSG_M3_5, 120, 220, BOX_W + 10, 20, 11, -2}},

 {BSCRIPT_NEW_CONVOY, {BCONVOY_0, 300, 100, RIGHT}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_OLD2, -10, -10}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_OLD2, -10, 10}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_OLD2, 10, -10}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_OLD2, 10, 10}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_OLD2, -17, 0}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_FIGHTER_FRIEND, -2, -6}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_FIGHTER_FRIEND, -2, 6}},
 {BSCRIPT_SELECT, {BCONVOY_0, 0, 0}},
 {BSCRIPT_BMESSAGE, {BMSG_M3_5, 120, 120, BOX_W + 10, -15, 9, BCONVOY_0}},
 {BSCRIPT_SETTLE},
 {BSCRIPT_WAIT, {20}},
 {BSCRIPT_HIT_FIRE},
 {BSCRIPT_NEW_CONVOY, {BCONVOY_2, 330, 100, RIGHT}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_2, BSHIP_MOTH, 0, 0}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_2, BSHIP_FIGHTER_FRIEND, -6, 0}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_2, BSHIP_FIGHTER_FRIEND, 6, 0}},
 {BSCRIPT_SELECT, {BCONVOY_2, 0, 0}},
 {BSCRIPT_BMESSAGE, {BMSG_M3_6, 480, 150, BOX_W + 10, 21, 9, BCONVOY_2}},
 {BSCRIPT_SETTLE},
 {BSCRIPT_WAIT, {20}},
 {BSCRIPT_HIT_FIRE},
 {BSCRIPT_CLEAR_MESSAGE},


 {BSCRIPT_MOVE, {BCONVOY_0, 0, 305, 200, 3000, RIGHT, 1}},
 {BSCRIPT_MOVE, {BCONVOY_2, 0, 335, 200, 3000, RIGHT, 0}},
 {BSCRIPT_SETTLE},
 {BSCRIPT_MOVE, {BCONVOY_0, 0, 35, 220, 3500, LEFT, 1}},
 {BSCRIPT_MOVE, {BCONVOY_2, 0, 65, 220, 3500, LEFT, 0}},
 {BSCRIPT_MOVE, {BCONVOY_4, 0, 105, 270, 2600, LEFT, 0}},
 {BSCRIPT_SETTLE},

 {BSCRIPT_NEW_CONVOY, {BCONVOY_1, 330, 480, LEFT}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_OLD3, 0, 0}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_OLD3, 10, 0}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_OLD3, -10, 0}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_OLD2, 0, -10}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_OLD2, -13, -10}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_OLD2, 13, -10}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_OLD2, 0, 10}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_OLD2, 0, -17}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_FIGHTER_FRIEND, 18, -15}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_FIGHTER_FRIEND, -18, -15}},
 {BSCRIPT_SELECT, {BCONVOY_1, 0, 0}},
 {BSCRIPT_BMESSAGE, {BMSG_M3_7, 140, 380, BOX_W + 10, -20, 7, BCONVOY_1}},
 {BSCRIPT_SETTLE},
 {BSCRIPT_WAIT, {20}},
 {BSCRIPT_HIT_FIRE},
 {BSCRIPT_CLEAR_MESSAGE},
 {BSCRIPT_MOVE, {BCONVOY_1, 0, 305, 320, 2500, LEFT, 1}},
 {BSCRIPT_MOVE, {BCONVOY_2, 0, 305, 250, 3000, RIGHT, 1}},
 {BSCRIPT_SETTLE},
 {BSCRIPT_HIT_FIRE},

 {BSCRIPT_NEW_CONVOY, {BCONVOY_5, 455, 310, LEFT}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_5, BSHIP_SCOUTCAR, -10, 0}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_5, BSHIP_SCOUTCAR, 10, 0}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_5, BSHIP_SCOUT2, -5, -10}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_5, BSHIP_SCOUT2, -5, 10}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_5, BSHIP_FIGHTER, -10, -15}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_5, BSHIP_FIGHTER, -15, -20}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_5, BSHIP_FIGHTER, -10, 15}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_5, BSHIP_FIGHTER, -15, 20}},
 {BSCRIPT_SELECT, {BCONVOY_5, 0, 0}},
 {BSCRIPT_BMESSAGE, {BMSG_M3_8, 490, 120, BOX_W + 10, 10, -35, BCONVOY_5}},
 {BSCRIPT_SETTLE},
 {BSCRIPT_WAIT, {20}},
 {BSCRIPT_HIT_FIRE},

 {BSCRIPT_END},

/*

 {BSCRIPT_HIT_FIRE},
 {BSCRIPT_SELECT, {BCONVOY_0, -15, 0}},
 {BSCRIPT_SELECT, {BCONVOY_1, -10, 0}},
 {BSCRIPT_SETTLE},
 {BSCRIPT_MOVE, {BCONVOY_0, 0, 200, 250, 3000, RIGHT}},
 {BSCRIPT_MOVE, {BCONVOY_1, 0, 200, 150, 2000, LEFT}},
 {BSCRIPT_SETTLE},
// {BSCRIPT_HIT_FIRE},
 {BSCRIPT_MOVE, {BCONVOY_0, 0, 100, 250, 3000, LEFT}},
 {BSCRIPT_MOVE, {BCONVOY_1, 20, 300, 210, 3500, LEFT}},
 {BSCRIPT_SETTLE},
// {BSCRIPT_HIT_FIRE},
 {BSCRIPT_END},
*/
/*
 {BSCRIPT_HEADER, {0}},
 {BSCRIPT_BMESSAGE, {BMSG_HELLO}}, // must always have a message - preferably straight after header
 {BSCRIPT_NEW_CONVOY, {BCONVOY_0, 100, 200, RIGHT}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_OLD3, 0, 0}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_OLD2, -10, -5}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_OLD2, -10, 5}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_MOTH, -15, 10}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_FIGHTER_FRIEND, -20, 15}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_FIGHTER_FRIEND, -15, -10}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_0, BSHIP_FIGHTER_FRIEND, -20, -15}},
 {BSCRIPT_NEW_CONVOY, {BCONVOY_1, 300, 200, LEFT}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_SCOUT3_HEAVY, 0, 0}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_SCOUT2_HEAVY, 13, -4}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_1, BSHIP_SCOUT2_HEAVY, 13, 4}},
 {BSCRIPT_NEW_CONVOY, {BCONVOY_2, 300, 300, LEFT}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_2, BSHIP_SCOUTBASE, 0, 0}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_2, BSHIP_SCOUTCAR, 18, -8}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_2, BSHIP_SCOUT2_HEAVY, 18, 8}},
 {BSCRIPT_NEW_CONVOY, {BCONVOY_3, 100, 50, LEFT}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_3, BSHIP_SCOUTBASE, 0, 0}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_3, BSHIP_SCOUTCAR, 18, -8}},
 {BSCRIPT_NEW_SHIP, {BCONVOY_3, BSHIP_SCOUT2_HEAVY, 18, 8}},
 {BSCRIPT_HIT_FIRE},
 {BSCRIPT_SELECT, {BCONVOY_0, -15, 0}},
 {BSCRIPT_SELECT, {BCONVOY_1, -10, 0}},
 {BSCRIPT_SETTLE},
 {BSCRIPT_MOVE, {BCONVOY_0, 0, 200, 250, 3000, RIGHT}},
 {BSCRIPT_MOVE, {BCONVOY_1, 0, 200, 150, 2000, LEFT}},
 {BSCRIPT_SETTLE},
// {BSCRIPT_HIT_FIRE},
 {BSCRIPT_MOVE, {BCONVOY_0, 0, 100, 250, 3000, LEFT}},
 {BSCRIPT_MOVE, {BCONVOY_1, 20, 300, 210, 3500, LEFT}},
 {BSCRIPT_SETTLE},
// {BSCRIPT_HIT_FIRE},*/
// {BSCRIPT_END}

};



/*
char bmsg [BMSGS] [150] =
{
 "Anti-fighter missile. Very effective against fighters. Lock on to target. ",
 "Anti-warship missile. Locks on to warships only. Powerful. ",
 "Torpedo. Most powerful weapon. ",
 "Rocket. ",
 "Hello everyone this is a $B test message $C to see how this msg thing works. $P Does it? $P  $P Let's find out! Also this is testing the small font. "

// remember trailing space!!
};
*/

int waiting;
int waiting_for_fire;
int pos;
int selecting [2];

enum
{
BOVER_MENU_START,
BOVER_MENU_REPLAY,
BOVER_MENU_DATA,
BOVER_MENU_QUIT,
BOVER_MENU_END
};

/*
#define BCOL_BACK (COL_F1 + TRANS_BLUE1)
#define BCOL_SCREEN (COL_F1)
#define BCOL_DARK (COL_STAR1)
#define BCOL_EDGE (COL_F2)
#define BCOL_TEXT (COL_F3 + TRANS_BLUE3)
*/
#define BCOL_BACK (COL_BOX1)
#define BCOL_SCREEN (COL_BOX1)
#define BCOL_DARK (COL_BOX0)
#define BCOL_EDGE (COL_BOX2)
#define BCOL_TEXT (COL_BOX4)
//(COL_F6 + TRANS_BLUE3)


#define EDGE 1


enum
{
CHOOSE_CANNON,
CHOOSE_WING1,
CHOOSE_WEAPON1,
CHOOSE_WEAPON2,
CHOOSE_LAUNCH,
CHOOSE_REPLAY,
CHOOSE_END

};


void init_mission_briefing(void)
{

 int c, s, i;

 for (c = 0; c < BCONVOYS; c ++)
 {
  bconvoy[c].exists = 0;
 }
 for (s = 0; s < BSHIPS; s ++)
 {
  bship[s].type = BSHIP_NONE;
 }
 for (s = 0; s < BSELECT; s ++)
 {
  bselect[s].exists = 0;
 }

 pos = 0;

 for (i = 0; i < BSCRIPTS; i ++)
 {
  if (bscript [i].type == BSCRIPT_HEADER
   && bscript [i].var [0] == arena.stage)
  {
   pos = i;
   break;
  }
 }

 bline[0].text [0] = END_MESSAGE;
 tbox_exists = 0;

 waiting = 0;
 waiting_for_fire = 0;


}

int mission_briefing(void)
{

 init_mission_briefing();

 int menu_return = -1;
 char replay = 0;

 do
 {

  replay = 0;

  if (briefing_loop() == 1)
   return 0; // must have quit

  if (bscript[pos].type == BSCRIPT_END
   || pressing_a_key(0, CKEY_FIRE2, JBUTTON_FIRE2))
   {
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
        return 0;
       }
       break;

     }
    } while (replay == 0);
   }


 } while (TRUE);

 return 1;

}

int briefing_loop(void)
{

  if (key[KEY_ESC])
  {
   return ask_bover_quit();
  }

  briefing_counter ++;

  run_bconvoys();

/*  if (tbox_exists)
  {
   run_tbox();
   if (tbox_in <= 0
    && tbox_flash_out <= 0
    && tbox_out <= 0)
    {
       check_for_fire();
       if (waiting_for_fire == 0)
        tbox_flash_out = 20;
    }
  }*/
  if (tbox_exists)
  {
   run_tbox();
/*   if (tbox_in <= 0
    && tbox_flash_out <= 0
    && tbox_out <= 0)
    {
       check_for_fire();
       if (waiting_for_fire == 0)
        tbox_flash_out = 20;
    }*/
  }
//   else
   {
    if (bscript[pos].type == BSCRIPT_SETTLE)
    {
     if (check_settled() == 0)
      waiting = 1;
    } // wait for all convoys to stop moving

    if (waiting > 0)
    {
     waiting --;
    }
     else
     {
      if (waiting_for_fire > 0)
      {
       check_for_fire();
      }
       else
       {
        run_bscripts();
       }
     }
   }

//  if (key [KEY_ESC])
//   exit(5);

  bdisplay();

  vsync();
  blit(display[0], screen, 0, 0, 0, 0, 800, 600);

    do
    {
        rest(1);
//        thing ++;
    } while (ticked == 0);
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

 clear_to_color(display [0], 0);

 rectfill(display[0], MAP_X-EDGE, MAP_Y-EDGE, MAP_X + MAP_W+EDGE, MAP_Y + MAP_H+EDGE, BCOL_EDGE);
 rectfill(display[0], MAP_X, MAP_Y, MAP_X + MAP_W, MAP_Y + MAP_H, BCOL_DARK);

 rectfill(display[0], MAP_X, MAP_Y + 5, MAP_X + MAP_W, MAP_Y + 22, BCOL_EDGE);
 textprintf_centre_ex(display[0], small_font, MAP_X + (MAP_W>>1), MAP_Y+7, BCOL_TEXT, -1, "Mission Briefing");


 int s, c;
// int col;
/*
 for (c = 0; c < BCONVOYS; c ++)
 {
  if (bconvoy [c].exists == 0)
   continue;
  textprintf_ex(display[0], small_font, MAP_X + (bconvoy[c].x>>10), MAP_Y + (bconvoy[c].y>>10), -1, -1, "%i, %i", bconvoy[c].x>>10, bconvoy[c].y>>10);
 }*/

  for (s = 0; s < BSHIPS; s ++)
  {
   if (bship[s].type == BSHIP_NONE)
    continue;
   if (bconvoy[bship[s].convoy].face == RIGHT)
    draw_sprite(display[0], bsprite [bship[s].side] [get_bsprite(bship[s].type)] [0], MAP_X + (bconvoy[bship[s].convoy].x>>10) + bship[s].x - 6, MAP_Y + (bconvoy[bship[s].convoy].y>>10) + bship[s].y - 6);
     else
      draw_sprite_h_flip(display[0], bsprite [bship[s].side] [get_bsprite(bship[s].type)] [0], MAP_X + (bconvoy[bship[s].convoy].x>>10) - bship[s].x - 6, MAP_Y + (bconvoy[bship[s].convoy].y>>10) - bship[s].y - 6);

//   circlefill(display[0], MAP_X + (bconvoy[bship[s].convoy].x>>10) + bship[s].x, MAP_Y + (bconvoy[bship[s].convoy].x>>10) + bship[s].y, 2, col);
  }

  for (s = 0; s < BSELECT; s ++)
  {
   if (bselect[s].exists == 0
    || bselect[s].count <= 0)
    continue;
   c = bselect[s].count*3 - 10 - 20;
   if (c < 10)
    c = 10;
   vline(display[0], bselect[s].x + bselect[s].x1 - c, bselect[s].y + bselect[s].y1 - c, bselect[s].y + bselect[s].y1 - c + 4, COL_F5 + TRANS_BLUE3);
   hline(display[0], bselect[s].x + bselect[s].x1 - c, bselect[s].y + bselect[s].y1 - c, bselect[s].x + bselect[s].x1 - c + 4, COL_F5 + TRANS_BLUE3);

   vline(display[0], bselect[s].x + bselect[s].x2 + c, bselect[s].y + bselect[s].y1 - c, bselect[s].y + bselect[s].y1 - c + 4, COL_F5 + TRANS_BLUE3);
   hline(display[0], bselect[s].x + bselect[s].x2 + c, bselect[s].y + bselect[s].y1 - c, bselect[s].x + bselect[s].x2 + c - 4, COL_F5 + TRANS_BLUE3);

   vline(display[0], bselect[s].x + bselect[s].x1 - c, bselect[s].y + bselect[s].y2 + c, bselect[s].y + bselect[s].y2 + c - 4, COL_F5 + TRANS_BLUE3);
   hline(display[0], bselect[s].x + bselect[s].x1 - c, bselect[s].y + bselect[s].y2 + c, bselect[s].x + bselect[s].x1 - c + 4, COL_F5 + TRANS_BLUE3);

   vline(display[0], bselect[s].x + bselect[s].x2 + c, bselect[s].y + bselect[s].y2 + c, bselect[s].y + bselect[s].y2 + c - 4, COL_F5 + TRANS_BLUE3);
   hline(display[0], bselect[s].x + bselect[s].x2 + c, bselect[s].y + bselect[s].y2 + c, bselect[s].x + bselect[s].x2 + c - 4, COL_F5 + TRANS_BLUE3);
/*
   vline(display[0], MAP_X + bselect[s].x - bselect[s].x1 - c, MAP_Y + bselect[s].y - bselect[s].y1 - c, MAP_Y + bselect[s].y - bselect[s].y1 - c + 4, COL_F5 + TRANS_BLUE3);
   hline(display[0], MAP_X + bselect[s].x - bselect[s].x1 - c, MAP_Y + bselect[s].y - bselect[s].y1 - c, MAP_X + bselect[s].x - bselect[s].x1 - c + 4, COL_F5 + TRANS_BLUE3);

   vline(display[0], MAP_X + bselect[s].x + bselect[s].x2 + c, MAP_Y + bselect[s].y - bselect[s].y1 - c, MAP_Y + bselect[s].y - bselect[s].y1 - c + 4, COL_F5 + TRANS_BLUE3);
   hline(display[0], MAP_X + bselect[s].x + bselect[s].x2 + c, MAP_Y + bselect[s].y - bselect[s].y1 - c, MAP_X + bselect[s].x + bselect[s].x2 + c - 4, COL_F5 + TRANS_BLUE3);

   vline(display[0], MAP_X + bselect[s].x - bselect[s].x1 - c, MAP_Y + bselect[s].y + bselect[s].y2 + c, MAP_Y + bselect[s].y + bselect[s].y2 + c - 4, COL_F5 + TRANS_BLUE3);
   hline(display[0], MAP_X + bselect[s].x - bselect[s].x1 - c, MAP_Y + bselect[s].y + bselect[s].y2 + c, MAP_X + bselect[s].x - bselect[s].x1 - c + 4, COL_F5 + TRANS_BLUE3);

   vline(display[0], MAP_X + bselect[s].x + bselect[s].x2 + c, MAP_Y + bselect[s].y + bselect[s].y2 + c, MAP_Y + bselect[s].y + bselect[s].y2 + c - 4, COL_F5 + TRANS_BLUE3);
   hline(display[0], MAP_X + bselect[s].x + bselect[s].x2 + c, MAP_Y + bselect[s].y + bselect[s].y2 + c, MAP_X + bselect[s].x + bselect[s].x2 + c - 4, COL_F5 + TRANS_BLUE3);
*/
//   textprintf_ex(display[0], small_font, MAP_X + (bselect[s].x) + 30, MAP_Y + (bselect[s].y), -1, -1, "%i, %i, %i, %i", bselect[s].x1, bselect[s].y1, bselect[s].x2,bselect[s].y2);

  }


/*
 rectfill(display[0], BOX_X-EDGE, BOX_Y-EDGE, BOX_X + BOX_W+EDGE, BOX_Y + BOX_H+EDGE, BCOL_EDGE);
 rectfill(display[0], BOX_X, BOX_Y, BOX_X + BOX_W, BOX_Y + BOX_H, BCOL_SCREEN);

 rectfill(display[0], BOX_X, BOX_Y + 5, BOX_X + BOX_W, MAP_Y + 35, BCOL_EDGE);
 textprintf_centre_ex(display[0], small_font, BOX_X+(BOX_W>>1), BOX_Y+8, BCOL_TEXT, -1, "Mission Briefing");
 textprintf_centre_ex(display[0], small_font, BOX_X+(BOX_W>>1), BOX_Y+20, BCOL_TEXT, -1, "Instructions");

 hline(display[0], MAP_X + MAP_W+1, MAP_Y + 100, BOX_X-9, BCOL_EDGE);
 vline(display[0], BOX_X - 9, MAP_Y + 100, MAP_Y + MAP_H - 100, BCOL_EDGE);
 hline(display[0], BOX_X - 9, MAP_Y + MAP_H - 100, BOX_X-1, BCOL_EDGE);
*/
// display_message(display[0], bline, BOX_X + 5, BOX_Y + 45);

 rectfill(display[0], 250, 575, 550, 595, BCOL_EDGE);

 vline(display[0], MAP_X + MAP_W - 80, MAP_Y + MAP_H + 2, MAP_Y + MAP_H + 8, BCOL_EDGE);
 hline(display[0], 320, MAP_Y + MAP_H + 8, MAP_X + MAP_W - 80, BCOL_EDGE);
 vline(display[0], 320, MAP_Y + MAP_H + 8, 595 - EDGE, BCOL_EDGE);

 if (tbox_exists)
  draw_tbox();

 if (waiting_for_fire)
 {
   rectfill(display[0], 250+EDGE, 575+EDGE, 550-EDGE, 595-EDGE, COL_BOX2);
   textprintf_centre_ex(display[0], small_font, 400, 580, BCOL_TEXT, -1, "Press fire 1 to continue or fire 2 to skip");
   draw_arrows(400, 580, 115, 2, briefing_counter);
 }
  else
   rectfill(display[0], 250+EDGE, 575+EDGE, 550-EDGE, 595-EDGE, BCOL_DARK);

}

int get_bsprite(int type)
{

// if (type == BSHIP_MOTH)
// return BSF_OLD3;

 switch(type)
 {
  case BSHIP_OLD2: return BSF_OLD2;
  case BSHIP_OLD3: return BSF_OLD3;
  case BSHIP_FIGHTER_FRIEND: return BSF_FIGHTER;
  case BSHIP_MOTH: return BSF_MOTH;

  case BSHIP_FIGHTER: return BSE_FIGHTER;
  case BSHIP_BOMBER: return BSE_FIGHTER;
  case BSHIP_EINT: return BSE_FIGHTER;
  case BSHIP_ESCOUT: return BSE_FIGHTER;
  case BSHIP_SCOUT2: return BSE_SCOUT2;
  case BSHIP_SCOUT3: return BSE_SCOUT3;
  case BSHIP_SCOUTCAR: return BSE_SCOUTCAR;
  case BSHIP_EBASE: return BSE_EBASE;

  default: return BSE_EBASE; // this is BSF_MOTH for friendly, I think

 }

// return BSF_OLD3;


}

int get_bsprite_from_ship(int type)
{


 switch(type)
 {
  case SHIP_OLD2: return BSF_OLD2;
  case SHIP_OLD3: return BSF_OLD3;
  case SHIP_FIGHTER_FRIEND: return BSF_FIGHTER;

  case SHIP_FIGHTER: return BSE_FIGHTER;
  case SHIP_BOMBER: return BSE_FIGHTER;
  case SHIP_ESCOUT: return BSE_FIGHTER;
  case SHIP_EINT: return BSE_FIGHTER;
  case SHIP_SCOUT2: return BSE_SCOUT2;
  case SHIP_SCOUT3: return BSE_SCOUT3;
  case SHIP_SCOUTCAR: return BSE_SCOUTCAR;
  case SHIP_EBASE: return BSE_EBASE;

  default: return BSE_EBASE;

 }

}



void run_bscripts(void)
{

 int c, s;
 float f_angle;

 do
 {
  pos ++;

  switch(bscript[pos].type)
  {
   case BSCRIPT_NEW_CONVOY:
    c = bscript[pos].var[BNEWCONVOY_CONVOY];
    bconvoy [c].x = bscript[pos].var[BNEWCONVOY_X]<<10;
    bconvoy [c].y = bscript[pos].var[BNEWCONVOY_Y]<<10;
    bconvoy [c].exists = 1;
    bconvoy [c].x_speed = 0;
    bconvoy [c].y_speed = 0;
    bconvoy [c].face = bscript[pos].var[BNEWCONVOY_FACE];
    break;
   case BSCRIPT_NEW_SHIP:
    c = bscript[pos].var[BNEWSHIP_BCONVOY];
    for (s = 0; s < BSHIPS; s ++)
    {
     if (bship[s].type == BSHIP_NONE)
     {
      bship[s].type = bscript[pos].var[BNEWSHIP_TYPE];
      bship[s].convoy = bscript[pos].var[BNEWSHIP_BCONVOY];
      bship[s].x = bscript[pos].var[BNEWSHIP_X];
      bship[s].y = bscript[pos].var[BNEWSHIP_Y];
      if (bship[s].type < BSHIP_END_FRIENDS)
       bship[s].side = TEAM_FRIEND;
        else
         bship[s].side = TEAM_ENEMY;
      break;
     }
    }
    break;
   case BSCRIPT_MOVE:
    c = bscript[pos].var[BMOVE_CONVOY];
    f_angle = atan2((bscript[pos].var[BMOVE_Y]<<10) - bconvoy[c].y, (bscript[pos].var[BMOVE_X]<<10) - bconvoy[c].x);
//    if (dist > 0)
    {
     bconvoy[c].x_speed = fxpart(f_angle, bscript[pos].var[BMOVE_SPEED]);
     bconvoy[c].y_speed = fypart(f_angle, bscript[pos].var[BMOVE_SPEED]);
    }
    bconvoy[c].goal_x = bscript[pos].var[BMOVE_X]<<10;
    bconvoy[c].goal_y = bscript[pos].var[BMOVE_Y]<<10;
    bconvoy[c].face = bscript[pos].var[BMOVE_FACE];
    bconvoy[c].move_settle = bscript[pos].var[BMOVE_SETTLE];
    break;
   case BSCRIPT_WAIT:
    waiting = bscript[pos].var[BWAIT_TIME];
    break;
   case BSCRIPT_HIT_FIRE:
    waiting_for_fire = 2;
    break;
   case BSCRIPT_BMESSAGE:
    create_tbox(bscript[pos].var[BMESSAGE_BMESSAGE],
     bscript[pos].var[BMESSAGE_X], bscript[pos].var[BMESSAGE_Y],
     bscript[pos].var[BMESSAGE_W], bscript[pos].var[BMESSAGE_X2],
     bscript[pos].var[BMESSAGE_Y2], bscript[pos].var[BMESSAGE_CONVOY]);
    break;
   case BSCRIPT_CLEAR_MESSAGE:
    tbox_exists = 0;
    break;
   case BSCRIPT_SELECT:
    for (s = 0; s < BSELECT; s ++)
    {
     if (bselect[s].exists == 0)
     {
      bselect[s].x = (bconvoy[bscript[pos].var[BSELECT_CONVOY]].x >> 10) + bscript[pos].var[BSELECT_X_OFFSET] + MAP_X;// + 5;
      bselect[s].y = (bconvoy[bscript[pos].var[BSELECT_CONVOY]].y >> 10) + bscript[pos].var[BSELECT_Y_OFFSET] + MAP_Y;// + 5;
      bselect[s].x1 = -5;
      bselect[s].y1 = -5;
      bselect[s].x2 = 5;
      bselect[s].y2 = 5;
      for (c = 0; c < BSHIPS; c ++)
      {
       if (bship[c].type != BSHIP_NONE
        && bship[c].convoy == bscript[pos].var[BSELECT_CONVOY])
        {
         if (bship[c].x < bselect[s].x1 - 0)
          bselect[s].x1 = bship[c].x - 0;
         if (bship[c].y < bselect[s].y1 - 0)
          bselect[s].y1 = bship[c].y - 0;
         if (bship[c].x > bselect[s].x2 + 0)
          bselect[s].x2 = bship[c].x + 0;
         if (bship[c].y > bselect[s].y2 + 0)
          bselect[s].y2 = bship[c].y + 0;
/*         if (bship[c].x < bselect[s].x1 + 15)
          bselect[s].x1 = bship[c].x - 15;
         if (bship[c].y < bselect[s].y1 + 15)
          bselect[s].y1 = bship[c].y - 15;
         if (bship[c].x > bselect[s].x2 - 15)
          bselect[s].x2 = bship[c].x + 15;
         if (bship[c].y > bselect[s].y2 - 15)
          bselect[s].y2 = bship[c].y + 15;*/
        }
      }
      bselect[s].exists = 1;
      bselect[s].count = 25;
      break;
     }
    }
    break;

  }

 } while (bscript [pos].type != BSCRIPT_WAIT
       && bscript [pos].type != BSCRIPT_HIT_FIRE
       && bscript [pos].type != BSCRIPT_SETTLE
       && bscript [pos].type != BSCRIPT_END);

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
 if (tbox_w > tbox_h)
  tbox_in = tbox_w;

 tbox_x = x;
 tbox_y = y;
 tbox_x2 = x2;
 tbox_y2 = y2;

 if (cv == -2)
 {
  tbox_x2 = x;
  tbox_y2 = y;
 }

 if (cv >= 0)
 {
  tbox_x2 += bconvoy[cv].x>>10;
  tbox_y2 += bconvoy[cv].y>>10;
 }

}


void draw_tbox(void)
{

 int w, h;
 int col = COL_BOX4;


 if (tbox_x2 != tbox_x || tbox_y2 != tbox_y)
 {
  if (tbox_x2 < tbox_x - tbox_w - 10)
  {
   hline(display[0], tbox_x, tbox_y, tbox_x - tbox_w - 10, col);
   vline(display[0], tbox_x - tbox_w - 10, tbox_y, tbox_y2, col);
   hline(display[0], tbox_x - tbox_w - 10, tbox_y2, tbox_x2, col);
  }
   else
   {
    if (tbox_x2 > tbox_x + tbox_w + 10)
    {
     hline(display[0], tbox_x, tbox_y, tbox_x + tbox_w + 10, col);
     vline(display[0], tbox_x + tbox_w + 10, tbox_y, tbox_y2, col);
     hline(display[0], tbox_x + tbox_w + 10, tbox_y2, tbox_x2, col);
    }
     else
     {
      if (tbox_y2 < tbox_y)
      {
       vline(display[0], tbox_x, tbox_y, tbox_y - tbox_h - 10, col);
       hline(display[0], tbox_x, tbox_y - tbox_h - 10, tbox_x2, col);
       vline(display[0], tbox_x2, tbox_y - tbox_h - 10, tbox_y2, col);
      }
       else
       {
        vline(display[0], tbox_x, tbox_y, tbox_y + tbox_h + 10, col);
        hline(display[0], tbox_x, tbox_y + tbox_h + 10, tbox_x2, col);
        vline(display[0], tbox_x2, tbox_y + tbox_h + 10, tbox_y2, col);
       }
     }
    }
  }


 if (tbox_in > 0)
 {
  w = tbox_w - tbox_in;
  if (w < 0)
   w = 0;
  h = tbox_h - tbox_in;
  if (h < 0)
   h = 0;
  rectfill(display[0], tbox_x - w, tbox_y - h, tbox_x + w, tbox_y + h, COL_BOX4);
  return;
 }

 if (tbox_out > 0)
 {
   w = tbox_out;
   if (w > tbox_w)
    w = tbox_w;
   h = tbox_out;
   if (h > tbox_h)
    h = tbox_h;
   rectfill(display[0], tbox_x - w, tbox_y - h, tbox_x + w, tbox_y + h, COL_BOX4);
   return;
 }

 col = COL_BOX1;

 if (tbox_flash_in > 0)
 {
  col = COL_BOX1 + tbox_flash_in / 5;
  if (col > COL_BOX4)
   col = COL_BOX4;
 }

 if (tbox_flash_out > 0)
 {
  col = COL_BOX4 - (tbox_flash_out / 5);
  if (col < COL_BOX1)
   col = COL_BOX1;
 }

 rectfill(display[0], tbox_x - tbox_w, tbox_y - tbox_h, tbox_x + tbox_w, tbox_y + tbox_h, col);
 col ++;
 if (col > COL_BOX4)
  col = COL_BOX4;
 rect(display[0], tbox_x - tbox_w, tbox_y - tbox_h, tbox_x + tbox_w, tbox_y + tbox_h, col);

 display_message(display[0], bline, tbox_x - tbox_w + 6, tbox_y - tbox_h + 6);


}

void run_tbox(void)
{

 if (tbox_in > 0)
 {
  tbox_in -= 10;
  if (tbox_in <= 0)
  {
   tbox_flash_in = 20;
//   waiting_for_fire = 2;
  }
  return;
 }

 if (tbox_flash_in > 0)
  tbox_flash_in -= 2;

 if (tbox_flash_out > 0)
 {
  tbox_flash_out --;
  if (tbox_flash_out <= 0)
  {
   tbox_out = tbox_w;
   if (tbox_h > tbox_out)
    tbox_out = tbox_h;
  }
 }

 if (tbox_out > 0)
 {
  tbox_out -= 3;
  if (tbox_out <= 0)
   tbox_exists = 0;
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

int get_bover_input(int p, int min, int max, int select2, int min2, int max2, int skip);

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
  input_value = get_bover_input(0, BOVER_MENU_START, BOVER_MENU_END - 1, 0, 0, 0, -1);
  if (input_value != -1)
   return bover_menu_select [0];

/*   if (key [options.ckey [0] [CKEY_FIRE2]]
    || (options.joystick_available [0] && joy[0].button[options.joy_button [0] [1]].b))
    break;*/

    do
    {
        rest(1);
//        thing ++;
    } while (ticked == 0);
    ticked = 0;


  vsync();
  blit(display[0], screen, 0, 0, 0, 0, 800, 600);



 };

// briefing_over_menu();

}

// "skip" is only used for the main selection, and currently only used
//  to skip third weapon selection in 2-p games.
int get_bover_input(int p, int min, int max, int select2, int min2, int max2, int skip)
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
   bover_menu_select [p] --;
   if (bover_menu_select [p] < min)
    bover_menu_select [p] = max;
   if (bover_menu_select [p] == skip)
    bover_menu_select [p] --;
   play_basicwfv(WAV_SELECT0, FREQ_BSELECT, VOL_BSELECT1);
  }

  if (move == 1)
  {
   bover_menu_select [p] ++;
   if (bover_menu_select [p] > max)
    bover_menu_select [p] = min;
   if (bover_menu_select [p] == skip)
    bover_menu_select [p] ++;
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

 if (col < COL_BOX4)
 {
 rectfill(display[0], BOX_X + 5, BOX_Y + (BOX_H>>1) - 30 + (30*bover_menu_select [0]), BOX_X + BOX_W - 5, BOX_Y + (BOX_H>>1) - 5 + (30*bover_menu_select [0]), col2);
 rect(display[0], BOX_X + 5, BOX_Y + (BOX_H>>1) - 30 + (30*bover_menu_select [0]), BOX_X + BOX_W - 5, BOX_Y + (BOX_H>>1) - 5 + (30*bover_menu_select [0]), col3);

 textprintf_centre_ex(display[0], small_font, BOX_X + (BOX_W>>1), BOX_Y + (BOX_H>>1) - 30+5, BCOL_TEXT, -1, "prepare to launch >>>");
 textprintf_centre_ex(display[0], small_font, BOX_X + (BOX_W>>1), BOX_Y + (BOX_H>>1) - 0+5, BCOL_TEXT, -1, "replay briefing");
 textprintf_centre_ex(display[0], small_font, BOX_X + (BOX_W>>1), BOX_Y + (BOX_H>>1) + 30+5, BCOL_TEXT, -1, "mission data");
 textprintf_centre_ex(display[0], small_font, BOX_X + (BOX_W>>1), BOX_Y + (BOX_H>>1) + 60+5, BCOL_TEXT, -1, "quit");
 }

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
 rectfill(display[0], 250, 200, 550, 310, COL_E1);
 rect(display[0], 250, 200, 550, 310, COL_E3);
 textprintf_centre_ex(display[0], small_font, 400, 230, COL_E5 + TRANS_RED2, -1, "QUIT GAME?");
 textprintf_centre_ex(display[0], small_font, 400, 260, COL_E5 + TRANS_RED3, -1, "press 'Y' to quit to main menu");
 textprintf_centre_ex(display[0], small_font, 400, 275, COL_E5 + TRANS_RED3, -1, "or 'N' to play on");

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

    do
    {
        rest(1);
    } while (ticked == 0);
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
      case 3: col = COL_E1 + TRANS_RED1; break;
      case 2: col = COL_E1 + TRANS_RED2; break;
      case 1: col = COL_E2 + TRANS_RED3; break;
      case 0: col = COL_E3 + TRANS_RED4; break;
     }
     break;
    case 2:
     switch(col)
     {
      case 3: col = COL_F1 + TRANS_BLUE1; break;
      case 2: col = COL_F1 + TRANS_BLUE2; break;
      case 1: col = COL_F2 + TRANS_BLUE3; break;
      case 0: col = COL_F3 + TRANS_BLUE4; break;
     }
     break;
   }
   textprintf_centre_ex(display[0], small_font, x - w - i*3, y, col, -1, ">");
   textprintf_centre_ex(display[0], small_font, x + w + i*3, y, col, -1, "<");
  }


}

#define WPNS 7


void bover_box(int x, int y);

int choose_weapons(void)
{

// char ckey_wait [2] = {10, 10};
 player[0].weapon_type [0] = WPN_AF_MISSILE;
 player[0].weapon_type [1] = WPN_AWS_MISSILE;
 player[1].weapon_type [0] = WPN_AF_MISSILE;
 player[1].weapon_type [1] = WPN_AWS_MISSILE;

 if (arena.players == 2)
 {
  player[0].weapon_type [1] = WPN_NONE;
  player[1].weapon_type [0] = WPN_AWS_MISSILE;
  player[1].weapon_type [1] = WPN_NONE;
 }

 char finished [2] = {0,0};
 int ret1;
// int wpn_swap;
 int w;

 bover_menu_select [0] = CHOOSE_LAUNCH;
 bover_menu_select2 [0] = 0;
 bover_menu_select [1] = CHOOSE_LAUNCH;
 bover_menu_select2 [1] = 0;
 selecting [0] = 0;
 selecting [1] = 0;

 do
 {

  briefing_counter ++;


    choose_display(finished [0], finished [1]);

    if (finished [0] == 0)
    {
     if (arena.players == 1)
      ret1 = get_bover_input(0, 0, CHOOSE_REPLAY, selecting [0], 0, WPNS - 1, -1);
       else
        ret1 = get_bover_input(0, 0, CHOOSE_REPLAY, selecting [0], 0, WPNS - 1, CHOOSE_WEAPON2);
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
        if (bover_menu_select [0] == CHOOSE_WEAPON1
         || bover_menu_select [0] == CHOOSE_WEAPON2)
         {
          if (selecting [0])
          {
           w = bover_menu_select [0] - CHOOSE_WEAPON1;
           if (player[0].weapon_type [w^1] == bover_menu_select2 [0])
           {
            player[0].weapon_type [w^1] = player[0].weapon_type [w];
           }
           player[0].weapon_type [w] = bover_menu_select2 [0];
           selecting [0] = 0;
           play_basicwfv(WAV_SELECT1, FREQ_BSELECT1, VOL_BSELECT1);
          }
           else
            {
             selecting [0] = 1;
             bover_menu_select2 [0] = player[0].weapon_type [bover_menu_select [0] - CHOOSE_WEAPON1];
             play_basicwfv(WAV_SELECT1, FREQ_BSELECT1, VOL_BSELECT1);
            }
         }
//          player[0].weapon_type [bover_menu_select [0] - CHOOSE_WEAPON1] = bover_menu_select2 [0];
//         if (player[0].weapon_type [bover_menu_select [0] - CHOOSE_WEAPON1] != bover_menu_select2 [0])
/*        if (player[0].weapon_type [bover_menu_select2 [0]^1] == bover_menu_select [0])
        {
         wpn_swap = player[0].weapon_type [bover_menu_select2 [0]];
         player[0].weapon_type [bover_menu_select2 [0]] = player[0].weapon_type [bover_menu_select2 [0]^1];
         player[0].weapon_type [bover_menu_select2 [0]^1] = wpn_swap;
        }
         else
          player[0].weapon_type [bover_menu_select2 [0]] = bover_menu_select [0];*/
       }
     }
    }

    if (arena.players == 2 && finished [1] == 0)
    {
     ret1 = get_bover_input(1, 0, CHOOSE_REPLAY, selecting [1], 0, WPNS - 1, CHOOSE_WEAPON2);
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
/*        if (player[1].weapon_type [bover_menu_select2 [1]^1] == bover_menu_select [1])
        {
         wpn_swap = player[1].weapon_type [bover_menu_select2 [1]];
         player[1].weapon_type [bover_menu_select2 [1]] = player[1].weapon_type [bover_menu_select2 [1]^1];
         player[1].weapon_type [bover_menu_select2 [1]^1] = wpn_swap;
        }
         else
          player[1].weapon_type [bover_menu_select2 [1]] = bover_menu_select [1];*/
//        player[1].weapon_type [bover_menu_select2 [1]] = bover_menu_select [1];
       }
     }
    }

    if (finished [0] == 1 && (arena.players == 1 || finished [1] == 1))
     return 1;

/*    if (pressing_a_key(0, CKEY_FIRE2, JBUTTON_FIRE2)
     || (arena.players == 2 && pressing_a_key(1, CKEY_FIRE2, JBUTTON_FIRE2)))
      return 0;*/

    do
    {
        rest(1);
    } while (ticked == 0);
    ticked = 0;

  vsync();
  blit(display[0], screen, 0, 0, 0, 0, 800, 600);

 } while (TRUE);



}


#define LSPACE1 35
#define LSPACE2 20
#define WBOX_Y 480

extern struct BMP_STRUCT fighter_sprite [FIGHTER_SPRITES] [FIGHTER_ROTATIONS];
extern struct BMP_STRUCT player_sprite [PLAYER_SPRITES] [PLAYER_ROTATIONS];
void print_weapon_name(int x, int y, int col, int w);


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

//  rectfill(display[0], x, y, x + 200, y + 420, BCOL_EDGE);
//  rectfill(display[0], x + 3, y + 3, x + 200 - 3, y + 420 - 3, BCOL_SCREEN);

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

void bover_box(int x, int y)
{
 rectfill(display[0], x - 5, y - 3, x + 150, y + 15, COL_BOX1);
 rect(display[0], x - 5, y - 3, x + 150, y + 15, COL_BOX2);

}

void print_weapon_name(int x, int y, int col, int w)
{
   switch(w)
   {
    case WPN_AF_MISSILE:
     textprintf_ex(display[0], small_font, x, y, col, -1, "anti-fighter missile"); break;
    case WPN_AWS_MISSILE:
     textprintf_ex(display[0], small_font, x, y, col, -1, "anti-warship missile"); break;
//    case WPN_ROCKET1:
//     textprintf_ex(display[0], small_font, x, y, col, -1, "rocket"); break;
    case WPN_ROCKET:
     textprintf_ex(display[0], small_font, x, y, col, -1, "multi-rockets"); break;
    case WPN_TORP:
     textprintf_ex(display[0], small_font, x, y, col, -1, "torpedo"); break;
    case WPN_ROCKET2:
     textprintf_ex(display[0], small_font, x, y, col, -1, "rocket"); break;
    case WPN_DEFLECT:
     textprintf_ex(display[0], small_font, x, y, col, -1, "tracking cannon"); break;
//    case WPN_SEEKERS:
//     textprintf_ex(display[0], small_font, x, y, col, -1, "seekers"); break;
    case WPN_BLASTER:
     textprintf_ex(display[0], small_font, x, y, col, -1, "heavy cannon"); break;
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

    do
    {
        rest(1);
    } while (ticked == 0);
    ticked = 0;



  if (finished && key [KEY_SPACE])
   break;
 };

}


void battle_report_display(int row, int flash, int space)
{


 clear_to_color(display [0], 0);

// rectfill

 rectfill(display[0], 100, 50, 700, 550, BCOL_DARK);
 rect(display[0], 100, 50, 700, 550, BCOL_EDGE);

 rectfill(display[0], 100, 55, 700, 72, BCOL_EDGE);
 textprintf_centre_ex(display[0], small_font, 400, 58, BCOL_TEXT, -1, "Battle Report");

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

 for (r = 0; r < row + 1; r ++)
 {
  if (br_row [r] == BR_ROW_ENEMY_TITLE)
  {
   mincol = COL_EBOX1;
   maxcol = COL_EBOX4;
   textcol = COL_EBOX4;
  }
  if (r == row && flash > 0)
  {
   col = mincol + (flash >> 1);
   if (col > maxcol)
    col = maxcol;
   rectfill(display[0], 100, y + 5, 700, y + 22, col);
  }


// if (r != row || col < maxcol)
  switch(br_row [r])
  {
   case BR_ROW_OURS_TITLE:
    if (r != row)
     rectfill(display[0], 101, y + 5, 699, y + 22, COL_BOX1);
    textprintf_ex(display[0], small_font, 150, y + 7, textcol, -1, "OCSF Forces");
    y += 5;
    break;
   case BR_ROW_OUR_SHIP:
    switch(br_row_val [r])
    {
     case SHIP_OLD2: strcpy(str, "Light Cruiser"); break;
     case SHIP_OLD3: strcpy(str, "Battle Cruiser"); break;
     case SHIP_FRIEND3: strcpy(str, "Trireme"); break;
     case SHIP_FIGHTER_FRIEND: strcpy(str, "Fighter"); break;
     default: strcpy(str, "Unknown ship type"); break;
/*
     case SHIP_SCOUT2: strcpy(str, "Small Warship"); break;
     case SHIP_SCOUT3: strcpy(str, "Large Warship"); break;
     case SHIP_BOMBER: strcpy(str, "Bomber"); break;
     case SHIP_FIGHTER: strcpy(str, "Fighter"); break;
     case SHIP_ESCOUT: strcpy(str, "Interceptor"); break;
     case SHIP_SCOUTCAR: strcpy(str, "Carrier"); break;
     default: strcpy(str, "Unknown ship type"); break;
*/
    }
    textprintf_right_ex(display[0], small_font, 230, y + 7, textcol, -1, str);
    col2 = 0;
    for (i = 0; i < arena.srecord [SREC_CREATED] [TEAM_FRIEND] [br_row_val [r]]; i ++)
    {
     if (i >= arena.srecord [SREC_CREATED] [TEAM_FRIEND] [br_row_val [r]] - arena.srecord [SREC_DESTROYED] [TEAM_FRIEND] [br_row_val [r]])
      col2 = 1; // destroyed
     draw_sprite(display[0], bsprite [TEAM_FRIEND] [get_bsprite_from_ship(br_row_val [r])] [col2],
      235 + (i * 13), y + 7);
    }
//    textprintf_right_ex(display[0], small_font, 245 + (i * 13), y + 7, textcol, 1, "%i", br_row_val [r]);
    break;
   case BR_ROW_AM:
    textprintf_right_ex(display[0], small_font, 230, y + 7, textcol, -1, "Angry Moth");
    col2 = 0;
    for (i = 0; i < player[0].starting_ships; i ++)
    {
     if (i >= player[0].starting_ships - player[0].ships_lost)
      col2 = 1; // destroyed
     draw_sprite(display[0], bsprite [TEAM_FRIEND] [BSF_FIGHTER] [col2],
      235 + (i * 13), y + 7);
    }
    y += 10;
    break;
   case BR_ROW_ENEMY_TITLE:
    if (r != row)
     rectfill(display[0], 101, y + 5, 699, y + 22, COL_EBOX1);
    textprintf_ex(display[0], small_font, 150, y + 7, textcol, -1, "Enemy Forces");
    y += 5;
    break;
   case BR_ROW_ENEMY_SHIP:
    switch(br_row_val [r])
    {
     case SHIP_SCOUT2: strcpy(str, "Small Warship"); break;
     case SHIP_SCOUT3: strcpy(str, "Large Warship"); break;
     case SHIP_BOMBER: strcpy(str, "Bomber"); break;
     case SHIP_FIGHTER: strcpy(str, "Fighter"); break;
     case SHIP_ESCOUT: strcpy(str, "Heavy Fighter"); break;
     case SHIP_EINT: strcpy(str, "Interceptor"); break;
     case SHIP_SCOUTCAR: strcpy(str, "Carrier"); break;
     case SHIP_EBASE: strcpy(str, "Base"); break;
     default: strcpy(str, "Unknown ship type"); break;
    }
    textprintf_right_ex(display[0], small_font, 230, y + 7, textcol, -1, str);
//    textprintf_right_ex(display[0], small_font, 300, y + 4, textcol, -1, "%i, %i, %i", br_row_val [r], arena.srecord [SREC_CREATED] [TEAM_ENEMY] [br_row_val [r]], arena.srecord [SREC_DESTROYED] [TEAM_ENEMY] [br_row_val [r]]);
    col2 = 0;
    for (i = 0; i < arena.srecord [SREC_CREATED] [TEAM_ENEMY] [br_row_val [r]]; i ++)
    {
     if (i >= arena.srecord [SREC_CREATED] [TEAM_ENEMY] [br_row_val [r]] - arena.srecord [SREC_DESTROYED] [TEAM_ENEMY] [br_row_val [r]])
      col2 = 1; // destroyed
     draw_sprite(display[0], bsprite [TEAM_ENEMY] [get_bsprite_from_ship(br_row_val [r])] [col2],
      235 + (i * 13), y + 7);
    }
    break;

  }

  y += 20;
 }

 y += 20;

 if (space)
 {
  report_text(y);

  textprintf_centre_ex(display[0], small_font, 400, 530, BCOL_TEXT, -1, "Press space to continue");
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
  case SHIP_OLD3: return 35;
  case SHIP_FIGHTER: return 2;
  case SHIP_FRIEND3: return 45;
  case SHIP_SCOUT2: return 7;
  case SHIP_BOMBER: return 5;
  case SHIP_FIGHTER_FRIEND: return 3;
  case SHIP_SCOUT3: return 35;
  case SHIP_ESCOUT: return 4;
  case SHIP_SCOUTCAR: return 45;
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
// only relevant consideration here is flost as all enemies must be destroyed to finish mission
   flost = loss_proportion(TEAM_FRIEND);
   lost_rank = get_rank(flost, 30, 50, 70, 80);
   switch(lost_rank)
   {

    case 0:
     strcpy(rmsg.text, "The mission was an absolute success. Enemy forces were annihilated, with no losses on our side. Well done! "); break;
    case 1:
     strcpy(rmsg.text, "The mission was a great success. Enemy forces were annihilated, with only minor losses on our side. Well done! "); break;
    case 2:
     strcpy(rmsg.text, "The mission was successful. Although we lost a number of ships, the enemy lost far more. "); break;
    case 3:
     strcpy(rmsg.text, "The mission was successful. Although we lost several ships, the enemy force was destroyed. "); break;
    case 4:
     strcpy(rmsg.text, "We lost a number of our ships, but the enemy force was destroyed. Overall, a success. "); break;
    case 5:
     strcpy(rmsg.text, "Most of our ships were destroyed, but at least we stopped the enemy force getting through. "); break;
   }

   break;
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
