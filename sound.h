
void init_sound(void);

void play_basicwfv(int sample, int f, int v);
void play_effect(int sample);
void play_effectwfv(int sample, int f, int v);
void play_effectwfvx(int sample, int f, int v, int x);
void play_effectwfvxy(int sample, int priority, int f, int v, int x, int y);
void play_effectwfvxy_xs_ys(int sample, int priority, int f, int v, int x, int y, int xs, int ys);
void indicator(int sample, int t, int v, int p);
void run_voices(void);
void init_voices(void);

//void init_beats(void);
//void build_tracks(void);
//void play_tracks(void);

void init_music(void);
void new_level_music(void);
void run_music(void);

void test_run_music(void);

#define SPRIORITY_LOW 0
#define SPRIORITY_HIGH 1

enum
{
WAV_NONE,
WAV_FIRE,
WAV_BANG1,
WAV_BANG3,
WAV_BANG4,
WAV_BBANG,
WAV_WHOOSH,
WAV_WHOOSH2,
WAV_BURST,
WAV_HIT,
WAV_BEAM,
WAV_BASIC,
WAV_SHIELD,
WAV_BLIP,
WAV_BLIP2,
WAV_BLIP_L,
WAV_BLIP_L2,
WAV_BLAT,
WAV_SELECT0,
WAV_SELECT1,
WAV_DRIVE,
WAV_EBASIC,
WAV_HEAVY,
WAV_HIT_HULL,
WAV_SINBLIP,
WAV_SINBLIP2,

WAV_BASS,
WAV_SAW,
WAV_LINE,
WAV_WARM,

BEAT_NOISEA,
BEAT_NOISEB,

BEAT_WAH1,
BEAT_WAH2,
BEAT_WEIRD,
//BEAT_MILLIPEDE,
BEAT_SPARKLE,
//BEAT_TODDLE,
//BEAT_WARM2,
//BEAT_WARM3,
BEAT_WARM4,
BEAT_TOLL,
BEAT_LINE1,
BEAT_LINE2,
BEAT_LINE3,
BEAT_AMBI,
BEAT_AMBI2,
//BEAT_LOOPA,
BEAT_DRONG,
BEAT_DRONG2,
BEAT_DRONG3,
BEAT_ULTRA,

BEAT_NASICA,
//BEAT_LOOPA2,
BEAT_ORGAN,
BEAT_STRINGY,
BEAT_GATE1,
BEAT_GATE2,
BEAT_GATE3,


BEAT_EMPTY,
BEAT_FINISHED,
NO_WAVS
};


// # is octave
enum
{
NOTE_0C,
NOTE_0CS,
NOTE_0D,
NOTE_0DS,
NOTE_0E,
NOTE_0F,
NOTE_0FS,
NOTE_0G,
NOTE_0GS,
NOTE_0A,
NOTE_0AS,
NOTE_0B,
NOTE_1C,
NOTE_1CS,
NOTE_1D,
NOTE_1DS,
NOTE_1E,
NOTE_1F,
NOTE_1FS,
NOTE_1G,
NOTE_1GS,
NOTE_1A,
NOTE_1AS,
NOTE_1B,
NOTE_2C,
NOTE_2CS,
NOTE_2D,
NOTE_2DS,
NOTE_2E,
NOTE_2F,
NOTE_2FS,
NOTE_2G,
NOTE_2GS,
NOTE_2A,
NOTE_2AS,
NOTE_2B,
NOTE_3C,
NOTE_3CS,
NOTE_3D,
NOTE_3DS,
NOTE_3E,
NOTE_3F,
NOTE_3FS,
NOTE_3G,
NOTE_3GS,
NOTE_3A,
NOTE_3AS,
NOTE_3B,
NOTE_4C,
NOTE_4CS,
NOTE_4D,
NOTE_4DS,
NOTE_4E,
NOTE_4F,
NOTE_4FS,
NOTE_4G,
NOTE_4GS,
NOTE_4A,
NOTE_4AS,
NOTE_4B,
NOTE_ENDNOTE

};

extern int tone [NOTE_ENDNOTE];
