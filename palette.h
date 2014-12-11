
void init_palette(int stage);



enum
{
COL_NULL1,
COL_1,
COL_2,
COL_3,
COL_4,
COL_5,
COL_6,
COL_7,
COL_8,
COL_9,
COL_10,
COL_11,
COL_12,
COL_13,
COL_14,
COL_15,
COL_16,
//COL_BACK,
/*COL_STAR1,
COL_STAR2,
COL_STAR3,
COL_STAR4,
COL_STAR5,
COL_STAR6,
COL_STAR7,
COL_OUTLINE,
COL_DGREY,
COL_LGREY,
COL_WHITE,
COL_HS_INNER,
COL_HS_INNER_EDGE,
COL_HS_OUTER,
COL_HS_OUTER_EDGE,
COL_HS_INNER_LINE,
COL_HS_HURT*/

};

enum
{
COL_NULL,
COL_F1,
COL_F2,
COL_F3,
COL_F4,
COL_F5,
COL_F6,
COL_F7,
COL_E1,
COL_E2,
COL_E3,
COL_E4,
COL_E5,
COL_E6,
COL_E7,
COL_STAR1,
COL_STAR2,
COL_STAR3,
COL_STAR4,
COL_STAR5

};

#define COL_OUTLINE 2
#define COL_DGREY 4
#define COL_LGREY 8
#define COL_WHITE 255
#define COL_BACK 1
//#define COL_HS_INNER_TRANS 21

#define TRANS_RED1 20
#define TRANS_GREEN1 39
#define TRANS_BLUE1 39
#define TRANS_RED2 58
#define TRANS_GREEN2 77
#define TRANS_BLUE2 96
#define TRANS_RED3 115
#define TRANS_GREEN3 134
#define TRANS_BLUE3 153
#define TRANS_YELLOW4 172
#define TRANS_RED4 172
#define TRANS_GREEN4 191
#define TRANS_BLUE4 210

/*
#define TRANS_RED1 20
#define TRANS_GREEN1 39
#define TRANS_BLUE1 58
#define TRANS_RED2 77
#define TRANS_GREEN2 96
#define TRANS_BLUE2 115
#define TRANS_RED3 134
#define TRANS_GREEN3 153
#define TRANS_BLUE3 172
#define TRANS_YELLOW4 191
#define TRANS_RED4 191
#define TRANS_GREEN4 191
// yellow is top colour for both red and green
#define TRANS_BLUE4 210
*/

// this must be the gap between those trans values:
#define COLOUR_GAP 19

#define COL_BRIGHT 252
#define WRITE_OUT 254
#define WRITE_IN 253

#define CONVERT_WHITE_TO_GREY 252

// used in display_init to fix file bitmaps so that the colours are right
#define TRANS_CONTROL 254
#define COL_CONTROL_BASE 16

#define TRANS_CONTROL_TRANS 254
//#define COL_CONTROL_BASE 255

enum
{
TRANS_DAM_GREEN = 230,
TRANS_DAM_GREEN1 = 231,
TRANS_DAM_YELLOW = 234,
TRANS_DAM_YELLOW1 = 235,
TRANS_DAM_RED = 238,
TRANS_DAM_RED1 = 239,
TRANS_DAM_GREY = 242



//TRANS_DAM_END

};

enum
{
COL_BOX0 = 245,
COL_BOX1,
COL_BOX2,
COL_BOX3,
COL_BOX4,
COL_EBOX1,
COL_EBOX2,
COL_EBOX3,
COL_EBOX4 // 253
};

// don't think these do anything at the moment:
#define TRANS_REVERSE 225
#define TRANS_DARKEN 226

#define FIX_BITMAP 244

int colour_to_trans(int y);
void set_control_blend(int colour_number, int control_number);
void set_control_trans_blend(int colour_number, int control_number);






