
#include "config.h"

#include "allegro.h"

#include <math.h>

#include "globvars.h"
#include "palette.h"
#include "stuff.h"
#include "display.h"
#include "briefing.h"

//#define TESTING_LEVEL_CALL

#ifdef TESTING_LEVEL_CALL
void level_test(void);
#endif


extern BITMAP *display [3];
extern RGB palet [256];

extern BITMAP* bsprite [2] [BSPRITES] [2]; // in briefing.c
extern BITMAP* wisprite [WISPRITES]; // in briefing.c

BITMAP *new_bitmap(int x, int y, const char errtxt []);
BITMAP *load_up_bitmap(const char fname []);
RLE_SPRITE *extract_rle_sprite(BITMAP *source, int x_source, int y_source, int x, int y);
//BITMAP *extract_bmp_sprite(BITMAP *source, int x_source, int y_source, int x, int y);
void new_bmp_struct(BITMAP *source, const char errtxt [], struct BMP_STRUCT *str, int bl);
void new_rle_struct(BITMAP *source, const char errtxt [], struct RLE_STRUCT *str, int bl);

void make_sprites(void);
BITMAP *extract_bitmap(BITMAP *source, int x_source, int y_source, int x, int y);
BITMAP *reduce_sprite(BITMAP *bmp, char do_alpha);
void prepare_circles(void);
RLE_SPRITE *extract_flip_rle_sprite(BITMAP *source, int x_source, int y_source, int x, int y, int flip_type);
BITMAP *extract_flip_bitmap(BITMAP *source, int x_source, int y_source, int x, int y, int flip_type);
void save_turret_bitmap(void);
void read_turret_bitmap(BITMAP *bmp);
void load_progress(void);

extern struct BMP_STRUCT wship_sprite [WSHIP_SPRITES] [WSHIP_ROTATIONS];
extern struct BMP_STRUCT turret_sprite [TURRET_SPRITES] [TURRET_ROTATIONS];
extern struct BMP_STRUCT fighter_sprite [FIGHTER_SPRITES] [FIGHTER_ROTATIONS];
struct BMP_STRUCT ship_collision_mask [NO_SHIP_TYPES]; // externed in bullet.c

extern struct BMP_STRUCT spulse_sprite [SHIELD_TYPES] [SHIELD_FRAMES] [SHIELD_ROTATIONS];

void announce(const char anntext []);
void make_player_sprite(BITMAP *base_bmp, int sprite, int x_pos, int y_pos, int xsize, int ysize, int size2, int x_offset, int y_offset);
void calculate_wship_rotations(int r, struct BMP_STRUCT* bmp_str, int t);
void make_wship_collision_mask(BITMAP *base_bmp, int wship, int x_pos, int y_pos, int xsize, int ysize, int type);
void make_damage_sprite(BITMAP *base_bmp, int index, int x_pos, int y_pos, int xsize, int ysize, int fix_colour);

extern RLE_SPRITE *RLE_bcircle [3] [RLE_BCIRCLES];
extern RLE_SPRITE *RLE_ccircle_basic [3] [RLE_CCIRCLES];
extern RLE_SPRITE *RLE_ccircle_3cols [3] [RLE_CCIRCLES];
extern RLE_SPRITE *RLE_ccircle_2cols [3] [RLE_CCIRCLES];
extern RLE_SPRITE *RLE_small_shock [3] [SMALL_SHOCK_TIME];
extern RLE_SPRITE *RLE_large_shock [3] [LARGE_SHOCK_TIME];
extern RLE_SPRITE *RLE_huge_shock [3] [HUGE_SHOCK_TIME];
extern RLE_SPRITE *RLE_player [PLAYER_RLES];
extern RLE_SPRITE *RLE_scircle [4] [RLE_SCIRCLES];
extern RLE_SPRITE *RLE_shield [SHIELD_RLES];

extern RLE_SPRITE *RLE_xcircle [3] [RLE_XCIRCLES];

extern RLE_SPRITE* damage_sprite [NO_DSPRITES] [DAMAGE_COLS];

extern struct BMP_STRUCT player_sprite [PLAYER_SPRITES] [PLAYER_ROTATIONS];
extern struct BMP_STRUCT missile_sprite [MISSILE_SPRITES] [MISSILE_ROTATIONS];

int rotate_pos_x [ROTATE_POS] [ANGLE_1];
int rotate_pos_y [ROTATE_POS] [ANGLE_1];

int loading_line;

#ifdef SHOW_GRAPHS
extern int graph_slack [200];
extern int graph_slack_pos;
extern int graph_fps [200];
extern int graph_fps_pos;
#endif

enum
{
FLIP_H,
FLIP_V,
FLIP_VH
};

FONT* small_font;
//extern BITMAP *interlude_screen;


int base_colour; // WARNING global variable horror

/*

Basic display init functions.

*/

/*
Call during start-up.
*/
void prepare_display(void)
{

 init_palette(-1);

/*
 display [0] = new_bitmap(640, 480, "Display");

 display [1] = create_sub_bitmap(display [0], 0, 0, 320, 480); // left screen
 display [2] = create_sub_bitmap(display [0], 320, 0, 320, 480); // right screen

 set_clip_rect(display[1], 0, 0, 320, 480);
 set_clip_rect(display[2], 0, 0, 320, 480);
*/

 display [0] = new_bitmap(800, 600, "Display");

 display [1] = create_sub_bitmap(display [0], 0, 0, 400, 600); // left screen
 display [2] = create_sub_bitmap(display [0], 400, 0, 400, 600); // right screen

 set_clip_rect(display[1], 0, 0, 400, 600);
 set_clip_rect(display[2], 0, 0, 400, 600);

 clear_to_color(display[0], COL_1);

 DATAFILE *datf = load_datafile("gfx//data.dat");
 if (datf == NULL)
 {
  set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
  allegro_message("Error: Couldn't find data.dat! \n");
  allegro_message("\n\r");
  exit(1);
 }

 font = (FONT *)datf[0].dat;
 small_font = (FONT *)datf[1].dat;

 clear_bitmap(screen);

#define LOADPIXELS 20

 textprintf_centre_ex(screen, small_font, 400, 300, COL_BOX4, -1, "Loading");
 vline(screen, 401 - LOADPIXELS, 397, 403, COL_BOX3);
 vline(screen, 400 + LOADPIXELS + 1, 397, 403, COL_BOX3);
 loading_line = 0;



#ifdef TESTING_LEVEL_CALL
level_test();
#endif



make_sprites();
prepare_circles();





//#define FIX_FONT

#ifdef FIX_FONT

int i,j;
 BITMAP *font_bitmap = load_up_bitmap("font_in.bmp");

 int back_col = getpixel(font_bitmap, 0, 0);
 int back_col2 = getpixel(font_bitmap, 3, 0);
 int out_col = getpixel(font_bitmap, 17, 4);
 int white_col = getpixel(font_bitmap, 18, 4);
 int other_col = getpixel(font_bitmap, 22, 44);
 int black_col = getpixel(font_bitmap, 1, 17);
// int lgrey_col = getpixel(font_bitmap, 8, 19);

 for (i = 0; i < font_bitmap->w; i ++)
 {
  for (j = 0; j < font_bitmap->h; j ++)
  {
   if (getpixel(font_bitmap, i, j) == back_col
       || getpixel(font_bitmap, i, j) == back_col2)
   {
       putpixel(font_bitmap, i, j, 255);
       continue;
   }
   if (getpixel(font_bitmap, i, j) == out_col)
   {
       putpixel(font_bitmap, i, j, WRITE_OUT);//COL_OUTLINE);
       continue;
   }
   if (getpixel(font_bitmap, i, j) == white_col)
   {
       putpixel(font_bitmap, i, j, WRITE_IN);//COL_WHITE);
       continue;
   }
   if (getpixel(font_bitmap, i, j) == other_col)
   {
       putpixel(font_bitmap, i, j, 245);//other colour
       continue;
   }
   if (getpixel(font_bitmap, i, j) == black_col)
   {
       putpixel(font_bitmap, i, j, 0);
       continue;
   }

   }
  }


 save_bitmap("font_out.bmp", font_bitmap, palet);
// exit(1);
#endif

#ifdef SHOW_GRAPHS

 int k;

 for (k = 0; k < 200; k ++)
 {
 graph_slack [k] = 0;
 graph_fps [k] = 0;
 }

 graph_slack_pos = 0;
 graph_fps_pos = 0;

#endif


 load_progress();

// the number at the end is the index for the control colour for the turret.
// So ships with one engine probably start at 2, 2 engines 3 etc.
 calculate_wship_rotations(RP_OLD2_ENGINE_1, wship_sprite [WSHIP_SPRITE_OLD2], 1);
 calculate_wship_rotations(RP_OLD2_TURRET_1, wship_sprite [WSHIP_SPRITE_OLD2], 2);
 calculate_wship_rotations(RP_OLD2_TURRET_2, wship_sprite [WSHIP_SPRITE_OLD2], 3);

 calculate_wship_rotations(RP_OLD3_ENGINE_1, wship_sprite [WSHIP_SPRITE_OLD3], 1);
 calculate_wship_rotations(RP_OLD3_TURRET_1, wship_sprite [WSHIP_SPRITE_OLD3], 2);
 calculate_wship_rotations(RP_OLD3_TURRET_2, wship_sprite [WSHIP_SPRITE_OLD3], 3);
 calculate_wship_rotations(RP_OLD3_TURRET_3, wship_sprite [WSHIP_SPRITE_OLD3], 4);

 calculate_wship_rotations(RP_FRIEND3_ENGINE_1, wship_sprite [WSHIP_SPRITE_FRIEND3], 1);
 calculate_wship_rotations(RP_FRIEND3_ENGINE_2, wship_sprite [WSHIP_SPRITE_FRIEND3], 2);
 calculate_wship_rotations(RP_FRIEND3_TURRET_1, wship_sprite [WSHIP_SPRITE_FRIEND3], 3);
 calculate_wship_rotations(RP_FRIEND3_TURRET_2, wship_sprite [WSHIP_SPRITE_FRIEND3], 4);
 calculate_wship_rotations(RP_FRIEND3_TURRET_3, wship_sprite [WSHIP_SPRITE_FRIEND3], 5);

 load_progress();

 calculate_wship_rotations(RP_SCOUT2_ENGINE_1, wship_sprite [WSHIP_SPRITE_SCOUT2], 1);
 calculate_wship_rotations(RP_SCOUT2_ENGINE_1, wship_sprite [WSHIP_SPRITE_SCOUT2], 2);
 calculate_wship_rotations(RP_SCOUT2_TURRET_1, wship_sprite [WSHIP_SPRITE_SCOUT2], 3);
 calculate_wship_rotations(RP_SCOUT2_TURRET_2, wship_sprite [WSHIP_SPRITE_SCOUT2], 4);

 calculate_wship_rotations(RP_SCOUT3_ENGINE_1, wship_sprite [WSHIP_SPRITE_SCOUT3], 1);
 calculate_wship_rotations(RP_SCOUT3_ENGINE_1, wship_sprite [WSHIP_SPRITE_SCOUT3], 2);
 calculate_wship_rotations(RP_SCOUT3_ENGINE_1, wship_sprite [WSHIP_SPRITE_SCOUT3], 3);
 calculate_wship_rotations(RP_SCOUT3_TURRET_1, wship_sprite [WSHIP_SPRITE_SCOUT3], 4);
 calculate_wship_rotations(RP_SCOUT3_TURRET_2, wship_sprite [WSHIP_SPRITE_SCOUT3], 5);
 calculate_wship_rotations(RP_SCOUT3_TURRET_3, wship_sprite [WSHIP_SPRITE_SCOUT3], 6);

 calculate_wship_rotations(RP_SCOUTCAR_ENGINE_1, wship_sprite [WSHIP_SPRITE_SCOUTCAR], 1);
 calculate_wship_rotations(RP_SCOUTCAR_TURRET_1, wship_sprite [WSHIP_SPRITE_SCOUTCAR], 2);

 calculate_wship_rotations(RP_EBASE_TURRET_1, wship_sprite [WSHIP_SPRITE_EBASE], 1);
 calculate_wship_rotations(RP_EBASE_TURRET_2, wship_sprite [WSHIP_SPRITE_EBASE], 2);
 calculate_wship_rotations(RP_EBASE_TURRET_3, wship_sprite [WSHIP_SPRITE_EBASE], 3);
 calculate_wship_rotations(RP_EBASE_TURRET_4, wship_sprite [WSHIP_SPRITE_EBASE], 4);
 calculate_wship_rotations(RP_EBASE_TURRET_5, wship_sprite [WSHIP_SPRITE_EBASE], 5);

 load_progress();

}


void calculate_wship_rotations(int r, struct BMP_STRUCT* bmp_str, int t)
{
 int a;
 int sprite_angle;

 int sprite_angle_flip;


 for (a = 0; a < ANGLE_1; a ++)
 {
  sprite_angle = ((a + ANGLE_4) >> WSHIP_ROTATION_BITSHIFT) & WSHIP_FULL_MASK;

  if (sprite_angle >= WSHIP_ROTATIONS)
  {
   if (sprite_angle >= WSHIP_ROTATIONS * 3)
   { // 2000-3000
     sprite_angle_flip = (WSHIP_ROTATIONS * 4) - sprite_angle - 1;
     sprite_angle_flip &= WSHIP_ROTATION_MASK;
     rotate_pos_x [r] [a] = bmp_str [sprite_angle_flip].x [t] << 10;
     rotate_pos_y [r] [a] = bmp_str [sprite_angle_flip].y [t] << 10;
     rotate_pos_x [r] [a] *= -1;
//     rotate_pos_y [r] [a] *= -1;
   }
    else
    {
       if (sprite_angle >= WSHIP_ROTATIONS * 2)
       { // 1000-2000
         sprite_angle_flip = sprite_angle - (WSHIP_ROTATIONS * 2);
         sprite_angle_flip &= WSHIP_ROTATION_MASK;
         rotate_pos_x [r] [a] = bmp_str [sprite_angle_flip].x [t] << 10;
         rotate_pos_y [r] [a] = bmp_str [sprite_angle_flip].y [t] << 10;
         rotate_pos_x [r] [a] *= -1;
         rotate_pos_y [r] [a] *= -1;
       }
        else
        { // 0 - 1024:
          sprite_angle_flip = (WSHIP_ROTATIONS * 0) - sprite_angle - 1;
          sprite_angle_flip &= WSHIP_ROTATION_MASK;
          rotate_pos_x [r] [a] = bmp_str [sprite_angle_flip].x [t] << 10;
          rotate_pos_y [r] [a] = bmp_str [sprite_angle_flip].y [t] << 10;
          rotate_pos_y [r] [a] *= -1;
        }
    }

   }
   else
   { // 3000-4000
      sprite_angle = (WSHIP_ROTATIONS) + sprite_angle;
      sprite_angle &= WSHIP_ROTATION_MASK;
      rotate_pos_x [r] [a] = bmp_str [sprite_angle].x [t] << 10;
      rotate_pos_y [r] [a] = bmp_str [sprite_angle].y [t] << 10;
   }

 }

/*  if (sprite_angle >= WSHIP_ROTATIONS)
  {
   if (sprite_angle >= WSHIP_ROTATIONS * 3)
   {
     sprite_angle_flip = (WSHIP_ROTATIONS * 4) - sprite_angle - 1;
     sprite_angle_flip &= WSHIP_ROTATION_MASK;
     rotate_pos_x [r] [a] = bmp_str [sprite_angle_flip].x [t] << 10;
     rotate_pos_y [r] [a] = bmp_str [sprite_angle_flip].y [t] << 10;
     rotate_pos_x [r] [a] *= -1;
     rotate_pos_y [r] [a] *= -1;
   }
    else
    {
       if (sprite_angle >= WSHIP_ROTATIONS * 2)
       {
         sprite_angle_flip = sprite_angle - (WSHIP_ROTATIONS * 2);
         sprite_angle_flip &= WSHIP_ROTATION_MASK;
         rotate_pos_x [r] [a] = bmp_str [sprite_angle_flip].x [t] << 10;
         rotate_pos_y [r] [a] = bmp_str [sprite_angle_flip].y [t] << 10;
         rotate_pos_x [r] [a] *= -1;
         rotate_pos_y [r] [a] *= -1;
       }
        else
        {
          sprite_angle_flip = (WSHIP_ROTATIONS * 2) - sprite_angle - 1;
          sprite_angle_flip &= WSHIP_ROTATION_MASK;
          rotate_pos_x [r] [a] = bmp_str [sprite_angle_flip].x [t] << 10;
          rotate_pos_y [r] [a] = bmp_str [sprite_angle_flip].y [t] << 10;
        }
    }

   }
   else
   {
      sprite_angle &= WSHIP_ROTATION_MASK;
      rotate_pos_x [r] [a] = bmp_str [sprite_angle].x [t] << 10;
      rotate_pos_y [r] [a] = bmp_str [sprite_angle].y [t] << 10;
   }

 }*/

}



void prepare_circles(void)
{


 int beam_colours [3] [4] =
 {
  {TRANS_RED1, TRANS_RED2, TRANS_RED3, TRANS_YELLOW4},
  {TRANS_GREEN1, TRANS_GREEN2, TRANS_GREEN3, TRANS_GREEN4},
  {TRANS_BLUE1, TRANS_BLUE2, TRANS_BLUE3, TRANS_BLUE4}
 };
// int cx, cy, ci, cs, ca, cd, cj, cn;

 int i, j;

 BITMAP *temp_bitmap;

 int rad;

 for (i = 0; i < RLE_CCIRCLES; i ++)
 {
  temp_bitmap = new_bitmap((i + 1) * 2, (i + 1) * 2, "prepare ccircles");
  clear_bitmap(temp_bitmap);
  for (j = 0; j < 3; j ++)
  {

   rad = i;
   circlefill(temp_bitmap, i + 1, i + 1, rad, beam_colours [j] [0]);
   rad --;//*= 0.90;
   if (rad > 0)
    circlefill(temp_bitmap, i + 1, i + 1, rad, beam_colours [j] [1]);

   RLE_ccircle_2cols [j] [i] = get_rle_sprite(temp_bitmap);

   rad --;//*= 0.90;
   if (rad > 0)
    circlefill(temp_bitmap, i + 1, i + 1, rad, beam_colours [j] [2]);

   RLE_ccircle_3cols [j] [i] = get_rle_sprite(temp_bitmap);

   rad --;//*= 0.90;
   if (rad > 0)
    circlefill(temp_bitmap, i + 1, i + 1, rad, beam_colours [j] [3]);

   RLE_ccircle_basic [j] [i] = get_rle_sprite(temp_bitmap);
  }

  destroy_bitmap(temp_bitmap);
 }

 load_progress();

 for (i = 0; i < RLE_SCIRCLES; i ++)
 {
  temp_bitmap = new_bitmap((i + 1) * 2, (i + 1) * 2, "prepare scircles");
  clear_bitmap(temp_bitmap);
  for (j = 0; j < 4; j ++)
  {

   rad = i;
   circlefill(temp_bitmap, i + 1, i + 1, rad, beam_colours [1] [0]);
   if (j > 0)
   {
   rad --;//*= 0.90;
   if (rad > 0)
    circlefill(temp_bitmap, i + 1, i + 1, rad, beam_colours [1] [1]);
   if (j > 1)
   {
   rad --;//*= 0.90;
   if (rad > 0)
    circlefill(temp_bitmap, i + 1, i + 1, rad, beam_colours [1] [2]);
   if (j > 2)
   {
   rad --;
   if (rad > 0)
    circlefill(temp_bitmap, i + 1, i + 1, rad, beam_colours [1] [3]);
   }
   rad --;
   if (rad > 0)
    circlefill(temp_bitmap, i + 1, i + 1, rad, beam_colours [1] [2]);
   }
   rad --;
   if (rad > 0)
    circlefill(temp_bitmap, i + 1, i + 1, rad, beam_colours [1] [1]);
   }
   rad --;
   if (rad > 0)
    circlefill(temp_bitmap, i + 1, i + 1, rad, beam_colours [1] [0]);
   rad --;
   if (rad > 0)
    circlefill(temp_bitmap, i + 1, i + 1, rad, 0);

   RLE_scircle [j] [i] = get_rle_sprite(temp_bitmap);
  }

   destroy_bitmap(temp_bitmap);
 }

 load_progress();

 int rad2;
/*
 for (i = 0; i < 30; i ++)
 {
  rad = i + 15; // see below
  temp_bitmap = new_bitmap((rad + 1) * 2, (rad + 1) * 2, "prepare small_shock");
  clear_bitmap(temp_bitmap);
  for (j = 0; j < 3; j ++)
  {
   rad = i + 15; // see above
   rad2 = rad;

   circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [0]);
   if (i < 22)
   {
    rad2 --;
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [2]);
   }

   if (i < 22)
   {
    rad2 --;
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [3]);
   }
   if (i < 24)
   {
    rad2 --;
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [2]);
   }
   if (i < 26)
   {
    rad2 --;
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [1]);
   }
   if (i < 28)
   {
    rad2 -= 2;
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [0]);
   }

   rad2 -= 2;

   if (rad2 > -1)
   circlefill(temp_bitmap, rad + 1, rad + 1, rad2, 0);

   RLE_small_shock [j] [i] = get_rle_sprite(temp_bitmap);
  }

  destroy_bitmap(temp_bitmap);
 }

*/

 for (i = 0; i < 15; i ++)
 {
  rad = (i*3) + 15; // see below
  temp_bitmap = new_bitmap((rad + 1) * 2, (rad + 1) * 2, "prepare small_shock");
  clear_bitmap(temp_bitmap);
  for (j = 0; j < 3; j ++)
  {
   rad = (i*3) + 15; // see above
   rad2 = rad;

   circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [0]);

   if (i < 13)
   {
    rad2 -= 1;
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [1]);
   }
   if (i < 11)
   {
    rad2 -= 1;
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [2]);
   }
   if (i < 9)
   {
    rad2 -= 1;
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [3]);
   }

   rad2 = i * 4;
   if (i < 9 && rad2 > 0)
   {
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [2]);
    rad2 -= 2;
   }
   if (i < 11 && rad2 > 0)
   {
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [1]);
    rad2 -= 2;
   }
   if (i < 13 && rad2 > 0)
   {
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [0]);
    rad2 -= 2;
   }
   if (rad2 > 0)
   circlefill(temp_bitmap, rad + 1, rad + 1, rad2, 0);

   RLE_small_shock [j] [i] = get_rle_sprite(temp_bitmap);
  }

  destroy_bitmap(temp_bitmap);
 }

 load_progress();

 for (i = 0; i < 20; i ++)
 {
  rad = (i*3) + 20; // see below
  temp_bitmap = new_bitmap((rad + 1) * 2, (rad + 1) * 2, "prepare large_shock");
  clear_bitmap(temp_bitmap);
  for (j = 0; j < 3; j ++)
  {
   rad = (i*3) + 20; // see above
   rad2 = rad;

   circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [0]);
/*   if (i < 42)
   {
    rad2 --;
    if (i < 32)
     rad2 --;
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [2]);
   }
*/

   if (i < 18)
   {
    rad2 -= 1;
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [1]);
   }
   if (i < 16)
   {
    rad2 -= 1;
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [2]);
   }
   if (i < 14)
   {
    rad2 -= 1;
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [3]);
   }

   rad2 = i * 4;

   if (i < 14 && rad2 > 0)
   {
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [2]);
    rad2 -= 2;
   }
   if (i < 16 && rad2 > 0)
   {
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [1]);
    rad2 -= 2;
   }
   if (i < 18 && rad2 > 0)
   {
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [0]);
    rad2 -= 2;
   }
   if (rad2 > 0)
   circlefill(temp_bitmap, rad + 1, rad + 1, rad2, 0);

   RLE_large_shock [j] [i] = get_rle_sprite(temp_bitmap);
  }

  destroy_bitmap(temp_bitmap);
 }

 load_progress();

 for (i = 0; i < 40; i ++)
 {

  rad = (i*3) + 45; // see below
  temp_bitmap = new_bitmap((rad + 1) * 2, (rad + 1) * 2, "prepare huge_shock");
  clear_bitmap(temp_bitmap);
  for (j = 0; j < 3; j ++)
  {
   rad = (i*3) + 40; // see above
   rad2 = rad;

   circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [0]);

   if (i < 36)
   {
    rad2 -= 1;
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [1]);
   }
   if (i < 32)
   {
    rad2 -= 1;
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [2]);
   }
   if (i < 28)
   {
    rad2 -= 1;
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [3]);
   }

   rad2 = i * 4;

   if (i < 28 && rad2 > 0)
   {
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [2]);
    rad2 -= 2;
   }
   if (i < 32 && rad2 > 0)
   {
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [1]);
    rad2 -= 2;
   }
   if (i < 36 && rad2 > 0)
   {
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [0]);
    rad2 -= 2;
   }
   if (rad2 > 0)
   circlefill(temp_bitmap, rad + 1, rad + 1, rad2, 0);

   RLE_huge_shock [j] [i] = get_rle_sprite(temp_bitmap);


  }

  destroy_bitmap(temp_bitmap);
 }

 load_progress();


/*
 for (i = 0; i < 50; i ++)
 {
  rad = i + 25; // see below
  temp_bitmap = new_bitmap((rad + 1) * 2, (rad + 1) * 2, "prepare large_shock");
  clear_bitmap(temp_bitmap);
  for (j = 0; j < 3; j ++)
  {
   rad = i + 25; // see above
   rad2 = rad;

   circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [0]);
   if (i < 42)
   {
    rad2 --;
    if (i < 32)
     rad2 --;
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [2]);
   }

   if (i < 42)
   {
    rad2 --;
    if (i < 36)
     rad2 --;
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [3]);
   }
   if (i < 44)
   {
    rad2 --;
    if (i < 38)
     rad2 --;
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [2]);
   }
   if (i < 46)
   {
    rad2 --;
    if (i < 40)
     rad2 --;
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [1]);
   }
   if (i < 48)
   {
    rad2 -= 2;
    if (i < 42)
     rad2 -= 2;
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [0]);
   }

   rad2 -= 2;
    if (i < 44)
     rad2 -= 2;

   if (rad2 > -1)
   circlefill(temp_bitmap, rad + 1, rad + 1, rad2, 0);

   RLE_large_shock [j] [i] = get_rle_sprite(temp_bitmap);
  }

  destroy_bitmap(temp_bitmap);
 }

*/
/*
 for (i = 0; i < 50; i ++)
 {
  rad = i + 50; // see below
  temp_bitmap = new_bitmap((rad + 1) * 2, (rad + 1) * 2, "prepare huge_shock");
  clear_bitmap(temp_bitmap);
  for (j = 0; j < 3; j ++)
  {
   rad = i + 50; // see above
   rad2 = rad;

   circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [0]);
   if (i < 42)
   {
    rad2 --;
    if (i < 32)
     rad2 --;
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [2]);
   }

   if (i < 42)
   {
    rad2 --;
    if (i < 36)
     rad2 --;
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [3]);
   }
   if (i < 44)
   {
    rad2 --;
    if (i < 38)
     rad2 --;
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [2]);
   }
   if (i < 46)
   {
    rad2 --;
    if (i < 40)
     rad2 --;
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [1]);
   }
   if (i < 48)
   {
    rad2 -= 2;
    if (i < 42)
     rad2 -= 2;
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [0]);
   }

   rad2 -= 2;
    if (i < 44)
     rad2 -= 2;

   if (rad2 > -1)
   circlefill(temp_bitmap, rad + 1, rad + 1, rad2, 0);

   RLE_huge_shock [j] [i] = get_rle_sprite(temp_bitmap);
  }

  destroy_bitmap(temp_bitmap);
 }*/
/*
 int inner_colour;

 for (i = 0; i < 20; i ++)
 {
  rad = i + 4; // see below
  temp_bitmap = new_bitmap((rad + 1) * 2, (rad + 1) * 2, "prepare xcircle");
  clear_bitmap(temp_bitmap);
  for (j = 0; j < 3; j ++)
  {
   rad = i + 4; // see above
   rad2 = rad;

   inner_colour = 0;

   circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [0]);
   if (i < 16)
   {
    rad2 --;
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [1]);
    inner_colour = beam_colours [j] [0];
   }
   if (i < 12)
   {
    rad2 --;
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [2]);
    inner_colour = beam_colours [j] [1];
   }
   if (i < 8)
   {
    rad2 --;
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [3]);
    inner_colour = beam_colours [j] [2];
   }
   if (i < 4)
   {
    rad2 --;
    circlefill(temp_bitmap, rad + 1, rad + 1, rad2, beam_colours [j] [4]);
    inner_colour = beam_colours [j] [3];
   }

// now make xcircle disappear from within:
   rad2 = (i % 4) * rad2;
   rad2 /= 4;

   circlefill(temp_bitmap, rad + 1, rad + 1, rad2, inner_colour);

   RLE_xcircle [j] [i] = get_rle_sprite(temp_bitmap);
  }

  destroy_bitmap(temp_bitmap);
 }

*/

}



/*
Safe wrapper for the Allegro create_bitmap function.
*/
BITMAP *new_bitmap(int x, int y, const char errtxt [])
{

 BITMAP *bmp = create_bitmap(x, y);
 if (bmp == NULL)
 {
  set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
  allegro_message("Bitmap creation failed!  \n%s\n\n\r%s", allegro_error, errtxt);
  exit(1);
 }

 return bmp;

}



BITMAP *load_up_bitmap(const char fname [])
{
 RGB temp_palette [256];

 BITMAP *temp_bitmap = load_bitmap(fname, temp_palette);
 if (temp_bitmap == NULL)
 {
  set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
  allegro_message("Failed to load in bitmap! (File not found?)  \n%s", fname);
  exit(1);
 }

 return temp_bitmap;

}


RLE_SPRITE *extract_rle_sprite(BITMAP *source, int x_source, int y_source, int x, int y)
{

 BITMAP *tmp = new_bitmap(x, y, "extract_rle_sprite");

 blit(source, tmp, x_source, y_source, 0, 0, x, y);

 RLE_SPRITE *retval = get_rle_sprite(tmp);

 if (retval == NULL)
 {
  set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
  allegro_message("Rle_sprite extraction failed!  \n%s\n\n\r%s", allegro_error, "extract_rle_sprite");
  exit(1);
 }

 destroy_bitmap(tmp);

 return retval;

}


BITMAP *extract_bitmap(BITMAP *source, int x_source, int y_source, int x, int y)
{

 BITMAP *tmp = new_bitmap(x, y, "extract_bitmap");

 if (tmp== NULL)
 {
  set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
  allegro_message("Bitmap extraction failed!  \n%s\n\n\r%s", allegro_error, "extract_bitmap");
  exit(1);
 }

 blit(source, tmp, x_source, y_source, 0, 0, x, y);

 return tmp;

}





void new_rle_struct(BITMAP *source, const char errtxt [], struct RLE_STRUCT *str, int bl)
{


 int x1 = -1, y1 = -1, x2 = -1, y2 = -1;

 int i, j;

 for (i = 0; i < source->w; i ++)
 {
  for (j = 0; j < source->h; j ++)
  {
    if (getpixel(source, i, j) != 0)
    {
     x1 = i;
     break;
    }
  }
  if (x1 != -1)
   break;
 }

  for (j = 0; j < source->h; j ++)
  {
   for (i = 0; i < source->w; i ++)
   {
    if (getpixel(source, i, j) != 0)
    {
     y1 = j;
     break;
    }
  }
  if (y1 != -1)
   break;
 }

 for (i = source->w - 1; i >= 0; i --)
 {
  for (j = 0; j < source->h; j ++)
  {
    if (getpixel(source, i, j) != 0)
    {
     x2 = i;
     break;
    }
  }
  if (x2 != -1)
   break;
 }


  for (j = source->h - 1; j >= 0; j --)
  {
   for (i = 0; i < source->w; i ++)
   {
    if (getpixel(source, i, j) != 0)
    {
     y2 = j;
     break;
    }
  }
  if (y2 != -1)
   break;
 }



 BITMAP *tmp = new_bitmap(x2 - x1 + 1, y2 - y1 + 1, "new_rle_struct temp bitmap");

 blit(source, tmp, x1, y1, 0, 0, x2 - x1 + 1, y2 - y1 + 1);

// circle(tmp, tmp->w / 2, tmp->h / 2, 2, 1);

 //rect(tmp, 0, 0, tmp->w-1, tmp->h-1, 15);

 if (bl == -1)
 {
   str->sprite = get_rle_sprite(tmp);
//   str->x = tmp->w / 2;//source->w / 2 - x1;
//   str->y = tmp->h / 2;//source->h / 2 - y1;
   str->x = tmp->w / 2;//source->w / 2 - x1;
   str->y = tmp->h / 2;//source->h / 2 - y1;
 }
  else
  {
   if (bl == -2)
   {
     str->sprite = get_rle_sprite(tmp);
//   str->x = tmp->w / 2;//source->w / 2 - x1;
//   str->y = tmp->h / 2;//source->h / 2 - y1;
     str->x = source->w / 2 - x1;
     str->y = source->h / 2 - y1;
   }
    else
    {
     str [bl].sprite = get_rle_sprite(tmp);
//     str [bl].x = tmp->w / 2; //source->w / 2 - x1;
//     str [bl].y = tmp->h / 2; //source->h / 2 - y1;
     str [bl].x = source->w / 2 - x1;
     str [bl].y = source->h / 2 - y1;
    }
  }

 destroy_bitmap(tmp);
}



void new_bmp_struct(BITMAP *source, const char errtxt [], struct BMP_STRUCT *str, int bl)
{


 int x1 = -1, y1 = -1, x2 = -1, y2 = -1;

 int i, j;

 for (i = 0; i < source->w; i ++)
 {
  for (j = 0; j < source->h; j ++)
  {
    if (getpixel(source, i, j) != 0)
    {
     x1 = i;
     break;
    }
  }
  if (x1 != -1)
   break;
 }

  for (j = 0; j < source->h; j ++)
  {
   for (i = 0; i < source->w; i ++)
   {
    if (getpixel(source, i, j) != 0)
    {
     y1 = j;
     break;
    }
  }
  if (y1 != -1)
   break;
 }

 for (i = source->w - 1; i >= 0; i --)
 {
  for (j = 0; j < source->h; j ++)
  {
    if (getpixel(source, i, j) != 0)
    {
     x2 = i;
     break;
    }
  }
  if (x2 != -1)
   break;
 }


  for (j = source->h - 1; j >= 0; j --)
  {
   for (i = 0; i < source->w; i ++)
   {
    if (getpixel(source, i, j) != 0)
    {
     y2 = j;
     break;
    }
  }
  if (y2 != -1)
   break;
 }



 BITMAP *tmp = new_bitmap(x2 - x1 + 1, y2 - y1 + 1, "new_bmp_struct temp bitmap");

 blit(source, tmp, x1, y1, 0, 0, x2 - x1 + 1, y2 - y1 + 1);

// circle(tmp, tmp->w / 2, tmp->h / 2, 2, 1);

 //rect(tmp, 0, 0, tmp->w-1, tmp->h-1, 15);

 if (bl == -1)
 {
   str->sprite = tmp;
//   str->x = tmp->w / 2;//source->w / 2 - x1;
//   str->y = tmp->h / 2;//source->h / 2 - y1;
   str->x [0] = tmp->w / 2;//source->w / 2 - x1;
   str->y [0] = tmp->h / 2;//source->h / 2 - y1;
 }
  else
  {
   if (bl == -2)
   {
     str->sprite = tmp;
//   str->x = tmp->w / 2;//source->w / 2 - x1;
//   str->y = tmp->h / 2;//source->h / 2 - y1;
     str->x [0] = source->w / 2 - x1;
     str->y [0] = source->h / 2 - y1;
   }
    else
    {
     str [bl].sprite = tmp;
//     str [bl].x = tmp->w / 2; //source->w / 2 - x1;
//     str [bl].y = tmp->h / 2; //source->h / 2 - y1;
     str [bl].x [0] = source->w / 2 - x1;
     str [bl].y [0] = source->h / 2 - y1;
    }
  }

// destroy_bitmap(tmp);
}



void announce(const char anntext [])
{

 static int row = 0;

 textprintf_ex(screen, font, 10, 20 + row * 12, COL_14, -1, anntext);

 row ++;

}

RLE_SPRITE *extract_flip_rle_sprite(BITMAP *source, int x_source, int y_source, int x, int y, int flip_type)
{

 BITMAP *tmp = new_bitmap(x, y, "extract_hflip_rle_sprite");
 BITMAP *tmp2 = new_bitmap(x, y, "extract_hflip_rle_sprite");

 clear_bitmap(tmp2);
 blit(source, tmp, x_source, y_source, 0, 0, x, y);

 switch(flip_type)
 {
  case FLIP_H: draw_sprite_h_flip(tmp2, tmp, 0, 0); break;
  case FLIP_V: draw_sprite_v_flip(tmp2, tmp, 0, 0); break;
  case FLIP_VH: draw_sprite_vh_flip(tmp2, tmp, 0, 0); break;
 }


 RLE_SPRITE *retval = get_rle_sprite(tmp2);

 if (retval == NULL)
 {
  set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
  allegro_message("Rle_sprite extraction failed!  \n%s\n\n\r%s", allegro_error, "extract_rle_sprite");
  exit(1);
 }

 destroy_bitmap(tmp);
 destroy_bitmap(tmp2);

 return retval;

}


BITMAP *extract_flip_bitmap(BITMAP *source, int x_source, int y_source, int x, int y, int flip_type)
{

 BITMAP *tmp = new_bitmap(x, y, "extract_flip_bitmap");
 BITMAP *tmp2 = new_bitmap(x, y, "extract_flip_bitmap2");

 clear_bitmap(tmp2);
 blit(source, tmp, x_source, y_source, 0, 0, x, y);

 switch(flip_type)
 {
  case FLIP_H: draw_sprite_h_flip(tmp2, tmp, 0, 0); break;
  case FLIP_V: draw_sprite_v_flip(tmp2, tmp, 0, 0); break;
  case FLIP_VH: draw_sprite_vh_flip(tmp2, tmp, 0, 0); break;
 }


 destroy_bitmap(tmp);

 return tmp2;

}


/*

*****************************************************************************************8

******************************************************************************************88

*/




















//BITMAP *new_bitmap(int x, int y, const char errtxt []);
//BITMAP *load_up_bitmap(const char fname []);
//RLE_SPRITE *extract_rle_sprite(BITMAP *source, int x_source, int y_source, int x, int y);
void new_rle_struct(BITMAP *source, const char errtxt [], struct RLE_STRUCT *str, int bl);
void new_bmp_struct(BITMAP *source, const char errtxt [], struct BMP_STRUCT *str, int bl);
void new_wship_bmp_struct(BITMAP *source, const char errtxt [], struct BMP_STRUCT *str, int bl);
void new_turret_bmp_struct(BITMAP *source, const char errtxt [], struct BMP_STRUCT *str, int bl);
void new_shield_bmp_struct(BITMAP *source, const char errtxt [], struct BMP_STRUCT *str, int bl);
void new_fighter_bmp_struct(BITMAP *source, const char errtxt [], struct BMP_STRUCT *str, int bl);
void new_player_bmp_struct(BITMAP *source, const char errtxt [], struct BMP_STRUCT *str, int bl);
void new_missile_bmp_struct(BITMAP *source, const char errtxt [], struct BMP_STRUCT *str, int bl);
BITMAP *reduce_player_sprite(BITMAP *bmp, char do_alpha);

void make_wship_sprite(BITMAP *base_bmp, int wship, int wship_index, int x_pos, int y_pos, int xsize, int ysize, int size2, int x_offset, int y_offset, int type);
void make_turret_sprite(BITMAP *base_bmp, int turret, int x_pos, int y_pos, int xsize, int ysize, int size2, int x_offset, int y_offset);
void make_shield_sprite(BITMAP *base_bmp, int type, int frame, int x_pos, int y_pos, int xsize, int ysize, int size2, int x_offset, int y_offset);
void make_turret_sprite_pivot(BITMAP *base_bmp, int turret, int x_pos, int y_pos, int xsize, int ysize, int size2, int x_offset, int y_offset, int pivot_x, int pivot_y);
void make_fighter_sprite(BITMAP *base_bmp, int fighter, int fighter_index, int x_pos, int y_pos, int xsize, int ysize, int size2, int x_offset, int y_offset);
void make_missile_sprite(BITMAP *base_bmp, int turret, int x_pos, int y_pos, int xsize, int ysize, int size2, int x_offset, int y_offset);
BITMAP *extract_bitmap(BITMAP *source, int x_source, int y_source, int x, int y);
BITMAP *reduce_sprite(BITMAP *bmp, char do_alpha);
BITMAP *reduce_wship_sprite(BITMAP *bmp, char do_alpha, int wship, int rotation);
BITMAP *reduce_turret_sprite(BITMAP *bmp, char do_alpha);
BITMAP *reduce_shield_sprite(BITMAP *bmp, char do_alpha);
BITMAP *reduce_fighter_sprite(BITMAP *bmp, char do_alpha);
BITMAP *reduce_missile_sprite(BITMAP *bmp, char do_alpha);
//BITMAP *reduce_fighter_sprite(BITMAP *bmp, char do_alpha, int fighter, int rotation);

int control_colour [BMPS_VALUES];
int control_x [BMPS_VALUES];
int control_y [BMPS_VALUES];
// assumes 0 is centre of sprite. Other values can be anything (eg turret locations or drive exhaust)

int control_trans_colour [BMPS_TRANS_VALUES];
int control_trans_x [BMPS_TRANS_VALUES];
int control_trans_y [BMPS_TRANS_VALUES];

/*int turret_pivot_x;
int turret_pivot_y;

int wship_pivot_x;
int wship_pivot_y;
int fighter_pivot_x;
int fighter_pivot_y;*/

/*

Basic display init functions.

*/



void make_sprites(void)
{

 BITMAP *base_bmp = load_up_bitmap("gfx//test.bmp");

 int i, x, y, width, j;

 for (i = 0; i < BMPS_VALUES; i ++)
 {
  control_colour [i] = getpixel(base_bmp, COL_CONTROL_BASE + i, 0);
  set_control_blend(control_colour [i], i);
 }

 TRANS_MODE
 rectfill(base_bmp, 0, 0, base_bmp->w, base_bmp->h, TRANS_CONTROL);
 END_TRANS

//  control_colour [0] = getpixel(base_bmp, 16, 0);

base_colour = COL_F1;

 make_wship_sprite(base_bmp, WSHIP_SPRITE_OLD2, WSHIP_OLD2, 1, 1, 263, 218, 310, 36, 36, SHIP_OLD2);

// for each wship, collision_mask must come after make_wship_sprite
 make_wship_collision_mask(base_bmp, WSHIP_SPRITE_OLD2, 301, 78, 69, 86, SHIP_OLD2);
// if a different value is used for offset (currently 35), must change the offset (35/3) in m_w_c_m!!
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_OLD2_1, 320, 1, 23, 20, -1);
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_OLD2_2, 318, 22, 27, 20, -1);
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_OLD2_3, 347, 1, 9, 22, -1);
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_OLD2_4, 307, 42, 49, 12, 3);

 load_progress();

 make_wship_sprite(base_bmp, WSHIP_SPRITE_OLD3, WSHIP_OLD3, 1, 222, 297, 291, 350, 36, 1, SHIP_OLD3);
// for each wship, collision_mask must come after make_wship_sprite
 make_wship_collision_mask(base_bmp, WSHIP_SPRITE_OLD3, 311, 176, 73, 113, SHIP_OLD3);
// if a different value is used for offset (currently 35), must change the offset (35/3) in m_w_c_m!!
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_OLD3_1, 410, 180, 23, 16, -1);
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_OLD3_2, 407, 197, 29, 15, -1);
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_OLD3_3, 407, 213, 29, 17, -1);
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_OLD3_4, 387, 181, 15, 33, -1);
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_OLD3_5, 437, 183, 53, 33, 3);

 load_progress();

 make_wship_sprite(base_bmp, WSHIP_SPRITE_FRIEND3, WSHIP_FRIEND3, 12, 524, 297, 300, 390, 36, 36, SHIP_FRIEND3);
// for each wship, collision_mask must come after make_wship_sprite
 make_wship_collision_mask(base_bmp, WSHIP_SPRITE_FRIEND3, 310, 510, 116, 116, SHIP_FRIEND3);
// if a different value is used for offset (currently 35), must change the offset (35/3) in m_w_c_m!!
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_FRIEND3_1, 338, 632, 74, 35, -1);
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_FRIEND3_2, 326, 667, 98, 37, -1);
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_FRIEND3_3, 341, 715, 68, 36, -1);
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_FRIEND3_4, 427, 647, 54, 67, -1);
  make_damage_sprite(base_bmp, DSPRITE_WSHIP_FRIEND3_5, 427, 647, 54, 67, -1);

 load_progress();

// make_wship_collision_mask(base_bmp, WSHIP_SPRITE_OLD3, 301, 78, 69, 86, SHIP_OLD3);
// if a different value is used for offset (currently 35), must change the offset (35/3) in m_w_c_m!!

base_colour = COL_F2; // use 2 value for trans sprites


 RGB temp_palette [256];

 BITMAP *turret_bitmap = load_bitmap("gfx//s_turret.bmp", temp_palette);
 if (turret_bitmap == NULL)
 {

 make_turret_sprite(base_bmp, TURRET_SPRITE_BASIC_1, 348, 927, 31, 36, 43, 6, 6);
 make_turret_sprite(base_bmp, TURRET_SPRITE_BASIC_2, 380, 927, 31, 36, 43, 6, 6);
 make_turret_sprite(base_bmp, TURRET_SPRITE_BASIC_3, 412, 927, 31, 36, 43, 6, 6);
 make_turret_sprite(base_bmp, TURRET_SPRITE_BASIC_4, 444, 927, 31, 36, 43, 6, 6);
 make_turret_sprite(base_bmp, TURRET_SPRITE_BASIC_5, 476, 927, 31, 36, 43, 6, 6);

 make_turret_sprite(base_bmp, TURRET_SPRITE_HEAVY_1, 318, 1002, 49, 69, 80, 12, 6);
 make_turret_sprite(base_bmp, TURRET_SPRITE_HEAVY_2, 368, 1002, 49, 69, 80, 12, 6);
 make_turret_sprite(base_bmp, TURRET_SPRITE_HEAVY_3, 418, 1002, 49, 69, 80, 12, 6);
 make_turret_sprite(base_bmp, TURRET_SPRITE_HEAVY_4, 318, 1072, 49, 69, 80, 12, 6);
 make_turret_sprite(base_bmp, TURRET_SPRITE_HEAVY_5, 368, 1072, 49, 69, 80, 12, 6);

// make_turret_sprite(base_bmp, TURRET_SPRITE_HEAVY_1, 325, 939, 41, 58, 70, 12, 6);
// make_turret_sprite(base_bmp, TURRET_SPRITE_HEAVY_2, 367, 939, 41, 58, 70, 12, 6);
// make_turret_sprite(base_bmp, TURRET_SPRITE_HEAVY_3, 409, 939, 41, 58, 70, 12, 6);


// make_turret_sprite(base_bmp, TURRET_SPRITE_HEAVY_2, 365, 780, 53, 59, 73, 15, 6);
// make_turret_sprite(base_bmp, TURRET_SPRITE_HEAVY_3, 419, 777, 53, 62, 76, 15, 6);

/* make_turret_sprite(base_bmp, TURRET_SPRITE_HEAVY_1, 311, 783, 53, 56, 70, 15, 6);
 make_turret_sprite(base_bmp, TURRET_SPRITE_HEAVY_2, 365, 780, 53, 59, 73, 15, 6);
 make_turret_sprite(base_bmp, TURRET_SPRITE_HEAVY_3, 419, 777, 53, 62, 76, 15, 6);*/
/*
 make_turret_sprite(base_bmp, TURRET_SPRITE_HEAVY_1, 322, 295, 41, 62, 70, 15, 6);
 make_turret_sprite(base_bmp, TURRET_SPRITE_HEAVY_2, 364, 295, 41, 65, 73, 15, 6);
 make_turret_sprite(base_bmp, TURRET_SPRITE_HEAVY_3, 406, 295, 41, 68, 76, 15, 6);*/

base_colour = COL_E2; // make sure to use 2 value for any transparent enemy sprites!

 make_turret_sprite(base_bmp, TURRET_SPRITE_EBASIC_1, 395, 41, 33, 41, 55, 12, 6);
 make_turret_sprite(base_bmp, TURRET_SPRITE_EBASIC_2, 431, 42, 35, 41, 55, 12, 6);
 make_turret_sprite(base_bmp, TURRET_SPRITE_EBASIC_3, 470, 42, 39, 42, 55, 12, 6);
 make_turret_sprite(base_bmp, TURRET_SPRITE_EBASIC_4, 372, 84, 45, 42, 60, 12, 6);
 make_turret_sprite(base_bmp, TURRET_SPRITE_EBASIC_5, 418, 84, 45, 42, 60, 12, 6);
 make_turret_sprite(base_bmp, TURRET_SPRITE_EBASIC_6, 464, 84, 45, 42, 60, 12, 6);

 make_turret_sprite(base_bmp, TURRET_SPRITE_EHEAVY_1, 299, 376, 47, 63, 75, 12, 6);
 make_turret_sprite(base_bmp, TURRET_SPRITE_EHEAVY_2, 347, 377, 51, 64, 75, 12, 6);
 make_turret_sprite(base_bmp, TURRET_SPRITE_EHEAVY_3, 399, 376, 55, 66, 78, 12, 6);
 make_turret_sprite(base_bmp, TURRET_SPRITE_EHEAVY_4, 455, 374, 55, 68, 81, 12, 6);

 make_turret_sprite(base_bmp, TURRET_SPRITE_ELONG_1, 313, 756, 35, 52, 62, 12, 6);
 make_turret_sprite(base_bmp, TURRET_SPRITE_ELONG_2, 349, 756, 37, 54, 65, 12, 6);
 make_turret_sprite(base_bmp, TURRET_SPRITE_ELONG_3, 387, 756, 41, 55, 68, 12, 6);
 make_turret_sprite(base_bmp, TURRET_SPRITE_ELONG_4, 429, 756, 49, 54, 72, 12, 6);

 make_turret_sprite(base_bmp, TURRET_SPRITE_EANTI_1, 312, 818, 45, 49, 62, 12, 6);
 make_turret_sprite(base_bmp, TURRET_SPRITE_EANTI_2, 358, 818, 45, 49, 62, 12, 6);
 make_turret_sprite(base_bmp, TURRET_SPRITE_EANTI_3, 404, 818, 47, 51, 64, 12, 6);
 make_turret_sprite(base_bmp, TURRET_SPRITE_EANTI_4, 452, 818, 51, 51, 66, 12, 6);
 make_turret_sprite(base_bmp, TURRET_SPRITE_EANTI_5, 315, 873, 57, 51, 68, 12, 6);

 make_turret_sprite(base_bmp, TURRET_SPRITE_EBEAM_1, 10, 861, 61, 72, 90, 12, 6);
 make_turret_sprite(base_bmp, TURRET_SPRITE_EBEAM_2, 73, 861, 67, 72, 90, 12, 6);
 make_turret_sprite(base_bmp, TURRET_SPRITE_EBEAM_3, 141, 861, 71, 73, 92, 12, 6);
 make_turret_sprite(base_bmp, TURRET_SPRITE_EBEAM_4, 214, 861, 75, 74, 94, 12, 6);

 save_turret_bitmap();

 }
  else
  {
    read_turret_bitmap(turret_bitmap);
  }

 destroy_bitmap(base_bmp);

 load_progress();

base_colour = COL_E1;


 base_bmp = load_up_bitmap("gfx//ewship.bmp");

 for (i = 0; i < BMPS_VALUES; i ++)
 {
  control_colour [i] = getpixel(base_bmp, COL_CONTROL_BASE + i, 0);
  set_control_blend(control_colour [i], i);
 }

 TRANS_MODE
 rectfill(base_bmp, 0, 0, base_bmp->w, base_bmp->h, TRANS_CONTROL);
 END_TRANS

 make_wship_sprite(base_bmp, WSHIP_SPRITE_SCOUT2, WSHIP_SCOUT2, 1, 1, 177, 216, 280, 60, 36, SHIP_SCOUT2);
// for each wship, collision_mask must come after make_wship_sprite
 make_wship_collision_mask(base_bmp, WSHIP_SPRITE_SCOUT2, 248, 3, 73, 74, SHIP_SCOUT2);
// if a different value is used for offset (currently 35), must change the offset (35/3) in m_w_c_m!!
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_SCOUT2_1, 322, 0, 40, 22, -1);
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_SCOUT2_2, 323, 23, 38, 16, -1);
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_SCOUT2_3, 363, 1, 16, 28, -1);
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_SCOUT2_4, 324, 40, 40, 15, 3);

 load_progress();

/*
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_SCOUT2_1, 326, 3, 59, 32, -1);
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_SCOUT2_2, 327, 36, 57, 25, -1);
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_SCOUT2_3, 385, 2, 23, 43, -1);
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_SCOUT2_4, 326, 65, 57, 22, 3);*/

 make_wship_sprite(base_bmp, WSHIP_SPRITE_SCOUT3, WSHIP_SCOUT3, 1, 219, 321, 350, 420, 51, 36, SHIP_SCOUT3);
// for each wship, collision_mask must come after make_wship_sprite
 make_wship_collision_mask(base_bmp, WSHIP_SPRITE_SCOUT3, 212, 83, 111, 110, SHIP_SCOUT3);
// if a different value is used for offset (currently 35), must change the offset (35/3) in m_w_c_m!!
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_SCOUT3_1, 386, 2, 41, 15, -1);
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_SCOUT3_2, 384, 18, 45, 15, -1);
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_SCOUT3_3, 389, 34, 35, 16, -1);
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_SCOUT3_4, 430, 4, 21, 33, -1);
  make_damage_sprite(base_bmp, DSPRITE_WSHIP_SCOUT3_5, 381, 51, 51, 24, 3);

 load_progress();

 make_wship_sprite(base_bmp, WSHIP_SPRITE_SCOUTCAR, WSHIP_SCOUTCAR, 1, 571, 221, 340, 400, 81, 36, SHIP_SCOUTCAR);
// for each wship, collision_mask must come after make_wship_sprite
 make_wship_collision_mask(base_bmp, WSHIP_SPRITE_SCOUTCAR, 341, 205, 83, 115, SHIP_SCOUTCAR);
// if a different value is used for offset (currently 35), must change the offset (35/3) in m_w_c_m!!
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_SCOUTCAR_1, 453, 34, 29, 14, -1);
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_SCOUTCAR_2, 453, 4, 29, 29, -1);
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_SCOUTCAR_3, 483, 15, 13, 17, -1);
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_SCOUTCAR_4, 452, 49, 31, 15, 3);

 load_progress();

 destroy_bitmap(base_bmp);




 base_bmp = load_up_bitmap("gfx//ebase.bmp");

 for (i = 0; i < BMPS_VALUES; i ++)
 {
  control_colour [i] = getpixel(base_bmp, COL_CONTROL_BASE + i, 0);
  set_control_blend(control_colour [i], i);
 }

 TRANS_MODE
 rectfill(base_bmp, 0, 0, base_bmp->w, base_bmp->h, TRANS_CONTROL);
 END_TRANS

// make_wship_sprite(base_bmp, WSHIP_SPRITE_EBASE, WSHIP_EBASE, 133, 589, 339, 607, 650, 162, 24, SHIP_EBASE);
 make_wship_sprite(base_bmp, WSHIP_SPRITE_EBASE, WSHIP_EBASE, 1, 1, 357, 661, 700, 168, 12, SHIP_EBASE);
// for each wship, collision_mask must come after make_wship_sprite
 make_wship_collision_mask(base_bmp, WSHIP_SPRITE_EBASE, 360, 3, 136, 236, SHIP_EBASE);
// if a different value is used for offset (currently 35), must change the offset (35/3) in m_w_c_m!!
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_EBASE_1, 504, 1, 27, 13, -1);
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_EBASE_2, 504, 64, 27, 13, -1);
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_EBASE_3, 502, 15, 31, 13, -1);
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_EBASE_4, 502, 50, 31, 13, -1);
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_EBASE_5, 499, 29, 37, 20, -1);
 make_damage_sprite(base_bmp, DSPRITE_WSHIP_EBASE_6, 537, 1, 9, 20, -1);


 destroy_bitmap(base_bmp);

 load_progress();




 base_bmp = load_up_bitmap("gfx//small.bmp");

  for (i = 0; i < BMPS_VALUES; i ++)
 {
  control_colour [i] = getpixel(base_bmp, COL_CONTROL_BASE + i, 0);
  set_control_blend(control_colour [i], i);
 }

// extract the briefing sprites before we fix the whole bitmap's colours:
 for (i = 0; i < BSPRITES; i ++)
 {
   bsprite [0] [i] [0] = extract_bitmap(base_bmp, 241 + i * 14, 1, 13, 13);
   for (x = 0; x < 13; x ++)
   {
    for (y = 0; y < 13; y ++)
    {
     j = getpixel(bsprite [0] [i] [0], x, y);
     if (j == control_colour [0]) putpixel(bsprite [0] [i] [0], x, y, TRANS_BLUE3);
     if (j == control_colour [1]) putpixel(bsprite [0] [i] [0], x, y, TRANS_BLUE1);
     if (j == control_colour [2]) putpixel(bsprite [0] [i] [0], x, y, TRANS_BLUE4);
    }
   }
   bsprite [0] [i] [1] = extract_bitmap(base_bmp, 241 + i * 14, 1, 13, 13);
   for (x = 0; x < 13; x ++)
   {
    for (y = 0; y < 13; y ++)
    {
     j = getpixel(bsprite [0] [i] [1], x, y);
     if (j == control_colour [0]) putpixel(bsprite [0] [i] [1], x, y, TRANS_RED2);
     if (j == control_colour [1]) putpixel(bsprite [0] [i] [1], x, y, TRANS_RED1);
     if (j == control_colour [2]) putpixel(bsprite [0] [i] [1], x, y, TRANS_RED3);
    }
   }
   bsprite [1] [i] [0] = extract_bitmap(base_bmp, 241 + i * 14, 15, 13, 13);
   for (x = 0; x < 13; x ++)
   {
    for (y = 0; y < 13; y ++)
    {
     j = getpixel(bsprite [1] [i] [0], x, y);
     if (j == control_colour [0]) putpixel(bsprite [1] [i] [0], x, y, TRANS_RED3);
     if (j == control_colour [1]) putpixel(bsprite [1] [i] [0], x, y, TRANS_RED1);
    }
   }
   bsprite [1] [i] [1] = extract_bitmap(base_bmp, 241 + i * 14, 15, 13, 13);
   for (x = 0; x < 13; x ++)
   {
    for (y = 0; y < 13; y ++)
    {
     j = getpixel(bsprite [1] [i] [1], x, y);
     if (j == control_colour [0]) putpixel(bsprite [1] [i] [1], x, y, TRANS_RED2);
     if (j == control_colour [1]) putpixel(bsprite [1] [i] [1], x, y, TRANS_RED1);
    }
   }
 }

 load_progress();

 int k;

 for (i = 0; i < WISPRITES; i ++)
 {
  wisprite [i] = extract_bitmap(base_bmp, 219 + i * 31, 36, 30, 30);
  for (x = 0; x < 30; x ++)
  {
   for (y = 0; y < 30; y ++)
   {
    j = getpixel(wisprite [i], x, y);
    for (k = 0; k < 7; k ++)
    {
     if (j == control_colour [k])
     {
      putpixel(wisprite [i], x, y, COL_F1 + k);
      break;
     }
    }
   }
  }
 }




 TRANS_MODE
 rectfill(base_bmp, 0, 0, base_bmp->w, base_bmp->h, TRANS_CONTROL);
 END_TRANS



 y = 2;
 x = 2;

base_colour = COL_F1;

// first row of player_sprite - each of the 3 fin positions for cannon rest position
 width = 63;
// make_player_sprite(base_bmp, PLAYER_SPRITE_1_1, x, y, width, 60, 95, 6, 6);
 make_player_sprite(base_bmp, PLAYER_SPRITE_1_1, x, y, width, 60, 95, 16, 16);
 x += width + 1;

 width = 69;
 make_player_sprite(base_bmp, PLAYER_SPRITE_1_2, x, y, width, 59, 95, 16, 16);
 x += width + 1;

 width = 75;
 make_player_sprite(base_bmp, PLAYER_SPRITE_1_3, x, y, width, 57, 95, 16, 16);

 y += 61; // this is how much we need to move downwards to get to the top useful pixel of next row.
 x = 2;

// second row of player_sprite - each of the 3 fin positions for cannon mid position

 width = 63;
 make_player_sprite(base_bmp, PLAYER_SPRITE_2_1, x, y, width, 60, 95, 16, 16);
 x += width + 1;

 width = 69;
 make_player_sprite(base_bmp, PLAYER_SPRITE_2_2, x, y, width, 59, 95, 16, 16);
 x += width + 1;

 width = 75;
 make_player_sprite(base_bmp, PLAYER_SPRITE_2_3, x, y, width, 57, 95, 16, 16);

 y += 61; // this is how much we need to move downwards to get to the top useful pixel of next row.
 x = 2;

// load_progress();

// third row of player_sprite - each of the 3 fin positions for cannon mid position

 width = 63;
 make_player_sprite(base_bmp, PLAYER_SPRITE_3_1, x, y, width, 60, 95, 16, 16);
 x += width + 1;

 width = 69;
 make_player_sprite(base_bmp, PLAYER_SPRITE_3_2, x, y, width, 59, 95, 16, 16);
 x += width + 1;

 width = 75;
 make_player_sprite(base_bmp, PLAYER_SPRITE_3_3, x, y, width, 57, 95, 16, 16);

 y += 63; // this is how much we need to move downwards to get to the top useful pixel of next row.
 x = 2;

base_colour = COL_E1;

 width = 64;
// make_fighter_sprite(base_bmp, FIGHTER_SPRITE_BASIC_1, FIGHTER_BASIC, x, y, width, 57, 75, 6, 6);

// make_fighter_sprite(base_bmp, FIGHTER_SPRITE_BASIC_1, FIGHTER_BASIC, 2, 187, 47, 56, 68, 6, 6);
 make_fighter_sprite(base_bmp, FIGHTER_SPRITE_BASIC_1, FIGHTER_BASIC, 435, 77, 53, 55, 68, 6, 6);
 make_fighter_sprite(base_bmp, FIGHTER_SPRITE_BOMBER_1, FIGHTER_BOMBER, 736, 49, 95, 75, 125, 6, 6);
// make_fighter_sprite(base_bmp, FIGHTER_SPRITE_BOMBER_1, FIGHTER_BOMBER, 50, 187, 81, 66, 95, 6, 6);
 make_fighter_sprite(base_bmp, FIGHTER_SPRITE_ESCOUT_1, FIGHTER_BASIC, 208, 182, 71, 53, 85, 6, 6);
 make_fighter_sprite(base_bmp, FIGHTER_SPRITE_EINT_1, FIGHTER_EINT, 280, 182, 73, 78, 95, 6, 6);

 load_progress();

 base_colour = COL_F1;
 make_fighter_sprite(base_bmp, FIGHTER_SPRITE_FRIEND_1, FIGHTER_FRIEND, 132, 187, 75, 57, 91, 6, 6);

// make_missile_sprite(base_bmp, MISSILE_SPRITE_PTORP1, 213, 4, 7, 10, 14, 0, 0);
 make_missile_sprite(base_bmp, MISSILE_SPRITE_PTORP1, 216, 4, 12, 27, 35, 12, 6);
 make_missile_sprite(base_bmp, MISSILE_SPRITE_AF, 230, 6, 7, 14, 30, 6, 6);

/*
 width = 68;
 make_fighter_sprite(base_bmp, FIGHTER_SPRITE_BASIC_1, FIGHTER_BASIC, x, y, width, 60, 75, 6, 6);
 x += width + 1;

 width = 72;
 make_fighter_sprite(base_bmp, FIGHTER_SPRITE_BASIC_2, FIGHTER_BASIC, x, y, width, 60, 80, 6, 6);
 x += width + 1;

 width = 76;
 make_fighter_sprite(base_bmp, FIGHTER_SPRITE_BASIC_3, FIGHTER_BASIC, x, y, width, 60, 85, 6, 6);
 x += width + 1;

 width = 80;
 make_fighter_sprite(base_bmp, FIGHTER_SPRITE_BASIC_4, FIGHTER_BASIC, x, y, width, 60, 90, 6, 6);
 x += width + 1;
*/
// width = 36;
 make_damage_sprite(base_bmp, DSPRITE_FIGHTER_1, 34, 277, 23, 23, -1);
 make_damage_sprite(base_bmp, DSPRITE_BOMBER_1, 0, 276, 33, 24, -1);
 make_damage_sprite(base_bmp, DSPRITE_FRIEND_1, 58, 276, 29, 24, -1);
 make_damage_sprite(base_bmp, DSPRITE_ESCOUT_1, 133, 250, 35, 24, -1);
 make_damage_sprite(base_bmp, DSPRITE_EINT_1, 169, 249, 32, 31, -1);

 load_progress();

 destroy_bitmap(base_bmp);



 base_bmp = load_up_bitmap("gfx//trans.bmp");

  for (i = 0; i < BMPS_TRANS_VALUES; i ++)
 {
  control_trans_colour [i] = getpixel(base_bmp, i, 0);
  set_control_trans_blend(control_trans_colour [i], i);
 }

 TRANS_MODE
 rectfill(base_bmp, 0, 0, base_bmp->w, base_bmp->h, TRANS_CONTROL_TRANS);
 END_TRANS
/*
 y = 1;
 width = 17;
 make_shield_sprite(base_bmp, SPULSE_1, 0, 1, y, 177, width, 200, 16, 19);
 y += width;

 width = 12;
 make_shield_sprite(base_bmp, SPULSE_1, 1, 1, y, 165, width, 190, 13, 19);
 y += width;

 width = 9;
 make_shield_sprite(base_bmp, SPULSE_1, 2, 1, y, 150, width, 170, 1, 19);
 y += width;

 width = 9;
 make_shield_sprite(base_bmp, SPULSE_1, 3, 1, y, 149, width, 170, 1, 19);
 y += width;

 width = 8;
 make_shield_sprite(base_bmp, SPULSE_1, 4, 1, y, 121, width, 150, 1, 19);
 y += width;

 width = 7;
 make_shield_sprite(base_bmp, SPULSE_1, 5, 1, y, 103, width, 130, 1, 19);
 y += width;

 width = 4;
 make_shield_sprite(base_bmp, SPULSE_1, 6, 1, y, 81, width, 110, 1, 19);
 y += width;

 width = 4;
 make_shield_sprite(base_bmp, SPULSE_1, 7, 1, y, 55, width, 100, 1, 19);
 y += width;
*/


// make_shield_sprite(base_bmp, SPULSE_1, 0, 1, 3, 39, 18, 45, 6, 6);
 y = 3;
 width = 57;
 make_shield_sprite(base_bmp, SHIELD_WSHIP, 0, 0, y, 106, width, 130, 16, 19);
 y += width;

 width = 50;
 make_shield_sprite(base_bmp, SHIELD_WSHIP, 1, 0, y, 100, width, 120, 13, 19);
 y += width;

 width = 42;
 make_shield_sprite(base_bmp, SHIELD_WSHIP, 2, 0, y, 95, width, 100, 1, 19);
 y += width;

 width = 30;
 make_shield_sprite(base_bmp, SHIELD_WSHIP, 3, 0, y, 63, width, 80, 1, 19);
 y += width;

 width = 10;
 make_shield_sprite(base_bmp, SHIELD_WSHIP, 4, 0, y, 53, width, 70, 1, 19);
 y += width;


 y = 1;
 width = 21;
 make_shield_sprite(base_bmp, SHIELD_FIGHTER, 0, 130, y, 67, width, 80, 15, 12);
 y += width;

 width = 17;
 make_shield_sprite(base_bmp, SHIELD_FIGHTER, 1, 130, y, 61, width, 73, 15, 12);
 y += width;

 width = 17;
 make_shield_sprite(base_bmp, SHIELD_FIGHTER, 2, 130, y, 67, width, 80, 15, 12);
 y += width;

 width = 17;
 make_shield_sprite(base_bmp, SHIELD_FIGHTER, 3, 130, y, 67, width, 80, 15, 12);
 y += width;

 load_progress();


 destroy_bitmap(base_bmp);


// base_bmp = load_up_bitmap("gfx//turret.bmp");

// make_turret_sprite_pivot(base_bmp, TURRET_SPRITE_BASIC, 1, 1, 45, 49, 100, 45, 22, 35, 35);

// destroy_bitmap(base_bmp);



}

void save_turret_bitmap(void)
{

 int i;
 int j;
 int x, y;

 BITMAP *bmp = new_bitmap(TURRET_ROTATIONS * 30, TURRET_SPRITES * 30, "save_turret_bitmap");
 clear_bitmap(bmp);

 for (i = 0; i < TURRET_SPRITES; i ++)
 {
  for (j = 0; j < TURRET_ROTATIONS; j ++)
  {
   x = j * 30;
   y = i * 30 + 1;
   draw_sprite(bmp, turret_sprite [i] [j].sprite, x, y);
   putpixel(bmp, x, y-1, turret_sprite [i] [j].sprite->w);
   x++;
   putpixel(bmp, x, y-1, turret_sprite [i] [j].sprite->h);
   x++;
   putpixel(bmp, x, y-1, turret_sprite [i] [j].x [0]);
   x++;
   putpixel(bmp, x, y-1, turret_sprite [i] [j].y [0]);
  }
 }

 save_bitmap("gfx//s_turret.bmp", bmp, palet);

 destroy_bitmap(bmp);

}

void read_turret_bitmap(BITMAP *bmp)
{
 int i;
 int j;
 int x, y;
 int w, h;

// BITMAP *bmp = load_up_bitmap("gfx//s_turret.bmp");

 for (i = 0; i < TURRET_SPRITES; i ++)
 {
  for (j = 0; j < TURRET_ROTATIONS; j ++)
  {
   x = j * 30;
   y = i * 30 + 1;
   w = getpixel(bmp, x, y-1);
   h = getpixel(bmp, x+1, y-1);
   turret_sprite [i] [j].sprite = extract_bitmap(bmp, x, y, w, h);
   turret_sprite [i] [j].x [0] = getpixel(bmp, x+2, y-1);
   turret_sprite [i] [j].y [0] = getpixel(bmp, x+3, y-1);
  }
 }

 destroy_bitmap(bmp);

}



void make_fighter_sprite(BITMAP *base_bmp, int fighter, int fship_index, int x_pos, int y_pos, int xsize, int ysize, int size2, int x_offset, int y_offset)
{

    int i;

    fixed rotation;


    BITMAP *test_bmp2 = extract_bitmap(base_bmp, x_pos, y_pos, xsize, ysize);

    BITMAP *test_bmp3 = new_bitmap(size2, size2, "test_thing");

    BITMAP *test_bmp4;

    int rotation_value = 256 / (FIGHTER_ROTATIONS*4);

    for (i = 0; i < FIGHTER_ROTATIONS; i ++)
    {

     clear_to_color(test_bmp3, 0);

     rotation = itofix(i * rotation_value);

     rotate_sprite(test_bmp3, test_bmp2, x_offset, y_offset, rotation);

     test_bmp4 = reduce_fighter_sprite(test_bmp3, 0);

//     circle(test_bmp4, 15, 15, 5, 8);
//     clear_to_color(test_bmp4, 13);

     new_fighter_bmp_struct(test_bmp4, "make_fighter_sprite", fighter_sprite [fighter], i);

    }

    destroy_bitmap(test_bmp2);
    destroy_bitmap(test_bmp3);
    destroy_bitmap(test_bmp4);

}





// wship is sprite number, wship_index is wship type number, type is ship (or eclass) index (e.g. SHIP_OLD2)
void make_wship_sprite(BITMAP *base_bmp, int wship, int wship_index, int x_pos, int y_pos, int xsize, int ysize, int size2, int x_offset, int y_offset, int type)
{

    int i;

    fixed rotation;

    BITMAP *test_bmp2 = extract_bitmap(base_bmp, x_pos, y_pos, xsize, ysize);

    BITMAP *test_bmp3 = new_bitmap(size2, size2, "test_thing");

    BITMAP *test_bmp4;

    int rotation_value = 256 / (WSHIP_ROTATIONS*4);

    for (i = 0; i < WSHIP_ROTATIONS; i ++)
    {

     clear_to_color(test_bmp3, 0);

     rotation = itofix(i * rotation_value);

     rotate_sprite(test_bmp3, test_bmp2, x_offset, y_offset, rotation);

     test_bmp4 = reduce_wship_sprite(test_bmp3, 0, wship_index, i);

     new_wship_bmp_struct(test_bmp4, "make_wship_sprite", wship_sprite [wship], i);

    }



    destroy_bitmap(test_bmp2);
    destroy_bitmap(test_bmp3);
    destroy_bitmap(test_bmp4);

}

void make_wship_collision_mask(BITMAP *base_bmp, int wship, int x_pos, int y_pos, int xsize, int ysize, int type)
{
int x_offset = 0, y_offset = 0;

    ship_collision_mask [type].sprite = extract_bitmap(base_bmp, x_pos, y_pos, xsize, ysize);
    switch(type)
    {
     case SHIP_OLD2: x_offset = -9; y_offset = 7; break;
     case SHIP_OLD3: x_offset = -13; y_offset = 8; break;
     case SHIP_FRIEND3: x_offset = 9; y_offset = 8; break;
     case SHIP_SCOUT2: x_offset = 8; y_offset = 8; break;
     case SHIP_SCOUT3: x_offset = 2; y_offset = 9; break;
     case SHIP_SCOUTCAR: x_offset = 5; y_offset = 7; break;
     case SHIP_EBASE: x_offset = 9; y_offset = 8; break;
    }
// the offsets are the size of the shield on the leftmost and forwardmost point.
//   --- or maybe not. Just test them by drawing the collision mask on screen and comparing to the
//    angle 0 sprite.
    ship_collision_mask [type].x [0] = wship_sprite [wship] [0].x [0] + x_offset;
    ship_collision_mask [type].y [0] = wship_sprite [wship] [0].y [0] + y_offset;

}


void make_damage_sprite(BITMAP *base_bmp, int index, int x_pos, int y_pos, int xsize, int ysize, int fix_colour)
{

 BITMAP* temp_base = extract_bitmap(base_bmp, x_pos, y_pos, xsize, ysize);
 BITMAP* temp_base2 = new_bitmap(xsize, ysize, "make_damage_sprite");

 int i;

 int dcols [7] =
 {
  TRANS_DAM_GREEN,
  TRANS_DAM_YELLOW,
  TRANS_DAM_RED,
  TRANS_DAM_GREY,
  TRANS_DAM_GREEN1,
  TRANS_DAM_YELLOW1,
  TRANS_DAM_RED1
 };

 TRANS_MODE

 if (fix_colour != -1)
 {
  blit(temp_base, temp_base2, 0, 0, 0, 0, xsize, ysize);
  rectfill(temp_base2, 0, 0, xsize, ysize, dcols [fix_colour]);
  damage_sprite [index] [fix_colour] = get_rle_sprite(temp_base2);
 }
  else
  {
   for (i = 0; i < DAMAGE_COLS; i ++)
   {
    blit(temp_base, temp_base2, 0, 0, 0, 0, xsize, ysize);
    rectfill(temp_base2, 0, 0, xsize, ysize, dcols [i]);
    damage_sprite [index] [i] = get_rle_sprite(temp_base2);
   }
  }

 destroy_bitmap(temp_base);
 destroy_bitmap(temp_base2);

 END_TRANS

}



void make_turret_sprite(BITMAP *base_bmp, int turret, int x_pos, int y_pos, int xsize, int ysize, int size2, int x_offset, int y_offset)
{

    int i;

    fixed rotation;

    BITMAP *test_bmp2 = extract_bitmap(base_bmp, x_pos, y_pos, xsize, ysize);

    BITMAP *test_bmp3 = new_bitmap(size2, size2, "turret bmp_3");

    BITMAP *test_bmp4;

    int rotation_value = 256 / (TURRET_ROTATIONS);

    for (i = 0; i < TURRET_ROTATIONS; i ++)
    {

     clear_to_color(test_bmp3, 0);

     rotation = itofix(i * rotation_value);

     rotate_sprite(test_bmp3, test_bmp2, x_offset, y_offset, rotation);

     test_bmp4 = reduce_turret_sprite(test_bmp3, 1);

     new_turret_bmp_struct(test_bmp4, "make_turret_sprite", turret_sprite [turret], i);

//     draw_sprite(screen, test_bmp4, 5 + i * 30, 5);
//     print_number(5 + i * 30, 20, test_bmp4->h);
//     print_number(5 + i * 30, 35, test_bmp4->w);
// textprintf_ex(screen, font, 10 + i * 30, 20 + i * 20, -1, -1, "%i, %i", test_bmp4->w, test_bmp4->h);

    }

    destroy_bitmap(test_bmp2);
    destroy_bitmap(test_bmp3);
    destroy_bitmap(test_bmp4);

}

void make_missile_sprite(BITMAP *base_bmp, int missile, int x_pos, int y_pos, int xsize, int ysize, int size2, int x_offset, int y_offset)
{

    int i;

    fixed rotation;

    BITMAP *test_bmp2 = extract_bitmap(base_bmp, x_pos, y_pos, xsize, ysize);

    BITMAP *test_bmp3 = new_bitmap(size2, size2, "missile bmp_3");

    BITMAP *test_bmp4;

    int rotation_value = 256 / (MISSILE_ROTATIONS);

    for (i = 0; i < MISSILE_ROTATIONS; i ++)
    {

     clear_to_color(test_bmp3, 0);

     rotation = itofix(i * rotation_value);

     rotate_sprite(test_bmp3, test_bmp2, x_offset, y_offset, rotation);

     test_bmp4 = reduce_sprite(test_bmp3, 0);

     new_missile_bmp_struct(test_bmp4, "make_missile_sprite", missile_sprite [missile], i);

    }

    destroy_bitmap(test_bmp2);
    destroy_bitmap(test_bmp3);
    destroy_bitmap(test_bmp4);

}



void make_player_sprite(BITMAP *base_bmp, int sprite, int x_pos, int y_pos, int xsize, int ysize, int size2, int x_offset, int y_offset)
{

    int i;

    fixed rotation;


    BITMAP *test_bmp2 = extract_bitmap(base_bmp, x_pos, y_pos, xsize, ysize);

    BITMAP *test_bmp3 = new_bitmap(size2, size2, "make_player_sprite");

    BITMAP *test_bmp4;

    float rotation_value = 256 / (PLAYER_ROTATIONS*4) + 0.2;
//    int rotation_value = 256 / (PLAYER_ROTATIONS*4);

    for (i = 0; i < PLAYER_ROTATIONS; i ++)
    {

     clear_to_color(test_bmp3, 0);

     rotation = ftofix(i * rotation_value);

     if (i == 0)
      draw_sprite(test_bmp3, test_bmp2, x_offset, y_offset);
       else
        rotate_sprite(test_bmp3, test_bmp2, x_offset, y_offset, rotation);

     test_bmp4 = reduce_player_sprite(test_bmp3, 0);

//     circle(test_bmp4, 15, 15, 5, 8);
//     clear_to_color(test_bmp4, 13);

     new_player_bmp_struct(test_bmp4, "make_player_sprite", player_sprite [sprite], i);

    }

    destroy_bitmap(test_bmp2);
    destroy_bitmap(test_bmp3);
    destroy_bitmap(test_bmp4);



/*
int x, y, p;

 for (x = 0; x < player_sprite [0] [0].sprite->w; x ++)
 {
  for (y = 0; y < player_sprite [0] [0].sprite->h; y ++)
  {
   p = getpixel(player_sprite [0] [0].sprite, x, y);
   rectfill(screen, x * 5, y * 5, x * 5 + 4, y * 5 + 4, p);
  }
 }

do
{
 x++;
} while (key [KEY_Q] == 0);
*/
}



/*
void make_turret_sprite(BITMAP *base_bmp, int turret, int x_pos, int y_pos, int xsize, int ysize, int size2, int x_offset, int y_offset)
{

    int i;

    fixed rotation;

    BITMAP *test_bmp2 = extract_bitmap(base_bmp, x_pos, y_pos, xsize, ysize);

    BITMAP *test_bmp3 = new_bitmap(size2, size2, "turret bmp_3");

    BITMAP *test_bmp4;

    int rotation_value = 256 / (TURRET_ROTATIONS*4);

    for (i = 0; i < TURRET_ROTATIONS; i ++)
    {

     clear_to_color(test_bmp3, 0);

     rotation = itofix(i * rotation_value);

     rotate_sprite(test_bmp3, test_bmp2, x_offset, y_offset, rotation);

     test_bmp4 = reduce_turret_sprite(test_bmp3, 1);

     new_turret_bmp_struct(test_bmp4, "make_turret_sprite", turret_sprite [turret], i);

     draw_sprite(screen, test_bmp4, 5 + i * 30, 5);
//     print_number(5 + i * 30, 20, test_bmp4->h);
//     print_number(5 + i * 30, 35, test_bmp4->w);
 textprintf_ex(screen, font, 10 + i * 30, 20 + i * 20, -1, -1, "%i, %i", test_bmp4->w, test_bmp4->h);

    }

    destroy_bitmap(test_bmp2);
    destroy_bitmap(test_bmp3);
    destroy_bitmap(test_bmp4);

}
*/

void make_turret_sprite_pivot(BITMAP *base_bmp, int turret, int x_pos, int y_pos, int xsize, int ysize, int size2, int x_offset, int y_offset, int pivot_x, int pivot_y)
{

    int i;

    fixed rotation;

    BITMAP *test_bmp2 = extract_bitmap(base_bmp, x_pos, x_pos, xsize, ysize);

    BITMAP *test_bmp3 = new_bitmap(size2, size2, "turret bmp_3");

    BITMAP *test_bmp4;

    int rotation_value = 256 / TURRET_ROTATIONS;

    for (i = 0; i < TURRET_ROTATIONS; i ++)
    {

     clear_to_color(test_bmp3, 0);

     rotation = itofix(i * rotation_value);

     pivot_sprite(test_bmp3, test_bmp2, x_offset, y_offset, pivot_x, pivot_y, rotation);

     test_bmp4 = reduce_turret_sprite(test_bmp3, 1);

     new_turret_bmp_struct(test_bmp4, "make_turret_sprite", turret_sprite [turret], i);

    }

    destroy_bitmap(test_bmp2);
    destroy_bitmap(test_bmp3);
    destroy_bitmap(test_bmp4);

}




BITMAP *reduce_sprite(BITMAP *bmp, char do_alpha)
{

 int i, j, k, alpha;

 int col;

 int w1 = bmp->w;
 int h1 = bmp->h;

 int w2 = w1 / 3 + 1;
 int h2 = h1 / 3 + 1;

 int p [9];

 BITMAP *reduced_bmp = new_bitmap(w2, h2, "reduce_sprite");

 for (i = 0; i < w2; i ++)
 {
  for (j = 0; j < h2; j ++)
  {
   alpha = 0, col = 0;

   p [0] = getpixel(bmp, i*3, j*3);
   p [1] = getpixel(bmp, i*3 + 1, j*3);
   p [2] = getpixel(bmp, i*3 + 2, j*3);
   p [3] = getpixel(bmp, i*3, j*3 + 1);
   p [4] = getpixel(bmp, i*3, j*3 + 2);
   p [5] = getpixel(bmp, i*3 + 1, j*3 + 1);
   p [6] = getpixel(bmp, i*3 + 1, j*3 + 2);
   p [7] = getpixel(bmp, i*3 + 2, j*3 + 1);
   p [8] = getpixel(bmp, i*3 + 2, j*3 + 2);

   for (k = 0; k < 9; k ++)
   {
    if (p [k] != 0 && p [k] >= 1 && p [k] <= 16)
     alpha ++;
    col += p [k];
   }

   if (do_alpha == 1)
   {
    if (alpha > 0)
     col /= alpha;
      else
       col = 0;
   }
    else
     col /= 9;

   if (col < 1 || col > 17)
    col = 0;

   if (do_alpha == 1)
   {
    col += 16 * alpha;
   }

   _putpixel(reduced_bmp, i, j, col);

  }
 }

 return reduced_bmp;
}



BITMAP *reduce_wship_sprite(BITMAP *bmp, char do_alpha, int wship, int rotation)
{

 int i, j, k, alpha;

 int col;

 int w1 = bmp->w;
 int h1 = bmp->h;

 int w2 = w1 / 3 + 1;
 int h2 = h1 / 3 + 1;

 int p [9];

 BITMAP *reduced_bmp = new_bitmap(w2, h2, "reduce_sprite");

 int l, m, num;

 for (i = 0; i < w2; i ++)
 {
  for (j = 0; j < h2; j ++)
  {
   alpha = 0, col = 0;

   for (l = 0; l < 3; l ++)
   {
    for (m = 0; m < 3; m ++)
    {
     num = getpixel(bmp, i*3 + l, j * 3 + m);
     if (num >= COL_CONTROL_BASE && num < COL_CONTROL_BASE + BMPS_VALUES)
     {
      control_x [num - COL_CONTROL_BASE] = i;
      control_y [num - COL_CONTROL_BASE] = j;
     }
     p [l + m * 3] = num;
    }
   }

   for (k = 0; k < 9; k ++)
   {
    if (p [k] != 0 && p [k] >= 1 && p [k] <= 16)
     alpha ++;
    col += p [k];
   }

   if (do_alpha == 1)
   {
    if (alpha > 0)
     col /= alpha;
      else
       col = 0;
   }
    else
     col /= 9;

    col *= 7;
    col /= 16;

   if (col < 1 || col > 7)
    col = 0;

   if (do_alpha == 1)
   {
    col += 19 * alpha;
   }

   if (col != 0)
    col += base_colour;

   _putpixel(reduced_bmp, i, j, col);

  }
 }


 for (i = 1; i < BMPS_VALUES; i ++)
 {
  control_x [i] -= control_x [0];
  control_y [i] -= control_y [0];
 }

 return reduced_bmp;
}




/*
like the other reduce_sprite but sets turret location values when it finds a special colour
*/
/*
BITMAP *reduce_wship_sprite(BITMAP *bmp, char do_alpha, int wship, int rotation)
{

 int i, j, k, alpha;

 int col;

 int w1 = bmp->w;
 int h1 = bmp->h;

 int w2 = w1 / 3 + 1;
 int h2 = h1 / 3 + 1;

 int p [9];

 BITMAP *reduced_bmp = new_bitmap(w2, h2, "reduce_sprite");

 int l, m, num;

 for (i = 0; i < w2; i ++)
 {
  for (j = 0; j < h2; j ++)
  {
   alpha = 0, col = 0;

   for (l = 0; l < 3; l ++)
   {
    for (m = 0; m < 3; m ++)
    {
     num = getpixel(bmp, i*3 + l, j * 3 + m);
     if (num >= COL_CONTROL_BASE && num < COL_CONTROL_BASE + BMPS_VALUES)
     {
      control_x [num - COL_CONTROL_BASE] = i;
      control_y [num - COL_CONTROL_BASE] = j;
     }
     p [l + m * 3] = num;
    }
   }

   for (k = 0; k < 9; k ++)
   {
    if (p [k] != 0 && p [k] >= 1 && p [k] <= 16)
     alpha ++;
    col += p [k];
   }

   if (do_alpha == 1)
   {
    if (alpha > 0)
     col /= alpha;
      else
       col = 0;
   }
    else
     col /= 9;

   if (col < 1 || col > 17)
    col = 0;

   if (do_alpha == 1)
   {
    col += 16 * alpha;
   }

   _putpixel(reduced_bmp, i, j, col);

  }
 }


 for (i = 1; i < BMPS_VALUES; i ++)
 {
  control_x [i] -= control_x [0];
  control_y [i] -= control_y [0];
 }

 return reduced_bmp;
}
*/
/*
like the other reduce_sprite but sets centre location values when it finds a special colour
*/
BITMAP *reduce_fighter_sprite(BITMAP *bmp, char do_alpha)
{

//static int print_count = 0;

 int i, j, k, alpha;

 int col;

 int w1 = bmp->w;
 int h1 = bmp->h;

 int w2 = w1 / 3 + 1;
 int h2 = h1 / 3 + 1;

// int centre_x = -10000;
// int centre_y = -10000;

 int p [9];

 BITMAP *reduced_bmp = new_bitmap(w2, h2, "reduce_sprite");

 for (i = 0; i < NO_TURRETS; i ++)
 {
//  turret_location [wship] [rotation].x [i] = -99999;
  //turret_location [wship] [rotation].y [i] = 0;
 }

 int l, m, num;

 for (i = 0; i < w2; i ++)
 {
  for (j = 0; j < h2; j ++)
  {
   alpha = 0, col = 0;

   for (l = 0; l < 3; l ++)
   {
    for (m = 0; m < 3; m ++)
    {
     num = getpixel(bmp, i*3 + l, j * 3 + m);
     if (num >= COL_CONTROL_BASE && num < COL_CONTROL_BASE + BMPS_VALUES)
//     if (num == control_colour [0])//>= COL_CONTROL_BASE && num < COL_CONTROL_BASE + BMPS_VALUES)
     {
      control_x [num - COL_CONTROL_BASE] = i;
      control_y [num - COL_CONTROL_BASE] = j;
//      control_x [num - COL_CONTROL_BASE] = 100;
// control_x [0] = num - COL_CONTROL_BASE;//num - COL_CONTROL_BASE;

//  textprintf_ex(screen, font, 10, 10 + print_count * 13, -1, -1, "num %i COL_CONTROL_BASE %i i %i j %i", num, COL_CONTROL_BASE, i, j);
/*  if (num > 16)
  {
  textprintf_ex(screen, font, 10, 20 + print_count * 14, -1, -1, "num %i COL_CONTROL_BASE %i i %i j %i", num, COL_CONTROL_BASE, i, j);

if (key [KEY_ESC]) exit(10);

 print_count ++;
 if (print_count > 20) print_count = 0;

  textprintf_ex(screen, font, 10, 5, -1, -1, "%i", print_count);
  }*/
// control_x [0] = i;//num - COL_CONTROL_BASE;
// control_y [0] = j;

     }
/*     if (num == centre_locator_colour)
     {
       centre_x = i;
       centre_y = j;
//       special = num;
       num = getpixel(bmp, i*3 + l - 1, j * 3 + m);
     }*/
     p [l + m * 3] = num;
    }
   }

   for (k = 0; k < 9; k ++)
   {
    if (p [k] != 0 && p [k] >= 1 && p [k] <= 16)
     alpha ++;
    col += p [k];
   }

   if (do_alpha == 1)
   {
    if (alpha > 0)
     col /= alpha;
      else
       col = 0;
   }
    else
     col /= 9;

    col *= 7;
    col /= 16;

   if (col < 1 || col > 7)
    col = 0;

   if (do_alpha == 1)
   {
    col += 19 * alpha;
   }

   if (col != 0)
    col += base_colour;

   _putpixel(reduced_bmp, i, j, col);

/*



   if (col < 1 || col > 17)
    col = 0;

   if (do_alpha == 1)
   {
    col += 16 * alpha;
   }

   _putpixel(reduced_bmp, i, j, col);
*/
  }
 }

 for (i = 1; i < BMPS_VALUES; i ++)
 {
  control_x [i] -= control_x [0];
  control_y [i] -= control_y [0];
 }

//     fighter_pivot_x = control_x [0];
//     fighter_pivot_y = control_y [0];

 return reduced_bmp;
}


BITMAP *reduce_player_sprite(BITMAP *bmp, char do_alpha)
{

//static int print_count = 0;

 int i, j, k, alpha;

 int col;

 int w1 = bmp->w;
 int h1 = bmp->h;

 int w2 = w1 / 3 + 1;
 int h2 = h1 / 3 + 1;

// int centre_x = -10000;
// int centre_y = -10000;

 int p [9];

 BITMAP *reduced_bmp = new_bitmap(w2, h2, "reduce_player_sprite");

 int l, m, num;

 for (i = 0; i < w2; i ++)
 {
  for (j = 0; j < h2; j ++)
  {
   alpha = 0, col = 0;

   for (l = 0; l < 3; l ++)
   {
    for (m = 0; m < 3; m ++)
    {
     num = getpixel(bmp, i*3 + l, j * 3 + m);
     if (num >= COL_CONTROL_BASE && num < COL_CONTROL_BASE + BMPS_VALUES)
     {
      control_x [num - COL_CONTROL_BASE] = i;
      control_y [num - COL_CONTROL_BASE] = j;
     }

     p [l + m * 3] = num;
    }
   }

   for (k = 0; k < 9; k ++)
   {
    if (p [k] != 0 && p [k] >= 1 && p [k] <= 16)
     alpha ++;
    col += p [k];
   }

   if (do_alpha == 1)
   {
    if (alpha > 0)
     col /= alpha;
      else
       col = 0;
   }
    else
     col /= 9;

    col *= 7;
    col /= 16;

   if (col < 1 || col > 17)
    col = 0;

   if (do_alpha == 1)
   {
    col += 19 * alpha;
   }

   if (col != 0)
    col += base_colour;

//   col += base_colour;

   _putpixel(reduced_bmp, i, j, col);

  }
 }

 for (i = 1; i < BMPS_VALUES; i ++)
 {
  control_x [i] -= control_x [0];
  control_y [i] -= control_y [0];
 }

 return reduced_bmp;
}



/*
like the other reduce_sprite but sets turret pivot location when it finds a special colour
*/
BITMAP *reduce_turret_sprite(BITMAP *bmp, char do_alpha)
{

 int i, j, k, alpha;

 int col;

 int w1 = bmp->w;
 int h1 = bmp->h;

 int w2 = w1 / 3 + 1;
 int h2 = h1 / 3 + 1;

 int p [9];

 BITMAP *reduced_bmp = new_bitmap(w2, h2, "reduce_turret_sprite");

 control_x [0] = -100;

 int l, m, num;

 for (i = 0; i < w2; i ++)
 {
  for (j = 0; j < h2; j ++)
  {
   alpha = 0, col = 0;

   for (l = 0; l < 3; l ++)
   {
    for (m = 0; m < 3; m ++)
    {
     num = getpixel(bmp, i*3 + l, j * 3 + m);
     if (num >= COL_CONTROL_BASE && num < COL_CONTROL_BASE + BMPS_VALUES)
     {
      control_x [num - COL_CONTROL_BASE] = i;
      control_y [num - COL_CONTROL_BASE] = j;
     }
/*     if (num == centre_locator_colour)
     {
       turret_pivot_x = i;
       turret_pivot_y = j;
       //special = 255;
       num = getpixel(bmp, i*3 + l - 1, j * 3 + m);
     }
//      else
      {
       if (num > 16)
        num = 16;
      }*/
     p [l + m * 3] = num;
    }
   }

   for (k = 0; k < 9; k ++)
   {
    if (p [k] != 0 && p [k] >= 1 && p [k] <= 16)
     alpha ++;
    col += p [k];
   }

   if (do_alpha == 1)
   {
    if (alpha > 0)
     col /= alpha;
      else
       col = 0;
   }
    else
     col /= 9;

    col *= 7;
    col /= 16;

   if (col < 1 || col > 19)
    col = 0;

   if (do_alpha == 1)
   {
    col += 19 * alpha;
   }

   if (col != 0)
    col += base_colour;

   _putpixel(reduced_bmp, i, j, col);

  }
 }

 for (i = 1; i < BMPS_VALUES; i ++)
 {
  control_x [i] -= control_x [0];
  control_y [i] -= control_y [0];
 }
 return reduced_bmp;
}



void new_fighter_bmp_struct(BITMAP *source, const char errtxt [], struct BMP_STRUCT *str, int bl)
{


 int x1 = -1, y1 = -1, x2 = -1, y2 = -1;

 int i, j;

 for (i = 0; i < source->w; i ++)
 {
  for (j = 0; j < source->h; j ++)
  {
    if (getpixel(source, i, j) != 0)
    {
     x1 = i;
     break;
    }
  }
  if (x1 != -1)
   break;
 }

  for (j = 0; j < source->h; j ++)
  {
   for (i = 0; i < source->w; i ++)
   {
    if (getpixel(source, i, j) != 0)
    {
     y1 = j;
     break;
    }
  }
  if (y1 != -1)
   break;
 }

 for (i = source->w - 1; i >= 0; i --)
 {
  for (j = 0; j < source->h; j ++)
  {
    if (getpixel(source, i, j) != 0)
    {
     x2 = i;
     break;
    }
  }
  if (x2 != -1)
   break;
 }


  for (j = source->h - 1; j >= 0; j --)
  {
   for (i = 0; i < source->w; i ++)
   {
    if (getpixel(source, i, j) != 0)
    {
     y2 = j;
     break;
    }
  }
  if (y2 != -1)
   break;
 }



 BITMAP *tmp = new_bitmap(x2 - x1 + 1, y2 - y1 + 1, "new_bmp_struct temp bitmap");

 blit(source, tmp, x1, y1, 0, 0, x2 - x1 + 1, y2 - y1 + 1);

// circle(tmp, tmp->w / 2, tmp->h / 2, 2, 1);

 //rect(tmp, 0, 0, tmp->w-1, tmp->h-1, 15);

     str [bl].sprite = tmp;

      str [bl].x [0] = control_x [0] - x1;
      str [bl].y [0] = control_y [0] - y1;


     for (i = 1; i < BMPS_VALUES; i ++)
     {
      str [bl].x [i] = control_x [i];
      str [bl].y [i] = control_y [i];
     }


}



void new_player_bmp_struct(BITMAP *source, const char errtxt [], struct BMP_STRUCT *str, int bl)
{
//rect(source, 0, 0, source->w-1, source->h-1, COL_F5);

 int x1 = -1, y1 = -1, x2 = -1, y2 = -1;

 int i, j;

 for (i = 0; i < source->w; i ++)
 {
  for (j = 0; j < source->h; j ++)
  {
    if (getpixel(source, i, j) != 0)
    {
     x1 = i;
     break;
    }
  }
  if (x1 != -1)
   break;
 }

  for (j = 0; j < source->h; j ++)
  {
   for (i = 0; i < source->w; i ++)
   {
    if (getpixel(source, i, j) != 0)
    {
     y1 = j;
     break;
    }
  }
  if (y1 != -1)
   break;
 }

 for (i = source->w - 1; i >= 0; i --)
 {
  for (j = 0; j < source->h; j ++)
  {
    if (getpixel(source, i, j) != 0)
    {
     x2 = i;
     break;
    }
  }
  if (x2 != -1)
   break;
 }


  for (j = source->h - 1; j >= 0; j --)
  {
   for (i = 0; i < source->w; i ++)
   {
    if (getpixel(source, i, j) != 0)
    {
     y2 = j;
     break;
    }
  }
  if (y2 != -1)
   break;
 }



 BITMAP *tmp = new_bitmap(x2 - x1 + 1, y2 - y1 + 1, "new_bmp_struct temp bitmap");

 blit(source, tmp, x1, y1, 0, 0, x2 - x1 + 1, y2 - y1 + 1);

// circle(tmp, tmp->w / 2, tmp->h / 2, 2, 1);

 //rect(tmp, 0, 0, tmp->w-1, tmp->h-1, 15);

     str [bl].sprite = tmp;

      str [bl].x [0] = control_x [0] - x1;
      str [bl].y [0] = control_y [0] - y1;


     for (i = 1; i < BMPS_VALUES; i ++)
     {
      str [bl].x [i] = control_x [i];
      str [bl].y [i] = control_y [i];
     }


}





void new_wship_bmp_struct(BITMAP *source, const char errtxt [], struct BMP_STRUCT *str, int bl)
{

// rect(source, 0, 0, source->w-1, source->h-1, COL_E5);

 int x1 = -1, y1 = -1, x2 = -1, y2 = -1;

 int i, j;

 for (i = 0; i < source->w; i ++)
 {
  for (j = 0; j < source->h; j ++)
  {
    if (getpixel(source, i, j) != 0)
    {
     x1 = i;
     break;
    }
  }
  if (x1 != -1)
   break;
 }

  for (j = 0; j < source->h; j ++)
  {
   for (i = 0; i < source->w; i ++)
   {
    if (getpixel(source, i, j) != 0)
    {
     y1 = j;
     break;
    }
  }
  if (y1 != -1)
   break;
 }

 for (i = source->w - 1; i >= 0; i --)
 {
  for (j = 0; j < source->h; j ++)
  {
    if (getpixel(source, i, j) != 0)
    {
     x2 = i;
     break;
    }
  }
  if (x2 != -1)
   break;
 }


  for (j = source->h - 1; j >= 0; j --)
  {
   for (i = 0; i < source->w; i ++)
   {
    if (getpixel(source, i, j) != 0)
    {
     y2 = j;
     break;
    }
  }
  if (y2 != -1)
   break;
 }



 BITMAP *tmp = new_bitmap(x2 - x1 + 1, y2 - y1 + 1, "new_bmp_struct temp bitmap");

 blit(source, tmp, x1, y1, 0, 0, x2 - x1 + 1, y2 - y1 + 1);

// circle(tmp, tmp->w / 2, tmp->h / 2, 2, 1);

// rect(tmp, 0, 0, tmp->w-1, tmp->h-1, 15);

     str [bl].sprite = tmp;
/*
     for (i = 0; i < BMPS_VALUES; i ++)
     {
      str [bl].x [i] = control_x [i] - x1;
      str [bl].y [i] = control_y [i] - y1;
     }
*/
      str [bl].x [0] = control_x [0] - x1;
      str [bl].y [0] = control_y [0] - y1;


     for (i = 1; i < BMPS_VALUES; i ++)
     {
      str [bl].x [i] = control_x [i];
      str [bl].y [i] = control_y [i];
     }


}





void new_turret_bmp_struct(BITMAP *source, const char errtxt [], struct BMP_STRUCT *str, int bl)
{

// rect(source, 0, 0, source->w-1, source->h-1, 150);

 int x1 = -1, y1 = -1, x2 = -1, y2 = -1;

 int i, j;

 for (i = 0; i < source->w; i ++)
 {
  for (j = 0; j < source->h; j ++)
  {
    if (getpixel(source, i, j) != 0)
    {
     x1 = i;
     break;
    }
  }
  if (x1 != -1)
   break;
 }

  for (j = 0; j < source->h; j ++)
  {
   for (i = 0; i < source->w; i ++)
   {
    if (getpixel(source, i, j) != 0)
    {
     y1 = j;
     break;
    }
  }
  if (y1 != -1)
   break;
 }

 for (i = source->w - 1; i >= 0; i --)
 {
  for (j = 0; j < source->h; j ++)
  {
    if (getpixel(source, i, j) != 0)
    {
     x2 = i;
     break;
    }
  }
  if (x2 != -1)
   break;
 }


  for (j = source->h - 1; j >= 0; j --)
  {
   for (i = 0; i < source->w; i ++)
   {
    if (getpixel(source, i, j) != 0)
    {
     y2 = j;
     break;
    }
  }
  if (y2 != -1)
   break;
 }



 BITMAP *tmp = new_bitmap(x2 - x1 + 1, y2 - y1 + 1, "new_turret_rle_struct temp bitmap");

 blit(source, tmp, x1, y1, 0, 0, x2 - x1 + 1, y2 - y1 + 1);


     str [bl].sprite = tmp;
/*
     for (i = 0; i < BMPS_VALUES; i ++)
     {
      str [bl].x [i] = control_x [i] - x1;
      str [bl].y [i] = control_y [i] - y1;
     }*/
      str [bl].x [0] = control_x [0] - x1;
      str [bl].y [0] = control_y [0] - y1;


     for (i = 1; i < BMPS_VALUES; i ++)
     {
      str [bl].x [i] = control_x [i];
      str [bl].y [i] = control_y [i];
     }

// destroy_bitmap(tmp);
}

void new_missile_bmp_struct(BITMAP *source, const char errtxt [], struct BMP_STRUCT *str, int bl)
{

// rect(source, 0, 0, source->w - 1, source->h - 1, COL_F5);

 int x1 = -1, y1 = -1, x2 = -1, y2 = -1;

 int i, j;

 for (i = 0; i < source->w; i ++)
 {
  for (j = 0; j < source->h; j ++)
  {
    if (getpixel(source, i, j) != 0)
    {
     x1 = i;
     break;
    }
  }
  if (x1 != -1)
   break;
 }

  for (j = 0; j < source->h; j ++)
  {
   for (i = 0; i < source->w; i ++)
   {
    if (getpixel(source, i, j) != 0)
    {
     y1 = j;
     break;
    }
  }
  if (y1 != -1)
   break;
 }

 for (i = source->w - 1; i >= 0; i --)
 {
  for (j = 0; j < source->h; j ++)
  {
    if (getpixel(source, i, j) != 0)
    {
     x2 = i;
     break;
    }
  }
  if (x2 != -1)
   break;
 }


  for (j = source->h - 1; j >= 0; j --)
  {
   for (i = 0; i < source->w; i ++)
   {
    if (getpixel(source, i, j) != 0)
    {
     y2 = j;
     break;
    }
  }
  if (y2 != -1)
   break;
 }



 BITMAP *tmp = new_bitmap(x2 - x1 + 1, y2 - y1 + 1, "new_missile_bmp_struct temp bitmap");

 blit(source, tmp, x1, y1, 0, 0, x2 - x1 + 1, y2 - y1 + 1);


     str [bl].sprite = tmp;

     str [bl].x [0] = tmp->w / 2;//control_x [0] - x1;
     str [bl].y [0] = tmp->h / 2;//control_y [0] - y1;


}


void make_shield_sprite(BITMAP *base_bmp, int type, int frame, int x_pos, int y_pos, int xsize, int ysize, int size2, int x_offset, int y_offset)
{

    int i;

    fixed rotation;

    BITMAP *test_bmp2 = extract_bitmap(base_bmp, x_pos, y_pos, xsize, ysize);

    BITMAP *test_bmp3 = new_bitmap(size2, size2, "spulse bmp_3");

    BITMAP *test_bmp4;

    int rotation_value = 256 / (SHIELD_ROTATIONS);

    for (i = 0; i < SHIELD_ROTATIONS; i ++)
    {

     clear_to_color(test_bmp3, 0);

     rotation = itofix(i * rotation_value);

     rotate_sprite(test_bmp3, test_bmp2, x_offset, y_offset, rotation);

//   rect(test_bmp3, 0, 0, test_bmp3->w - 1, test_bmp3->h - 1, 5);

     test_bmp4 = reduce_shield_sprite(test_bmp3, 0);

     new_shield_bmp_struct(test_bmp4, "make_shield_sprite", spulse_sprite [type] [frame], i);

//     draw_sprite(screen, test_bmp4, 5 + i * 30, 5);
//     print_number(5 + i * 30, 20, test_bmp4->h);
//     print_number(5 + i * 30, 35, test_bmp4->w);
// textprintf_ex(screen, font, 10 + i * 30, 20 + i * 20, -1, -1, "%i, %i", test_bmp4->w, test_bmp4->h);

    }

    destroy_bitmap(test_bmp2);
    destroy_bitmap(test_bmp3);
    destroy_bitmap(test_bmp4);

/*
int x, y, p;

 for (x = 0; x < spulse_sprite [0] [0] [0].sprite->w; x ++)
 {
  for (y = 0; y < spulse_sprite [0] [0] [0].sprite->h; y ++)
  {
   p = getpixel(spulse_sprite [0] [0] [0].sprite, x, y);
   rectfill(screen, x * 25, y * 25, x * 25 + 24, y * 25 + 24, p);
   textprintf_centre_ex(screen, font,x * 25 + 12, y * 25 + 1, -1, -1, "%i", p);

  }
 }

do
{
 x++;
} while (key [KEY_Q] == 0);
*/
}



BITMAP *reduce_shield_sprite(BITMAP *bmp, char do_alpha)
{

 int i, j, k, alpha;

 int col;

 int w1 = bmp->w;
 int h1 = bmp->h;

 int w2 = w1 / 3 + 1;
 int h2 = h1 / 3 + 1;

 int p [9];

 BITMAP *reduced_bmp = new_bitmap(w2, h2, "reduce_shield_sprite");

 control_x [0] = -100;

 int l, m, num;

 for (i = 0; i < w2; i ++)
 {
  for (j = 0; j < h2; j ++)
  {
   alpha = 0, col = 0;

   for (l = 0; l < 3; l ++)
   {
    for (m = 0; m < 3; m ++)
    {
     num = getpixel(bmp, i*3 + l, j * 3 + m);
     if (num == 16)
     {
      control_x [0] = i;
      control_y [0] = j;
      num = getpixel(bmp, i*3 + l - 1, j * 3 + m);
     }
/*     if (num == centre_locator_colour)
     {
       turret_pivot_x = i;
       turret_pivot_y = j;
       //special = 255;
       num = getpixel(bmp, i*3 + l - 1, j * 3 + m);
     }
//      else
      {
       if (num > 16)
        num = 16;
      }*/
     p [l + m * 3] = num;
    }
   }

   for (k = 0; k < 9; k ++)
   {
/*    if (p [k] != 0 && p [k] >= 1 && p [k] <= 16)
     alpha ++;*/
    col += p [k];
   }

/*   if (do_alpha == 1)
   {
    if (alpha > 0)
     col /= alpha;
      else
       col = 0;
   }
    else*/
     col /= 9;

   if (col < 1)
    col = 0;
   if (col > 4)
    col = 4;
/*
   if (do_alpha == 1)
   {
    col += 16 * alpha;
   }*/

//   if (col == 0)
//    _putpixel(reduced_bmp, i, j, 0);
//     else
     switch(col)
     {
      default: _putpixel(reduced_bmp, i, j, 0); break;
      case 1: _putpixel(reduced_bmp, i, j, TRANS_BLUE1); break;
      case 2: _putpixel(reduced_bmp, i, j, TRANS_BLUE2); break;
      case 3: _putpixel(reduced_bmp, i, j, TRANS_BLUE3); break;
      case 4: _putpixel(reduced_bmp, i, j, TRANS_BLUE4); break;
     }
//     _putpixel(reduced_bmp, i, j, ((col-1) * COLOUR_GAP*3) + TRANS_BLUE1);
//     _putpixel(reduced_bmp, i, j, col);

  }
 }


 return reduced_bmp;
}





void new_shield_bmp_struct(BITMAP *source, const char errtxt [], struct BMP_STRUCT *str, int bl)
{


 int x1 = -1, y1 = -1, x2 = -1, y2 = -1;

 int i, j;

 for (i = 0; i < source->w; i ++)
 {
  for (j = 0; j < source->h; j ++)
  {
    if (getpixel(source, i, j) != 0)
    {
     x1 = i;
     break;
    }
  }
  if (x1 != -1)
   break;
 }

  for (j = 0; j < source->h; j ++)
  {
   for (i = 0; i < source->w; i ++)
   {
    if (getpixel(source, i, j) != 0)
    {
     y1 = j;
     break;
    }
  }
  if (y1 != -1)
   break;
 }

 for (i = source->w - 1; i >= 0; i --)
 {
  for (j = 0; j < source->h; j ++)
  {
    if (getpixel(source, i, j) != 0)
    {
     x2 = i;
     break;
    }
  }
  if (x2 != -1)
   break;
 }


  for (j = source->h - 1; j >= 0; j --)
  {
   for (i = 0; i < source->w; i ++)
   {
    if (getpixel(source, i, j) != 0)
    {
     y2 = j;
     break;
    }
  }
  if (y2 != -1)
   break;
 }



 BITMAP *tmp = new_bitmap(x2 - x1 + 1, y2 - y1 + 1, "new_shield_rle_struct temp bitmap");

 blit(source, tmp, x1, y1, 0, 0, x2 - x1 + 1, y2 - y1 + 1);


     str [bl].sprite = tmp;
/*
     for (i = 0; i < BMPS_VALUES; i ++)
     {
      str [bl].x [i] = control_x [i] - x1;
      str [bl].y [i] = control_y [i] - y1;
     }*/
      str [bl].x [0] = control_x [0] - x1;
      str [bl].y [0] = control_y [0] - y1;



// destroy_bitmap(tmp);
}

void load_progress(void)
{

 loading_line ++;

 putpixel(screen, 400 - LOADPIXELS + loading_line*2, 400, COL_BOX3);
 putpixel(screen, 401 - LOADPIXELS + loading_line*2, 400, COL_BOX3);


}
