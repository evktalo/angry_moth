#include "config.h"

#include <math.h>

#include "allegro.h"

#include "globvars.h"
#include "cloud.h"
#include "sound.h"
#include "display.h"
#include "palette.h"
#include "ship.h"

#include "stuff.h"



void run_bullets(void);
void destroy_bullet(int a, int b);
void drag_bullet(int a, int b, int drag);
void bullet_explodes(int a, int b, int hit_a, int hit_e, int hit_hull);
int detect_bullet_collision(int a, int b, struct bulletstruct* pb);
//void run_etorp(int a, int b);
void run_ptorp(int a, int b);
void run_rocket(int a, int b);
void run_rocket2(int a, int b);
void run_aws_missile(int a, int b);
void run_af_missile(int a, int b);
/*void bullet_torque(int b, int p);
void run_bullet_seeker(int b);
void run_bullet_rocket(int b);
void run_bullet_missile(int b);*/
int pbedist(int ba, int b, int ea, int e);
int xyedist(int x, int y, int ea, int e);
//int check_collision_mask(struct bulletstruct* pb, struct shipstruct* ee, int ba, int b, int ea, int e);
int detect_beam_collision(int ba, int b, struct bulletstruct* pb);
int check_collision_mask(struct shipstruct* ee, int ea, int e, int ba, int b, int bx, int by, char beam);
int hurt_player(int p, int dam, int owner, int ba, int b, int beam, int bx, int by);
void player_explodes(int p);
void add_hitpulse(int p, int type, int start, int end, char beam);

extern struct BMP_STRUCT ship_collision_mask [NO_SHIP_TYPES]; // defined in display_init.c
int pbpdist(int b, int p);
int xypdist(int x, int y, int p);
int xyedist_test_less(int x, int y, int ea, int e, int max);
int xyedist_test_more(int x, int y, int ea, int e, int min);

int flashback_angle(int ship_x, int ship_y, int bx, int by, int bx_speed, int by_speed);

enum
{
BEAM_NO,
BEAM_YES
};

/*
Initialises the player's bullets. Must be called at start of each game/level.
*/
void init_bullets(void)
{

 int a, b;
 for (a = 0; a < NO_TEAMS; a ++)\
 {
  for (b = 0; b < NO_BULLETS; b ++)
  {
   bullet[a][b].type = BULLET_NONE;
  }
 }

}

/*
Creates a bullet of type type and returns the index in the bullet struct array.
*/
int create_bullet(int type, int a)
{
  int b;

  for (b = 0; b < NO_BULLETS; b ++)
  {
      if (bullet[a][b].type == BULLET_NONE)
       break;
      if (b == NO_BULLETS - 1)
       return -1;

  }

 bullet[a][b].type = type;
 bullet[a][b].timeout = 0;
 bullet[a][b].time = 0;
// bullet[a][b].range = PP.range;
// bullet[a][b].direction = PP.angle;
 bullet[a][b].size = 1000;
 bullet[a][b].owner = -1;
// bullet[a][b].new_bullet = 1;
 return b;

}

/*
Call this from the game loop to make the bullets work.
*/
void run_bullets(void)
{

  int a, b, bh, c, angle, dist;//, c, angle, dist, i, speed;
  char finished = 0;

  for (a = 0; a < NO_TEAMS; a ++)
  {
  for (b = 0; b < NO_BULLETS; b ++)
  {
    if (bullet[a][b].type == BULLET_NONE)
     continue;
    finished = 0;
    bullet[a][b].time ++;
    switch(bullet[a][b].type)
    {
        default:
            bullet[a][b].x += bullet[a][b].x_speed;
            bullet[a][b].y += bullet[a][b].y_speed;
            break;
        case BULLET_BIGSHOT:
            if ((BL.time >> 1) & 1)
            {
             angle = grand(ANGLE_1);
             dist = grand(4000);
             c = simple_cloud(CLOUD_DOUBLE_BALL, BL.x + xpart(angle, dist), BL.y + ypart(angle, dist), BL.colour, 30 + grand(10));
             if (c != -1)
             {
              cloud[c].status = 1 + grand(3);
              cloud[c].x_speed = BL.x2;
              cloud[c].y_speed = BL.y2;
             }
            }
            bullet[a][b].x += bullet[a][b].x_speed;
            bullet[a][b].y += bullet[a][b].y_speed;
            break;
          case BULLET_ETORP1:
//            if ((BL.time >> 1) & 1)
            {
             angle = grand(ANGLE_1);
             dist = grand(4000);
             c = simple_cloud(CLOUD_BALL_COL2_SLOW, BL.x + xpart(angle, dist), BL.y + ypart(angle, dist), BL.colour, 20 + grand(10));
             if (c != -1)
             {
              cloud[c].status = 1 + grand(3);
              cloud[c].x_speed = BL.x2;
              cloud[c].y_speed = BL.y2;
             }
            }
            BL.x_speed += xpart(BL.angle, 50);
            BL.y_speed += ypart(BL.angle, 50);
            bullet[a][b].x += bullet[a][b].x_speed;
            bullet[a][b].y += bullet[a][b].y_speed;
            break;
          case BULLET_PTORP1:
           run_ptorp(a, b);
           break;
          case BULLET_AWS_MISSILE:
          case BULLET_HOMING:
           run_aws_missile(a, b);
           break;
          case BULLET_ROCKET:
           run_rocket(a, b);
           break;
          case BULLET_ROCKET2:
           run_rocket2(a, b);
           break;
          case BULLET_AF_MISSILE:
           run_af_missile(a, b);
           break;
          case BULLET_EBEAM1:
           bullet[a][b].timeout --;
           if (bullet[a][b].timeout <= 0)
           {
            destroy_bullet(a, b);
            finished = 1;
            break;
           }
           detect_beam_collision(a, b, &bullet[a][b]);
//           destroy_bullet(a, b);
           finished = 1;
           break;

           case BULLET_ELONGSHOT:
            bullet[a][b].x += bullet[a][b].x_speed;
            bullet[a][b].y += bullet[a][b].y_speed;
            BL.angle += BL.status;
            BL.angle &= ANGLE_MASK;
            break;

/*        case bullet_SCATTER:
            bullet[a][b].x += bullet[a][b].x_speed;
            bullet[a][b].y += bullet[a][b].y_speed;
            bullet[a][b].size = 5000 + (bullet[a][b].status << 7);// / 2;
//            bullet[a][b].draw_size = (bullet[a][b].timeout << 1);
            bullet[a][b].damage = 40 + (bullet[a][b].status * 5);
            bullet[a][b].force = bullet[a][b].damage * 2;
            bullet[a][b].timeout ++;
            if (bullet[a][b].time > bullet[a][b].status2)
             bullet[a][b].status --;
            if (bullet[a][b].status <= 0)
            {
             destroy_bullet(b);
             no_more_bullet = 1;
            }
            break;
        case bullet_WORM:
            run_bullet_seeker(b);
            break;
        case bullet_ROCKET:
            run_bullet_rocket(b);
            break;
        case bullet_MISSILE:
            run_bullet_missile(b);
            break;
        case bullet_MINE:
            for (i = 0; i < MINE_BITS; i ++)
            {
             bullet[a][b].bit_angle [i] += bullet[a][b].bit_spin [i];
             bullet[a][b].bit_angle [i] &= ANGLE_MASK;
             bullet[a][b].bit_time [i] --;
             if (bullet[a][b].bit_time [i] < 0)
             {
              bullet[a][b].bit_time [i] = 40;
              bullet[a][b].bit_angle [i] = grand(ANGLE_1);
              bullet[a][b].bit_dist [i] = 10 + grand(5);
              if (i & 1)
              {
                bullet[a][b].bit_spin [i] = 25 + grand(25);
              }
               else
                bullet[a][b].bit_spin [i] = -25 - grand(25);
             }
            }
*/
/*            angle = bullet[a][b].angle + ANGLE_4;
            speed = xpart(bullet[a][b].timeout * 150, 2500);
            c = simple_cloud(CLOUD_2BALL, bullet[a][b].x, bullet[a][b].y,
                 bullet[a][b].colour, 8 + grand(bullet[a][b].draw_size << 2));
            if (c != -1)
            {
             cloud[c].x_speed = bullet[a][b].x_speed + xpart(angle, speed);
             cloud[c].y_speed = bullet[a][b].y_speed + ypart(angle, speed);
             cloud[c].drag = 950;
            }
            bullet[a][b].x += bullet[a][b].x_speed;
            bullet[a][b].y += bullet[a][b].y_speed;
* /

            angle = grand(ANGLE_1);
            dist = grand(bullet[a][b].damage << 1);
            speed = 300 + grand(300);
            c = simple_cloud(CLOUD_SMALL, bullet[a][b].x + xpart(angle, dist), bullet[a][b].y + ypart(angle, dist),
                 bullet[a][b].colour, 20 + grand(bullet[a][b].draw_size << 2));
            if (c != -1)
            {
             cloud[c].x_speed = bullet[a][b].x_speed + xpart(angle, speed);
             cloud[c].y_speed = bullet[a][b].y_speed + ypart(angle, speed);
             cloud[c].drag = 960;
            }
            bullet[a][b].x += bullet[a][b].x_speed;
            bullet[a][b].y += bullet[a][b].y_speed;
            bullet[a][b].angle += 60;

            break;
        case bullet_MINE_BLAST:
            for (i = 0; i < MINE_BITS; i ++)
            {
             bullet[a][b].bit_angle [i] += bullet[a][b].bit_spin [i];
             bullet[a][b].bit_angle [i] &= ANGLE_MASK;
             bullet[a][b].bit_time [i] --;
//             bullet[a][b].bit_dist [i] --;
            }
            break;
        case bullet_BASIC:
            angle = grand(ANGLE_1);
            dist = grand(bullet[a][b].damage << 3);
            c = simple_cloud(CLOUD_2BALL, bullet[a][b].x + xpart(angle, dist), bullet[a][b].y + ypart(angle, dist),
                 bullet[a][b].colour, 5 + grand(bullet[a][b].draw_size << 3));
            if (c != -1)
            {
             cloud[c].x_speed = bullet[a][b].x_speed;
             cloud[c].y_speed = bullet[a][b].y_speed;
             cloud[c].drag = 960;
            }
            c = simple_cloud(CLOUD_BALL, bullet[a][b].x + xpart(angle, dist), bullet[a][b].y + ypart(angle, dist),
                 bullet[a][b].colour, 3 + grand(bullet[a][b].draw_size << 2));
            if (c != -1)
            {
             cloud[c].x_speed = bullet[a][b].x_speed;
             cloud[c].y_speed = bullet[a][b].y_speed;
             cloud[c].drag = 960;
            }
            bullet[a][b].x += bullet[a][b].x_speed;
            bullet[a][b].y += bullet[a][b].y_speed;
            break;*/
    }

    if (finished == 1)
     continue;

    bullet[a][b].timeout --;

        {
         if (bullet[a][b].timeout <= 0)
          bullet_explodes(a, b, -1, -1, 0);
        }

    // can't hit anything if damage is 0
    if (bullet[a][b].damage > 0)
    {

     bh = detect_bullet_collision(a, b, &bullet[a][b]);

     if (bh >= 0)
     {
//      bullet_torque(b, bh);
//      if (PP.played_hit == 0)
//       play_effectwfvxy(WAV_HIT, 1000 - part[bh].size * 15, 50, bullet[a][b].x, bullet[a][b].y);
//      PP.played_hit = 1;
//      hurt_part(bh, bullet[a][b].damage);
//      continue;
     }

    }

  }
  }

}


void drag_bullet(int a, int b, int drag)
{
 bullet[a][b].x_speed *= drag;
 bullet[a][b].x_speed >>= 10;
 bullet[a][b].y_speed *= drag;
 bullet[a][b].y_speed >>= 10;

}

void bullet_torque(int b, int p)
{
/*
 int speed = ((int) hypot(bullet[a][b].y_speed - hs[0].y_speed, bullet[a][b].x_speed - hs[0].x_speed)) >> 12;
 int force_angle = radians_to_angle(atan2(bullet[a][b].y_speed - hs[0].y_speed, bullet[a][b].x_speed - hs[0].x_speed)) - ANGLE_2;

 apply_torque_to_hs(0, bullet[a][b].x, bullet[a][b].y, bullet[a][b].force * speed * 20, force_angle);

 hs_impulse(0, force_angle + ANGLE_2, bullet[a][b].force * speed * 20);
*/
}


int detect_bullet_collision(int ba, int b, struct bulletstruct* pb)
{

 int e;
 struct shipstruct* ee;
 int size;
 int hit;
 int ea;

 ea = ba ^ 1; // NOTE: assumes NO_TEAMS is 2!!

 for (e = 0; e < NO_SHIPS; e ++)
 {
  if (ship[ea][e].type == SHIP_NONE)
   continue;
  ee = &ship[ea][e];
  size = eclass[ee->type].size;

  if (eclass[ee->type].ship_class == ECLASS_WSHIP)
  {
   if (pb->x + pb->size > ee->x - size
    && pb->x - pb->size < ee->x + size
    && pb->y + pb->size > ee->y - size
    && pb->y - pb->size < ee->y + size)
    {
     if (pbedist(ba, b, ea, e) <= size + (pb->size))
     {

      hit = check_collision_mask(ee, ea, e, ba, b, pb->x, pb->y, BEAM_NO);

      if (hit == -1) // hits shield - note that the pulse graphic thing is done in the collision mask function
      {
       if (pb->owner == -1 && (player[0].target_a == -1 || player[0].target_auto == 1))
       {
        if (ea != player[0].target_a || e != player[0].target_e)
         player[0].target_new = 10;
        player[0].target_a = ea;
        player[0].target_e = e;
       }
       if (pb->owner == -2 && (player[1].target_a == -1 || player[1].target_auto == 1))
       {
        if (ea != player[1].target_a || e != player[1].target_e)
         player[1].target_new = 10;
        player[1].target_a = ea;
        player[1].target_e = e;
       }
       hurt_wship_shield(ea, e, pb->damage, pb->owner, ba, b);
       bullet_explodes(ba, b, ea, e, 0);
       return 1;
      }

      if (hit > -1)
      {
       if (pb->owner == -1 && (player[0].target_a == -1 || player[0].target_auto == 1))
       {
        if (ea != player[0].target_a || e != player[0].target_e)
         player[0].target_new = 10;
        player[0].target_a = ea;
        player[0].target_e = e;
       }
       if (pb->owner == -2 && (player[1].target_a == -1 || player[1].target_auto == 1))
       {
        if (ea != player[1].target_a || e != player[1].target_e)
         player[1].target_new = 10;
        player[1].target_a = ea;
        player[1].target_e = e;
       }
       hurt_wship_part(ea, e, pb->damage, pb->owner, ba, b, hit);
       bullet_explodes(ba, b, ea, e, 1);
       return 1;
      }
     }
    }
    continue;
  }
  if (pb->x + pb->size > ee->x - size
   && pb->x - pb->size < ee->x + size
   && pb->y + pb->size > ee->y - size
   && pb->y - pb->size < ee->y + size)
   {
    if (pbedist(ba, b, ea, e) <= size + (pb->size<<10)) // just use a simple circular hit zone for fighters, for now at least.
    {
//     bullet_explodes(b, e);
/*       if (pb->owner == -1)
       {
        player[0].target_a = ea;
        player[0].target_e = e;
       }
       if (pb->owner == -2)
       {
        player[1].target_a = ea;
        player[1].target_e = e;
       }*/
//     player[pb->owner].target_e = e; // if the ship is destroyed in the call to hurt_e_f it'll just set target to -1 in destroy_ship
//     player[pb->owner].target_a = ea;
//     hurt_ship_fighter(ea, e, pb->damage, pb->owner, ba, b);
       if (pb->owner == -1 && (player[0].target_a == -1 || player[0].target_auto == 1))
       {
        if (ea != player[0].target_a || e != player[0].target_e)
         player[0].target_new = 10;
        player[0].target_a = ea;
        player[0].target_e = e;
       }
       if (pb->owner == -2 && (player[1].target_a == -1 || player[1].target_auto == 1))
       {
        if (ea != player[1].target_a || e != player[1].target_e)
         player[1].target_new = 10;
        player[1].target_a = ea;
        player[1].target_e = e;
       }
     int hit_hull_status = hurt_ship_fighter(ea, e, pb->damage, ba, b, 0, pb->x, pb->y);
     bullet_explodes(ba, b, ea, e, hit_hull_status);
     return 1;
    }
   }
 }

 if (ba == TEAM_ENEMY)
 {
  int p;

  for (p = 0; p < arena.players; p ++)
  {
   if (PP.alive == 0)
    continue;
   size = 8000;
   if (pb->x + pb->size > PP.x - size
    && pb->x - pb->size < PP.x + size
    && pb->y + pb->size > PP.y - size
    && pb->y - pb->size < PP.y + size)
   {
    if (pbpdist(b, p) <= size + (pb->size<<10)) // just use a simple circular hit zone for players, for now at least.
    {
     int hit_player_hull = hurt_player(p, pb->damage, pb->owner, ba, b, BEAM_NO, pb->x - (pb->x_speed * 4), pb->y - (pb->y_speed * 4));
     if (p == 0)
      bullet_explodes(ba, b, TEAM_FRIEND, -1, hit_player_hull);
       else
        bullet_explodes(ba, b, TEAM_FRIEND, -2, hit_player_hull);
     return 1;
    }
   }

  }

 }

 return 0;

/*

 if (hs[0].alive == 0)
  return -1;

 if (bullet[a][b].x < hs[0].x - 700000
  || bullet[a][b].x > hs[0].x + 700000
  || bullet[a][b].y < hs[0].y - 700000
  || bullet[a][b].y > hs[0].y + 700000)
   return -1;

 int p, size;


 for (p = 0; p < NO_PARTS; p ++)
 {
  if (part[p].type != PART_SHIELD)
   continue;
  size = part[p].size << 10;
  if (part[p].shield > 0)
   size += part[p].shield;
  if (part[p].x - size > bullet[a][b].x + bullet[a][b].size + 10000
   || part[p].x + size < bullet[a][b].x - bullet[a][b].size - 10000
   || part[p].y - size > bullet[a][b].y + bullet[a][b].size + 10000
   || part[p].y + size < bullet[a][b].y - bullet[a][b].size - 10000)
    continue;

  if (hypot(part[p].x - bullet[a][b].x, part[p].y - bullet[a][b].y) <= size + bullet[a][b].size)
  {
   return p;
  }

 }

 for (p = 0; p < NO_PARTS; p ++)
 {
  if (part[p].type == PART_NONE
   || part[p].type == PART_SHIELD)
   continue;
  size = part[p].size << 10;
  if (part[p].x - size > bullet[a][b].x + bullet[a][b].size + 10000
   || part[p].x + size < bullet[a][b].x - bullet[a][b].size - 10000
   || part[p].y - size > bullet[a][b].y + bullet[a][b].size + 10000
   || part[p].y + size < bullet[a][b].y - bullet[a][b].size - 10000)
    continue;

  if (hypot(part[p].x - bullet[a][b].x, part[p].y - bullet[a][b].y) <= size + bullet[a][b].size)
  {
   return p;
  }

 }

 for (p = 0; p < NO_CORE_CIRCLES; p ++)
 {
  if (hs[0].core_circle_size [p] == 0)
   continue;
  size = hs[0].core_circle_size [p] << 10;
  if (hs[0].core_circle_x [p] - size > bullet[a][b].x + bullet[a][b].size + 10000
   || hs[0].core_circle_x [p] + size < bullet[a][b].x - bullet[a][b].size - 10000
   || hs[0].core_circle_y [p] - size > bullet[a][b].y + bullet[a][b].size + 10000
   || hs[0].core_circle_y [p] + size < bullet[a][b].y - bullet[a][b].size - 10000)
    continue;

  if (hypot(hs[0].core_circle_x [p] - bullet[a][b].x, hs[0].core_circle_y [p] - bullet[a][b].y) <= size + bullet[a][b].size)
  {
   return -2;
  }

 }



 return 0;
*/
}




int detect_beam_collision(int ba, int b, struct bulletstruct* pb)
{

 int e;
 struct shipstruct* ee;
 int size;
 int hit;
 int ea;

 ea = ba ^ 1; // NOTE: assumes NO_TEAMS is 2!!

 int x, y, i;
 int bsize = pb->size << 9;

 for (i = 0; i < pb->status; i ++)
 {
 x = pb->x + xpart(pb->angle, (pb->status2 * i) << 10);
 y = pb->y + ypart(pb->angle, (pb->status2 * i) << 10);

 for (e = 0; e < NO_SHIPS; e ++)
 {
  if (ship[ea][e].type == SHIP_NONE)
   continue;
  ee = &ship[ea][e];
  size = eclass[ee->type].size;
  if (eclass[ee->type].ship_class == ECLASS_WSHIP)
  {
   if (x + bsize > ee->x - size
    && x - bsize < ee->x + size
    && y + bsize > ee->y - size
    && y - bsize < ee->y + size)
    {
     if (xyedist_test_less(x, y, ea, e, size + bsize))
     {
      hit = check_collision_mask(ee, ea, e, ba, b, x, y, BEAM_YES);
      if (hit == -1) // hits shield - note that the pulse graphic thing is done in the collision mask function
      {
/*       if (pb->owner == -1)
       {
        player[0].target_a = ea;
        player[0].target_e = e;
       }
       if (pb->owner == -2)
       {
        player[1].target_a = ea;
        player[1].target_e = e;
       }*/
       hurt_wship_shield(ea, e, pb->damage, pb->owner, ba, b);
       pb->status = i;
       pb->status3 = BEAM_HIT_SHIELD;
       return 1;
      }
      if (hit > -1)
      {
/*       if (pb->owner == -1)
       {
        player[0].target_a = ea;
        player[0].target_e = e;
       }
       if (pb->owner == -2)
       {
        player[1].target_a = ea;
        player[1].target_e = e;
       }*/
       hurt_wship_part(ea, e, pb->damage, pb->owner, ba, b, hit);
       pb->status = i;
       pb->status3 = BEAM_HIT_HULL;
       return 1;
      }
     }
    }
    continue;
  }
  if (x + bsize > ee->x - size
   && x - bsize < ee->x + size
   && y + bsize > ee->y - size
   && y - bsize < ee->y + size)
   {
    if (xyedist_test_less(x, y, ea, e, size + bsize)) // just use a simple circular hit zone for fighters, for now at least.
    {
//     bullet_explodes(b, e);
/*       if (pb->owner == -1)
       {
        player[0].target_a = ea;
        player[0].target_e = e;
       }
       if (pb->owner == -2)
       {
        player[1].target_a = ea;
        player[1].target_e = e;
       }*/
     if (ship[ea][e].shield_up)
      pb->status3 = BEAM_HIT_SHIELD;
       else
        pb->status3 = BEAM_HIT_HULL;
     hurt_ship_fighter(ea, e, pb->damage, ba, b, BEAM_YES, x, y);
//     hurt_ship_fighter(ea, e, pb->damage, pb->owner, ba, b);
     pb->status = i;
     return 1;
    }
   }
 }

 if (ba == TEAM_ENEMY)
 {
  int p;

  for (p = 0; p < arena.players; p ++)
  {
   if (PP.alive == 0)
    continue;
   size = 8000;
   if (x + bsize > PP.x - size
    && x - bsize < PP.x + size
    && y + bsize > PP.y - size
    && y - bsize < PP.y + size)
   {
    if (xypdist(x, y, p) <= size + (bsize<<10)) // just use a simple circular hit zone for players, for now at least.
    {
     hurt_player(p, pb->damage, pb->owner, ba, b, BEAM_YES, x, y);
     pb->status = i;
     return 1;
    }
   }

  }

 }

 }
 return 0;

}

int hurt_player(int p, int dam, int owner, int ba, int b, int beam, int bx, int by)
{
// note that bx and by are not exactly the same as the bullet's actual x and y
// return 0;
 if (PP.shield > 0 && PP.shield_up)
 {
  PP.shield -= dam;
  PP.shield_flash = 5;
  if (PP.shield <= 0)
  {
   add_hitpulse(p, 1, 0, PP.shield + dam, beam); // needs to know the negative shield value
   PP.shield = 0;
   PP.shield_up = 0;
   indicator(WAV_BLIP_L, NOTE_2C, 210, p);

     int i, c;
     int angle = grand(ANGLE_1);
     int dist;
     int speed;
     for (i = 0; i < 8; i ++)
     {
//       angle = grand(ANGLE_1);
       angle += ANGLE_8 + grand(ANGLE_16);
       dist = 6000 + grand(6000);
       speed = dist>>2;
       c = quick_cloud(CLOUD_EX_SHIELD,
            PP.x + xpart(angle, dist),
            PP.y + ypart(angle, dist),
            PP.x_speed + xpart(angle, speed),
            PP.y_speed + ypart(angle, speed),
            8 + grand(5), 2, 0);
       if (c != -1)
       {
        CC.status = 3 + grand(3);
        CC.angle = angle;
        CC.drag = 980;
       }
     }
     if (beam)
      quick_cloud(CLOUD_SMALL_SHOCK, PP.x, PP.y, 0, 0, SMALL_SHOCK_TIME, 2, 0);
       else
        quick_cloud(CLOUD_SMALL_SHOCK, bx, by, 0, 0, SMALL_SHOCK_TIME, 2, 0);



  }
   else
    add_hitpulse(p, 1, PP.shield, PP.shield + dam, beam);

  int pl, angle_diff, dist;
  struct playerstruct* pp;
  pp = &player[p];
  char found_beam = 0;


               dist = 6;//xypdist(bx, by, p) >> 10;
               int angle = radians_to_angle(atan2(by - pp->y, bx - pp->x));
               angle_diff = angle - pp->angle - ANGLE_4;


           if (beam)
           {
// first check for an spulse reserved for this beam:
            for (pl = 0; pl < PULSE; pl ++)
            {
             if (pp->spulse_time [pl] >= 0
              && pp->spulse_beam_e [pl] == bullet[ba][b].owner
              && pp->spulse_beam_t [pl] == bullet[ba][b].owner_t)
             {
              pp->spulse_angle [pl] = angle_diff;
              pp->spulse_dist [pl] = dist;
              pp->spulse_time [pl] = 60 + grand(50);
              pp->spulse_time_delta [pl] = 10;
              pp->spulse_beam_e [pl] = bullet[ba][b].owner;
              pp->spulse_beam_t [pl] = bullet[ba][b].owner_t;
              found_beam = 1;
              break;
             }
            }
// if none found, reserve a new one:
            if (!found_beam)
            {
             for (pl = 0; pl < PULSE; pl ++)
             {
              if (pp->spulse_time [pl] <= 0 || pl == PULSE - 1)
              {
               pp->spulse_angle [pl] = angle_diff;
               pp->spulse_dist [pl] = dist;
               pp->spulse_time [pl] = 100;// + grand(12);
               pp->spulse_time_delta [pl] = 10;
               pp->spulse_beam_e [pl] = bullet[ba][b].owner;
               pp->spulse_beam_t [pl] = bullet[ba][b].owner_t;
               break;
              }
             }
            }
           }
              else // not a beam
              {
               for (pl = 0; pl < PULSE; pl ++)
               {
                if (pp->spulse_time [pl] <= 0 || pl == PULSE - 1)
                {
                  pp->spulse_angle [pl] = angle_diff;
                  pp->spulse_dist [pl] = dist;
                  pp->spulse_time [pl] = 100;// + grand(12);
                  pp->spulse_time_delta [pl] = 10;
                  play_effectwfv(WAV_SHIELD, 1000, 120);
                  break;
                }
               }
              }


  return 0; // absorbed by shield
 }

 PP.hp -= dam;

 if (!beam)
  play_effectwfv(WAV_HIT_HULL, 1200, 150);

 if (PP.hp <= 0)
 {
  add_hitpulse(p, 0, 0, PP.hp + dam, beam); // needs to know negative hp value to calculate end value
  PP.hp = 0;
  player_explodes(p);
 }
  else
   add_hitpulse(p, 0, PP.hp, PP.hp + dam, beam);

 return 1; // hit hull

}

void add_hitpulse(int p, int type, int start, int end, char beam)
{

 int i;

 if (beam)
 {
  int lowest = 100000;
  for (i = 0; i < HPULSE; i ++)
  {
// if it's a beam, and the player was hit most recently by a beam, adds to the beam hitpulse
//  instead of starting a new one. also resets thickness.
   if (PP.hitpulse_thickness [type] [i] > 0
    && PP.hitpulse_beam [type] [i]
    && PP.hitpulse_start [type] [i] < lowest)
   {
    PP.hitpulse_start [type] [i] = start;
// don't change end
    PP.hitpulse_thickness [type] [i] = 30; // reset
    return;
   }
// if no existing beam found, fall through:
  }
 }

 for (i = 0; i < HPULSE; i ++)
 {
  if (PP.hitpulse_thickness [type] [i] <= 0)
  {
   PP.hitpulse_start [type] [i] = start;
   PP.hitpulse_end [type] [i] = end;
   PP.hitpulse_thickness [type] [i] = 30;
   PP.hitpulse_beam [type] [i] = 0;
   if (beam)
    PP.hitpulse_beam [type] [i] = 1;
   return;
  }

 }


}

void player_explodes(int p)
{

 int x = PP.x;
 int y = PP.y;
 int c, i, dist;

 quick_cloud(CLOUD_LARGE_SHOCK, x, y, 0, 0, LARGE_SHOCK_TIME, 0, 0);

 int angle = grand(ANGLE_1);

 for (i = 0; i < 5; i ++)
 {
    c = simple_cloud(CLOUD_SYNCHFLARE, x, y, 0, 15 + grand(5));
    if (c != -1)
    {
     cloud[c].x_speed = PP.x_speed + xpart(angle, 6000 + grand(6000));
     cloud[c].y_speed = PP.y_speed + ypart(angle, 6000 + grand(6000));
     CC.x2 = PP.x_speed;
     CC.y2 = PP.y_speed;
     cloud[c].drag = 940;
    }
    angle += ANGLE_8 + grand(ANGLE_3);
 }


 dist = 0;

 for (i = 0; i < 9; i ++)
 {
        c = quick_cloud(CLOUD_FADEBALL,
             x + xpart(angle, dist),
             y + ypart(angle, dist),
             0, 0, 35 + grand(10), 0, 0);
        if (c != -1)
        {
         CC.status = 10 + grand(8);
        }

 angle = grand(ANGLE_1);
 dist = 20000 + grand(20000);


 }

 play_effectwfvxy_xs_ys(WAV_BASIC, SPRIORITY_HIGH, 900 + grand(200), 240, PP.x, PP.y, 0, 0);


 PP.alive = 0;
 PP.turning = 0;
 PP.x_speed = 0;
 PP.y_speed = 0;
 player[0].ships_lost ++;
 if (player[0].ships <= 0)
 {
   if (arena.players == 1)
    arena.game_over = 200;
     else
     {
      if (player[p^1].alive == 0)
      {
       arena.game_over = 200;
       player[0].respawning = 0;
       player[1].respawning = 0;
      }
     }
 }

 PP.respawning = 150;


 int e, t;
 int a = TEAM_ENEMY;
 int ptarget = TARGET_P1;
 if (p == 1)
  ptarget = TARGET_P2;

 for (e = 0; e < NO_SHIPS; e ++)
 {
  if (EE.target == ptarget)
   EE.target = TARGET_NONE;
  if (eclass[EE.type].turrets > 0)
  {
   for (t = 0; t < eclass[EE.type].turrets; t ++)
   {
    if (EE.turret_target [t] == ptarget)
      EE.turret_target [t] = TARGET_NONE; // doesn't matter if we clear target for destroyed turret
   }
  }

 }

}


void bullet_explodes(int a, int b, int hit_a, int hit_e, int hit_hull)
{
//destroy_bullet(a, b);
 static struct bulletstruct *eb;

 eb = &bullet[a][b];

 int x = eb->x;
 int y = eb->y;
 int angle, i, dist, size, c;
 int target_x = 0, target_y = 0, target_x_speed = 0, target_y_speed = 0;

// int target_shielded;

 if (hit_a >= 0)
 {
  if (hit_e >= 0)
  {
   target_x = ship[hit_a][hit_e].x;
   target_y = ship[hit_a][hit_e].y;
   target_x_speed = ship[hit_a][hit_e].x_speed;
   target_y_speed = ship[hit_a][hit_e].y_speed;
//   target_shielded = ship[hit_a][hit_e].shield_up;
  }
   else
   {
     int p;

     if (hit_e == -1)
      p = 0;
       else
        p = 1;
      target_x = PP.x;
      target_y = PP.y;
      target_x_speed = PP.x_speed;
      target_y_speed = PP.y_speed;

   }
 }

 switch(eb->type)
 {
/*  case bullet_MINE:
   simple_cloud(CLOUD_BALL_COL3, x, y, eb->colour, 30 + (bullet[a][b].damage >> 7));
   simple_cloud(CLOUD_BALL, x, y, eb->colour, 20 + (bullet[a][b].damage >> 8));
   quick_cloud(CLOUD_LARGE_SHOCK, x, y, 0, 0, 50, eb->colour, 0);
   bullet[a][b].timeout = 90;
   bullet[a][b].x_speed = 0;
   bullet[a][b].y_speed = 0;
   bullet[a][b].type = bullet_MINE_BLAST;
   for (i = 0; i < MINE_BITS; i ++)
   {
    bullet[a][b].bit_dist [i] = 1 + grand(20) + grand(20) + grand(20) + grand(bullet[a][b].damage >> 7);
    bullet[a][b].bit_angle [i] = grand(ANGLE_1);
    bullet[a][b].bit_time [i] = 50 + grand(40);

    if (i & 1)
    {
     bullet[a][b].bit_spin [i] = 45 + grand(45);
    }
     else
      bullet[a][b].bit_spin [i] = -45 - grand(45);
   }

   int freq = 1000 - bullet[a][b].damage /9;
   if (freq < 300)
    freq = 300;

   freq = 1000;

   play_effectwfvxy(WAV_PHASEY, freq, 150, bullet[a][b].x, bullet[a][b].y);

   bullet[a][b].damage = 0;

   return; // note this return*/

  case BULLET_SHOT:
  if (hit_a >= 0 && hit_hull)
  {
   if (hit_hull)
   {
    c = simple_cloud(CLOUD_DOUBLE_BALL, x, y, BL.colour, 40 + grand(10));
    if (c != -1)
    {
     cloud[c].x_speed = target_x_speed;
     cloud[c].y_speed = target_y_speed;
    }
   }
   angle = flashback_angle(target_x, target_y, eb->x, eb->y, eb->x_speed, eb->y_speed);
   angle += grand(ANGLE_32) - grand(ANGLE_32);
   for (i = 0; i < 5; i ++)
   {
    x += xpart(angle, 4000) + xpart(grand(ANGLE_1), 2000);
    y += ypart(angle, 4000) + ypart(grand(ANGLE_1), 2000);
    c = simple_cloud(CLOUD_FADEBALL, x, y, eb->colour, 10 + grand(10) + i);
    if (c != -1)
    {
     CC.x_speed = target_x_speed + xpart(angle, i * 300);
     CC.y_speed = target_y_speed + ypart(angle, i * 300);
     CC.status = 7 + i;
    }
   }

  }
    else
    {
     if (hit_a < 0) // hit nothing
     {
      c = simple_cloud(CLOUD_FADEBALL, x, y, eb->colour, 13);
      if (c != -1)
      {
       CC.x_speed = 0;//eb->x_speed;
       CC.y_speed = 0;//eb->y_speed;
       CC.status = 8;
      }
     }
    }
   break;



  case BULLET_BLAST:
  if (hit_a >= 0 && hit_hull)
  {
   if (hit_hull)
   {
    c = simple_cloud(CLOUD_DOUBLE_BALL, x, y, BL.colour, 50 + grand(20));
    if (c != -1)
    {
     cloud[c].x_speed = target_x_speed;
     cloud[c].y_speed = target_y_speed;
    }
   }
   angle = flashback_angle(target_x, target_y, eb->x, eb->y, eb->x_speed, eb->y_speed);
   angle += grand(ANGLE_32) - grand(ANGLE_32);
   for (i = 0; i < 5; i ++)
   {
    x += xpart(angle, 5000) + xpart(grand(ANGLE_1), 2000);
    y += ypart(angle, 5000) + ypart(grand(ANGLE_1), 2000);
    c = simple_cloud(CLOUD_FADEBALL, x, y, eb->colour, 20 + grand(10) + i);
    if (c != -1)
    {
     CC.x_speed = target_x_speed + xpart(angle, i * 500);
     CC.y_speed = target_y_speed + ypart(angle, i * 500);
     CC.status = 7 + i;
    }
   }

  }
    else
    {
     if (hit_a < 0) // hit nothing
     {
      c = simple_cloud(CLOUD_FADEBALL, x, y, eb->colour, 23);
      if (c != -1)
      {
       CC.x_speed = 0;//eb->x_speed;
       CC.y_speed = 0;//eb->y_speed;
       CC.status = 11;
      }
     }
      else // must have hit shield
      {
       c = simple_cloud(CLOUD_FADEBALL, x - BL.x_speed, y - BL.y_speed, eb->colour, 23 + grand(15));
       if (c != -1)
       {
        CC.x_speed = 0;//eb->x_speed;
        CC.y_speed = 0;//eb->y_speed;
        CC.status = 11;
       }
      }

    }
   break;




//  case BULLET_SHOT:
//  c = simple_cloud(CLOUD_BALL, x, y, eb->colour, 20);
  if (hit_a >= 0)
  {
   c = simple_cloud(CLOUD_BALL, x, y, eb->colour, 13 + grand(15));
   if (c != -1)
   {
    CC.x_speed = target_x_speed;
    CC.y_speed = target_y_speed;
   }
//    c = simple_cloud(CLOUD_FLARE, BL.x + xpart(angle, dist), BL.y + ypart(angle, dist), BL.colour, 10 + grand(10));
    c = simple_cloud(CLOUD_DOUBLE_BALL, BL.x + xpart(angle, dist), BL.y + ypart(angle, dist), BL.colour, 20 + grand(20));
    if (c != -1)
    {
     angle = radians_to_angle(atan2(eb->y - ship[hit_a][hit_e].y, eb->x - ship[hit_a][hit_e].x));
     cloud[c].x_speed = target_x_speed + xpart(angle, 3000);
     cloud[c].y_speed = target_y_speed + ypart(angle, 3000);
     cloud[c].drag = 970;
    }


  }
    else
    {
     c = simple_cloud(CLOUD_BALL_COL3, x, y, eb->colour, 13);
     if (c != -1)
     {
      CC.x_speed = 0;//eb->x_speed;
      CC.y_speed = 0;//eb->y_speed;
     }
    }
   break;


  case BULLET_AWS_MISSILE:
  if (hit_a >= 0)
  {
   size = 0;
   if (hit_hull)
   {
    size = 8;
    c = simple_cloud(CLOUD_DOUBLE_BALL, x, y, BL.colour, 60 + grand(10));
    if (c != -1)
    {
     cloud[c].x_speed = target_x_speed;
     cloud[c].y_speed = target_y_speed;
    }
//    play_effectwfvxy_xs_ys(WAV_HIT_HULL, SPRIORITY_LOW, 1300, 90, eb->x, eb->y, eb->x_speed, eb->y_speed);

   }
   angle = flashback_angle(target_x, target_y, eb->x, eb->y, eb->x_speed, eb->y_speed);
   angle += grand(ANGLE_32) - grand(ANGLE_32);
   for (i = 0; i < 6; i ++)
   {
    x += xpart(angle, 4000) + xpart(grand(ANGLE_1), 2000);
    y += ypart(angle, 4000) + ypart(grand(ANGLE_1), 2000);
    c = simple_cloud(CLOUD_FADEBALL, x, y, eb->colour, 9 + size + grand(9 + size) + i);
    if (c != -1)
    {
     CC.x_speed = target_x_speed + xpart(angle, i * (240 + size*20));
     CC.y_speed = target_y_speed + ypart(angle, i * (240 + size*20));
     CC.status = 3 + size + i;
    }
   }

  }
    else
    {
     c = simple_cloud(CLOUD_FADEBALL, x, y, eb->colour, 25);
     if (c != -1)
     {
      CC.x_speed = 0;//eb->x_speed;
      CC.y_speed = 0;//eb->y_speed;
      CC.status = 8;
     }
    }
   play_effectwfvxy_xs_ys(WAV_BBANG, SPRIORITY_LOW, 1200, 100, BL.x, BL.y, BL.x_speed, BL.y_speed);
   break;

  case BULLET_AF_MISSILE:
  case BULLET_HOMING:
  case BULLET_ROCKET:
  case BULLET_ROCKET2:
  if (hit_a >= 0)
  {
   if (hit_hull)
   {
    c = simple_cloud(CLOUD_DOUBLE_BALL, x, y, BL.colour, 40 + grand(10));
    if (c != -1)
    {
     cloud[c].x_speed = target_x_speed;
     cloud[c].y_speed = target_y_speed;
    }
   }
   angle = flashback_angle(target_x, target_y, eb->x, eb->y, eb->x_speed, eb->y_speed);
   angle += grand(ANGLE_32) - grand(ANGLE_32);
   for (i = 0; i < 6; i ++)
   {
    x += xpart(angle, 4000) + xpart(grand(ANGLE_1), 2000);
    y += ypart(angle, 4000) + ypart(grand(ANGLE_1), 2000);
    c = simple_cloud(CLOUD_FADEBALL, x, y, eb->colour, 14 + grand(14) + i);
    if (c != -1)
    {
     CC.x_speed = target_x_speed + xpart(angle, i * 340);
     CC.y_speed = target_y_speed + ypart(angle, i * 340);
     CC.status = 7 + i;
    }
   }

  }
    else
    {
     c = simple_cloud(CLOUD_FADEBALL, x, y, eb->colour, 25);
     if (c != -1)
     {
      CC.x_speed = 0;//eb->x_speed;
      CC.y_speed = 0;//eb->y_speed;
      CC.status = 8;
     }
    }
   play_effectwfvxy_xs_ys(WAV_BBANG, SPRIORITY_LOW, 1400, 80, BL.x, BL.y, BL.x_speed, BL.y_speed);
   break;


  case BULLET_ELONGSHOT:
  if (hit_a >= 0)
  {
    c = simple_cloud(CLOUD_FADEBALL, x, y, eb->colour, 35);
    if (c != -1)
    {
     cloud[c].x_speed = target_x_speed;
     cloud[c].y_speed = target_y_speed;
     CC.status = 8;
    }
  }
    else
    {
     c = simple_cloud(CLOUD_FADEBALL, x, y, eb->colour, 35);
     if (c != -1)
     {
      CC.x_speed = 0;//eb->x_speed;
      CC.y_speed = 0;//eb->y_speed;
      CC.status = 8;
     }
    }
   break;

  case BULLET_ESHOT1:
  case BULLET_ESHOT2:
  if (hit_a >= 0)
  {
    c = simple_cloud(CLOUD_FADEBALL, x, y, eb->colour, 25);
    if (c != -1)
    {
     cloud[c].x_speed = target_x_speed;
     cloud[c].y_speed = target_y_speed;
     CC.status = 6;
    }
  }
    else
    {
     c = simple_cloud(CLOUD_FADEBALL, x, y, eb->colour, 15);
     if (c != -1)
     {
      CC.x_speed = 0;//eb->x_speed;
      CC.y_speed = 0;//eb->y_speed;
      CC.status = 6;
     }
    }
   break;





//  case BULLET_SHOT:
//  c = simple_cloud(CLOUD_BALL, x, y, eb->colour, 20);
  if (hit_a >= 0)
  {
   c = simple_cloud(CLOUD_BALL, x, y, eb->colour, 13 + grand(15));
   if (c != -1)
   {
    CC.x_speed = target_x_speed;
    CC.y_speed = target_y_speed;
   }
//    c = simple_cloud(CLOUD_FLARE, BL.x + xpart(angle, dist), BL.y + ypart(angle, dist), BL.colour, 10 + grand(10));
    c = simple_cloud(CLOUD_DOUBLE_BALL, BL.x + xpart(angle, dist), BL.y + ypart(angle, dist), BL.colour, 20 + grand(20));
    if (c != -1)
    {
     angle = radians_to_angle(atan2(eb->y - ship[hit_a][hit_e].y, eb->x - ship[hit_a][hit_e].x));
     cloud[c].x_speed = target_x_speed + xpart(angle, 3000);
     cloud[c].y_speed = target_y_speed + ypart(angle, 3000);
     cloud[c].drag = 970;
    }


  }
    else
    {
     c = simple_cloud(CLOUD_BALL_COL3, x, y, eb->colour, 13);
     if (c != -1)
     {
      CC.x_speed = 0;//eb->x_speed;
      CC.y_speed = 0;//eb->y_speed;
     }
    }
   break;
  case BULLET_ETORP1:
  case BULLET_BIGSHOT:
  case BULLET_EBIGSHOT:
  case BULLET_OLDSHOT:
//  c = simple_cloud(CLOUD_BALL, x, y, eb->colour, 20);


  if (hit_a >= 0)
  {
//   c = simple_cloud(CLOUD_DOUBLE_BALL, x, y, eb->colour, 33 + grand(15));
/*   c = simple_cloud(CLOUD_FLASH, x, y, eb->colour, 12);
   if (c != -1)
   {
    CC.x_speed = ship[hit_a][hit_e].x_speed;
    CC.y_speed = ship[hit_a][hit_e].y_speed;
    CC.status = 30 + grand(10);
    CC.status2 = 1;
   }*/
    quick_cloud(CLOUD_SMALL_SHOCK, x, y, 0, 0, SMALL_SHOCK_TIME, eb->colour, 0);

    if (!hit_hull)
    {
     c = simple_cloud(CLOUD_BALL_COL3, x, y, eb->colour, 20 + grand(20));
     if (c != -1)
     {
      CC.x_speed = 0;//eb->x_speed;
      CC.y_speed = 0;//eb->y_speed;
     }

    }
     else
     {
      c = simple_cloud(CLOUD_SYNCHFLARE, x, y, eb->colour, 15);
      if (c != -1)
      {
       angle = flashback_angle(target_x, target_y, eb->x, eb->y, eb->x_speed, eb->y_speed);
       CC.x_speed = target_x_speed + xpart(angle, 9000);
       CC.y_speed = target_y_speed + ypart(angle, 9000);
       CC.x2 = target_x_speed;
       CC.y2 = target_y_speed;
       CC.drag = 940;
      }
//     play_effectwfvxy_xs_ys(WAV_HIT_HULL, SPRIORITY_LOW, 500, 100, eb->x, eb->y, eb->x_speed, eb->y_speed);
     }


/*    c = simple_cloud(CLOUD_FLARE, BL.x, BL.y, BL.colour, 30 + grand(20));
//    c = simple_cloud(CLOUD_DOUBLE_BALL, BL.x + xpart(angle, dist), BL.y + ypart(angle, dist), BL.colour, 20 + grand(20));
    if (c != -1)
    {
     angle = flashback_angle(ship[hit_a][hit_e].x, ship[hit_a][hit_e].y, eb->x, eb->y, eb->x_speed, eb->y_speed);
     cloud[c].x_speed = ship[hit_a][hit_e].x_speed + xpart(angle, 4000);
     cloud[c].y_speed = ship[hit_a][hit_e].y_speed + ypart(angle, 4000);
     cloud[c].drag = 940;
    }*/


  }
    else
    {
/*
   c = simple_cloud(CLOUD_FLASH, x, y, eb->colour, 12);
   if (c != -1)
   {
    CC.x_speed = 0;//ship[hit_a][hit_e].x_speed;
    CC.y_speed = 0;//ship[hit_a][hit_e].y_speed;
    CC.status = 20 + grand(8);
    CC.status2 = 1;
   }*/

   quick_cloud(CLOUD_SMALL_SHOCK, x, y, 0, 0, SMALL_SHOCK_TIME, eb->colour, 0);

    c = simple_cloud(CLOUD_SYNCHFLARE, x, y, eb->colour, 15);
    if (c != -1)
    {

     CC.x_speed = 0;//target_x_speed;
     CC.y_speed = 0;//target_y_speed;
     CC.x2 = 0;
     CC.y2 = 0;
     CC.drag = 940;
    }
//     play_effectwfvxy_xs_ys(WAV_HIT_HULL, SPRIORITY_LOW, 600, 50, eb->x, eb->y, eb->x_speed, eb->y_speed);


/*    c = simple_cloud(CLOUD_FLARE, BL.x, BL.y, BL.colour, 30 + grand(20));
    if (c != -1)
    {
     angle = flashback_angle(ship[hit_a][hit_e].x, ship[hit_a][hit_e].y, eb->x, eb->y, eb->x_speed, eb->y_speed);
     cloud[c].x_speed = 0;
     cloud[c].y_speed = 0;
     cloud[c].drag = 970;
    }*/

/*        c = simple_cloud(CLOUD_BALL_COL3, x, y, eb->colour, 13);
     if (c != -1)
     {
      CC.x_speed = 0;//eb->x_speed;
      CC.y_speed = 0;//eb->y_speed;
     }*/
    }
   play_effectwfvxy_xs_ys(WAV_BBANG, SPRIORITY_LOW, 900, 100, BL.x, BL.y, BL.x_speed, BL.y_speed);
   break;

  case BULLET_PTORP1:
//  c = simple_cloud(CLOUD_BALL, x, y, eb->colour, 20);
  size = 0;
  if (!hit_hull)
   size = 7;
  if (hit_a >= 0)
  {
    if (BL.status == 0) // has primed
    {
     quick_cloud(CLOUD_SMALL_SHOCK, x, y, 0, 0, SMALL_SHOCK_TIME, eb->colour, 0);

     c = simple_cloud(CLOUD_SYNCHFLARE, x, y, eb->colour, size + 13);
     if (c != -1)
     {
      angle = flashback_angle(target_x, target_y, eb->x, eb->y, eb->x_speed, eb->y_speed);
      CC.x_speed = target_x_speed + xpart(angle, 2500 + size * 1000);
      CC.y_speed = target_y_speed + ypart(angle, 2500 + size * 1000);
      CC.x2 = target_x_speed;
      CC.y2 = target_y_speed;
      CC.drag = 940;
     }
    }
     else
     {
      c = simple_cloud(CLOUD_SYNCHFLARE, x, y, eb->colour, size + 5);
      if (c != -1)
      {
       angle = flashback_angle(target_x, target_y, eb->x, eb->y, eb->x_speed, eb->y_speed);
       CC.x_speed = target_x_speed + xpart(angle, 2000 + size * 600);
       CC.y_speed = target_y_speed + ypart(angle, 2000 + size * 600);
       CC.x2 = target_x_speed;
       CC.y2 = target_y_speed;
       CC.drag = 940;
      }

     }



  }
    else
    {
    if (BL.status == 0)
     quick_cloud(CLOUD_SMALL_SHOCK, x, y, 0, 0, SMALL_SHOCK_TIME, eb->colour, 0);

    c = simple_cloud(CLOUD_SYNCHFLARE, x, y, eb->colour, 15);
    if (c != -1)
    {

     CC.x_speed = 0;//target_x_speed;
     CC.y_speed = 0;//target_y_speed;
     CC.x2 = 0;
     CC.y2 = 0;
     CC.drag = 940;
    }

    }
   play_effectwfvxy_xs_ys(WAV_BBANG, SPRIORITY_LOW, 1000, 200, BL.x, BL.y, BL.x_speed, BL.y_speed);
   break;


/*  case bullet_SCATTER:
   if (hit_a >= 0)
    simple_cloud(CLOUD_BALL_COL3, x, y, eb->colour, 20 + (bullet[a][b].damage >> 6));
   break;
  case bullet_WORM:
  case bullet_ROCKET:
  case bullet_MISSILE:
   if (hit_a >= 0)
    simple_cloud(CLOUD_BALL_COL3, x, y, eb->colour, 45);
     else
      simple_cloud(CLOUD_BALL_COL2, x, y, eb->colour, 25);
   break;
  case bullet_BASIC:
   size = bullet[a][b].damage;
   if (hit_a >= 0)
    size /= 3;
   simple_cloud(CLOUD_BALL_COL3, x, y, eb->colour, 20 + (size >> 6));
   simple_cloud(CLOUD_BALL, x, y, eb->colour, 10 + (size >> 5));
   angle = grand(ANGLE_1);

   for (i = 0; i < 15; i ++)
   {
    angle += ANGLE_16 + grand(ANGLE_32);
    dist = 5000;
    speed = 2000 + grand(3000) + (size >> 2);
    quick_cloud(CLOUD_2BALL, x + xpart(angle, dist), y + ypart(angle, dist), xpart(angle, speed), ypart(angle, speed), 15 + grand(15) + (size >> 6), eb->colour, 940);
   }
   break;*/
 }

// bullet might not get here if transformed
 destroy_bullet(a, b);
}

int flashback_angle(int ship_x, int ship_y, int bx, int by, int bx_speed, int by_speed)
{

 bx -= bx_speed * 3;
 by -= by_speed * 3;

 return radians_to_angle(atan2(by - ship_y, bx - ship_x));

}

/*
void run_etorp(int a, int b)
{

    int old_x = BL.x;
    int old_y = BL.y;

//    bullet[a][b].x_speed += xpart(bullet[a][b].angle, 300 + (bullet[a][b].status2 << 4));
//    bullet[a][b].y_speed += ypart(bullet[a][b].angle, 300 + (bullet[a][b].status2 << 4));
    BL.x_speed += xpart(BL.angle, 50);
    BL.y_speed += ypart(BL.angle, 50);
    BL.x += BL.x_speed;
    BL.y += BL.y_speed;

//    drag_bullet(b, 890);
//    drag_bullet(b, 950);
//    drag_bullet(b, 950);

    int c = create_cloud(CLOUD_LINE_TRAIL);
    if (c != -1)
    {
     cloud[c].x = BL.x;
     cloud[c].y = BL.y;
     cloud[c].x2 = old_x;
     cloud[c].y2 = old_y;
     cloud[c].x_speed = 0;
     cloud[c].y_speed = 0;
     cloud[c].timeout = 8;//bullet[a][b].status;// + bullet[a][b].level;
     cloud[c].fangle = atan2(BL.y - old_y, BL.x - old_x);
     cloud[c].status = (int) hypot(BL.y - old_y, BL.x - old_x) >> 10;
    }

  {
//     if (hs[0].alive)
//      bullet[a][b].angle = turn_towards_xy(bullet[a][b].x, bullet[a][b].y, hs[0].x, hs[0].y, bullet[a][b].angle, 22 + (bullet[a][b].status2 >> 0)); // 22
  }

//    BL.timeout --;


    if (BL.timeout <= 0)
    {
     bullet_explodes(a, b, -1, -1);
     destroy_bullet(a, b);
    }

}
*/

void run_ptorp(int a, int b)
{

//    int old_x = BL.x2;
//    int old_y = BL.y2;

    int cloudtime = 12;
    int accel = 30;

    if (BL.status > 0)
    {
     BL.status --;
     if (BL.status == 0)
     {
      BL.damage = BL.status2;
      BL.force <<= 2;
      play_effectwfvxy_xs_ys(WAV_WHOOSH, SPRIORITY_LOW, 700, 220, BL.x, BL.y, BL.x_speed, BL.y_speed);
     }
     cloudtime = 6;
     accel = 5;
    }


    BL.x_speed += xpart(BL.angle, 40);
    BL.y_speed += ypart(BL.angle, 40);
    BL.x += BL.x_speed;
    BL.y += BL.y_speed;

//BL.x2 = BL.x;
//BL.y2 = BL.y;


//    bullet[a][b].x_speed += xpart(bullet[a][b].angle, 300 + (bullet[a][b].status2 << 4));
//    bullet[a][b].y_speed += ypart(bullet[a][b].angle, 300 + (bullet[a][b].status2 << 4));


//    drag_bullet(b, 890);
//    drag_bullet(b, 950);
//    drag_bullet(b, 950);


    int c = create_cloud(CLOUD_SEEKER_TRAIL);
    if (c != -1)
    {
     cloud[c].x_speed = BL.x3;
     cloud[c].y_speed = BL.y3;
     cloud[c].x = BL.x - xpart(BL.angle, 5000) - CC.x_speed;
     cloud[c].y = BL.y - ypart(BL.angle, 5000) - CC.y_speed;

//     BL.x2 = CC.x;
//     BL.y2 = CC.y;

//     cloud[c].x2 = old_x;
//     cloud[c].y2 = old_y;
     cloud[c].timeout = cloudtime;//bullet[a][b].status;// + bullet[a][b].level;
     cloud[c].fangle = atan2(CC.y - BL.y2, CC.x - BL.x2);
//     cloud[c].fangle = atan2(CC.y - old_y, CC.x - old_x);
     cloud[c].status = (int) hypot(CC.y - BL.y2, CC.x - BL.x2) >> 10;
//     cloud[c].status = (int) hypot(CC.y - old_y, CC.x - old_x) >> 10;
     CC.colour = 0;

     BL.x2 = CC.x + cloud[c].x_speed;
     BL.y2 = CC.y + cloud[c].y_speed;

    }




  {
//     if (hs[0].alive)
//      bullet[a][b].angle = turn_towards_xy(bullet[a][b].x, bullet[a][b].y, hs[0].x, hs[0].y, bullet[a][b].angle, 22 + (bullet[a][b].status2 >> 0)); // 22
  }

//    BL.timeout --;


    if (BL.timeout <= 0)
    {
     bullet_explodes(a, b, -1, -1, 0);
     destroy_bullet(a, b);
    }

}

void run_rocket(int a, int b)
{

//    int old_x = BL.x2;
//    int old_y = BL.y2;

//    int cloudtime = 6;
//    int accel = 30;
    int drag = 990;


    BL.x_speed += xpart(BL.angle, 400);
    BL.y_speed += ypart(BL.angle, 400);
    BL.x += BL.x_speed;
    BL.y += BL.y_speed;
/*
    if (BL.turning < 0)
    {
     BL.angle -= BL.status4;
     BL.status --;
//     drag = 960;
     if (BL.status <= 0)
     {
      if (BL.turning == -1)
      {
       BL.turning = 2;
       BL.status = BL.status2;
      }
      if (BL.turning == -2)
      {
       BL.turning = 0;
      }
     }
    }
     else
     {
      if (BL.turning > 0)
      {
       BL.angle += BL.status4;
       BL.status --;
//       drag = 960;
       if (BL.status <= 0)
       {
        if (BL.turning == 1)
        {
         BL.turning = -2;
         BL.status = BL.status2;
        }
        if (BL.turning == 2)
        {
         BL.turning = 0;
        }
       }
      }
     }*/

    drag_bullet(a, b, drag);

    if (grand(8) == 0)
    {
     int angle = grand(ANGLE_1);
     int accel = 1 + grand(800);
     BL.x_speed += xpart(angle, accel);
     BL.y_speed += ypart(angle, accel);
    }
/*
    if (BL.turning == 0
     && grand(20) == 0)
    {
     BL.turning = pos_or_neg(1);
     BL.status = grand(10) + 2;
     BL.status2 = BL.status;
     BL.status4 = 1 + grand(40);
    }*/

       int c = quick_cloud(CLOUD_LINKLINE,
                           0,
                           0,
                           BL.x_speed - xpart(BL.angle, 5000), BL.y_speed - ypart(BL.angle, 5000), 6, 0, 0);
        if (c != -1)
        {
          // must come after speed is set:
          CC.x = BL.x - xpart(BL.angle, 5000) - CC.x_speed;
          CC.y = BL.y - ypart(BL.angle, 5000) - CC.y_speed;


         if (BL.status3 != -1)
         {
          CC.x2 = cloud[BL.status3].x;
          CC.y2 = cloud[BL.status3].y;
          CC.x3 = cloud[BL.status3].x_speed;
          CC.y3 = cloud[BL.status3].y_speed;
         }
          else
          {
           // must be first cloud for this bullet
           CC.x2 = CC.x + xpart(BL.angle, 3000);
           CC.y2 = CC.y + ypart(BL.angle, 3000);
           CC.x3 = CC.x_speed;
           CC.y3 = CC.y_speed;
          }
        }
        BL.status3 = c; // still works if c == -1



    if (BL.timeout <= 0)
    {
     bullet_explodes(a, b, -1, -1, 0);
     destroy_bullet(a, b);
    }

}


void run_rocket2(int a, int b)
{

    int drag = 1000;

    BL.x_speed += xpart(BL.angle, 350);
    BL.y_speed += ypart(BL.angle, 350);
    BL.x += BL.x_speed;
    BL.y += BL.y_speed;


    drag_bullet(a, b, drag);

    if (grand(8) == 0)
    {
     int angle = grand(ANGLE_1);
     int accel = 1 + grand(800);
     BL.x_speed += xpart(angle, accel);
     BL.y_speed += ypart(angle, accel);
    }


       int c = quick_cloud(CLOUD_LINKLINE,
                           0,
                           0,
                           BL.x_speed - xpart(BL.angle, 5000), BL.y_speed - ypart(BL.angle, 5000), 7, 0, 0);
        if (c != -1)
        {
          // must come after speed is set:
          CC.x = BL.x - xpart(BL.angle, 5000) - CC.x_speed;
          CC.y = BL.y - ypart(BL.angle, 5000) - CC.y_speed;


         if (BL.status3 != -1)
         {
          CC.x2 = cloud[BL.status3].x;
          CC.y2 = cloud[BL.status3].y;
          CC.x3 = cloud[BL.status3].x_speed;
          CC.y3 = cloud[BL.status3].y_speed;
         }
          else
          {
           // must be first cloud for this bullet
           CC.x2 = CC.x + xpart(BL.angle, 3000);
           CC.y2 = CC.y + ypart(BL.angle, 3000);
           CC.x3 = CC.x_speed;
           CC.y3 = CC.y_speed;
          }
        }
        BL.status3 = c; // still works if c == -1



    if (BL.timeout <= 0)
    {
     bullet_explodes(a, b, -1, -1, 0);
     destroy_bullet(a, b);
    }

}




void run_aws_missile(int a, int b)
{

//    int old_x = BL.x2;
//    int old_y = BL.y2;
    int target_x, target_y, dist, speed, target_angle, dist2;

      if (BL.time > 50)
      {
       BL.x_speed += xpart(BL.angle, 500);
       BL.y_speed += ypart(BL.angle, 500);
       drag_bullet(a, b, 965);
      }
       else
       {
        BL.x_speed += xpart(BL.angle, 10);
        BL.y_speed += ypart(BL.angle, 10);
       }

       if (BL.time == 50)
       {
        BL.damage = 5000; // primed
        play_effectwfvxy_xs_ys(WAV_WHOOSH, SPRIORITY_LOW, 1000, 200, BL.x, BL.y, BL.x_speed, BL.y_speed);
       }

    if (BL.target_e != TARGET_NONE)
    {


       BL.angle += BL.turning;
       BL.angle &= ANGLE_MASK;

       dist = hypot(BL.y - ship[a^1][BL.target_e].y, BL.x - ship[a^1][BL.target_e].x);

       if (dist > 40000)
       {
        speed = hypot(BL.y_speed, BL.x_speed);
        dist2 = 0;
        if (speed != 0)
         dist2 = dist / speed;

        target_x = ship[a^1][BL.target_e].x;// + (ship[a^1][BL.target_e].x_speed * dist2);// - BL.x_speed * dist;
        target_y = ship[a^1][BL.target_e].y;// + (ship[a^1][BL.target_e].y_speed * dist2);// - BL.y_speed * dist;

        target_angle = radians_to_angle(atan2(target_y - BL.y, target_x - BL.x));

        if (angle_difference(target_angle, BL.angle) <= BL.status2)
        {
//         BL.turning = 0;
         BL.angle = target_angle;
        }
         else
         {
          BL.turning = delta_turn_towards_angle(BL.angle, target_angle, BL.status2);
         }
       }
        else
        {

         target_x = ship[a^1][BL.target_e].x;
         target_y = ship[a^1][BL.target_e].y;
         BL.turning = delta_turn_towards_xy(BL.x, BL.y, target_x, target_y, BL.angle, BL.status2);
        }


    }
/*     else
     {
       BL.x_speed += xpart(BL.angle, 370);
       BL.y_speed += ypart(BL.angle, 370);
       drag_bullet(a, b, 1000);
     }*/

    BL.x += BL.x_speed;
    BL.y += BL.y_speed;

//BL.x2 = BL.x;
//BL.y2 = BL.y;


//    bullet[a][b].x_speed += xpart(bullet[a][b].angle, 300 + (bullet[a][b].status2 << 4));
//    bullet[a][b].y_speed += ypart(bullet[a][b].angle, 300 + (bullet[a][b].status2 << 4));


//    drag_bullet(b, 890);
//    drag_bullet(b, 950);
//    drag_bullet(b, 950);

       int c = quick_cloud(CLOUD_LINKLINE,
                           0,
                           0,
                           BL.x_speed - xpart(BL.angle, 5000), BL.y_speed - ypart(BL.angle, 5000), 5 + (BL.time>50)*5, 0, 0);
        if (c != -1)
        {
          // must come after speed is set:
          CC.x = BL.x - xpart(BL.angle, 5000) - CC.x_speed;
          CC.y = BL.y - ypart(BL.angle, 5000) - CC.y_speed;


         if (BL.status != -1)
         {
          CC.x2 = cloud[BL.status].x;
          CC.y2 = cloud[BL.status].y;
          CC.x3 = cloud[BL.status].x_speed;
          CC.y3 = cloud[BL.status].y_speed;
         }
          else
          {
           // must be first cloud for this bullet
           CC.x2 = CC.x + xpart(BL.angle, 3000);
           CC.y2 = CC.y + ypart(BL.angle, 3000);
           CC.x3 = CC.x_speed;
           CC.y3 = CC.y_speed;
          }
        }
        BL.status = c; // still works if c == -1

//    BL.timeout --;

    if (grand(4) == 0)
    {
     int angle = grand(ANGLE_1);
     int accel = 1 + grand(800);
     BL.x_speed += xpart(angle, accel);
     BL.y_speed += ypart(angle, accel);
    }

    if (BL.timeout <= 0)
    {
     bullet_explodes(a, b, -1, -1, 0);
     destroy_bullet(a, b);
    }

}



void run_af_missile(int a, int b)
{

//    int old_x = BL.x2;
//    int old_y = BL.y2;
    int target_x, target_y, dist, speed, target_angle, dist2;

    if (BL.target_e != TARGET_NONE)
    {

       BL.x_speed += xpart(BL.angle, 700);
       BL.y_speed += ypart(BL.angle, 700);
       drag_bullet(a, b, 965);

       BL.angle += BL.turning;
       BL.angle &= ANGLE_MASK;

       dist = hypot(BL.y - ship[a^1][BL.target_e].y, BL.x - ship[a^1][BL.target_e].x);

       if (dist > 40000)
       {
        speed = hypot(BL.y_speed, BL.x_speed);
        dist2 = 0;
        if (speed != 0)
         dist2 = dist / speed;

        target_x = ship[a^1][BL.target_e].x;// + (ship[a^1][BL.target_e].x_speed * dist2);// - BL.x_speed * dist;
        target_y = ship[a^1][BL.target_e].y;// + (ship[a^1][BL.target_e].y_speed * dist2);// - BL.y_speed * dist;

        target_angle = radians_to_angle(atan2(target_y - BL.y, target_x - BL.x));

        if (angle_difference(target_angle, BL.angle) <= BL.status2)
        {
//         BL.turning = 0;
         BL.angle = target_angle;
        }
         else
         {
          BL.turning = delta_turn_towards_angle(BL.angle, target_angle, BL.status2);
         }
       }
        else
        {

         target_x = ship[a^1][BL.target_e].x;
         target_y = ship[a^1][BL.target_e].y;
         BL.turning = delta_turn_towards_xy(BL.x, BL.y, target_x, target_y, BL.angle, BL.status2);
        }


    }
     else
     {
       BL.x_speed += xpart(BL.angle, 450);
       BL.y_speed += ypart(BL.angle, 450);
       drag_bullet(a, b, 1000);
     }

    BL.x += BL.x_speed;
    BL.y += BL.y_speed;


       int c = quick_cloud(CLOUD_LINKLINE,
                           0,
                           0,
                           BL.x_speed - xpart(BL.angle, 5000), BL.y_speed - ypart(BL.angle, 5000), 10, 0, 0);
        if (c != -1)
        {
          // must come after speed is set:
          CC.x = BL.x - xpart(BL.angle, 5000) - CC.x_speed;
          CC.y = BL.y - ypart(BL.angle, 5000) - CC.y_speed;


         if (BL.status != -1)
         {
          CC.x2 = cloud[BL.status].x;
          CC.y2 = cloud[BL.status].y;
          CC.x3 = cloud[BL.status].x_speed;
          CC.y3 = cloud[BL.status].y_speed;
         }
          else
          {
           // must be first cloud for this bullet
           CC.x2 = CC.x + xpart(BL.angle, 3000);
           CC.y2 = CC.y + ypart(BL.angle, 3000);
           CC.x3 = CC.x_speed;
           CC.y3 = CC.y_speed;
          }
        }
        BL.status = c; // still works if c == -1

//    BL.timeout --;

    if (grand(4) == 0)
    {
     int angle = grand(ANGLE_1);
     int accel = 1 + grand(800);
     BL.x_speed += xpart(angle, accel);
     BL.y_speed += ypart(angle, accel);
    }

    if (BL.timeout <= 0)
    {
     bullet_explodes(a, b, -1, -1, 0);
     destroy_bullet(a, b);
    }

}


/*
void run_aws_missile(int a, int b)
{

    int old_x = BL.x2;
    int old_y = BL.y2;
    int target_x, target_y, dist, speed, target_angle, dist2;

    if (BL.target_e != TARGET_NONE)
    {

       BL.x_speed += xpart(BL.angle, 650);
       BL.y_speed += ypart(BL.angle, 650);
       drag_bullet(a, b, 980);

       BL.angle += BL.turning;
       BL.angle &= ANGLE_MASK;

       dist = hypot(BL.y - ship[a^1][BL.target_e].y, BL.x - ship[a^1][BL.target_e].x);

       if (dist > 40000)
       {
        speed = hypot(BL.y_speed, BL.x_speed);
        dist2 = 0;
        if (speed != 0)
         dist2 = dist / speed;

        target_x = ship[a^1][BL.target_e].x;// + (ship[a^1][BL.target_e].x_speed * dist2);// - BL.x_speed * dist;
        target_y = ship[a^1][BL.target_e].y;// + (ship[a^1][BL.target_e].y_speed * dist2);// - BL.y_speed * dist;

        target_angle = radians_to_angle(atan2(target_y - BL.y, target_x - BL.x));

        if (angle_difference(target_angle, BL.angle) < 71)
        {
         BL.turning = 0;
        }
         else
         {
          BL.turning = delta_turn_towards_angle(BL.angle, target_angle, 70);
         }
       }
        else
        {

         target_x = ship[a^1][BL.target_e].x;
         target_y = ship[a^1][BL.target_e].y;
         BL.turning = delta_turn_towards_xy(BL.x, BL.y, target_x, target_y, BL.angle, 70);
        }


    }
     else
     {
       BL.x_speed += xpart(BL.angle, 450);
       BL.y_speed += ypart(BL.angle, 450);
       drag_bullet(a, b, 1000);
     }

    BL.x += BL.x_speed;
    BL.y += BL.y_speed;

//BL.x2 = BL.x;
//BL.y2 = BL.y;


//    bullet[a][b].x_speed += xpart(bullet[a][b].angle, 300 + (bullet[a][b].status2 << 4));
//    bullet[a][b].y_speed += ypart(bullet[a][b].angle, 300 + (bullet[a][b].status2 << 4));


//    drag_bullet(b, 890);
//    drag_bullet(b, 950);
//    drag_bullet(b, 950);

       int c = quick_cloud(CLOUD_LINKLINE,
                           0,
                           0,
                           BL.x_speed - xpart(BL.angle, 5000), BL.y_speed - ypart(BL.angle, 5000), 10, 0, 0);
        if (c != -1)
        {
          // must come after speed is set:
          CC.x = BL.x - xpart(BL.angle, 5000) - CC.x_speed;
          CC.y = BL.y - ypart(BL.angle, 5000) - CC.y_speed;


         if (BL.status != -1)
         {
          CC.x2 = cloud[BL.status].x;
          CC.y2 = cloud[BL.status].y;
          CC.x3 = cloud[BL.status].x_speed;
          CC.y3 = cloud[BL.status].y_speed;
         }
          else
          {
           // must be first cloud for this bullet
           CC.x2 = CC.x + xpart(BL.angle, 3000);
           CC.y2 = CC.y + ypart(BL.angle, 3000);
           CC.x3 = CC.x_speed;
           CC.y3 = CC.y_speed;
          }
        }
        BL.status = c; // still works if c == -1

//    BL.timeout --;


    if (BL.timeout <= 0)
    {
     bullet_explodes(a, b, -1, -1);
     destroy_bullet(a, b);
    }

}
*/


/*
void run_aws_missile(int a, int b)
{

    int old_x = BL.x2;
    int old_y = BL.y2;
    int target_x, target_y, dist, speed, target_angle, dist2;

    if (BL.target_e != TARGET_NONE)
    {

       BL.x_speed += xpart(BL.angle, 550);
       BL.y_speed += ypart(BL.angle, 550);
       drag_bullet(a, b, 990);

       BL.angle += BL.turning;
       BL.angle &= ANGLE_MASK;

       dist = hypot(BL.y - ship[a^1][BL.target_e].y, BL.x - ship[a^1][BL.target_e].x);
       speed = hypot(BL.y_speed, BL.x_speed);
       dist2 = 0;
       if (speed != 0)
        dist2 = dist / speed;
       target_x = ship[a^1][BL.target_e].x + (ship[a^1][BL.target_e].x_speed * dist2);// - BL.x_speed * dist;
       target_y = ship[a^1][BL.target_e].y + (ship[a^1][BL.target_e].y_speed * dist2);// - BL.y_speed * dist;

       if (dist > 30000)
       {
        target_angle = radians_to_angle(atan2(target_y - BL.y, target_x - BL.x));
        if (angle_difference(target_angle, BL.angle) < ANGLE_128)
        {
         BL.turning = 0;
        }
         else
         {
          BL.turning += delta_turn_towards_angle(BL.angle, target_angle, 5 + grand(10));
         }
       }
        else
         BL.turning += delta_turn_towards_xy(BL.x, BL.y, target_x, target_y, BL.angle, 5 + grand(10));

//       BL.turning += delta_turn_towards_xy(BL.x, BL.y, target_x, target_y, BL.angle, 5 + grand(10));

//       BL.turning += delta_turn_towards_xy(BL.x, BL.y, ship[a^1][BL.target_e].x, ship[a^1][BL.target_e].y, BL.angle, 5);
       if (BL.turning > 60)
        BL.turning = 60;
       if (BL.turning < -60)
        BL.turning = -60;

//     bullet[a][b].angle = turn_towards_angle(bullet[a][b].angle, tar_angle, 128); // 22
    }
     else
     {
       BL.x_speed += xpart(BL.angle, 450);
       BL.y_speed += ypart(BL.angle, 450);
       drag_bullet(a, b, 1000);
     }

    BL.x += BL.x_speed;
    BL.y += BL.y_speed;

//BL.x2 = BL.x;
//BL.y2 = BL.y;


//    bullet[a][b].x_speed += xpart(bullet[a][b].angle, 300 + (bullet[a][b].status2 << 4));
//    bullet[a][b].y_speed += ypart(bullet[a][b].angle, 300 + (bullet[a][b].status2 << 4));


//    drag_bullet(b, 890);
//    drag_bullet(b, 950);
//    drag_bullet(b, 950);

       int c = quick_cloud(CLOUD_LINKLINE,
                           0,
                           0,
                           BL.x_speed - xpart(BL.angle, 5000), BL.y_speed - ypart(BL.angle, 5000), 10, 0, 0);
        if (c != -1)
        {
          // must come after speed is set:
          CC.x = BL.x - xpart(BL.angle, 5000) - CC.x_speed;
          CC.y = BL.y - ypart(BL.angle, 5000) - CC.y_speed;


         if (BL.status != -1)
         {
          CC.x2 = cloud[BL.status].x;
          CC.y2 = cloud[BL.status].y;
          CC.x3 = cloud[BL.status].x_speed;
          CC.y3 = cloud[BL.status].y_speed;
         }
          else
          {
           // must be first cloud for this bullet
           CC.x2 = CC.x + xpart(BL.angle, 3000);
           CC.y2 = CC.y + ypart(BL.angle, 3000);
           CC.x3 = CC.x_speed;
           CC.y3 = CC.y_speed;
          }
        }
        BL.status = c; // still works if c == -1

//    BL.timeout --;


    if (BL.timeout <= 0)
    {
     bullet_explodes(a, b, -1, -1);
     destroy_bullet(a, b);
    }

}

*/


/*
void run_bullet_seeker(int b)
{

    int old_x = bullet[a][b].x;
    int old_y = bullet[a][b].y;

//    bullet[a][b].x_speed += xpart(bullet[a][b].angle, 300 + (bullet[a][b].status2 << 4));
//    bullet[a][b].y_speed += ypart(bullet[a][b].angle, 300 + (bullet[a][b].status2 << 4));
    bullet[a][b].x_speed += xpart(bullet[a][b].angle, 500);
    bullet[a][b].y_speed += ypart(bullet[a][b].angle, 500);
    bullet[a][b].x += bullet[a][b].x_speed;
    bullet[a][b].y += bullet[a][b].y_speed;

//    drag_bullet(b, 890);
//    drag_bullet(b, 950);
    drag_bullet(b, 950);

    int c = create_cloud(CLOUD_WORM_TRAIL);
    if (c != -1)
    {
     cloud[c].x = bullet[a][b].x;
     cloud[c].y = bullet[a][b].y;
     cloud[c].x2 = old_x;
     cloud[c].y2 = old_y;
     cloud[c].x_speed = 0;
     cloud[c].y_speed = 0;
     cloud[c].timeout = bullet[a][b].status;// + bullet[a][b].level;
     cloud[c].fangle = atan2(bullet[a][b].y - old_y, bullet[a][b].x - old_x);
     cloud[c].status = (int) hypot(bullet[a][b].y - old_y, bullet[a][b].x - old_x) >> 10;
    }

  {
//     if (hs[0].alive)
//      bullet[a][b].angle = turn_towards_xy(bullet[a][b].x, bullet[a][b].y, hs[0].x, hs[0].y, bullet[a][b].angle, 22 + (bullet[a][b].status2 >> 0)); // 22
  }

    bullet[a][b].timeout --;


    if (bullet[a][b].timeout <= 0)
    {
     bullet_explodes(b, -1, -1);
     destroy_bullet(b);
    }

}


void run_bullet_rocket(int b)
{


            int angle = grand(ANGLE_1);
            int dist = grand(bullet[a][b].damage << 3);
            int c = simple_cloud(CLOUD_2BALL, bullet[a][b].x + xpart(angle, dist), bullet[a][b].y + ypart(angle, dist),
                 bullet[a][b].colour, 5 + grand(bullet[a][b].draw_size << 2));
            if (c != -1)
            {
             cloud[c].x_speed = bullet[a][b].x_speed;
             cloud[c].y_speed = bullet[a][b].y_speed;
             cloud[c].drag = 990;
            }

    if (bullet[a][b].time > 30)
    {
     bullet[a][b].x_speed += xpart(bullet[a][b].angle, 300 + (bullet[a][b].status2 << 4));
     bullet[a][b].y_speed += ypart(bullet[a][b].angle, 300 + (bullet[a][b].status2 << 4));
    }
//    if (bullet[a][b].time == 30)
//      play_effectwfvxy_xs_ys(WAV_WHOOSH, 1000 + (PP.upgrade [U_POWER] * 150), 100, bullet[a][b].x, bullet[a][b].y, bullet[a][b].x_speed + xpart(bullet[a][b].angle, 1300), bullet[a][b].y_speed + ypart(bullet[a][b].angle, 1300));

    bullet[a][b].x += bullet[a][b].x_speed;
    bullet[a][b].y += bullet[a][b].y_speed;

    drag_bullet(b, 950);

  {
//    if (target_x != -1 || target_y != -1)
//     bullet[a][b].angle = turn_towards_xy(bullet[a][b].x, bullet[a][b].y, hs[0].x, hs[0].y, bullet[a][b].angle, 22 + (bullet[a][b].status2 >> 0)); // 22
  }

    bullet[a][b].timeout --;


    if (bullet[a][b].timeout <= 0)
    {
     bullet_explodes(b, -1, -1);
     destroy_bullet(b);
    }

}


void run_bullet_missile(int b)
{

    int old_x = bullet[a][b].x;
    int old_y = bullet[a][b].y;





    if (bullet[a][b].time > 30)
    {
     bullet[a][b].x_speed += xpart(bullet[a][b].angle, 300 + (bullet[a][b].status2 << 5));
     bullet[a][b].y_speed += ypart(bullet[a][b].angle, 300 + (bullet[a][b].status2 << 5));
     drag_bullet(b, 970);
//     if (hs[0].alive)
//      bullet[a][b].angle = turn_towards_xy(bullet[a][b].x, bullet[a][b].y, hs[0].x, hs[0].y, bullet[a][b].angle, 8); // 22
    }
//    if (bullet[a][b].time == 30)
//      play_effectwfvxy_xs_ys(WAV_WHOOSH, 1300 + (PP.upgrade [U_POWER] * 150), 100, bullet[a][b].x, bullet[a][b].y, bullet[a][b].x_speed + xpart(bullet[a][b].angle, 1300), bullet[a][b].y_speed + ypart(bullet[a][b].angle, 1300));


    bullet[a][b].x += bullet[a][b].x_speed;
    bullet[a][b].y += bullet[a][b].y_speed;

    int c = create_cloud(CLOUD_ROCKET_TRAIL);
    if (c != -1)
    {
     cloud[c].x = bullet[a][b].x;
     cloud[c].y = bullet[a][b].y;
     cloud[c].x2 = old_x;
     cloud[c].y2 = old_y;
     cloud[c].x_speed = 0;
     cloud[c].y_speed = 0;
     cloud[c].timeout = bullet[a][b].status;// + bullet[a][b].level;
     cloud[c].fangle = atan2(bullet[a][b].y - old_y, bullet[a][b].x - old_x);
     cloud[c].status = (int) hypot(bullet[a][b].y - old_y, bullet[a][b].x - old_x) >> 10;
    }



  {
//    if (target_x != -1 || target_y != -1)
  }

    bullet[a][b].timeout --;


    if (bullet[a][b].timeout <= 0)
    {
     bullet_explodes(b, -1, -1);
     destroy_bullet(b);
    }

}
*/
int pbedist(int ba, int b, int ea, int e)
{

 return (int) hypot(ship[ea][e].y - bullet[ba][b].y, ship[ea][e].x - bullet[ba][b].x);

}

int pbpdist(int b, int p)
{

 return (int) hypot(PP.y - bullet[TEAM_ENEMY][b].y, PP.x - bullet[TEAM_ENEMY][b].x);

}

int xypdist(int x, int y, int p)
{

 return (int) hypot(PP.y - y, PP.x - x);

}

int xyedist(int x, int y, int ea, int e)
{

 return (int) hypot(ship[ea][e].y - y, ship[ea][e].x - x);

}

int xyedist_test_less(int x, int y, int ea, int e, int max)
{

 if (abs(ship[ea][e].y - y) > max || abs(ship[ea][e].x - x) > max)
  return 0;

 if (hypot(ship[ea][e].y - y, ship[ea][e].x - x) > max)
  return 0;

 return 1;

}

int xyedist_test_more(int x, int y, int ea, int e, int min)
{

 if (abs(ship[ea][e].y - y) + abs(ship[ea][e].x - x) < min)
  return 0;

 if (hypot(ship[ea][e].y - y, ship[ea][e].x - x) < min)
  return 0;

 return 1;

}


void destroy_bullet(int a, int b)
{
  bullet[a][b].type = BULLET_NONE;
}

int check_collision_mask(struct shipstruct* ee, int ea, int e, int ba, int b, int bx, int by, char beam)
{


 int dist = xyedist(bx, by, ea, e) >> 10;
 int angle = radians_to_angle(atan2(by - ee->y, bx - ee->x));
 int angle_diff = angle - ee->angle - ANGLE_4;
// int x = ship_collision_mask [ee->type].x [0] + xpart(angle_diff, dist >> 2);
// int y = ship_collision_mask [ee->type].y [0] + ypart(angle_diff, dist >> 2);
 int x = ship_collision_mask [ee->type].x [0] + xpart(angle_diff, dist);
 int y = ship_collision_mask [ee->type].y [0] + ypart(angle_diff, dist);
 int pix;

//putpixel(ship_collision_mask [ee->type].sprite, x, y, TRANS_RED4 + COL_12);

     angle_diff &= ANGLE_MASK;


 if (x > 0 && y > 0 && x < ship_collision_mask [ee->type].sprite->w && y < ship_collision_mask [ee->type].sprite->h)
 {

//putpixel(ship_collision_mask [ee->type].sprite, x, y, TRANS_BLUE2 + COL_7);

   pix = getpixel(ship_collision_mask [ee->type].sprite, x, y);

//putpixel(ship_collision_mask [ee->type].sprite, x, y, TRANS_RED2 + COL_9);

//        ship_collision_mask [ee->type].x [0] + xpart(angle_diff, dist>>2),
//        ship_collision_mask [ee->type].y [0] + ypart(angle_diff, dist>>2));

//bullet[ba][b].status2 = pix;

//return 0;
//             return 0;

        if (pix != 0)
        {
//             return 1;
          if (ee->shield > 0 && ee->shield_up == 1)
          {
           int pl;
           ship[ea][e].shield_bar_pulse = 5;
           if (beam)
           {


               if (eclass[ee->type].elongation != 1024)
               {
                if (eclass[ee->type].elongation > 0) // only elongated at the front
                {
                 if (angle_diff > ANGLE_2)
                 {
                  angle_diff &= ANGLE_MASK;
                  dist *= eclass[ee->type].elongation;
                  dist >>= 10;
                 }
                }
                 else // elongated all around
                 {
                   angle_diff &= ANGLE_MASK;
                   dist *= (eclass[ee->type].elongation * -1);
                   dist >>= 10;
                 }
               }
/*
               if (eclass[ee->type].elongation != 1024 && angle_diff > ANGLE_2)// || angle_diff > (ANGLE_1 - ANGLE_4))
               {
                angle_diff &= ANGLE_MASK;
                dist *= eclass[ee->type].elongation;
                dist >>= 10;
               }
*/
// first check for an spulse reserved for this beam:
            for (pl = 0; pl < PULSE; pl ++)
            {
             if (ee->spulse_time [pl] >= 0
              && ee->spulse_beam_e [pl] == bullet[ba][b].owner
              && ee->spulse_beam_t [pl] == bullet[ba][b].owner_t)
             {
              ee->spulse_angle [pl] = angle_diff;
              ee->spulse_dist [pl] = dist;
              ee->spulse_time [pl] = 60 + grand(50);
              ee->spulse_time_delta [pl] = 10;
              ee->spulse_beam_e [pl] = bullet[ba][b].owner;
              ee->spulse_beam_t [pl] = bullet[ba][b].owner_t;
              if (bullet[ba][b].damage >= ee->shield)
               play_effectwfvxy_xs_ys(WAV_SHIELD, SPRIORITY_LOW, 1000, 140, ee->x, ee->y, ee->x_speed, ee->y_speed);
              return -1;
             }
            }
// if none found, reserve a new one:
            for (pl = 0; pl < PULSE; pl ++)
            {
             if (ee->spulse_time [pl] <= 0 || pl == PULSE - 1)
             {
              ee->spulse_size [pl] = 3;// + grand(4);
              ee->spulse_size2 [pl] = 1;// + grand(3);
              ee->spulse_angle [pl] = angle_diff;
              ee->spulse_dist [pl] = dist;
              ee->spulse_time [pl] = 100;// + grand(12);
              ee->spulse_time_delta [pl] = 10;
              ee->spulse_beam_e [pl] = bullet[ba][b].owner;
              ee->spulse_beam_t [pl] = bullet[ba][b].owner_t;
              if (bullet[ba][b].damage >= ee->shield)
               play_effectwfvxy_xs_ys(WAV_SHIELD, SPRIORITY_LOW, 1000, 140, ee->x, ee->y, ee->x_speed, ee->y_speed);
              return -1;
             }
            }
           }
              else // not a beam
              {
               // let's recalc these numbers to find out where the bullet was coming from:
//               bx += (bullet[ba][b].x_speed * 1);
//               by += (bullet[ba][b].y_speed * 1);
               dist = xyedist(bx, by, ea, e) >> 10;
               bx -= (bullet[ba][b].x_speed * 4);
               by -= (bullet[ba][b].y_speed * 4);
               angle = radians_to_angle(atan2(by - ee->y, bx - ee->x));
               angle_diff = angle - ee->angle - ANGLE_4;
               if (bullet[ba][b].damage >= ee->shield)
                play_effectwfvxy_xs_ys(WAV_SHIELD, SPRIORITY_LOW, 1000, 140, bullet[ba][b].x, bullet[ba][b].y, ee->x_speed, ee->y_speed);
                 else
                  play_effectwfvxy_xs_ys(WAV_SHIELD, SPRIORITY_LOW, 2000, 100, bullet[ba][b].x, bullet[ba][b].y, ee->x_speed, ee->y_speed);

/*
               if (eclass[ee->type].elongation != 1024 && angle_diff > ANGLE_2)// || angle_diff > (ANGLE_1 - ANGLE_4))
               {
                angle_diff &= ANGLE_MASK;
                dist *= eclass[ee->type].elongation;
                dist >>= 10;
               }
*/
               if (eclass[ee->type].elongation != 1024)
               {
                if (eclass[ee->type].elongation > 0) // only elongated at the front
                {
                 if (angle_diff > ANGLE_2)
                 {
                  angle_diff &= ANGLE_MASK;
                  dist *= eclass[ee->type].elongation;
                  dist >>= 10;
                 }
                }
                 else // elongated all around
                 {
                   angle_diff &= ANGLE_MASK;
                   dist *= (eclass[ee->type].elongation * -1);
                   dist >>= 10;
                 }
               }


               for (pl = 0; pl < PULSE; pl ++)
               {
                if (ee->spulse_time [pl] <= 0 || pl == PULSE - 1)
                {
                  ee->spulse_size [pl] = 3;// + grand(4);
                  ee->spulse_size2 [pl] = 1;// + grand(3);
                  ee->spulse_angle [pl] = angle_diff;
                  ee->spulse_dist [pl] = dist;
                  ee->spulse_time [pl] = 100;// + grand(12);
                  ee->spulse_time_delta [pl] = 10;
                  return -1;
                }
               }
              }

           return -1; // no spare pulses - probably impossible to get to this stage because the last pulse index is used if the array is full
          }
//           else
           {
            if (pix < COL_CONTROL_BASE + 6) // i.e. is hull, not just shield
            {
//             putpixel(ship_collision_mask [ee->type].sprite, x, y, 6 + TRANS_BLUE2);
             return pix - COL_CONTROL_BASE;
            }
           }


          return -2; // hit nothing
        }
 }

 return -2; // hit nothing

}


/*
int check_collision_mask(struct bulletstruct* pb, struct shipstruct* ee, int ba, int b, int ea, int e)
{

 int dist = pbedist(ba, b, ea, e) >> 10;
 int angle = radians_to_angle(atan2(pb->y - ee->y, pb->x - ee->x));
 int angle_diff = angle - ee->angle - ANGLE_4;
// int x = ship_collision_mask [ee->type].x [0] + xpart(angle_diff, dist >> 2);
// int y = ship_collision_mask [ee->type].y [0] + ypart(angle_diff, dist >> 2);
 int x = ship_collision_mask [ee->type].x [0] + xpart(angle_diff, dist);
 int y = ship_collision_mask [ee->type].y [0] + ypart(angle_diff, dist);
 int pix;

//putpixel(ship_collision_mask [ee->type].sprite, x, y, TRANS_RED4 + COL_12);


 if (x > 0 && y > 0 && x < ship_collision_mask [ee->type].sprite->w && y < ship_collision_mask [ee->type].sprite->h)
 {

//putpixel(ship_collision_mask [ee->type].sprite, x, y, TRANS_BLUE2 + COL_7);

   pix = getpixel(ship_collision_mask [ee->type].sprite, x, y);

//putpixel(ship_collision_mask [ee->type].sprite, x, y, TRANS_RED2 + COL_9);

//        ship_collision_mask [ee->type].x [0] + xpart(angle_diff, dist>>2),
//        ship_collision_mask [ee->type].y [0] + ypart(angle_diff, dist>>2));

//bullet[ba][b].status2 = pix;

//return 0;
//             return 0;

        if (pix != 0)
        {
//             return 1;
          if (ee->shield > 0 && ee->shield_up == 1)
          {
           int pl;
           ship[ea][e].shield_bar_pulse = 5;
           for (pl = 0; pl < PULSE; pl ++)
           {
            if (ee->spulse_time [pl] <= 0 || pl == PULSE - 1)
            {
             ee->spulse_size [pl] = 3;// + grand(4);
             ee->spulse_size2 [pl] = 1;// + grand(3);
             ee->spulse_angle [pl] = angle_diff;
             ee->spulse_dist [pl] = dist;
             ee->spulse_time [pl] = 10;// + grand(12);
             if (ee->spulse_size [pl] < ee->spulse_time [pl] * ee->spulse_size2 [pl])
              ee->spulse_size [pl] = ee->spulse_time [pl] * ee->spulse_size2 [pl];
             return -1;
            }
           }
           return -1; // no spare pulses - probably impossible to get to this stage because the last pulse index is used if the array is full
          }
//           else
           {
            if (pix < COL_CONTROL_BASE + 4) // i.e. is hull, not just shield
            {
//             putpixel(ship_collision_mask [ee->type].sprite, x, y, 6 + TRANS_BLUE2);
             return pix - COL_CONTROL_BASE;
            }
           }


          return -2; // hit nothing
        }
 }

 return -2; // hit nothing

}

*/
