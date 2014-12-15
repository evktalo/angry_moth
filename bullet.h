void run_bullets(void);
void init_bullets(void);
int create_bullet(int type, int a);
int fighter_fire(int a, int e, int p, int wpn, int lock, int status);

int xyedist(int x, int y, int ea, int e);
int xyedist_test_less(int x, int y, int ea, int e, int max);
int xyedist_test_more(int x, int y, int ea, int e, int min);
