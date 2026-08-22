// ORIGINAL: 0x00614B90 ?on_button_clicked@FileWin@@QAEXH@Z 0x00614B90-0x00614C40 FILE
// TRIED: plain if/else-if chain (je/jne #4); single-level `!=` nesting (#6); hoisted &field_1CF8_ local (still diverges #18, deep in case -1 loop)
// working copy - scaffold materialised by --work
// size      176 bytes
// prototype void (__thiscall ?on_button_clicked@FileWin@@QAEXH@Z)(FileWin* this, int)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00614360 0x006146A0
// indirect  0x00614BB3 0x00614C36

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_button_clicked@FileWin@@QAEXH@Z  at 0x00614B90  (176 bytes)
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

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 58
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
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669114 = (int *)0x00669114;
static int *const g_006971c4 = (int *)0x006971C4;
static int *const g_006971c8 = (int *)0x006971C8;
static int *const g_009b7b38 = (int *)0x009B7B38;

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
    uint8_t field_532_[0x106];
    uint32_t field_638_;
    uint32_t field_63C_;
    uint32_t field_640_;
    uint32_t field_644_;
    uint32_t field_648_;
    uint32_t field_64C_;
    uint32_t field_650_;
    uint32_t field_654_;
    uint32_t field_658_;
    uint32_t field_65C_;
    uint32_t field_660_;
    uint8_t field_664_[0xB48];
    uint32_t field_11AC_;
    uint8_t field_11B0_[0x440];
    uint32_t field_15F0_;
    uint8_t field_15F4_[0x704];
    uint32_t field_1CF8_;
    uint8_t field_1CFC_[0x440];
    uint32_t field_213C_;
    uint8_t field_2140_[0x6CC];
    uint32_t field_280C_;
    uint8_t field_2810_[0x28];
    uint32_t field_2838_;
    uint32_t field_283C_;
    uint8_t field_2840_[0x2C];
    uint32_t field_286C_;
    uint8_t field_2870_[0x4];
    uint32_t field_2874_;
    uint8_t field_2878_[0x24];
    uint32_t field_289C_;
    uint8_t field_28A0_[0xB20];
    uint32_t field_33C0_;

    int UNK3(char *);
    void UNK4();
    void on_button_clicked(int);
};
void FileWin::on_button_clicked(int a1) {
    if (a1 != -2) {
        if (a1 != -1) {
            if (a1 == 3) {
                typedef int (__stdcall *SetCurDirFn)(const char *);
                (*reinterpret_cast<SetCurDirFn *>(g_00669114))(reinterpret_cast<char *>(g_006971c4));
                UNK3(reinterpret_cast<char *>(g_006971c8));
                return;
            }
        } else {
            if (*g_009b7b38 == reinterpret_cast<int>(&field_1CF8_)) {
                int list = static_cast<int>(field_63C_) + 0xc8;
                int count = 0;
                if (*reinterpret_cast<int *>(list + 4) != 0) {
                    *reinterpret_cast<int *>(list + 0x10) = 0;
                    *reinterpret_cast<int *>(list + 8) = *reinterpret_cast<int *>(list + 4);
                    if (*reinterpret_cast<int *>(list + 0xc) > 0) {
                        do {
                            int node = *reinterpret_cast<int *>(list + 8);
                            if (*reinterpret_cast<int *>(node + 4) == reinterpret_cast<int>(&field_1CF8_))
                                break;
                            ++count;
                            *reinterpret_cast<int *>(list + 0x10) = *reinterpret_cast<int *>(list + 0x10) + 1;
                            *reinterpret_cast<int *>(list + 8) = *reinterpret_cast<int *>(node + 0xc);
                        } while (count < *reinterpret_cast<int *>(list + 0xc));
                    }
                }
            }
            UNK4();
            return;
        }
    } else {
        field_638_ = 0x12c;
        reinterpret_cast<VCall *>(field_63C_)->slot058();
    }
}
