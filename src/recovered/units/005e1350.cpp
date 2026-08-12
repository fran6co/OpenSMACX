// ORIGINAL: 0x005E1350 FILE
// name      ?vline@Buffer@@QAEXHHHHHHHH@Z
// size      663 bytes
// spans     0x005E1350-0x005E15E7
// prototype void (__thiscall ?vline@Buffer@@QAEXHHHHHHHH@Z)(Buffer* this, int, int, int, int, int, int, int, int)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005E1471 0x005E15D2
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005E1350
// measured tier  MISMATCH
// divergence     8
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005e1350/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?vline@Buffer@@QAEXHHHHHHHH@Z  at 0x005E1350  (663 bytes)
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

struct BITMAPINFO;
typedef int BOOL;
class Buffer;
class Font;
typedef void * HBITMAP;
typedef void * HDC;
typedef void * HFONT;
typedef void * HRGN;
typedef char * LPSTR;
typedef void * LPVOID;
struct RECT;
class Spot;

class Font { public:
    int unk_1_;
    BOOL is_fot_set_;
    HFONT font_obj_;
    int line_height_;
    int height_;
    int internal_leading_;
    int ascent_;
    int descent_;
    int pad_;
    LPSTR fot_file_name_;
};

struct RECT {
    long left;
    long top;
    long right;
    long bottom;
};

class Spot { public:
    void * spots_;
    uint32_t max_count_;
    uint32_t add_count_;
};

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 25, 32
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
    virtual void slot025();  // <-- used
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032();  // <-- used
};

class Buffer { public:
    LPVOID vtable_;
    uint32_t poOwner_;
    uint32_t field_8_;
    uint32_t field_C_;
    uint32_t field_10_;
    uint32_t field_14_;
    uint32_t field_18_;
    uint32_t field_1C_;
    RECT rect1_;
    RECT rect2_;
    uint32_t field_40_[4];
    uint32_t field_50_;
    LPVOID * ppv_bits_;
    uint32_t field_58_;
    uint32_t field_5C_;
    HDC hdc2_;
    HDC hdc_;
    uint32_t field_68_;
    uint32_t field_6C_;
    HRGN field_70_;
    uint32_t field_74_;
    HBITMAP bitmap_handle_;
    const BITMAPINFO * bitmap_info_;
    uint32_t width_;
    uint32_t height_;
    uint16_t field_88_;
    uint16_t field_8A_;
    uint32_t field_8C_;
    uint32_t field_90_;
    uint32_t field_94_;
    uint32_t field_98_;
    uint32_t field_9C_;
    uint32_t field_A0_;
    int32_t dib_[256];
    uint32_t field_4A4_;
    uint32_t field_4A8_;
    uint32_t field_4AC_;
    Spot spot_;
    uint8_t field_4BC_[80];
    uint32_t field_50C_;
    uint32_t field_510_;
    uint32_t field_514_;
    uint32_t field_518_;
    uint32_t field_51C_;
    uint32_t field_520_;
    uint32_t field_524_;
    uint32_t field_528_;
    Font * font1_;
    Font * font2_;
    Font * font3_;
    Font * font4_;
    uint32_t color_val_1_;
    uint32_t color_2_val_1_;
    uint32_t color_3_val_1_;
    uint32_t color_hyper_val_1_;
    uint32_t color_val_2_;
    uint32_t color_2_val_2_;
    uint32_t color_3_val_2_;
    uint32_t color_hyper_val_2_;
    uint32_t color_val_3_;
    uint32_t color_2_val_3_;
    uint32_t color_3_val_3_;
    uint32_t color_hyper_val_3_;
    uint32_t color_val_4_;
    uint32_t color_2_val_4_;
    uint32_t color_3_val_4_;
    uint32_t color_hyper_val_4_;
    uint32_t field_57C_;
    int8_t field_580_;
    uint32_t field_584_;

    void vline(int, int, int, int, int, int, int, int);
};

// The emitter's VCall shim declares slot025/slot032 nullary; both calls here
// re-push the object pointer as an explicit first stack argument (this is a
// raw function-pointer vtable, not a normal __thiscall dispatch), so this
// shim widens both slots to take that pointer plus the real arguments.
class VCall2 { public:
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
    virtual int slot025(int, int, void *, int, int);  // <-- used, widened
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032(int, int);  // <-- used, widened
};

void Buffer::vline(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8) {
    char *self = reinterpret_cast<char *>(this);

    if (*reinterpret_cast<int *>(self + 0x54) != 0 || *reinterpret_cast<int *>(self + 0x58) != 0) {
        if (a1 >= *reinterpret_cast<int *>(self + 0x20)) {
            if (a1 < *reinterpret_cast<int *>(self + 0x28)) {
                if (a2 != a3) {
                    if (a4 != -1 || a5 != -1) {
                        if (a3 < a2) {
                            int t;
                            t = a2; a2 = a3; a3 = t;
                            t = a4; a4 = a5; a5 = t;
                            a8 = (a7 - a8) + a6;
                            t = a6; a6 = a7; a7 = t;
                        }

                        if (a2 < *reinterpret_cast<int *>(self + 0x2c)) {
                            if (*reinterpret_cast<int *>(self + 0x24) <= a3) {
                                if (a2 < *reinterpret_cast<int *>(self + 0x24)) {
                                    a2 = *reinterpret_cast<int *>(self + 0x24);
                                }
                                if (*reinterpret_cast<int *>(self + 0x2c) <= a3) {
                                    a3 = *reinterpret_cast<int *>(self + 0x2c) - 1;
                                }

                                if (a1 < *reinterpret_cast<int *>(self + 0x80)) {
                                    if (a2 < -*reinterpret_cast<int *>(self + 0x84)) {
                                        void *owner = *reinterpret_cast<void **>(self + 0x58);
                                        if (owner == 0) {
                                            int v = *reinterpret_cast<int *>(self + 0x54);
                                            *reinterpret_cast<int *>(self + 0x50) = v;
                                            if (v == 0) {
                                                return;
                                            }
                                            ++*reinterpret_cast<int *>(self + 0x6c);
                                        } else if (*reinterpret_cast<int *>(self + 0x50) == 0) {
                                            int localBuf[10];
                                            localBuf[0] = 0x6c;
                                            int rc = reinterpret_cast<VCall2 *>(owner)->slot025(
                                                reinterpret_cast<int>(owner), 0, localBuf, 1, 0);
                                            if (rc != 0) {
                                                return;
                                            }
                                            ++*reinterpret_cast<int *>(self + 0x6c);
                                            *reinterpret_cast<int *>(self + 0x4a8) = localBuf[4];
                                            *reinterpret_cast<int *>(self + 0x50) = localBuf[9];
                                        } else {
                                            ++*reinterpret_cast<int *>(self + 0x6c);
                                        }

                                        if (*reinterpret_cast<int *>(self + 0x50) != 0) {
                                            int stride = *reinterpret_cast<int *>(self + 0x4a8);
                                            uint8_t *p = reinterpret_cast<uint8_t *>(
                                                stride * a2 + *reinterpret_cast<int *>(self + 0x50) + a1);
                                            if (p != 0) {
                                                int phase = a8 % (a6 + a7);
                                                if (phase < a6) {
                                                    phase = phase - a6;
                                                } else {
                                                    phase = (a6 - phase) + a7;
                                                }
                                                int negA6 = -a6;
                                                int count = (a3 - a2) + 1;

                                                if (a4 == -1) {
                                                    do {
                                                        if (phase < 0) {
                                                            phase++;
                                                            if (phase == 0) phase = a7;
                                                        } else {
                                                            *p = static_cast<uint8_t>(a5);
                                                            phase--;
                                                            if (phase == 0) phase = negA6;
                                                        }
                                                        p += stride;
                                                    } while (--count);
                                                } else if (a5 == -1) {
                                                    do {
                                                        if (phase < 0) {
                                                            *p = static_cast<uint8_t>(a4);
                                                            phase++;
                                                            if (phase == 0) phase = a7;
                                                        } else {
                                                            phase--;
                                                            if (phase == 0) phase = negA6;
                                                        }
                                                        p += stride;
                                                    } while (--count);
                                                } else {
                                                    do {
                                                        if (phase < 0) {
                                                            *p = static_cast<uint8_t>(a4);
                                                            phase++;
                                                            if (phase == 0) phase = a7;
                                                        } else {
                                                            *p = static_cast<uint8_t>(a5);
                                                            phase--;
                                                            if (phase == 0) phase = negA6;
                                                        }
                                                        p += stride;
                                                    } while (--count);
                                                }

                                                owner = *reinterpret_cast<void **>(self + 0x58);
                                                if (owner == 0) {
                                                    int c = *reinterpret_cast<int *>(self + 0x6c) - 1;
                                                    *reinterpret_cast<int *>(self + 0x6c) = c;
                                                    if (c < 1) {
                                                        *reinterpret_cast<int *>(self + 0x50) = 0;
                                                        *reinterpret_cast<int *>(self + 0x6c) = 0;
                                                        return;
                                                    }
                                                } else {
                                                    int c = *reinterpret_cast<int *>(self + 0x6c) - 1;
                                                    *reinterpret_cast<int *>(self + 0x6c) = c;
                                                    if (*reinterpret_cast<int *>(self + 0x50) != 0 && c < 1) {
                                                        reinterpret_cast<VCall2 *>(owner)->slot032(
                                                            reinterpret_cast<int>(owner),
                                                            *reinterpret_cast<int *>(self + 0x50));
                                                        *reinterpret_cast<int *>(self + 0x50) = 0;
                                                        *reinterpret_cast<int *>(self + 0x6c) = 0;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
