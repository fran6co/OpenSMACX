// ORIGINAL: 0x0050C340 ?steal_tech@@YAHHHH@Z 0x0050C340-0x0050C4AF
// RULED-OUT: writing the tech-flag bit test with an intermediate `and` result (rather than folding into the `if`'s condition) adds an extra `mov`; a ternary for the message-name select lowers to `neg/sbb/and` instead of the original's `mov`/`test`/`je` overwrite. Landed body: 0.976 mnemonic similarity, first divergence at #17 (one extra `mov`, one `lea` vs `add` in the tech-preference-table address computation), not chased further.
// size      367 bytes
// prototype int (__cdecl ?steal_tech@@YAHHHH@Z)(int factionID1, int factionID2, int)
// callers   2   call targets   7
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A890 0x004E6300 0x00592EE0 0x005B9C40 0x005BB000 0x005BEB70 0x00625EC0

typedef unsigned char uint8_t;

class NetMsg;

extern "C" int __cdecl sub_5beb70(int, int, int, char *);
int parse_says(int, char *, int, int);
void bases_reset(int, int, int);
void message_data(int, int, int, int, int, int);
void say_tech(char *, int, int);
void tech_achieved(int, int, int, int);

class NetMsg { public:
    void pop(const char *, int, int, const char *);
};

static NetMsg *const g_net_msg = (NetMsg *)0x00805338;
static int *const g_diplo_flag = (int *)0x0093F660;
static uint8_t *const g_tech_flags = (uint8_t *)0x009A64E8;
static int *const g_player_id = (int *)0x00939284;
static char *const g_str_steal_tech = (char *)0x0068A334;
static char *const g_str_acquire_tech = (char *)0x0068A340;
static int *const g_tech_pref_a = (int *)0x00946D4C;
static int *const g_tech_pref_b = (int *)0x00946D50;
static char *const g_tech_says_base = (char *)0x00946D34;
static int *const g_say_out_a = (int *)0x009BBFEC;
static int *const g_say_out_b = (int *)0x009BBFF0;
static char *const g_say_buf = (char *)0x009B86A0;
static char *const g_str_stole_tech = (char *)0x0068A34C;

int __cdecl steal_tech(int a1, int a2, int a3) {
    if (a1 == 0 || a2 == 0) {
        return 0;
    }
    if (*g_diplo_flag != 0) {
        if ((*g_tech_flags & (1 << a1)) != 0 && a1 != *g_player_id) {
            return 0;
        }
    }

    char *name = g_str_steal_tech;
    if (a3 == 0) {
        name = g_str_acquire_tech;
    }
    int cost = sub_5beb70(a1, 0, a2, name);
    if (cost < 0) {
        cost = 9999;
    }

    if (*g_diplo_flag != 0 && a1 == *g_player_id) {
        message_data(0x244c, 0, a1, cost, a2, 0);
        return cost == 9999;
    }

    if (a2 == *g_player_id && cost != 9999) {
        *g_say_out_a = g_tech_pref_a[a1 * 0x167];
        *g_say_out_b = g_tech_pref_b[a1 * 0x167];
        parse_says(0, g_tech_says_base + a1 * 0x59c, -1, -1);
        *g_say_buf = 0;
        say_tech(g_say_buf, cost, 0);
        parse_says(1, g_say_buf, -1, -1);
        g_net_msg->pop(g_str_stole_tech, 5000, 0, 0);
    }

    tech_achieved(a1, cost, a2, 0);

    if (((*g_tech_flags & (1 << a1)) == 0) && cost != 9999) {
        bases_reset(-1, a1, 0);
    }

    return cost == 9999;
}
