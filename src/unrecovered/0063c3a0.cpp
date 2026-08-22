// ORIGINAL: 0x0063C3A0 sub_63c3a0 0x0063C3A0-0x0063C4D6 FILE
// TRIED: IDA guessed 1 stack arg; ecx=esi with no matching stack read confirms a __thiscall receiver, its own minimal class (no catalogued name). The vtable-slot-58 call needs its own dummy-padded virtual class (same reason as 0x5ED880), so this stays FILE mode. `if (a2==-2){A}else if(a2!=-1){...}` compiles the -2 case inline (jne to skip past it); the original places that handler in a separate far block and jumps to it directly (je) - a block-layout difference this source form did not reproduce. Landed at MISMATCH #4.
// size      310 bytes
// prototype
// callers   0   call targets   4
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00627200 0x0062E540 0x0063C510 0x00644F3A
// indirect  0x0063C3CE 0x0063C4B4 0x0063C4CC

struct SessionStruct;

class Net {
public:
    int join_session(SessionStruct *, char *, char *);
};

extern "C" int __cdecl sub_63c510();
extern "C" int __cdecl _abs(int);
int __cdecl pop(char *, char *, int, int, int (__cdecl *)());

static int *const g_00698c50 = (int *)0x00698C50;
static int *const g_00698c60 = (int *)0x00698C60;
static int *const g_009bb5e8 = (int *)0x009BB5E8;

class VCall58 {
public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual void slot009();
    virtual void slot010();
    virtual void slot011();
    virtual void slot012();
    virtual void slot013();
    virtual void slot014();
    virtual void slot015();
    virtual void slot016();
    virtual void slot017();
    virtual void slot018();
    virtual void slot019();
    virtual void slot020();
    virtual void slot021();
    virtual void slot022();
    virtual void slot023();
    virtual void slot024();
    virtual void slot025();
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032();
    virtual void slot033();
    virtual void slot034();
    virtual void slot035();
    virtual void slot036();
    virtual void slot037();
    virtual void slot038();
    virtual void slot039();
    virtual void slot040();
    virtual void slot041();
    virtual void slot042();
    virtual void slot043();
    virtual void slot044();
    virtual void slot045();
    virtual void slot046();
    virtual void slot047();
    virtual void slot048();
    virtual void slot049();
    virtual void slot050();
    virtual void slot051();
    virtual void slot052();
    virtual void slot053();
    virtual void slot054();
    virtual void slot055();
    virtual void slot056();
    virtual void slot057();
    virtual void slot058();
};

class MPSession {
public:
    void method(int a2);
};

void MPSession::method(int a2) {
    char *self = reinterpret_cast<char *>(this);

    if (a2 == -2) {
        *reinterpret_cast<int *>(self + 0x37a4) = 0;
        reinterpret_cast<VCall58 *>(this)->slot058();
        return;
    }
    if (a2 != -1) {
        if (a2 != 0) {
            return;
        }
        if (sub_63c510() != 0) {
            return;
        }
        reinterpret_cast<VCall58 *>(this)->slot058();
        return;
    }

    int r = pop(reinterpret_cast<char *>(g_00698c60), reinterpret_cast<char *>(g_00698c50), 0x3c, 0x44, 0);
    if (r == -1) {
        return;
    }

    int base = *reinterpret_cast<int *>(self + 0xa50);
    int val = *reinterpret_cast<int *>(*reinterpret_cast<int *>(base + 8) + 0xb3c + reinterpret_cast<int>(self));

    if (val <= *reinterpret_cast<int *>(self + 0x36b8) - 1) {
        *reinterpret_cast<int *>(self + 0x36b4) = *reinterpret_cast<int *>(self + 0x36b0);
        if (val < 0) {
            int n = _abs(val);
            if (*reinterpret_cast<int *>(self + 0x36b8) < n) {
                goto join;
            }
            n = _abs(val);
            while (n > 0) {
                --n;
                *reinterpret_cast<int *>(self + 0x36b4) =
                    *reinterpret_cast<int *>(*reinterpret_cast<int *>(self + 0x36b4) + 0x10);
            }
            val += *reinterpret_cast<int *>(self + 0x36b8);
        } else {
            int n = val;
            while (n > 0) {
                --n;
                *reinterpret_cast<int *>(self + 0x36b4) =
                    *reinterpret_cast<int *>(*reinterpret_cast<int *>(self + 0x36b4) + 0xc);
            }
        }
        *reinterpret_cast<int *>(self + 0x36bc) = val;
    }
join:
    int sessionArg;
    if (*reinterpret_cast<int *>(self + 0x36b0) == 0) {
        sessionArg = 0;
    } else {
        sessionArg = *reinterpret_cast<int *>(*reinterpret_cast<int *>(self + 0x36b4) + 8);
    }

    Net *net = *reinterpret_cast<Net **>(self + 0x36a4);
    int joined = net->join_session(reinterpret_cast<SessionStruct *>(sessionArg),
                                   reinterpret_cast<char *>(g_009bb5e8), 0);
    *reinterpret_cast<int *>(self + 0x37a4) = joined;
    if (joined == 0) {
        return;
    }
    reinterpret_cast<VCall58 *>(this)->slot058();
}
