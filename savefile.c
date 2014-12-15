#include <string.h>

#include "config.h"
#include "allegro.h"
#include "globvars.h"
#include "stuff.h"

#define SAVE_LENGTH 1000

unsigned char save_buffer [SAVE_LENGTH];

int pos;

void add_8b(unsigned char val);
void add_16b(int val);
void save_to_disk(int filenumber);

unsigned char read_8b(void);
int read_16b(void);
int read_from_disk(int filenumber);

/*

***************************************************************************************************************

Functions for saving games


***************************************************************************************************************

*/

int save_game(int filenumber)
{

 pos = 0;

 add_8b(arena.stage);


 save_to_disk(filenumber);

 return pos;
}


void add_8b(unsigned char val)
{
 save_buffer [pos] = val;
 pos ++;
}

void add_16b(int val)
{
 save_buffer [pos] = val & 0xFF;
 pos ++;
 save_buffer [pos] = (val>>8) & 0xFF;
 pos ++;

}

void save_to_disk(int filenumber)
{


 RGB temp_palette [256];
 char fname [20];

 switch(filenumber)
 {
     case 0: strcpy(fname, "save1.sav"); break;
     case 1: strcpy(fname, "save2.sav"); break;
     case 2: strcpy(fname, "save3.sav"); break;
     case 3: strcpy(fname, "save4.sav"); break;
     case 4: strcpy(fname, "save5.sav"); break;
     default: return; // error?
 }

 int x, y;

 x = 100;
 y = SAVE_LENGTH / x;

 BITMAP *bmp = create_bitmap(x, y);

 if (bmp == NULL)
 {
  set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
  allegro_message("ERROR: Savefile could not be created (bitmap creation failed? Why?).");
  exit(1);
 }

 int i;

 for (i = 0; i < SAVE_LENGTH; i++)
 {
  x = i % 100;
  y = i / 100;
  putpixel(bmp, x, y, save_buffer [i]);
 }

 save_bitmap(fname, bmp, temp_palette);


}

/*

***************************************************************************************************************

Functions for loading games


***************************************************************************************************************

*/

int load_game(int filenumber)
{

 pos = 0;
 if (read_from_disk(filenumber) == 0)
  return 0;

 arena.stage = read_8b();

 return 1;
}

unsigned char read_8b(void)
{

 unsigned char val = save_buffer [pos];
 pos ++;
 return val;

}

int read_16b(void)
{

 int val = save_buffer [pos];
 pos ++;
 val += save_buffer [pos] << 8;
 pos ++;
 return val;

}


int read_from_disk(int filenumber)
{

 RGB temp_palette [256];
 char fname [20];

 switch(filenumber)
 {
     case 0: strcpy(fname, "save1.sav"); break;
     case 1: strcpy(fname, "save2.sav"); break;
     case 2: strcpy(fname, "save3.sav"); break;
     case 3: strcpy(fname, "save4.sav"); break;
     case 4: strcpy(fname, "save5.sav"); break;
     default: return 0; // error?
 }

 BITMAP *temp_bitmap = load_bitmap(fname, temp_palette);

 if (temp_bitmap == NULL)
  return 0;

 int i, x, y;

 for (i = 0; i < SAVE_LENGTH; i++)
 {
  x = i % 100;
  y = i / 100;
  save_buffer [i] = getpixel(temp_bitmap, x, y);

 }

 return 1;

}


