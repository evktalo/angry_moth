// Global variables:
//  at the moment these are all defined in main.c.
//  but they have to be externed here so all modules with #include globvars.h can access them.

extern struct playerstruct player [2];

extern struct arenastruct arena;

extern unsigned char counter;

extern struct cloudstruct cloud [NO_CLOUDS];

extern struct bulletstruct bullet [NO_TEAMS] [NO_BULLETS];

extern struct starstruct star [2] [NO_STARS];

extern struct convoystruct convoy [NO_CONVOYS];

extern struct optionstruct options;

extern int glob;

//extern struct ebulletstruct ebullet [NO_EBULLETS];

extern struct shipstruct ship [NO_TEAMS] [NO_SHIPS];

extern struct eclass_struct eclass [NO_SHIP_TYPES];

// in display_init
extern int rotate_pos_x [ROTATE_POS] [ANGLE_1];
extern int rotate_pos_y [ROTATE_POS] [ANGLE_1];

extern struct commstruct comm [COMMS];
extern struct linestruct ctext [COMMS] [LINES];
