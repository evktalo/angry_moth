
#include "config.h"

#include "allegro.h"

#include <math.h>
#include <string.h>

#include "globvars.h"
#include "palette.h"

#define IN_DISPLAY_C
#include "display.h"
#undef IN_DISPLAY_C

#include "game.h"
#include "stuff.h"
#include "message.h"
#include "text.h"

#include "level.h"

#define SCAN_SIZE 50

#define TRANS_TABLE color_map = &trans_table;
#define ALPHA_TABLE color_map = &alpha_table;




BITMAP *display [3];

extern COLOR_MAP trans_table;
extern COLOR_MAP alpha_table;
// in palette.c

//#define SCREENSHOT

#ifdef SCREENSHOT
extern RGB palet [256];

#include "string.h"
#endif

//RLE_SPRITE *RLE_bubble [RLE_BUBBLES];

extern volatile int frames_per_second;
extern int slacktime;
extern int long_slacktime_store;

void draw_player(int d, int p);
int get_player_sprite(int p);

void draw_bullets(int d, int p);
void draw_a_bullet(int d, int b);

//void draw_bullets(int d, int p);

void pline(BITMAP *bmp, int x1, int y1, int x2, int y2, int colour);

void draw_stars(int d, int p, char motion);
void poly4(BITMAP *target, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int col);
void print_number(int x, int y, int n);
void draw_clouds(int d, int p);
void draw_a_cloud(int d, int c);
void draw_HUD(int d, int p);
void torp_arming_lines(int d, int p, int w, int max_charge, int max_reload, int width);
void draw_overscan(int x, int y);
void draw_trail_line(BITMAP* bmp, int x, int y, int size);
void draw_trail_line3(BITMAP* bmp, int x, int y, int size);
void draw_worm_trail_line(BITMAP* bmp, int x, int y, int size);
void draw_rocket_trail_line(BITMAP* bmp, int x, int y, int size);
//void draw_shield_circle(BITMAP* bmp, int x, int y, int size);
void draw_pshield_circle(BITMAP *bmp, int x, int y, int strength);
char check_pixel(int x, int y);
void run_display(int draw_everything, int star_motion);
void draw_ships(int d, int p);
void draw_a_ship(int d, int a, int e, int p);
void draw_a_wship(int a, int e, int d, int p, int x, int y);
void draw_edrive(int d, int x, int y, int angle, int dist, int size, int reduce, int randsize, int col);
void draw_a_turret(int d, int a, int e, int t, int p, int x, int y);
void draw_other_player(int d, int p, int op);
void draw_player_shield(int d, int p, int x, int y, int angle_draw);
void draw_final_details(void);
void draw_hitpulses(int d, int p, int type, int x, int y, int bitshift, int col);

void display_message_header(int i, int x, int y);

void draw_convoys(int d, int p);
void draw_a_convoy(int d, int cv, int p);


int points [8];

int scanner [SCAN_SIZE] [SCAN_SIZE];

RLE_SPRITE *circle_in [CIRCLES];
RLE_SPRITE *circle_out [CIRCLES];
RLE_SPRITE *circle_hurt [CIRCLES];
RLE_SPRITE *circle_white [CIRCLES];
RLE_SPRITE *circle_grey [CIRCLES];

//RLE_SPRITE *RLE_bcircle [3] [RLE_BCIRCLES];
//RLE_SPRITE *RLE_wcircle [3] [RLE_BCIRCLES];

RLE_SPRITE *RLE_ccircle_basic [3] [RLE_CCIRCLES];
RLE_SPRITE *RLE_ccircle_3cols [3] [RLE_CCIRCLES];
RLE_SPRITE *RLE_ccircle_2cols [3] [RLE_CCIRCLES];
RLE_SPRITE *RLE_small_shock [3] [SMALL_SHOCK_TIME];
RLE_SPRITE *RLE_large_shock [3] [LARGE_SHOCK_TIME];
RLE_SPRITE *RLE_huge_shock [3] [HUGE_SHOCK_TIME];
RLE_SPRITE *RLE_player [PLAYER_RLES];

RLE_SPRITE *RLE_scircle [4] [RLE_SCIRCLES];

RLE_SPRITE *RLE_xcircle [3] [RLE_XCIRCLES];

RLE_SPRITE* damage_sprite [NO_DSPRITES] [DAMAGE_COLS];

extern struct BMP_STRUCT ship_collision_mask [NO_SHIP_TYPES]; // defined in display_init.c

void draw_circle_in(int d, int x, int y, int size, char hurt);
void draw_circle_out(int d, int x, int y, int size);
void draw_circle_white(int d, int x, int y, int size);
void draw_circle_grey(int d, int x, int y, int size);
void ccircle(int d, int x, int y, int size, int colour);
void ccircle3(int d, int x, int y, int size, int colour);
void ccircle2(int d, int x, int y, int size, int colour);
void ccircle3_bmp(BITMAP *bmp, int x, int y, int size, int colour);
void ccircle2_bmp(BITMAP *bmp, int x, int y, int size, int colour);

extern volatile unsigned char ticked;

#ifdef SHOW_GRAPHS
int graph_slack [200];
int graph_slack_pos;
int graph_fps [200];
int graph_fps_pos;
#endif

extern FONT* gfont;
extern FONT* small_font;

struct BMP_STRUCT wship_sprite [WSHIP_SPRITES] [WSHIP_ROTATIONS];

struct BMP_STRUCT turret_sprite [TURRET_SPRITES] [TURRET_ROTATIONS];

// these two are externed in both display.c and briefing.c
struct BMP_STRUCT fighter_sprite [FIGHTER_SPRITES] [FIGHTER_ROTATIONS];
struct BMP_STRUCT player_sprite [PLAYER_SPRITES] [PLAYER_ROTATIONS];

struct BMP_STRUCT spulse_sprite [SHIELD_TYPES] [SHIELD_FRAMES] [SHIELD_ROTATIONS];

struct BMP_STRUCT missile_sprite [MISSILE_SPRITES] [MISSILE_ROTATIONS];

//struct BMP_STRUCT shield_sprite [SHIELD_SPRITES] [

int trans_col [3] [4] =
{
 {TRANS_RED1, TRANS_RED2, TRANS_RED3, TRANS_YELLOW4},
 {TRANS_GREEN1, TRANS_GREEN2, TRANS_GREEN3, TRANS_GREEN4},
 {TRANS_BLUE1, TRANS_BLUE2, TRANS_BLUE3, TRANS_BLUE4}
};

// in palette.c
extern int comm_col [COMM_COLS] [COMM_COL_MAX+1];


int camera_angle;
float camera_angle_rad;
//char camera_fix;

//BITMAP *interlude_screen;

/*
Call when you want the display updated.
Uses buffering: everything is drawn to the display bitmap, then the display bitmap is blitted to the screen
 in one go.
*/
void run_display(int draw_everything, int star_motion)
{

  if (!draw_everything)
  {
    return;
  }



 if (arena.only_player != -1) // i.e. there is only one player
 {
  camera_angle = player[arena.only_player].angle;
  if (arena.camera_fix)
  {
   camera_angle = -ANGLE_4;
/*   player[0].camera_x = 400 - xpart(player[0].angle, 200);
   player[0].camera_y = 300 - ypart(player[0].angle, 200);*/
  }
  camera_angle_rad = angle_to_radians(camera_angle);
  clear_to_color(display [0], COL_STAR1);
  draw_stars(0, arena.only_player, star_motion);
  draw_ships(0, arena.only_player);
  if (player[arena.only_player].alive)
  {
   if (arena.camera_fix)
    draw_other_player(0, arena.only_player, arena.only_player); // this draws player[arena.only_player] as if they were the other player.
     else
      draw_player(0, arena.only_player);
  }
  draw_bullets(0, arena.only_player);
  draw_clouds(0, arena.only_player);
  draw_convoys(0, arena.only_player);
  draw_HUD(0, arena.only_player);
  draw_overscan(640, 440);
 }
  else
  {
   camera_angle = player[0].angle;
   camera_angle_rad = angle_to_radians(camera_angle);
   clear_to_color(display [0], COL_STAR1);
   draw_stars(1, 0, star_motion);
   draw_ships(1, 0);
    if (player[1].alive) {
        draw_other_player(1, 0, 1);
    }
    if (player[0].alive) {
        draw_player(1, 0);
    }
   draw_bullets(1, 0);
   draw_clouds(1, 0);
   draw_HUD(1, 0);
// draw_HUD for a p must come after draw_ships for that p and before draw_ships for other p
//  because of stored_angle value

   camera_angle = player[1].angle;
   camera_angle_rad = angle_to_radians(camera_angle);
   draw_stars(2, 1, star_motion);
   draw_ships(2, 1);
    if (player[0].alive) {
        draw_other_player(2, 1, 0);
    }
    if (player[1].alive) {
        draw_player(2, 1);
    }
    draw_bullets(2, 1);
    draw_clouds(2, 1);
    draw_HUD(2, 1);
  }

  draw_final_details(); // just draws ships left and the central line and overscan for 2-player

//#ifdef DEBUG_DISPLAY


#define FPS_DISPLAY

#ifdef FPS_DISPLAY
//  textprintf_ex(display[0], small_font, 1, 562, -1, -1, "%i, %i", player[0].x>>10, player[0].y>>10);
  textprintf_ex(display[0], small_font, 1, 575, -1, -1, "fps %i", frames_per_second);
  textprintf_ex(display[0], small_font, 1, 587, -1, -1, "slack %i", slacktime);
//  textprintf_ex(display[0], small_font, 150, 487, -1, -1, "ad %i", player[0].rocket_burst);
#endif
//  textprintf_ex(display[0], small_font, 1, 375, -1, -1, "%i", player[0].rocket_burst);
//  textprintf_ex(display[0], small_font, 10, 270, -1, -1, "threat %i ws%i f%i", arena.threat, arena.wship_threat, arena.fighter_threat);
//#endif

  if (arena.game_over > 0)
  {
   textprintf_centre_ex(display[0], small_font, 400, 300, COL_WHITE, -1, "G A M E    O V E R");
  }

  if (arena.mission_over > 0)
  {
   textprintf_centre_ex(display[0], small_font, 400, 300, COL_WHITE, -1, "M I S S I O N    O V E R");
  }

  if (arena.all_wships_lost > 0)
  {
   textprintf_centre_ex(display[0], small_font, 400, 280, COL_WHITE, -1, "A L L    W A R S H I P S    L O S T");
   textprintf_centre_ex(display[0], small_font, 400, 320, COL_WHITE, -1, "G A M E    O V E R");
  }

  if (arena.missed_jump > 0)
  {
   textprintf_centre_ex(display[0], small_font, 400, 280, COL_WHITE, -1, "M I S S E D    J U M P");
   textprintf_centre_ex(display[0], small_font, 400, 320, COL_WHITE, -1, "G A M E    O V E R");
  }


  if (arena.jump_countdown != -1)
  {
   if (arena.jump_countdown > 0)
   {
    textprintf_centre_ex(display[0], small_font, 400, 280, COL_WHITE, -1, "WARSHIPS WILL JUMP OUT IN %i SECONDS", arena.jump_countdown / 50);
   }
   if (arena.jump_countdown == 0)
   {
    textprintf_centre_ex(display[0], small_font, 400, 280, COL_WHITE, -1, "WARSHIPS JUMPING OUT");
   }
   if (arena.only_player != -1)
   {
    switch(player[arena.only_player].jump_safe)
    {
     case JUMP_SAFE:
      textprintf_centre_ex(display[0], small_font, 400, 320, COL_BOX3, -1, "STAY CLOSE TO ENSURE PICKUP"); break;
     case JUMP_RISK:
      textprintf_centre_ex(display[0], small_font, 400, 320, COL_EBOX3, -1, "MOVE CLOSER TO ENSURE PICKUP"); break;
     case JUMP_NOT_SAFE:
      textprintf_centre_ex(display[0], small_font, 400, 320, TRANS_RED2, -1, "RETURN TO A FRIENDLY WARSHIP"); break;
    }
   }
    else
    {
     int jp;
     for (jp = 0; jp < 2; jp++)
     {
      switch(player[jp].jump_safe)
      {
       case JUMP_SAFE:
        textprintf_centre_ex(display[0], small_font, 200 + jp*400, 320, COL_BOX3, -1, "STAY CLOSE TO ENSURE PICKUP"); break;
       case JUMP_RISK:
        textprintf_centre_ex(display[0], small_font, 200 + jp*400, 320, COL_EBOX3, -1, "MOVE CLOSER TO ENSURE PICKUP"); break;
       case JUMP_NOT_SAFE:
        textprintf_centre_ex(display[0], small_font, 200 + jp*400, 320, TRANS_RED2, -1, "RETURN TO A FRIENDLY WARSHIP"); break;
    }

     }

    }

  }

  blit(display[0], screen, 0, 0, 0, 0, 800, 600);

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
   scrshot_bmp = create_bitmap(640, 480);
   blit(screen, scrshot_bmp, 0,0,0,0,640,480);

  strcpy(sfile, "scr");
  strcat(sfile, itoa(scrs, istr, 10));
  strcat(sfile, ".bmp");
  save_bitmap(sfile, scrshot_bmp, palet);
  clear_to_color(screen, COL_WHITE);
  scrs ++;
  sshot_counter = 15;
  destroy_bitmap(scrshot_bmp);
 }

#endif

}

void get_interlude_background(void)
{

 run_display(1, 0);

// blit(display[0], interlude_screen, 0,0,0,0,640,480);

}

int get_player_sprite(int p)
{
 int sprite;

 switch(PP.type)
 {
  case SHIP_FIGHTER_FRIEND:
   sprite = FIGHTER_SPRITE_FRIEND_1;
   if (PP.drive [0] > 0)
    sprite ++;
   if (PP.drive [0] == eclass[PP.type].engine_power [0])
    sprite ++;
   break;
  case SHIP_FSTRIKE:
   sprite = FIGHTER_SPRITE_FSTRIKE_1;
   if (PP.drive [0] > 0)
    sprite ++;
   if (PP.drive [0] == eclass[PP.type].engine_power [0])
    sprite ++;
   break;
  case SHIP_MONARCH: // old Angry Moth:
  sprite = FIGHTER_SPRITE_MONARCH_1;
/*
  if (PP.recycle > 2)
   sprite = PLAYER_SPRITE_2_1;
  if (PP.recycle > 6)
   sprite = PLAYER_SPRITE_3_1;
*/
  if (PP.drive [0] > 0)
   sprite ++;
  if (PP.drive [0] == eclass[PP.type].engine_power [0])
   sprite ++;
   break;
  case SHIP_LACEWING:
   sprite = FIGHTER_SPRITE_LACEWING_1;
  if (PP.drive [0] > 0)
   sprite ++;
  if (PP.drive [0] > 2)
   sprite ++;
   if (PP.drive [0] == eclass[PP.type].engine_power [0])
   sprite ++;
   break;
  case SHIP_IBEX:
   sprite = FIGHTER_SPRITE_IBEX_1;
   if (PP.drive [0] > 0)
    sprite ++;
   if (PP.drive [0] == eclass[PP.type].engine_power [0])
    sprite ++;
   break;
  case SHIP_AUROCHS:
   sprite = FIGHTER_SPRITE_AUROCHS_1;
   if (PP.drive [0] > 0)
    sprite ++;
   if (PP.drive [0] == eclass[PP.type].engine_power [0])
    sprite ++;
   break;
 }
 return sprite;
}

void draw_player(int d, int p)
{
    int sprite;
    sprite = get_player_sprite(p);

    draw_sprite(display[d], fighter_sprite [sprite] [0].sprite, PP.camera_x - fighter_sprite [sprite] [0].x [0], PP.camera_y - fighter_sprite [sprite] [0].y [0]);
    print_number(300, 300, PP.weapon_block [0]);
    print_number(300, 320, PP.weapon_block [1]);
    print_number(300, 340, PP.weapon_block [2]);

    if (PP.drive [0] > 0)
    {
        draw_edrive(d, PP.camera_x - fighter_sprite [sprite] [0].x [1], PP.camera_y + fighter_sprite [sprite] [0].y [1], 0, 0, PP.drive [0], 2, 3, 0);
        if (eclass[PP.type].engines == 2) {
            draw_edrive(d, PP.camera_x - fighter_sprite [sprite] [0].x [2], PP.camera_y + fighter_sprite [sprite] [0].y [2], 0, 0, PP.drive [0], 2, 3, 0);
        }
    }

    if (PP.drive [1] > 0) {
        draw_edrive(d, PP.camera_x, PP.camera_y - 9, -ANGLE_2, PP.drive[1], PP.drive[1], 1, 2, 0);
    }
    if (PP.drive [2] > 0) {
        draw_edrive(d, PP.camera_x + 5, PP.camera_y, -ANGLE_4, PP.drive[2]>>1, PP.drive[2], 1, 2, 0);
    }
    if (PP.drive [3] > 0) {
        draw_edrive(d, PP.camera_x - 5, PP.camera_y, ANGLE_4, PP.drive[3]>>1, PP.drive[3], 1, 2, 0);
    }

    draw_player_shield(d, p, PP.camera_x, PP.camera_y, 0);
}

void draw_other_player(int d, int p, int op)
{
    if (player[op].x < PP.x - (600 << 10)
        || player[op].x > PP.x + (600 << 10)
        || player[op].y < PP.y - (600 << 10)
        || player[op].y > PP.y + (600 << 10)
    ) {
        return;
    }

 int sprite;
 sprite = get_player_sprite(op);

   float angle = atan2(player[op].y - PP.y, player[op].x - PP.x) - (PI/2) - angle_to_radians(camera_angle);
   int dist = hypot(player[op].y - PP.y, player[op].x - PP.x);
   int x = PP.camera_x + cos(angle) * (dist >> 10);
   int y = PP.camera_y + sin(angle) * (dist >> 10);
   int sprite_angle;
   int sprite_angle_flip = 0;
   int angle_draw;
//   int fsprite;

//   int ldrive_size, rdrive_size, reduce, randsize;


   angle_draw = player[op].angle - camera_angle;
   angle_draw &= ANGLE_MASK;
   sprite_angle = ((angle_draw) >> PLAYER_ROTATION_BITSHIFT) & PLAYER_FULL_MASK;
   sprite_angle_flip = 0;

//   ldrive_size = EE.engine1 / 30;
//   rdrive_size = EE.engine2 / 30;

   if (sprite_angle >= PLAYER_ROTATIONS)
   {
    if (sprite_angle >= PLAYER_ROTATIONS * 3)
    {
      sprite_angle_flip = (PLAYER_ROTATIONS * 4) - sprite_angle - 1;
      sprite_angle_flip &= PLAYER_ROTATION_MASK;
      draw_sprite_h_flip(display[d], fighter_sprite [sprite] [sprite_angle_flip].sprite, x - (fighter_sprite [sprite] [sprite_angle_flip].sprite->w - fighter_sprite [sprite] [sprite_angle_flip].x [0]), y - fighter_sprite [sprite] [sprite_angle_flip].y [0]);
      if (player[op].drive [0] > 0)
       draw_edrive(d, x - fighter_sprite [sprite] [sprite_angle_flip].x [1], y - fighter_sprite [sprite] [sprite_angle_flip].y [1],
        angle_draw, 0, player[op].drive[0], 2, 3, 0);
//  draw_edrive(d, x + xpart(angle_draw + ANGLE_4, 4), y + ypart(angle_draw + ANGLE_4, 4), angle_draw, player[op].drive[0], player[op].drive[0], 2, 3, 0);

    }
      else
      {
       if (sprite_angle >= PLAYER_ROTATIONS * 2)
       {
         sprite_angle_flip = sprite_angle - (PLAYER_ROTATIONS * 2);
         sprite_angle_flip &= PLAYER_ROTATION_MASK;
         draw_sprite_vh_flip(display[d], fighter_sprite [sprite] [sprite_angle_flip].sprite, x - (fighter_sprite [sprite] [sprite_angle_flip].sprite->w - fighter_sprite [sprite] [sprite_angle_flip].x [0]), y - (fighter_sprite [sprite] [sprite_angle_flip].sprite->h - fighter_sprite [sprite] [sprite_angle_flip].y [0]));
         if (player[op].drive [0] > 0)
          draw_edrive(d, x - fighter_sprite [sprite] [sprite_angle_flip].x [1], y - fighter_sprite [sprite] [sprite_angle_flip].y [1],
           angle_draw, 0, player[op].drive[0], 2, 3, 0);
       }
        else
        {
          sprite_angle_flip = (PLAYER_ROTATIONS * 2) - sprite_angle - 1;
          sprite_angle_flip &= PLAYER_ROTATION_MASK;
          draw_sprite_v_flip(display[d], fighter_sprite [sprite] [sprite_angle_flip].sprite, x - fighter_sprite [sprite] [sprite_angle_flip].x [0], y - (fighter_sprite [sprite] [sprite_angle_flip].sprite->h - fighter_sprite [sprite] [sprite_angle_flip].y [0]));
          if (player[op].drive [0] > 0)
           draw_edrive(d, x - fighter_sprite [sprite] [sprite_angle_flip].x [1], y - fighter_sprite [sprite] [sprite_angle_flip].y [1],
            angle_draw, 0, player[op].drive[0], 2, 3, 0);

        }

      }

   }
    else
    {
      sprite_angle &= PLAYER_ROTATION_MASK;
      draw_sprite(display[d], fighter_sprite [sprite] [sprite_angle].sprite, x - fighter_sprite [sprite] [sprite_angle].x [0], y - fighter_sprite [sprite] [sprite_angle].y [0]);
      if (player[op].drive [0] > 0)
       draw_edrive(d, x - fighter_sprite [sprite] [sprite_angle].x [1], y - fighter_sprite [sprite] [sprite_angle].y [1],
        angle_draw, 0, player[op].drive[0], 2, 3, 0);

    }


// if (player[op].drive [0] > 0)
//  draw_edrive(d, x + xpart(angle_draw + ANGLE_4, 4), y + ypart(angle_draw + ANGLE_4, 4), angle_draw, player[op].drive[0], player[op].drive[0], 2, 3, 0);

 if (player[op].drive [1] > 0)
  draw_edrive(d, x + xpart(angle_draw - ANGLE_4, 9), y + ypart(angle_draw - ANGLE_4, 9), angle_draw - ANGLE_2, player[op].drive[1], player[op].drive[1], 1, 2, 0);

 if (player[op].drive [2] > 0)
  draw_edrive(d, x + xpart(angle_draw, 7), y + ypart(angle_draw, 7), angle_draw - ANGLE_4, player[op].drive[2]>>1, player[op].drive[2], 1, 2, 0);

 if (player[op].drive [3] > 0)
  draw_edrive(d, x + xpart(angle_draw - ANGLE_2, 7), y + ypart(angle_draw - ANGLE_2, 7), angle_draw + ANGLE_4, player[op].drive[3]>>1, player[op].drive[3], 1, 2, 0);
/*
 if (player[op].mflash [0] > 0)
  ccircle2(d, x + xpart(angle_draw - ANGLE_4 - ANGLE_16, 9), y + ypart(angle_draw - ANGLE_4 - ANGLE_16, 9), player[op].mflash [0], 0);

 if (player[op].mflash [1] > 0)
  ccircle2(d, x + xpart(angle_draw - ANGLE_4 + ANGLE_16, 9), y + ypart(angle_draw - ANGLE_4 + ANGLE_16, 9), player[op].mflash [1], 0);
*/

 draw_player_shield(d, op, x, y, angle_draw);


//  draw_edrive(d, x, y, angle_draw, player[op].drive >> 1, 2, 2, 0);

// vline(display[d], x, y - 15, y + 15, COL_7);
// hline(display[d], x - 15, y, x + 15, COL_7);


}

void draw_player_shield(int d, int p, int x, int y, int angle_draw)
{

   int pangle, pdist, ptype, pframe, i, draw_angle;

   ptype = SHIELD_FIGHTER;

   for (i = 0; i < PULSE; i ++)
   {
    if (PP.spulse_time [i] > 0)
    {
     pangle = ((PP.spulse_angle [i] + angle_draw + ANGLE_4) >> SHIELD_ROTATION_BITSHIFT) & SHIELD_FULL_MASK;
//     pangle = ((EE.spulse_angle [i] + angle_draw) ) & SHIELD_FULL_MASK;
     draw_angle = PP.spulse_angle [i] + angle_draw;
     pdist = PP.spulse_dist [i] + 10;
     pframe = 0;
     if (PP.spulse_time [i] < 90)
      pframe = 1;
     if (PP.spulse_time [i] < 60)
      pframe = 2;
     if (PP.spulse_time [i] < 30)
      pframe = 3;

     draw_trans_sprite(display[d], spulse_sprite [ptype] [pframe] [pangle].sprite, x + xpart(draw_angle, pdist) - spulse_sprite [ptype] [pframe] [pangle].x [0], y + ypart(draw_angle, pdist) - spulse_sprite [ptype] [pframe] [pangle].y [0]);
//     draw_trans_sprite(display[d], spulse_sprite [ptype] [pframe] [pangle].sprite, x - spulse_sprite [ptype] [pframe] [pangle].x [0], y - spulse_sprite [ptype] [pframe] [pangle].y [0]);

    }

   }


}


char check_pixel(int x, int y)
{
  int pix = getpixel(display[0], x, y);

  if (pix == COL_14 || pix >= TRANS_YELLOW4)
    return 1;

 return 0;

}



void draw_convoys(int d, int p)
{
//return;
//#define SHOW_RUNNING_SCRIPTS

#ifdef SHOW_RUNNING_SCRIPTS
 int r;
 extern int running_script [16];
 char sname [30];

 for (r = 0; r < 16; r ++)
 {
  get_script_name(r, sname);
  textprintf_ex(display[d], small_font, 10, 270 + r*12, COL_WHITE, -1, "%i, %i (%s)", r, running_script [r], sname);
 }
#endif

 int cv;

 for (cv = 0; cv < NO_CONVOYS; cv ++)
 {
   if (convoy[cv].active < 1)
    continue;
   if (convoy[cv].x < PP.x - (600 << 10) || convoy[cv].x > PP.x + (600 << 10)
    || convoy[cv].y < PP.y - (600 << 10) || convoy[cv].y > PP.y + (600 << 10))
     continue;
   draw_a_convoy(d, cv, p);
 }

 int i, a, col;
/*
 for (a = 0; a < 2; a ++)
 {
  for (i = 0; i < NO_SHIP_TYPES; i ++)
  {
   col = COL_BOX4;
   if (i == SHIP_BOMBER)
    col = COL_EBOX4;
   textprintf_ex(display[d], small_font, 80 + a * 20, 270 + i*12, col, -1, "%i/%i", arena.srecord [0] [a] [i], arena.srecord [1] [a] [i]);

  }
 }
*/
}


void draw_a_convoy(int d, int cv, int p)
{
// int turret_angle_draw;
   float angle = atan2(convoy[cv].y - PP.y, convoy[cv].x - PP.x) - (PI/2) - angle_to_radians(camera_angle);
   int dist = hypot(convoy[cv].y - PP.y, convoy[cv].x - PP.x);
   int x = PP.camera_x + cos(angle) * (dist >> 10);
   int y = PP.camera_y + sin(angle) * (dist >> 10);

   int angle_draw;

    angle_draw = convoy[cv].angle - camera_angle - ANGLE_4;
    angle_draw &= ANGLE_MASK;


    circle(display[d], x, y, 36, COL_F1 + TRANS_RED1);

    pline(display[d], x + xpart(angle_draw, 30), y + ypart(angle_draw, 30),
                      x + xpart(angle_draw, 50), y + ypart(angle_draw, 50), COL_F1 + TRANS_RED1);
  textprintf_ex(display[d], small_font, x + 50, y - 20, COL_WHITE, -1, "c %i arr %i app %i: %i, %i", cv, convoy[cv].arrangement, convoy[cv].approach_convoy, convoy[cv].target_x>>10, convoy[cv].target_y>>10);
//  textprintf_ex(display[d], small_font, x + 50, y - 20, -1, -1, "%i %i, %i: %i", cv, (int) hypot(convoy[cv].y_speed, convoy[cv].x_speed), convoy[cv].throttle, convoy[cv].can_turn);
//  textprintf_ex(display[d], font, x + 50, y - 20, -1, -1, "%i, %i, %i, %i", convoy[cv].x_speed, convoy[cv].y_speed, convoy[cv].approach_convoy, convoy[cv].throttle);
//  textprintf_ex(display[d], font, x + 20, y, -1, -1, "%i, %i", convoy[cv].turning, convoy[cv].turn_count);
//  textprintf_ex(display[d], font, x + 20, y, -1, -1, "%i, %i", convoy[cv].turning, convoy[cv].turn_count);
//  textprintf_ex(display[d], font, x + 20, y - 20, -1, -1, "%i", (int) hypot(convoy[cv].y - convoy[cv].target_y, convoy[cv].x - convoy[cv].target_x));
//  textprintf_ex(display[d], font, x, y + 20, -1, -1, "%i, %i", convoy[cv].x_speed, convoy[cv].y_speed);

   angle = atan2(convoy[cv].target_y - PP.y, convoy[cv].target_x - PP.x) - (PI/2) - camera_angle_rad;
   dist = hypot(convoy[cv].target_y - PP.y, convoy[cv].target_x - PP.x);
   x = PP.camera_x + cos(angle) * (dist >> 10);
   y = PP.camera_y + sin(angle) * (dist >> 10);

//    circle(display[d], x, y, 16, COL_E5);


}


void draw_ships(int d, int p)
{

 int a, e;

 for (a = 0; a < NO_TEAMS; a ++)
 {
  for (e = 0; e < NO_SHIPS; e ++)
  {
   if (EE.type == SHIP_NONE
    || eclass[EE.type].ship_class == ECLASS_FIGHTER)
    continue;
   if (EE.x < PP.x - (700 << 10) || EE.x > PP.x + (700 << 10)
    || EE.y < PP.y - (600 << 10) || EE.y > PP.y + (600 << 10))
     continue;
   draw_a_ship(d, a, e, p);
  }
 }

 for (a = 0; a < NO_TEAMS; a ++)
 {
  for (e = 0; e < NO_SHIPS; e ++)
  {
   if (EE.type == SHIP_NONE
    || eclass[EE.type].ship_class == ECLASS_WSHIP)
    continue;
   if (EE.x < PP.x - (700 << 10) || EE.x > PP.x + (700 << 10)
    || EE.y < PP.y - (600 << 10) || EE.y > PP.y + (600 << 10))
     continue;
   draw_a_ship(d, a, e, p);
  }
 }

}


void draw_a_ship(int d, int a, int e, int p)
{
// int turret_angle_draw;
   float angle = atan2(EE.y - PP.y, EE.x - PP.x);// - (PI/2) - angle_to_radians(PP.angle);
   EE.stored_angle = radians_to_angle(angle);
   angle -= (PI/2) + camera_angle_rad;
   int dist = hypot(EE.y - PP.y, EE.x - PP.x);
   EE.stored_dist = dist;

   int x = PP.camera_x + cos(angle) * (dist >> 10);
   int y = PP.camera_y + sin(angle) * (dist >> 10);

//   int angle_draw = ((radians_to_angle(angle) - PP.angle) >> 2) & 255;
//   int angle_draw = ((EE.angle - PP.angle - ANGLE_4) >> WSHIP_ROTATION_BITSHIFT);
//   int angle_draw = ((EE.angle - PP.angle) >> WSHIP_ROTATION_BITSHIFT) & WSHIP_FULL_MASK;
   int sprite_angle;
   int sprite_angle_flip = 0;
   int angle_draw = 0;
   int engine_dist;
   int fsprite;
   int i;
   int dcol = 0;
//   int xa, ya;

//   int xa, ya, xb, yb;
//   int ldrive_size, rdrive_size;

 switch(EE.type)
 {
  case SHIP_FIGHTER: // single-engine fighters
  case SHIP_EINT:
   dcol = 1;
  case SHIP_FIGHTER_FRIEND:
  case SHIP_LACEWING:
  case SHIP_MONARCH:

//    circle(display[d], x, y, 10, 14);
    angle_draw = EE.angle - camera_angle;
    angle_draw &= ANGLE_MASK;
   sprite_angle = ((angle_draw) >> FIGHTER_ROTATION_BITSHIFT) & FIGHTER_FULL_MASK;
   sprite_angle_flip = 0;
   fsprite = EE.sprite;
//   fsprite = FIGHTER_SPRITE_BASIC_1;

//   ldrive_size = EE.engine [0];//eclass[EE.type].engine_power [0];//(EE.engine [0] / 10) + 1;
//   rdrive_size = ldrive_size;
//   ldrive_size = (EE.engine [0] / 10) + 1;
//   rdrive_size = (EE.engine [1] / 10) + 1;

   if (sprite_angle >= FIGHTER_ROTATIONS)
   {
    if (sprite_angle >= FIGHTER_ROTATIONS * 3)
    {
      sprite_angle_flip = (FIGHTER_ROTATIONS * 4) - sprite_angle - 1;
      sprite_angle_flip &= FIGHTER_ROTATION_MASK;
//      draw_sprite_h_flip(display[d], FIGHTER_sprite [FIGHTER_SPRITE_BASIC] [sprite_angle_flip].sprite, x - FIGHTER_sprite [FIGHTER_SPRITE_BASIC] [sprite_angle_flip].x, y - FIGHTER_sprite [FIGHTER_SPRITE_BASIC] [sprite_angle_flip].y);
      draw_sprite_h_flip(display[d], fighter_sprite [fsprite] [sprite_angle_flip].sprite, x - (fighter_sprite [fsprite] [sprite_angle_flip].sprite->w - fighter_sprite [fsprite] [sprite_angle_flip].x [0]), y - fighter_sprite [fsprite] [sprite_angle_flip].y [0]);
      //ccircle2(d, x - fighter_sprite [fsprite] [sprite_angle_flip].x [1], y + fighter_sprite [fsprite] [sprite_angle_flip].y [1], 2 + grand(3), 0);
      if (EE.engine [0] > 0)
       draw_edrive(d, x - fighter_sprite [fsprite] [sprite_angle_flip].x [1], y + fighter_sprite [fsprite] [sprite_angle_flip].y [1],
        angle_draw, 0, EE.engine [0], 2, 3, dcol); // RH drive
//      draw_edrive(d, x - fighter_sprite [fsprite] [sprite_angle_flip].x [2], y + fighter_sprite [fsprite] [sprite_angle_flip].y [2],
//       angle_draw, 2 + ldrive_size, ldrive_size, 2, 3, 0); // LH drive
    }
      else
      {
       if (sprite_angle >= FIGHTER_ROTATIONS * 2)
       {
         sprite_angle_flip = sprite_angle - (FIGHTER_ROTATIONS * 2);
         sprite_angle_flip &= FIGHTER_ROTATION_MASK;
         draw_sprite_vh_flip(display[d], fighter_sprite [fsprite] [sprite_angle_flip].sprite, x - (fighter_sprite [fsprite] [sprite_angle_flip].sprite->w - fighter_sprite [fsprite] [sprite_angle_flip].x [0]), y - (fighter_sprite [fsprite] [sprite_angle_flip].sprite->h - fighter_sprite [fsprite] [sprite_angle_flip].y [0]));
//         draw_edrive(d, x - fighter_sprite [fsprite] [sprite_angle_flip].x [2], y - fighter_sprite [fsprite] [sprite_angle_flip].y [2],
//          angle_draw, 2 + rdrive_size, rdrive_size, 2, 3, 0); // RH drive
         if (EE.engine_power > 0)
          draw_edrive(d, x - fighter_sprite [fsprite] [sprite_angle_flip].x [1], y - fighter_sprite [fsprite] [sprite_angle_flip].y [1],
           angle_draw, 0, EE.engine [0], 2, 3, dcol); // LH drive
       }
        else
        {
          sprite_angle_flip = (FIGHTER_ROTATIONS * 2) - sprite_angle - 1;
          sprite_angle_flip &= FIGHTER_ROTATION_MASK;
          draw_sprite_v_flip(display[d], fighter_sprite [fsprite] [sprite_angle_flip].sprite, x - fighter_sprite [fsprite] [sprite_angle_flip].x [0], y - (fighter_sprite [fsprite] [sprite_angle_flip].sprite->h - fighter_sprite [fsprite] [sprite_angle_flip].y [0]));
          if (EE.engine_power > 0)
           draw_edrive(d, x + fighter_sprite [fsprite] [sprite_angle_flip].x [1], y - fighter_sprite [fsprite] [sprite_angle_flip].y [1],
            angle_draw, 0, EE.engine [0], 2, 3, dcol); // RH drive
//          draw_edrive(d, x + fighter_sprite [fsprite] [sprite_angle_flip].x [2], y - fighter_sprite [fsprite] [sprite_angle_flip].y [2],
//           angle_draw, 2 + ldrive_size, ldrive_size, 2, 3, 0); // LH drive
        }

      }

   }
    else
    {
      sprite_angle &= FIGHTER_ROTATION_MASK;
      draw_sprite(display[d], fighter_sprite [fsprite] [sprite_angle].sprite, x - fighter_sprite [fsprite] [sprite_angle].x [0], y - fighter_sprite [fsprite] [sprite_angle].y [0]);
//      draw_edrive(d, x + fighter_sprite [fsprite] [sprite_angle].x [2], y + fighter_sprite [fsprite] [sprite_angle].y [2],
//       angle_draw, 2 + rdrive_size, rdrive_size, 2, 3, 0); // RH drive
      if (EE.engine_power > 0)
       draw_edrive(d, x + fighter_sprite [fsprite] [sprite_angle].x [1], y + fighter_sprite [fsprite] [sprite_angle].y [1],
        angle_draw, 0, EE.engine [0], 2, 3, dcol); // LH drive
    }

/*
DRIVE indexes alternate!
*/




    if (EE.slide_count > 0)
    {
     if (EE.slide_dir == -1)
     {
       draw_edrive(d, x + xpart(angle_draw, 9),// - fighter_sprite [fsprite] [sprite_angle].x [0],
                      y + ypart(angle_draw, 9),// - fighter_sprite [fsprite] [sprite_angle].y [0],
                      angle_draw - ANGLE_4, 4, 4, 2, 2, dcol);

     }
     if (EE.slide_dir == 1)
     {
       draw_edrive(d, x - xpart(angle_draw, 9),// - fighter_sprite [fsprite] [sprite_angle].x [0],
                      y - ypart(angle_draw, 9),// - fighter_sprite [fsprite] [sprite_angle].y [0],
                      angle_draw + ANGLE_4, 4, 4, 2, 2, dcol);

     }
    }

     textprintf_ex(display[0], small_font, x + 10, y + 0, COL_WHITE, -1, "m %i a %i l %i tc %i tt %i", EE.mission, EE.action, EE.leader, EE.think_count, EE.turning_time);
/*
     textprintf_ex(display[0], small_font, x + 10, y + 0, COL_WHITE, -1, "%i", EE.engine [2]);
        angle_draw = EE.engine [1] - camera_angle + ANGLE_4;
     line(display[d], x, y, x + xpart(angle_draw, 20), y + ypart(angle_draw, 20), COL_F5);

    engine_dist = EE.sprite - FIGHTER_SPRITE_BASIC_1;

   angle = atan2(EE.formation_y - PP.y, EE.formation_x - PP.x);// - (PI/2) - angle_to_radians(PP.angle);
   angle -= (PI/2) + camera_angle_rad;
   dist = hypot(EE.formation_y - PP.y, EE.formation_x - PP.x);

     line(display[d], x, y, PP.camera_x + cos(angle) * (dist >> 10), PP.camera_y + sin(angle) * (dist >> 10), COL_WHITE);
*/



/*    xa = x + xpart(angle_draw + ANGLE_4, 13) + xpart(angle_draw + ANGLE_2, 7 + engine_dist);
    ya = y + ypart(angle_draw + ANGLE_4, 13) + ypart(angle_draw + ANGLE_2, 7 + engine_dist);
    ccircle2(d, xa, ya, 2 + grand(3), 0);

    xa = x + xpart(angle_draw + ANGLE_4, 13) + xpart(angle_draw, 7 + engine_dist);
    ya = y + ypart(angle_draw + ANGLE_4, 13) + ypart(angle_draw, 7 + engine_dist);
    ccircle2(d, xa, ya, 2 + grand(3), 0);*/

//    ccircle2(d, x + fighter_sprite [fsprite] [sprite_angle].x [1] - fighter_sprite [fsprite] [sprite_angle].x [0], y + fighter_sprite [fsprite] [sprite_angle].y [1] - fighter_sprite [fsprite] [sprite_angle].y [0], 2 + grand(3), 0);

//    vline(display[d], x, 0, 480, 12);
//    hline(display,[d] 0, y, 640, 12);

// line(display[0], x + xpart(EE.target_angle-PP.angle-ANGLE_4, 200), y + ypart(EE.target_angle-PP.angle-ANGLE_4, 200), x, y, COL_BOX4);

   break;
  case SHIP_BOMBER: // 2-engine fighters
  case SHIP_ESCOUT:
   dcol = 1;
// put fall-through friendly fighters here so dcol stays == 0
  case SHIP_FSTRIKE:
  case SHIP_IBEX:
  case SHIP_AUROCHS:
//    circle(display[d], x, y, 10, 14);
//    circle(display[d], x, y, 10, 14);
    angle_draw = EE.angle - camera_angle;
    angle_draw &= ANGLE_MASK;
   sprite_angle = ((angle_draw) >> FIGHTER_ROTATION_BITSHIFT) & FIGHTER_FULL_MASK;
   sprite_angle_flip = 0;
   fsprite = EE.sprite;

//   ldrive_size = EE.engine [0] / 5;
//   rdrive_size = EE.engine [1] / 5;
//   ldrive_size = eclass[EE.type].engine_power [0];//(EE.engine [0] / 10) + 1;
//   rdrive_size = ldrive_size;

   if (sprite_angle >= FIGHTER_ROTATIONS)
   {
    if (sprite_angle >= FIGHTER_ROTATIONS * 3)
    {
      sprite_angle_flip = (FIGHTER_ROTATIONS * 4) - sprite_angle - 1;
      sprite_angle_flip &= FIGHTER_ROTATION_MASK;
//      draw_sprite_h_flip(display[d], FIGHTER_sprite [FIGHTER_SPRITE_BASIC] [sprite_angle_flip].sprite, x - FIGHTER_sprite [FIGHTER_SPRITE_BASIC] [sprite_angle_flip].x, y - FIGHTER_sprite [FIGHTER_SPRITE_BASIC] [sprite_angle_flip].y);
      draw_sprite_h_flip(display[d], fighter_sprite [fsprite] [sprite_angle_flip].sprite, x - (fighter_sprite [fsprite] [sprite_angle_flip].sprite->w - fighter_sprite [fsprite] [sprite_angle_flip].x [0]), y - fighter_sprite [fsprite] [sprite_angle_flip].y [0]);
      //ccircle2(d, x - fighter_sprite [fsprite] [sprite_angle_flip].x [1], y + fighter_sprite [fsprite] [sprite_angle_flip].y [1], 2 + grand(3), 0);
      if (EE.engine_power > 0)
      {
       draw_edrive(d, x - fighter_sprite [fsprite] [sprite_angle_flip].x [1], y + fighter_sprite [fsprite] [sprite_angle_flip].y [1],
        angle_draw, 0 + EE.engine [0], EE.engine [0], 2, 3, dcol); // RH drive
       draw_edrive(d, x - fighter_sprite [fsprite] [sprite_angle_flip].x [2], y + fighter_sprite [fsprite] [sprite_angle_flip].y [2],
        angle_draw, 0 + EE.engine [0], EE.engine [0], 2, 3, dcol); // LH drive
      }
    }
      else
      {
       if (sprite_angle >= FIGHTER_ROTATIONS * 2)
       {
         sprite_angle_flip = sprite_angle - (FIGHTER_ROTATIONS * 2);
         sprite_angle_flip &= FIGHTER_ROTATION_MASK;
         draw_sprite_vh_flip(display[d], fighter_sprite [fsprite] [sprite_angle_flip].sprite, x - (fighter_sprite [fsprite] [sprite_angle_flip].sprite->w - fighter_sprite [fsprite] [sprite_angle_flip].x [0]), y - (fighter_sprite [fsprite] [sprite_angle_flip].sprite->h - fighter_sprite [fsprite] [sprite_angle_flip].y [0]));
         if (EE.engine_power > 0)
         {
          draw_edrive(d, x - fighter_sprite [fsprite] [sprite_angle_flip].x [2], y - fighter_sprite [fsprite] [sprite_angle_flip].y [2],
           angle_draw, 0 + EE.engine [0], EE.engine [0], 2, 3, dcol); // RH drive
          draw_edrive(d, x - fighter_sprite [fsprite] [sprite_angle_flip].x [1], y - fighter_sprite [fsprite] [sprite_angle_flip].y [1],
           angle_draw, 0 + EE.engine [0], EE.engine [0], 2, 3, dcol); // LH drive
         }
       }
        else
        {
          sprite_angle_flip = (FIGHTER_ROTATIONS * 2) - sprite_angle - 1;
          sprite_angle_flip &= FIGHTER_ROTATION_MASK;
          draw_sprite_v_flip(display[d], fighter_sprite [fsprite] [sprite_angle_flip].sprite, x - fighter_sprite [fsprite] [sprite_angle_flip].x [0], y - (fighter_sprite [fsprite] [sprite_angle_flip].sprite->h - fighter_sprite [fsprite] [sprite_angle_flip].y [0]));
          if (EE.engine_power > 0)
          {
           draw_edrive(d, x + fighter_sprite [fsprite] [sprite_angle_flip].x [1], y - fighter_sprite [fsprite] [sprite_angle_flip].y [1],
            angle_draw, 0 + EE.engine [0], EE.engine [0], 2, 3, dcol); // RH drive
           draw_edrive(d, x + fighter_sprite [fsprite] [sprite_angle_flip].x [2], y - fighter_sprite [fsprite] [sprite_angle_flip].y [2],
            angle_draw, 0 + EE.engine [0], EE.engine [0], 2, 3, dcol); // LH drive
          }
        }

      }

   }
    else
    {
      sprite_angle &= FIGHTER_ROTATION_MASK;
      draw_sprite(display[d], fighter_sprite [fsprite] [sprite_angle].sprite, x - fighter_sprite [fsprite] [sprite_angle].x [0], y - fighter_sprite [fsprite] [sprite_angle].y [0]);
      if (EE.engine_power > 0)
      {
       draw_edrive(d, x + fighter_sprite [fsprite] [sprite_angle].x [2], y + fighter_sprite [fsprite] [sprite_angle].y [2],
        angle_draw, 0 + EE.engine [0], EE.engine [0], 2, 3, dcol); // RH drive
       draw_edrive(d, x + fighter_sprite [fsprite] [sprite_angle].x [1], y + fighter_sprite [fsprite] [sprite_angle].y [1],
        angle_draw, 0 + EE.engine [0], EE.engine [0], 2, 3, dcol); // LH drive
      }
    }

/*
DRIVE indexes alternate!
*/

    if (EE.slide_count > 0)
    {
     if (EE.slide_dir == -1)
     {
       draw_edrive(d, x + xpart(angle_draw, 6),// - fighter_sprite [fsprite] [sprite_angle].x [0],
                      y + ypart(angle_draw, 6),// - fighter_sprite [fsprite] [sprite_angle].y [0],
                      angle_draw - ANGLE_4, 4, 4, 2, 2, dcol);

     }
     if (EE.slide_dir == 1)
     {
       draw_edrive(d, x - xpart(angle_draw, 6),// - fighter_sprite [fsprite] [sprite_angle].x [0],
                      y - ypart(angle_draw, 6),// - fighter_sprite [fsprite] [sprite_angle].y [0],
                      angle_draw + ANGLE_4, 4, 4, 2, 2, dcol);

     }
    }

    engine_dist = EE.sprite - FIGHTER_SPRITE_BASIC_1;

//     textprintf_ex(display[0], small_font, x + 10, y + 0, COL_WHITE, -1, "e %i m %i a %i t %i L %i FS %i", e, EE.mission, EE.action, EE.target, EE.leader, EE.formation_size);
//     textprintf_ex(display[0], small_font, x + 10, y + 0, COL_WHITE, -1, "x %i y %i fx %i fy %i", EE.x >> 10, EE.y >> 10, EE.formation_x >> 10, EE.formation_y >> 10);
/*
   if (EE.leader != -1)
   {
   angle = atan2(EE.formation_y - PP.y, EE.formation_x - PP.x);// - (PI/2) - angle_to_radians(PP.angle);
   angle -= (PI/2) + camera_angle_rad;
   dist = hypot(EE.formation_y - PP.y, EE.formation_x - PP.x);

     line(display[d], x, y, PP.camera_x + cos(angle) * (dist >> 10), PP.camera_y + sin(angle) * (dist >> 10), COL_WHITE);
   }
*/

   break;

  case SHIP_OLD2:
  case SHIP_OLD3:
  case SHIP_DROM:
  case SHIP_LINER:
  case SHIP_SCOUT2:
  case SHIP_SCOUT3:
  case SHIP_FRIEND3:
  case SHIP_SCOUTCAR:
  case SHIP_ECARRIER:
  case SHIP_EBASE:
  case SHIP_FREIGHT:
   draw_a_wship(a, e, d, p, x, y);
/*    angle_draw = EE.angle - PP.angle;
    angle_draw &= ANGLE_MASK;
   sprite_angle = ((angle_draw) >> WSHIP_ROTATION_BITSHIFT) & WSHIP_FULL_MASK;
   sprite_angle_flip = 0;
   fsprite = EE.sprite;

   ldrive_size = EE.engine1 / 2;
   rdrive_size = EE.engine2 / 2;
   reduce = 2;
   randsize = 5;

   if (sprite_angle >= WSHIP_ROTATIONS)
   {
    if (sprite_angle >= WSHIP_ROTATIONS * 3)
    {
      sprite_angle_flip = (WSHIP_ROTATIONS * 4) - sprite_angle - 1;
      sprite_angle_flip &= WSHIP_ROTATION_MASK;
      draw_sprite_h_flip(display[d], wship_sprite [fsprite] [sprite_angle_flip].sprite, x - (wship_sprite [fsprite] [sprite_angle_flip].sprite->w - wship_sprite [fsprite] [sprite_angle_flip].x [0]), y - wship_sprite [fsprite] [sprite_angle_flip].y [0]);
      draw_edrive(d, x - wship_sprite [fsprite] [sprite_angle_flip].x [1], y + wship_sprite [fsprite] [sprite_angle_flip].y [1],
       angle_draw, 4 + rdrive_size, rdrive_size, reduce, randsize, 0); // RH drive

      draw_a_turret(d, a, e, 0, p, x - wship_sprite [fsprite] [sprite_angle_flip].x [3], y + wship_sprite [fsprite] [sprite_angle_flip].y [3]);
      draw_a_turret(d, a, e, 1, p, x - wship_sprite [fsprite] [sprite_angle_flip].x [4], y + wship_sprite [fsprite] [sprite_angle_flip].y [4]);

    }
      else
      {
       if (sprite_angle >= WSHIP_ROTATIONS * 2)
       {
         sprite_angle_flip = sprite_angle - (WSHIP_ROTATIONS * 2);
         sprite_angle_flip &= WSHIP_ROTATION_MASK;
         draw_sprite_vh_flip(display[d], wship_sprite [fsprite] [sprite_angle_flip].sprite, x - (wship_sprite [fsprite] [sprite_angle_flip].sprite->w - wship_sprite [fsprite] [sprite_angle_flip].x [0]), y - (wship_sprite [fsprite] [sprite_angle_flip].sprite->h - wship_sprite [fsprite] [sprite_angle_flip].y [0]));
         draw_edrive(d, x - wship_sprite [fsprite] [sprite_angle_flip].x [1], y - wship_sprite [fsprite] [sprite_angle_flip].y [1],
          angle_draw, 4 + rdrive_size, rdrive_size, reduce, randsize, 0); // RH drive

         draw_a_turret(d, a, e, 0, p, x - wship_sprite [fsprite] [sprite_angle_flip].x [3], y - wship_sprite [fsprite] [sprite_angle_flip].y [3]);
         draw_a_turret(d, a, e, 1, p, x - wship_sprite [fsprite] [sprite_angle_flip].x [4], y - wship_sprite [fsprite] [sprite_angle_flip].y [4]);

       }
        else
        {
          sprite_angle_flip = (WSHIP_ROTATIONS * 2) - sprite_angle - 1;
          sprite_angle_flip &= WSHIP_ROTATION_MASK;
          draw_sprite_v_flip(display[d], wship_sprite [fsprite] [sprite_angle_flip].sprite, x - wship_sprite [fsprite] [sprite_angle_flip].x [0], y - (wship_sprite [fsprite] [sprite_angle_flip].sprite->h - wship_sprite [fsprite] [sprite_angle_flip].y [0]));
          draw_edrive(d, x + wship_sprite [fsprite] [sprite_angle_flip].x [1], y - wship_sprite [fsprite] [sprite_angle_flip].y [1],
           angle_draw, 4 + rdrive_size, rdrive_size, reduce, randsize, 0); // RH drive

          draw_a_turret(d, a, e, 0, p, x + wship_sprite [fsprite] [sprite_angle_flip].x [3], y - wship_sprite [fsprite] [sprite_angle_flip].y [3]);
          draw_a_turret(d, a, e, 1, p, x + wship_sprite [fsprite] [sprite_angle_flip].x [4], y - wship_sprite [fsprite] [sprite_angle_flip].y [4]);

        }

      }

   }
    else
    {
      sprite_angle &= WSHIP_ROTATION_MASK;

      draw_sprite(display[d], wship_sprite [fsprite] [sprite_angle].sprite, x - wship_sprite [fsprite] [sprite_angle].x [0], y - wship_sprite [fsprite] [sprite_angle].y [0]);
      draw_edrive(d, x + wship_sprite [fsprite] [sprite_angle].x [1], y + wship_sprite [fsprite] [sprite_angle].y [1],
       angle_draw, 4 + rdrive_size, rdrive_size, reduce, randsize, 0); // RH drive

      draw_a_turret(d, a, e, 0, p, x + wship_sprite [fsprite] [sprite_angle].x [3], y + wship_sprite [fsprite] [sprite_angle].y [3]);
      draw_a_turret(d, a, e, 1, p, x + wship_sprite [fsprite] [sprite_angle].x [4], y + wship_sprite [fsprite] [sprite_angle].y [4]);


    }

/ *
DRIVE indexes alternate!
* /

     TRANS_MODE
   for (i = 0; i < PULSE; i ++)
   {
    if (EE.spulse_time [i] > 0)
    {
     int pangle = EE.spulse_angle [i] + angle_draw;
     int pdist = EE.spulse_dist [i];
     int psize = EE.spulse_size [i];// >> 2;
     int pcol = EE.spulse_time [i] / 6;
     if (pcol > 3)
      pcol = 3;

     xa = x + xpart(pangle, pdist);
     ya = y + ypart(pangle, pdist);
     poly4(display[d],
       xa + xpart(pangle + ANGLE_4 - ANGLE_16, psize),
       ya + ypart(pangle + ANGLE_4 - ANGLE_16, psize),
       xa + xpart(pangle + ANGLE_4 + ANGLE_16, psize),
       ya + ypart(pangle + ANGLE_4 + ANGLE_16, psize),
       xa + xpart(pangle - ANGLE_4 - ANGLE_16, psize),
       ya + ypart(pangle - ANGLE_4 - ANGLE_16, psize),
       xa + xpart(pangle - ANGLE_4 + ANGLE_16, psize),
       ya + ypart(pangle - ANGLE_4 + ANGLE_16, psize),
       trans_col [2] [pcol]);

    }

   }
   END_TRANS*/
   break;



  case 1000://SHIP_EXAMPLE:
   sprite_angle = ((EE.angle - camera_angle) >> WSHIP_ROTATION_BITSHIFT) & WSHIP_FULL_MASK;
   sprite_angle_flip = 0;
//    circlefill(display[d], x, y, 10, 14);


//   draw_rle_sprite(display[d], wship_sprite [WSHIP_SPRITE_BASIC] [sprite_angle].sprite, x - wship_sprite [WSHIP_SPRITE_BASIC] [sprite_angle].x, y - wship_sprite [WSHIP_SPRITE_BASIC] [sprite_angle].y);
   if (sprite_angle >= WSHIP_ROTATIONS)
   {
    if (sprite_angle >= WSHIP_ROTATIONS * 3)
    {
      sprite_angle_flip = (WSHIP_ROTATIONS * 4) - sprite_angle - 1;
      sprite_angle_flip &= WSHIP_ROTATION_MASK;
//      draw_sprite_h_flip(display[d], wship_sprite [WSHIP_SPRITE_BASIC] [sprite_angle_flip].sprite, x - wship_sprite [WSHIP_SPRITE_BASIC] [sprite_angle_flip].x, y - wship_sprite [WSHIP_SPRITE_BASIC] [sprite_angle_flip].y);
      draw_sprite_h_flip(display[d], wship_sprite [WSHIP_SPRITE_OLD2] [sprite_angle_flip].sprite, x - (wship_sprite [WSHIP_SPRITE_OLD2] [sprite_angle_flip].sprite->w - wship_sprite [WSHIP_SPRITE_OLD2] [sprite_angle_flip].x [0]), y - wship_sprite [WSHIP_SPRITE_OLD2] [sprite_angle_flip].y [0]);
    }
      else
      {
       if (sprite_angle >= WSHIP_ROTATIONS * 2)
       {
         sprite_angle_flip = sprite_angle - (WSHIP_ROTATIONS * 2);
         sprite_angle_flip &= WSHIP_ROTATION_MASK;
         draw_sprite_vh_flip(display[d], wship_sprite [WSHIP_SPRITE_OLD2] [sprite_angle_flip].sprite, x - (wship_sprite [WSHIP_SPRITE_OLD2] [sprite_angle_flip].sprite->w - wship_sprite [WSHIP_SPRITE_OLD2] [sprite_angle_flip].x [0]), y - (wship_sprite [WSHIP_SPRITE_OLD2] [sprite_angle_flip].sprite->h - wship_sprite [WSHIP_SPRITE_OLD2] [sprite_angle_flip].y [0]));
       }
        else
        {
          sprite_angle_flip = (WSHIP_ROTATIONS * 2) - sprite_angle - 1;
          sprite_angle_flip &= WSHIP_ROTATION_MASK;
          draw_sprite_v_flip(display[d], wship_sprite [WSHIP_SPRITE_OLD2] [sprite_angle_flip].sprite, x - wship_sprite [WSHIP_SPRITE_OLD2] [sprite_angle_flip].x [0], y - (wship_sprite [WSHIP_SPRITE_OLD2] [sprite_angle_flip].sprite->h - wship_sprite [WSHIP_SPRITE_OLD2] [sprite_angle_flip].y [0]));
        }

      }

   }
    else
    {
      sprite_angle &= WSHIP_ROTATION_MASK;
      draw_sprite(display[d], wship_sprite [WSHIP_SPRITE_OLD2] [sprite_angle].sprite, x - wship_sprite [WSHIP_SPRITE_OLD2] [sprite_angle].x [0], y - wship_sprite [WSHIP_SPRITE_OLD2] [sprite_angle].y [0]);
    }


/*
   turret_sprite_angle = ((EE.turret_angle[0] - PP.angle - ANGLE_4) >> TURRET_ROTATION_BITSHIFT) & TURRET_ROTATION_MASK;
   draw_trans_rle_sprite(display[d], turret_sprite [TURRET_SPRITE_BASIC] [turret_sprite_angle].sprite, x - turret_sprite [TURRET_SPRITE_BASIC] [turret_sprite_angle].x + turret_location [EE.type] [sprite_angle].x [0], y - turret_sprite [TURRET_SPRITE_BASIC] [turret_sprite_angle].y + turret_location [EE.type] [sprite_angle].y [0]);
   turret_sprite_angle = ((EE.turret_angle[1] - PP.angle - ANGLE_4) >> TURRET_ROTATION_BITSHIFT) & TURRET_ROTATION_MASK;
   draw_trans_rle_sprite(display[d], turret_sprite [TURRET_SPRITE_BASIC] [turret_sprite_angle].sprite, x - turret_sprite [TURRET_SPRITE_BASIC] [turret_sprite_angle].x + turret_location [EE.type] [sprite_angle].x [1], y - turret_sprite [TURRET_SPRITE_BASIC] [turret_sprite_angle].y + turret_location [EE.type] [sprite_angle].y [1]);
*/
   break;

   default:
    circlefill(display[d], x, y, 10, 14);
    break;
 }
/*
    print_number(x + 10, y + 10, EE.formation_position);


 char actstr [20] = "nothing";
 switch(EE.action)
 {
  case ACT_AWAY: strcpy(actstr, "Away"); break;
  case ACT_SEEK: strcpy(actstr, "Seek"); break;
  case ACT_ATTACK: strcpy(actstr, "Attack"); break;
  case ACT_EVADE: strcpy(actstr, "Evade"); break;
  case ACT_TRANSIT: strcpy(actstr, "Transit"); break;
  case ACT_FORM: strcpy(actstr, "Form"); break;
  case ACT_GUARD: strcpy(actstr, "Guard"); break;
  case ACT_WING_FORM: strcpy(actstr, "W-Form"); break;
  case ACT_WING_AWAY: strcpy(actstr, "W-Away"); break;
  case ACT_WING_SEEK: strcpy(actstr, "W-Seek"); break;
  case ACT_WING_ATTACK: strcpy(actstr, "W-Attack"); break;
  case ACT_WING_EVADE: strcpy(actstr, "W-Evade"); break;

 }

 textprintf_ex(display[0], small_font, x + 10, y + 10, COL_WHITE, -1, actstr);

 switch(EE.mission)
 {
  case MISSION_ESCORT: strcpy(actstr, "Escort"); break;
  case MISSION_GUARD: strcpy(actstr, "Guard"); break;
  case MISSION_INTERCEPT: strcpy(actstr, "Intercept"); break;
  case MISSION_ATTACK_WSHIP: strcpy(actstr, "Attack Wship"); break;
  case MISSION_PLAYER_WING: strcpy(actstr, "Wing"); break;
  default: strcpy(actstr, "No mission?"); break;
 }

 textprintf_ex(display[0], small_font, x + 10, y + 0, COL_WHITE, -1, actstr);

 textprintf_ex(display[0], small_font, x + 10, y + 20, COL_WHITE, -1, "%i", EE.leader);
*/
/*
    print_number(x + 10, y + 10, e);
    print_number(x + 10, y + 20, EE.leader);
    print_number(x + 10, y + 30, EE.target);
    print_number(x + 10, y + 40, EE.action);
*/
//    print_number(x + 10, y + 10, EE.convoy);
//print_number(x + 10, y + 20, EE.y_speed);
//    print_number(x + 10, y + 30, EE.turning);
//    print_number(x + 10, y + 40, EE.turning_time);
//    print_number(x + 10, y - 50, EE.target_angle);
//    print_number(x + 70, y - 50, player[0].angle);
//    print_number(x + 10, y - 60, EE.turning);
//    print_number(x + 10, y - 70, EE.turning_time);
//    print_number(x + 10, y - 80, EE.wship_throttle);
//    print_number(x + 10, y +15, EE.y_speed);
//    print_number(x + 10, y + 10, EE.angle);
//    print_number(x + 10, y + 25, EE.away_dist);
//    print_number(x + 10, y + 40, EE.think_count);
/*    print_number(x + 10, y, EE.mission);
    print_number(x + 10, y + 20, EE.action);
    print_number(x + 10, y + 40, EE.force_away);*/
/*
    print_number(x + 10, y, EE.target);
    print_number(x + 10, y + 20, EE.action);


//    print_number(x + 10, y + 60, EE.engine_power);

    print_number(x + 10, y + 40, ship[a][e].target);
    print_number(x + 10, y + 80, ship[a][e].formation_size);



    print_number(100, 120, ship[a][0].action);
    print_number(100, 140, ship[a][0].sprite);
    print_number(100, 155, ship[a][0].sprite_count);
*/

// int i = 0;
/*
 while(hull[EE.type].cbox [i] [0] != 0)
 {
  rect(display[d],
   x + (hull[EE.type].cbox [i] [0] >> 10),
   y + (hull[EE.type].cbox [i] [1] >> 10),
   x + (hull[EE.type].cbox [i] [2] >> 10),
   y + (hull[EE.type].cbox [i] [3] >> 10), 218);
  rect(display[d],
   x - 10,
   y - 10,
   x + 10,
   y + 10, 207);
  i ++;
 };
*/

 if (eclass[EE.type].ship_class == ECLASS_FIGHTER)
 {

   int pangle, pdist, ptype, pframe, draw_angle;

    ptype = SHIELD_FIGHTER;

   for (i = 0; i < PULSE; i ++)
   {
    if (EE.spulse_time [i] > 0)
    {
     pangle = ((EE.spulse_angle [i] + angle_draw + ANGLE_4) >> SHIELD_ROTATION_BITSHIFT) & SHIELD_FULL_MASK;
//     pangle = ((EE.spulse_angle [i] + angle_draw) ) & SHIELD_FULL_MASK;
     draw_angle = EE.spulse_angle [i] + angle_draw;
     pdist = EE.spulse_dist [i] + 10;
     pframe = 0;
     if (EE.spulse_time [i] < 90)
      pframe = 1;
     if (EE.spulse_time [i] < 60)
      pframe = 2;
     if (EE.spulse_time [i] < 30)
      pframe = 3;

     draw_trans_sprite(display[d], spulse_sprite [ptype] [pframe] [pangle].sprite, x + xpart(draw_angle, pdist) - spulse_sprite [ptype] [pframe] [pangle].x [0], y + ypart(draw_angle, pdist) - spulse_sprite [ptype] [pframe] [pangle].y [0]);
//     draw_trans_sprite(display[d], spulse_sprite [ptype] [pframe] [pangle].sprite, x - spulse_sprite [ptype] [pframe] [pangle].x [0], y - spulse_sprite [ptype] [pframe] [pangle].y [0]);

    }

   }


 }

 if (a == TEAM_ENEMY)
 {
  int w;
  for (w = 0; w < 2; w ++)
  {
   if (PP.weapon_target [w] [0] == e)
   {
    PP.weapon_sight_x [w] [0] = x;
    PP.weapon_sight_y [w] [0] = y;
    PP.weapon_sight_visible [w] [0] = 1;
   }
  }
 }

 if (a == PP.target_a && e == PP.target_e)
 {
  PP.target_sight_x = x;
  PP.target_sight_y = y;
  PP.target_sight_visible = 1;
 }

}

void draw_a_wship(int a, int e, int d, int p, int x, int y)
{
//if (EE.type == SHIP_OLD2) return;
    int angle_draw = EE.angle - camera_angle;
    angle_draw &= ANGLE_MASK;
   int sprite_angle = ((angle_draw) >> WSHIP_ROTATION_BITSHIFT) & WSHIP_FULL_MASK;
   int sprite_angle_flip = 0;
   int fsprite = EE.sprite;

   int reduce = 2;
   int randsize = 5;
//   int rdrive_size = EE.engine / 2;
   int t;
   int k;
   int i;
   int xa, ya;
   int engine_power;
   int dcol = EE.drive_colour;
   if (EE.jump > 0
    && EE.jump < 50)
    dcol = 2;

// draw_sprite(display[d], ship_collision_mask [SHIP_ECARRIER].sprite, x - wship_sprite [fsprite] [0].x [0] - 9, y - wship_sprite [fsprite] [0].y [0] - 7);
// for testing, must manually change the offsets here rather than in display_init

   if (sprite_angle >= WSHIP_ROTATIONS)
   {
    if (sprite_angle >= WSHIP_ROTATIONS * 3)
    {
      sprite_angle_flip = (WSHIP_ROTATIONS * 4) - sprite_angle - 1;
      sprite_angle_flip &= WSHIP_ROTATION_MASK;
      draw_sprite_h_flip(display[d], wship_sprite [fsprite] [sprite_angle_flip].sprite, x - (wship_sprite [fsprite] [sprite_angle_flip].sprite->w - wship_sprite [fsprite] [sprite_angle_flip].x [0]), y - wship_sprite [fsprite] [sprite_angle_flip].y [0]);

      for (t = 0; t < eclass[EE.type].engines; t ++)
      {
       k = t + 1;
       engine_power = eclass[EE.type].engine_power [t];
       if (EE.jump > 0
        && EE.jump < 50)
         engine_power += 2;
       draw_edrive(d, x - wship_sprite [fsprite] [sprite_angle_flip].x [k], y + wship_sprite [fsprite] [sprite_angle_flip].y [k],
        angle_draw, engine_power, engine_power, reduce, randsize, dcol); // RH drive
      }

      for (t = 0; t < eclass[EE.type].turrets; t ++)
      {
       if (EE.turret_type [t] == TURRET_NONE)
        continue;
       k = eclass[EE.type].engines + 1 + t;
       draw_a_turret(d, a, e, t, p, x - wship_sprite [fsprite] [sprite_angle_flip].x [k], y + wship_sprite [fsprite] [sprite_angle_flip].y [k]);
//       draw_a_turret(d, a, e, 1, p, x - wship_sprite [fsprite] [sprite_angle_flip].x [4], y + wship_sprite [fsprite] [sprite_angle_flip].y [4]);
      }
    }
      else
      {
       if (sprite_angle >= WSHIP_ROTATIONS * 2)
       {
         sprite_angle_flip = sprite_angle - (WSHIP_ROTATIONS * 2);
         sprite_angle_flip &= WSHIP_ROTATION_MASK;
         draw_sprite_vh_flip(display[d], wship_sprite [fsprite] [sprite_angle_flip].sprite, x - (wship_sprite [fsprite] [sprite_angle_flip].sprite->w - wship_sprite [fsprite] [sprite_angle_flip].x [0]), y - (wship_sprite [fsprite] [sprite_angle_flip].sprite->h - wship_sprite [fsprite] [sprite_angle_flip].y [0]));
/*         draw_edrive(d, x - wship_sprite [fsprite] [sprite_angle_flip].x [1], y - wship_sprite [fsprite] [sprite_angle_flip].y [1],
          angle_draw, 4 + rdrive_size, rdrive_size, reduce, randsize, 0); // RH drive

         draw_a_turret(d, a, e, 0, p, x - wship_sprite [fsprite] [sprite_angle_flip].x [3], y - wship_sprite [fsprite] [sprite_angle_flip].y [3]);
         draw_a_turret(d, a, e, 1, p, x - wship_sprite [fsprite] [sprite_angle_flip].x [4], y - wship_sprite [fsprite] [sprite_angle_flip].y [4]);
*/
         for (t = 0; t < eclass[EE.type].engines; t ++)
         {
          k = t + 1;
          engine_power = eclass[EE.type].engine_power [t];
          if (EE.jump > 0
           && EE.jump < 50)
           engine_power += 2;
          draw_edrive(d, x - wship_sprite [fsprite] [sprite_angle_flip].x [k], y - wship_sprite [fsprite] [sprite_angle_flip].y [k],
           angle_draw, engine_power, engine_power, reduce, randsize, dcol); // RH drive
         }

         for (t = 0; t < eclass[EE.type].turrets; t ++)
         {
          if (EE.turret_type [t] == TURRET_NONE)
           continue;
          k = eclass[EE.type].engines + 1 + t;
          draw_a_turret(d, a, e, t, p, x - wship_sprite [fsprite] [sprite_angle_flip].x [k], y - wship_sprite [fsprite] [sprite_angle_flip].y [k]);
//       draw_a_turret(d, a, e, 1, p, x - wship_sprite [fsprite] [sprite_angle_flip].x [4], y + wship_sprite [fsprite] [sprite_angle_flip].y [4]);
         }

       }
        else
        {
          sprite_angle_flip = (WSHIP_ROTATIONS * 2) - sprite_angle - 1;
          sprite_angle_flip &= WSHIP_ROTATION_MASK;
          draw_sprite_v_flip(display[d], wship_sprite [fsprite] [sprite_angle_flip].sprite, x - wship_sprite [fsprite] [sprite_angle_flip].x [0], y - (wship_sprite [fsprite] [sprite_angle_flip].sprite->h - wship_sprite [fsprite] [sprite_angle_flip].y [0]));
          for (t = 0; t < eclass[EE.type].engines; t ++)
          {
           k = t + 1;
           engine_power = eclass[EE.type].engine_power [t];
           if (EE.jump > 0
            && EE.jump < 50)
            engine_power += 2;
           draw_edrive(d, x + wship_sprite [fsprite] [sprite_angle_flip].x [k], y - wship_sprite [fsprite] [sprite_angle_flip].y [k],
            angle_draw, engine_power, engine_power, reduce, randsize, dcol); // RH drive
          }

          for (t = 0; t < eclass[EE.type].turrets; t ++)
          {
           if (EE.turret_type [t] == TURRET_NONE)
            continue;
           k = eclass[EE.type].engines + 1 + t;
           draw_a_turret(d, a, e, t, p, x + wship_sprite [fsprite] [sprite_angle_flip].x [k], y - wship_sprite [fsprite] [sprite_angle_flip].y [k]);
//        draw_a_turret(d, a, e, 1, p, x - wship_sprite [fsprite] [sprite_angle_flip].x [4], y + wship_sprite [fsprite] [sprite_angle_flip].y [4]);
          }
/*
          draw_edrive(d, x + wship_sprite [fsprite] [sprite_angle_flip].x [1], y - wship_sprite [fsprite] [sprite_angle_flip].y [1],
           angle_draw, 4 + rdrive_size, rdrive_size, reduce, randsize, 0); // RH drive

          draw_a_turret(d, a, e, 0, p, x + wship_sprite [fsprite] [sprite_angle_flip].x [3], y - wship_sprite [fsprite] [sprite_angle_flip].y [3]);
          draw_a_turret(d, a, e, 1, p, x + wship_sprite [fsprite] [sprite_angle_flip].x [4], y - wship_sprite [fsprite] [sprite_angle_flip].y [4]);
*/
        }

      }

   }
    else
    {
      sprite_angle &= WSHIP_ROTATION_MASK;

      draw_sprite(display[d], wship_sprite [fsprite] [sprite_angle].sprite, x - wship_sprite [fsprite] [sprite_angle].x [0], y - wship_sprite [fsprite] [sprite_angle].y [0]);

      for (t = 0; t < eclass[EE.type].engines; t ++)
      {
       k = t + 1;
       engine_power = eclass[EE.type].engine_power [t];
       if (EE.jump > 0
        && EE.jump < 50)
         engine_power += 2;
       draw_edrive(d, x + wship_sprite [fsprite] [sprite_angle].x [k], y + wship_sprite [fsprite] [sprite_angle].y [k],
        angle_draw, engine_power, engine_power, reduce, randsize, dcol); // RH drive
      }

      for (t = 0; t < eclass[EE.type].turrets; t ++)
      {
       if (EE.turret_type [t] == TURRET_NONE)
        continue;
       k = eclass[EE.type].engines + 1 + t;
       draw_a_turret(d, a, e, t, p, x + wship_sprite [fsprite] [sprite_angle].x [k], y + wship_sprite [fsprite] [sprite_angle].y [k]);
//       draw_a_turret(d, a, e, 1, p, x - wship_sprite [fsprite] [sprite_angle_flip].x [4], y + wship_sprite [fsprite] [sprite_angle_flip].y [4]);
      }
/*
      draw_edrive(d, x + wship_sprite [fsprite] [sprite_angle].x [1], y + wship_sprite [fsprite] [sprite_angle].y [1],
       angle_draw, 4 + rdrive_size, rdrive_size, reduce, randsize, 0); // RH drive

      draw_a_turret(d, a, e, 0, p, x + wship_sprite [fsprite] [sprite_angle].x [3], y + wship_sprite [fsprite] [sprite_angle].y [3]);
      draw_a_turret(d, a, e, 1, p, x + wship_sprite [fsprite] [sprite_angle].x [4], y + wship_sprite [fsprite] [sprite_angle].y [4]);
*/

    }

/*

Remember: if more than one drive, need to account for flipping! The indices alternate!

*/
/*
     TRANS_MODE
   for (i = 0; i < PULSE; i ++)
   {
    if (EE.spulse_time [i] > 0)
    {
     int pangle = EE.spulse_angle [i] + angle_draw;
     int pdist = EE.spulse_dist [i];
     int psize = EE.spulse_size [i];// >> 2;
     int pcol = EE.spulse_time [i] / 6;
     if (pcol > 3)
      pcol = 3;

     xa = x + xpart(pangle, pdist);
     ya = y + ypart(pangle, pdist);
     poly4(display[d],
       xa + xpart(pangle + ANGLE_4 - ANGLE_16, psize),
       ya + ypart(pangle + ANGLE_4 - ANGLE_16, psize),
       xa + xpart(pangle + ANGLE_4 + ANGLE_16, psize),
       ya + ypart(pangle + ANGLE_4 + ANGLE_16, psize),
       xa + xpart(pangle - ANGLE_4 - ANGLE_16, psize),
       ya + ypart(pangle - ANGLE_4 - ANGLE_16, psize),
       xa + xpart(pangle - ANGLE_4 + ANGLE_16, psize),
       ya + ypart(pangle - ANGLE_4 + ANGLE_16, psize),
       trans_col [2] [pcol]);

    }

   }
   END_TRANS

*/
     int pangle, pdist, ptype, pframe, draw_angle;

   for (i = 0; i < PULSE; i ++)
   {
    if (EE.spulse_time [i] > 0)
    {
     pangle = ((EE.spulse_angle [i] + angle_draw + ANGLE_4) >> SHIELD_ROTATION_BITSHIFT) & SHIELD_FULL_MASK;
//     pangle = ((EE.spulse_angle [i] + angle_draw) ) & SHIELD_FULL_MASK;
     draw_angle = EE.spulse_angle [i] + angle_draw;
     pdist = EE.spulse_dist [i] + 10;
     ptype = 0;
     pframe = 0;
     if (EE.spulse_time [i] < 80)
      pframe = 1;
     if (EE.spulse_time [i] < 60)
      pframe = 2;
     if (EE.spulse_time [i] < 40)
      pframe = 3;
     if (EE.spulse_time [i] < 20)
      pframe = 4;

     xa = x + xpart(draw_angle, pdist);
     ya = y + ypart(draw_angle, pdist);
//     print_number(x, y, pframe);
// pangle = 0;

     draw_trans_sprite(display[d], spulse_sprite [ptype] [pframe] [pangle].sprite, xa - spulse_sprite [ptype] [pframe] [pangle].x [0], ya - spulse_sprite [ptype] [pframe] [pangle].y [0]);
//     draw_trans_sprite(display[d], spulse_sprite [ptype] [pframe] [pangle].sprite, x - spulse_sprite [ptype] [pframe] [pangle].x [0], y - spulse_sprite [ptype] [pframe] [pangle].y [0]);

    }

   }
/*
  print_number(x + 10, y + 20, radians_to_angle(atan2(PP.y + ypart(PP.angle+ANGLE_2, 100<<10) - EE.y,
                                    PP.x + xpart(PP.angle+ANGLE_2, 100<<10) - EE.x)));
  print_number(x + 10, y + 40, radians_to_angle(atan2(PP.y - EE.y,
                                    PP.x  - EE.x)));
  print_number(x + 10, y + 60, angle_difference(radians_to_angle(atan2(PP.y + ypart(PP.angle+ANGLE_2, 100<<10) - EE.y,
                                    PP.x + xpart(PP.angle+ANGLE_2, 100<<10) - EE.x)),
                                    radians_to_angle(atan2(PP.y - EE.y,
                                    PP.x  - EE.x))));
*/
/*
   float angle = atan2(EE.convoy_y - PP.y, EE.convoy_x - PP.x) - (PI/2) - angle_to_radians(PP.angle);
   int dist = hypot(EE.convoy_y - PP.y, EE.convoy_x - PP.x);
   int x2 = PP.camera_x + cos(angle) * (dist >> 10);
   int y2 = PP.camera_y + sin(angle) * (dist >> 10);

   pline(display[d], x, y, x2, y2, COL_F4);

     print_number(x, y + 20, EE.turning);
     print_number(x, y + 40, EE.turning_time);
*/
/*
int v;

for (v = 0; v < 3; v ++)
{
   int angle = radians_to_angle(atan2(rotate_pos_y [eclass[EE.type].base_turret_rp + v] [EE.angle], rotate_pos_x [eclass[EE.type].base_turret_rp + v] [EE.angle])) + EE.angle - PP.angle;
   angle += ANGLE_4;
   int dist = hypot(rotate_pos_y [eclass[EE.type].base_turret_rp + v] [EE.angle], rotate_pos_x [eclass[EE.type].base_turret_rp + v] [EE.angle]);
   int x2 = x + xpart(angle, dist >> 10);
   int y2 = y + ypart(angle, dist >> 10);

   print_number(x2, y2, v);

}

*/
}



void draw_a_turret(int d, int a, int e, int t, int p, int x, int y)
{

   int angle_draw, sprite_angle;
   angle_draw = EE.turret_angle [t] - camera_angle;
   angle_draw &= ANGLE_MASK;
   sprite_angle = ((angle_draw) >> TURRET_ROTATION_BITSHIFT) & TURRET_FULL_MASK;
//   int sprite_angle_flip = 0;
   int fsprite = 0;//EE.turret_sprite;

   switch (EE.turret_type [t])
   {
/*    case TURRET_BASIC:
     fsprite = TURRET_SPRITE_BASIC_1;
     if (EE.turret_recoil [t] > 0)
      fsprite = TURRET_SPRITE_BASIC_2;
     if (EE.turret_recoil [t] > 2)
      fsprite = TURRET_SPRITE_BASIC_3;
      break;*/
     case TURRET_BASIC:
      fsprite = TURRET_SPRITE_BASIC_1;
     if (EE.turret_recoil [t] > 0)
      fsprite = TURRET_SPRITE_BASIC_2 + (EE.turret_side [t] == 1) * 2;
     if (EE.turret_recoil [t] > 4)
      fsprite = TURRET_SPRITE_BASIC_3 + (EE.turret_side [t] == 1) * 2;
      break;
     case TURRET_HEAVY:
      fsprite = TURRET_SPRITE_HEAVY_1;
     if (EE.turret_recoil [t] > 0)
      fsprite = TURRET_SPRITE_HEAVY_2 + (EE.turret_side [t] == 1) * 2;
     if (EE.turret_recoil [t] > 4)
      fsprite = TURRET_SPRITE_HEAVY_3 + (EE.turret_side [t] == 1) * 2;
//     if (EE.turret_side [t] == 1)
//      fsprite += 2;
      break;
    case TURRET_EBASIC:
     fsprite = TURRET_SPRITE_EBASIC_1;
     if (EE.turret_burst [t] == BURST_FIRING)
     {
      fsprite = TURRET_SPRITE_EBASIC_4;
      if (EE.turret_status [t] > 0)
      {
       fsprite = TURRET_SPRITE_EBASIC_3;
       if (EE.turret_status [t] > 3)
        fsprite = TURRET_SPRITE_EBASIC_2;
       break;
      }
       if (EE.turret_recoil [t] > 0)
        fsprite = TURRET_SPRITE_EBASIC_5;
       if (EE.turret_recoil [t] > 2)
        fsprite = TURRET_SPRITE_EBASIC_6;
     }

     if (EE.turret_burst [t] == BURST_CHARGING)
     {
       if (EE.turret_recoil [t] > 0)
        fsprite = TURRET_SPRITE_EBASIC_2;
       if (EE.turret_recoil [t] > 3)
        fsprite = TURRET_SPRITE_EBASIC_3;
       if (EE.turret_recoil [t] > 7)
        fsprite = TURRET_SPRITE_EBASIC_4;
       if (EE.turret_recoil [t] > 10)
        fsprite = TURRET_SPRITE_EBASIC_5;
       if (EE.turret_recoil [t] > 13)
        fsprite = TURRET_SPRITE_EBASIC_6;
     }
     break;
    case TURRET_EANTI:
     fsprite = TURRET_SPRITE_EANTI_1;
     if (EE.turret_burst [t] == BURST_FIRING)
     {
      fsprite = TURRET_SPRITE_EANTI_5;
      if (EE.turret_status [t] > 0)
      {
       fsprite = TURRET_SPRITE_EANTI_4;
       if (EE.turret_status [t] > 3)
        fsprite = TURRET_SPRITE_EANTI_3;
       break;
      }
     }

     if (EE.turret_burst [t] == BURST_CHARGING)
     {
       if (EE.turret_recoil [t] > 0)
        fsprite = TURRET_SPRITE_EANTI_2;
       if (EE.turret_recoil [t] > 3)
        fsprite = TURRET_SPRITE_EANTI_3;
       if (EE.turret_recoil [t] > 7)
        fsprite = TURRET_SPRITE_EANTI_4;
       if (EE.turret_recoil [t] > 10)
        fsprite = TURRET_SPRITE_EANTI_5;
     }
     break;

     case TURRET_EHEAVY:
      fsprite = TURRET_SPRITE_EHEAVY_4;
     if (EE.turret_status [t] > 0)
      fsprite = TURRET_SPRITE_EHEAVY_3;
     if (EE.turret_status [t] > 4)
      fsprite = TURRET_SPRITE_EHEAVY_2;
     if (EE.turret_status [t] > 8)
      fsprite = TURRET_SPRITE_EHEAVY_1;
      break;
     case TURRET_EBEAM:
      fsprite = TURRET_SPRITE_EBEAM_1;
     if (EE.turret_status [t] > 0)
      fsprite = TURRET_SPRITE_EBEAM_2;
     if (EE.turret_status [t] > 4)
      fsprite = TURRET_SPRITE_EBEAM_3;
     if (EE.turret_status [t] > 8)
      fsprite = TURRET_SPRITE_EBEAM_4;
      break;

     case TURRET_ELONG:
      fsprite = TURRET_SPRITE_ELONG_4;
     if (EE.turret_status [t] > 0)
      fsprite = TURRET_SPRITE_ELONG_3;
     if (EE.turret_status [t] > 4)
      fsprite = TURRET_SPRITE_ELONG_2;
     if (EE.turret_status [t] > 8)
      fsprite = TURRET_SPRITE_ELONG_1;
      break;
     case TURRET_CGUN:
      fsprite = TURRET_SPRITE_CGUN_1;
      if (EE.turret_recoil [t] > 0)
       fsprite = TURRET_SPRITE_CGUN_2;
      if (EE.turret_recoil [t] > 5)
       fsprite = TURRET_SPRITE_CGUN_3;
      break;
     case TURRET_CLAUNCHER:
      fsprite = TURRET_SPRITE_CLAUNCHER_1;
      break;
   }

/*
   if (sprite_angle >= TURRET_ROTATIONS)
   {
    if (sprite_angle >= TURRET_ROTATIONS * 3)
    {
      sprite_angle_flip = (TURRET_ROTATIONS * 4) - sprite_angle - 1;
      sprite_angle_flip &= TURRET_ROTATION_MASK;
      draw_sprite_h_flip(display[d], turret_sprite [fsprite] [sprite_angle_flip].sprite, x - (turret_sprite [fsprite] [sprite_angle_flip].sprite->w - turret_sprite [fsprite] [sprite_angle_flip].x [0]), y - turret_sprite [fsprite] [sprite_angle_flip].y [0]);
    }
      else
      {
       if (sprite_angle >= TURRET_ROTATIONS * 2)
       {
         sprite_angle_flip = sprite_angle - (TURRET_ROTATIONS * 2);
         sprite_angle_flip &= TURRET_ROTATION_MASK;
         draw_sprite_vh_flip(display[d], turret_sprite [fsprite] [sprite_angle_flip].sprite, x - (turret_sprite [fsprite] [sprite_angle_flip].sprite->w - turret_sprite [fsprite] [sprite_angle_flip].x [0]), y - (turret_sprite [fsprite] [sprite_angle_flip].sprite->h - turret_sprite [fsprite] [sprite_angle_flip].y [0]));
       }
        else
        {
          sprite_angle_flip = (TURRET_ROTATIONS * 2) - sprite_angle - 1;
          sprite_angle_flip &= TURRET_ROTATION_MASK;
          draw_sprite_v_flip(display[d], turret_sprite [fsprite] [sprite_angle_flip].sprite, x - turret_sprite [fsprite] [sprite_angle_flip].x [0], y - (turret_sprite [fsprite] [sprite_angle_flip].sprite->h - turret_sprite [fsprite] [sprite_angle_flip].y [0]));
        }

      }

   }
    else
    {*/
//      sprite_angle &= TURRET_ROTATION_MASK;
ALPHA_TABLE
      draw_trans_sprite(display[d], turret_sprite [fsprite] [sprite_angle].sprite, x - turret_sprite [fsprite] [sprite_angle].x [0], y - turret_sprite [fsprite] [sprite_angle].y [0]);
      TRANS_TABLE

//print_number(x + 20, y, EE.turret_target [t]);
//print_number(x + 50, y, EE.turret_burst [t]);

//    }


}








//#define SHOW_NUMBER
/*
void draw_edrive(int d, int x, int y, int angle, int dist, int size, int reduce, int randsize, int col)
{
    if (size <= 0)
     return;

// int i;
// int dist = 0;
 angle += ANGLE_4;

 x += xpart(angle, dist);
 y += ypart(angle, dist);

 ccircle(d, x, y, size + grand(randsize), col);
 ccircle3(d, x, y, (size*1.5) + grand(randsize), col);
 ccircle2(d, x, y, (size<<1) + grand(randsize), col);

 x += xpart(angle, size * 2);
 y += ypart(angle, size * 2);
// size -= reduce;// + grand(reduce);

#ifdef SHOW_NUMBER
int num = 0;
#endif

 while(size > 0)
 {
  ccircle2(d, x, y, size + grand(randsize), col);
  x += xpart(angle, size + grand(randsize));
  y += ypart(angle, size + grand(randsize));
  size -= reduce + grand(reduce);
#ifdef SHOW_NUMBER
num++;
#endif
 };

#ifdef SHOW_NUMBER
print_number(x + 30, y, num);
#endif

}

*/


void draw_edrive(int d, int x, int y, int angle, int dist, int size, int reduce, int randsize, int col)
{
    if (size <= 0)
     return;

 angle += ANGLE_4;

 x += xpart(angle, dist);
 y += ypart(angle, dist);

 ccircle(d, x, y, (size>>1) + grand(randsize), col);
 ccircle3(d, x, y, (size*0.7) + grand(randsize), col);
 ccircle2(d, x, y, size + grand(randsize), col);

 x += xpart(angle, size);
 y += ypart(angle, size);
// size -= reduce;// + grand(reduce);

#ifdef SHOW_NUMBER
int num = 0;
#endif

 while(size > 0)
 {
  ccircle2(d, x, y, size + grand(randsize), col);
  x += xpart(angle, size + grand(randsize));
  y += ypart(angle, size + grand(randsize));
  size -= reduce + grand(reduce);
#ifdef SHOW_NUMBER
num++;
#endif
 };

#ifdef SHOW_NUMBER
print_number(x + 30, y, num);
#endif

}

/*

void draw_edrive(int d, int x, int y, int angle, int dist, int size, int reduce, int randsize, int col)
{
    if (size <= 0)
     return;

// int i;
// int dist = 0;
 angle += ANGLE_4;

 x += xpart(angle, dist);
 y += ypart(angle, dist);

 ccircle(d, x, y, size + grand(randsize), col);
 ccircle3(d, x, y, (size*1.5) + grand(randsize), col);
 ccircle2(d, x, y, (size<<1) + grand(randsize), col);

 x += xpart(angle, size * 2);
 y += ypart(angle, size * 2);
// size -= reduce;// + grand(reduce);

#ifdef SHOW_NUMBER
int num = 0;
#endif

 while(size > 0)
 {
  ccircle2(d, x, y, size + grand(randsize), col);
  x += xpart(angle, size + grand(randsize));
  y += ypart(angle, size + grand(randsize));
  size -= reduce + grand(reduce);
#ifdef SHOW_NUMBER
num++;
#endif
 };

#ifdef SHOW_NUMBER
print_number(x + 30, y, num);
#endif

}

*/

/*
#define SHOW_NUMBER

void draw_edrive(int d, int x, int y, int angle, int dist, int size, int reduce, int randsize, int col)
{
    if (size <= 0)
     return;

// int i;
// int dist = 0;
 angle += ANGLE_4;

 x += xpart(angle, dist);
 y += ypart(angle, dist);

 ccircle(d, x, y, size + grand(randsize), col);
 ccircle2(d, x, y, (size*2) + grand(randsize), col);

 x += xpart(angle, size);
 y += ypart(angle, size);
 size -= reduce + grand(reduce);

 size *= 2;

#ifdef SHOW_NUMBER
int num = 0;
#endif

 while(size > 0)
 {
  ccircle2(d, x, y, size + grand(randsize), col);
  x += xpart(angle, size);
  y += ypart(angle, size);
  size -= reduce + grand(reduce);
#ifdef SHOW_NUMBER
num++;
#endif
 };

#ifdef SHOW_NUMBER
print_number(x + 30, y, num);
#endif

}

*/
#define SCANNER_X scan_x
#define SCANNER_Y scan_y
//#define SCANNER_Y 540
#define SCANNER_SIZE 50

 int acol [SCANCOLS] [7] [2] =
 {
//    {COL_F6 + TRANS_BLUE1, COL_F7 + TRANS_BLUE2},
//    {COL_E6 + TRANS_RED1, COL_E7 + TRANS_RED2}
   { // SCANCOL_OCSF:
    {TRANS_BLUE1, TRANS_BLUE3},
    {TRANS_BLUE1, TRANS_BLUE3},
    {TRANS_BLUE1, TRANS_BLUE3},
    {TRANS_BLUE2, TRANS_BLUE3},
    {TRANS_BLUE2, TRANS_BLUE4},
    {TRANS_BLUE2, TRANS_BLUE4},
    {TRANS_BLUE3, TRANS_BLUE4},
   },
   { // SCANCOL_FED:
    {TRANS_RED1, TRANS_RED2},
    {TRANS_RED2, TRANS_RED2},
    {TRANS_RED2, TRANS_RED2},
    {TRANS_RED2, TRANS_RED3},
    {TRANS_RED2, TRANS_RED4},
    {TRANS_RED2, TRANS_RED4},
    {TRANS_RED3, TRANS_RED4},

   },
   { // SCANCOL_CWLTH:
    {TRANS_GREEN2, TRANS_GREEN2},
    {TRANS_GREEN2, TRANS_GREEN2},
    {TRANS_GREEN2, TRANS_GREEN2},
    {TRANS_GREEN2, TRANS_GREEN2},
    {TRANS_GREEN2, TRANS_GREEN3},
    {TRANS_GREEN2, TRANS_GREEN3},
    {TRANS_GREEN2, TRANS_GREEN3},
/*    {COL_CBOX2, TRANS_GREEN2},
    {COL_CBOX2, TRANS_GREEN2},
    {COL_CBOX2, TRANS_GREEN2},
    {COL_CBOX2, TRANS_GREEN2},
    {COL_CBOX2, TRANS_GREEN3},
    {COL_CBOX2, TRANS_GREEN3},
    {COL_CBOX2, TRANS_GREEN3},*/
   },


//    {TRANS_BLUE2, TRANS_BLUE3},
//    {TRANS_RED2, TRANS_RED3}
 };


#define HP_BAR_X 5
#define HP_BAR_Y 15
#define HP_BAR_THICK 3
#define HP_BAR_END_THICK 5

#define SH_BAR_Y 35

#define LIVES_Y 55

void draw_final_details(void)
{
 int i, x, y;

 if (arena.only_player != -1)
 {
  y = SH_BAR_Y + HP_BAR_THICK + 10;
  for (i = 0; i < player[0].ships; i ++)
  {
   x = HP_BAR_X + (i*16);
   rectfill(display[0], x, y, x + 10, y + 10, COL_F6);
  }
 }
  else
  {
   if (player[0].ships > 0)
   {
     int x2;
     x2 = player[0].ships * 8;
     vline(display[0], 400, 0, LIVES_Y, COL_F3);
     rect(display[0], 400 - x2 - 1, LIVES_Y, 400 + x2 + 1, LIVES_Y + 16, COL_F3);
     vline(display[0], 400, LIVES_Y + 16, 600, COL_F3);
     y = LIVES_Y + 3;
     for (i = 0; i < player[0].ships; i ++)
     {
      x = 400 - x2 + (i*16);
      rectfill(display[0], x + 3, y, x + 13, y + 10, COL_F6);
     }
   }
    else
    {
     vline(display[0], 400, 0, 600, COL_F3);
    }
   draw_overscan(325, 440);
  }




 y = 20;
 int rsize;
// int x_plus;
 int size2;
 int mcol;
 int header_lines;


 for (i = 0; i < COMMS; i ++)
 {
  if (comm[i].exists == 0)
   continue;
  rsize = comm[i].lines*MESSAGE_LINE_SPACE + 4 + COMM_END_SPACE;//COMM_END_SPACE;
  if (comm[i].from_type != SHIP_NONE)
   rsize += MESSAGE_LINE_SPACE;
  if (comm[i].to != MSG_TO_ALL)
   rsize += MESSAGE_LINE_SPACE;
  //msg [comm[i].message].lines * 12 + 14;

/*  x_plus = 0;
  if (comm[i].fade > 0)
  {
//   rsize -= comm[i].fade;
   x_plus = comm[i].fade;
//   rect(display[0], 550 + x_plus, y, 800, y + rsize, COL_F5);
//   y += 4 + rsize + 20;
//   continue;
  }*/
  y += comm[i].y_offset;
  if (comm[i].flash > 0)
  {
   x = 299 + (comm[i].flash<<1);
   if (x < 549)
    x = 549;
   size2 = (250 - comm[i].flash)>>2;
   if (size2 > (rsize>>1))
    size2 = (rsize>>1);
   mcol = COMM_COL_MAX;// + TRANS_BLUE2;
   if (comm[i].flash < 125)
   {
//    mcol = comm[i].col_max - ((125-comm[i].flash)>>3);
//    if (mcol < comm[i].col_min)
//     mcol = comm[i].col_min;
    mcol = COMM_COL_MAX - ((125-comm[i].flash)>>3);
    if (mcol < COMM_COL_MIN)
     mcol = COMM_COL_MIN;
   }
   rectfill(display[0], x, y+(rsize>>1)-size2, 800, y+(rsize>>1)+(rsize-(rsize>>1)), comm_col [comm[i].comm_col] [mcol]);// + TRANS_BLUE2);
   if (mcol < COMM_COL_MIN + 1)
    rect(display[0], x, y+(rsize>>1)-size2, 800, y+(rsize>>1)+(rsize-(rsize>>1)), comm_col [comm[i].comm_col] [COMM_COL_MIN + 1]);
   if (comm[i].flash < 125)
   {
    header_lines = (comm[i].from_type != SHIP_NONE);
    header_lines += (comm[i].to != MSG_TO_ALL);
    if (header_lines > 0)
    {
     mcol ++;
     if (mcol > COMM_COL_MAX)
      mcol = COMM_COL_MAX;
     rectfill(display[0], x, y, 800, y + header_lines*MESSAGE_LINE_SPACE, comm_col [comm[i].comm_col] [mcol]);
    }
    display_message_header(i, x, y);
    display_message(display[0], ctext[comm[i].ctext], x + 5, y+4 + header_lines*MESSAGE_LINE_SPACE);
   }

//   rect(display[0], 549 + comm[i].flash, y, 800, y + rsize, COL_F5);

//   x_plus = comm[i].fade;
//   rect(display[0], 550 + x_plus, y, 800, y + rsize, COL_F5);
   y += rsize + COMM_BETWEEN; // also in level.c where y_offset is set
    continue;
  }
   x = 299 + (comm[i].fade<<1);
   if (x < 549)
    x = 549;
  rectfill(display[0], x, y, 800, y + rsize, comm_col [comm[i].comm_col] [COMM_COL_MIN]);
  rect(display[0], x, y, 800, y + rsize, comm_col [comm[i].comm_col] [COMM_COL_MIN+1]);
  header_lines = (comm[i].from_type != SHIP_NONE);
  header_lines += (comm[i].to != MSG_TO_ALL);
  if (header_lines > 0)
   rectfill(display[0], x, y, 800, y + header_lines*MESSAGE_LINE_SPACE, comm_col [comm[i].comm_col] [COMM_COL_MIN+1]);
  display_message_header(i, x, y);
//  y += 4;
  display_message(display[0], ctext[comm[i].ctext], x + 5, y + 4 + header_lines*MESSAGE_LINE_SPACE);
/*  for (l = 0; l < msg [comm[i].message].lines; l ++)
  {
   textprintf_right_ex(display[0], font, 790 + comm[i].fade, y, -1, -1, msg [comm[i].message].line [l]);
   y += 12;
  }*/
  y += (comm[i].lines + header_lines)*MESSAGE_LINE_SPACE;

  y += 4 + COMM_END_SPACE + COMM_BETWEEN;

 }

 int seconds, minutes;

 minutes = arena.time / 60;
 seconds = arena.time % 60;

 if (seconds > 9)
  textprintf_right_ex(display[0], small_font, 797, 5, COL_BOX3, -1, "%i:%i", minutes, seconds);
   else
    textprintf_right_ex(display[0], small_font, 797, 5, COL_BOX3, -1, "%i:0%i", minutes, seconds);

}

char message_header [40];

void display_message_header(int i, int x, int y)
{

 message_header [0] = '\0';
// int pix;

 if (comm[i].from_type != SHIP_NONE)
 {
  switch(comm[i].from_type)
  {
   case SHIP_OLD2: strcpy(message_header, "CTBR-SUNSHARK "); break;
   case SHIP_OLD3: strcpy(message_header, "CTBR-STARWHALE "); break;
   case SHIP_DROM: strcpy(message_header, "CTBR-DROMEDARY "); break;
   case SHIP_LINER: strcpy(message_header, "STARLINER "); break;

   case SHIP_FRIEND3: strcpy(message_header, "IF-TRIREME "); break;
   case SHIP_SCOUT2: strcpy(message_header, "FSF-RIGHTEOUS "); break;
   case SHIP_SCOUT3: strcpy(message_header, "FSF-MERCIFUL "); break;




  }
//  pix = text_length(small_font, message_header);
  //textprintf_ex(display[0], small_font, x + 3, y, COL_STAR1, -1, message_header);
  if (comm[i].from_letter != -1)
  {
   switch(comm[i].from_letter)
   {
    case 0: strcat(message_header, "Gazer"); break;
    case 1: strcat(message_header, "Beta"); break;
    case 2: strcat(message_header, "Gamma"); break;
//   default: strcat(message_header, "Unknown"); break;
   }
  }
  if (comm[i].from_rank == -1)
   textprintf_ex(display[0], small_font, x + 3, y, COL_WHITE, -1, message_header);
    else
     textprintf_ex(display[0], small_font, x + 3, y, COL_WHITE, -1, "%s %i", message_header, comm[i].from_rank);
 }


 if (comm[i].to != MSG_TO_ALL)
 {
  switch(comm[i].to)
  {
   case MSG_TO_AM:
    textprintf_ex(display[0], small_font, x + 3, y + 12, COL_WHITE, -1, "  >> Angry Moth"); break;
  }
 }
}


void draw_HUD(int d, int p)
{


 int scan_x = 730;
// int scan_x = 60;
 int scan_y = 380;

 if (arena.only_player == -1)
 {
   scan_y = 535;
   scan_x = 270;
   if (p == 1)
    scan_x = 130;

//  scan_x = 340;
//  scan_x = 60;

 }


 int i, j, col, x, y;

 int angle, dist;

 if (PP.alive == 0)
  return;

#define TORP_Y 350

#define HUD_COL_1 COL_EBOX4
#define HUD_COL_2 COL_WHITE
#define HUD_COL_3 COL_WHITE


// let's draw secondary weapon details:
//  first, for non-fixed camera:
if (!arena.camera_fix)
{
 for (i = 0; i < 2; i ++)
 {
  switch(PP.weapon_type [i])
  {
   case WPN_TORP:
   torp_arming_lines(d, p, i, 40, 160, 40);

    if (PP.weapon_charge [i] > 0)
    {
     vline(display[d], PP.camera_x - 40, TORP_Y - 5, TORP_Y + 5, HUD_COL_3);
     vline(display[d], PP.camera_x + 40, TORP_Y - 5, TORP_Y + 5, HUD_COL_3);

     vline(display[d], PP.camera_x - 40 + PP.weapon_charge [i], TORP_Y - 2, TORP_Y + 2, HUD_COL_3);
     hline(display[d], PP.camera_x - 40, TORP_Y, PP.camera_x - 40 + PP.weapon_charge [i], HUD_COL_3);
     vline(display[d], PP.camera_x + 40 - PP.weapon_charge [i], TORP_Y - 2, TORP_Y + 2, HUD_COL_3);
     hline(display[d], PP.camera_x + 40, TORP_Y, PP.camera_x + 40 - PP.weapon_charge [i], HUD_COL_3);
    } else
     {
      if (PP.weapon_charge [i] < 0)
      {
       vline(display[d], PP.camera_x - 40, TORP_Y - 5, TORP_Y + 5, HUD_COL_1);
       vline(display[d], PP.camera_x + 40, TORP_Y - 5, TORP_Y + 5, HUD_COL_1);

       vline(display[d], PP.camera_x - 40 - (PP.weapon_charge [i]>>2), TORP_Y - 2, TORP_Y + 2, HUD_COL_1);
       hline(display[d], PP.camera_x - 40, TORP_Y, PP.camera_x - 40 - (PP.weapon_charge [i]>>2), HUD_COL_1);
       vline(display[d], PP.camera_x + 40 + (PP.weapon_charge [i]>>2), TORP_Y - 2, TORP_Y + 2, HUD_COL_1);
       hline(display[d], PP.camera_x + 40, TORP_Y, PP.camera_x + 40 + (PP.weapon_charge [i]>>2), HUD_COL_1);
      }
       else
       {
        vline(display[d], PP.camera_x - 40, TORP_Y - 5, TORP_Y + 5, HUD_COL_2);
        vline(display[d], PP.camera_x + 40, TORP_Y - 5, TORP_Y + 5, HUD_COL_2);
       }
     }

    break;
   case WPN_HROCKET:
   torp_arming_lines(d, p, i, 40, 80, 40);
   break;
   case WPN_WROCKET:
   torp_arming_lines(d, p, i, 40, 80, 40);

/*
    if (PP.weapon_charge [i] > 0)
    {
     vline(display[d], PP.camera_x - 40, TORP_Y - 5, TORP_Y + 5, HUD_COL_3);
     vline(display[d], PP.camera_x + 40, TORP_Y - 5, TORP_Y + 5, HUD_COL_3);

     vline(display[d], PP.camera_x - 40 + PP.weapon_charge [i], TORP_Y - 2, TORP_Y + 2, HUD_COL_3);
     hline(display[d], PP.camera_x - 40, TORP_Y, PP.camera_x - 40 + PP.weapon_charge [i], HUD_COL_3);
     vline(display[d], PP.camera_x + 40 - PP.weapon_charge [i], TORP_Y - 2, TORP_Y + 2, HUD_COL_3);
     hline(display[d], PP.camera_x + 40, TORP_Y, PP.camera_x + 40 - PP.weapon_charge [i], HUD_COL_3);
    } else
     {
      if (PP.weapon_charge [i] < 0)
      {
       vline(display[d], PP.camera_x - 40, TORP_Y - 5, TORP_Y + 5, HUD_COL_1);
       vline(display[d], PP.camera_x + 40, TORP_Y - 5, TORP_Y + 5, HUD_COL_1);

       vline(display[d], PP.camera_x - 40 - (PP.weapon_charge [i]>>1), TORP_Y - 2, TORP_Y + 2, HUD_COL_1);
       hline(display[d], PP.camera_x - 40, TORP_Y, PP.camera_x - 40 - (PP.weapon_charge [i]>>1), HUD_COL_1);
       vline(display[d], PP.camera_x + 40 + (PP.weapon_charge [i]>>1), TORP_Y - 2, TORP_Y + 2, HUD_COL_1);
       hline(display[d], PP.camera_x + 40, TORP_Y, PP.camera_x + 40 + (PP.weapon_charge [i]>>1), HUD_COL_1);
      }
       else
       {
        vline(display[d], PP.camera_x - 40, TORP_Y - 5, TORP_Y + 5, HUD_COL_2);
        vline(display[d], PP.camera_x + 40, TORP_Y - 5, TORP_Y + 5, HUD_COL_2);
       }
     }
*/
    break;
#define ROCKET_Y 500
   case WPN_ROCKET:
   case WPN_RAIN:
    if (PP.weapon_charge [i] > 0)
    {
     vline(display[d], PP.camera_x - 10, ROCKET_Y - 2, ROCKET_Y + 2, COL_F7);
     vline(display[d], PP.camera_x + 10, ROCKET_Y - 2, ROCKET_Y + 2, COL_F7);

     vline(display[d], PP.camera_x - 10 + (PP.weapon_charge [i]>>1), ROCKET_Y - 1, ROCKET_Y + 1, COL_F7);
     hline(display[d], PP.camera_x - 10, ROCKET_Y, PP.camera_x - 10 + (PP.weapon_charge [i]>>1), COL_F7);
     vline(display[d], PP.camera_x + 10 - (PP.weapon_charge [i]>>1), ROCKET_Y - 1, ROCKET_Y + 1, COL_F7);
     hline(display[d], PP.camera_x + 10, ROCKET_Y, PP.camera_x + 10 - (PP.weapon_charge [i]>>1), COL_F7);
    }
     else
     {
      vline(display[d], PP.camera_x - 10, ROCKET_Y - 2, ROCKET_Y + 2, COL_F4);
      vline(display[d], PP.camera_x + 10, ROCKET_Y - 2, ROCKET_Y + 2, COL_F4);
     }
#define ROCKET_Y_DIST 10
    col = COL_F6;
    if (PP.weapon_firing [i])
     col = COL_F6 + TRANS_RED1;
      else
      {
       if (PP.weapon_status2 [i] == 6)
        col = COL_F6 + TRANS_BLUE1;
      }
    for (j = 0; j < PP.weapon_status2 [i]; j ++)
    {
      vline(display[d], PP.camera_x - 10, ROCKET_Y + (j+1)*ROCKET_Y_DIST - 2, ROCKET_Y + (j+1)*ROCKET_Y_DIST + 2, col);
      vline(display[d], PP.camera_x + 10, ROCKET_Y + (j+1)*ROCKET_Y_DIST - 2, ROCKET_Y + (j+1)*ROCKET_Y_DIST + 2, col);
      hline(display[d], PP.camera_x - 10, ROCKET_Y + (j+1)*ROCKET_Y_DIST, PP.camera_x + 10, col);
    }
    break;

#define AWS_SIGHT_DIST 320
// also used in input.c as AWS_TARGET_DIST or something
   case WPN_AWS_MISSILE:
//    circle(display[d], PP.camera_x, PP.camera_y - 180, 50, COL_F6);
    angle = PP.weapon_angle [i] + ANGLE_6;
/*     line(display[d], PP.camera_x + xpart(angle, 10), PP.camera_y - 180 + ypart(angle, 10),
                     PP.camera_x + xpart(angle, 96), PP.camera_y - 180 + ypart(angle, 96), COL_F5);*/
    col = HUD_COL_1;
    if (PP.weapon_target [i] [0] != TARGET_NONE)
     col = HUD_COL_3;
    for (j = 0; j < 5; j ++)
    {
     line(display[d], PP.camera_x + xpart(angle, 50), PP.camera_y - AWS_SIGHT_DIST + ypart(angle, 50),
                     PP.camera_x + xpart(angle, 56), PP.camera_y - AWS_SIGHT_DIST + ypart(angle, 56), col);
     angle += ANGLE_6;

    }
/*#define RET_SIZE 50
#define RET_LINE 6
    line(display[d], PP.camera_x - RET_SIZE, PP.camera_y - 180 - RET_SIZE,
                     PP.camera_x - RET_SIZE - RET_LINE, PP.camera_y - 180 - RET_SIZE - RET_LINE, COL_F4);
    line(display[d], PP.camera_x + RET_SIZE, PP.camera_y - 180 - RET_SIZE,
                     PP.camera_x + RET_SIZE + RET_LINE, PP.camera_y - 180 - RET_SIZE - RET_LINE, COL_F4);
    line(display[d], PP.camera_x - RET_SIZE, PP.camera_y - 180 + RET_SIZE,
                     PP.camera_x - RET_SIZE - RET_LINE, PP.camera_y - 180 + RET_SIZE + RET_LINE, COL_F4);
    line(display[d], PP.camera_x + RET_SIZE, PP.camera_y - 180 + RET_SIZE,
                     PP.camera_x + RET_SIZE + RET_LINE, PP.camera_y - 180 + RET_SIZE + RET_LINE, COL_F4);
                     */
#define LOCK_SIDE_LINES 11

    if (PP.weapon_sight_visible [i] [0])
    {
//        circle(display[d], PP.weapon_sight_x [i] [0], PP.weapon_sight_y [i] [0],
//          (eclass[ship[TEAM_ENEMY][PP.weapon_target [i] [0]].type].size >> 10) + 10 + ((2000 - PP.weapon_lock [i])>>5),
//          COL_F4);

        angle = (PP.weapon_angle [i] + ANGLE_6 + ANGLE_2);
        angle &= ANGLE_MASK;

        dist = (eclass[ship[TEAM_ENEMY][PP.weapon_target [i] [0]].type].size >> 10) + 20 + ((1000 - PP.weapon_lock [i])>>4);
        if (dist < (eclass[ship[TEAM_ENEMY][PP.weapon_target [i] [0]].type].size >> 10) + 20)
         dist = (eclass[ship[TEAM_ENEMY][PP.weapon_target [i] [0]].type].size >> 10) + 20;
        int line_length = ((PP.weapon_lock [i])>>7) + 3;
        if (line_length > 10)
         line_length = 10;

        col = HUD_COL_1;
        if (PP.weapon_lock [i] >= 1000)
         col = HUD_COL_3;


        for (j = 0; j < 3; j ++)
        {
         line(display[d], PP.weapon_sight_x [i] [0] + xpart(angle, dist), PP.weapon_sight_y [i] [0] + ypart(angle, dist),
                     PP.weapon_sight_x [i] [0] + xpart(angle, dist + line_length), PP.weapon_sight_y [i] [0] + ypart(angle, dist + line_length), col);
         angle += ANGLE_3;

        }

        if (PP.weapon_lock [i] < 1000)
         break;


        for (j = 0; j < 3; j ++)
        {
         line(display[d], PP.weapon_sight_x [i] [0] + xpart(angle, dist) + xpart(angle+ANGLE_4, LOCK_SIDE_LINES), PP.weapon_sight_y [i] [0] + ypart(angle, dist) + ypart(angle+ANGLE_4, LOCK_SIDE_LINES),
                     PP.weapon_sight_x [i] [0] + xpart(angle, dist) + xpart(angle-ANGLE_4, LOCK_SIDE_LINES), PP.weapon_sight_y [i] [0] + ypart(angle, dist) + ypart(angle-ANGLE_4, LOCK_SIDE_LINES), HUD_COL_3);
         angle += ANGLE_3;

        }


        dist = (eclass[ship[TEAM_ENEMY][PP.weapon_target [i] [0]].type].size >> 10) + 20 + ((2000 - PP.weapon_lock [i])>>4);
        line_length = ((PP.weapon_lock [i]-1000)>>7) + 3;

        angle += ANGLE_6;

        col = HUD_COL_1;
        if (PP.weapon_lock [i] >= 2000)
         col = HUD_COL_3;

        for (j = 0; j < 2; j ++)
        {
         line(display[d], PP.weapon_sight_x [i] [0] + xpart(angle, dist), PP.weapon_sight_y [i] [0] + ypart(angle, dist),
                     PP.weapon_sight_x [i] [0] + xpart(angle, dist + line_length), PP.weapon_sight_y [i] [0] + ypart(angle, dist + line_length), col);
         angle += ANGLE_3;

        }
        if (PP.weapon_lock [i] >= 2000)
        {
         angle -= ANGLE_3 + ANGLE_3;
         for (j = 0; j < 2; j ++)
         {
          line(display[d], PP.weapon_sight_x [i] [0] + xpart(angle, dist) + xpart(angle+ANGLE_4, LOCK_SIDE_LINES), PP.weapon_sight_y [i] [0] + ypart(angle, dist) + ypart(angle+ANGLE_4, LOCK_SIDE_LINES),
                      PP.weapon_sight_x [i] [0] + xpart(angle, dist) + xpart(angle-ANGLE_4, LOCK_SIDE_LINES), PP.weapon_sight_y [i] [0] + ypart(angle, dist) + ypart(angle-ANGLE_4, LOCK_SIDE_LINES), col);
          angle += ANGLE_3;

         }
        }



/*        if (line_length > 10)
         line_length = 10;*/


//          print_number(PP.weapon_sight_x [i] [0] + 20, PP.weapon_sight_y [i] [0] + 20, PP.weapon_lock [i]);
//          print_number(PP.weapon_sight_x [i] [0] + 20, PP.weapon_sight_y [i] [0] + 35, line_length);
//          print_number(100, 120, PP.weapon_charge [i]);
    }

    break;

#define AF_Y (PP.camera_y + 40)

   case WPN_AF_MISSILE:
   case WPN_LW_MISSILE:
   case WPN_ADV_LW_MISSILE:
   case WPN_HVY_LW_MISSILE:
    col = HUD_COL_1;
    if (PP.weapon_target [i] [0] != TARGET_NONE)
     col = HUD_COL_3;
    if (PP.weapon_charge [i] > 0)
    {
     vline(display[d], PP.camera_x - 90, AF_Y - 3, AF_Y + 3, HUD_COL_3);
     vline(display[d], PP.camera_x - 40, AF_Y - 3, AF_Y + 3, HUD_COL_3);

     vline(display[d], PP.camera_x - 90 + (PP.weapon_charge [i]>>2), AF_Y - 2, AF_Y + 2, HUD_COL_3);
     hline(display[d], PP.camera_x - 40, AF_Y, PP.camera_x - 90 + (PP.weapon_charge [i]>>2), HUD_COL_3);

    }
    x = PP.camera_x;
    y = PP.camera_y + 300;
    for (j = 0; j < 3; j ++)
    {
     dist = 440 + 120 * j;
     angle = - ANGLE_4 - ANGLE_16;
     line(display[d], x + xpart(angle, dist), y + ypart(angle, dist),
                     x + xpart(angle, dist + 5), y + ypart(angle, dist + 5), HUD_COL_3);
     angle = - ANGLE_4 + ANGLE_16;
     line(display[d], x + xpart(angle, dist), y + ypart(angle, dist),
                     x + xpart(angle, dist + 5), y + ypart(angle, dist + 5), HUD_COL_3);

    }
#define LOCK_SIDE_LINES 11

    if (PP.weapon_lock [i] < 2)
     break;

    if (PP.weapon_sight_visible [i] [0])
    {


        angle = (PP.weapon_angle [i] + ANGLE_6 + ANGLE_2);
        angle &= ANGLE_MASK;

        int lock_invert = 20 - PP.weapon_lock [i];
        if (lock_invert < 0)
         lock_invert = 0;

        dist = (eclass[ship[TEAM_ENEMY][PP.weapon_target [i] [0]].type].size >> 10) + 20 + (lock_invert << 2);
        if (dist < (eclass[ship[TEAM_ENEMY][PP.weapon_target [i] [0]].type].size >> 10) + 20)
         dist = (eclass[ship[TEAM_ENEMY][PP.weapon_target [i] [0]].type].size >> 10) + 20;
        int line_length = ((PP.weapon_lock [i])>>1) + 3;
        if (line_length > (10>>1) + 3)
         line_length = (10>>1) + 3;

        col = HUD_COL_1;
        if (PP.weapon_lock [i] >= 20)
         col = HUD_COL_3;


        for (j = 0; j < 3; j ++)
        {
         line(display[d],
          PP.weapon_sight_x [i] [0] + xpart(angle, dist) + xpart(angle+ANGLE_4, line_length),
          PP.weapon_sight_y [i] [0] + ypart(angle, dist) + ypart(angle+ANGLE_4, line_length),
          PP.weapon_sight_x [i] [0] + xpart(angle, dist) + xpart(angle-ANGLE_4, line_length),
          PP.weapon_sight_y [i] [0] + ypart(angle, dist) + ypart(angle-ANGLE_4, line_length), col);
         angle += ANGLE_3;

        }

        if (PP.weapon_lock [i] < 20)
         break;

        lock_invert = 40 - PP.weapon_lock [i];
        if (lock_invert < 0)
         lock_invert = 0;

        dist = (eclass[ship[TEAM_ENEMY][PP.weapon_target [i] [0]].type].size >> 10) + 20 + (lock_invert << 2);
        if (dist < (eclass[ship[TEAM_ENEMY][PP.weapon_target [i] [0]].type].size >> 10) + 20)
         dist = (eclass[ship[TEAM_ENEMY][PP.weapon_target [i] [0]].type].size >> 10) + 20;

        angle += ANGLE_6;

        line_length = ((PP.weapon_lock [i] - 19)>>2);
//        if (line_length > 10)
//         line_length = 10;

        col = HUD_COL_1;
        if (PP.weapon_lock [i] >= 40)
         col = HUD_COL_3;

        for (j = 0; j < 3; j ++)
        {
         line(display[d],
          PP.weapon_sight_x [i] [0] + xpart(angle, dist) + xpart(angle+ANGLE_8, line_length),
          PP.weapon_sight_y [i] [0] + ypart(angle, dist) + ypart(angle+ANGLE_8, line_length),
          PP.weapon_sight_x [i] [0] + xpart(angle, dist),
          PP.weapon_sight_y [i] [0] + ypart(angle, dist), col);
         line(display[d],
          PP.weapon_sight_x [i] [0] + xpart(angle, dist) + xpart(angle-ANGLE_8, line_length),
          PP.weapon_sight_y [i] [0] + ypart(angle, dist) + ypart(angle-ANGLE_8, line_length),
          PP.weapon_sight_x [i] [0] + xpart(angle, dist),
          PP.weapon_sight_y [i] [0] + ypart(angle, dist), col);
         angle += ANGLE_3;

        }

    }

    break;

   case WPN_HOMING:
    circle(display[d], PP.camera_x, PP.camera_y - 250, 50, COL_F6);
    if (PP.weapon_sight_visible [i] [0])
    {
        circle(display[d], PP.weapon_sight_x [i] [0], PP.weapon_sight_y [i] [0],
          (eclass[ship[TEAM_ENEMY][PP.weapon_target [i] [0]].type].size >> 10) + 10 + ((2000 - PP.weapon_charge [i])>>5),
          COL_F4);
    }

    break;

   case WPN_DEFLECT:
    col = HUD_COL_2;
//    if (PP.weapon_target [i] [0] != TARGET_NONE)
//     col = COL_F6;
    x = PP.camera_x;
    y = PP.camera_y;
    for (j = 0; j < 2; j ++)
    {
     dist = 90 + 120 * j;
     angle = - ANGLE_4 - ANGLE_8;
     line(display[d], x + xpart(angle, dist), y + ypart(angle, dist),
                     x + xpart(angle, dist + 5), y + ypart(angle, dist + 5), col);
     angle = - ANGLE_4 + ANGLE_8;
     line(display[d], x + xpart(angle, dist), y + ypart(angle, dist),
                     x + xpart(angle, dist + 5), y + ypart(angle, dist + 5), col);

    }

    if (PP.weapon_sight_visible [i] [0])
    {

//     circle(display[d], PP.weapon_sight_x [i] [0], PP.weapon_sight_y [i] [0], 20, COL_F6);
      dist = eclass[ship[TEAM_ENEMY][PP.weapon_target [i] [0]].type].size >> 11; // note >> 11 to get extra /2

      dist += PP.weapon_lock [i];

      vline(display[d], PP.weapon_sight_x [i] [0] - dist - 25, PP.weapon_sight_y [i] [0] - 8,
            PP.weapon_sight_y [i] [0] + 8, HUD_COL_3);
      hline(display[d], PP.weapon_sight_x [i] [0] - dist - 25, PP.weapon_sight_y [i] [0],
            PP.weapon_sight_x [i] [0] - dist - 35, HUD_COL_3);
      vline(display[d], PP.weapon_sight_x [i] [0] + dist + 25, PP.weapon_sight_y [i] [0] - 8,
            PP.weapon_sight_y [i] [0] + 8, HUD_COL_3);
      hline(display[d], PP.weapon_sight_x [i] [0] + dist + 25, PP.weapon_sight_y [i] [0],
            PP.weapon_sight_x [i] [0] + dist + 35, HUD_COL_3);

    }

    break;

 case WPN_ROCKET2:
    if (PP.weapon_charge [i] > 0)
    {
     vline(display[d], PP.camera_x + 40, AF_Y - 3, AF_Y + 3, HUD_COL_3);
     vline(display[d], PP.camera_x + 65, AF_Y - 3, AF_Y + 3, HUD_COL_3);

     vline(display[d], PP.camera_x + 65 - (PP.weapon_charge [i]>>1), AF_Y - 2, AF_Y + 2, HUD_COL_3);
     hline(display[d], PP.camera_x + 40, AF_Y, PP.camera_x + 65 - (PP.weapon_charge [i]>>1), HUD_COL_3);
    }
  break;

  }

 }
}
 else // fixed camera
 {

 for (i = 0; i < 2; i ++)
 {
  int rotate_angle = PP.angle;
  switch(PP.weapon_type [i])
  {
   case WPN_TORP:
    if (PP.weapon_charge [i] > 0)
    {
     vline(display[d], PP.camera_x - 40, TORP_Y - 5, TORP_Y + 5, HUD_COL_3);
     vline(display[d], PP.camera_x + 40, TORP_Y - 5, TORP_Y + 5, HUD_COL_3);

     vline(display[d], PP.camera_x - 40 + PP.weapon_charge [i], TORP_Y - 4, TORP_Y + 4, HUD_COL_3);
     hline(display[d], PP.camera_x - 40, TORP_Y, PP.camera_x - 40 + PP.weapon_charge [i], HUD_COL_3);
     vline(display[d], PP.camera_x + 40 - PP.weapon_charge [i], TORP_Y - 4, TORP_Y + 4, HUD_COL_3);
     hline(display[d], PP.camera_x + 40, TORP_Y, PP.camera_x + 40 - PP.weapon_charge [i], HUD_COL_3);
    } else
     {
      if (PP.weapon_charge [i] < 0)
      {
       vline(display[d], PP.camera_x - 40, TORP_Y - 5, TORP_Y + 5, HUD_COL_1);
       vline(display[d], PP.camera_x + 40, TORP_Y - 5, TORP_Y + 5, HUD_COL_1);

       vline(display[d], PP.camera_x - 40 - (PP.weapon_charge [i]>>1), TORP_Y - 2, TORP_Y + 2, HUD_COL_1);
       hline(display[d], PP.camera_x - 40, TORP_Y, PP.camera_x - 40 - (PP.weapon_charge [i]>>1), HUD_COL_1);
       vline(display[d], PP.camera_x + 40 + (PP.weapon_charge [i]>>1), TORP_Y - 2, TORP_Y + 2, HUD_COL_1);
       hline(display[d], PP.camera_x + 40, TORP_Y, PP.camera_x + 40 + (PP.weapon_charge [i]>>1), HUD_COL_1);
      }
       else
       {
        vline(display[d], PP.camera_x - 40, TORP_Y - 5, TORP_Y + 5, HUD_COL_2);
        vline(display[d], PP.camera_x + 40, TORP_Y - 5, TORP_Y + 5, HUD_COL_2);
       }
     }

    break;
#define ROCKET_Y 500
   case WPN_ROCKET:
   case WPN_RAIN:
    if (PP.weapon_charge [i] > 0)
    {
     vline(display[d], PP.camera_x - 10, ROCKET_Y - 3, ROCKET_Y + 3, COL_F7);
     vline(display[d], PP.camera_x + 10, ROCKET_Y - 3, ROCKET_Y + 3, COL_F7);

     vline(display[d], PP.camera_x - 10 + (PP.weapon_charge [i]>>1), ROCKET_Y - 3, ROCKET_Y + 3, COL_F7);
     hline(display[d], PP.camera_x - 10, ROCKET_Y, PP.camera_x - 10 + (PP.weapon_charge [i]>>1), COL_F7);
     vline(display[d], PP.camera_x + 10 - (PP.weapon_charge [i]>>1), ROCKET_Y - 3, ROCKET_Y + 3, COL_F7);
     hline(display[d], PP.camera_x + 10, ROCKET_Y, PP.camera_x + 10 - (PP.weapon_charge [i]>>1), COL_F7);
    }
     else
     {
      vline(display[d], PP.camera_x - 10, ROCKET_Y - 3, ROCKET_Y + 3, COL_F4);
      vline(display[d], PP.camera_x + 10, ROCKET_Y - 3, ROCKET_Y + 3, COL_F4);
     }
#define ROCKET_Y_DIST 10
    col = COL_F6;
    if (PP.weapon_firing [i])
     col = COL_F6 + TRANS_RED1;
      else
      {
       if (PP.weapon_status2 [i] == 6)
        col = COL_F6 + TRANS_BLUE1;
      }
    for (j = 0; j < PP.weapon_status2 [i]; j ++)
    {
      vline(display[d], PP.camera_x - 10, ROCKET_Y + (j+1)*ROCKET_Y_DIST - 3, ROCKET_Y + (j+1)*ROCKET_Y_DIST + 3, col);
      vline(display[d], PP.camera_x + 10, ROCKET_Y + (j+1)*ROCKET_Y_DIST - 3, ROCKET_Y + (j+1)*ROCKET_Y_DIST + 3, col);
      hline(display[d], PP.camera_x - 10, ROCKET_Y + (j+1)*ROCKET_Y_DIST, PP.camera_x + 10, col);
    }
    break;

//#define AWS_SIGHT_DIST 320
// also used in input.c as AWS_TARGET_DIST or something
   case WPN_AWS_MISSILE:
//    circle(display[d], PP.camera_x, PP.camera_y - 180, 50, COL_F6);
    angle = PP.weapon_angle [i] + ANGLE_6 + rotate_angle + ANGLE_4;
/*     line(display[d], PP.camera_x + xpart(angle, 10), PP.camera_y - 180 + ypart(angle, 10),
                     PP.camera_x + xpart(angle, 96), PP.camera_y - 180 + ypart(angle, 96), COL_F5);*/
    col = HUD_COL_1;
    if (PP.weapon_target [i] [0] != TARGET_NONE)
     col = HUD_COL_3;
    for (j = 0; j < 5; j ++)
    {
     line(display[d], PP.camera_x + xpart(angle, 50) + xpart(rotate_angle, AWS_SIGHT_DIST), PP.camera_y + ypart(angle, 50) + ypart(rotate_angle, AWS_SIGHT_DIST),
                     PP.camera_x + xpart(angle, 56) + xpart(rotate_angle, AWS_SIGHT_DIST), PP.camera_y + ypart(angle, 56) + ypart(rotate_angle, AWS_SIGHT_DIST), col);
     angle += ANGLE_6;

    }
/*#define RET_SIZE 50
#define RET_LINE 6
    line(display[d], PP.camera_x - RET_SIZE, PP.camera_y - 180 - RET_SIZE,
                     PP.camera_x - RET_SIZE - RET_LINE, PP.camera_y - 180 - RET_SIZE - RET_LINE, COL_F4);
    line(display[d], PP.camera_x + RET_SIZE, PP.camera_y - 180 - RET_SIZE,
                     PP.camera_x + RET_SIZE + RET_LINE, PP.camera_y - 180 - RET_SIZE - RET_LINE, COL_F4);
    line(display[d], PP.camera_x - RET_SIZE, PP.camera_y - 180 + RET_SIZE,
                     PP.camera_x - RET_SIZE - RET_LINE, PP.camera_y - 180 + RET_SIZE + RET_LINE, COL_F4);
    line(display[d], PP.camera_x + RET_SIZE, PP.camera_y - 180 + RET_SIZE,
                     PP.camera_x + RET_SIZE + RET_LINE, PP.camera_y - 180 + RET_SIZE + RET_LINE, COL_F4);
                     */
#define LOCK_SIDE_LINES 11

    if (PP.weapon_sight_visible [i] [0])
    {
//        circle(display[d], PP.weapon_sight_x [i] [0], PP.weapon_sight_y [i] [0],
//          (eclass[ship[TEAM_ENEMY][PP.weapon_target [i] [0]].type].size >> 10) + 10 + ((2000 - PP.weapon_lock [i])>>5),
//          COL_F4);

        angle = (PP.weapon_angle [i] + ANGLE_6 + ANGLE_2);
        angle &= ANGLE_MASK;

        dist = (eclass[ship[TEAM_ENEMY][PP.weapon_target [i] [0]].type].size >> 10) + 20 + ((1000 - PP.weapon_lock [i])>>4);
        if (dist < (eclass[ship[TEAM_ENEMY][PP.weapon_target [i] [0]].type].size >> 10) + 20)
         dist = (eclass[ship[TEAM_ENEMY][PP.weapon_target [i] [0]].type].size >> 10) + 20;
        int line_length = ((PP.weapon_lock [i])>>7) + 3;
        if (line_length > 10)
         line_length = 10;

        col = HUD_COL_1;
        if (PP.weapon_lock [i] >= 1000)
         col = HUD_COL_3;


        for (j = 0; j < 3; j ++)
        {
         line(display[d], PP.weapon_sight_x [i] [0] + xpart(angle, dist), PP.weapon_sight_y [i] [0] + ypart(angle, dist),
                     PP.weapon_sight_x [i] [0] + xpart(angle, dist + line_length), PP.weapon_sight_y [i] [0] + ypart(angle, dist + line_length), col);
         angle += ANGLE_3;

        }

        if (PP.weapon_lock [i] < 1000)
         break;


        for (j = 0; j < 3; j ++)
        {
         line(display[d], PP.weapon_sight_x [i] [0] + xpart(angle, dist) + xpart(angle+ANGLE_4, LOCK_SIDE_LINES), PP.weapon_sight_y [i] [0] + ypart(angle, dist) + ypart(angle+ANGLE_4, LOCK_SIDE_LINES),
                     PP.weapon_sight_x [i] [0] + xpart(angle, dist) + xpart(angle-ANGLE_4, LOCK_SIDE_LINES), PP.weapon_sight_y [i] [0] + ypart(angle, dist) + ypart(angle-ANGLE_4, LOCK_SIDE_LINES), HUD_COL_3);
         angle += ANGLE_3;

        }


        dist = (eclass[ship[TEAM_ENEMY][PP.weapon_target [i] [0]].type].size >> 10) + 20 + ((2000 - PP.weapon_lock [i])>>4);
        line_length = ((PP.weapon_lock [i]-1000)>>7) + 3;

        angle += ANGLE_6;

        col = HUD_COL_1;
        if (PP.weapon_lock [i] >= 2000)
         col = HUD_COL_3;

        for (j = 0; j < 2; j ++)
        {
         line(display[d], PP.weapon_sight_x [i] [0] + xpart(angle, dist), PP.weapon_sight_y [i] [0] + ypart(angle, dist),
                     PP.weapon_sight_x [i] [0] + xpart(angle, dist + line_length), PP.weapon_sight_y [i] [0] + ypart(angle, dist + line_length), col);
         angle += ANGLE_3;

        }
        if (PP.weapon_lock [i] >= 2000)
        {
         angle -= ANGLE_3 + ANGLE_3;
         for (j = 0; j < 2; j ++)
         {
          line(display[d], PP.weapon_sight_x [i] [0] + xpart(angle, dist) + xpart(angle+ANGLE_4, LOCK_SIDE_LINES), PP.weapon_sight_y [i] [0] + ypart(angle, dist) + ypart(angle+ANGLE_4, LOCK_SIDE_LINES),
                      PP.weapon_sight_x [i] [0] + xpart(angle, dist) + xpart(angle-ANGLE_4, LOCK_SIDE_LINES), PP.weapon_sight_y [i] [0] + ypart(angle, dist) + ypart(angle-ANGLE_4, LOCK_SIDE_LINES), col);
          angle += ANGLE_3;

         }
        }



/*        if (line_length > 10)
         line_length = 10;*/


//          print_number(PP.weapon_sight_x [i] [0] + 20, PP.weapon_sight_y [i] [0] + 20, PP.weapon_lock [i]);
//          print_number(PP.weapon_sight_x [i] [0] + 20, PP.weapon_sight_y [i] [0] + 35, line_length);
//          print_number(100, 120, PP.weapon_charge [i]);
    }

    break;

#define AF_Y (PP.camera_y + 40)

   case WPN_AF_MISSILE:
   case WPN_LW_MISSILE:
   case WPN_ADV_LW_MISSILE:
   case WPN_HVY_LW_MISSILE:
    col = HUD_COL_1;
    if (PP.weapon_target [i] [0] != TARGET_NONE)
     col = HUD_COL_3;
    if (PP.weapon_charge [i] > 0)
    {
     vline(display[d], PP.camera_x - 90, AF_Y - 3, AF_Y + 3, HUD_COL_3);
     vline(display[d], PP.camera_x - 40, AF_Y - 3, AF_Y + 3, HUD_COL_3);

     vline(display[d], PP.camera_x - 90 + (PP.weapon_charge [i]>>2), AF_Y - 2, AF_Y + 2, HUD_COL_3);
     hline(display[d], PP.camera_x - 40, AF_Y, PP.camera_x - 90 + (PP.weapon_charge [i]>>2), HUD_COL_3);

    }
    x = PP.camera_x - xpart(rotate_angle, 300);
    y = PP.camera_y - ypart(rotate_angle, 300);
    for (j = 0; j < 3; j ++)
    {
     dist = 440 + 120 * j;
     angle = - ANGLE_16 + rotate_angle;
     line(display[d], x + xpart(angle, dist), y + ypart(angle, dist),
                     x + xpart(angle, dist + 5), y + ypart(angle, dist + 5), HUD_COL_3);
     angle = ANGLE_16 + rotate_angle;
     line(display[d], x + xpart(angle, dist), y + ypart(angle, dist),
                     x + xpart(angle, dist + 5), y + ypart(angle, dist + 5), HUD_COL_3);

    }
#define LOCK_SIDE_LINES 11

    if (PP.weapon_lock [i] < 2)
     break;

    if (PP.weapon_sight_visible [i] [0])
    {


        angle = (PP.weapon_angle [i] + ANGLE_6 + ANGLE_2);
        angle &= ANGLE_MASK;

        int lock_invert = 20 - PP.weapon_lock [i];
        if (lock_invert < 0)
         lock_invert = 0;

        dist = (eclass[ship[TEAM_ENEMY][PP.weapon_target [i] [0]].type].size >> 10) + 20 + (lock_invert << 2);
        if (dist < (eclass[ship[TEAM_ENEMY][PP.weapon_target [i] [0]].type].size >> 10) + 20)
         dist = (eclass[ship[TEAM_ENEMY][PP.weapon_target [i] [0]].type].size >> 10) + 20;
        int line_length = ((PP.weapon_lock [i])>>1) + 3;
        if (line_length > (10>>1) + 3)
         line_length = (10>>1) + 3;

        col = HUD_COL_1;
        if (PP.weapon_lock [i] >= 20)
         col = HUD_COL_3;


        for (j = 0; j < 3; j ++)
        {
         line(display[d],
          PP.weapon_sight_x [i] [0] + xpart(angle, dist) + xpart(angle+ANGLE_4, line_length),
          PP.weapon_sight_y [i] [0] + ypart(angle, dist) + ypart(angle+ANGLE_4, line_length),
          PP.weapon_sight_x [i] [0] + xpart(angle, dist) + xpart(angle-ANGLE_4, line_length),
          PP.weapon_sight_y [i] [0] + ypart(angle, dist) + ypart(angle-ANGLE_4, line_length), col);
         angle += ANGLE_3;

        }

        if (PP.weapon_lock [i] < 20)
         break;

        lock_invert = 40 - PP.weapon_lock [i];
        if (lock_invert < 0)
         lock_invert = 0;

        dist = (eclass[ship[TEAM_ENEMY][PP.weapon_target [i] [0]].type].size >> 10) + 20 + (lock_invert << 2);
        if (dist < (eclass[ship[TEAM_ENEMY][PP.weapon_target [i] [0]].type].size >> 10) + 20)
         dist = (eclass[ship[TEAM_ENEMY][PP.weapon_target [i] [0]].type].size >> 10) + 20;

        angle += ANGLE_6;

        line_length = ((PP.weapon_lock [i] - 19)>>2);
//        if (line_length > 10)
//         line_length = 10;

        col = HUD_COL_1;
        if (PP.weapon_lock [i] >= 40)
         col = HUD_COL_3;

        for (j = 0; j < 3; j ++)
        {
         line(display[d],
          PP.weapon_sight_x [i] [0] + xpart(angle, dist) + xpart(angle+ANGLE_8, line_length),
          PP.weapon_sight_y [i] [0] + ypart(angle, dist) + ypart(angle+ANGLE_8, line_length),
          PP.weapon_sight_x [i] [0] + xpart(angle, dist),
          PP.weapon_sight_y [i] [0] + ypart(angle, dist), col);
         line(display[d],
          PP.weapon_sight_x [i] [0] + xpart(angle, dist) + xpart(angle-ANGLE_8, line_length),
          PP.weapon_sight_y [i] [0] + ypart(angle, dist) + ypart(angle-ANGLE_8, line_length),
          PP.weapon_sight_x [i] [0] + xpart(angle, dist),
          PP.weapon_sight_y [i] [0] + ypart(angle, dist), col);
         angle += ANGLE_3;

        }

    }

    break;

   case WPN_HOMING:
    circle(display[d], PP.camera_x, PP.camera_y - 250, 50, COL_F6);
    if (PP.weapon_sight_visible [i] [0])
    {
        circle(display[d], PP.weapon_sight_x [i] [0], PP.weapon_sight_y [i] [0],
          (eclass[ship[TEAM_ENEMY][PP.weapon_target [i] [0]].type].size >> 10) + 10 + ((2000 - PP.weapon_charge [i])>>5),
          COL_F4);
    }

    break;

   case WPN_DEFLECT:
    col = HUD_COL_2;
//    if (PP.weapon_target [i] [0] != TARGET_NONE)
//     col = COL_F6;
    x = PP.camera_x;
    y = PP.camera_y;
    for (j = 0; j < 2; j ++)
    {
     dist = 90 + 120 * j;
     angle = rotate_angle - ANGLE_8;
     line(display[d], x + xpart(angle, dist), y + ypart(angle, dist),
                     x + xpart(angle, dist + 5), y + ypart(angle, dist + 5), col);
     angle = rotate_angle + ANGLE_8;
     line(display[d], x + xpart(angle, dist), y + ypart(angle, dist),
                     x + xpart(angle, dist + 5), y + ypart(angle, dist + 5), col);

    }

    if (PP.weapon_sight_visible [i] [0])
    {

//     circle(display[d], PP.weapon_sight_x [i] [0], PP.weapon_sight_y [i] [0], 20, COL_F6);
      dist = eclass[ship[TEAM_ENEMY][PP.weapon_target [i] [0]].type].size >> 11; // note >> 11 to get extra /2

      dist += PP.weapon_lock [i];

      vline(display[d], PP.weapon_sight_x [i] [0] - dist - 25, PP.weapon_sight_y [i] [0] - 8,
            PP.weapon_sight_y [i] [0] + 8, HUD_COL_3);
      hline(display[d], PP.weapon_sight_x [i] [0] - dist - 25, PP.weapon_sight_y [i] [0],
            PP.weapon_sight_x [i] [0] - dist - 35, HUD_COL_3);
      vline(display[d], PP.weapon_sight_x [i] [0] + dist + 25, PP.weapon_sight_y [i] [0] - 8,
            PP.weapon_sight_y [i] [0] + 8, HUD_COL_3);
      hline(display[d], PP.weapon_sight_x [i] [0] + dist + 25, PP.weapon_sight_y [i] [0],
            PP.weapon_sight_x [i] [0] + dist + 35, HUD_COL_3);

    }

    break;

 case WPN_ROCKET2:
    if (PP.weapon_charge [i] > 0)
    {
     vline(display[d], PP.camera_x + 40, AF_Y - 3, AF_Y + 3, HUD_COL_3);
     vline(display[d], PP.camera_x + 65, AF_Y - 3, AF_Y + 3, HUD_COL_3);

     vline(display[d], PP.camera_x + 65 - (PP.weapon_charge [i]>>1), AF_Y - 2, AF_Y + 2, HUD_COL_3);
     hline(display[d], PP.camera_x + 40, AF_Y, PP.camera_x + 65 - (PP.weapon_charge [i]>>1), HUD_COL_3);
    }
  break;

  }

 }



 }



 circlefill(display[d], SCANNER_X, SCANNER_Y, SCANNER_SIZE + 5, COL_F4 + TRANS_BLUE2);
 circle(display[d], SCANNER_X, SCANNER_Y, SCANNER_SIZE + 5, COL_F1 + TRANS_BLUE2);
 circlefill(display[d], SCANNER_X, SCANNER_Y, SCANNER_SIZE + 2, COL_SCANNER_BACKGROUND);
 circle(display[d], SCANNER_X, SCANNER_Y, SCANNER_SIZE + 2, COL_F1 + TRANS_BLUE2);
 circle(display[d], SCANNER_X, SCANNER_Y, SCANNER_SIZE + 0, COL_F3);

 angle = ANGLE_1 - ANGLE_2  - camera_angle;

 line(display[d], SCANNER_X + xpart(angle, 40), SCANNER_Y + ypart(angle, 40),
                  SCANNER_X + xpart(angle, SCANNER_SIZE), SCANNER_Y + ypart(angle, SCANNER_SIZE),
                  COL_F4);

/*
 circlefill(display[d], SCANNER_X, SCANNER_Y, SCANNER_SIZE + 2, COL_STAR1);
 circle(display[d], SCANNER_X, SCANNER_Y, SCANNER_SIZE + 2, COL_F4 + TRANS_BLUE2);
 circle(display[d], SCANNER_X, SCANNER_Y, SCANNER_SIZE + 0, COL_F3);
*/
/*
 for (b = 0; b < NO_PBULLETS; b ++)
 {
  if (.type == PBULLET_NONE
   || .type == PBULLET_SHOT
   || .type == PBULLET_MINE_BLAST)
    continue;

 angle = radians_to_angle(atan2(PP.y - .y, PP.x - .x)) - PP.angle + ANGLE_4;
 dist = (int) hypot(PP.y - .y, PP.x - .x) >> 16;

 if (dist >= SCANNER_SIZE)
 {
  putpixel(display[d], SCANNER_X + xpart(angle, SCANNER_SIZE - 1), SCANNER_Y + ypart(angle, SCANNER_SIZE - 1), COL_4 + TRANS_BLUE1);
 }
  else
  {
   putpixel(display[d], SCANNER_X + xpart(angle, dist), SCANNER_Y + ypart(angle, dist), COL_4 + TRANS_BLUE3);
  }


 }

*/


 int a, e, size;
 int select_x = 0, select_y = 0;

 putpixel(display[d], SCANNER_X, SCANNER_Y, COL_F7);
 putpixel(display[d], SCANNER_X - 1, SCANNER_Y, COL_F4);
 putpixel(display[d], SCANNER_X + 1, SCANNER_Y, COL_F4);
 putpixel(display[d], SCANNER_X, SCANNER_Y - 1, COL_F4);
 putpixel(display[d], SCANNER_X, SCANNER_Y + 1, COL_F4);

 for (a = 0; a < NO_TEAMS; a ++)
 {
 for (e = 0; e < NO_SHIPS; e ++)
 {
  if (EE.type == SHIP_NONE)
    continue;

 if (EE.stored_angle == -1)
 {
  angle = radians_to_angle(atan2(PP.y - EE.y, PP.x - EE.x)) - camera_angle + ANGLE_4;
  dist = (int) hypot(PP.y - EE.y, PP.x - EE.x) >> 15;
 }
  else
  {
   angle = EE.stored_angle - camera_angle - ANGLE_4;
   EE.stored_angle = -1;
   dist = EE.stored_dist >> 15;
  }

 size = 1;
 if (eclass[EE.type].blip_strength == 5)
  size = 2;
 if (eclass[EE.type].blip_strength == 6)
  size = 3;
TRANS_MODE
 if (dist >= SCANNER_SIZE)
 {
//  circlefill(display[d], SCANNER_X + xpart(angle, SCANNER_SIZE - 1), SCANNER_Y + ypart(angle, SCANNER_SIZE - 1), 1, acol [a] [eclass[EE.type].blip_strength] [0]);
//  putpixel(display[d], SCANNER_X + xpart(angle, SCANNER_SIZE - 1), SCANNER_Y + ypart(angle, SCANNER_SIZE - 1), acol [a] [eclass[EE.type].blip_strength] [1]);
  dist = SCANNER_SIZE - 1;
/*  if (PP.target_a == a && PP.target_e == e)
  {
   select_x = SCANNER_X + xpart(angle, SCANNER_SIZE - 1);
   select_y = SCANNER_Y + ypart(angle, SCANNER_SIZE - 1);
  }*/
 }

  {
   switch(eclass[EE.type].blip_strength)
   {
    default:
    circlefill(display[d], SCANNER_X + xpart(angle, dist), SCANNER_Y + ypart(angle, dist), 1, acol [EE.scancol] [eclass[EE.type].blip_strength] [0]);
    putpixel(display[d], SCANNER_X + xpart(angle, dist), SCANNER_Y + ypart(angle, dist), acol [EE.scancol] [eclass[EE.type].blip_strength] [1]);
    break;
   case 5:
    circlefill(display[d], SCANNER_X + xpart(angle, dist), SCANNER_Y + ypart(angle, dist), 2, acol [EE.scancol] [eclass[EE.type].blip_strength] [0]);
    putpixel(display[d], SCANNER_X + xpart(angle, dist), SCANNER_Y + ypart(angle, dist), acol [EE.scancol] [eclass[EE.type].blip_strength] [1]);
    break;
   case 6:
    circlefill(display[d], SCANNER_X + xpart(angle, dist), SCANNER_Y + ypart(angle, dist), 2, acol [EE.scancol] [eclass[EE.type].blip_strength] [0]);
    circlefill(display[d], SCANNER_X + xpart(angle, dist), SCANNER_Y + ypart(angle, dist), 1, acol [EE.scancol] [eclass[EE.type].blip_strength] [1]);
    break;
  }
   if (PP.target_a == a && PP.target_e == e)
   {
    select_x = SCANNER_X + xpart(angle, dist);
    select_y = SCANNER_Y + ypart(angle, dist);
   }
  }

/*
 if (dist >= SCANNER_SIZE)
 {
  circlefill(display[d], SCANNER_X + xpart(angle, SCANNER_SIZE - 1), SCANNER_Y + ypart(angle, SCANNER_SIZE - 1), 1, acol [a] [eclass[EE.type].blip_strength] [0]);
  putpixel(display[d], SCANNER_X + xpart(angle, SCANNER_SIZE - 1), SCANNER_Y + ypart(angle, SCANNER_SIZE - 1), acol [a] [eclass[EE.type].blip_strength] [1]);
  if (PP.target_a == a && PP.target_e == e)
  {
   select_x = SCANNER_X + xpart(angle, SCANNER_SIZE - 1);
   select_y = SCANNER_Y + ypart(angle, SCANNER_SIZE - 1);
  }
 }
  else
  {
   switch(eclass[EE.type].blip_strength)
   {
    default:
    circlefill(display[d], SCANNER_X + xpart(angle, dist), SCANNER_Y + ypart(angle, dist), 1, acol [a] [eclass[EE.type].blip_strength] [0]);
    putpixel(display[d], SCANNER_X + xpart(angle, dist), SCANNER_Y + ypart(angle, dist), acol [a] [eclass[EE.type].blip_strength] [1]);
    break;
   case 5:
    circlefill(display[d], SCANNER_X + xpart(angle, dist), SCANNER_Y + ypart(angle, dist), 2, acol [a] [eclass[EE.type].blip_strength] [0]);
    putpixel(display[d], SCANNER_X + xpart(angle, dist), SCANNER_Y + ypart(angle, dist), acol [a] [eclass[EE.type].blip_strength] [1]);
    break;
   case 6:
    circlefill(display[d], SCANNER_X + xpart(angle, dist), SCANNER_Y + ypart(angle, dist), 2, acol [a] [eclass[EE.type].blip_strength] [0]);
    circlefill(display[d], SCANNER_X + xpart(angle, dist), SCANNER_Y + ypart(angle, dist), 1, acol [a] [eclass[EE.type].blip_strength] [1]);
    break;
  }
   if (PP.target_a == a && PP.target_e == e)
   {
    select_x = SCANNER_X + xpart(angle, dist);
    select_y = SCANNER_Y + ypart(angle, dist);
   }
  }
*/
END_TRANS


 }
 }

 if (select_x != 0)
 {
  vline(display[d], select_x - 3, select_y - 2, select_y + 2, COL_F7 + TRANS_BLUE4);
  vline(display[d], select_x + 3, select_y - 2, select_y + 2, COL_F7 + TRANS_BLUE4);
  putpixel(display[d], select_x - 2, select_y - 2, COL_F7 + TRANS_BLUE4);
  putpixel(display[d], select_x + 2, select_y - 2, COL_F7 + TRANS_BLUE4);
  putpixel(display[d], select_x - 2, select_y + 2, COL_F7 + TRANS_BLUE4);
  putpixel(display[d], select_x + 2, select_y + 2, COL_F7 + TRANS_BLUE4);
 }


 int op = 0;

 if (arena.only_player == -1)
 {
  if (p == 0)
   op = 1;
  angle = radians_to_angle(atan2(PP.y - player[op].y, PP.x - player[op].x)) - camera_angle + ANGLE_4;
  dist = (int) hypot(PP.y - player[op].y, PP.x - player[op].x) >> 15;

  if (dist >= SCANNER_SIZE)
  {
   circlefill(display[d], SCANNER_X + xpart(angle, SCANNER_SIZE - 1), SCANNER_Y + ypart(angle, SCANNER_SIZE - 1), 1, COL_F1 + TRANS_BLUE1);
   putpixel(display[d], SCANNER_X + xpart(angle, SCANNER_SIZE - 1), SCANNER_Y + ypart(angle, SCANNER_SIZE - 1), COL_F2 + TRANS_BLUE2);
  }
   else
   {
    circlefill(display[d], SCANNER_X + xpart(angle, dist), SCANNER_Y + ypart(angle, dist), 1, COL_F2 + TRANS_BLUE3);
    putpixel(display[d], SCANNER_X + xpart(angle, dist), SCANNER_Y + ypart(angle, dist), COL_F2 + TRANS_BLUE3);
   }

 }


 putpixel(display[d], SCANNER_X - 1, SCANNER_Y, COL_F6);
 putpixel(display[d], SCANNER_X + 1, SCANNER_Y, COL_F6);
 putpixel(display[d], SCANNER_X, SCANNER_Y - 1, COL_F6);
 putpixel(display[d], SCANNER_X, SCANNER_Y + 1, COL_F6);


// extern struct BMP_STRUCT ship_collision_mask [NO_SHIP_TYPES]; // defined in display_init.c
 //draw_sprite(display[d], ship_collision_mask [SHIP_OLD2].sprite, 20, 220);

 int bitshift = 4;

 if (arena.only_player == -1)
  bitshift = 5;

#define COL_BAR_END COL_F7

 i = PP.max_hp >> bitshift;
 //rect(display[d], HP_BAR_X - 2, HP_BAR_Y - 2, HP_BAR_X + i + 2, HP_BAR_Y + HP_BAR_THICK + 2, 13);
 vline(display[d], HP_BAR_X - 1, HP_BAR_Y - HP_BAR_END_THICK, HP_BAR_Y + HP_BAR_END_THICK, COL_BAR_END);
 vline(display[d], HP_BAR_X - 2, HP_BAR_Y - HP_BAR_END_THICK, HP_BAR_Y + HP_BAR_END_THICK, COL_BAR_END);
 vline(display[d], HP_BAR_X + i + 1, HP_BAR_Y - HP_BAR_END_THICK, HP_BAR_Y + HP_BAR_END_THICK, COL_BAR_END);
 vline(display[d], HP_BAR_X + i + 2, HP_BAR_Y - HP_BAR_END_THICK, HP_BAR_Y + HP_BAR_END_THICK, COL_BAR_END);
 hline(display[d], HP_BAR_X, HP_BAR_Y, HP_BAR_X + i, COL_F3);

 draw_hitpulses(d, p, 0, HP_BAR_X, HP_BAR_Y, bitshift, COL_F1 + TRANS_RED2);

 i = PP.hp >> bitshift;
 col = COL_E6;
 if (i > 0)
  rectfill(display[d], HP_BAR_X, HP_BAR_Y - HP_BAR_THICK, HP_BAR_X + i, HP_BAR_Y + HP_BAR_THICK, col);

 i = PP.max_shield >> bitshift;
 vline(display[d], HP_BAR_X - 1, SH_BAR_Y - HP_BAR_END_THICK, SH_BAR_Y + HP_BAR_END_THICK, COL_BAR_END);
 vline(display[d], HP_BAR_X - 2, SH_BAR_Y - HP_BAR_END_THICK, SH_BAR_Y + HP_BAR_END_THICK, COL_BAR_END);
 vline(display[d], HP_BAR_X + i + 1, SH_BAR_Y - HP_BAR_END_THICK, SH_BAR_Y + HP_BAR_END_THICK, COL_BAR_END);
 vline(display[d], HP_BAR_X + i + 2, SH_BAR_Y - HP_BAR_END_THICK, SH_BAR_Y + HP_BAR_END_THICK, COL_BAR_END);
 hline(display[d], HP_BAR_X, SH_BAR_Y, HP_BAR_X + i, COL_F2);
 vline(display[d], (HP_BAR_X + (i/4)), SH_BAR_Y - HP_BAR_END_THICK + 1, SH_BAR_Y + HP_BAR_END_THICK - 1, COL_BAR_END-1);
// rect(display[d], HP_BAR_X - 2, SH_BAR_Y - 2, HP_BAR_X + i + 2, SH_BAR_Y + HP_BAR_THICK + 2, 13);

 draw_hitpulses(d, p, 1, HP_BAR_X, SH_BAR_Y, bitshift, COL_F1 + TRANS_RED2);

 i = PP.shield >> bitshift;
 if (i > 0)
 {
  col = COL_F5 + TRANS_BLUE2;
  if (PP.shield_flash > 0)
   col = COL_F6 + TRANS_BLUE3;
  if (PP.shield_up == 0)
   col = COL_F2 + TRANS_BLUE1;
  rectfill(display[d], HP_BAR_X, SH_BAR_Y - HP_BAR_THICK, HP_BAR_X + i, SH_BAR_Y + HP_BAR_THICK, col);
 }


/*
 int bitshift = 4;

 if (arena.only_player == -1)
  bitshift = 5;

 i = PP.max_hp >> bitshift;
 rect(display[d], HP_BAR_X - 2, HP_BAR_Y - 2, HP_BAR_X + i + 2, HP_BAR_Y + HP_BAR_THICK + 2, 13);
 i = PP.hp >> bitshift;
 col = COL_E6;
 if (i > 0)
  rectfill(display[d], HP_BAR_X, HP_BAR_Y, HP_BAR_X + i, HP_BAR_Y + HP_BAR_THICK, col);

 i = PP.max_shield >> bitshift;
 rect(display[d], HP_BAR_X - 2, SH_BAR_Y - 2, HP_BAR_X + i + 2, SH_BAR_Y + HP_BAR_THICK + 2, 13);
 i = PP.shield >> bitshift;
 if (i > 0)
 {
  col = COL_F5 + TRANS_BLUE2;
  if (PP.shield_flash > 0)
   col = COL_F6 + TRANS_BLUE3;
  if (PP.shield_up == 0)
   col = COL_F2 + TRANS_BLUE1;
  rectfill(display[d], HP_BAR_X, SH_BAR_Y, HP_BAR_X + i, SH_BAR_Y + HP_BAR_THICK, col);
 }
*/
// draw target damage display:

 int tx = 60;

 if (arena.only_player == -1 && p == 1)
  tx = 340;


#define TARGET_X tx
#define TARGET_Y 540
//#define TARGET_Y 440
/*
for (i = 0; i < DAMAGE_COLS; i++)
{
     draw_rle_sprite(display[d], damage_sprite [eclass[SHIP_FIGHTER].dsprite [0] [0]] [i], 100 + i * 40, 100);
}*/
//extern struct BMP_STRUCT ship_collision_mask [NO_SHIP_TYPES]; // externed in bullet.c

// draw_sprite(display[0], ship_collision_mask [SHIP_OLD2].sprite, 200, 200);

 int ta = PP.target_a;
 int te = PP.target_e;
 /*
SHIP_OLD2,
SHIP_OLD3,
SHIP_FIGHTER,
SHIP_FRIEND3,
SHIP_SCOUT2,
SHIP_BOMBER,
SHIP_FIGHTER_FRIEND,
SHIP_SCOUT3,
SHIP_ESCOUT,
SHIP_SCOUTCAR,
*/

#define NAME_COL COL_BOX3
#define ENAME_COL COL_EBOX3

 if (ta != -1)
 {
  switch(ship[ta][te].type)
  {
   case SHIP_OLD2: textprintf_ex(display[d], small_font, TARGET_X - 30, TARGET_Y - 43, NAME_COL, -1, "CTBR-SUNSHARK"); break;
   case SHIP_OLD3: textprintf_ex(display[d], small_font, TARGET_X - 30, TARGET_Y - 43, NAME_COL, -1, "CTBR-STARWHALE"); break;
   case SHIP_FIGHTER_FRIEND: textprintf_ex(display[d], small_font, TARGET_X - 30, TARGET_Y - 43, NAME_COL, -1, "CTBR-SANDFLY"); break;
   case SHIP_FSTRIKE: textprintf_ex(display[d], small_font, TARGET_X - 30, TARGET_Y - 43, NAME_COL, -1, "CTBR-RAM"); break;
   case SHIP_IBEX: textprintf_ex(display[d], small_font, TARGET_X - 30, TARGET_Y - 43, NAME_COL, -1, "CTBR-IBEX"); break;
   case SHIP_AUROCHS: textprintf_ex(display[d], small_font, TARGET_X - 30, TARGET_Y - 43, NAME_COL, -1, "CTBR-AUROCHS"); break;
   case SHIP_LACEWING: textprintf_ex(display[d], small_font, TARGET_X - 30, TARGET_Y - 43, NAME_COL, -1, "CTBR-LACEWING"); break;
   case SHIP_MONARCH: textprintf_ex(display[d], small_font, TARGET_X - 30, TARGET_Y - 43, NAME_COL, -1, "CTBR-MONARCH"); break;
   case SHIP_DROM: textprintf_ex(display[d], small_font, TARGET_X - 30, TARGET_Y - 43, NAME_COL, -1, "OCSF-DROMEDARY"); break;
   case SHIP_LINER: textprintf_ex(display[d], small_font, TARGET_X - 30, TARGET_Y - 43, NAME_COL, -1, "STARLINER"); break;
   case SHIP_FRIEND3: textprintf_ex(display[d], small_font, TARGET_X - 30, TARGET_Y - 43, NAME_COL, -1, "IF-TRIREME"); break;

   case SHIP_SCOUT2: textprintf_ex(display[d], small_font, TARGET_X - 30, TARGET_Y - 43, ENAME_COL, -1, "FSF-RIGHTEOUS"); break;
   case SHIP_SCOUT3: textprintf_ex(display[d], small_font, TARGET_X - 30, TARGET_Y - 43, ENAME_COL, -1, "FSF-MERCIFUL"); break;
   case SHIP_FIGHTER: textprintf_ex(display[d], small_font, TARGET_X - 30, TARGET_Y - 43, ENAME_COL, -1, "FSF-CALLED"); break;
   case SHIP_BOMBER: textprintf_ex(display[d], small_font, TARGET_X - 30, TARGET_Y - 43, ENAME_COL, -1, "FSF-"); break;
   case SHIP_ESCOUT: textprintf_ex(display[d], small_font, TARGET_X - 30, TARGET_Y - 43, ENAME_COL, -1, "FSF-"); break;
   case SHIP_EINT: textprintf_ex(display[d], small_font, TARGET_X - 30, TARGET_Y - 43, ENAME_COL, -1, "FSF-CHOSEN"); break;
   case SHIP_SCOUTCAR: textprintf_ex(display[d], small_font, TARGET_X - 30, TARGET_Y - 43, ENAME_COL, -1, "FSF-GRACEFUL"); break;
   case SHIP_ECARRIER: textprintf_ex(display[d], small_font, TARGET_X - 30, TARGET_Y - 43, ENAME_COL, -1, "FSF-INFINITE"); break;
   case SHIP_FREIGHT: textprintf_ex(display[d], small_font, TARGET_X - 30, TARGET_Y - 43, ENAME_COL, -1, "FSF-BENEVOLENT"); break;
   case SHIP_EBASE: textprintf_ex(display[d], small_font, TARGET_X - 30, TARGET_Y - 43, ENAME_COL, -1, "FSF-ORBITAL"); break;



  }
  if (ship[ta][te].letter != -1)
  {
   switch(ship[ta][te].letter)
   {
    case 0: textprintf_ex(display[d], small_font, TARGET_X - 30, TARGET_Y - 53, NAME_COL, -1, "Gazer %i", ship[ta][te].letter_rank); break;
    case 1: textprintf_ex(display[d], small_font, TARGET_X - 30, TARGET_Y - 53, NAME_COL, -1, "Beta %i", ship[ta][te].letter_rank); break;
    case 2: textprintf_ex(display[d], small_font, TARGET_X - 30, TARGET_Y - 53, NAME_COL, -1, "Gamma %i", ship[ta][te].letter_rank); break;
   }
  }
 }

// rect(display[d], TARGET_X - 30, TARGET_Y - 30, TARGET_X + 30, TARGET_Y + 30, COL_F4);

 if (PP.target_new > 3)
 {
  TRANS_MODE
  switch((PP.target_new-3)>>1)
  {
   case 0: rectfill(display[d], TARGET_X - 30, TARGET_Y - 30, TARGET_X + 30, TARGET_Y + 30, TRANS_BLUE1); break;
   case 1: rectfill(display[d], TARGET_X - 30, TARGET_Y - 30, TARGET_X + 30, TARGET_Y + 30, TRANS_BLUE2); break;
   case 2: rectfill(display[d], TARGET_X - 30, TARGET_Y - 30, TARGET_X + 30, TARGET_Y + 30, TRANS_BLUE3); break;
   default: rectfill(display[d], TARGET_X - 30, TARGET_Y - 30, TARGET_X + 30, TARGET_Y + 30, TRANS_BLUE3); break;
  }
  END_TRANS
 }

 if (ta == -1)
 {
     textprintf_centre_ex(display[d], small_font, TARGET_X, TARGET_Y - 12, COL_F5, -1, "no");
     textprintf_centre_ex(display[d], small_font, TARGET_X, TARGET_Y, COL_F5, -1, "target");
 }
  else
  {
    int integrity;
    if (eclass[ship[ta][te].type].parts == 1) // probably a fighter
    {
     col = 0;
     integrity = (10 * ship[ta][te].hp [0]) / ship[ta][te].max_hp [0];
     if (integrity < 7)
      col = 1;
     if (integrity < 3)
      col = 2;
     if (ship[ta][te].hit_pulse [0] > 0)
      col += 4;
     draw_rle_sprite(display[d], damage_sprite [eclass[ship[ta][te].type].dsprite [0] [0]] [col], TARGET_X - eclass[ship[ta][te].type].dsprite [0] [1], TARGET_Y - eclass[ship[ta][te].type].dsprite [0] [2]);
    }
     else
     {
      for (i = 0; i < eclass[ship[ta][te].type].parts; i ++)
      {
       col = 0;
       integrity = (10 * ship[ta][te].hp [i]) / ship[ta][te].max_hp [i];
       if (integrity < 7)
        col = 1;
       if (integrity < 3)
        col = 2;
       if (ship[ta][te].hit_pulse [i] > 0)
        col += 4;
       if (ship[ta][te].hp [i] <= 0)
        col = 3; // no pulse for this one
       draw_rle_sprite(display[d], damage_sprite [eclass[ship[ta][te].type].dsprite [i] [0]] [col], TARGET_X - eclass[ship[ta][te].type].dsprite [i] [1], TARGET_Y - eclass[ship[ta][te].type].dsprite [i] [2]);
      }
       if (eclass[ship[ta][te].type].fins)
        draw_rle_sprite(display[d], damage_sprite [eclass[ship[ta][te].type].dsprite [i+1] [0]] [3], TARGET_X - eclass[ship[ta][te].type].dsprite [i+1] [1], TARGET_Y - eclass[ship[ta][te].type].dsprite [i+1] [2]);
       col = 0;
       integrity = (10 * ship[ta][te].structure) / ship[ta][te].max_structure;
       if (integrity < 7)
        col = 1;
       if (integrity < 3)
        col = 2;
       if (ship[ta][te].hit_pulse_structure > 0)
        col += 4;
// the value of i is still used here...
       draw_rle_sprite(display[d], damage_sprite [eclass[ship[ta][te].type].dsprite [i] [0]] [col], TARGET_X - eclass[ship[ta][te].type].dsprite [i] [1], TARGET_Y - eclass[ship[ta][te].type].dsprite [i] [2]);
     }

#define SHIELDBAR_X (TARGET_X + 35)
#define SHIELDBAR_X2 (TARGET_X + 2)
#define SHIELDBAR_W 6
#define SHIELDBAR_W2 2

     if (ship[ta][te].shield_generator)
     {
//      rect(display[d], SHIELDBAR_X, TARGET_Y - 31, SHIELDBAR_X + SHIELDBAR_W, TARGET_Y + 31, COL_F4);
      hline(display[d], SHIELDBAR_X, TARGET_Y - 31, SHIELDBAR_X + SHIELDBAR_W, COL_F5);
      hline(display[d], SHIELDBAR_X, TARGET_Y + 31, SHIELDBAR_X + SHIELDBAR_W, COL_F5);
      hline(display[d], SHIELDBAR_X, TARGET_Y + 10, SHIELDBAR_X + SHIELDBAR_W, COL_F4);

      if (eclass[ship[ta][te].type].ship_class == ECLASS_WSHIP)
      {
       hline(display[d], SHIELDBAR_X, TARGET_Y + 31, SHIELDBAR_X - 5, COL_F4);
       vline(display[d], SHIELDBAR_X - 5, TARGET_Y + 31, TARGET_Y + eclass[ship[ta][te].type].generator_y, COL_F4);
       hline(display[d], SHIELDBAR_X - 5, TARGET_Y + eclass[ship[ta][te].type].generator_y, TARGET_X, COL_F4);
      }

      if (ship[ta][te].shield > 0)
      {
       int shield_bar = (ship[ta][te].shield * 60) / ship[ta][te].max_shield;
       if (ship[ta][te].shield_bar_pulse > 0)
       {
//        rectfill(display[d], SHIELDBAR_X + 1, TARGET_Y + (30 - shield_bar), SHIELDBAR_X + SHIELDBAR_W - 1, TARGET_Y + 30, COL_F4 + TRANS_BLUE2);
        rectfill(display[d], SHIELDBAR_X + 2, TARGET_Y + (30 - shield_bar), SHIELDBAR_X + SHIELDBAR_W - 2, TARGET_Y + 30, COL_F4 + TRANS_BLUE2);
       }
         else
         {
          if (ship[ta][te].shield_up)
//           rectfill(display[d], SHIELDBAR_X + 1, TARGET_Y + (30 - shield_bar), SHIELDBAR_X + SHIELDBAR_W - 1, TARGET_Y + 30, COL_F2 + TRANS_BLUE1);
           rectfill(display[d], SHIELDBAR_X + 2, TARGET_Y + (30 - shield_bar), SHIELDBAR_X + SHIELDBAR_W - 2, TARGET_Y + 30, COL_F2 + TRANS_BLUE1);
            else
             rectfill(display[d], SHIELDBAR_X + 2, TARGET_Y + (30 - shield_bar), SHIELDBAR_X + SHIELDBAR_W - 2, TARGET_Y + 30, COL_E1 + TRANS_RED1);
//             rectfill(display[d], SHIELDBAR_X + 1, TARGET_Y + (30 - shield_bar), SHIELDBAR_X + SHIELDBAR_W - 1, TARGET_Y + 30, COL_E1 + TRANS_RED1);
         }
      }


     }


// we've drawn the target damage display - now do a little thing around the target, if it's visible:
//  - using target_sight variables set in display_ship
   if (PP.target_sight_visible)
   {
/*        circle(display[d], PP.target_sight_x, PP.target_sight_y,
          (eclass[ship[PP.target_a][PP.target_e].type].size >> 10) + 20,
          COL_F5);*/
#define CORNER_LINE 7
#define LINE_COL COL_WHITE

 int lsize = (eclass[ship[PP.target_a][PP.target_e].type].size >> 10);
 if (eclass[ship[PP.target_a][PP.target_e].type].ship_class == ECLASS_FIGHTER)
  lsize += 16;
 lsize += PP.target_new<<2;

     hline(display[d], PP.target_sight_x - lsize,
                       PP.target_sight_y - lsize,
                       PP.target_sight_x - lsize
                                         + CORNER_LINE, LINE_COL);
     hline(display[d], PP.target_sight_x + lsize,
                       PP.target_sight_y - lsize,
                       PP.target_sight_x + lsize
                                         - CORNER_LINE, LINE_COL);
     hline(display[d], PP.target_sight_x - lsize,
                       PP.target_sight_y + lsize,
                       PP.target_sight_x - lsize
                                         + CORNER_LINE, LINE_COL);
     hline(display[d], PP.target_sight_x + lsize,
                       PP.target_sight_y + lsize,
                       PP.target_sight_x + lsize
                                         - CORNER_LINE, LINE_COL);

     vline(display[d], PP.target_sight_x - lsize,
                       PP.target_sight_y - lsize,
                       PP.target_sight_y - lsize
                                         + CORNER_LINE, LINE_COL);
     vline(display[d], PP.target_sight_x + lsize,
                       PP.target_sight_y - lsize,
                       PP.target_sight_y - lsize
                                         + CORNER_LINE, LINE_COL);
     vline(display[d], PP.target_sight_x - lsize,
                       PP.target_sight_y + lsize,
                       PP.target_sight_y + lsize
                                         - CORNER_LINE, LINE_COL);
     vline(display[d], PP.target_sight_x + lsize,
                       PP.target_sight_y + lsize,
                       PP.target_sight_y + lsize
                                         - CORNER_LINE, LINE_COL);


   }
//    else
// target not visible, so we'll draw an indicator at the edge of the screen:
    {
      int dist = ((int) hypot(ship[ta][te].y - PP.y, ship[ta][te].x - PP.x)) >> 10;
      if (dist > 80)
      {
       int ang = radians_to_angle(atan2(PP.y - ship[ta][te].y, PP.x - ship[ta][te].x)) - camera_angle + ANGLE_4;
       int disp = dist >> 7;
       if (disp > 20)
        disp = 20;
       line(display[d], PP.camera_x + xpart(ang, 80) + xpart(ang + ANGLE_4, disp),
                        PP.camera_y + ypart(ang, 80) + ypart(ang + ANGLE_4, disp),
                        PP.camera_x + xpart(ang, 84) + xpart(ang + ANGLE_4, disp),
                        PP.camera_y + ypart(ang, 84) + ypart(ang + ANGLE_4, disp), COL_WHITE);
       line(display[d], PP.camera_x + xpart(ang, 80) + xpart(ang - ANGLE_4, disp),
                        PP.camera_y + ypart(ang, 80) + ypart(ang - ANGLE_4, disp),
                        PP.camera_x + xpart(ang, 84) + xpart(ang - ANGLE_4, disp),
                        PP.camera_y + ypart(ang, 84) + ypart(ang - ANGLE_4, disp), COL_WHITE);
//       PP.camera_y + ypart(ang, 80), 4 + (dist>>3), COL_WHITE);
       //circle(display[0], PP.camera_x + xpart(ang, 80), PP.camera_y + ypart(ang, 80), 4 + (dist>>3), COL_WHITE);
      }
/*      int dist = ((int) hypot(ship[ta][te].y - PP.y, ship[ta][te].x - PP.x)) >> 10;
      int ang = radians_to_angle(atan2(PP.y - ship[ta][te].y, PP.x - ship[ta][te].x)) - camera_angle + ANGLE_4;
      int ix, iy;
      if (xpart(angle, dist) < 0-PP.camera_x)
      {
       ix = 0;
       iy = PP.camera_x dist
      }*/
    }

  }

 if (PP.target_auto == 0)
 {
   textprintf_ex(display[d], small_font, TARGET_X - 25, TARGET_Y + 42, COL_E5, -1, "manual target");
 }
  else
   textprintf_ex(display[d], small_font, TARGET_X - 25, TARGET_Y + 42, COL_F5, -1, "auto target");

 if (!arena.camera_fix)
 {
  hline(display[d], PP.camera_x - 20, PP.camera_y - 250, PP.camera_x - 15, HUD_COL_2);
  hline(display[d], PP.camera_x + 20, PP.camera_y - 250, PP.camera_x + 15, HUD_COL_2);
  vline(display[d], PP.camera_x - 15, PP.camera_y - 252, PP.camera_y - 248, HUD_COL_2);
  vline(display[d], PP.camera_x + 15, PP.camera_y - 252, PP.camera_y - 248, HUD_COL_2);
 }
  else
  {
   line(display[d], PP.camera_x + xpart(PP.angle, 250) + xpart(PP.angle + ANGLE_4, 15),
                    PP.camera_y + ypart(PP.angle, 250) + ypart(PP.angle + ANGLE_4, 15),
                    PP.camera_x + xpart(PP.angle, 250) + xpart(PP.angle + ANGLE_4, 20),
                    PP.camera_y + ypart(PP.angle, 250) + ypart(PP.angle + ANGLE_4, 20), HUD_COL_2);
   line(display[d], PP.camera_x + xpart(PP.angle, 250) + xpart(PP.angle - ANGLE_4, 15),
                    PP.camera_y + ypart(PP.angle, 250) + ypart(PP.angle - ANGLE_4, 15),
                    PP.camera_x + xpart(PP.angle, 250) + xpart(PP.angle - ANGLE_4, 20),
                    PP.camera_y + ypart(PP.angle, 250) + ypart(PP.angle - ANGLE_4, 20), HUD_COL_2);
  }


 if (PP.respawning > 0
  && PP.respawning < 70)
 {
   textprintf_centre_ex(display[d], small_font, PP.camera_x, 300, -1, -1, "Relaunching");

 }

 if (PP.shield_up == 0)
  textprintf_ex(display[d], small_font, PP.camera_x + 40, AF_Y + 10, COL_EBOX3, -1, "shield down");
   else
 {
   if (PP.shield_just_up > 0)
    textprintf_ex(display[d], small_font, PP.camera_x + 40, AF_Y + 10, COL_BOX3, -1, "shield up");
 }


#define WING_Y 120

 y = WING_Y;
 int integrity, w;
 int old_y;

 for (w = 0; w < WINGS; w ++)
 {
     y += 20;

 old_y = y; // if this is still the same after the next loop, there are no fighters in this wing

 for (i = 0; i < WING_SIZE; i ++)
 {
  if (PP.wing [w] [i] != -1)
  {
   col = 0;
   integrity = (10 * ship[TEAM_FRIEND][PP.wing [w] [i]].hp [0]) / ship[TEAM_FRIEND][PP.wing [w] [i]].max_hp [0];
   if (integrity < 7)
    col = 1;
   if (integrity < 3)
    col = 2;
   if (ship[TEAM_FRIEND][PP.wing [w] [i]].hit_pulse [0] > 0)
    col += 4;
   draw_rle_sprite(display[d], damage_sprite [eclass[ship[TEAM_FRIEND][PP.wing [w] [i]].type].dsprite [0] [0]] [col], 15 - eclass[ship[TEAM_FRIEND][PP.wing [w] [i]].type].dsprite [0] [1], y - eclass[ship[TEAM_FRIEND][PP.wing [w] [i]].type].dsprite [0] [2]);
//   circlefill(display[d], 10, y, 5, col);
   vline(display[d], 32, y - 3, y + 3, COL_WHITE);
   vline(display[d], 34 + (ship[TEAM_FRIEND][PP.wing [w] [i]].max_shield>>6), y - 3, y + 3, COL_WHITE);
   col = COL_F2 + TRANS_BLUE2;
   if (ship[TEAM_FRIEND][PP.wing [w] [i]].shield_up == 0)
    col = COL_E3 + TRANS_RED1;
   if (ship[TEAM_FRIEND][PP.wing [w] [i]].shield > 0)
    rectfill(display[d], 33, y - 1, 33 + (ship[TEAM_FRIEND][PP.wing [w] [i]].shield>>6), y + 1, col);
/*   switch(ship[TEAM_FRIEND][PP.wing [i]].action)
   {
    case ACT_WING_FORM:
     textprintf_ex(display[0], small_font, 35, y - 4, COL_WHITE, -1, "form"); break;
    case ACT_WING_AWAY:
     textprintf_ex(display[0], small_font, 35, y - 4, COL_WHITE, -1, "away"); break;
    case ACT_WING_SEEK:
     textprintf_ex(display[0], small_font, 35, y - 4, COL_WHITE, -1, "seek"); break;
    case ACT_WING_ATTACK:
     textprintf_ex(display[0], small_font, 35, y - 4, COL_WHITE, -1, "attack"); break;
    case ACT_WING_EVADE:
     textprintf_ex(display[0], small_font, 35, y - 4, COL_WHITE, -1, "evade"); break;
    default:
     textprintf_ex(display[0], small_font, 35, y - 4, COL_WHITE, -1, "not wing: %i", ship[TEAM_FRIEND][PP.wing [i]].action); break;

   }*/
   y += 25;
  }
 }

  char comstring [20];


 if (y != old_y) // i.e. if there is at least one fighter in wing
 {
  strcpy(comstring, "command: ");

  switch(PP.wing_orders [w])
  {
   case COMMAND_FORM: strcat(comstring, "form up"); break;
   case COMMAND_COVER: strcat(comstring, "cover me"); break;
   case COMMAND_ATTACK: strcat(comstring, "attack"); break;
   case COMMAND_ENGAGE: strcat(comstring, "engage"); break;
   case COMMAND_DEFEND: strcat(comstring, "defend"); break;
// REMEMBER length of string!!
   default: strcat(comstring, "ERROR"); break;
  }

  textprintf_ex(display[d], small_font, 5, y - 10, COL_WHITE, -1, comstring);

 }

/*  if (PP.just_commanded > 0
   && ((PP.just_commanded+2) >> 3) & 1)*/
/*   {
    col = COL_BOX0 + (PP.just_commanded >> 1);
    if (col > COL_BOX3)
     col = COL_BOX3;
    rectfill(display[0], 10, y - 5, 19 + text_length(small_font, comstring), y + 9, col);
    col += 1;
    if (col > COL_BOX4)
     col = COL_BOX4;
    rect(display[0], 10, y - 5, 19 + text_length(small_font, comstring), y + 9, col);
   }*/


 }

  if (PP.commanding > 0)
  {
    textprintf_ex(display[d], small_font, 20, y + 15, COL_WHITE, -1, "GIVE COMMAND -");
    y += 15;

#define COM_X 55

    textprintf_right_ex(display[d], small_font, COM_X, y + 15, COL_BOX3, -1, "fire 1: ");
    textprintf_right_ex(display[d], small_font, COM_X, y + 30, COL_BOX3, -1, "fire 2: ");
    textprintf_right_ex(display[d], small_font, COM_X, y + 45, COL_BOX3, -1, "fire 3: ");
    textprintf_right_ex(display[d], small_font, COM_X, y + 60, COL_BOX3, -1, "slide: ");
    textprintf_right_ex(display[d], small_font, COM_X, y + 75, COL_BOX3, -1, "target: ");
    textprintf_right_ex(display[d], small_font, COM_X, y + 90, COL_BOX3, -1, "command: ");

   if (PP.command_mode == CMODE_TACTIC)
   {
    if (PP.target_a == TEAM_ENEMY)
     textprintf_ex(display[d], small_font, COM_X, y + 15, COL_BOX4, -1, " attack my current target");
      else
       textprintf_ex(display[d], small_font, COM_X, y + 15, COL_BOX4, -1, " engage my targets");
    textprintf_ex(display[d], small_font, COM_X, y + 30, COL_BOX4, -1, " cover me");
    textprintf_ex(display[d], small_font, COM_X, y + 45, COL_BOX4, -1, " form up");
//    textprintf_ex(display[d], small_font, COM_X, y + 45, COL_BOX4, -1, " wc %i", PP.wing_command);

     int col1, col2;

     textprintf_ex(display[d], small_font, COM_X, y + 60, COL_BOX4, -1, " command");

     for (i = -1; i < 2; i ++)
     {

        if (PP.wing_command == i)
        {
            col1 = COL_WHITE;
            col2 = COL_BOX2;
        }
         else
         {
             col1 = COL_BOX1;
             col2 = -1;
         }
      switch(i)
      {
          case 0: textprintf_ex(display[d], small_font, COM_X + 60, y + 60, col1, col2, " wing 1"); break;
          case 1: textprintf_ex(display[d], small_font, COM_X + 100, y + 60, col1, col2, " wing 2"); break;
          case -1: textprintf_ex(display[d], small_font, COM_X + 140, y + 60, col1, col2, " all"); break;
      }

     }

/*    switch(PP.wing_command)
    {
     case -1: textprintf_ex(display[d], small_font, COM_X, y + 60, COL_WHITE, -1, " command all"); break;
     case 0: textprintf_ex(display[d], small_font, COM_X, y + 60, COL_BOX4, -1, " command wing 1"); break;
     case 1: textprintf_ex(display[d], small_font, COM_X, y + 60, COL_EBOX4, -1, " command wing 2"); break;
    }*/
    textprintf_ex(display[d], small_font, COM_X, y + 75, COL_BOX4, -1, " special >>");
   }
    else // must be CMODE_WING
    {
     for (w = 0; w < 2; w ++)
     {
      if (PP.wing_size [w] > 0)
      {
       if (PP.wing_size [w] == 1)
        textprintf_ex(display[d], small_font, COM_X, y + 15 + (w*30), COL_BOX4, -1, " reinforce wing %i", w + 1);
         else
          textprintf_ex(display[d], small_font, COM_X, y + 15 + (w*30), COL_EBOX4, -1, " no order");
       textprintf_ex(display[d], small_font, COM_X, y + 30 + (w*30), COL_BOX4, -1, " dismiss wing %i", w + 1);
      }
       else
       {
        textprintf_ex(display[d], small_font, COM_X, y + 15 + (w*30), COL_BOX4, -1, " call up fighters for wing %i", w + 1);
        textprintf_ex(display[d], small_font, COM_X, y + 30 + (w*30), COL_BOX4, -1, " call up bombers for wing %i", w + 1);
       }
     }

    }
    textprintf_ex(display[d], small_font, COM_X, y + 90, COL_BOX3, -1, " cancel");

  }

// }



/*
// shield time

#define SHIELD_X 10
#define SHIELD_Y 449
#define SHIELD_WIDTH 7

   for (i = 0; i < PP.max_shield; i ++)
   {
    rect(display[d], SHIELD_X - 1, SHIELD_Y - i * 28 + 1,
                     SHIELD_X + SHIELD_WIDTH + 1, SHIELD_Y - i * 28 - 24 - 1,
                     TRANS_BLUE2);
   }

   for (i = 0; i < PP.shield_stock; i ++)
   {
    rectfill(display[d], SHIELD_X , SHIELD_Y - i * 28,
                     SHIELD_X + SHIELD_WIDTH, SHIELD_Y - i * 28 - 24,
                     TRANS_BLUE3);
   }

   if (PP.next_shield > 0)
   {
        i = PP.shield_stock;

       int shield_bar = (PP.next_shield * 24) / 12000;

    rectfill(display[d], SHIELD_X , SHIELD_Y - i * 28,
                     SHIELD_X + SHIELD_WIDTH, SHIELD_Y - i * 28 - shield_bar,
                     TRANS_BLUE2);


   }

// charge time

#define CHARGE_X 25
#define CHARGE_Y 450
#define CHARGE_WIDTH 10

 rect(display[d], CHARGE_X - 1, CHARGE_Y - 102 - 1, CHARGE_X + CHARGE_WIDTH + 1, CHARGE_Y + 1, COL_2);
 rect(display[d], CHARGE_X, CHARGE_Y - 102, CHARGE_X + CHARGE_WIDTH, CHARGE_Y, COL_8);
 rect(display, CHARGE_X + 1, CHARGE_Y - 102 + 1, CHARGE_X + CHARGE_WIDTH - 1, CHARGE_Y - 1, COL_2);
 hline(display, CHARGE_X + 1, CHARGE_Y - 20, CHARGE_X + CHARGE_WIDTH - 1, COL_4);

 if (PP.charge > 0)
 {

 int bar = PP.charge / 100;
 int col = TRANS_RED2;

 if (PP.charge > 2000)
  col = TRANS_YELLOW4;

 TRANS_MODE
 rectfill(display[d], CHARGE_X + 2, CHARGE_Y - bar, CHARGE_X + CHARGE_WIDTH - 2, CHARGE_Y - 2, col);
 END_TRANS
 }

 if (PP.rocket_burst > 0)
 {
  TRANS_MODE
  if (PP.weapon == WEAPON_MISSILE)
  {
   for (i = 0; i < PP.rocket_burst; i ++)
   {
    rectfill(display, CHARGE_X + CHARGE_WIDTH + 5, CHARGE_Y - i * 8,
                     CHARGE_X + CHARGE_WIDTH + 15, CHARGE_Y - i * 8 - 5,
                     TRANS_YELLOW4);
    rect(display, CHARGE_X + CHARGE_WIDTH + 5 - 1, CHARGE_Y - i * 8 + 1,
                     CHARGE_X + CHARGE_WIDTH + 15 + 1, CHARGE_Y - i * 8 - 5 - 1,
                     TRANS_GREEN2);
   }
  }
   else
   {
    for (i = 0; i < PP.rocket_burst; i ++)
    {
     rectfill(display, CHARGE_X + CHARGE_WIDTH + 5, CHARGE_Y - i * 6,
                     CHARGE_X + CHARGE_WIDTH + 15, CHARGE_Y - i * 6 - 4,
                     TRANS_GREEN3);
    }
   }
  END_TRANS
 }


#define BAR_X 610
#define BAR_Y 365
#define END_WIDTH 2

  hline(display, BAR_X - END_WIDTH, BAR_Y, BAR_X + END_WIDTH, COL_8);
  rect(display, BAR_X - END_WIDTH - 1, BAR_Y - 1, BAR_X + END_WIDTH + 1, BAR_Y + 1, COL_2);


  if (arena.end_stage > 0
   && arena.game_over == 0)
  {
   textprintf_centre_ex(display, font, 320, 100, -1, -1, "Level");
   textprintf_centre_ex(display, font, 320, 140, -1, -1, "Completed");
  }

  if (arena.game_over > 0)
  {
   textprintf_centre_ex(display, font, 320, 100, -1, -1, "Game");
   textprintf_centre_ex(display, font, 320, 120, -1, -1, "Over");
   if (arena.time_left <= 0)
    textprintf_centre_ex(display, font, 320, 160, -1, -1, "Out of time.");
  }

   int minutes_left = arena.time_left / 3000;
   int seconds_left = (arena.time_left / 50) - (minutes_left * 60);

 if (seconds_left < 10)
 {
  textprintf_right_ex(display, font, 630, 10, -1, -1, "%i:0%i", minutes_left, seconds_left);
  textprintf_right_ex(display, font, 630, 10, -1, -1, "%i:0%i", minutes_left, seconds_left);
 }
  else
  {
   textprintf_right_ex(display, font, 630, 10, -1, -1, "%i:%i", minutes_left, seconds_left);
   textprintf_right_ex(display, font, 630, 10, -1, -1, "%i:%i", minutes_left, seconds_left);
  }

 if (arena.end_stage == 0 && arena.game_over == 0
  && minutes_left == 0)
  {
   if (seconds_left > 26 && seconds_left < 31)
    textprintf_centre_ex(display, font, 320, 100, -1, -1, "30 seconds left");
   if (seconds_left > 6 && seconds_left < 11)
    textprintf_centre_ex(display, font, 320, 100, -1, -1, "10 seconds left");
  }

 if (PP.ships > 0)
 {
  for (i = 0; i < PP.ships; i ++)
  {
   draw_rle_sprite(display, RLE_player [PLAYER_RLE_MINI], 618 - i * 14, 25);
  }
 }
*/
}


void torp_arming_lines(int d, int p, int w, int max_charge, int max_reload, int width)
{


 int length;

    if (PP.weapon_charge [w] > 0)
    {
     vline(display[d], PP.camera_x - width, TORP_Y - 5, TORP_Y + 5, HUD_COL_3);
     vline(display[d], PP.camera_x + width, TORP_Y - 5, TORP_Y + 5, HUD_COL_3);

     length = (width * PP.weapon_charge [w]) / max_charge;

     vline(display[d], PP.camera_x - width + length, TORP_Y - 2, TORP_Y + 2, HUD_COL_3);
     hline(display[d], PP.camera_x - width, TORP_Y, PP.camera_x - width + length, HUD_COL_3);
     vline(display[d], PP.camera_x + width - length, TORP_Y - 2, TORP_Y + 2, HUD_COL_3);
     hline(display[d], PP.camera_x + width, TORP_Y, PP.camera_x + width - length, HUD_COL_3);
    } else
     {
      if (PP.weapon_charge [w] < 0)
      {
       length = (width * PP.weapon_charge [w]) / max_reload;

       vline(display[d], PP.camera_x - width, TORP_Y - 5, TORP_Y + 5, HUD_COL_1);
       vline(display[d], PP.camera_x + width, TORP_Y - 5, TORP_Y + 5, HUD_COL_1);

       vline(display[d], PP.camera_x - width - length, TORP_Y - 2, TORP_Y + 2, HUD_COL_1);
       hline(display[d], PP.camera_x - width, TORP_Y, PP.camera_x - width - length, HUD_COL_1);
       vline(display[d], PP.camera_x + width + length, TORP_Y - 2, TORP_Y + 2, HUD_COL_1);
       hline(display[d], PP.camera_x + width, TORP_Y, PP.camera_x + width + length, HUD_COL_1);
      }
       else
       {
        vline(display[d], PP.camera_x - width, TORP_Y - 5, TORP_Y + 5, HUD_COL_2);
        vline(display[d], PP.camera_x + width, TORP_Y - 5, TORP_Y + 5, HUD_COL_2);
       }
     }

}

void draw_hitpulses(int d, int p, int type, int x, int y, int bitshift, int col)
{

 int i;

 for (i = 0; i < HPULSE; i ++)
 {
  if (PP.hitpulse_thickness [type] [i] > 0)
  {
   rectfill(display[d],
            x + (PP.hitpulse_start [type] [i] >> bitshift), y - (PP.hitpulse_thickness [type] [i] >> 1),
            x + (PP.hitpulse_end [type] [i] >> bitshift), y + (PP.hitpulse_thickness [type] [i] >> 1),
            col);
  }
 }
}



#define OSCAN_WIDTH 150
#define OSCAN_HEIGHT 150
//#define OSCAN_SCALE 0.00002
#define OSCAN_SCALE 0.000005

void draw_overscan(int x, int y)
{

 rectfill(display [0], x, y, x + OSCAN_WIDTH, y + OSCAN_HEIGHT, COL_SCANNER_BACKGROUND);
 rect(display [0], x, y, x + OSCAN_WIDTH, y + OSCAN_HEIGHT, COL_F4);

 int centre_offset_x = ((float) OSCAN_WIDTH / OSCAN_SCALE) / 2;
 int centre_offset_y = ((float) OSCAN_HEIGHT / OSCAN_SCALE) / 2;


 int a, e, x2, y2, col, a_col, x3, y3;

 int p;

 for (p = 0; p < 2; p ++)
 {
  if (PP.alive)
  {
   x2 = (PP.x + centre_offset_x) * OSCAN_SCALE;
   y2 = (PP.y + centre_offset_y) * OSCAN_SCALE;
   if (x2 >= OSCAN_WIDTH)
    x2 = OSCAN_WIDTH - 1;
   if (x2 < 1)
    x2 = 1;
   if (y2 >= OSCAN_HEIGHT)
    y2 = OSCAN_HEIGHT - 1;
   if (y2 < 1)
    y2 = 1;

   vline(display[0], x + x2, y + 1, y + OSCAN_HEIGHT - 1, COL_F1);
   hline(display[0], x + 1, y + y2, x + OSCAN_WIDTH - 1, COL_F1);

//   x3 = x2 + xpart(camera_angle, 7);
//   y3 = y2 + ypart(camera_angle, 7);
   x3 = x2 + xpart(PP.angle, 7);
   y3 = y2 + ypart(PP.angle, 7);
   if (x3 > 1 && x3 < OSCAN_WIDTH - 1 && y3 > 1 && y3 < OSCAN_HEIGHT - 1)
   {
    circle(display[0], x + x2, y + y2, 7, COL_F3);
//    putpixel(display[0], x + x2, y + y2, x + x3, y + y3, COL_11 + TRANS_GREEN2);
    line(display[0], x + x2, y + y2, x + x3, y + y3, COL_F4);
    putpixel(display[0], x + x3, y + y3, COL_F6);
   }

   putpixel(display[0], x + x2, y + y2, COL_F6);

//   if (p == 0)
//    textprintf_ex(display[0], font, 100, 100, -1, -1, "x2 %i y2 %i", x2, y2);

  }
 }


TRANS_MODE
 for (a = 0; a < NO_TEAMS; a ++)
 {
 if (a == 0)
  a_col = TRANS_BLUE1;
   else
    a_col = TRANS_RED2;

 for (e = 0; e < NO_SHIPS; e ++)
 {
  if (EE.type == SHIP_NONE)
    continue;
  col = a_col + eclass[EE.type].blip_strength;

  x2 = (EE.x + centre_offset_x) * OSCAN_SCALE;
  y2 = (EE.y + centre_offset_y) * OSCAN_SCALE;

  if (x2 >= OSCAN_WIDTH)
   x2 = OSCAN_WIDTH - 1;
  if (x2 < 1)
   x2 = 1;
  if (y2 >= OSCAN_HEIGHT)
   y2 = OSCAN_HEIGHT - 1;
  if (y2 < 1)
   y2 = 1;


   switch(eclass[EE.type].blip_strength)
   {
    default:
    circlefill(display[0], x + x2, y + y2, 1, acol [EE.scancol] [eclass[EE.type].blip_strength] [0]);
    putpixel(display[0], x + x2, y + y2, acol [EE.scancol] [eclass[EE.type].blip_strength] [1]);
    break;
   case 5:
    circlefill(display[0], x + x2, y + y2, 2, acol [EE.scancol] [eclass[EE.type].blip_strength] [0]);
    circlefill(display[0], x + x2, y + y2, 1, acol [EE.scancol] [eclass[EE.type].blip_strength] [1]);
//    putpixel(display[0], x + x2, y + y2, acol [EE.scancol] [eclass[EE.type].blip_strength] [1]);
    break;
   case 6:
    circlefill(display[0], x + x2, y + y2, 2, acol [EE.scancol] [eclass[EE.type].blip_strength] [0]);
    circlefill(display[0], x + x2, y + y2, 1, acol [EE.scancol] [eclass[EE.type].blip_strength] [1]);
    break;
  }

//  putpixel(display [0], x + x2, y + y2, col);



 }
 }
END_TRANS

/*
  x2 = (convoy[1].target_x + centre_offset_x) * OSCAN_SCALE;
  y2 = (convoy[1].target_y + centre_offset_y) * OSCAN_SCALE;

  putpixel(display [0], x + x2, y + y2, grand(150));
*/


}



void draw_stars(int d, int p, char motion)
{
 int i;

 for (i = 0; i < NO_STARS; i ++)
 {

/*  if (star[i].bright > 4)
  {
   putpixel(display, (star[i].x >> 10) - 1, (star[i].y >> 10), star[i].bright2-2);
   putpixel(display, (star[i].x >> 10) + 1, (star[i].y >> 10), star[i].bright2-2);
   putpixel(display, (star[i].x >> 10), (star[i].y >> 10) - 1, star[i].bright2-2);
   putpixel(display, (star[i].x >> 10), (star[i].y >> 10) + 1, star[i].bright2-2);
  }*/
  if (motion)
   pline(display[d], star[p][i].x >> 10, star[p][i].y >> 10, star[p][i].old_x >> 10, star[p][i].old_y >> 10, star[p][i].bright2);
    else
     putpixel(display[d], star[p][i].x >> 10, star[p][i].y >> 10, star[p][i].bright);

//if (i < 5)
  //textprintf_ex(display[0], small_font, star[p][i].x >> 10, star[p][i].y >> 10, -1, -1, "%i:%i,%i(%i,%i)", i, star[p][i].x, star[p][i].y, star[p][i].old_x, star[p][i].old_y);
//  textprintf_ex(display[0], small_font, star[p][i].x >> 10, star[p][i].y >> 10, -1, -1, "%i", i);
//break;

//  putpixel(display, (star[i].x >> 11) + 320, (star[i].y >> 11) + 240, star[i].bright);

 }

}




void draw_bullets(int d, int p)
{

int b, size, i, a, visible, beam_power, x2, y2, x3 = 0, y3 = 0, side;

for (a = 0; a < NO_TEAMS; a ++)
{
for (b = 0; b < NO_BULLETS; b ++)
{

   if (BL.type == BULLET_NONE)
    continue;

   if ((BL.x < PP.x - (600 << 10) || BL.x > PP.x + (600 << 10)
    || BL.y < PP.y - (600 << 10) || BL.y > PP.y + (500 << 10))
     && BL.type != BULLET_EBEAM1)
     continue;

   float angle = atan2(BL.y - PP.y, BL.x - PP.x) - (PI/2) - camera_angle_rad;
   int dist = hypot(BL.y - PP.y, BL.x - PP.x);
   int x = PP.camera_x + cos(angle) * (dist >> 10);
   int y = PP.camera_y + sin(angle) * (dist >> 10);

//   float angle_draw = .angle - angle_to_radians(PP.angle + ANGLE_4);
   float angle_draw =  angle_to_radians(BL.angle - camera_angle - ANGLE_4);

 switch(BL.type)
 {
    case BULLET_PTORP1:
     dist = 1; // just need to put something between the case and the variable declaration

     int int_angle_draw, sprite_angle;
     int_angle_draw = BL.angle - camera_angle;
     int_angle_draw &= ANGLE_MASK;
     sprite_angle = ((int_angle_draw) >> MISSILE_ROTATION_BITSHIFT) & MISSILE_FULL_MASK;

     draw_sprite(display[d], missile_sprite [MISSILE_SPRITE_PTORP1] [sprite_angle].sprite, x - missile_sprite [MISSILE_SPRITE_PTORP1] [sprite_angle].x [0], y - missile_sprite [MISSILE_SPRITE_PTORP1] [sprite_angle].y [0]);

     if (BL.status > 0)
     {
// still priming...
      ccircle3(d, x + xpart(int_angle_draw + ANGLE_4, 6), y + ypart(int_angle_draw + ANGLE_4, 6), 2 + grand(2), 0);
      ccircle2(d, x + xpart(int_angle_draw + ANGLE_4, 8), y + ypart(int_angle_draw + ANGLE_4, 8), 4 + grand(2), 0);
      ccircle2(d, x + xpart(int_angle_draw + ANGLE_4, 11), y + ypart(int_angle_draw + ANGLE_4, 11), 2 + grand(2), 0);
     }
      else
      {
       ccircle(d, x + xpart(int_angle_draw + ANGLE_4, 6), y + ypart(int_angle_draw + ANGLE_4, 6), 3 + grand(3), 0);
       ccircle2(d, x + xpart(int_angle_draw + ANGLE_4, 9), y + ypart(int_angle_draw + ANGLE_4, 9), 7 + grand(2), 0);
       ccircle2(d, x + xpart(int_angle_draw + ANGLE_4, 12), y + ypart(int_angle_draw + ANGLE_4, 12), 3 + grand(3), 0);
      }
//     vline(display[d], x, y - 20, y + 20, COL_E5);
//     hline(display[d], x - 20, y, x + 20, COL_E5);
     break;
/*    case BULLET_AWS_MISSILE:
     dist = 1; // just need to put something between the case and the variable declaration

     int int_angle_draw2_aws, sprite_angle2_aws;
     int_angle_draw2_aws = BL.angle - camera_angle;
     int_angle_draw2_aws &= ANGLE_MASK;
     sprite_angle2_aws = ((int_angle_draw2_aws) >> MISSILE_ROTATION_BITSHIFT) & MISSILE_FULL_MASK;

     draw_sprite(display[d], missile_sprite [MISSILE_SPRITE_AF] [sprite_angle2_aws].sprite, x - missile_sprite [MISSILE_SPRITE_AF] [sprite_angle2_aws].x [0], y - missile_sprite [MISSILE_SPRITE_AF] [sprite_angle2_aws].y [0]);
     if (BL.time > 50)
     {
      ccircle(d, x + xpart(int_angle_draw2_aws + ANGLE_4, 6), y + ypart(int_angle_draw2_aws + ANGLE_4, 6), 2 + grand(3), 0);
      ccircle2(d, x + xpart(int_angle_draw2_aws + ANGLE_4, 9), y + ypart(int_angle_draw2_aws + ANGLE_4, 9), 5 + grand(2), 0);
      ccircle2(d, x + xpart(int_angle_draw2_aws + ANGLE_4, 12), y + ypart(int_angle_draw2_aws + ANGLE_4, 12), 2 + grand(3), 0);
     }
      else
      {
//       ccircle(d, x + xpart(int_angle_draw2 + ANGLE_4, 4), y + ypart(int_angle_draw2 + ANGLE_4, 4), 2 + grand(3), 0);
       ccircle2(d, x + xpart(int_angle_draw2_aws + ANGLE_4, 4), y + ypart(int_angle_draw2_aws + ANGLE_4, 4), 3 + grand(2), 0);
       ccircle2(d, x + xpart(int_angle_draw2_aws + ANGLE_4, 7), y + ypart(int_angle_draw2_aws + ANGLE_4, 7), 2 + grand(1), 0);
      }
//     vline(display[d], x, y - 20, y + 20, COL_E5);
//     hline(display[d], x - 20, y, x + 20, COL_E5);
     break;*/
    case BULLET_EAF_MISSILE:
    case BULLET_AF_MISSILE:
    case BULLET_ROCKET:
    case BULLET_HOMING:
    case BULLET_ROCKET2:
    case BULLET_LW_MISSILE:
    case BULLET_HVY_LW_MISSILE:
    case BULLET_ADV_LW_MISSILE:
    case BULLET_RAIN:
    case BULLET_AWS_MISSILE:
     dist = 1; // just need to put something between the case and the variable declaration

     int int_angle_draw2, sprite_angle2;
     int_angle_draw2 = BL.angle - camera_angle;
     int_angle_draw2 &= ANGLE_MASK;
     sprite_angle2 = ((int_angle_draw2) >> MISSILE_ROTATION_BITSHIFT) & MISSILE_FULL_MASK;

     draw_sprite(display[d], missile_sprite [MISSILE_SPRITE_AF] [sprite_angle2].sprite, x - missile_sprite [MISSILE_SPRITE_AF] [sprite_angle2].x [0], y - missile_sprite [MISSILE_SPRITE_AF] [sprite_angle2].y [0]);
/*
     ccircle(d, x + xpart(int_angle_draw2 + ANGLE_4, 6), y + ypart(int_angle_draw2 + ANGLE_4, 6), 2 + grand(3), 0);
     ccircle2(d, x + xpart(int_angle_draw2 + ANGLE_4, 9), y + ypart(int_angle_draw2 + ANGLE_4, 9), 5 + grand(2), 0);
     ccircle2(d, x + xpart(int_angle_draw2 + ANGLE_4, 12), y + ypart(int_angle_draw2 + ANGLE_4, 12), 2 + grand(3), 0);*/
//     vline(display[d], x, y - 20, y + 20, COL_E5);
//     hline(display[d], x - 20, y, x + 20, COL_E5);
     break;
    case BULLET_HROCKET:

// vline(display[d], x, y + 5, y - 5, COL_WHITE);
// hline(display[d], x-5, y, x + 5, COL_WHITE);
     dist = 1; // just need to put something between the case and the variable declaration

     int int_angle_draw3, sprite_angle3;
     int_angle_draw3 = BL.angle - camera_angle;
     int_angle_draw3 &= ANGLE_MASK;
     sprite_angle3 = ((int_angle_draw3) >> MISSILE_ROTATION_BITSHIFT) & MISSILE_FULL_MASK;

     draw_sprite(display[d], missile_sprite [MISSILE_SPRITE_AF] [sprite_angle3].sprite, x - (missile_sprite [MISSILE_SPRITE_AF] [sprite_angle3].sprite->w >> 1), y - (missile_sprite [MISSILE_SPRITE_AF] [sprite_angle3].sprite->h >> 1));
/*
     ccircle(d, x + xpart(int_angle_draw3 + ANGLE_4, 6), y + ypart(int_angle_draw3 + ANGLE_4, 6), 2 + grand(3), 0);
     ccircle2(d, x + xpart(int_angle_draw3 + ANGLE_4, 9), y + ypart(int_angle_draw3 + ANGLE_4, 9), 5 + grand(2), 0);
     ccircle2(d, x + xpart(int_angle_draw3 + ANGLE_4, 12), y + ypart(int_angle_draw3 + ANGLE_4, 12), 2 + grand(3), 0);
*/
     break;
    case BULLET_FROCK:
     dist = 1; // just need to put something between the case and the variable declaration

     int int_angle_draw4, sprite_angle4;
     int_angle_draw4 = BL.angle - camera_angle;
     int_angle_draw4 &= ANGLE_MASK;
     sprite_angle4 = ((int_angle_draw4) >> MISSILE_ROTATION_BITSHIFT) & MISSILE_FULL_MASK;

     draw_sprite(display[d], missile_sprite [MISSILE_SPRITE_PTORP1] [sprite_angle4].sprite, x - missile_sprite [MISSILE_SPRITE_PTORP1] [sprite_angle4].x [0], y - missile_sprite [MISSILE_SPRITE_PTORP1] [sprite_angle4].y [0]);
/*
      ccircle(d, x + xpart(int_angle_draw4 + ANGLE_4, 6), y + ypart(int_angle_draw4 + ANGLE_4, 6), 6 + grand(3), 0);
      ccircle3(d, x + xpart(int_angle_draw4 + ANGLE_4, 12), y + ypart(int_angle_draw4 + ANGLE_4, 12), 5 + grand(2), 0);
      ccircle2(d, x + xpart(int_angle_draw4 + ANGLE_4, 17), y + ypart(int_angle_draw4 + ANGLE_4, 17), 3 + grand(2), 0);
*/
// vline(display[d], x, y + 5, y - 5, COL_WHITE);
// hline(display[d], x-5, y, x + 5, COL_WHITE);
     break;


default:
    case BULLET_SHOT:
//     x += grand(2) - grand(2);
//     y += grand(2) - grand(2);
     ccircle(d, x, y, BL.status + grand(2), BL.colour);

//     if (.time < 20)
//      ccircle2(d, x, y, 11 - (.time >> 1), .colour);
     if (BL.time < 16)
     {
      ccircle2(d, x, y, 9 - (BL.time >> 1), BL.colour);
     }

     size = BL.status - grand(2);
     x -= fxpart(angle_draw, size+1);
     y -= fypart(angle_draw, size+1);
     ccircle2(d, x, y, size, BL.colour);

     size = BL.status - grand(2);
     if (size > 0)
     {
      x -= fxpart(angle_draw, size+1);
      y -= fypart(angle_draw, size+1);
      ccircle2(d, x, y, size, BL.colour);
     }
/*
     size = BL.status - 1 + grand(2);
     if (size > 0)
     {
      x -= fxpart(angle_draw, size+1);
      y -= fypart(angle_draw, size+1);
      ccircle2(d, x, y, size, BL.colour);
     }
*/
     break;
    case BULLET_FSHOT:
     size = BL.status + grand(2);
     ccircle(d, x, y, size, BL.colour);

     if (BL.time < 16)
     {
      ccircle2(d, x, y, 9 - (BL.time >> 1), BL.colour);
     }

     size = BL.status - grand(2) - 1;
     x -= fxpart(angle_draw, size+1);
     y -= fypart(angle_draw, size+1);
     ccircle3(d, x, y, size, BL.colour);

     size -= 1 + grand(2);
     if (size > 0)
     {
      x -= fxpart(angle_draw, size+1);
      y -= fypart(angle_draw, size+1);
      ccircle2(d, x, y, size, BL.colour);
     }

     size -= 1 + grand(2);
     if (size > 0)
     {
      x -= fxpart(angle_draw, size+1);
      y -= fypart(angle_draw, size+1);
      ccircle2(d, x, y, size, BL.colour);
     }
     break;
    case BULLET_BLAST:
     ccircle(d, x, y, BL.status + grand(2), BL.colour);

     size = 6 + grand(3);

     if (BL.time < 16)
     {
      size = 14 - (BL.time >> 1);
     }

     ccircle2(d, x, y, size, BL.colour);

     size = BL.status - grand(2);
     x -= fxpart(angle_draw, size+1);
     y -= fypart(angle_draw, size+1);
     ccircle3(d, x, y, size, BL.colour);

     size = size - 1 - grand(2);
     if (size > 0)
     {
      x -= fxpart(angle_draw, size+1);
      y -= fypart(angle_draw, size+1);
      ccircle3(d, x, y, size, BL.colour);
     }

     size = size - 1 - grand(2);
     if (size > 0)
     {
      x -= fxpart(angle_draw, size+1);
      y -= fypart(angle_draw, size+1);
      ccircle2(d, x, y, size, BL.colour);
     }

     break;
    case BULLET_ESHOT1:
     ccircle(d, x, y, 3 + grand(2), BL.colour);

     if (BL.time < 16)
     {
      ccircle2(d, x, y, 9 - (BL.time >> 1), BL.colour);
     }

     size = 3;
     x -= fxpart(angle_draw, size+1);
     y -= fypart(angle_draw, size+1);
     ccircle(d, x, y, 2, BL.colour);
//     ccircle2(d, x, y, 3, BL.colour);
     break;
    case BULLET_ESHOT2:
     ccircle(d, x, y, 4 + grand(2), BL.colour);

     if (BL.time < 16)
     {
      ccircle2(d, x, y, 9 - (BL.time >> 1), BL.colour);
     }

     size = 4;
     x -= fxpart(angle_draw, size+1);
     y -= fypart(angle_draw, size+1);
     ccircle2(d, x, y, 3 + grand(2), BL.colour);
//     ccircle2(d, x, y, 3, BL.colour);
     break;

    case BULLET_EBIGSHOT:
     ccircle(d, x, y, BL.status + grand(2), BL.colour);
     ccircle3(d, x, y, (BL.status*1.5) + grand(5), BL.colour);
     ccircle2(d, x, y, (BL.status<<1) + grand(5), BL.colour);
     ccircle3(d, x - fxpart(angle_draw, (BL.status<<1)), y - fypart(angle_draw, (BL.status<<1)), BL.status + grand(2), BL.colour);
     ccircle2(d, x - fxpart(angle_draw, (BL.status<<1)), y - fypart(angle_draw, (BL.status<<1)), (BL.status*1.2) + grand(5), BL.colour);
     if (BL.time < 16)
     {
      ccircle2(d, x, y, 16 - (BL.time >> 1), BL.colour);
     }
     break;
    case BULLET_OLDSHOT:
     ccircle(d, x, y, BL.status + grand(2), BL.colour);
//     ccircle3(d, x, y, (BL.status<<1) + grand(4), BL.colour);
//     ccircle2(d, x, y, (BL.status<<2) + grand(4), BL.colour);
     x2 = 5;//BL.status * 1.3;
     y2 = 6;//BL.status * 1.5;
     ccircle3(d, x, y, x2 + grand(5), BL.colour);
     ccircle2(d, x, y, (y2) + grand(5), BL.colour);
     ccircle3(d, x - fxpart(angle_draw, x2), y - fypart(angle_draw, x2), BL.status + grand(3), BL.colour);
//     ccircle2(d, x - fxpart(angle_draw, x2), y - fypart(angle_draw, x2), (BL.status<<1) + grand(5), BL.colour);
     ccircle2(d, x - fxpart(angle_draw, y2), y - fypart(angle_draw, y2), BL.status + grand(3), BL.colour);

     y2 += grand(4) + 2;
     x2 = grand(ANGLE_1);
     x3 = grand(4);
     ccircle2(d, x - fxpart(angle_draw, y2) + xpart(x2, x3), y - fypart(angle_draw, y2) + ypart(x2, x3), 3 + grand(4), BL.colour);
     ccircle3(d, x - fxpart(angle_draw, y2) + xpart(x2, x3), y - fypart(angle_draw, y2) + ypart(x2, x3), 1 + grand(3), BL.colour);
     y2 += grand(4) + 2;
     x2 = grand(ANGLE_1);
     x3 = grand(3);
     ccircle2(d, x - fxpart(angle_draw, y2) + xpart(x2, x3), y - fypart(angle_draw, y2) + ypart(x2, x3), 1 + grand(3), BL.colour);


//     ccircle2(d, x - fxpart(angle_draw, y2), y - fypart(angle_draw, y2), (BL.status) + grand(5), BL.colour);
/*     ccircle3(d, x - fxpart(angle_draw, (BL.status<<1)), y - fypart(angle_draw, (BL.status<<1)), BL.status + grand(2), BL.colour);
     ccircle2(d, x - fxpart(angle_draw, (BL.status<<1)), y - fypart(angle_draw, (BL.status<<1)), (BL.status<<1) + grand(5), BL.colour);
     ccircle3(d, x - fxpart(angle_draw, (BL.status<<2)), y - fypart(angle_draw, (BL.status<<2)), BL.status + grand(2), BL.colour);
     ccircle2(d, x - fxpart(angle_draw, (BL.status<<2)), y - fypart(angle_draw, (BL.status<<2)), (BL.status) + grand(5), BL.colour);*/
     if (BL.time < 32)
     {
      ccircle2(d, x, y, 16 - (BL.time >> 1), BL.colour);
     }
     break;
    case BULLET_BIGSHOT: // note: this is not the OLD2/OLD3 shot! They use OLDSHOT
//     ccircle(d, x, y, BL.status + grand(2), BL.colour);
//     ccircle3(d, x, y, (BL.status*1.5) + grand(5), BL.colour);
//     ccircle2(d, x, y, (BL.status<<1) + grand(5), BL.colour);
     ccircle(d, x, y, BL.status + grand(2), BL.colour);
     ccircle3(d, x, y, (BL.status*1.5) + grand(5), BL.colour);
//     ccircle2(d, x, y, (BL.status<<1) + grand(5), BL.colour);
     ccircle3(d, x - fxpart(angle_draw, (BL.status<<1)), y - fypart(angle_draw, (BL.status<<1)), BL.status + grand(2), BL.colour);
//     ccircle2(d, x - fxpart(angle_draw, (BL.status<<1)), y - fypart(angle_draw, (BL.status<<1)), (BL.status*1.2) + grand(5), BL.colour);
     ccircle2(d, x - fxpart(angle_draw, 4), y - fypart(angle_draw, 4), 3 + grand(4), BL.colour);
     if (BL.time < 16)
     {
//      ccircle2(d, x, y, 16 - (BL.time >> 1), BL.colour);
     }
     break;
     break;
    case BULLET_ELONGSHOT:
     if (BL.time < 20)
     {
      size = 11 - (BL.time >> 2);
      ccircle(d, x, y, size, BL.colour);
     }
      else
       size = 5 + grand(2);

     ccircle(d, x, y, size, BL.colour);

     if (BL.time < 20)
     {
      size = 16 - (BL.time >> 1);
     }
      else
       size = 7 + grand(2);

      ccircle2(d, x, y, size, BL.colour);

     size = 4;
     ccircle(d, x + fxpart(angle_draw, 4), y + fypart(angle_draw, 4), size, BL.colour);
     ccircle(d, x - fxpart(angle_draw, 4), y - fypart(angle_draw, 4), size, BL.colour);

/*     size = 6 - grand(2);
     x -= fxpart(angle_draw, size+1);
     y -= fypart(angle_draw, size+1);
     ccircle2(d, x, y, size, BL.colour);

     size -= 1 + grand(2);
     if (size > 0)
     {
      x -= fxpart(angle_draw, size+1);
      y -= fypart(angle_draw, size+1);
      ccircle2(d, x, y, size, BL.colour);
     }

     size -= 1 + grand(2);
     if (size > 0)
     {
      x -= fxpart(angle_draw, size+1);
      y -= fypart(angle_draw, size+1);
      ccircle2(d, x, y, size, BL.colour);
     }
*/
     break;



    case BULLET_ETORP1:
     ccircle(d, x, y, BL.status + grand(2), BL.colour);
     ccircle3(d, x, y, (BL.status*1.5) + grand(5), BL.colour);
     ccircle2(d, x, y, (BL.status<<1) + grand(5), BL.colour);
     break;
    case BULLET_EBEAM1:
// status2 is how many pixels apart each collision check for the beam is
// status is how long the beam is in status2s
// size is how wide the beam is
     i = 0;
     x2 = x;
     y2 = y;
//     fxpart(angle_draw, bullet[a][b].status2);
//     fypart(angle_draw, bullet[a][b].status2);
     visible = 0;

     beam_power = bullet[a][b].size >> 1;
/*     if (bullet[a][b].status > 50)
      beam_power = (70 - bullet[a][b].status) >> 1;
     if (beam_power > 10 + bullet[a][b].status3)
      beam_power = 10 + bullet[a][b].status3;*/


     if (x2 > -20 && x2 < PP.window_x + 20 && y2 > -20 && y2 < PP.window_y + 20)
     {
      ccircle(d, x2, y2, (beam_power + grand(3)), bullet[a][b].colour);
     }

// This loop ends up with x3,y3 being the beam's first appearance on screen and x2, y2 its last
     do
     {
       if (x2 > -50 && x2 < PP.window_x + 50 && y2 > -50 && y2 < PP.window_y + 50)
       {
        if (visible == 0)
        {
         x3 = x2;
         y3 = y2;
         visible = 1; // has just become visible.
        }
       }
        else
        {
         if (visible == 1)
          break; // must have been on screen for a while then left.
        }
       x2 = x + fxpart(angle_draw, bullet[a][b].status2 * i);
       y2 = y + fypart(angle_draw, bullet[a][b].status2 * i);
       i ++;
     } while (i < bullet[a][b].status);

     if (i > bullet[a][b].status - 1)
     {
      if (x2 > -20 && x2 < PP.window_x + 20 && y2 > -20 && y2 < PP.window_y + 20)
      {
       switch (bullet[a][b].status3)
       {
        default: // actually just BEAM_HIT_NOTHING
         ccircle(d, x2, y2, (beam_power + grand(3)), bullet[a][b].colour); break;
        case BEAM_HIT_SHIELD:
         ccircle(d, x2, y2, (beam_power + grand(8)), bullet[a][b].colour); break;
        case BEAM_HIT_HULL:
         ccircle(d, x2, y2, (beam_power + grand(3)), 1);
         ccircle2(d, x2, y2, ((beam_power*2) + grand(8)), 1); break;
       }
      }
     }

     if (visible)
     {
//      TRANS_MODE

      side = beam_power;
      TRANS_MODE
      poly4(display[d], x2 + fxpart(angle_draw+PI_2, side), y2 + fypart(angle_draw+PI_2, side),
                    x2 + fxpart(angle_draw-PI_2, side), y2 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw-PI_2, side), y3 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw+PI_2, side), y3 + fypart(angle_draw+PI_2, side),
                    TRANS_BLUE1);
      END_TRANS

/*      beam_power = (bullet[a][b].status - 10) >> 1;
      if (bullet[a][b].status > 50)
       beam_power = (65 - bullet[a][b].status) >> 1;
      if (beam_power > 7 + bullet[a][b].status3)
       beam_power = 7 + bullet[a][b].status3;*/

      if (beam_power > 4)
      {
       side = beam_power - 4;
       TRANS_MODE
       poly4(display[d], x2 + fxpart(angle_draw+PI_2, side), y2 + fypart(angle_draw+PI_2, side),
                    x2 + fxpart(angle_draw-PI_2, side), y2 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw-PI_2, side), y3 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw+PI_2, side), y3 + fypart(angle_draw+PI_2, side),
                    TRANS_BLUE2);
       END_TRANS
      }

/*      beam_power = (bullet[a][b].status - 20) >> 1;
      if (bullet[a][b].status > 50)
       beam_power = (60 - bullet[a][b].status) >> 1;
      if (beam_power > 5 + bullet[a][b].status3)
       beam_power = 5 + bullet[a][b].status3;*/

      if (beam_power > 8)
      {
       side = beam_power - 8;
       TRANS_MODE
       poly4(display[d], x2 + fxpart(angle_draw+PI_2, side), y2 + fypart(angle_draw+PI_2, side),
                    x2 + fxpart(angle_draw-PI_2, side), y2 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw-PI_2, side), y3 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw+PI_2, side), y3 + fypart(angle_draw+PI_2, side),
                    TRANS_BLUE3);
       END_TRANS
      }

/*      beam_power = (bullet[a][b].status - 40) >> 1;
      if (bullet[a][b].status > 50)
       beam_power = (56 - bullet[a][b].status) >> 1;
      if (beam_power > 3 + bullet[a][b].status3)
       beam_power = 3 + bullet[a][b].status3;*/

      if (beam_power > 12)
      {
       side = beam_power - 12;
       TRANS_MODE
       poly4(display[d], x2 + fxpart(angle_draw+PI_2, side), y2 + fypart(angle_draw+PI_2, side),
                    x2 + fxpart(angle_draw-PI_2, side), y2 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw-PI_2, side), y3 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw+PI_2, side), y3 + fypart(angle_draw+PI_2, side),
                    TRANS_BLUE4);
       END_TRANS

      }

//      line(display[d], x2, y2, x3, y3, COL_WHITE);
//      pline(display[d], x2, y2, x3, y3, TRANS_RED3);
//      END_TRANS
     }
     break;
/*    case BULLET_EBEAM:
     i = 0;
     x2 = x;
     y2 = y;
     fxpart(angle_draw, bullet[a][b].status2);
     fypart(angle_draw, bullet[a][b].status2);
     visible = 0;

     beam_power = bullet[a][b].status >> 1;
     if (bullet[a][b].status > 50)
      beam_power = (70 - bullet[a][b].status) >> 1;
     if (beam_power > 10 + bullet[a][b].status3)
      beam_power = 10 + bullet[a][b].status3;


     if (x2 > -20 && x2 < 660 && y2 > -20 && y2 < 500)
     {
      ccircle(d, x2, y2, (beam_power + grand(3)), bullet[a][b].colour);
     }

     do
     {
       if (x2 > -50 && x2 < 690 && y2 > -50 && y2 < 530)
       {
        if (visible == 0)
        {
         x3 = x2;
         y3 = y2;
         visible = 1; // has just become visible.
        }
       }
        else
        {
         if (visible == 1)
          break; // must have been on screen for a while then left.
        }
       x2 = x + fxpart(angle_draw, bullet[a][b].status2 * i);
       y2 = y + fypart(angle_draw, bullet[a][b].status2 * i);
       i ++;
     } while (i < bullet[a][b].status);

     if (i > bullet[a][b].status - 1)
     {
      ccircle(d, x2, y2, (beam_power + grand(3)), bullet[a][b].colour);
     }

     if (visible)
     {
//      TRANS_MODE

      side = beam_power;
      TRANS_MODE
      poly4(display[d], x2 + fxpart(angle_draw+PI_2, side), y2 + fypart(angle_draw+PI_2, side),
                    x2 + fxpart(angle_draw-PI_2, side), y2 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw-PI_2, side), y3 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw+PI_2, side), y3 + fypart(angle_draw+PI_2, side),
                    TRANS_RED1);
      END_TRANS

      beam_power = (bullet[a][b].status - 10) >> 1;
      if (bullet[a][b].status > 50)
       beam_power = (65 - bullet[a][b].status) >> 1;
      if (beam_power > 7 + bullet[a][b].status3)
       beam_power = 7 + bullet[a][b].status3;

      if (beam_power > 0)
      {
       side = beam_power;
       TRANS_MODE
       poly4(display[d], x2 + fxpart(angle_draw+PI_2, side), y2 + fypart(angle_draw+PI_2, side),
                    x2 + fxpart(angle_draw-PI_2, side), y2 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw-PI_2, side), y3 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw+PI_2, side), y3 + fypart(angle_draw+PI_2, side),
                    TRANS_RED2);
       END_TRANS
      }

      beam_power = (bullet[a][b].status - 20) >> 1;
      if (bullet[a][b].status > 50)
       beam_power = (60 - bullet[a][b].status) >> 1;
      if (beam_power > 5 + bullet[a][b].status3)
       beam_power = 5 + bullet[a][b].status3;

      if (beam_power > 0)
      {
       side = beam_power;
       TRANS_MODE
       poly4(display[d], x2 + fxpart(angle_draw+PI_2, side), y2 + fypart(angle_draw+PI_2, side),
                    x2 + fxpart(angle_draw-PI_2, side), y2 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw-PI_2, side), y3 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw+PI_2, side), y3 + fypart(angle_draw+PI_2, side),
                    TRANS_RED3);
       END_TRANS
      }

      beam_power = (bullet[a][b].status - 40) >> 1;
      if (bullet[a][b].status > 50)
       beam_power = (56 - bullet[a][b].status) >> 1;
      if (beam_power > 3 + bullet[a][b].status3)
       beam_power = 3 + bullet[a][b].status3;

      if (beam_power > 0)
      {
       side = beam_power;
       TRANS_MODE
       poly4(display[d], x2 + fxpart(angle_draw+PI_2, side), y2 + fypart(angle_draw+PI_2, side),
                    x2 + fxpart(angle_draw-PI_2, side), y2 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw-PI_2, side), y3 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw+PI_2, side), y3 + fypart(angle_draw+PI_2, side),
                    TRANS_YELLOW4);
       END_TRANS

      }

//      line(display[d], x2, y2, x3, y3, COL_WHITE);
//      pline(display[d], x2, y2, x3, y3, TRANS_RED3);
//      END_TRANS
     }
     break;
*/
 }
}
}
}


/*
void draw_bullets(int d, int p)
{

int b, size, x2, y2, x, y, x3, y3, dist, size2, i, visible, side, beam_power;
float angle, angle_draw, angle2;

for (b = 0; b < NO_BULLETS; b ++)
{

   if (bullet[a][b].type == BULLET_NONE)
    continue;

   if ((bullet[a][b].x < PP.x - (600 << 10) || bullet[a][b].x > PP.x + (600 << 10)
    || bullet[a][b].y < PP.y - (600 << 10) || bullet[a][b].y > PP.y + (600 << 10))
    && bullet[a][b].type != EBULLET_BEAM
    && bullet[a][b].type != EBULLET_BEAM2)
     continue;

   angle = atan2(bullet[a][b].y - PP.y, bullet[a][b].x - PP.x) - (PI/2) - angle_to_radians(PP.angle);
   dist = hypot(bullet[a][b].y - PP.y, bullet[a][b].x - PP.x);
   x = PP.camera_x + cos(angle) * (dist >> 10);
   y = PP.camera_y + sin(angle) * (dist >> 10);

//   float angle_draw = bullet[a][b].angle - angle_to_radians(PP.angle + ANGLE_4);
   angle_draw =  angle_to_radians(bullet[a][b].angle - PP.angle - ANGLE_4);

 switch(bullet[a][b].type)
 {
    case EBULLET_BEAM:
     i = 0;
     x2 = x;
     y2 = y;
     fxpart(angle_draw, bullet[a][b].status2);
     fypart(angle_draw, bullet[a][b].status2);
     visible = 0;

     beam_power = bullet[a][b].status >> 1;
     if (bullet[a][b].status > 50)
      beam_power = (70 - bullet[a][b].status) >> 1;
     if (beam_power > 10 + bullet[a][b].status3)
      beam_power = 10 + bullet[a][b].status3;


     if (x2 > -20 && x2 < 660 && y2 > -20 && y2 < 500)
     {
      ccircle(d, x2, y2, (beam_power + grand(3)), bullet[a][b].colour);
     }

     do
     {
       if (x2 > -50 && x2 < 690 && y2 > -50 && y2 < 530)
       {
        if (visible == 0)
        {
         x3 = x2;
         y3 = y2;
         visible = 1; // has just become visible.
        }
       }
        else
        {
         if (visible == 1)
          break; // must have been on screen for a while then left.
        }
       x2 = x + fxpart(angle_draw, bullet[a][b].status2 * i);
       y2 = y + fypart(angle_draw, bullet[a][b].status2 * i);
       i ++;
     } while (i < bullet[a][b].status);

     if (i > bullet[a][b].status - 1)
     {
      ccircle(d, x2, y2, (beam_power + grand(3)), bullet[a][b].colour);
     }

     if (visible)
     {
//      TRANS_MODE

      side = beam_power;
      TRANS_MODE
      poly4(display[d], x2 + fxpart(angle_draw+PI_2, side), y2 + fypart(angle_draw+PI_2, side),
                    x2 + fxpart(angle_draw-PI_2, side), y2 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw-PI_2, side), y3 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw+PI_2, side), y3 + fypart(angle_draw+PI_2, side),
                    TRANS_RED1);
      END_TRANS

      beam_power = (bullet[a][b].status - 10) >> 1;
      if (bullet[a][b].status > 50)
       beam_power = (65 - bullet[a][b].status) >> 1;
      if (beam_power > 7 + bullet[a][b].status3)
       beam_power = 7 + bullet[a][b].status3;

      if (beam_power > 0)
      {
       side = beam_power;
       TRANS_MODE
       poly4(display[d], x2 + fxpart(angle_draw+PI_2, side), y2 + fypart(angle_draw+PI_2, side),
                    x2 + fxpart(angle_draw-PI_2, side), y2 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw-PI_2, side), y3 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw+PI_2, side), y3 + fypart(angle_draw+PI_2, side),
                    TRANS_RED2);
       END_TRANS
      }

      beam_power = (bullet[a][b].status - 20) >> 1;
      if (bullet[a][b].status > 50)
       beam_power = (60 - bullet[a][b].status) >> 1;
      if (beam_power > 5 + bullet[a][b].status3)
       beam_power = 5 + bullet[a][b].status3;

      if (beam_power > 0)
      {
       side = beam_power;
       TRANS_MODE
       poly4(display[d], x2 + fxpart(angle_draw+PI_2, side), y2 + fypart(angle_draw+PI_2, side),
                    x2 + fxpart(angle_draw-PI_2, side), y2 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw-PI_2, side), y3 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw+PI_2, side), y3 + fypart(angle_draw+PI_2, side),
                    TRANS_RED3);
       END_TRANS
      }

      beam_power = (bullet[a][b].status - 40) >> 1;
      if (bullet[a][b].status > 50)
       beam_power = (56 - bullet[a][b].status) >> 1;
      if (beam_power > 3 + bullet[a][b].status3)
       beam_power = 3 + bullet[a][b].status3;

      if (beam_power > 0)
      {
       side = beam_power;
       TRANS_MODE
       poly4(display[d], x2 + fxpart(angle_draw+PI_2, side), y2 + fypart(angle_draw+PI_2, side),
                    x2 + fxpart(angle_draw-PI_2, side), y2 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw-PI_2, side), y3 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw+PI_2, side), y3 + fypart(angle_draw+PI_2, side),
                    TRANS_YELLOW4);
       END_TRANS

      }

//      line(display[d], x2, y2, x3, y3, COL_WHITE);
//      pline(display[d], x2, y2, x3, y3, TRANS_RED3);
//      END_TRANS
     }
     break;
    case EBULLET_BEAM2:
     i = 0;
     x2 = x;
     y2 = y;
     fxpart(angle_draw, bullet[a][b].status2);
     fypart(angle_draw, bullet[a][b].status2);
     visible = 0;

     beam_power = bullet[a][b].status >> 1;
     if (bullet[a][b].status > 50)
      beam_power = (70 - bullet[a][b].status);
     if (beam_power > 11 + bullet[a][b].status3)
      beam_power = 11 + bullet[a][b].status3;


     if (x2 > -20 && x2 < 660 && y2 > -20 && y2 < 500)
     {
      ccircle(d, x2, y2, (beam_power + grand(3)), bullet[a][b].colour);
     }

     do
     {
       if (x2 > -50 && x2 < 690 && y2 > -50 && y2 < 530)
       {
        if (visible == 0)
        {
         x3 = x2;
         y3 = y2;
         visible = 1; // has just become visible.
        }
       }
        else
        {
         if (visible == 1)
          break; // must have been on screen for a while then left.
        }
       x2 = x + fxpart(angle_draw, bullet[a][b].status2 * i);
       y2 = y + fypart(angle_draw, bullet[a][b].status2 * i);
       i ++;
     } while (i < bullet[a][b].status);

     if (i > bullet[a][b].status - 1)
     {
      ccircle(d, x2, y2, (beam_power + grand(3)), bullet[a][b].colour);
     }

     if (visible)
     {
//      TRANS_MODE

      side = beam_power;
      TRANS_MODE
      poly4(display[d], x2 + fxpart(angle_draw+PI_2, side), y2 + fypart(angle_draw+PI_2, side),
                    x2 + fxpart(angle_draw-PI_2, side), y2 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw-PI_2, side), y3 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw+PI_2, side), y3 + fypart(angle_draw+PI_2, side),
                    TRANS_BLUE1);
      END_TRANS

      beam_power = (bullet[a][b].status - 10) >> 1;
      if (bullet[a][b].status > 50)
       beam_power = (65 - bullet[a][b].status) >> 1;
      if (beam_power > 9 + bullet[a][b].status3)
       beam_power = 9 + bullet[a][b].status3;

      if (beam_power > 0)
      {
       side = beam_power;
       TRANS_MODE
       poly4(display[d], x2 + fxpart(angle_draw+PI_2, side), y2 + fypart(angle_draw+PI_2, side),
                    x2 + fxpart(angle_draw-PI_2, side), y2 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw-PI_2, side), y3 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw+PI_2, side), y3 + fypart(angle_draw+PI_2, side),
                    TRANS_BLUE2);
       END_TRANS
      }

      beam_power = (bullet[a][b].status - 20) >> 1;
      if (bullet[a][b].status > 50)
       beam_power = (60 - bullet[a][b].status) >> 1;
      if (beam_power > 7 + bullet[a][b].status3)
       beam_power = 7 + bullet[a][b].status3;

      if (beam_power > 0)
      {
       side = beam_power;
       TRANS_MODE
       poly4(display[d], x2 + fxpart(angle_draw+PI_2, side), y2 + fypart(angle_draw+PI_2, side),
                    x2 + fxpart(angle_draw-PI_2, side), y2 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw-PI_2, side), y3 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw+PI_2, side), y3 + fypart(angle_draw+PI_2, side),
                    TRANS_BLUE3);
       END_TRANS
      }

      beam_power = (bullet[a][b].status - 40) >> 1;
      if (bullet[a][b].status > 50)
       beam_power = (56 - bullet[a][b].status) >> 1;
      if (beam_power > 5 + bullet[a][b].status3)
       beam_power = 5 + bullet[a][b].status3;

      if (beam_power > 0)
      {
       side = beam_power;
       TRANS_MODE
       poly4(display[d], x2 + fxpart(angle_draw+PI_2, side), y2 + fypart(angle_draw+PI_2, side),
                    x2 + fxpart(angle_draw-PI_2, side), y2 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw-PI_2, side), y3 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw+PI_2, side), y3 + fypart(angle_draw+PI_2, side),
                    TRANS_BLUE4);
       END_TRANS

      }

//      line(display[d], x2, y2, x3, y3, COL_WHITE);
//      pline(display[d], x2, y2, x3, y3, TRANS_RED3);
//      END_TRANS
     }
     break;
/ *    case EBULLET_BEAM2:
     i = 0;
     x2 = x;
     y2 = y;
     fxpart(angle_draw, bullet[a][b].status2);
     fypart(angle_draw, bullet[a][b].status2);
     visible = 0;

     beam_power = bullet[a][b].status >> 1;
     if (beam_power > 20)
      beam_power = 20;
     if (bullet[a][b].status > 50)
      beam_power = (70 - bullet[a][b].status);


     if (x2 > -20 && x2 < 660 && y2 > -20 && y2 < 500)
     {
      ccircle(d, x2, y2, (beam_power + grand(3)), bullet[a][b].colour);
     }

     do
     {
       if (x2 > -50 && x2 < 690 && y2 > -50 && y2 < 530)
       {
        if (visible == 0)
        {
         x3 = x2;
         y3 = y2;
         visible = 1; // has just become visible.
        }
       }
        else
        {
         if (visible == 1)
          break; // must have been on screen for a while then left.
        }
       x2 = x + fxpart(angle_draw, bullet[a][b].status2 * i);
       y2 = y + fypart(angle_draw, bullet[a][b].status2 * i);
       i ++;
     } while (i < bullet[a][b].status);

     if (i > bullet[a][b].status - 1)
     {
      ccircle(d, x2, y2, (beam_power + grand(3)), bullet[a][b].colour);
     }

     if (visible)
     {
//      TRANS_MODE

      side = beam_power;
      TRANS_MODE
      poly4(display[d], x2 + fxpart(angle_draw+PI_2, side), y2 + fypart(angle_draw+PI_2, side),
                    x2 + fxpart(angle_draw-PI_2, side), y2 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw-PI_2, side), y3 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw+PI_2, side), y3 + fypart(angle_draw+PI_2, side),
                    TRANS_BLUE1);
      END_TRANS

      beam_power = (bullet[a][b].status - 10) >> 1;
      if (bullet[a][b].status > 50)
       beam_power = (65 - bullet[a][b].status) >> 1;
      if (beam_power > 13)
       beam_power = 13;

      if (beam_power > 0)
      {
       side = beam_power;
       TRANS_MODE
       poly4(display[d], x2 + fxpart(angle_draw+PI_2, side), y2 + fypart(angle_draw+PI_2, side),
                    x2 + fxpart(angle_draw-PI_2, side), y2 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw-PI_2, side), y3 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw+PI_2, side), y3 + fypart(angle_draw+PI_2, side),
                    TRANS_BLUE2);
       END_TRANS
      }

      beam_power = (bullet[a][b].status - 20) >> 1;
      if (bullet[a][b].status > 50)
       beam_power = (60 - bullet[a][b].status) >> 1;
      if (beam_power > 8)
       beam_power = 8;

      if (beam_power > 0)
      {
       side = beam_power;
       TRANS_MODE
       poly4(display[d], x2 + fxpart(angle_draw+PI_2, side), y2 + fypart(angle_draw+PI_2, side),
                    x2 + fxpart(angle_draw-PI_2, side), y2 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw-PI_2, side), y3 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw+PI_2, side), y3 + fypart(angle_draw+PI_2, side),
                    TRANS_BLUE3);
       END_TRANS
      }

      beam_power = (bullet[a][b].status - 40) >> 1;
      if (bullet[a][b].status > 50)
       beam_power = (56 - bullet[a][b].status) >> 1;
      if (beam_power > 5)
       beam_power = 5;

      if (beam_power > 0)
      {
       side = beam_power;
       TRANS_MODE
       poly4(display[d], x2 + fxpart(angle_draw+PI_2, side), y2 + fypart(angle_draw+PI_2, side),
                    x2 + fxpart(angle_draw-PI_2, side), y2 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw-PI_2, side), y3 + fypart(angle_draw-PI_2, side),
                    x3 + fxpart(angle_draw+PI_2, side), y3 + fypart(angle_draw+PI_2, side),
                    TRANS_BLUE4);
       END_TRANS

      }

//      line(display[d], x2, y2, x3, y3, COL_WHITE);
//      pline(display[d], x2, y2, x3, y3, TRANS_RED3);
//      END_TRANS
     }
     break;* /

    case EBULLET_SEEKER:
     ccircle(d, x, y, bullet[a][b].draw_size + grand(3), bullet[a][b].colour);

     if (bullet[a][b].time < 20)
      ccircle2(d, x, y, 11 - (bullet[a][b].time >> 1), bullet[a][b].colour);

//     size = bullet[a][b].draw_size + grand(2) + 1;
/ *     x -= fxpart(angle_draw, size);
     y -= fypart(angle_draw, size);
     size = bullet[a][b].draw_size2 + grand(2);
     ccircle2(d, x, y, size, bullet[a][b].colour);* /
     break;
    case EBULLET_EEL:
     ccircle(d, x, y, bullet[a][b].draw_size + grand(2), bullet[a][b].colour);
     ccircle2(d, x, y, bullet[a][b].draw_size + 2 + grand(2), bullet[a][b].colour);
     ccircle2(d, x + fxpart(angle_draw + PI_2 + PI_4, bullet[a][b].draw_size << 1), y + fypart(angle_draw + PI_2 + PI_4, bullet[a][b].draw_size << 1), (bullet[a][b].draw_size >> 1) + grand(2), bullet[a][b].colour);
     ccircle2(d, x + fxpart(angle_draw - PI_2 - PI_4, bullet[a][b].draw_size << 1), y + fypart(angle_draw - PI_2 - PI_4, bullet[a][b].draw_size << 1), (bullet[a][b].draw_size >> 1) + grand(2), bullet[a][b].colour);
     break;
    case EBULLET_FLAK:
     ccircle(d, x, y, bullet[a][b].draw_size, bullet[a][b].colour);
     if (bullet[a][b].time < 20)
      ccircle2(d, x, y, 11 - (bullet[a][b].time >> 1), bullet[a][b].colour);
//     size = bullet[a][b].draw_size + grand(2) + 1;
     angle = angle_draw;//bullet[a][b].angle;
     size = (bullet[a][b].timeout >> 1) + bullet[a][b].draw_size - 2;
     if (size > bullet[a][b].draw_size + 6)
      size = bullet[a][b].draw_size + 6;
     for (i = 0; i < 3; i ++)
     {
         angle += (PI*2)/3;
         x2 = x + fxpart(angle, size);
         y2 = y + fypart(angle, size);
         ccircle(d, x2, y2, 3, bullet[a][b].colour);
     }
     break;
    case EBULLET_FLAK2:
     ccircle(d, x, y, (bullet[a][b].timeout>>2), bullet[a][b].colour);
//     if (bullet[a][b].time < 20)
//      ccircle2(d, x, y, 11 - (bullet[a][b].time >> 1), bullet[a][b].colour);
//     size = bullet[a][b].draw_size + grand(2) + 1;
     break;
    case EBULLET_BLOB:
     if (bullet[a][b].time < 32)
      ccircle2(d, x, y, 16 - (bullet[a][b].time >> 1), bullet[a][b].colour);
     ccircle(d, x, y, bullet[a][b].draw_size, bullet[a][b].colour);
     break;

    case EBULLET_SHOT:
//     x += grand(2) - grand(2);
//     y += grand(2) - grand(2);
     ccircle(d, x, y, bullet[a][b].draw_size + grand(2), bullet[a][b].colour);

     if (bullet[a][b].time < 20)
      ccircle2(d, x, y, 11 - (bullet[a][b].time >> 1), bullet[a][b].colour);

     size = bullet[a][b].draw_size + grand(2) + 1;
     x -= fxpart(angle_draw, size);
     y -= fypart(angle_draw, size);
     size = bullet[a][b].draw_size2 + grand(2);
     ccircle2(d, x, y, size, bullet[a][b].colour);

/ *     size = 2 + grand(2);
     x -= xpart(bullet[a][b].angle, size);
     y -= ypart(bullet[a][b].angle, size);
     ccircle2(bmp, x, y, size, bullet[a][b].colour);* /
     break;
    case EBULLET_SPIN:
     ccircle(d, x, y, bullet[a][b].draw_size + grand(2), bullet[a][b].colour);

     if (bullet[a][b].time < 20)
      ccircle2(d, x, y, 11 - (bullet[a][b].time >> 1), bullet[a][b].colour);

     size = bullet[a][b].draw_size - grand(2);
     x2 = x + fxpart(angle_draw, size);
     y2 = y + fypart(angle_draw, size);
     size = bullet[a][b].draw_size2 + grand(2);
     ccircle(d, x2, y2, size, bullet[a][b].colour);
     size = bullet[a][b].draw_size - grand(2);
     x2 = x - fxpart(angle_draw, size);
     y2 = y - fypart(angle_draw, size);
     size = bullet[a][b].draw_size2 + grand(2);
     ccircle(d, x2, y2, size, bullet[a][b].colour);
     break;
    case EBULLET_SHOT2:
     ccircle(d, x, y, bullet[a][b].draw_size + grand(2), bullet[a][b].colour);

//     if (bullet[a][b].time < 20)
     ccircle2(d, x, y, bullet[a][b].draw_size + 4 + grand(4), bullet[a][b].colour);

     size = bullet[a][b].draw_size + grand(2) + 1;
     x -= fxpart(angle_draw, size);
     y -= fypart(angle_draw, size);
     size = bullet[a][b].draw_size + grand(2);
     ccircle2(d, x, y, size, bullet[a][b].colour);
     size = bullet[a][b].draw_size + grand(2) + 1;
     x -= fxpart(angle_draw, size);
     y -= fypart(angle_draw, size);
     size = bullet[a][b].draw_size2 - grand(2);
     ccircle2(d, x, y, size, bullet[a][b].colour);
     ccircle3(d, x, y, size >> 1, bullet[a][b].colour);

     break;
    case EBULLET_MISSILE_BLAST:
     size = bullet[a][b].status;
     if (size >= RLE_CCIRCLES)
      size = RLE_CCIRCLES - 1;
     switch(bullet[a][b].timeout >> 2)
     {
      case 0:
      TRANS_MODE
      switch(bullet[a][b].colour)
      {
       default:
       case 0: circlefill(display[d], x, y, size, TRANS_RED1); break;
       case 1: circlefill(display[d], x, y, size, TRANS_GREEN1); break;
       case 2: circlefill(display[d], x, y, size, TRANS_BLUE1); break;
      }
      END_TRANS
      break;
      case 1: ccircle2(d, x, y, size, bullet[a][b].colour); break;
      case 2: ccircle3(d, x, y, size, bullet[a][b].colour); break;
      default:
      case 3: ccircle(d, x, y, size, bullet[a][b].colour); break;
     }

      break;
    case EBULLET_MISSILE:
     draw_circle_white(d, x, y, bullet[a][b].draw_size);
     size = bullet[a][b].draw_size;
     x2 = x - fxpart(angle_draw, size);
     y2 = y - fypart(angle_draw, size);
     draw_circle_white(d, x2, y2, bullet[a][b].draw_size - 2);
     size = bullet[a][b].draw_size + 1;
     angle2 = 0;
     if (bullet[a][b].time < 30)
      angle2 = 0.76 - (float) bullet[a][b].time / 30;
     x2 = x - fxpart(angle_draw - PI_2 + PI_4 + angle2, size);
     y2 = y - fypart(angle_draw - PI_2 + PI_4 + angle2, size);
     draw_circle_grey(d, x2, y2, bullet[a][b].draw_size - 1);
     x2 = x - fxpart(angle_draw + PI_2 - PI_4 - angle2, size);
     y2 = y - fypart(angle_draw + PI_2 - PI_4 - angle2, size);
     draw_circle_grey(d, x2, y2, bullet[a][b].draw_size - 1);
     size2 = size + grand(4) - 1;
     if (size2 > bullet[a][b].time>>2)
      size2 = bullet[a][b].time>>2;
     if (bullet[a][b].time < 20)
      break;
     x2 = x - fxpart(angle_draw, size + 7);
     y2 = y - fypart(angle_draw, size + 7);
     ccircle2(d, x2, y2, size2, bullet[a][b].colour);
     x2 = x - fxpart(angle_draw, size + 4);
     y2 = y - fypart(angle_draw, size + 4);
     ccircle3(d, x2, y2, size2, bullet[a][b].colour);
     break;
 }
}

// debug - show parts:
#ifdef SHOW_PARTS

for (b = 0; b < NO_PARTS; b ++)
{

   if (part[b].type == PART_NONE)
    continue;

   if (part[b].x < PP.x - (400 << 10) || part[b].x > PP.x + (400 << 10)
    || part[b].y < PP.y - (400 << 10) || part[b].y > PP.y + (300 << 10))
     return;

   float angle = atan2(part[b].y - PP.y, part[b].x - PP.x) - (PI/2) - angle_to_radians(PP.angle);
   int dist = hypot(part[b].y - PP.y, part[b].x - PP.x);
   int x = PP.camera_x + cos(angle) * (dist >> 10);
   int y = PP.camera_y + sin(angle) * (dist >> 10);

//   float angle_draw = part[b].angle - angle_to_radians(PP.angle + ANGLE_4);
   float angle_draw =  angle_to_radians(part[b].angle - PP.angle - ANGLE_4);

     ccircle(d, x, y, 3 + grand(2), 1);
     print_number(x, y, b);

}
#endif

}
*/



void draw_clouds(int d, int p)
{


int c, size, i, x2, y2, angle2, size2, dist, x, y, dist2;
float angle_draw, angle, angle3;

for (c = 0; c < NO_CLOUDS; c ++)
{

   if (cloud[c].type == CLOUD_NONE)
    continue;

   if (cloud[c].x < PP.x - (600 << 10) || cloud[c].x > PP.x + (600 << 10)
    || cloud[c].y < PP.y - (600 << 10) || cloud[c].y > PP.y + (500 << 10))
     continue;

   angle = atan2(cloud[c].y - PP.y, cloud[c].x - PP.x) - (PI/2) - camera_angle_rad;
   dist = hypot(cloud[c].y - PP.y, cloud[c].x - PP.x);
   x = PP.camera_x + cos(angle) * (dist >> 10);
   y = PP.camera_y + sin(angle) * (dist >> 10);

//   float angle_draw =  angle_to_radians(.angle - PP.angle - ANGLE_4);

 switch(cloud[c].type)
 {
/*    case CLOUD_DELAY_SPARKLE:
     size = cloud[c].timeout >> 1;
     size += grand(4);
     if (size >= RLE_CCIRCLES)
      size = RLE_CCIRCLES - 1;
     ccircle(d, x, y, size, cloud[c].colour);
     break;
    case CLOUD_DELAY_SPARKLE_COL2:
     size = cloud[c].timeout >> 1;
     size += grand(4);
     if (size >= RLE_CCIRCLES)
      size = RLE_CCIRCLES - 1;
     ccircle2(d, x, y, size, cloud[c].colour);
     break;
*/
    case CLOUD_LINE_TRAIL:
     angle_draw = cloud[c].fangle - camera_angle_rad - PI_2;
     x2 = x - fxpart(angle_draw, cloud[c].status);
     y2 = y - fypart(angle_draw, cloud[c].status);
     size = (cloud[c].timeout >> 1);
     if (size > 3)
      size = 3;
     TRANS_MODE
     line(display[d], x, y, x2, y2, trans_col [2] [size]);
     END_TRANS
     break;


     case CLOUD_LINKLINE:
      angle3 = atan2(CC.y2 - PP.y, CC.x2 - PP.x) - (PI/2) - camera_angle_rad;
      dist2 = hypot(CC.y2 - PP.y, CC.x2 - PP.x);
      x2 = PP.camera_x + cos(angle3) * (dist2 >> 10);
      y2 = PP.camera_y + sin(angle3) * (dist2 >> 10);
      size = (cloud[c].timeout >> 2) + 1 + coin();
//      line(display[d], x, y, x2, y2, COL_F5);
     do_line(display[d], x, y, x2, y2, size, draw_trail_line);
     break;

         case CLOUD_SEEKER_TRAIL:
     angle_draw = cloud[c].fangle - camera_angle_rad - PI_2;
     x2 = x - fxpart(angle_draw, cloud[c].status);
     y2 = y - fypart(angle_draw, cloud[c].status);
     size = (cloud[c].timeout >> 2) + 1 + coin();
//     line(display[d], x, y, x2, y2, COL_F5);
     do_line(display[d], x, y, x2, y2, size, draw_trail_line);
     break;
    case CLOUD_SEEKER_TRAIL2:
     angle_draw = cloud[c].fangle - camera_angle_rad - PI_2;
     x2 = x - fxpart(angle_draw, cloud[c].status);
     y2 = y - fypart(angle_draw, cloud[c].status);
     size = (cloud[c].timeout >> 2) + 1 + coin();
     do_line(display[d], x, y, x2, y2, size, draw_trail_line3);
     break;
    case CLOUD_WORM_TRAIL:
     angle_draw = cloud[c].fangle - camera_angle_rad - PI_2;
     x2 = x - fxpart(angle_draw, cloud[c].status);
     y2 = y - fypart(angle_draw, cloud[c].status);
     size = (cloud[c].timeout >> 2) + 1 + coin();
//     do_line(display[d], x, y, x2, y2, size, draw_worm_trail_line);
     line(display[d], x, y, x2, y2, COL_F5);
     break;
    case CLOUD_ROCKET_TRAIL:
     angle_draw = cloud[c].fangle - camera_angle_rad - PI_2;
     x2 = x - fxpart(angle_draw, cloud[c].status);
     y2 = y - fypart(angle_draw, cloud[c].status);
     size = (cloud[c].timeout >> 2) + 1 + coin();
//     if (size > 2)
//      size = 2;
     do_line(display[d], x, y, x2, y2, size, draw_rocket_trail_line);
     break;
    case CLOUD_BURST:
     size = cloud[c].status;
     if (size >= RLE_CCIRCLES)
      size = RLE_CCIRCLES - 1;
     switch(cloud[c].timeout >> 2)
     {
      case 0:
      TRANS_MODE
      switch(cloud[c].colour)
      {
       default:
       case 0: circlefill(display[d], x, y, size, TRANS_RED1); break;
       case 1: circlefill(display[d], x, y, size, TRANS_GREEN1); break;
       case 2: circlefill(display[d], x, y, size, TRANS_BLUE1); break;
      }
      END_TRANS
      break;
      case 1: ccircle2(d, x, y, size, cloud[c].colour); break;
      case 2: ccircle3(d, x, y, size, cloud[c].colour); break;
      default:
      case 3: ccircle(d, x, y, size, cloud[c].colour); break;
     }
     break;
    case CLOUD_FADEFLARE:
     size = cloud[c].timeout >> 2;
     if (size >= RLE_CCIRCLES)
      size = RLE_CCIRCLES - 1;
     ccircle2(d, x, y, size, cloud[c].colour);
     size >>= 1;
     if (size > 0)
      ccircle3(d, x, y, size, cloud[c].colour);
     break;
    case CLOUD_FADEBALL:
     size = cloud[c].timeout >> 1;
     if (size >= RLE_CCIRCLES)
      size = RLE_CCIRCLES - 1;
     if (size < 0)
      break;

      ccircle(d, x, y, size, cloud[c].colour);
//  textprintf_ex(display[0], small_font, x + 20, y, -1, -1, "xs %i ys %i x3 %i", cloud[c].x_speed, CC.y_speed, CC.x3);

      break;
/*
     switch(cloud[c].status >> 1)
     {

      case 0:
      case 1: ccircle2(d, x, y, size, cloud[c].colour); break;
      case 2: ccircle3(d, x, y, size, cloud[c].colour); break;
      default:
      case 3: ccircle(d, x, y, size, cloud[c].colour); break;
     }
     break;*/
    case CLOUD_EX_SHIELD:
     size = cloud[c].timeout >> 1;
     if (size > CC.status)
      size = CC.status;
     ccircle3(d, x, y, size, cloud[c].colour);
     ccircle(d, x, y, size>>1, cloud[c].colour);
     x2 = CC.angle - camera_angle + ANGLE_4;
//     ccircle3(d, x + xpart(CC.angle, size>>1), y + ypart(CC.angle, size>>1), size>>1, cloud[c].colour);
     ccircle3(d, x + xpart(x2, size), y + ypart(x2, size), (size>>1)+1, cloud[c].colour);
     ccircle2(d, x + xpart(x2, size), y + ypart(x2, size), size, cloud[c].colour);
//     ccircle3(d, x + xpart(x2, size<<1), y + ypart(x2, size<<1), size, cloud[c].colour);
     break;
    case CLOUD_SYNCHFLARE:
     size = cloud[c].timeout >> 1;
     if (size >= RLE_CCIRCLES)
      size = RLE_CCIRCLES - 1;
     ccircle3(d, x, y, size, cloud[c].colour);
     break;
    case CLOUD_SYNCHBALL:
     size = cloud[c].timeout >> 1;
     if (size >= RLE_CCIRCLES)
      size = RLE_CCIRCLES - 1;
     ccircle(d, x, y, size, cloud[c].colour);
     break;
    case CLOUD_BALL:
     size = cloud[c].timeout >> 1;
     if (size >= RLE_CCIRCLES)
      size = RLE_CCIRCLES - 1;
     ccircle(d, x, y, size, cloud[c].colour);
     break;
    case CLOUD_FIRE:
     size = cloud[c].timeout >> 1;
     if (size >= RLE_CCIRCLES)
      size = RLE_CCIRCLES - 1;
     if (size > 0)
      ccircle2(d, x, y, size, cloud[c].colour);
     size = cloud[c].status >> 1;
     if (size >= RLE_CCIRCLES)
      size = RLE_CCIRCLES - 1;
     if (size > 0)
      ccircle3(d, x, y, size, cloud[c].colour);
     size = cloud[c].status2 >> 1;
     if (size >= RLE_CCIRCLES)
      size = RLE_CCIRCLES - 1;
     if (size > 0)
      ccircle(d, x, y, size, cloud[c].colour);
     break;
    case CLOUD_FIRE3:
     size = cloud[c].timeout;
     if (size >= RLE_CCIRCLES)
      size = RLE_CCIRCLES - 1;
     ccircle2(d, x, y, size, cloud[c].colour);
     size = cloud[c].status;
     if (size >= RLE_CCIRCLES)
      size = RLE_CCIRCLES - 1;
     if (size > 0)
      ccircle3(d, x, y, size, cloud[c].colour);
     break;
    case CLOUD_FIRE2:
     size = cloud[c].timeout;
     if (size >= RLE_CCIRCLES)
      size = RLE_CCIRCLES - 1;
     circlefill(display [d], x, y, size, trans_col [cloud[c].colour] [0]);
     size = cloud[c].status;
     if (size >= RLE_CCIRCLES)
      size = RLE_CCIRCLES - 1;
     if (size > 0)
      ccircle2(d, x, y, size, cloud[c].colour);
     break;
    case CLOUD_FLASH:
     size = cloud[c].status;
     if (size >= RLE_CCIRCLES)
      size = RLE_CCIRCLES - 1;
     switch((int) CC.timeout / 4)
     {
      default:
      case 3:
       ccircle(d, x, y, size, cloud[c].colour);
       break;
      case 2:
       ccircle3(d, x, y, size, cloud[c].colour);
       break;
      case 1:
       ccircle2(d, x, y, size, cloud[c].colour);
       break;
      case 0:
      TRANS_MODE
       circlefill(display[d], x, y, size, trans_col [cloud[c].colour] [0]);
       END_TRANS
       break;
     }
     break;
    case CLOUD_DOUBLE_BALL:
     size = cloud[c].timeout >> 2;
//     size += grand(3);
     if (size >= RLE_CCIRCLES)
      size = RLE_CCIRCLES - 1;
     ccircle2(d, x, y, size, cloud[c].colour);
     size -= cloud[c].status;
     if (size > 0)
      ccircle3(d, x, y, size, cloud[c].colour);
     break;
    case CLOUD_MAX_DBALL:
     size = cloud[c].timeout;
     if (size > cloud[c].status2)
      size = cloud[c].status2;
     size >>= 2;
//     size += grand(3);
     if (size >= RLE_CCIRCLES)
      size = RLE_CCIRCLES - 1;
     ccircle2(d, x, y, size, cloud[c].colour);
     size -= cloud[c].status;
     if (size > 0)
      ccircle3(d, x, y, size, cloud[c].colour);
     break;
    case CLOUD_MAX_DBALL_BRIGHT:
     size = cloud[c].timeout;
     if (size > cloud[c].status2)
      size = cloud[c].status2;
     size >>= 2;
//     size += grand(3);
     if (size >= RLE_CCIRCLES)
      size = RLE_CCIRCLES - 1;
     ccircle2(d, x, y, size, cloud[c].colour);
     size -= cloud[c].status;
     if (size > 0)
      ccircle(d, x, y, size, cloud[c].colour);
     break;
    case CLOUD_SMALL:
     size = cloud[c].timeout >> 3;
     if (size >= RLE_CCIRCLES)
      size = RLE_CCIRCLES - 1;
     ccircle(d, x, y, size, cloud[c].colour);
     break;
    case CLOUD_2BALL:
     size = cloud[c].timeout >> 1;
     if (size >= RLE_CCIRCLES)
      size = RLE_CCIRCLES - 1;
     ccircle2(d, x, y, size, cloud[c].colour);
     size = size - 2;//cloud[c].timeout >> 1;
//     if (size >= RLE_CCIRCLES)
//      size = RLE_CCIRCLES - 1;
     if (size > 0)
      ccircle3(d, x, y, size, cloud[c].colour);
     break;
    case CLOUD_BANG:
//    break;
     size = cloud[c].timeout >> 1;
     if (size >= RLE_CCIRCLES)
      size = RLE_CCIRCLES - 1;
     ccircle2(d, x, y, size, cloud[c].colour);
     size = cloud[c].timeout / 5;
     if (size >= RLE_CCIRCLES)
      size = RLE_CCIRCLES - 1;
     ccircle(d, x, y, size, cloud[c].colour);
     angle2 = cloud[c].angle - camera_angle - ANGLE_4;

     size = cloud[c].timeout / 3;
     if (size >= RLE_CCIRCLES)
      size = RLE_CCIRCLES - 1;
     size2 = cloud[c].timeout / 7;
     if (size2 >= RLE_CCIRCLES)
      size2 = RLE_CCIRCLES - 1;

//     size -= 3;
//     size2 -= 1;
     if (size < 0)
      break;
     for (i = 0; i < 7; i ++)
     {
       angle2 += ANGLE_7;
       x2 = x + xpart(angle2, cloud[c].status);
       y2 = y + ypart(angle2, cloud[c].status);
       ccircle2(d, x2, y2, size + grand(3), cloud[c].colour);
       ccircle(d, x2, y2, size2 + grand(3), cloud[c].colour);
     }
/*     size = cloud[c].timeout >> 2;
     if (size >= RLE_CCIRCLES)
      size = RLE_CCIRCLES - 1;
     size2 = cloud[c].timeout / 8;
     if (size2 >= RLE_CCIRCLES)
      size2 = RLE_CCIRCLES - 1;

     size -= 3;
     size2 -= 1;
     if (size < 0)
      break;
     for (i = 0; i < 3; i ++)
     {
       angle += ANGLE_3;
       x2 = x + xpart(angle, cloud[c].status>>2);
       y2 = y + ypart(angle, cloud[c].status>>2);
       ccircle2(d, x2, y2, size + grand(3), cloud[c].colour);
       ccircle(d, x2, y2, size2 + grand(3), cloud[c].colour);
     }*/
     break;
    case CLOUD_BALL_COL3:
     size = cloud[c].timeout >> 1;
     if (size >= RLE_CCIRCLES)
      size = RLE_CCIRCLES - 1;
     ccircle3(d, x, y, size, cloud[c].colour);
     break;
    case CLOUD_BALL_COL3_SLOW:
     size = cloud[c].timeout >> 2;
     if (size >= RLE_CCIRCLES)
      size = RLE_CCIRCLES - 1;
     ccircle3(d, x, y, size, cloud[c].colour);
     break;
    case CLOUD_BALL_COL2:
     size = cloud[c].timeout >> 1;
     if (size >= RLE_CCIRCLES)
      size = RLE_CCIRCLES - 1;
     ccircle2(d, x, y, size, cloud[c].colour);
     break;
    case CLOUD_BALL_COL2_SLOW:
     size = cloud[c].timeout >> 2;
     if (size >= RLE_CCIRCLES)
      size = RLE_CCIRCLES - 1;
     ccircle2(d, x, y, size, cloud[c].colour);
     break;
    case CLOUD_XBALL:
     size = 20 - cloud[c].timeout;
     if (size >= 20)
      size = 19;
     if (size < 0)
      size = 0;
     draw_trans_rle_sprite(display[d], RLE_xcircle [cloud[c].colour] [size], x - size - 4, y - size - 4);
     break;
    case CLOUD_SMALL_SHOCK:
//     circle(display, x, y, 5, COL_WHITE);
     size = SMALL_SHOCK_TIME - cloud[c].timeout;
     if (size >= SMALL_SHOCK_TIME)
      size = SMALL_SHOCK_TIME-1;
     if (size < 0)
      size = 0;
//     size2 = size * 3 + 12;
     if (CC.colour < 0 || CC.colour > 2)
      exit(CC.colour);
     size2 = RLE_small_shock [cloud[c].colour] [size]->w >> 1;
     draw_trans_rle_sprite(display[d], RLE_small_shock [cloud[c].colour] [size], x - size2, y - size2);
     break;
    case CLOUD_LARGE_SHOCK:
//     circle(display, x, y, 5, COL_WHITE);
     size = LARGE_SHOCK_TIME - cloud[c].timeout;
     if (size >= LARGE_SHOCK_TIME)
      size = LARGE_SHOCK_TIME-1;
     if (size < 0)
      size = 0;
//     size2 = size * 3 + 20;
     size2 = RLE_large_shock [cloud[c].colour] [size]->w >> 1;
     draw_trans_rle_sprite(display[d], RLE_large_shock [cloud[c].colour] [size], x - size2, y - size2);
     break;
    case CLOUD_HUGE_SHOCK:
     size = HUGE_SHOCK_TIME - cloud[c].timeout;
     if (size >= HUGE_SHOCK_TIME)
      size = HUGE_SHOCK_TIME-1;
     if (size < 0)
      size = 0;
//     size2 = size * 3 + 20;
     size2 = RLE_huge_shock [cloud[c].colour] [size]->w >> 1;
     draw_trans_rle_sprite(display[d], RLE_huge_shock [cloud[c].colour] [size], x - size2, y - size2);
     break;
/*    case CLOUD_HUGE_SHOCK:
     size = 49 - cloud[c].timeout;
     if (size >= 50)
      size = 49;
     if (size < 0)
      size = 0;
     draw_trans_rle_sprite(display[d], RLE_huge_shock [cloud[c].colour] [size], x - size - 51, y - size - 51);
     break;
*/
 }

}

}

void draw_trail_line(BITMAP* bmp, int x, int y, int size)
{
    static char drawing;
    drawing ^= 1;
    if (drawing & 1)
     return;

//x += grand(3) - grand(3);
//y += grand(3) - grand(3);

    ccircle2_bmp(bmp, x, y, size, 0);
TRANS_MODE
//if (size > 2)
// putpixel(bmp, x, y, TRANS_RED4);
if (size > 2)
 putpixel(bmp, x, y, TRANS_RED3);
if (size > 2)
    ccircle3_bmp(bmp, x, y, size-1, 0);

/* putpixel(bmp, x-1, y, TRANS_RED3);
 putpixel(bmp, x+1, y, TRANS_RED3);
 putpixel(bmp, x, y-1, TRANS_RED3);
 putpixel(bmp, x, y+1, TRANS_RED3);*/
/* putpixel(bmp, x-1, y-1, TRANS_RED2);
 putpixel(bmp, x+1, y+1, TRANS_RED1);
 putpixel(bmp, x+1, y-1, TRANS_RED1);
 putpixel(bmp, x-1, y+1, TRANS_RED1);*/
END_TRANS
}

void draw_trail_line3(BITMAP* bmp, int x, int y, int size)
{
    static char drawing;
    drawing ^= 1;
    if (drawing & 1)
     return;

    ccircle3_bmp(bmp, x, y, size, 0);
}

void draw_worm_trail_line(BITMAP* bmp, int x, int y, int size)
{
    static char drawing;
    drawing ^= 1;
    if (drawing & 1)
     return;

    ccircle3_bmp(bmp, x, y, size + coin(), 0);
}

void draw_rocket_trail_line(BITMAP* bmp, int x, int y, int size)
{
    static char drawing;
    drawing ^= 1;
    if (drawing & 1)
     return;

    ccircle3_bmp(bmp, x, y, size + coin(), 1);
}



void draw_pshield_circle(BITMAP *bmp, int x, int y, int strength)
{

int d = 0;
    strength += 20;

    static char size2;
    size2 = (grand(strength) >> 5) + coin();
    if (size2 < 2)
    {
     putpixel(display[d], x, y, TRANS_BLUE1);
    }
      else
      {
        if (size2 < 4)
         putpixel(display[d], x, y, TRANS_BLUE2);
          else
           putpixel(display[d], x, y, TRANS_BLUE3);
      }
//       ccircle3(d, x, y, size2 - 1, 2);
}

/*

Use this instead of Allegro's line function - it has better bounds checking.
Sometimes for some reason I get an extremely long line which slows things down.

*/
void pline(BITMAP *bmp, int x1, int y1, int x2, int y2, int colour)
{
 if (x1 < -500 || x1 > 900
     || x2 < -500 || x2 > 900
     || y2 < -500 || y2 > 900
     || y2 < -500 || y2 > 900)
      return;

 line(bmp, x1, y1, x2, y2, colour);

}

void poly4(BITMAP *target, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int col)
{

  points [0] = x1;
  points [1] = y1;
  points [2] = x2;
  points [3] = y2;
  points [4] = x3;
  points [5] = y3;
  points [6] = x4;
  points [7] = y4;

  polygon(target, 4, points, col);

}



void print_number(int x, int y, int n)
{
  textprintf_ex(display[0], small_font, x, y, -1, -1, "%i", n);
}


void draw_circle_in(int d, int x, int y, int size, char hurt)
{

   if (size > CIRCLES - 1)
    size = CIRCLES - 1;
   if (size < 0)
    size = 0;

   if (hurt)
    draw_trans_rle_sprite(display[d], circle_hurt [size], x - (size) - 1, y - (size) - 1);
     else
      draw_trans_rle_sprite(display[d], circle_in [size], x - (size) - 1, y - (size) - 1);


}

void draw_circle_out(int d, int x, int y, int size)
{

   if (size > RLE_CCIRCLES - 1)
    size = RLE_CCIRCLES - 1;
   if (size < 0)
    size = 0;
   draw_trans_rle_sprite(display[d], circle_out [size], x - (size) - 1, y - (size) - 1);

}

void draw_circle_white(int d, int x, int y, int size)
{

   if (size > RLE_CCIRCLES - 1)
    size = RLE_CCIRCLES - 1;
   if (size < 0)
    size = 0;
   draw_trans_rle_sprite(display[d], circle_white [size], x - (size) - 1, y - (size) - 1);

}

void draw_circle_grey(int d, int x, int y, int size)
{

   if (size > RLE_CCIRCLES - 1)
    size = RLE_CCIRCLES - 1;
   if (size < 0)
    size = 0;
   draw_trans_rle_sprite(display[d], circle_grey [size], x - (size) - 1, y - (size) - 1);

}


void ccircle(int d, int x, int y, int size, int colour)
{
if (size < 0)
 return;
if (size >= RLE_CCIRCLES)
 size = RLE_CCIRCLES - 1;
   draw_trans_rle_sprite(display[d], RLE_ccircle_basic [colour] [size], x - size - 1, y - size - 1);

}

void ccircle3(int d, int x, int y, int size, int colour)
{
if (size < 0)
 return;
if (size >= RLE_CCIRCLES)
 size = RLE_CCIRCLES - 1;

   draw_trans_rle_sprite(display[d], RLE_ccircle_3cols [colour] [size], x - size - 1, y - size - 1);

}

void ccircle2(int d, int x, int y, int size, int colour)
{
if (size < 0)
 return;
if (size >= RLE_CCIRCLES)
 size = RLE_CCIRCLES - 1;

   draw_trans_rle_sprite(display[d], RLE_ccircle_2cols [colour] [size], x - size - 1, y - size - 1);

}

void ccircle3_bmp(BITMAP *bmp, int x, int y, int size, int colour)
{
if (size < 0)
 return;
if (size >= RLE_CCIRCLES)
 size = RLE_CCIRCLES - 1;

   draw_trans_rle_sprite(bmp, RLE_ccircle_3cols [colour] [size], x - size - 1, y - size - 1);

}

void ccircle2_bmp(BITMAP *bmp, int x, int y, int size, int colour)
{
if (size < 0)
 return;
if (size >= RLE_CCIRCLES)
 size = RLE_CCIRCLES - 1;

   draw_trans_rle_sprite(bmp, RLE_ccircle_2cols [colour] [size], x - size - 1, y - size - 1);

}


void display_pause(int count)
{


//  vsync();

  int x1 = 300;
  int y1 = 250;
  int y2 = 350;

//  TRANS_MODE
  rectfill(display[0], x1, y1, 800 - x1, y2, COL_BOX1);
  rect(display[0], x1, y1, 800 - x1, y2, COL_BOX2);
  rectfill(display[0], x1, y1, 800 - x1, y1 + 14, COL_BOX2);
  textprintf_centre_ex(display[0], small_font, 400, y1 + 1, COL_BOX4, -1, "Paused");

/*  rect(display[0], x1 - 1, y1 - 1, 801 - x1, y2 + 1, COL_BOX2);
  rect(display[0], x1 - 2, y1 - 2, 802 - x1, y2 + 2, COL_BOX3);
  rect(display[0], x1 - 3, y1 - 3, 803 - x1, y2 + 3, COL_BOX1);*/
//  END_TRANS


  textprintf_centre_ex(display[0], small_font, 400, y1 + 30, COL_BOX4, -1, "QUIT GAME?");

  int i, col;

  for (i = 0; i < 4; i ++)
  {
   col = ((count + i * 8)/8)%4;
   switch(col)
   {
    case 3: col = COL_E1 + TRANS_RED1; break;
    case 2: col = COL_E1 + TRANS_RED2; break;
    case 1: col = COL_E2 + TRANS_RED3; break;
    case 0: col = COL_E3 + TRANS_RED4; break;
   }
   textprintf_centre_ex(display[0], small_font, 400 - 40 - i*5, y1 + 29, col, -1, ">");
   textprintf_centre_ex(display[0], small_font, 400 + 40 + i*5, y1 + 29, col, -1, "<");
  }


  textprintf_centre_ex(display[0], small_font, 400, y1 + 57, COL_BOX4, -1, "Press 'Y' to quit to main menu");
  textprintf_centre_ex(display[0], small_font, 400, y1 + 73, COL_BOX4, -1, "or 'N' to play on");


  blit(display[0], screen, 0, 0, 0, 0, 800, 600);

}


void test_old2(int number)
{


 int i;
 int x = 400;
 int y = 300;

 for (i = 0; i < WSHIP_ROTATIONS; i ++)
 {
  rectfill(screen, x - 100, y - 100, x + 100, y + 100, 0);
  draw_sprite(screen, wship_sprite [WSHIP_SPRITE_OLD2] [i].sprite, x - wship_sprite [WSHIP_SPRITE_OLD2] [i].x [0], y - wship_sprite [WSHIP_SPRITE_OLD2] [i].y [0]);
  textprintf_ex(screen, font, x - 90, y - 90, -1, -1, "%i, %i", number, i);

 }


 do
 {
  i ++;
  if (key [KEY_ESC])
   exit(10);
    } while (key [KEY_SPACE] != 0);

 do
 {
  i ++;
  if (key [KEY_ESC])
   exit(10);
 } while (key [KEY_SPACE] == 0);

}
