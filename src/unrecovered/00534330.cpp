// ORIGINAL: 0x00534330 FILE
// RULED-OUT: MISMATCH, closest at 212/204 bytes (mnemonic_similarity 0.87,
//            3 edit blocks). `goto fail;`/`fail:` in place of five
//            scattered `return 0;` was needed to get VC6 to share ONE
//            epilogue the way the original's disassembly jumps into a
//            single shared block - plain early returns each got their
//            own copy. `int code` (signed) not `unsigned short`/`unsigned
//            int` reproduces the `cmp eax,CONST` 32-bit signed compares
//            (jg/jge) the original uses, and writing the boundaries as
//            `<= 0x2406`/`<= 0x2402` (not `< 0x2407`/`< 0x2403`) matches
//            the exact `jg` vs `jge` original picked for each - the
//            `x<N` vs `x<=N-1` lever. `DeletionList*` for `this+0x158c`
//            inlined at each of the three call sites (not cached in an
//            upfront local) matches the original computing it lazily,
//            right before its first use. Remaining diff: the `code==
//            0x2403` arm compiles to `jne fail; jmp common` where the
//            original does a single `je common`, and the `0x2408 ||
//            0x2413` pair still compiles as two `cmp/je` where the
//            original uses a `sub eax,0x2408; je; sub eax,0xb; jne`
//            switch-lowering idiom - not chased further.
// working copy - scaffold materialised by --work
// name      ?fixup_message@NetDaemon@@QAEHPAD@Z
// size      204 bytes
// spans     0x00534330-0x005343FC
// prototype int (__thiscall ?fixup_message@NetDaemon@@QAEHPAD@Z)(NetDaemon* this, int8*)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0052DDC0

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?fixup_message@NetDaemon@@QAEHPAD@Z  at 0x00534330  (204 bytes)
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

class DeletionList;
class NetDaemon;

// ---- callees, declared and never defined (a definition would be inlined) ----
class DeletionList { public:
    uint32_t field_0_;
    uint32_t field_4_;
    uint32_t field_8_;
    uint32_t field_C_;
    uint32_t field_10_;
    uint32_t field_14_;
    uint32_t field_18_;
    uint32_t field_1C_;
    uint32_t field_20_;
    uint32_t field_24_;
    uint32_t field_28_;
    uint32_t field_2C_;
    uint32_t field_30_;
    uint32_t field_34_;
    uint32_t field_38_;
    uint32_t field_3C_;
    uint32_t field_40_;
    uint32_t field_44_;
    uint32_t field_48_;
    uint32_t field_4C_;
    uint32_t field_50_;
    uint32_t field_54_;
    uint32_t field_58_;
    uint32_t field_5C_;
    uint32_t field_60_;
    uint32_t field_64_;
    uint32_t field_68_;
    uint32_t field_6C_;
    uint32_t field_70_;
    uint32_t field_74_;
    uint32_t field_78_;
    uint32_t field_7C_;
    uint32_t field_80_;
    uint32_t field_84_;
    uint32_t field_88_;
    uint32_t field_8C_;
    uint32_t field_90_;
    uint32_t field_94_;
    uint32_t field_98_;
    uint32_t field_9C_;
    uint32_t field_A0_;
    uint32_t field_A4_;
    uint32_t field_A8_;
    uint32_t field_AC_;
    uint32_t field_B0_;
    uint32_t field_B4_;
    uint32_t field_B8_;
    uint32_t field_BC_;
    uint32_t field_C0_;
    uint32_t field_C4_;
    uint32_t field_C8_;
    uint32_t field_CC_;
    uint32_t field_D0_;
    uint32_t field_D4_;
    uint32_t field_D8_;
    uint32_t field_DC_;
    uint32_t field_E0_;
    uint32_t field_E4_;
    uint32_t field_E8_;
    uint32_t field_EC_;
    uint32_t field_F0_;
    uint32_t field_F4_;
    uint32_t field_F8_;
    uint32_t field_FC_;
    uint32_t field_100_;
    uint32_t field_104_;
    uint32_t field_108_;
    uint32_t field_10C_;
    uint32_t field_110_;
    uint32_t field_114_;
    uint32_t field_118_;
    uint32_t field_11C_;
    uint32_t field_120_;
    uint32_t field_124_;
    uint32_t field_128_;
    uint32_t field_12C_;
    uint32_t field_130_;
    uint32_t field_134_;
    uint32_t field_138_;
    uint32_t field_13C_;
    uint32_t field_140_;
    uint32_t field_144_;
    uint32_t field_148_;
    uint32_t field_14C_;
    uint32_t field_150_;
    uint32_t field_154_;
    uint32_t field_158_;
    uint32_t field_15C_;
    uint32_t field_160_;
    uint32_t field_164_;
    uint32_t field_168_;
    uint32_t field_16C_;
    uint32_t field_170_;
    uint32_t field_174_;
    uint32_t field_178_;
    uint32_t field_17C_;
    uint32_t field_180_;
    uint32_t field_184_;
    uint32_t field_188_;
    uint32_t field_18C_;
    uint32_t field_190_;
    uint32_t field_194_;
    uint32_t field_198_;
    uint32_t field_19C_;
    uint32_t field_1A0_;
    uint32_t field_1A4_;
    uint32_t field_1A8_;
    uint32_t field_1AC_;
    uint32_t field_1B0_;
    uint32_t field_1B4_;
    uint32_t field_1B8_;
    uint32_t field_1BC_;
    uint32_t field_1C0_;
    uint32_t field_1C4_;
    uint32_t field_1C8_;
    uint32_t field_1CC_;
    uint32_t field_1D0_;
    uint32_t field_1D4_;
    uint32_t field_1D8_;
    uint32_t field_1DC_;
    uint32_t field_1E0_;
    uint32_t field_1E4_;
    uint32_t field_1E8_;
    uint32_t field_1EC_;
    uint32_t field_1F0_;
    uint32_t field_1F4_;
    uint32_t field_1F8_;
    uint32_t field_1FC_;
    uint32_t field_200_;
    uint32_t field_204_;
    uint32_t field_208_;
    uint32_t field_20C_;
    uint32_t field_210_;
    uint32_t field_214_;
    uint32_t field_218_;
    uint32_t field_21C_;
    uint32_t field_220_;
    uint32_t field_224_;
    uint32_t field_228_;
    uint32_t field_22C_;
    uint32_t field_230_;
    uint32_t field_234_;
    uint32_t field_238_;
    uint32_t field_23C_;
    uint32_t field_240_;
    uint32_t field_244_;
    uint32_t field_248_;
    uint32_t field_24C_;
    uint32_t field_250_;
    uint32_t field_254_;
    uint32_t field_258_;
    uint32_t field_25C_;
    uint32_t field_260_;
    uint32_t field_264_;
    uint32_t field_268_;
    uint32_t field_26C_;
    uint32_t field_270_;
    uint32_t field_274_;
    uint32_t field_278_;
    uint32_t field_27C_;
    uint32_t field_280_;
    uint32_t field_284_;
    uint32_t field_288_;
    uint32_t field_28C_;
    uint32_t field_290_;
    uint32_t field_294_;
    uint32_t field_298_;
    uint32_t field_29C_;
    uint32_t field_2A0_;
    uint32_t field_2A4_;
    uint32_t field_2A8_;
    uint32_t field_2AC_;
    uint32_t field_2B0_;
    uint32_t field_2B4_;
    uint32_t field_2B8_;
    uint32_t field_2BC_;
    uint32_t field_2C0_;
    uint32_t field_2C4_;
    uint32_t field_2C8_;
    uint32_t field_2CC_;
    uint32_t field_2D0_;
    uint32_t field_2D4_;
    uint32_t field_2D8_;
    uint32_t field_2DC_;
    uint32_t field_2E0_;
    uint32_t field_2E4_;
    uint32_t field_2E8_;
    uint32_t field_2EC_;
    uint32_t field_2F0_;
    uint32_t field_2F4_;
    uint32_t field_2F8_;
    uint32_t field_2FC_;
    uint32_t field_300_;
    uint32_t field_304_;
    uint32_t field_308_;
    uint32_t field_30C_;
    uint32_t field_310_;
    uint32_t field_314_;
    uint32_t field_318_;
    uint32_t field_31C_;
    uint32_t field_320_;
    uint32_t field_324_;
    uint32_t field_328_;
    uint32_t field_32C_;
    uint32_t field_330_;
    uint32_t field_334_;
    uint32_t field_338_;
    uint32_t field_33C_;
    uint32_t field_340_;
    uint32_t field_344_;
    uint32_t field_348_;
    uint32_t field_34C_;
    uint32_t field_350_;
    uint32_t field_354_;
    uint32_t field_358_;
    uint32_t field_35C_;
    uint32_t field_360_;
    uint32_t field_364_;
    uint32_t field_368_;
    uint32_t field_36C_;
    uint32_t field_370_;
    uint32_t field_374_;
    uint32_t field_378_;
    uint32_t field_37C_;
    uint32_t field_380_;
    uint32_t field_384_;
    uint32_t field_388_;
    uint32_t field_38C_;
    uint32_t field_390_;
    uint32_t field_394_;
    uint32_t field_398_;
    uint32_t field_39C_;
    uint32_t field_3A0_;
    uint32_t field_3A4_;
    uint32_t field_3A8_;
    uint32_t field_3AC_;
    uint32_t field_3B0_;
    uint32_t field_3B4_;
    uint32_t field_3B8_;
    uint32_t field_3BC_;
    uint32_t field_3C0_;
    uint32_t field_3C4_;
    uint32_t field_3C8_;
    uint32_t field_3CC_;
    uint32_t field_3D0_;
    uint32_t field_3D4_;
    uint32_t field_3D8_;
    uint32_t field_3DC_;
    uint32_t field_3E0_;
    uint32_t field_3E4_;
    uint32_t field_3E8_;
    uint32_t field_3EC_;
    uint32_t field_3F0_;
    uint32_t field_3F4_;
    uint32_t field_3F8_;
    uint32_t field_3FC_;
    uint32_t field_400_;
    uint32_t field_404_;
    uint32_t field_408_;
    uint32_t field_40C_;
    uint32_t field_410_;
    uint32_t field_414_;
    uint32_t field_418_;
    uint32_t field_41C_;
    uint32_t field_420_;
    uint32_t field_424_;
    uint32_t field_428_;
    uint32_t field_42C_;
    uint32_t field_430_;
    uint32_t field_434_;
    uint32_t field_438_;
    uint32_t field_43C_;
    uint32_t field_440_;
    uint32_t field_444_;
    uint32_t field_448_;
    uint32_t field_44C_;
    uint32_t field_450_;
    uint32_t field_454_;
    uint32_t field_458_;
    uint32_t field_45C_;
    uint32_t field_460_;
    uint32_t field_464_;
    uint32_t field_468_;
    uint32_t field_46C_;
    uint32_t field_470_;
    uint32_t field_474_;
    uint32_t field_478_;
    uint32_t field_47C_;
    uint32_t field_480_;
    uint32_t field_484_;
    uint32_t field_488_;
    uint32_t field_48C_;
    uint32_t field_490_;
    uint32_t field_494_;
    uint32_t field_498_;
    uint32_t field_49C_;
    uint32_t field_4A0_;
    uint32_t field_4A4_;
    uint32_t field_4A8_;
    uint32_t field_4AC_;
    uint32_t field_4B0_;
    uint32_t field_4B4_;
    uint32_t field_4B8_;
    uint32_t field_4BC_;
    uint32_t field_4C0_;
    uint32_t field_4C4_;
    uint32_t field_4C8_;
    uint32_t field_4CC_;
    uint32_t field_4D0_;
    uint32_t field_4D4_;
    uint32_t field_4D8_;
    uint32_t field_4DC_;
    uint32_t field_4E0_;
    uint32_t field_4E4_;
    uint32_t field_4E8_;
    uint32_t field_4EC_;
    uint32_t field_4F0_;
    uint32_t field_4F4_;
    uint32_t field_4F8_;
    uint32_t field_4FC_;
    uint32_t field_500_;
    uint32_t field_504_;
    uint32_t field_508_;
    uint32_t field_50C_;
    uint32_t field_510_;
    uint32_t field_514_;
    uint32_t field_518_;
    uint32_t field_51C_;
    uint32_t field_520_;
    uint32_t field_524_;
    uint32_t field_528_;
    uint32_t field_52C_;
    uint32_t field_530_;
    uint32_t field_534_;
    uint32_t field_538_;
    uint32_t field_53C_;
    uint32_t field_540_;
    uint32_t field_544_;
    uint32_t field_548_;
    uint32_t field_54C_;
    uint32_t field_550_;
    uint32_t field_554_;
    uint32_t field_558_;
    uint32_t field_55C_;
    uint32_t field_560_;
    uint32_t field_564_;
    uint32_t field_568_;
    uint32_t field_56C_;
    uint32_t field_570_;
    uint32_t field_574_;
    uint32_t field_578_;
    uint32_t field_57C_;
    uint32_t field_580_;
    uint32_t field_584_;
    uint32_t field_588_;
    uint32_t field_58C_;
    uint32_t field_590_;
    uint32_t field_594_;
    uint32_t field_598_;
    uint32_t field_59C_;
    uint32_t field_5A0_;
    int convert(int, int);
};


class NetDaemon { public:
    int fixup_message(char *);
};
int NetDaemon::fixup_message(char * a1) {
    unsigned short *msg = reinterpret_cast<unsigned short *>(a1);
    int code = *msg;
    if (code <= 0x2406) {
        if (code < 0x2404) {
            if (code <= 0x2402) {
                if (code < 0x2401) {
                    if (code != 0x2101) {
                        goto fail;
                    }
                    if (msg[8] != 0x11) {
                        goto fail;
                    }
                    int r1 = reinterpret_cast<DeletionList *>(reinterpret_cast<char *>(this) + 0x158c)
                                 ->convert(*reinterpret_cast<int *>(msg + 10), *reinterpret_cast<int *>(msg + 6));
                    *reinterpret_cast<int *>(msg + 10) = r1;
                    if (*(reinterpret_cast<char *>(msg) + 0x35) != 1) {
                        goto fail;
                    }
                    if (static_cast<short>(msg[0x1c]) < 0) {
                        goto fail;
                    }
                    int r2 = reinterpret_cast<DeletionList *>(reinterpret_cast<char *>(this) + 0x158c)
                                 ->convert(static_cast<short>(msg[0x1c]), *reinterpret_cast<int *>(msg + 6));
                    unsigned short r2s = static_cast<unsigned short>(r2);
                    msg[0x1c] = r2s;
                    if (r2s != 0xffff) {
                        goto fail;
                    }
                    *(reinterpret_cast<char *>(msg) + 0x35) = 0;
                    goto fail;
                }
            } else if (code != 0x2403) {
                goto fail;
            }
        }
    } else if (code != 0x2408 && code != 0x2413) {
        goto fail;
    }
    {
        int r3 = reinterpret_cast<DeletionList *>(reinterpret_cast<char *>(this) + 0x158c)
                     ->convert(*reinterpret_cast<int *>(msg + 8), *reinterpret_cast<int *>(msg + 6));
        *reinterpret_cast<int *>(msg + 8) = r3;
        if (r3 >= 0) {
            goto fail;
        }
        return 1;
    }
fail:
    return 0;
}
