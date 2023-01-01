
#include "config.h"

#include "allegro.h"

#include <math.h>
#include <string.h>
#include <stdio.h>

#include "globvars.h"
#include "palette.h"


FILE* out;

void log_digit(int dig);


void init_logfile(void)
{

 out = fopen("logfile.log", "w");

}

void log_int(int val)
{

 fputc(val & 0xff, out);
 fputc((val >> 8) & 0xff, out);
 fputc((val >> 16) & 0xff, out);
 fputc((val >> 24) & 0xff, out);
 fflush(out);

}

void log_char(unsigned char val)
{

 fputc(val, out);
// fflush(out);

}

void log_num(int num)
{
 int len = log10(num)+1;
 int i;
 int val;
 for (i = len; i > 0; i --)
 {
  val = (int) (num / pow(10,i-1)) % 10;
  log_digit(val);
 }

 fputc('\n', out);
 fputc('\r', out);
// fflush(out);
}

void finish_log(void)
{
    fflush(out);
}

void log_str(const char str [], int end_line)
{

 int i;
 for (i = 0; i < strlen(str); i ++)
 {
  fputc(str [i], out);
 }

 if (end_line)
 {

  fputc('\n', out);
  fputc('\r', out);
 }

 if (end_line)
  fflush(out);
}


void log_digit(int dig)
{

 fputc(dig + '0', out);

}
/*
void check_cloud_speed(int val)
{
 int c;
 int exiting = -1;

 for (c = 0; c < NO_CLOUDS; c ++)
 {
     if (CC.type == CLOUD_FADEBALL &&
          (abs(CC.x_speed) > 50000 || abs(CC.y_speed > 50000)))
          {
           exiting = val;
          }
     if (CC.type == CLOUD_SYNCHFLARE &&
          (abs(CC.x2) > 50000 || abs(CC.y2> 50000)))
          {
           exiting = val + 100;
          }
     if (CC.type == CLOUD_FADEFLARE &&
          (abs(CC.x2) > 50000 || abs(CC.y2> 50000)))
          {
           exiting = val + 200;
          }

     if (CC.type == CLOUD_SYNCHBALL &&
          (abs(CC.x_speed) > 50000 || abs(CC.y_speed > 50000)))
           {
           exiting = val + 300;
          }

 }

if (exiting != -1)
{
 log_str("cloud check failed ", 0);
 log_num(exiting);
 rest(20);
 exit(exiting);
}


}
*/

