#include <string.h>
#include <math.h>

#include "config.h"

#include "allegro.h"

#include "globvars.h"

#include "palette.h"

#include "text.h"

#define TRANS_MODE drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
#define END_TRANS_MODE drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
#define END_TRANS drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);

extern RGB palet [256];
extern BITMAP *display [3];
extern FONT* small_font;



//struct linestruct tline [LINES];
int message_lines; // number of actual lines (not just tlines) in message. externed in various places.

int current_line;
int cumulative_length;
//int current_line_pos;
char current_word [30];


#define TEMPSTRING_LENGTH 50

//#define LINE_WIDTH 220
// line_width is in pixels

void new_line(struct linestruct lines [LINES], int line_spacing);
void get_current_word(struct msgstruct tmsg, int m);
void add_current_word(struct linestruct lines [LINES], int w, int line_spacing);
void new_text_colour(struct linestruct lines [LINES], int col);

int generate_message(struct msgstruct tmsg, int w, int line_spacing, int col, struct linestruct lines [LINES])
{

 lines[0].text [0] = END_STRING;
 lines[0].x = 0;
 lines[0].y = 0;
 lines[0].col = col;

// lines[0].text [1] = END_STRING;
/* for (i = 0; i < LINES; i ++)
 {
  lines[i].text [0] = END_STRING;
  lines[i].x = 0;
  lines[i].y = 0;
  lines[i].col = col;
 }*/

 cumulative_length = 0;

 int m = 0;

 current_line = 0;
// current_line_pos = 0;

 message_lines = 1;

 while (TRUE)
 {
  m ++;
  if (tmsg.text [m] == END_STRING)
  {
   lines[current_line+1].text [0] = END_MESSAGE;
   break;
  }
  if (tmsg.text [m] == ' ')
  {
   get_current_word(tmsg, m);
   add_current_word(lines, w, line_spacing);
  }
  if (tmsg.text [m] == '$')
  {
   if (m > 1)
   {
    get_current_word(tmsg, m);
    add_current_word(lines, w, line_spacing);
   }
   m ++;
   switch(tmsg.text [m])
   {
    case 'P': new_line(lines, line_spacing); break;
    case 'B': new_text_colour(lines, COL_BOX3);
     break;
    case 'C': new_text_colour(lines, col);
     break;
   }
   m++;
  }

 };

 return message_lines;

/*
 strcpy(tline[0].text, "hello!");
 tline[0].x = 0;
 tline[0].y = 0;
 tline[0].col = COL_F7;
*/
}


// Goes back to start of current word. Copies that word, plus trailing space, into current_word buffer.
// Call this function when m is at the space after a word.
void get_current_word(struct msgstruct tmsg, int m)
{

 int i;

 do
 {
  m --;
  if (m < 0)
   break;
 } while(tmsg.text [m] != ' '
  && tmsg.text [m] != '$');

// if (m < 0)
//  m = 0;

  if (m < 0 || tmsg.text [m] == '$')
   m ++;
//      && bmsg [msg] [m] != ']');

// if (bmsg [msg] [m] == ']')
  //m ++;

 i = 0;

 do
 {
  m ++;
  if (tmsg.text [m] == '$')
   break;
  current_word [i] = tmsg.text [m];
  i ++;
//  line[current_line].text [current_line_pos] = bmsg [msg] [m];
 } while(tmsg.text [m] != ' ');
//      && tmsg.text [m] != '$');

 current_word [i] = END_STRING;

}

void add_current_word(struct linestruct lines [LINES], int w, int line_spacing)
{

// int line_length;
 int current_word_length;

// line_length = text_length(font, tline[current_line].text);
 current_word_length = text_length(small_font, current_word);

 if (cumulative_length + current_word_length > w)
 {
  new_line(lines, line_spacing);
  strcat(lines[current_line].text, current_word);
 }
  else
   strcat(lines[current_line].text, current_word);

 cumulative_length += text_length(small_font, current_word);

}

void new_line(struct linestruct lines [LINES], int line_spacing)
{
// tline[current_line].text [current_line_pos] = END_STRING;
 current_line ++;
// current_line_pos = 0;
 lines[current_line].x = 0;
 lines[current_line].y = lines[current_line-1].y + line_spacing;//MESSAGE_LINE_SPACE;
 lines[current_line].col = lines[current_line-1].col;
 lines[current_line].text [0] = END_STRING;
 cumulative_length = 0;
 message_lines ++;
}

void new_text_colour(struct linestruct lines [LINES], int col)
{
// cumulative_length += text_length(tline[current_line].text;
// int x = text_length(font, tline[current_line].text);
 current_line ++;

 lines[current_line].x = cumulative_length;
 lines[current_line].y = lines[current_line-1].y;
 lines[current_line].col = col;
 lines[current_line].text [0] = END_STRING;
}

void display_message(BITMAP *bmp, struct linestruct lines[LINES], int x, int y)
{
    int i;

    for (i = 0; i < LINES; i ++) {
        if (lines[i].text[0] == END_MESSAGE) {
            return;
        }
        textprintf_ex(bmp, small_font, x + lines[i].x, y + lines[i].y, lines[i].col, -1, "%s", lines[i].text);

    }
}
