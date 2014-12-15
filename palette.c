/*

This function is straight out of Excellent Bifurcation.
I haven't fixed it up for this example at all.
Use the colour enum in palette.h.

*/


#include "config.h"

#include "allegro.h"


#include "palette.h"
#include "globvars.h"

#include "stuff.h"

COLOR_MAP trans_table;
COLOR_MAP alpha_table;
//COLOR_MAP trans_table2;

//int blend_function(int base, int trans, RGB *rgbl);
int blend_function(int trans, int base, RGB *rgbl);
int blend_function2(int trans, int base, RGB *rgbl);
int alpha_blend_function(int trans, int base, RGB *rgbl);
int limit_colour(int colour_input);
void halfway_colour(int base, int target, int r, int g, int b);
void setcol(int col, int r, int g, int b);

RGB other_palet [1324];
RGB palet [256];
//RGB palet2 [256];
//RGB light_palet [256];
//RGB dark_palet [256];
RGB other_palet2 [2048];
// what seems to be a weird bug in Allegro forces me to put padding around
//  the palet array, or it becomes corrupted.

/*


IMPORTANT TO REMEMBER:

    I created the palette for the GIMP by saving a 16x16 image with all colours
    as a gif then importing the palette from that. This let me get the colours
    in the right order!


 In the GIMP, must go to image|mode|indexed colour and change the settings there.






*/


void special_create_color_table(COLOR_MAP *table, AL_CONST PALETTE pal);
void special_create_color_table2(COLOR_MAP *table, AL_CONST PALETTE pal);
int base_colour(int y);

void colour_table(const char *which_call);

// this is externed in various places
int comm_col [COMM_COLS] [COMM_COL_MAX+1] =
{
 {COL_BOX0, COL_BOX1, COL_BOX2, COL_BOX3, COL_BOX4}, // CWLTH
 {COL_BOX1, COL_BOX2, COL_BOX3, COL_BOX4, COL_WHITE}, // TO_AM
 {COL_EBOX1, COL_EBOX1, COL_EBOX2, COL_EBOX3, COL_EBOX4}, // FED
 {COL_BOX0, COL_BOX1, COL_CBOX2, COL_BOX3, COL_BOX4}, // IMP
 {COL_EBOX1, COL_EBOX1, COL_EBOX2, COL_EBOX3, COL_EBOX4} // other?

};




void reset_palette(void)
{
 vsync();
 set_palette(palet);
}

void init_palette(int stage)
{

int i;


 for (i = 0; i < 256; i ++)
 {

     palet [i].r = 0;
     palet [i].g = 0;
     palet [i].b = 0;

  }

/*
 for (i = 1; i < 17; i ++)
 {

     palet [i].r = limit_colour(i * 3);
     palet [i].g = limit_colour(i * 3);
     palet [i].b = limit_colour(i * 3);

 }
*/
/*
int basic_cols [19] [3] =
{
 {12, 15, 20},
 {15, 17, 24},
 {19, 21, 27},
 {23, 25, 30},
 {27, 28, 33},
 {31, 31, 35},
 {37, 37, 38},

 {20, 12, 15},
 {24, 15, 17},
 {27, 19, 21},
 {30, 23, 25},
 {33, 27, 29},
 {35, 32, 33},
 {38, 37, 38},

 {20, 20, 20},
 {25, 25, 25},
 {30, 30, 30},
 {35, 35, 35},
 {40, 40, 40}
};
*/

int basic_cols [19] [3] =
{
    /*
 {4, 6, 20},
 {7, 9, 24},
 {11, 13, 27},
 {15, 16, 32},
 {19, 20, 35},
 {25, 26, 40},
 {32, 32, 44},

 {22, 5, 8},
 {25, 9, 11},
 {28, 13, 15},
 {32, 29, 22},
 {36, 25, 26},
 {40, 30, 31},
 {42, 36, 37},
*/

 {8, 11, 20},
 {10, 13, 24},
 {16, 18, 27},
 {24, 26, 32},
 {29, 30, 35},
 {34, 34, 40},
 {42, 42, 44},

 {22, 10, 13},
 {25, 14, 16},
 {28, 18, 20},
 {32, 25, 27},
 {36, 30, 31},
 {40, 35, 36},
 {42, 41, 42},

 {20, 20, 20},
 {25, 25, 25},
 {30, 30, 30},
 {35, 35, 35},
 {40, 40, 40}
};

 for (i = 0; i < 19; i ++)
 {
  palet [i + 1].r = basic_cols [i] [0];
  palet [i + 1].g = basic_cols [i] [1];
  palet [i + 1].b = basic_cols [i] [2];
 }

int damage_cols [16] [3] =
{
// green
 {5, 20, 5},
 {10, 35, 10},
 {20, 42, 20},
 {25, 48, 25},
// yellow
 {15, 15, 5},
 {25, 25, 10},
 {35, 35, 20},
 {40, 40, 25},
// red
 {20, 5, 5},
 {35, 10, 10},
 {42, 20, 20},
 {48, 25, 25},
// grey
 {9, 9, 9},
 {18, 18, 18},
 {30, 30, 30},
 {35, 35, 35}

};

 for (i = 0; i < 16; i ++)
 {
  palet [i + 230].r = damage_cols [i] [0];
  palet [i + 230].g = damage_cols [i] [1];
  palet [i + 230].b = damage_cols [i] [2];
 }



/*

 for (i = 1; i < 8; i ++)
 {

// friend colours
     palet [i].r = limit_colour(i * 3 + 15);
     palet [i].g = limit_colour(i * 2 + 15);
     palet [i].b = limit_colour(i * 5 + 15);

// enemy colours
     palet [i + 7].r = limit_colour(i * 7 + 8);
     palet [i + 7].g = limit_colour(i * 2 + 8);
     palet [i + 7].b = limit_colour(i * 3 + 8);

// star colours - these go past the end but the extras will be overwritten later
     palet [i + 14].r = limit_colour(i * 5 + 15);
     palet [i + 14].g = limit_colour(i * 5 + 15);
     palet [i + 14].b = limit_colour(i * 5 + 15);

 }
*/
 setcol(COL_STAR1, 1, 1, 1);

 setcol(COL_BRIGHT, 55, 40, 10);

 setcol(255, 63, 63, 63);
  palet[0].r = 0;
  palet[0].g = 0;
  palet[0].b = 0;

  int current_base [21] [3];

  for (i = 0; i < 21; i ++)
  {
   current_base [i] [0] = palet[i].r;
   current_base [i] [1] = palet[i].g;
   current_base [i] [2] = palet[i].b;
  }

 int base [3], j;

    for (i = 1; i < 20; i ++)
    {
      for (j = 0; j < 3; j ++)
      {
       base [j] = current_base [i] [j];
//       if (base [j] < 15)
//        base[j] = 15;
       if (base [j] < 6)
        base[j] = 6;
      }
/*
      setcol(TRANS_RED1 + i-1, base [0] + 15, base [1] + 1, base [2] / 2);
      setcol(TRANS_RED2 + i-1, base [0] + 31, base [1] + 5, base [2] / 2);
      setcol(TRANS_RED3 + i-1, base [0] / 3 + 42, base [1] / 3 + 25, base [2] / 4);

      setcol(TRANS_GREEN2 + i-1, base [0] + 35, base [1] + 12, base [2] + 16);
      setcol(TRANS_GREEN3 + i-1, base [0] / 4 + 53, base [1] / 4 + 38, base [2] / 3 + 24);
      setcol(TRANS_GREEN4 + i-1, base [0] / 5 + 60, base [1] / 5 + 53, base [2] / 5 + 53);

      setcol(TRANS_YELLOW4 + i-1, base [0] / 4 + 45, base [1] / 4 + 39, base [2] / 5 + 15);

      setcol(TRANS_BLUE1 + i-1, base [0] + 1, base [1] + 1, base [2] + 15);
      setcol(TRANS_BLUE2 + i-1, base [0] / 2 + 18, base [1] / 2 + 18, base [2] / 2 + 30);
      setcol(TRANS_BLUE3 + i-1, base [0] / 4 + 36, base [1] / 4 + 36, base [2] / 4 + 50);
      setcol(TRANS_BLUE4 + i-1, base [0] / 5 + 51, base [1] / 5 + 51, base [2] / 5 + 65);
*/

      setcol(TRANS_RED1 + i-1, base [0] + 12, base [1] + 1, base [2] - 3);
      setcol(TRANS_RED2 + i-1, base [0] + 27, base [1] + 4, base [2] - 9);
      setcol(TRANS_RED3 + i-1, base [0] / 3 + 42, base [1] / 3 + 25, base [2] / 4);
      setcol(TRANS_YELLOW4 + i-1, base [0] / 4 + 45, base [1] / 4 + 39, base [2] / 5 + 15);

      setcol(TRANS_GREEN2 + i-1, base [0] + 12, base [1] / 2 + 5, base [2] + 22);
      setcol(TRANS_GREEN3 + i-1, base [0] / 4 + 43, base [1] / 4 + 15, base [2] / 4 + 43);
      setcol(TRANS_GREEN4 + i-1, base [0] / 5 + 55, base [1] / 5 + 25, base [2] / 5 + 65);
//      setcol(TRANS_GREEN2 + i-1, base [0] + 12, base [1] / 2 + 13, base [2] + 22);
//      setcol(TRANS_GREEN3 + i-1, base [0] / 4 + 43, base [1] / 4 + 32, base [2] / 4 + 43);
//      setcol(TRANS_GREEN4 + i-1, base [0] / 5 + 55, base [1] / 5 + 41, base [2] / 5 + 65);

      setcol(TRANS_BLUE1 + i-1, base [0] - 1, base [1] - 1, base [2] + 12);
      setcol(TRANS_BLUE2 + i-1, base [0] / 2 + 13, base [1] / 2 + 13, base [2] + 22);
      setcol(TRANS_BLUE3 + i-1, base [0] / 4 + 36, base [1] / 4 + 36, base [2] / 4 + 50);
      setcol(TRANS_BLUE4 + i-1, base [0] / 5 + 51, base [1] / 5 + 51, base [2] / 5 + 65);



    }



// to delete!!!
/* for (i = 1; i < 17; i ++)
 {
   for (j = 0; j < 10; j ++)
   {
     palet [i + j * 16].r = limit_colour(20 + i * 2);
     palet [i + j * 16].g = limit_colour(0 + j * 2);
     palet [i + j * 16].b = limit_colour(0 + j * 2);
   }
 }*/

/*

BITMAP *bmp = create_bitmap(16, 16);
   set_palette(palet);

for (i = 0; i < 16; i ++)
{
 for (j = 0; j < 16; j ++)
 {
     putpixel(bmp, i, j, i + j * 16);
 }

}

save_bitmap("spal.bmp", bmp, palet);

*/

// setcol(WRITE_IN, 55, 55, 55);
// setcol(WRITE_OUT, 20, 20, 20);

 setcol(COL_BOX0, 12, 12, 20);
 setcol(COL_BOX1, 19, 19, 31);
 setcol(COL_BOX2, 26, 26, 41);
 setcol(COL_BOX3, 36, 36, 52);
 setcol(COL_BOX4, 46, 46, 62);

 setcol(COL_EBOX1, 32, 20, 20);
 setcol(COL_EBOX2, 42, 28, 28);
 setcol(COL_EBOX3, 52, 36, 36);
 setcol(COL_EBOX4, 60, 44, 44);

 setcol(COL_CBOX2, 38, 28, 40);


   vsync();
   set_palette(palet);

  if (stage == -1)
  {
   special_create_color_table(&trans_table, palet);
   special_create_color_table2(&alpha_table, palet);
  }

   color_map = &trans_table;


}

void setcol(int col, int r, int g, int b)
{
  palet[col].r = limit_colour(r);
  palet[col].g = limit_colour(g);
  palet[col].b = limit_colour(b);


}

int limit_colour(int colour_input)
{

 if (colour_input < 0) return 0;
 if (colour_input > 63) return 63;
 return colour_input;

}

/*
int blend_function(int trans, int base, RGB *rgbl)
{

 if (base <= COLOUR_11)
  return trans + base;

// int low_colour = base % 12;
// int low_trans = trans / 12;
 int trans_strength = (trans / 12 - 1) % 6;

 if (trans == TRANS_PURPLE1)
  trans_strength = 10;
 if (trans == TRANS_PURPLE2)
  trans_strength = 11;

 int base_trans_strength = (base / 12 - 1) % 6;
 if (trans == TRANS_PURPLE1)
  base_trans_strength = 10;
 if (trans == TRANS_PURPLE2)
  base_trans_strength = 11;

 if (trans_strength >= base_trans_strength)
  return trans + (base % 12);
   else return base;

 return trans;

}
*/

int blend_function(int trans, int base, RGB *rgbl)
{


  if (trans == 255)
  {
   return 255;
  }
/*
  if (trans == COL_CBOX2) // this colour is a special case in the scanner functions in display.c - it's used as the outer colour for Commonwealth ships' radar blips.
  {
      if (base == COL_SCANNER_BACKGROUND)
       return COL_CBOX2;
      return base;
  }

//  if (base == COL_CBOX2) // as above
//   return trans;*/

  if (base == 0)
   return 0;

  if (trans == 0)
   return base;

// these are the transparencies that colour the ship damage indicators:
  if (base >= COL_CONTROL_BASE
   && base <= COL_CONTROL_BASE + 4
   && trans >= TRANS_DAM_GREEN
   && trans <= TRANS_DAM_GREY)
  {
   return (base - COL_CONTROL_BASE) + trans;
/*   switch (trans)
   {
    case TRANS_DAM_GREEN: return ;
    case TRANS_DAM_YELLOW: return COL_1 + TRANS_YELLOW4;
    case TRANS_DAM_RED: return COL_4 + TRANS_RED2;
    case TRANS_DAM_GREY: return COL_6;
// the following are for flashes when hit
    case TRANS_DAM_GREEN1: return COL_8 + TRANS_GREEN2;
    case TRANS_DAM_YELLOW1: return COL_5 + TRANS_YELLOW4;
    case TRANS_DAM_RED1: return COL_8 + TRANS_RED2;
//    case TRANS_DAM_GREY: return COL_10;
   }*/
  }
/*
  if (base == COL_CONTROL_BASE + 1)
  {
   switch (trans)
   {
    case TRANS_DAM_GREEN: return COL_2 + TRANS_GREEN1;
    case TRANS_DAM_YELLOW: return COL_1 + TRANS_YELLOW4;
    case TRANS_DAM_RED: return COL_2 + TRANS_RED1;
    case TRANS_DAM_GREY: return COL_4;
    case TRANS_DAM_GREEN1: return COL_6 + TRANS_GREEN2;
    case TRANS_DAM_YELLOW1: return COL_3 + TRANS_YELLOW4;
    case TRANS_DAM_RED1: return COL_6 + TRANS_RED2;
   }
  }*/

  if (trans == TRANS_CONTROL)
   return base;

  if (trans >= TRANS_RED1)
  {
//    int trans_strength = trans - TRANS_RED1;
    int trans_strength = trans / (TRANS_RED1-1);
    if (base <= 19)
     return base + (trans_strength + 0) * 19;

    int base_strength = (base - 1) / 19;

    if (trans_strength >= base_strength)
     return ((base - 1) % 19) + 1 + ((trans_strength + 0) * 19);
      else
       return base;

  }

 base = ((base - 1) % 19) + 1; // need to fix this for alpha overwriting of trans colours!!

 int alpha = (trans - 1) / 16;
 int trans_colour = (trans - 1) % 19;

 if (alpha == 0 || alpha > 9)
  return base;

 if (alpha == 9)
  return trans_colour;

 int output = 0;

 output += trans_colour * alpha;
 output += base * (9 - alpha);
 output /= 9;

 if (output < 1) output = 1;
 if (output > 16) output = 16;

 return output;
/*

 int save_trans = 0;
 int save_base = 0;

 if (trans == CONVERT_WHITE_TO_GREY)
 {
  if (base == WRITE_IN)
   return COL_LGREY;
  return base;
 }

 if (base == WRITE_IN)
  base = COL_WHITE;
 if (base == WRITE_OUT)
  base = COL_DGREY;

 if (trans == COL_HS_OUTER
  || trans == COL_HS_OUTER_EDGE)
 {
  save_trans = (base - 1) / 19;
  save_trans *= 19;
//  save_trans += 1;
  save_base = (base - 0) % 19;
  switch(save_base)
  {
   case COL_HS_INNER:
   case COL_HS_INNER_EDGE:
   case COL_HS_OUTER:
   case COL_HS_INNER_LINE:
   case COL_HS_HURT:
    return save_base + save_trans;
   default: return trans + save_trans;
  }

 }

 if (trans == COL_HS_INNER
  || trans == COL_HS_HURT
  || trans == COL_HS_INNER_EDGE)
 {
  save_trans = (base - 1) / 19;
  save_trans *= 19;
//  save_trans += 1;
  save_base = (base - 0) % 19;
  switch(save_base)
  {
   case COL_HS_INNER:
   case COL_HS_HURT:
   case COL_HS_INNER_LINE:
    return save_base + save_trans;
   default: return trans + save_trans;
  }

 }

 if (trans == COL_HS_INNER_LINE)
 {
  save_trans = (base - 1) / 19;
  save_trans *= 19;
  save_base = (base - 0) % 19;
  return trans + save_trans;
 }

 if (trans == COL_HS_INNER_TRANS)
 {
  save_trans = (base - 1) / 19;
  save_trans *= 19;
  save_base = (base - 0) % 19;
  return COL_HS_HURT + save_trans;
 }

 if (trans == COL_WHITE)
 {
  save_trans = (base - 1) / 19;
  save_trans *= 19;
  save_base = (base - 0) % 19;
  return trans + save_trans;

 }

 if (trans == COL_LGREY)
 {
  save_trans = (base - 1) / 19;
  save_trans *= 19;
  save_base = (base - 0) % 19;
  switch(save_base)
  {
   case COL_WHITE:
   case COL_DGREY:
    return save_base + save_trans;
   default: return trans + save_trans;
  }

 }

 if (trans == COL_DGREY)
 {
  save_trans = (base - 1) / 19;
  save_trans *= 19;
  save_base = (base - 0) % 19;
  switch(save_base)
  {
   case COL_WHITE:
   case COL_LGREY:
//   case COL_OUTLINE:
    return save_base + save_trans;
   default: return trans + save_trans;
  }

 }

 if (trans == COL_OUTLINE)
 {
  save_trans = (base - 1) / 19;
  save_trans *= 19;
  save_base = (base - 0) % 19;
  return save_base + save_trans;
 }



/ *
 if (trans == COL_HS_OUTER
  || trans == COL_HS_OUTER_EDGE)
 {
  save_trans = (base - 1) / 19;
  save_trans *= 19;
//  save_trans += 1;
  save_base = (base - 0) % 19;
  switch(save_base)
  {
   case COL_HS_INNER:
   case COL_HS_INNER_EDGE:
   case COL_HS_OUTER:
   case COL_HS_INNER_LINE:
   case COL_HS_HURT:
    return save_base + save_trans;
   default: return trans + save_trans;
  }

 }

 if (trans == COL_HS_INNER
  || trans == COL_HS_HURT
  || trans == COL_HS_INNER_EDGE)
 {
  save_trans = (base - 1) / 19;
  save_trans *= 19;
//  save_trans += 1;
  save_base = (base - 0) % 19;
//  save_base2 = base - 1;// % 19;
  switch(save_base)
  {
   case COL_HS_INNER:
   case COL_HS_HURT:
   case COL_HS_INNER_LINE:
    return save_base + save_trans;
   default: return trans + save_trans;
  }

 }
*/
/* if (base < TRANS_RED1)
  return trans + base - 1;

 int trans_strength = (trans - 1) / 19;

 int base_trans_strength = (base - 1) / 19;

 if (trans_strength > base_trans_strength)
  return trans + ((base - 1) % 19);
   else return base;*/

// return trans;

}

int alpha_blend_function(int trans, int base, RGB *rgbl)
{

  if (base > COL_STAR5)
  {
   return base;
  }

 int alpha = (trans - 1) / 19;
 int trans_colour_raw = (trans-1) % 19;
 int trans_colour = COL_STAR1;
 if (trans_colour_raw <= COL_E7)
  trans_colour = COL_E1;
 if (trans_colour_raw <= COL_F7)
  trans_colour = COL_F1;
 int trans_bright = trans_colour_raw - trans_colour;

 if (alpha == 0 || alpha > 9)
  return base;

 if (alpha == 9)
  return trans_colour + trans_bright;

 int output = 0;
 int base_bright;
 int base_col;

 base_col = COL_STAR1;
 if (base <= COL_E7)
  base_col = COL_E1;
 if (base <= COL_F7)
  base_col = COL_F1;

 base_bright = base - base_col;

 output += trans_bright * alpha;
 output += base_bright * (9 - alpha);
 output /= 9;

 output += base_col;

 if (output < 1) output = 1;
 if (output > 19) output = 19;

 return output;

}


/*
int alpha_blend_function(int trans, int base, RGB *rgbl)
{

  if (base > 16)
  {
   return base;
  }

 int alpha = (trans - 1) / 16;
 int trans_colour = (trans - 1) % 16;

 if (alpha == 0 || alpha > 9)
  return base;

 if (alpha == 9)
  return trans_colour;

 int output;

 output += trans_colour * alpha;
 output += base * (9 - alpha);
 output /= 9;

 if (output < 1) output = 1;
 if (output > 16) output = 16;

 return output;

}
*/

void set_control_blend(int colour_number, int control_number)
{

     trans_table.data [TRANS_CONTROL] [colour_number] = control_number + COL_CONTROL_BASE;

}

void set_control_trans_blend(int colour_number, int control_number)
{
/*
     trans_table.data [TRANS_CONTROL_TRANS] [colour_number] = ((control_number-1) * COLOUR_GAP) + TRANS_RED1;
    if (colour_number == 16)
     trans_table.data [TRANS_CONTROL_TRANS] [colour_number] = 16;

 if (control_number == 0)
     trans_table.data [TRANS_CONTROL_TRANS] [control_number] = 0;*/

     trans_table.data [TRANS_CONTROL] [colour_number] = control_number;

}


/*
This function had to be modified from the allegro create_color_table
because the allegro version used bestfit_color, whereas we need
specific color values
*/
void special_create_color_table(COLOR_MAP *table, AL_CONST PALETTE pal)
{
   int x, y, z;
   RGB c;

   for (x = 0; x < PAL_SIZE; x++)
   {
      for (y = 0; y < PAL_SIZE; y++)
      {
       z = blend_function(x, y, &c);

       table->data [x] [y] = z;
      }

   }
}

void special_create_color_table2(COLOR_MAP *table, AL_CONST PALETTE pal)
{
   int x, y, z;
   RGB c;

   for (x = 0; x < PAL_SIZE; x++)
   {
      for (y = 0; y < PAL_SIZE; y++)
      {
       z = alpha_blend_function(x, y, &c);

       table->data [x] [y] = z;
      }

   }
}


