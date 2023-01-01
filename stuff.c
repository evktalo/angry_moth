/*

This file contains some maths and utility functions. The only ones used at present are:

xpart/ypart - call them with angle x and distance d; they'll give you the x component and y component
              of a line in the angle x and the distance d

angle_to_radians - I use a 1024-degree circle in a format called 'angle'. It lets me use lookup tables
radians_to_angle   without interpolation for simple trig, and minimises slow floating-point operations.
                   angle_to_radians takes 'angle' values and turns them into radians; radians_to_angle etc.

grand(n) - random number from 1 to n


*/


#include "config.h"

#include "allegro.h"

#include <math.h>

int turn_towards_angle(int angle, int tangle, int turning);
int delta_turn_towards_angle(int angle, int tangle, int turning);
int turn_towards_angle_forbid(int angle, int tangle, int turning, int forbid);

float angle_to_radians(int angle);

// I have no idea why, but the first few elements of cos_table always get corrupted
//  unless I put a big fat decoy array just above. A similar thing happens to the
//  palette arrays; allegro seems to have a problem with global arrays like these.
float decoy_table [ANGLE_1]; // not used
float cos_table [ANGLE_1];
float sin_table [ANGLE_1];

void init_trig(void)
{
    int i;

    for (i = 0; i < ANGLE_1; i ++) {
        cos_table [i] = cos(angle_to_radians(i));// * ANGLE_1;
        sin_table [i] = sin(angle_to_radians(i));// * ANGLE_1;
    }
}

int xpart(int angle, int length)
{
    return (cos_table[angle & ANGLE_MASK] * length);
}

int ypart(int angle, int length)
{
    return (sin_table[angle & ANGLE_MASK] * length);
}

int fxpart(float angle, int length)
{
    return (cos(angle) * length);
}

int fypart(float angle, int length)
{
    return (sin(angle) * length);
}

float angle_to_radians(int angle)
{
    return ((float)angle * PI * 2) / ANGLE_1;
}

int radians_to_angle(float angle)
{
    if (angle < 0) {
        angle += PI * 2;
    }
    return (int)((angle * ANGLE_1) / (PI * 2));
}

int grand(int number)
{
    if (number == 0) {
        return 0;
    }
    return ((rand() + (rand() << 16)) & 0x7fffffff) % number;
}

// returns the new angle
int turn_towards_angle(int angle, int tangle, int turning)
{
    if ((angle < tangle && tangle > angle + ANGLE_2)
        || (angle > tangle && tangle > angle - ANGLE_2)
    ) {
        angle -= turning;
    } else {
        angle += turning;
    }

    return angle & ANGLE_MASK;
}

// delta version just returns the change needed
int delta_turn_towards_angle(int angle, int tangle, int turning)
{
    if ((angle < tangle && tangle > angle + ANGLE_2)
        || (angle > tangle && tangle > angle - ANGLE_2)
    ) {
        return turning * -1;
    }

    return turning;
}

int delta_turn_towards_xy(int x1, int y1, int x2, int y2, int angle, int turning)
{
    int tangle = radians_to_angle(atan2((y2 - y1), (x2 - x1)));
    tangle &= ANGLE_MASK;
    return delta_turn_towards_angle(angle, tangle, turning);
}

int turn_towards_angle_forbid(int angle, int tangle, int turning, int forbid)
{
    if ((angle < tangle && tangle > angle + ANGLE_2)
        || (angle > tangle && tangle > angle - ANGLE_2)
    ) {
        if (forbid == -1) {
            return angle;
        }
        angle -= turning;
        if (angle < 0) {
            angle += ANGLE_1;
        }
    } else {
        if (forbid == 1) {
            return angle;
        }
        angle += turning;
        if (angle > ANGLE_1) {
            angle -= ANGLE_1;
        }
    }
    return angle;
}

int angle_difference(int a1, int a2)
{
    int d1, d2;

    d1 = (a1 - a2 + ANGLE_1) & ANGLE_MASK;
    d2 = (a2 - a1 + ANGLE_1) & ANGLE_MASK;

    if (d1 < d2) {
        return abs(d1) & ANGLE_MASK;
    }
    return abs(d2) & ANGLE_MASK;
}

int angle_difference_signed(int a1, int a2)
{
    int d1;

    d1 = (a2 - a1) & ANGLE_MASK;
    if (d1 > ANGLE_2) {
        return -ANGLE_1 + d1;
    }
    return d1;
}

int pos_or_neg(int a)
{
    if (grand(2) == 0) {
        return a;
    }
    return a * -1;
}

char coin(void)
{
    return rand() & 1;
}

int incr(int val, int min, int max)
{
    val ++;
    if (val > max) {
        return min;
    }
    return val;
}

int decr(int val, int min, int max)
{
    val --;
    if (val < min) {
        return max;
    }
    return val;
}
