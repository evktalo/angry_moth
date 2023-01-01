/*
*********************************************************************************************************

Music time!

*********************************************************************************************************
*/

#include "palette.h"
#include "allegro.h"
#include "config.h"
#include "music.h"
#include "sound.h"
#include "globvars.h"
#include "stuff.h"

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

extern SAMPLE *sounds [NO_WAVS];

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

int beat;
int bt;
int echo;
int ec;
int recycle_chip;

void new_level_music(void)
{
    int i, j;

    for (i = 0; i < MBOX_X; i ++) {
        for (j = 0; j < MBOX_Y; j ++) {
            music_box [i] [j] = -1;
        }
    }

    for (i = 0; i < CHIPS; i ++) {
        chip [i] = CH_NONE;
    }

    for (i = 0; i < ECHOES; i ++) {
        ech [i] = CH_NONE;
    }

    beat = 10;
    bt = 0;
    echo = 20;
    ec = 0;
    recycle_chip = 0;

    int ch;
    int ch_type = CH_LINE;

    for (i = 0; i < 7; i ++) {
        ch_type = CH_WARM;
        ch_type = CH_LINE;
        ch = new_chip(ch_type);
        if (ch != -1) {
            place_chip(ch, grand(MBOX_X), grand(MBOX_Y), grand(CD_DIRS));
        }
    }
}

int new_random_chip(void)
{
    int ch_type = CH_LINE;
    if (grand(4) == 0) {
       ch_type = CH_BASS;
    }

    ch_type = grand(4) + 1;
    int ch = new_chip(ch_type);
    int val;

    if (ch != -1) {
        do {
            val = place_chip(ch, grand(MBOX_X), grand(MBOX_Y), grand(CD_DIRS));
        } while (val == -1);
    }

    return ch;
}

int new_chip(int ch_type)
{
    int i;

    for (i = 0; i < CHIPS+1; i ++) {
        if (i >= CHIPS) {
            return -1;
        }
        if (chip [i] == CH_NONE) {
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
    if (music_box [x] [y] != -1) {
        return -1;
    }

    chip_x [ch] = x;
    chip_y [ch] = y;
    chip_dir [ch] = dir;
    music_box [x] [y] = ch;
    return 1;
}

void run_music(void)
{
    if (!options.sound_init) {
        return;
    }
    if (!options.music) {
        return;
    }
 
    bt ++;
    if (bt == beat) {
        play_music();
        bt = 0;
        recycle_chip ++;
        if (recycle_chip > 100) {
            destroy_chip(0);
            new_random_chip();
            recycle_chip = 0;
        }
    }

    ec ++;
    if (ec == echo) {
        run_echoes();
        ec = 0;
    }
}

void play_music(void)
{
    if (!options.sound_init) {
        return;
    }
    if (!options.music) {
        return;
    }

    int ch;
    int move_x = 0;
    int move_y = 0;

    for (ch = 0; ch < CHIPS; ch ++) {
        if (chip [ch] == CH_NONE) {
            continue;
        }

        switch(chip_dir [ch]) {
            case CD_LEFT:
                move_y = 0;
                move_x = -1;
                if (chip_x [ch] == 0) {
                    play_chip(ch);
                    move_x = 1;
                    chip_dir [ch] = CD_RIGHT;
                }
                break;
            case CD_RIGHT:
                move_x = 1;
                move_y = 0;
                if (chip_x [ch] == MBOX_X-1) {
                    play_chip(ch);
                    move_x = -1;
                    chip_dir [ch] = CD_LEFT;
                }
                break;
            case CD_UP:
                move_x = 0;
                move_y = -1;
                if (chip_y [ch] == 0) {
                    play_chip(ch);
                    move_y = 1;
                    chip_dir [ch] = CD_DOWN;
                }
                break;
            case CD_DOWN:
                move_x = 0;
                move_y = 1;
                if (chip_y [ch] == MBOX_Y-1) {
                    play_chip(ch);
                    move_y = -1;
                    chip_dir [ch] = CD_UP;
                }
                break;
        }

        if (music_box [chip_x [ch] + move_x] [chip_y [ch] + move_y] == -1) {
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
    if (chip_dir [ch] == CD_DIRS) {
        chip_dir [ch] = 0;
    }
}

int chip_notes [MBOX_X] = {NOTE_3C, NOTE_3D, NOTE_3E, NOTE_3G, NOTE_3A, NOTE_4C, NOTE_4D, NOTE_4E, NOTE_4G};

void play_chip(int ch)
{
    if (!options.sound_init) {
        return;
    }
    if (!options.music) {
        return;
    }
    int n = NOTE_2C;

    if (chip_dir [ch] == CD_DOWN
        || chip_dir [ch] == CD_UP
    ) {
        n = chip_notes [chip_x [ch]];
    }
    if (chip_dir [ch] == CD_LEFT
        || chip_dir [ch] == CD_RIGHT
    ) {
        n = chip_notes [chip_y [ch]];
    }

    n += chip_freq_offset [ch];

    play_music_note(chip [ch], 150, 7 + chip_x [ch] * 30, n);
    new_ech(ch, n);
}

int new_ech(int ch, int note)
{
    int i;

    for (i = 0; i < ECHOES; i ++) {
        if (i >= ECHOES) {
            return -1;
        }
        if (ech [i] == CH_NONE) {
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
    if (!options.sound_init) {
        return;
    }
    if (!options.music) {
        return;
    }

    int i;

    for (i = 0; i < ECHOES; i++) {
        if (ech [i] != CH_NONE) {
            play_music_note(ech[i], ech_vol[i] * 30, ech_pan [i], ech_note[i]+ech_freq_offset [i]);
            ech_vol[i] --;
            if (ech_vol[i] == 0) {
                ech [i] = CH_NONE;
            }
        }
    }
}

void play_music_note(int ch_type, int vol, int pan, int n)
{
    if (!options.sound_init) {
        return;
    }
    if (!options.music) {
        return;
    }

    int wv = WAV_BASS;
    switch(ch_type) {
        case CH_LINE: wv = WAV_LINE; break;
        case CH_SAW: wv = WAV_LINE; break;
        case CH_BASS: wv = WAV_BASS; break;
        case CH_WARM: wv = WAV_WARM; break;
    }

    vol *= options.ambience_volume;
    vol /= 100;
    play_sample(sounds [wv], vol, pan, tone [n], 0);
}
