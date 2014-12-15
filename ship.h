void init_ships(void);
void run_ships(void);
int create_ship(int type, int a, int srec);

int hurt_ship_fighter(int ea, int e, int dam, int ba, int b, int beam, int bullet_x, int bullet_y);
void hurt_wship_shield(int ea, int e, int dam, int owner, int ba, int b);
void hurt_wship_part(int ea, int e, int dam, int owner, int ba, int b, int part);
void formation_position(int a, int e);
void convoy_position(int a, int e);
int edist_test_less(int a, int e, int target, int max);
int edist_test_more(int a, int e, int target, int min);
void fighter_find_wship_to_guard(int a, int e, int x, int y);
void jump_clouds(int a, int e);
int get_ship_sprite(int type);

void ship_explodes(int a, int e);
