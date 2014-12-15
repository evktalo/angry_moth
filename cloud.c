

#include "config.h"

#include <math.h>

#include "allegro.h"

#include "globvars.h"

#include "stuff.h"


#include "palette.h"


void run_clouds(void);
void destroy_cloud(int b);
int quick_cloud(int type, int x, int y, int x_speed, int y_speed, int timeout, int colour, int drag);
void quick_fire(int x, int y, int x_speed, int y_speed, int size, int col);


void init_clouds(void)
{

 int c;

 for (c = 0; c < NO_CLOUDS; c ++)
 {
  cloud[c].type = CLOUD_NONE;
 }

}

int create_cloud(int type)
{
  int c;

  for (c = 0; c < NO_CLOUDS; c ++)
  {
      if (cloud[c].type == CLOUD_NONE)
       break;
      if (c == NO_CLOUDS - 1)
       return -1;

  }

 cloud[c].type = type;
 cloud[c].timeout = 0;
 cloud[c].x_speed = 0;
 cloud[c].y_speed = 0;
 cloud[c].drag = 0;
 cloud[c].time = 0;
 return c;

}

void run_clouds(void)
{


  int c, dist, speed, angle2, c2, size;

  for (c = 0; c < NO_CLOUDS; c ++)
  {
    if (cloud[c].type == CLOUD_NONE)
     continue;
//    if (arena.counter % 8 == 0)
     cloud[c].timeout --;
     cloud[c].time ++;
    if (cloud[c].timeout <= 0)
    {
     destroy_cloud(c);
     continue;
    }
    if (cloud[c].drag != 0)
    {
     cloud[c].x_speed *= cloud[c].drag;
     cloud[c].x_speed >>= 10;
     cloud[c].y_speed *= cloud[c].drag;
     cloud[c].y_speed >>= 10;
    }
    cloud[c].x += cloud[c].x_speed;
    cloud[c].y += cloud[c].y_speed;
    switch(cloud[c].type)
    {
      case CLOUD_XBALL:
       cloud[c].timeout --;
       break;
        default:
         break;
/*       case CLOUD_SEEKER_TRAIL:
        cloud[c].timeout ++;
        break;*/
       case CLOUD_BANG:
//        cloud[c].status ++;
        break;
       case CLOUD_FADEBALL:
        cloud[c].status --;
        if (cloud[c].status < 0)
         cloud[c].status = 0;
/*        if (abs(CC.x_speed) > 50000 || abs(CC.y_speed) > 50000)
        {
            log_str("fadeball ", 0);
//            log_num(CC.x_speed);
//            log_num(CC.y_speed);
            log_num(CC.x3);
//            log_num(CC.colour);
            finish_log();
            destroy_cloud(c);
            player[0].ships++;
            break;
        }*/
        break;
       case CLOUD_FIRE:
/*        CC.timeout -= CC.status2 >> 1;
        CC.status -= CC.status2;
        CC.status2 ++;
        if (CC.status < 0)
         CC.status = 0;*/
        //CC.timeout -= 1;
        CC.status -= 2;
//        if (CC.status <= 0)
//         CC.timeout --;
        CC.status2 -= 3;
//        if (CC.status2 <= 0)
//        {
//          CC.timeout --;
//          CC.status --;
//        }
//        CC.status2 ++;
/*        if (CC.status < 0)
         CC.status = 0;
        if (CC.status2 < 0)
         CC.status2 = 0;*/
        //if (CC.timeout <= 0)
        // destroy_cloud(c);
        break;

       case CLOUD_BURST:
/*       if (cloud[c].time % 10 != 0)
       {
        cloud[c].timeout ++;
        break;
       }*/
        cloud[c].status -= cloud[c].status2;
        if (cloud[c].status < 0)
         destroy_cloud(c);
        break;
/*       case CLOUD_DELAY_SPARKLE:
       case CLOUD_DELAY_SPARKLE_COL2:
        cloud[c].status --;
        if (cloud[c].status > 0)
         cloud[c].timeout ++;
        break;*/
//       case CLOUD_SMALL_SHOCK:
       case CLOUD_FLASH:
        cloud[c].status += cloud[c].status2;
        break;

       case CLOUD_LARGE_SHOCK:
       case CLOUD_HUGE_SHOCK:
//       if (arena.counter % 20 != 0 && cloud[c].timeout < 20)
//        cloud[c].timeout --;
        break;
       case CLOUD_SMALL_SHOCK:
//       if (arena.counter % 20 != 0)
//        cloud[c].timeout --;
        break;

       case CLOUD_SYNCHBALL:
        CC.status2 --;
        CC.timeout ++;
        if (CC.status2 <= 0)
        {
         CC.type = CLOUD_FADEBALL;
         CC.status = 12;
        }
/*        if (abs(CC.x_speed) > 50000 || abs(CC.y_speed) > 50000)
        {
            log_str("synchball ", 0);
//            log_num(CC.x_speed);
//            log_num(CC.y_speed);
            log_num(CC.x3);
//            log_num(CC.colour);
            finish_log();
            destroy_cloud(c);
            player[0].ships++;
            break;
        }*/
        break;
       case CLOUD_SYNCHFLARE:
       if (cloud[c].timeout & 1)
        continue;

        speed = 0;
        angle2 = grand(ANGLE_1);
        dist = grand(7000);
        size = 20 + grand(20);//grand(30);//20 + grand(20) - (cloud[c].timeout >> 1);
        quick_fire(cloud[c].x + xpart(angle2, dist), cloud[c].y + ypart(angle2, dist),
           cloud[c].x2, cloud[c].y2, size, cloud[c].colour);
/*
        c2 = quick_cloud(CLOUD_SYNCHBALL, cloud[c].x + xpart(angle2, dist), cloud[c].y + ypart(angle2, dist),
           cloud[c].x2, cloud[c].y2, size, cloud[c].colour, 900);
        if (c2 != -1)
        {
         cloud[c2].status2 = (cloud[c].timeout >> 1);
         cloud[c2].x3 = cloud[c].x3;
         if (abs(cloud[c2].x_speed) > 50000 || abs(cloud[c2].y_speed) > 50000)
          exit(500);
        }
        if (abs(cloud[c].x2) > 50000 || abs(cloud[c].y2) > 50000)
         exit(400);
*/
        break;


       case CLOUD_FADEFLARE:
       if (cloud[c].timeout & 1)
        continue;

        speed = 0;//2000 + grand(100);//1000 + grand(1000);
        angle2 = grand(ANGLE_1);
        dist = grand(7000);
        size = 20 + grand(20);//grand(30);//20 + grand(20) - (cloud[c].timeout >> 1);
        quick_fire(cloud[c].x + xpart(angle2, dist), cloud[c].y + ypart(angle2, dist),
           cloud[c].x2, cloud[c].y2, size, cloud[c].colour);

/*
        c2 = quick_cloud(CLOUD_FADEBALL, cloud[c].x + xpart(angle2, dist), cloud[c].y + ypart(angle2, dist),
           cloud[c].x2 + xpart(angle2, speed), cloud[c].y2 + ypart(angle2, speed), size, cloud[c].colour, 900);
        if (c2 != -1)
        {
         cloud[c2].status = 10 + grand(13);
        }*/
        break;


       case CLOUD_BIGFADEFLARE:
       if (cloud[c].timeout & 1)
        continue;

        angle2 = grand(ANGLE_1);
        dist = grand(47000);
        size = 30 + grand(30);//grand(30);//20 + grand(20) - (cloud[c].timeout >> 1);

        quick_fire(cloud[c].x + xpart(angle2, dist), cloud[c].y + ypart(angle2, dist),
           0,0, size, cloud[c].colour);

/*
        c2 = quick_cloud(CLOUD_FADEBALL, cloud[c].x + xpart(angle2, dist), cloud[c].y + ypart(angle2, dist),
           0,0, size, cloud[c].colour, 0);
        if (c2 != -1)
        {
         cloud[c2].status = 10 + grand(23);
        }*/
        break;


       case CLOUD_FLARE:
       if (cloud[c].timeout & 1)
        continue;

        speed = 2000 + grand(100);//1000 + grand(1000);
        angle2 = grand(ANGLE_1);
//        dist = 4000 + grand(4000);
        dist = grand(7000);
        size = cloud[c].timeout + 20 + grand(20);//grand(30);//20 + grand(20) - (cloud[c].timeout >> 1);

        quick_fire(cloud[c].x + xpart(angle2, dist), cloud[c].y + ypart(angle2, dist),
           cloud[c].x_speed + xpart(angle2, speed), cloud[c].y_speed + ypart(angle2, speed), size, cloud[c].colour);

/*
        c2 = quick_cloud(CLOUD_MAX_DBALL, cloud[c].x + xpart(angle2, dist), cloud[c].y + ypart(angle2, dist),
           cloud[c].x_speed + xpart(angle2, speed), cloud[c].y_speed + ypart(angle2, speed), size, cloud[c].colour, 810);
        if (c2 != -1)
        {
         cloud[c2].status2 = 20 + grand(30);
        }*/
        break;

       case CLOUD_BIGFLARE:
       if (cloud[c].timeout & 1)
        continue;

        speed = 2000 + grand(100);//1000 + grand(1000);
        angle2 = grand(ANGLE_1);
        dist = grand(7000 + CC.timeout * 300);
        size = cloud[c].timeout + 30 + grand(30);//grand(30);//20 + grand(20) - (cloud[c].timeout >> 1);
/*
        c2 = quick_cloud(CLOUD_MAX_DBALL_BRIGHT, cloud[c].x + xpart(angle2, dist), cloud[c].y + ypart(angle2, dist),
           cloud[c].x_speed + xpart(angle2, speed), cloud[c].y_speed + ypart(angle2, speed), size, cloud[c].colour, 810);
        if (c2 != -1)
        {
         cloud[c2].status2 = 30 + grand(30);
        }*/
                quick_fire(cloud[c].x + xpart(angle2, dist), cloud[c].y + ypart(angle2, dist),
           cloud[c].x_speed + xpart(angle2, speed), cloud[c].y_speed + ypart(angle2, speed), size, cloud[c].colour);

        break;


       case CLOUD_SPARSE_FLARE:
       if (!(cloud[c].timeout & 7))
        continue;

        speed = 1000 + grand(100);//1000 + grand(1000);
        angle2 = grand(ANGLE_1);
//        dist = 7000 + grand(7000);

        dist = grand(10000);
        quick_cloud(CLOUD_BALL_COL2, cloud[c].x + xpart(angle2, dist), cloud[c].y + ypart(angle2, dist),
         cloud[c].x_speed + xpart(angle2, speed), cloud[c].y_speed + ypart(angle2, speed), 20 + grand(20) + (cloud[c].timeout >> 1), cloud[c].colour, 810);
        quick_cloud(CLOUD_BALL, cloud[c].x + xpart(angle2, dist), cloud[c].y + ypart(angle2, dist),
         cloud[c].x_speed + xpart(angle2, speed), cloud[c].y_speed + ypart(angle2, speed), 15 + grand(10) + (cloud[c].timeout >> 1), cloud[c].colour, 815);
        break;
       case CLOUD_SPARSE_NARROW_FLARE:
       if (!(cloud[c].timeout & 7))
        continue;

        speed = 1000 + grand(100);//1000 + grand(1000);
        angle2 = grand(ANGLE_1);
        dist = grand(3000);

        quick_cloud(CLOUD_BALL_COL2, cloud[c].x + xpart(angle2, dist), cloud[c].y + ypart(angle2, dist),
         cloud[c].x_speed + xpart(angle2, speed), cloud[c].y_speed + ypart(angle2, speed), 20 + grand(20) + (cloud[c].timeout >> 1), cloud[c].colour, 810);
        quick_cloud(CLOUD_BALL, cloud[c].x + xpart(angle2, dist), cloud[c].y + ypart(angle2, dist),
         cloud[c].x_speed + xpart(angle2, speed), cloud[c].y_speed + ypart(angle2, speed), 15 + grand(10) + (cloud[c].timeout >> 1), cloud[c].colour, 815);
        break;
       case CLOUD_LINKLINE:
        CC.x2 += CC.x3;
        CC.y2 += CC.y3;
        break;

    } // cloud may be destroyed here

// if timeout is being manipulated in the switch, make sure it's not possible for it to be -1 at this point because the display functions will run.

    }

}



int quick_cloud(int type, int x, int y, int x_speed, int y_speed, int timeout, int colour, int drag)
{

  int c;

  c = create_cloud(type);

  if (c != -1)
  {
   cloud[c].x = x;
   cloud[c].y = y;
   cloud[c].timeout = timeout;
   cloud[c].colour = colour;
   cloud[c].drag = drag;
   cloud[c].x_speed = x_speed;
   cloud[c].y_speed = y_speed;
  }

  return c;

}

int simple_cloud(int type, int x, int y, int col, int timeout)
{

  int c;

  c = create_cloud(type);

  if (c != -1)
  {
   cloud[c].x = x;
   cloud[c].y = y;
   cloud[c].timeout = timeout;
   cloud[c].colour = col;
   cloud[c].drag = 0;
   cloud[c].x_speed = 0;
   cloud[c].y_speed = 0;
  }

  return c;

}

void quick_fire(int x, int y, int x_speed, int y_speed, int size, int col)
{

 int c;

 c = create_cloud(CLOUD_FIRE);

  if (c != -1)
  {
   cloud[c].x = x;
   cloud[c].y = y;
   cloud[c].timeout = size;
   cloud[c].colour = col;
   cloud[c].drag = 1000;
   cloud[c].x_speed = x_speed;
   cloud[c].y_speed = y_speed;
   CC.status = size;
   cloud[c].status2 = size;
  }

}

void double_ball(int x, int y, int size1, int size2, int col)
{
    quick_cloud(CLOUD_BALL, x, y, 0, 0, size1, col, 0);
    quick_cloud(CLOUD_BALL_COL2, x, y, 0, 0, size2, col, 0);
}

void destroy_cloud(int c)
{
/* if (CC.link_down != -1)
 {
  cloud[CC.link_down].link_up = -1;
 }
 if (CC.link_up != -1)
 {
  cloud[CC.link_up].link_down = -1;
 }*/

 cloud[c].type = CLOUD_NONE;

}

