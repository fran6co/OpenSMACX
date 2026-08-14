// ORIGINAL: 0x005CA970 FILE
// RULED-OUT: reads `[ecx+N]` with no stack arg -> __thiscall receiver, its
//            own minimal class (own field offsets, not a catalogued name).
//            Under frameless /O2 flags this body's prologue and the first
//            ~40 bytes are an exact instruction-for-instruction match, but
//            the scorer's `/Oy-` attempt (forced frame pointer) has a
//            higher overall mnemonic similarity (0.51 vs 0.35) despite
//            diverging on instruction #1, and `_better` ranks by similarity
//            before divergence point - so `/Oy-` is the kept verdict.
//            Not chased past that: the two flag sets disagree on which of
//            this body's branches tracks the original, which is a
//            structural difference deeper in the function, not a cheap
//            source-form tweak.
// name      sub_5ca970
// size      305 bytes
// spans     0x005CA970-0x005CAAA1
// prototype
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005C9EC0
// indirect  0x005CA999 0x005CA9AD 0x005CAA6E 0x005CAA82

extern "C" int __cdecl sub_5c9ec0();

typedef void (__cdecl *SleepFn)(int);
typedef int (__cdecl *SeekFn)(int, int, int);

static int *const g_00669364 = (int *)0x00669364;
static int *const g_0066912c = (int *)0x0066912C;

class AudioChan {
public:
    int *field_0_;
    char pad_004_[0x380 - 4];
    int field_380_;
    int field_384_;
    char pad_388_[0x3a0 - 0x388];
    int field_3a0_;
    int field_3a4_;
    int field_3a8_;

    void sub_5ca970();
};

void AudioChan::sub_5ca970() {
    SleepFn sleep = reinterpret_cast<SleepFn>(*g_0066912c);
    SeekFn seek = reinterpret_cast<SeekFn>(*g_00669364);

    if (field_3a0_ == -1) {
        if ((field_0_[1] & 0x8000) == 0) {
            field_3a0_ = -1;
        } else {
            sleep(0);
            seek(field_0_[0x28], field_3a8_, 0);
            field_3a0_ = sub_5c9ec0();
        }
    }
    if (field_3a0_ != -1 && field_384_ < 0x16 && field_380_ < 0x16) {
        int r = sub_5c9ec0();
        field_3a4_ = r;
        if (r == -1) {
            if ((field_0_[1] & 0x8000) != 0) {
                sleep(0);
                seek(field_0_[0x28], field_3a8_, 0);
                field_3a0_ = sub_5c9ec0();
                return;
            }
            field_3a0_ = -1;
        } else if (r == 1) {
            if (field_0_[0x1f] != 0) {
                int r2 = sub_5c9ec0();
                field_3a0_ = r2;
                if (r2 == 1) {
                    field_0_[0x1f] = 0;
                    return;
                }
            }
        } else if (r == 2) {
            if (field_0_[0x20] == 0 || field_384_ < 0x16) {
                field_3a0_ = sub_5c9ec0();
                return;
            }
        }
    }
}
