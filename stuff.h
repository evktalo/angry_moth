void init_trig(void);

float angle_to_radians(int angle);
int radians_to_angle(float angle);
fixed angle_to_fixed(int angle);
int grand(int number);
int crandom(int number);
int turn_towards_angle(int angle, int tangle, int turning);
int turn_towards_xy(int x1, int y1, int x2, int y2, int angle, int turning);
int xpart(int angle, int length);
int ypart(int angle, int length);
int pulsate(int speed, int amount, int county);
int angle_difference(int a1, int a2);
int angle_difference_signed(int a1, int a2);

void error_message_out(const char *errm);
int delta_turn_towards_angle(int angle, int tangle, int turning);
int delta_turn_towards_xy(int x1, int y1, int x2, int y2, int angle, int turning);
int turn_towards_xy_forbid(int x1, int y1, int x2, int y2, int angle, int turning, int forbid);
int turn_towards_angle_forbid(int angle, int tangle, int turning, int forbid);
int pos_or_neg(int a);
char coin(void);

int fxpart(float angle, int length);
int fypart(float angle, int length);
