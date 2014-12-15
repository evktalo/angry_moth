
void init_level(void);
void run_level(void);
void setup_new_ship(int a, int e, int subtype);
void condition_eship_destroyed(int e);
void condition_fship_destroyed(int e);
void setup_player_wing(int p, int w, int type, int number);
void calculate_threat(void);
char nearby(int x, int y);
void condition_ecarrier_disabled(int e);
void get_script_name(int r, char sname [30]);

int pre_briefing_wship_sprite_check(void);
