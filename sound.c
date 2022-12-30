#include "config.h"

#include "allegro.h"

#include <string.h>

#include "sound.h"

#include "globvars.h"

#include "math.h"
#include "stuff.h"



int tone [NOTE_ENDNOTE];

SAMPLE *sounds [NO_WAVS];

void load_sample_in(int samp, const char *sfile);
void new_voice_wvfxy_xs_ys(int sample, int priority, int vol, int freq, int x, int y, int x_speed, int y_speed);
int fix_freq(int new_freq);
int position_pan(int x, int y);
int position_vol(int x, int y, int base_vol);
int doppler_shift(int x, int y, int xs, int ys, int base_freq);
void init_sound(void);

/*
Call at startup of program.
*/
void init_sound(void)
{

// first we'll populate the tone array with the right frequencies:
   int i, j;
   float t;
   for (i = 0; i < NOTE_ENDNOTE; i ++)
   {
    t = (float) BASE_TONE;
    for (j = 0; j < i; j ++)
    {
     t *= (float) 1.059463094359;//(1000 + (1000 / 18)) / 1000;
    }
    tone [i] = t;
    // saves me from having to remember how to use the pow function
   }

   reserve_voices(16, 0); // not sure this is necessary
   if (install_sound (DIGI_AUTODETECT, MIDI_NONE, NULL) == -1)
   {
    exit(1); // don't bother with proper error handling yet. This should just disable sound.
   }
   set_volume(255, 0);

   for (i = 0; i < NO_WAVS; i ++)
   {
    sounds [i] = NULL;
   }

   load_sample_in(WAV_BANG1, "bang1");
   load_sample_in(WAV_BANG3, "bang3");
   load_sample_in(WAV_BANG4, "bang4");
   load_sample_in(WAV_BBANG, "bbang");
   load_sample_in(WAV_BASIC, "basic");
   load_sample_in(WAV_BURST, "burst");
   load_sample_in(WAV_SELECT0, "select0");
   load_sample_in(WAV_SELECT1, "select1");
   load_sample_in(WAV_SHIELD, "shield");
   load_sample_in(WAV_WHOOSH, "whoosh");
   load_sample_in(WAV_WHOOSH2, "whoosh2");
   load_sample_in(WAV_BLAT, "blat");
   load_sample_in(WAV_DRIVE, "drive");
   load_sample_in(WAV_BLIP, "blip");
   load_sample_in(WAV_BLIP2, "blip2");
   load_sample_in(WAV_BLIP_L, "blip_l");
   load_sample_in(WAV_BLIP_L2, "blip_l2");
   load_sample_in(WAV_FIRE, "fire");
   load_sample_in(WAV_EBASIC, "ebasic");
   load_sample_in(WAV_HEAVY, "heavy");
   load_sample_in(WAV_HIT_HULL, "hit_hull");
   load_sample_in(WAV_BEAM, "beam");
   load_sample_in(WAV_SINBLIP, "sinblip");
   load_sample_in(WAV_SINBLIP2, "sinblip2");

   load_sample_in(WAV_HIT, "hit");

   load_sample_in(WAV_BASS, "music//bass");
   load_sample_in(WAV_SAW, "music//saw");
   load_sample_in(WAV_LINE, "music//line");
   load_sample_in(WAV_WARM, "music//warm");

//   load_sample_in(BEAT_, "beat//");

// to add a wave file, add a new WAV_??? entry to sound.h and add a line here for it. The sound is now ready!

}


/*
Loads a .wav file. Should be in the .wavs subdirectory
*/
void load_sample_in(int samp, const char *sfile)
{

 char sfile_name [70];

 strcpy(sfile_name, ".//wavs//");
 strcat(sfile_name, sfile);
 strcat(sfile_name, ".wav");

 sounds [samp] = load_sample(sfile_name);

 if (sounds [samp] == NULL)
 {
      set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
      allegro_message("Error: Unable to load sound file: %s", sfile_name);
      exit(1);
 }
}

/*
Pass e.g. WAV_FIRE to this and it plays it
*/
void play_effect(int sample)
{

// play_sample(sounds [sample], 200, 127, 1000, 0);
 new_voice_wvfxy_xs_ys(sample, SPRIORITY_LOW, 200, 1000, 0, 0, 0, 0);

}

void play_basicwfv(int sample, int f, int v)
{

 v *= options.sfx_volume;
 v /= 100;

    play_sample(sounds [sample], v, 127, f, 0);

}


// Only use this for sounds which come from the angry moth itself: (or interface sounds)
void play_effectwfv(int sample, int f, int v)
{

// play_sample(sounds [sample], v, 127, f, 0);

 v *= options.sfx_volume;
 v /= 100;

 play_sample(sounds [sample], v, 127, f, 0);



// new_voice_wvfxy_xs_ys(sample, v, f, 0, 0, 0, 0);


}

/*
plays sample at frequency f (normal is 1000), volume v (255 is max), x position x (middle is 127)
*/
void play_effectwfvx(int sample, int f, int v, int x)
{

// int pan = x / 2500; // this gives us a # from 0 to 255, from the left of the screen to the right. For stereo

// play_sample(sounds [sample], v, pan, f, 0);

// new_voice_wvfxy_xs_ys(sample, v, f, x, 0, 0, 0);

}

// This is probably not so useful as it doesn't have a speed:
void play_effectwfvxy(int sample, int priority, int f, int v, int x, int y)
{

// int pan = 127;//x / 2500; // this gives us a # from 0 to 255, from the left of the screen to the right. For stereo

// play_sample(sounds [sample], v, pan, f, 0);

 if (arena.only_player == -1) // i.e. there are two players
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
    {
     v *= options.sfx_volume;
     v /= 100;

     int dist [2];
     int d = 0;

     dist [0] = abs(hypot(player[0].y - y, player[0].x - x));
     dist [1] = abs(hypot(player[1].y - y, player[1].x - x));

     if (dist [1] < dist [0])
      d = 1;

     dist [d] /= 8000;

     v -= dist [d];

     if (v < 0)
      return;

    play_sample(sounds [sample], v, 127, f, 0);
//     new_voice_wvfxy_xs_ys(sample, v, f, x, y, 0, 0);

    }

  return;
 }

// just one player:

   if (x > player[arena.only_player].x - 900000
     && x < player[arena.only_player].x + 900000
     && y > player[arena.only_player].y - 900000
     && y < player[arena.only_player].y + 900000)
   {

    v *= options.sfx_volume;
    v /= 100;

    new_voice_wvfxy_xs_ys(sample, priority, v, f, x, y, 0, 0);

   }

}

void play_effectwfvxy_xs_ys(int sample, int priority, int f, int v, int x, int y, int xs, int ys)
{

 if (arena.only_player == -1) // i.e. there are two players
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
    {
     v *= options.sfx_volume;
     v /= 100;

     int dist [2];
     int d = 0;

     dist [0] = abs(hypot(player[0].y - y, player[0].x - x));
     dist [1] = abs(hypot(player[1].y - y, player[1].x - x));

     if (dist [1] < dist [0])
      d = 1;

     dist [d] /= 8000;

     v -= dist [d];

     if (v < 0)
      return;

    play_sample(sounds [sample], v, 127, f, 0);
//     new_voice_wvfxy_xs_ys(sample, v, f, x, y, 0, 0);

    }

  return;
 }

// just one player:

   if (x > player[arena.only_player].x - 900000
     && x < player[arena.only_player].x + 900000
     && y > player[arena.only_player].y - 900000
     && y < player[arena.only_player].y + 900000)
   {

    v *= options.sfx_volume;
    v /= 100;

    new_voice_wvfxy_xs_ys(sample, priority, v, f, x, y, xs, ys);

   }

}

void indicator(int sample, int t, int v, int p)
{

 int pan = 127;

 if (arena.only_player == -1
  && p != -1)
 {
  pan = 245;
  if (p == 0)
   pan = 10;
 }

 v *= options.sfx_volume;
 v /= 100;

 play_sample(sounds [sample], v, pan, tone [t], 0);

}


#define VOICES 25

int voice_in_use [VOICES];
int voice_freq [VOICES];
int voice_vol [VOICES];
int voice_x [VOICES];
int voice_y [VOICES];
int voice_x_speed [VOICES];
int voice_y_speed [VOICES];
int voice_index [VOICES];
int voice_player_location [VOICES];

void init_voices(void)
{
 int v;

 for (v = 0; v < VOICES; v ++)
 {
  voice_in_use [v] = 0;
 }

}


// DO NOT call this function when arena.only_player is -1! Only use it when there is just one player.
void new_voice_wvfxy_xs_ys(int sample, int priority, int vol, int freq, int x, int y, int x_speed, int y_speed)
{

 int v;

// vol2 is the volume that the sound will actually play at (we can't change vol
//  because we need it to feed into the voice struct so we have a base value).
//  If spriority is set to LOW, a vol2 of <= 0 will not play at all.
 int vol2;

// if (x == 0 && y == 0 && x_speed == 0)
  //vol2 = vol;
   //else
    vol2 = position_vol(x, y, vol);

 if (vol2 <= 0 && priority == SPRIORITY_LOW)
  return;

// if (sample != WAV_PHASEY)
//  return;

 for (v = 0; v < VOICES; v ++)
 {
  if (voice_in_use [v] == 0)
   break;
  if (v == VOICES - 1)
   return; // oh well, you're not hearing that sound
 }

 if (sounds [sample] == NULL)
  exit(sample);

 voice_index [v] = allocate_voice(sounds [sample]); //play_sample(sounds [sample], vol, 127, freq, 0);

 if (voice_index [v] == -1)
  return;

 voice_x [v] = x;
 voice_y [v] = y;
 voice_x_speed [v] = x_speed;
 voice_y_speed [v] = y_speed;
 voice_freq [v] = freq;
 voice_vol [v] = vol;
/* if (x == 0 && y == 0 && x_speed == 0)
 {
  voice_player_location [v] = 1;
  voice_set_volume(voice_index [v], vol2);
  voice_set_frequency(voice_index [v], fix_freq(freq));
  voice_set_pan(voice_index [v], 127);
 }
   else*/
   {
    voice_player_location [v] = 0;
    voice_set_volume(voice_index [v], vol2);
    voice_set_frequency(voice_index [v], fix_freq(doppler_shift(x, y, x_speed, y_speed, freq)));
    voice_set_pan(voice_index [v], position_pan(x, y));
   }


 voice_in_use [v] = 1;

 voice_start(voice_index [v]);

}

int position_pan(int x, int y)
{

 float angle = atan2(player[arena.only_player].y - y, player[arena.only_player].x - x) - angle_to_radians(player[arena.only_player].angle);

 return 127 - fypart(angle, 127);
}

int position_vol(int x, int y, int base_vol)
{

 int dist = hypot(player[arena.only_player].y - y, player[arena.only_player].x - x);

 dist /= 8000;

 dist -= 50;

 if (dist < 0)
  dist = 0;

 int vol = base_vol - dist;

 if (vol < 0)
  return 0;

 return vol;

}

int doppler_shift(int x, int y, int xs, int ys, int base_freq)
{

 int speed = hypot(player[arena.only_player].y_speed - ys, player[arena.only_player].x_speed - xs);
/*
 int x1 = (PP.x - x);
 int y1 = (PP.y - y);

// unsigned int dist1 = (x1 * x1) + (y1 * y1);
 unsigned int dist1 = hypot(y1, x1);//(x1 * x1) + (y1 * y1);
// if ((x < 0) ^ (y < 0))
//  dist1 *= -1;

 x1 = x1 + ((xs - PP.x_speed));
 y1 = y1 + ((ys - PP.y_speed));

 unsigned int dist2 = hypot(y1, x1);//((x1 + xs - PP.x_speed) * (x1 + xs - PP.x_speed)) + ((y1 + ys - PP.y_speed) * (y1 + ys - PP.y_speed));
*/
// if ((x+xs < 0) ^ (y+ys < 0))
//  dist2 *= -1;

// speed /= 40;

 float position_angle = atan2(player[arena.only_player].y - y, player[arena.only_player].x - x);
 float travel_angle = atan2(ys - player[arena.only_player].y_speed, xs - player[arena.only_player].x_speed);
 int angle_difference = radians_to_angle(position_angle - travel_angle) & ANGLE_MASK;

 int shift = xpart(angle_difference, speed / 70);

 int new_freq = base_freq;

/* if (dist1 < dist2)
  new_freq += shift;
   else
    new_freq -= shift;*/

 new_freq += shift;

/* part[50].x = shift;
 part[50].y = base_freq;
 part[50].old_x = dist1 / 1000;
 part[50].old_y = dist2 / 1000;
 part[50].hp = new_freq;
*/

 if (new_freq < 200)
  return 200;

 return new_freq;

}

int fix_freq(int new_freq)
{

 return (44100 * new_freq) / 1000;

}

void run_voices(void)
{

 int v;

 for (v = 0; v < VOICES; v ++)
 {
  if (voice_in_use [v] == 0)
   continue;

  if (voice_get_position(voice_index [v]) == -1)
  {
   deallocate_voice(voice_index [v]);
   voice_in_use [v] = 0;
   continue;
  }

  voice_x [v] += voice_x_speed [v];
  voice_y [v] += voice_y_speed [v];
  if (!voice_player_location [v])
  {
   voice_set_pan(voice_index [v], position_pan(voice_x [v], voice_y [v]));
   voice_set_volume(voice_index [v], position_vol(voice_x [v], voice_y [v], voice_vol [v]));
   voice_set_frequency(voice_index [v], fix_freq(doppler_shift(voice_x [v], voice_y [v], voice_x_speed [v], voice_y_speed [v], voice_freq [v])));
  }

 }

}

/*

*********************************************************************************************************

Music time!

*********************************************************************************************************

*/

#include "palette.h"

#define MBOX_X 9
#define MBOX_Y 9
#define CHIPS 20

int music_box [MBOX_X] [MBOX_Y];
int chip [CHIPS];
int chip_x [CHIPS];
int chip_y [CHIPS];
char chip_dir [CHIPS];
int chip_freq_offset [CHIPS];

#define ECHOES 20

int ech [ECHOES];
int ech_vol [ECHOES];
int ech_note [ECHOES];
int ech_freq_offset [ECHOES];
int ech_pan [ECHOES];

//int music_count;
//int note_length;

enum
{
CH_NONE,
CH_BASS,
CH_SAW,
CH_LINE,
CH_WARM

};

enum
{
CD_UP,
CD_RIGHT,
CD_DOWN,
CD_LEFT,
CD_DIRS
};

extern volatile unsigned char ticked;
extern BITMAP *display [3];


void init_music(void);
void new_level_music(void);
int new_chip(int ch_type);
int place_chip(int ch, int x, int y, int dir);
void run_music(void);
void play_music(void);
void rotate_chip(int ch);
void play_chip(int ch);
void destroy_chip(int ch);
int new_random_chip(void);

int new_ech(int ch, int note);
void run_echoes(void);
void play_music_note(int ch_type, int vol, int pan, int n);

// testing functions:
void test_run_music(void);
void music_display(void);



int beat;
int bt;
int echo;
int ec;
int recycle_chip;


void init_music(void)
{

// init_sound();


}


void new_level_music(void)
{

 int i, j;

 for (i = 0; i < MBOX_X; i ++)
 {
  for (j = 0; j < MBOX_Y; j ++)
  {
   music_box [i] [j] = -1;
  }
 }

 for (i = 0; i < CHIPS; i ++)
 {
  chip [i] = CH_NONE;
 }

 for (i = 0; i < ECHOES; i ++)
 {
  ech [i] = CH_NONE;
 }


 beat = 10;
 bt = 0;
 echo = 20;
 ec = 0;
 recycle_chip = 0;

 int ch;
 int ch_type = CH_LINE;


 for (i = 0; i < 7; i ++)
 {
  ch_type = CH_WARM;
//  if (grand(4) == 0)
   ch_type = CH_LINE;
  ch = new_chip(ch_type);
  if (ch != -1)
  {
//   if (ch_type == CH_SAW)
//    chip_freq_offset [ch] = -12;
   place_chip(ch, grand(MBOX_X), grand(MBOX_Y), grand(CD_DIRS));
//   chip_x [ch] = 2 + i;
//   chip_y [ch] = 2 + i;
  }
 }


}

int new_random_chip(void)
{

  int ch_type = CH_LINE;
  if (grand(4) == 0)
   ch_type = CH_BASS;

   ch_type = grand(4) + 1;

  int ch = new_chip(ch_type);

  int val;

  if (ch != -1)
  {
//   if (ch_type == CH_SAW)
//    chip_freq_offset [ch] = -12;
   do
   {
    val = place_chip(ch, grand(MBOX_X), grand(MBOX_Y), grand(CD_DIRS));
   } while (val == -1);
//   chip_x [ch] = 2 + i;
//   chip_y [ch] = 2 + i;
  }


 return ch;
}

int new_chip(int ch_type)
{
 int i;

 for (i = 0; i < CHIPS+1; i ++)
 {
  if (i >= CHIPS)
   return -1;
  if (chip [i] == CH_NONE)
  {
   chip [i] = ch_type;
   chip_freq_offset [i] = 0;
   return i;
  }
 }

 return -1;
}

void destroy_chip(int ch)
{
 chip [ch] = CH_NONE;
}

int place_chip(int ch, int x, int y, int dir)
{

 if (music_box [x] [y] != -1)
  return -1;

 chip_x [ch] = x;
 chip_y [ch] = y;
 chip_dir [ch] = dir;
 music_box [x] [y] = ch;
 return 1;

}

void run_music(void)
{

  bt ++;
  if (bt == beat)
  {
   play_music();
   bt = 0;
   recycle_chip ++;
   if (recycle_chip > 100) //grand(1000) < recycle_chip)
   {
    destroy_chip(0);
    new_random_chip();
    recycle_chip = 0;
   }
  }
  ec ++;
  if (ec == echo)
  {
   run_echoes();
   ec = 0;
  }
}

void play_music(void)
{

 int ch;
 char move_x=0;
 char move_y=0;

 for (ch = 0; ch < CHIPS; ch ++)
 {
  if (chip [ch] == CH_NONE)
   continue;
  switch(chip_dir [ch])
  {
   case CD_LEFT:
    move_y = 0;
    move_x = -1;
    if (chip_x [ch] == 0)
    {
     play_chip(ch);
     move_x = 1;
     chip_dir [ch] = CD_RIGHT;
    }
   break;
   case CD_RIGHT:
    move_x = 1;
    move_y = 0;
    if (chip_x [ch] == MBOX_X-1)
    {
     play_chip(ch);
     move_x = -1;
     chip_dir [ch] = CD_LEFT;
    }
    break;
   case CD_UP:
    move_x = 0;
    move_y = -1;
    if (chip_y [ch] == 0)
    {
     play_chip(ch);
     move_y = 1;
     chip_dir [ch] = CD_DOWN;
    }
    break;
   case CD_DOWN:
    move_x = 0;
    move_y = 1;
    if (chip_y [ch] == MBOX_Y-1)
    {
     play_chip(ch);
     move_y = -1;
     chip_dir [ch] = CD_UP;
    }
    break;

  }

 if (music_box [chip_x [ch] + move_x] [chip_y [ch] + move_y] == -1)
 {
  music_box [chip_x [ch]] [chip_y [ch]] = -1;
  chip_x [ch] += move_x;
  chip_y [ch] += move_y;
  music_box [chip_x [ch]] [chip_y [ch]] = ch;
  continue;
 }

// must have collided with another one:
 rotate_chip(ch);
 rotate_chip(music_box [chip_x [ch] + move_x] [chip_y [ch] + move_y]);

 } // ch loop


}

void rotate_chip(int ch)
{
 chip_dir [ch] ++;
 if (chip_dir [ch] == CD_DIRS)
  chip_dir [ch] = 0;
}

int chip_notes [MBOX_X] = {NOTE_3C, NOTE_3D, NOTE_3E, NOTE_3G, NOTE_3A, NOTE_4C, NOTE_4D, NOTE_4E, NOTE_4G};

void play_chip(int ch)
{

 int n = NOTE_2C;

 if (chip_dir [ch] == CD_DOWN
  || chip_dir [ch] == CD_UP)
 {
  n = chip_notes [chip_x [ch]];
 }

 if (chip_dir [ch] == CD_LEFT
  || chip_dir [ch] == CD_RIGHT)
 {
  n = chip_notes [chip_y [ch]];
 }

 n += chip_freq_offset [ch];

/*
 int wv = WAV_LINE;

 switch(chip [ch])
 {
  case CH_SAW:
  wv = WAV_SAW; break;
  case CH_BASS:
  wv = WAV_BASS; break;
  case CH_LINE:
  wv = WAV_LINE; break;
  case CH_WARM:
  wv = WAV_WARM; break;
 }

 wv = WAV_WARM;
*/
 play_music_note(chip [ch], 150, 7 + chip_x [ch] * 30, n);
 new_ech(ch, n);

}

int new_ech(int ch, int note)
{
 int i;

 for (i = 0; i < ECHOES; i ++)
 {
  if (i >= ECHOES)
   return -1;
  if (ech [i] == CH_NONE)
  {
   ech [i] = chip [ch];
   ech_note [i] = note;
   ech_vol [i] = 4;
   ech_freq_offset [i] = chip_freq_offset [ch];
   ech_pan [i] = 7 + chip_x [ch] * 30;
   return i;
  }
 }

 return -1;
}

void run_echoes(void)
{
 int i;

 for (i = 0; i < ECHOES; i++)
 {
  if (ech [i] != CH_NONE)
  {
   play_music_note(ech[i], ech_vol[i] * 30, ech_pan [i], ech_note[i]+ech_freq_offset [i]);
   ech_vol[i] --;
   if (ech_vol[i] == 0)
    ech [i] = CH_NONE;

  }

 }

}

void play_music_note(int ch_type, int vol, int pan, int n)
{
 int wv = WAV_BASS;
 switch(ch_type)
 {
  case CH_LINE: wv = WAV_LINE; break;
  case CH_SAW: wv = WAV_LINE; break;
  case CH_BASS: wv = WAV_BASS; break;
  case CH_WARM: wv = WAV_WARM; break;
 }

  vol *= options.ambience_volume;
  vol /= 100;


 play_sample(sounds [wv], vol, pan, tone [n], 0);

}

#define MB_X 100
#define MB_Y 100
#define MB_SIZE 20
