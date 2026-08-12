// ORIGINAL: 0x006146A0 FILE
// name      ?UNK4@FileWin@@QAEXXZ
// size      1209 bytes
// spans     0x006146A0-0x00614B59
// prototype void (__thiscall ?UNK4@FileWin@@QAEXXZ)(FileWin* this)
// callers   2   call targets   10
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006007B0 0x00614360 0x00625EC0 0x00627260 0x00628AF0 0x006453E0 0x00645470 0x00645660 0x00645DD0 0x006474A0
// indirect  0x006146FE 0x006147DA 0x006147E6 0x0061480D 0x00614891 0x006148E6 0x00614947 0x00614958 0x006149EE 0x00614A82 0x00614A9D 0x00614B48
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x006146A0
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/006146a0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?UNK4@FileWin@@QAEXXZ  at 0x006146A0  (1209 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.

typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
// `char`, NOT `signed char`. They are distinct MSVC types and mangle
// differently - D against C - and the catalogue's `int8` means the first:
// counted over every catalogued mangled name, `PAD` appears 508 times and
// `PAC` once. Spelling it `signed char` made 150 derived prototypes emit a
// symbol no target object holds. `int8_t` keeps its C meaning below; neither
// catalogue ever uses it.
typedef char int8;
typedef unsigned char uint8;

class FileWin;

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" char *strcat(char *, const char *);
extern "C" int __cdecl strchr();
extern "C" int __cdecl strcspn();
extern "C" int strcmp(const char *, const char *);
extern "C" unsigned int strlen(const char *);
int __cdecl memrchr(const void *, const void *, int);
int __cdecl parse_says(int, char *, int, int);
int __cdecl pop(char *, char *, int, int (__cdecl *)());
void __cdecl purge_spaces(char *);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 58, 62, 65
class VCall { public:
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
    virtual void slot058();  // <-- used
    virtual void slot059();
    virtual void slot060();
    virtual void slot061();
    virtual void slot062();  // <-- used
    virtual void slot063();
    virtual void slot064();
    virtual void slot065();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_006690e0 = (int *)0x006690E0;
static int *const g_006690e4 = (int *)0x006690E4;
static int *const g_0066911c = (int *)0x0066911C;
static int *const g_0069714c = (int *)0x0069714C;
static int *const g_00697150 = (int *)0x00697150;
static int *const g_00697154 = (int *)0x00697154;
static int *const g_00697158 = (int *)0x00697158;
static int *const g_0069715c = (int *)0x0069715C;
static int *const g_00697164 = (int *)0x00697164;
static int *const g_00697174 = (int *)0x00697174;
static int *const g_00697178 = (int *)0x00697178;
static int *const g_00697180 = (int *)0x00697180;
static int *const g_00697194 = (int *)0x00697194;
static int *const g_006971a0 = (int *)0x006971A0;
static int *const g_006971a4 = (int *)0x006971A4;
static int *const g_006971ac = (int *)0x006971AC;
static int *const g_006971c0 = (int *)0x006971C0;

class FileWin { public:
    uint8_t unmapped_0_[0x208];
    uint8_t field_208_;
    uint8_t unmapped_209_[0x30C - 0x209];
    uint8_t field_30C_;
    uint8_t unmapped_30D_[0x410 - 0x30D];
    uint8_t field_410_;
    uint8_t unmapped_411_[0x514 - 0x411];
    int32_t field_514_;
    uint8_t unmapped_518_[0x531 - 0x518];
    uint8_t field_531_;
    uint8_t unmapped_532_[0x33C4 - 0x532];

    int UNK3(char *);
    void UNK4();
};

class VCall65 { public:
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
    virtual void slot059();
    virtual void slot060();
    virtual void slot061();
    virtual void slot062();
    virtual void slot063();
    virtual void slot064();
    virtual void slot065(int);
};

struct FindDataBlob { char data[320]; };

void FileWin::UNK4() {
    char *self = reinterpret_cast<char *>(this);
    int *fld = reinterpret_cast<int *>(self);

    int counter = 0;
    int *piVar2;
    int *piVar1 = reinterpret_cast<int *>(self + 0x1cf8);
    char *pathBuf = self + 0x270c;

    if (*reinterpret_cast<int *>(*reinterpret_cast<char **>(self + 0x63c) + 0xcc) == 0) {
        piVar2 = 0;
    } else {
        piVar2 = *reinterpret_cast<int **>(
            *reinterpret_cast<int *>(*reinterpret_cast<char **>(self + 0x63c) + 0xd0) + 4);
    }

    *reinterpret_cast<int *>(self + 0x2838) = 0;
    unsigned int nameLen = strlen(pathBuf);
    *reinterpret_cast<unsigned int *>(self + 0x283c) = nameLen;

    reinterpret_cast<VCall *>(piVar1)->slot062();
    purge_spaces(pathBuf);

    int status;
    char nameBuf[260];

    if (strcmp(pathBuf, reinterpret_cast<char *>(g_0069714c)) == 0) {
        status = 2;
    } else {
        nameBuf[0] = 0;
        strcat(nameBuf, pathBuf);
        unsigned int len2 = strlen(nameBuf);
        char *dot = reinterpret_cast<char *>(memrchr(nameBuf, nameBuf + len2, 0x2e));
        status = 0;

        if (dot != nameBuf && 0 < fld[0x145]) {
            char *cursor = self + 0x518;
            do {
                if (dot == 0) {
                    unsigned int len3 = strlen(nameBuf);
                    char *dot2 = reinterpret_cast<char *>(memrchr(nameBuf, nameBuf + len3, 0x2e));
                    if (dot2 != 0) {
                        *dot2 = 0;
                    }
                    strcat(nameBuf, cursor);
                }

                FindDataBlob findData;
                void *h = (*reinterpret_cast<void *(__stdcall **)(const char *, FindDataBlob *)>(
                    g_006690e0))(nameBuf, &findData);
                if (h != reinterpret_cast<void *>(-1)) {
                    status = 1;
                    (*reinterpret_cast<int(__stdcall **)(void *)>(g_0066911c))(h);
                    break;
                }
                (*reinterpret_cast<int(__stdcall **)(void *)>(g_0066911c))(reinterpret_cast<void *>(-1));
                ++counter;
                cursor += 5;
            } while (counter < fld[0x145]);
        }
    }

    if (status == 0) {
        if (UNK3(pathBuf) != 0) {
            goto tail;
        }
    } else if (status != 1) {
        if (status == 2) {
            nameBuf[0] = 0;
            strcat(nameBuf, reinterpret_cast<char *>(g_006971c0));
            UNK3(nameBuf);
        }
        goto tail;
    }

    {
        char *dirBuf = self + 0x208;
        char *fileBuf = self + 0x30c;
        *dirBuf = 0;
        *fileBuf = 0;

        char *slash = reinterpret_cast<char *(__cdecl *)(const char *, int)>(&strchr)(nameBuf, 0x5c);
        if (slash == 0) {
            (*reinterpret_cast<void(__stdcall **)(unsigned int, char *)>(g_006690e4))(0x104, dirBuf);
            unsigned int dlen = strlen(dirBuf);
            if (nameBuf[0] != '\\' && dirBuf[dlen - 1] != '\\') {
                strcat(dirBuf, reinterpret_cast<char *>(g_00697154));
            }
            strcat(dirBuf, nameBuf);
            *fileBuf = 0;
            strcat(fileBuf, nameBuf);
        } else {
            if (nameBuf[1] != ':') {
                (*reinterpret_cast<void(__stdcall **)(unsigned int, char *)>(g_006690e4))(0x104, dirBuf);
                if (nameBuf[0] != '\\') {
                    strcat(dirBuf, reinterpret_cast<char *>(g_00697150));
                }
            }
            strcat(dirBuf, nameBuf);
            unsigned int dlen2 = strlen(dirBuf);
            char *p = dirBuf + dlen2;
            while (p != 0 && *p != '\\') {
                --p;
            }
            strcat(fileBuf, p + 1);
        }

        FindDataBlob findData2;
        void *h2 = (*reinterpret_cast<void *(__stdcall **)(const char *, FindDataBlob *)>(
            g_006690e0))(nameBuf, &findData2);
        bool exists = h2 != reinterpret_cast<void *>(-1);
        if (exists) {
            (*reinterpret_cast<int(__stdcall **)(void *)>(g_0066911c))(h2);
        }

        if (fld[0x18e] == 200) {
            if (exists) {
                reinterpret_cast<VCall *>(*reinterpret_cast<int **>(self + 0x63c))->slot058();
            } else {
                parse_says(0, nameBuf, -1, -1);
                char msg[260];
                msg[0] = 0;
                strcat(msg, self + 0x104);
                strcat(msg, reinterpret_cast<char *>(g_00697158));
                strcat(msg, reinterpret_cast<char *>(g_0069715c));
                pop(msg, reinterpret_cast<char *>(g_00697164), 0x200000, 0);
            }
        } else if (fld[0x18e] == 100) {
            if (exists) {
                parse_says(0, nameBuf, -1, -1);
                char msg[260];
                msg[0] = 0;
                strcat(msg, self + 0x104);
                strcat(msg, reinterpret_cast<char *>(g_00697174));
                strcat(msg, reinterpret_cast<char *>(g_00697178));
                if (pop(msg, reinterpret_cast<char *>(g_00697180), 0x200040, 0) != 0) {
                    goto tail;
                }
            } else {
                unsigned int spn = reinterpret_cast<unsigned int (__cdecl *)(const char *, const char *)>(&strcspn)(fileBuf, reinterpret_cast<char *>(g_00697194));
                unsigned int flen = strlen(fileBuf);
                if (spn != flen) {
                    char msg[260];
                    msg[0] = 0;
                    strcat(msg, self + 0x104);
                    strcat(msg, reinterpret_cast<char *>(g_006971a0));
                    strcat(msg, reinterpret_cast<char *>(g_006971a4));
                    pop(msg, reinterpret_cast<char *>(g_006971ac), 0x200040, 0);
                    goto tail;
                }
            }
            reinterpret_cast<VCall *>(*reinterpret_cast<int **>(self + 0x63c))->slot058();
        }
    }

tail:
    if (piVar2 == piVar1) {
        reinterpret_cast<VCall65 *>(*reinterpret_cast<int **>(self + 0x63c))->slot065(reinterpret_cast<int>(piVar1));
        return;
    }
    {
        int arg = 0;
        if (self + 0x286c != 0) {
            arg = *reinterpret_cast<int *>(*reinterpret_cast<int *>(self + 0x286c) + 4) + 0x286c +
                  reinterpret_cast<int>(self);
        }
        reinterpret_cast<VCall65 *>(*reinterpret_cast<int **>(self + 0x63c))->slot065(arg);
    }
}
