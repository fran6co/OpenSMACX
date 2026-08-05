// Recovery leaf tests: pops_and_console.
//
// Carved verbatim out of tests/recovery_leaf_tests.cpp.  Add a case by
// appending a `void test_*()` here plus `LEAF_CASE(LEAF_APPEND, test_*);` at
// the bottom.  This file is the only file that edit touches: the per-family
// case counts are derived from these `LEAF_CASE(` lines at build time, and
// LEAF_APPEND needs no offset because ties there break by name.
#include "leaf_shared.h"

#define LEAF_FAMILY "pops_and_console"

namespace {

void *g_datalink_exec_self = nullptr;

unsigned int g_datalink_exec_topic = 0;

int g_datalink_exec_index = 0;

int g_datalink_exec_calls = 0;

void __thiscall observe_datalink_exec(void *self, unsigned int topic,
                                      int index) {
    g_datalink_exec_self = self;
    g_datalink_exec_topic = topic;
    g_datalink_exec_index = index;
    ++g_datalink_exec_calls;
}

}  // namespace

void test_datalink_help_forwarders() {
    // Every help_* forwarder is a thin dispatch to Datalink::exec against the
    // fixed-address Datalink singleton: `DatalinkExec(DatalinkMain, topic,
    // index)`. Rebinding both seams and asserting the observed `self` equals
    // the REBOUND DatalinkMain (not the real singleton address) proves each
    // forwarder reads the seam rather than the original 0x00703EA0 literal.
    auto *const saved_exec = DatalinkExec;
    void *const saved_main = DatalinkMain;
    int fake_datalink = 0;
    DatalinkExec = &observe_datalink_exec;
    DatalinkMain = &fake_datalink;

    // help_tech -> topic 0xE
    g_datalink_exec_calls = 0;
    help_tech(101);
    expect(g_datalink_exec_calls == 1);
    expect(g_datalink_exec_self == &fake_datalink);
    expect(g_datalink_exec_topic == 0xE);
    expect(g_datalink_exec_index == 101);

    // help_weapon -> topic 0x6
    g_datalink_exec_calls = 0;
    help_weapon(102);
    expect(g_datalink_exec_calls == 1);
    expect(g_datalink_exec_self == &fake_datalink);
    expect(g_datalink_exec_topic == 0x6);
    expect(g_datalink_exec_index == 102);

    // help_armor -> topic 0x7
    g_datalink_exec_calls = 0;
    help_armor(103);
    expect(g_datalink_exec_calls == 1);
    expect(g_datalink_exec_self == &fake_datalink);
    expect(g_datalink_exec_topic == 0x7);
    expect(g_datalink_exec_index == 103);

    // help_chassis -> topic 0x4
    g_datalink_exec_calls = 0;
    help_chassis(104);
    expect(g_datalink_exec_calls == 1);
    expect(g_datalink_exec_self == &fake_datalink);
    expect(g_datalink_exec_topic == 0x4);
    expect(g_datalink_exec_index == 104);

    // help_facility -> topic 0xA
    g_datalink_exec_calls = 0;
    help_facility(105);
    expect(g_datalink_exec_calls == 1);
    expect(g_datalink_exec_self == &fake_datalink);
    expect(g_datalink_exec_topic == 0xA);
    expect(g_datalink_exec_index == 105);

    // help_abil -> topic 0x8
    g_datalink_exec_calls = 0;
    help_abil(106);
    expect(g_datalink_exec_calls == 1);
    expect(g_datalink_exec_self == &fake_datalink);
    expect(g_datalink_exec_topic == 0x8);
    expect(g_datalink_exec_index == 106);

    // help_social -> topic 0xC
    g_datalink_exec_calls = 0;
    help_social(107);
    expect(g_datalink_exec_calls == 1);
    expect(g_datalink_exec_self == &fake_datalink);
    expect(g_datalink_exec_topic == 0xC);
    expect(g_datalink_exec_index == 107);

    // help_faction -> topic 0xF
    g_datalink_exec_calls = 0;
    help_faction(108);
    expect(g_datalink_exec_calls == 1);
    expect(g_datalink_exec_self == &fake_datalink);
    expect(g_datalink_exec_topic == 0xF);
    expect(g_datalink_exec_index == 108);

    // help_veh -> topic 0x3
    g_datalink_exec_calls = 0;
    help_veh(109);
    expect(g_datalink_exec_calls == 1);
    expect(g_datalink_exec_self == &fake_datalink);
    expect(g_datalink_exec_topic == 0x3);
    expect(g_datalink_exec_index == 109);

    // help_topic passes its own topic argument straight through, rather than
    // a literal - so this call uses a topic value none of the fixed helpers
    // above use, to make sure it is not silently constant.
    g_datalink_exec_calls = 0;
    help_topic(0x1234, 110);
    expect(g_datalink_exec_calls == 1);
    expect(g_datalink_exec_self == &fake_datalink);
    expect(g_datalink_exec_topic == 0x1234);
    expect(g_datalink_exec_index == 110);

    DatalinkExec = saved_exec;
    DatalinkMain = saved_main;
}

namespace {

void *g_synch_self = nullptr;

int16_t g_synch_opcode = 0;

int g_synch_a = 0;

int g_synch_b = 0;

int g_synch_c = 0;

char *g_synch_text = nullptr;

int g_synch_d = 0;

int16_t g_synch_flags = 0;

int g_synch_calls = 0;

void __thiscall observe_synch(void *daemon, int16_t opcode, int a, int b,
                              int c, char *text, int d, int16_t flags) {
    g_synch_self = daemon;
    g_synch_opcode = opcode;
    g_synch_a = a;
    g_synch_b = b;
    g_synch_c = c;
    g_synch_text = text;
    g_synch_d = d;
    g_synch_flags = flags;
    ++g_synch_calls;
}

}  // namespace

void test_net_daemon_synch_forwarders() {
    // Fourteen forwarders, all one shape: push the four shared literals
    // (0, 0, nullptr, 1, 0x2101) plus one caller id and a fixed opcode,
    // then call NetDaemon::synch on the daemon at 0x0093CD90 - the same
    // receiver NetDaemonNet already binds for the receive path. synch_diplo
    // is the one exception: both leading arguments come from its caller.
    auto *const saved_synch = NetDaemonSynch;
    void *const saved_net = NetDaemonNet;
    int fake_net = 0;
    NetDaemonSynch = &observe_synch;
    NetDaemonNet = &fake_net;

    auto check_id_forward = [&](void (*fn)(int), int16_t opcode, int id) {
        g_synch_calls = 0;
        fn(id);
        expect(g_synch_calls == 1);
        expect(g_synch_self == &fake_net);   // rebound NetDaemonNet, not NetDaemonSynch
        expect(g_synch_opcode == opcode);
        expect(g_synch_a == id);
        expect(g_synch_b == 0);
        expect(g_synch_c == 0);
        expect(g_synch_text == nullptr);
        expect(g_synch_d == 1);
        expect(g_synch_flags == 0x2101);
    };

    check_id_forward(&synch_veh, 0x11, 1001);
    check_id_forward(&synch_base, 0x13, 1002);
    check_id_forward(&synch_energy, 0xB, 1003);
    check_id_forward(&synch_researching, 0xA, 1004);
    check_id_forward(&synch_leader, 0x6, 1005);
    check_id_forward(&synch_ai, 0x8, 1006);
    check_id_forward(&synch_research, 0x9, 1007);
    check_id_forward(&synch_alloc, 0xC, 1008);
    check_id_forward(&synch_soc, 0xD, 1009);
    check_id_forward(&synch_proto, 0xE, 1010);
    check_id_forward(&synch_obs, 0x10, 1011);
    check_id_forward(&synch_template, 0x18, 1012);
    check_id_forward(&synch_radius, 0x23, 1013);

    // synch_diplo takes two caller arguments; two distinct sentinels pin
    // that a and b are not swapped en route to NetDaemon::synch.
    g_synch_calls = 0;
    synch_diplo(2001, 2002);
    expect(g_synch_calls == 1);
    expect(g_synch_self == &fake_net);
    expect(g_synch_opcode == 0x16);
    expect(g_synch_a == 2001);
    expect(g_synch_b == 2002);
    expect(g_synch_c == 0);
    expect(g_synch_text == nullptr);
    expect(g_synch_d == 1);
    expect(g_synch_flags == 0x2101);

    NetDaemonSynch = saved_synch;
    NetDaemonNet = saved_net;
}

namespace {

// unlock_veh's highest store is at 0x1BCC, so the object image must reach
// 0x1BD0 - far past sizeof(NetDaemon), because the class tail is deliberately
// un-modelled and the recovery writes it through raw offsets.
const size_t NetDaemonUnlockVehSpan = 0x1BD0;

// The four announce-side clears, in the original's own store order
// (0x1BB0, 0x1BAC, 0x1BCC, 0x1BC8 - deliberately not monotonic), and the two
// clears the shared tail performs on every path.
const size_t NetDaemonAnnounceClears[4] = {0x1BB0, 0x1BAC, 0x1BCC, 0x1BC8};

const size_t NetDaemonTailClears[2] = {0x1B78, 0x1BC4};

struct NetDaemonMessageCall {
    int a1, a2, a3, a4, a5, a6;
    int calls;
    // env_open_calls sampled on entry. log_say reaches env_open exactly once,
    // so a value of 1 here proves log_say ran BEFORE message_data - the order
    // the two call opcodes appear in at 0x0053110C and 0x0053111B.
    int env_open_calls_at_entry;
} g_nd_message = {};

uint32_t g_nd_message_residue = 0;

uint32_t __cdecl observe_net_daemon_message_data(int a1, int a2, int a3,
                                                 int a4, int a5, int a6) {
    g_nd_message = {a1, a2, a3, a4, a5, a6, g_nd_message.calls + 1,
                    env_open_calls};
    return g_nd_message_residue;
}

}  // namespace

void test_net_daemon_unlock_veh() {
    // unlock_veh is __thiscall/arity 0 (bare ret at 0x00531149) entered on an
    // unadjusted this (mov esi, ecx at 0x005310FB, no vbtable load), so unlike
    // the RadioButton/ListBox family there is no second vbtable shape to
    // install here. The two genuinely different input shapes are the two arms
    // of the only branch - the transport flag at 0x0093F660 - crossed with
    // different faction ids, different seam residues, and different poison.
    int *const saved_net_flag = NetDaemonIsMultiplayerNet;
    int *const saved_faction = NetDaemonLocalFaction;
    func_net_message_data *const saved_message = NetDaemonMessageData;
    Log *const saved_logging = Logging;
    BOOL *const saved_logging_disabled = IsLoggingDisabled;
    const bool saved_capture = capture_closed_file;
    const int saved_env_open_calls = env_open_calls;

    // log_say at 0x006262F0 is source_complete, so the recovery calls it
    // directly rather than through a seam and the test observes the real
    // thing: a Log with a non-null file name and a clear disabled flag makes
    // Log::say reach the harness's env_open, and __wrap_fclose copies the
    // written bytes into closed_file_output. That is the same mechanism
    // test_log_lifecycle uses, and it is what pins the "Client releasing
    // lock" literal, the *LocalFaction argument, and the argument order.
    alignas(uint32_t) uint8_t log_storage[sizeof(Log) + 32];
    for (size_t index = 0; index < sizeof(log_storage); ++index) {
        log_storage[index] = static_cast<uint8_t>(0xC7U + index * 13U);
    }
    char log_name[] = "unlock_veh.log";
    LPSTR log_name_pointer = log_name;
    write_at(log_storage, 16 + 0x00, log_name_pointer);  // Log::log_file_
    const BOOL not_disabled = FALSE;
    write_at(log_storage, 16 + 0x04, not_disabled);      // Log::is_disabled_
    uint8_t log_expected[sizeof(log_storage)];
    std::memcpy(log_expected, log_storage, sizeof(log_storage));

    static BOOL logging_never_disabled = FALSE;
    Logging = reinterpret_cast<Log *>(log_storage + 16);
    IsLoggingDisabled = &logging_never_disabled;
    capture_closed_file = true;

    int net_flag = 0;
    int faction = 0;
    NetDaemonIsMultiplayerNet = &net_flag;
    NetDaemonLocalFaction = &faction;
    NetDaemonMessageData = &observe_net_daemon_message_data;

    struct Shape {
        int net_flag;              // the sole guard: cmp eax, edi against 0
        int faction;               // log_say's num1
        uint32_t residue;          // what message_data leaves in EAX
        const char *expected_log;  // nullptr => log_say must not run at all
        uint32_t poison;           // distinct pre-call sentinel per shape
    };
    const Shape shapes[] = {
        // Not a net game. The whole announce block is skipped and the residue
        // is the loaded zero flag - NOT the seam's residue, which is a loud
        // nonzero value here so a wrong model cannot pass.
        {0, 0x11223344, 0xFEEDFACEU, nullptr, 0x7A5100C1U},
        // Ordinary net game.
        {1, 7, 0xA5A5A5A5U, "Client releasing lock 7 0 0\n", 0x3C0DE001U},
        // ANY nonzero flag announces (the original compares against zero, not
        // one), and a negative faction pins the signedness of the second
        // log_say argument. A zero residue proves the residue is read from
        // the seam rather than assumed nonzero.
        {-1, -3, 0x00000000U, "Client releasing lock -3 0 0\n", 0x11BADD11U},
        // INT_MIN flag is still nonzero; faction 0 is a distinct text shape.
        {-0x7FFFFFFF - 1, 0, 0xDEADBEEFU, "Client releasing lock 0 0 0\n",
         0x600DF00DU},
    };

    for (size_t s = 0; s < ARRAYSIZE(shapes); ++s) {
        const Shape &shape = shapes[s];
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            std::vector<uint8_t> storage(NetDaemonUnlockVehSpan + 32);
            std::vector<uint8_t> reference(storage.size());
            seed_storage(storage.data(), reference.data(), storage.size());

            uint8_t *const object = storage.data() + 16;
            uint8_t *const ref_object = reference.data() + 16;

            // Poison every dword the function is expected to clear with a
            // distinct nonzero sentinel first. A field that was already zero
            // cannot demonstrate that it was zeroed.
            for (size_t i = 0; i < ARRAYSIZE(NetDaemonAnnounceClears); ++i) {
                const uint32_t value =
                    shape.poison + static_cast<uint32_t>(i) * 0x1111U;
                write_at(object, NetDaemonAnnounceClears[i], value);
                write_at(ref_object, NetDaemonAnnounceClears[i], value);
            }
            for (size_t i = 0; i < ARRAYSIZE(NetDaemonTailClears); ++i) {
                const uint32_t value =
                    shape.poison + 0x4444U + static_cast<uint32_t>(i) * 0x1111U;
                write_at(object, NetDaemonTailClears[i], value);
                write_at(ref_object, NetDaemonTailClears[i], value);
            }

            // Byte-exact hand-built reference: exactly six dwords go to zero
            // on the net path, exactly two on the other, and nothing else in
            // the buffer moves. 16 leading and 16 trailing canary bytes.
            const uint32_t zero = 0;
            if (shape.net_flag != 0) {
                for (size_t i = 0; i < ARRAYSIZE(NetDaemonAnnounceClears);
                     ++i) {
                    write_at(ref_object, NetDaemonAnnounceClears[i], zero);
                }
            }
            for (size_t i = 0; i < ARRAYSIZE(NetDaemonTailClears); ++i) {
                write_at(ref_object, NetDaemonTailClears[i], zero);
            }

            net_flag = shape.net_flag;
            faction = shape.faction;
            g_nd_message = NetDaemonMessageCall();
            g_nd_message_residue = shape.residue;
            env_open_calls = 0;
            env_open_source = nullptr;
            env_open_mode = nullptr;
#if defined(__MINGW32__)
            env_close_calls = 0;
#endif
            closed_file_output[0] = 0;

            auto *const daemon = reinterpret_cast<NetDaemon *>(object);
            const uint32_t result =
                use_adapter ? net_daemon_unlock_veh_redirect(daemon, nullptr)
                            : daemon->unlock_veh();

            expect_storage_bytes(storage.data(), reference.data(),
                                 storage.size());
            // log_say must not disturb the Log object it logs through.
            expect_storage_bytes(log_storage, log_expected,
                                 sizeof(log_storage));

            if (shape.net_flag == 0) {
                // EAX residue on this path is the loaded transport flag,
                // which is zero exactly because the branch was taken - not
                // the seam's residue, which is 0xFEEDFACE for this shape.
                expect(result == 0U);
                expect(g_nd_message.calls == 0);
                expect(env_open_calls == 0);
#if defined(__MINGW32__)
                expect(env_close_calls == 0);
                expect(closed_file_output[0] == 0);
#endif
            } else {
                // EAX residue on the announce path is message_data's EAX.
                expect(result == shape.residue);

                // message_data(0x2212, 0, 0, 0, 0, 0) - one call, exact args.
                expect(g_nd_message.calls == 1);
                expect(g_nd_message.a1 == 0x2212);
                expect(g_nd_message.a2 == 0);
                expect(g_nd_message.a3 == 0);
                expect(g_nd_message.a4 == 0);
                expect(g_nd_message.a5 == 0);
                expect(g_nd_message.a6 == 0);
                // Call order: log_say first (0x0053110C), message_data second
                // (0x0053111B).
                expect(g_nd_message.env_open_calls_at_entry == 1);

                // Call target: the real source-owned log_say, reached through
                // the rebound Logging object and no seam.
                expect(env_open_calls == 1);
                expect(env_open_source == log_name_pointer);
                expect(env_open_mode &&
                       std::strcmp(env_open_mode, "at") == 0);
#if defined(__MINGW32__)
                expect(env_close_calls == 1);
                // Exact text: pins the 0x0068C2DC literal, *LocalFaction as
                // num1, and the two trailing zeros as num2/num3.
                expect(std::strcmp(closed_file_output,
                                   shape.expected_log) == 0);
#endif
            }
        }
    }

    env_open_calls = saved_env_open_calls;
    capture_closed_file = saved_capture;
    IsLoggingDisabled = saved_logging_disabled;
    Logging = saved_logging;
    NetDaemonMessageData = saved_message;
    NetDaemonLocalFaction = saved_faction;
    NetDaemonIsMultiplayerNet = saved_net_flag;
}

namespace {

// Six-argument family (target: XPopOriginalFull). Field order mirrors
// func_x_pop_full: caption, label, value, text, flags, callback.
struct XPopCall {
    char *caption; const char *label; int value; char *text; int flags;
    int (__cdecl *callback)();
    int calls;
} g_xpop6 = {};

int __cdecl observe_xpop6(char *caption, const char *label, int value,
                          char *text, int flags, int (__cdecl *callback)()) {
    g_xpop6 = {caption, label, value, text, flags, callback, g_xpop6.calls + 1};
    return 0x5AA7;
}

int __cdecl xpop6_callback() { return 0; }

// Nine-argument family (target: XPopsOriginalFull). Field order mirrors
// func_x_pops_full: caption, label, title, text, value, sprite, flag1,
// flag2, callback.
struct XPopsCall9 {
    char *caption; const char *label; int title; char *text; int value;
    Sprite *sprite; int flag1; int flag2;
    int (__cdecl *callback)();
    int calls;
} g_xpops9 = {};

int __cdecl observe_xpops9(char *caption, const char *label, int title,
                           char *text, int value, Sprite *sprite, int flag1,
                           int flag2, int (__cdecl *callback)()) {
    g_xpops9 = {caption, label, title, text, value, sprite, flag1, flag2,
               callback, g_xpops9.calls + 1};
    return 0x5AA7;
}

int __cdecl xpops9_callback() { return 0; }

}  // namespace

void test_x_pop_forwarders() {
    // XPopOriginalFull(caption, label, value, text, flags, callback) is the
    // six-argument sibling of the nine-argument popup builder (no sprite, no
    // trailing flag pair). Each of the six forwarders below defaults a
    // different subset of these six arguments; every literal and every
    // passthrough position is asserted, with distinct sentinels per
    // parameter, so a swapped argument cannot pass unnoticed.
    auto *const saved_full = XPopOriginalFull;
    char *const saved_buffer = XPopsCaptionBuffer;
    char buffer[8] = {};
    XPopOriginalFull = &observe_xpop6;
    XPopsCaptionBuffer = buffer;

    char caption[] = "caption";
    const char label[] = "label";
    char text[] = "text";
    const int kValue = 0x2002;
    const int kFlags = 0x3003;

    // x_pop_caption_label: caller's own caption/label/callback; value -1,
    // text nullptr and flags 0 all default.
    g_xpop6.calls = 0;
    expect(x_pop_caption_label(caption, label, &xpop6_callback) == 0x5AA7);
    expect(g_xpop6.calls == 1);
    expect(g_xpop6.caption == caption);
    expect(g_xpop6.label == label);
    expect(g_xpop6.value == -1);
    expect(g_xpop6.text == nullptr);
    expect(g_xpop6.flags == 0);
    expect(g_xpop6.callback == &xpop6_callback);

    // x_pop_value_flags: shared (rebound) buffer, caller's value and flags,
    // no override text.
    g_xpop6.calls = 0;
    expect(x_pop_value_flags(label, kValue, kFlags, &xpop6_callback) == 0x5AA7);
    expect(g_xpop6.calls == 1);
    expect(g_xpop6.caption == buffer);  // proves the rebound seam is read
    expect(g_xpop6.label == label);
    expect(g_xpop6.value == kValue);
    expect(g_xpop6.text == nullptr);
    expect(g_xpop6.flags == kFlags);
    expect(g_xpop6.callback == &xpop6_callback);

    // x_pop_caption_value_flags: caller's caption/label/value/flags, no
    // override text.
    g_xpop6.calls = 0;
    expect(x_pop_caption_value_flags(caption, label, kValue, kFlags,
                                     &xpop6_callback) == 0x5AA7);
    expect(g_xpop6.calls == 1);
    expect(g_xpop6.caption == caption);
    expect(g_xpop6.label == label);
    expect(g_xpop6.value == kValue);
    expect(g_xpop6.text == nullptr);
    expect(g_xpop6.flags == kFlags);
    expect(g_xpop6.callback == &xpop6_callback);

    // x_pop_short: shared (rebound) buffer, value -1, no override text,
    // caller's flags.
    g_xpop6.calls = 0;
    expect(x_pop_short(label, kFlags, &xpop6_callback) == 0x5AA7);
    expect(g_xpop6.calls == 1);
    expect(g_xpop6.caption == buffer);
    expect(g_xpop6.label == label);
    expect(g_xpop6.value == -1);
    expect(g_xpop6.text == nullptr);
    expect(g_xpop6.flags == kFlags);
    expect(g_xpop6.callback == &xpop6_callback);

    // x_pop_caption_flags: caller's caption/label/flags; value -1, no
    // override text.
    g_xpop6.calls = 0;
    expect(x_pop_caption_flags(caption, label, kFlags, &xpop6_callback) == 0x5AA7);
    expect(g_xpop6.calls == 1);
    expect(g_xpop6.caption == caption);
    expect(g_xpop6.label == label);
    expect(g_xpop6.value == -1);
    expect(g_xpop6.text == nullptr);
    expect(g_xpop6.flags == kFlags);
    expect(g_xpop6.callback == &xpop6_callback);

    // x_pop_default_caption: shared (rebound) buffer; caller's label, value,
    // override text and flags.
    g_xpop6.calls = 0;
    expect(x_pop_default_caption(label, kValue, text, kFlags,
                                 &xpop6_callback) == 0x5AA7);
    expect(g_xpop6.calls == 1);
    expect(g_xpop6.caption == buffer);
    expect(g_xpop6.label == label);
    expect(g_xpop6.value == kValue);
    expect(g_xpop6.text == text);
    expect(g_xpop6.flags == kFlags);
    expect(g_xpop6.callback == &xpop6_callback);

    XPopOriginalFull = saved_full;
    XPopsCaptionBuffer = saved_buffer;
}

void test_x_pops_forwarders() {
    // XPopsOriginalFull(caption, label, title, text, value, sprite, flag1,
    // flag2, callback) is the nine-argument popup builder already exercised
    // by x_pops_short/x_pops_minimal. Each of the thirteen forwarders below
    // defaults a different subset of these nine arguments; every literal and
    // every passthrough position is asserted, with distinct sentinels per
    // parameter, so a swapped argument cannot pass unnoticed.
    auto *const saved_full = XPopsOriginalFull;
    char *const saved_buffer = XPopsCaptionBuffer;
    char buffer[8] = {};
    XPopsOriginalFull = &observe_xpops9;
    XPopsCaptionBuffer = buffer;

    char caption[] = "caption";
    const char label[] = "label";
    char text[] = "text";
    Sprite sprite_value;

    const int kTitle = 0x1001;
    const int kValue = 0x2002;
    const int kFlag1 = 0x4004;
    const int kFlag2 = 0x5005;

    // x_pops_flags: shared (rebound) buffer, title -1, no override text,
    // value 0; caller's sprite and both flags (unlike short/minimal, the
    // flags are not fixed at 1 here).
    g_xpops9.calls = 0;
    expect(x_pops_flags(label, &sprite_value, kFlag1, kFlag2,
                        &xpops9_callback) == 0x5AA7);
    expect(g_xpops9.calls == 1);
    expect(g_xpops9.caption == buffer);  // proves the rebound seam is read
    expect(g_xpops9.label == label);
    expect(g_xpops9.title == -1);
    expect(g_xpops9.text == nullptr);
    expect(g_xpops9.value == 0);
    expect(g_xpops9.sprite == &sprite_value);
    expect(g_xpops9.flag1 == kFlag1);
    expect(g_xpops9.flag2 == kFlag2);
    expect(g_xpops9.callback == &xpops9_callback);

    // x_pops_caption_minimal: caller's caption/label/sprite/callback; title
    // -1, no override text, value 0, both flags fixed at 1.
    g_xpops9.calls = 0;
    expect(x_pops_caption_minimal(caption, label, &sprite_value,
                                  &xpops9_callback) == 0x5AA7);
    expect(g_xpops9.calls == 1);
    expect(g_xpops9.caption == caption);
    expect(g_xpops9.label == label);
    expect(g_xpops9.title == -1);
    expect(g_xpops9.text == nullptr);
    expect(g_xpops9.value == 0);
    expect(g_xpops9.sprite == &sprite_value);
    expect(g_xpops9.flag1 == 1);
    expect(g_xpops9.flag2 == 1);
    expect(g_xpops9.callback == &xpops9_callback);

    // x_pops_caption_flags: caller's caption/label/sprite/both flags; title
    // -1, no override text, value 0.
    g_xpops9.calls = 0;
    expect(x_pops_caption_flags(caption, label, &sprite_value, kFlag1, kFlag2,
                                &xpops9_callback) == 0x5AA7);
    expect(g_xpops9.calls == 1);
    expect(g_xpops9.caption == caption);
    expect(g_xpops9.label == label);
    expect(g_xpops9.title == -1);
    expect(g_xpops9.text == nullptr);
    expect(g_xpops9.value == 0);
    expect(g_xpops9.sprite == &sprite_value);
    expect(g_xpops9.flag1 == kFlag1);
    expect(g_xpops9.flag2 == kFlag2);
    expect(g_xpops9.callback == &xpops9_callback);

    // x_pops_title: shared (rebound) buffer, caller's title/value/sprite; no
    // override text, both flags fixed at 1.
    g_xpops9.calls = 0;
    expect(x_pops_title(label, kTitle, kValue, &sprite_value,
                        &xpops9_callback) == 0x5AA7);
    expect(g_xpops9.calls == 1);
    expect(g_xpops9.caption == buffer);
    expect(g_xpops9.label == label);
    expect(g_xpops9.title == kTitle);
    expect(g_xpops9.text == nullptr);
    expect(g_xpops9.value == kValue);
    expect(g_xpops9.sprite == &sprite_value);
    expect(g_xpops9.flag1 == 1);
    expect(g_xpops9.flag2 == 1);
    expect(g_xpops9.callback == &xpops9_callback);

    // x_pops_title_flags: shared (rebound) buffer, caller's title/value/
    // sprite/both flags; no override text.
    g_xpops9.calls = 0;
    expect(x_pops_title_flags(label, kTitle, kValue, &sprite_value, kFlag1,
                              kFlag2, &xpops9_callback) == 0x5AA7);
    expect(g_xpops9.calls == 1);
    expect(g_xpops9.caption == buffer);
    expect(g_xpops9.label == label);
    expect(g_xpops9.title == kTitle);
    expect(g_xpops9.text == nullptr);
    expect(g_xpops9.value == kValue);
    expect(g_xpops9.sprite == &sprite_value);
    expect(g_xpops9.flag1 == kFlag1);
    expect(g_xpops9.flag2 == kFlag2);
    expect(g_xpops9.callback == &xpops9_callback);

    // x_pops_caption_title: caller's caption/label/title/value/sprite; no
    // override text, both flags fixed at 1.
    g_xpops9.calls = 0;
    expect(x_pops_caption_title(caption, label, kTitle, kValue, &sprite_value,
                                &xpops9_callback) == 0x5AA7);
    expect(g_xpops9.calls == 1);
    expect(g_xpops9.caption == caption);
    expect(g_xpops9.label == label);
    expect(g_xpops9.title == kTitle);
    expect(g_xpops9.text == nullptr);
    expect(g_xpops9.value == kValue);
    expect(g_xpops9.sprite == &sprite_value);
    expect(g_xpops9.flag1 == 1);
    expect(g_xpops9.flag2 == 1);
    expect(g_xpops9.callback == &xpops9_callback);

    // x_pops_no_text: everything but the override text caller-supplied -
    // caption, label, title, value, sprite and both flags.
    g_xpops9.calls = 0;
    expect(x_pops_no_text(caption, label, kTitle, kValue, &sprite_value,
                          kFlag1, kFlag2, &xpops9_callback) == 0x5AA7);
    expect(g_xpops9.calls == 1);
    expect(g_xpops9.caption == caption);
    expect(g_xpops9.label == label);
    expect(g_xpops9.title == kTitle);
    expect(g_xpops9.text == nullptr);
    expect(g_xpops9.value == kValue);
    expect(g_xpops9.sprite == &sprite_value);
    expect(g_xpops9.flag1 == kFlag1);
    expect(g_xpops9.flag2 == kFlag2);
    expect(g_xpops9.callback == &xpops9_callback);

    // x_pops_value_flags: shared (rebound) buffer, title -1, no override
    // text; caller's value/sprite/both flags.
    g_xpops9.calls = 0;
    expect(x_pops_value_flags(label, kValue, &sprite_value, kFlag1, kFlag2,
                              &xpops9_callback) == 0x5AA7);
    expect(g_xpops9.calls == 1);
    expect(g_xpops9.caption == buffer);
    expect(g_xpops9.label == label);
    expect(g_xpops9.title == -1);
    expect(g_xpops9.text == nullptr);
    expect(g_xpops9.value == kValue);
    expect(g_xpops9.sprite == &sprite_value);
    expect(g_xpops9.flag1 == kFlag1);
    expect(g_xpops9.flag2 == kFlag2);
    expect(g_xpops9.callback == &xpops9_callback);

    // x_pops_caption_value: caller's caption/label/value/sprite; title -1,
    // no override text, both flags fixed at 1.
    g_xpops9.calls = 0;
    expect(x_pops_caption_value(caption, label, kValue, &sprite_value,
                                &xpops9_callback) == 0x5AA7);
    expect(g_xpops9.calls == 1);
    expect(g_xpops9.caption == caption);
    expect(g_xpops9.label == label);
    expect(g_xpops9.title == -1);
    expect(g_xpops9.text == nullptr);
    expect(g_xpops9.value == kValue);
    expect(g_xpops9.sprite == &sprite_value);
    expect(g_xpops9.flag1 == 1);
    expect(g_xpops9.flag2 == 1);
    expect(g_xpops9.callback == &xpops9_callback);

    // x_pops_caption_value_flags: caller's caption/label/value/sprite/both
    // flags; title -1, no override text.
    g_xpops9.calls = 0;
    expect(x_pops_caption_value_flags(caption, label, kValue, &sprite_value,
                                      kFlag1, kFlag2,
                                      &xpops9_callback) == 0x5AA7);
    expect(g_xpops9.calls == 1);
    expect(g_xpops9.caption == caption);
    expect(g_xpops9.label == label);
    expect(g_xpops9.title == -1);
    expect(g_xpops9.text == nullptr);
    expect(g_xpops9.value == kValue);
    expect(g_xpops9.sprite == &sprite_value);
    expect(g_xpops9.flag1 == kFlag1);
    expect(g_xpops9.flag2 == kFlag2);
    expect(g_xpops9.callback == &xpops9_callback);

    // x_pops_override_text: shared (rebound) buffer, caller's title/override
    // text/value/sprite; both flags fixed at 1.
    g_xpops9.calls = 0;
    expect(x_pops_override_text(label, kTitle, text, kValue, &sprite_value,
                                &xpops9_callback) == 0x5AA7);
    expect(g_xpops9.calls == 1);
    expect(g_xpops9.caption == buffer);
    expect(g_xpops9.label == label);
    expect(g_xpops9.title == kTitle);
    expect(g_xpops9.text == text);
    expect(g_xpops9.value == kValue);
    expect(g_xpops9.sprite == &sprite_value);
    expect(g_xpops9.flag1 == 1);
    expect(g_xpops9.flag2 == 1);
    expect(g_xpops9.callback == &xpops9_callback);

    // x_pops_default_caption: shared (rebound) buffer; everything else -
    // label, title, override text, value, sprite and both flags -
    // caller-supplied.
    g_xpops9.calls = 0;
    expect(x_pops_default_caption(label, kTitle, text, kValue, &sprite_value,
                                  kFlag1, kFlag2,
                                  &xpops9_callback) == 0x5AA7);
    expect(g_xpops9.calls == 1);
    expect(g_xpops9.caption == buffer);
    expect(g_xpops9.label == label);
    expect(g_xpops9.title == kTitle);
    expect(g_xpops9.text == text);
    expect(g_xpops9.value == kValue);
    expect(g_xpops9.sprite == &sprite_value);
    expect(g_xpops9.flag1 == kFlag1);
    expect(g_xpops9.flag2 == kFlag2);
    expect(g_xpops9.callback == &xpops9_callback);

    // x_pops_no_flags: everything but the flags caller-supplied; both flags
    // fixed at 1.
    g_xpops9.calls = 0;
    expect(x_pops_no_flags(caption, label, kTitle, text, kValue,
                           &sprite_value, &xpops9_callback) == 0x5AA7);
    expect(g_xpops9.calls == 1);
    expect(g_xpops9.caption == caption);
    expect(g_xpops9.label == label);
    expect(g_xpops9.title == kTitle);
    expect(g_xpops9.text == text);
    expect(g_xpops9.value == kValue);
    expect(g_xpops9.sprite == &sprite_value);
    expect(g_xpops9.flag1 == 1);
    expect(g_xpops9.flag2 == 1);
    expect(g_xpops9.callback == &xpops9_callback);

    XPopsOriginalFull = saved_full;
    XPopsCaptionBuffer = saved_buffer;
}

namespace {

void *g_caviar_freed;

int g_caviar_free_calls;

void __cdecl observe_caviar_free_record(void *record) {
    g_caviar_freed = record;
    ++g_caviar_free_calls;
}

}  // namespace

void test_caviar_data_close() {
    // close releases the record at 0x8 through the helper and forgets it; a
    // slot with no record is left entirely untouched, and the two leading
    // fields keep whatever they held either way.
    auto *const saved = CaviarDataFreeRecord;
    CaviarDataFreeRecord = &observe_caviar_free_record;

    alignas(4) uint8_t storage[sizeof(CaviarData) + 16];
    uint8_t expected[sizeof(storage)];
    auto *slot = reinterpret_cast<CaviarData *>(storage);
    int record = 0;

    // No record: nothing is called and not one byte moves.
    seed_storage(storage, expected, sizeof(storage));
    std::memset(storage + 8, 0, 4);
    std::memcpy(expected, storage, sizeof(storage));
    g_caviar_free_calls = 0;
    slot->close();
    expect(g_caviar_free_calls == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // A record: the helper sees exactly that pointer, the slot forgets it,
    // and everything else survives byte for byte.
    seed_storage(storage, expected, sizeof(storage));
    void *pointer = &record;
    std::memcpy(storage + 8, &pointer, 4);
    std::memcpy(expected, storage, sizeof(storage));
    std::memset(expected + 8, 0, 4);
    g_caviar_free_calls = 0;
    g_caviar_freed = nullptr;
    slot->close();
    expect(g_caviar_free_calls == 1);
    expect(g_caviar_freed == &record);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // Redirect entry.
    seed_storage(storage, expected, sizeof(storage));
    std::memcpy(storage + 8, &pointer, 4);
    g_caviar_free_calls = 0;
    caviar_data_close_redirect(slot, nullptr);
    expect(g_caviar_free_calls == 1);

    CaviarDataFreeRecord = saved;
}

void test_texture_dtor() {
    // The destructor settles only the pixel block: freed and cleared when
    // there are pixels and they are ours, left alone otherwise. Unlike close
    // it resets nothing else - the descriptors and the borrowed flag keep
    // their values on every path.
    auto *const saved = TextureFree;
    TextureFree = &observe_texture_free;

    alignas(4) uint8_t storage[sizeof(Texture) + 16];
    uint8_t expected[sizeof(storage)];
    auto *texture = reinterpret_cast<Texture *>(storage);
    int pixels = 0;
    void *pointer = &pixels;

    // No pixels: nothing freed, nothing written.
    seed_storage(storage, expected, sizeof(storage));
    std::memset(storage + 0x00, 0, 4);
    std::memcpy(expected, storage, sizeof(storage));
    g_tex_free_calls = 0;
    texture->~Texture();
    expect(g_tex_free_calls == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // Borrowed pixels: kept, and still nothing written.
    seed_storage(storage, expected, sizeof(storage));
    std::memcpy(storage + 0x00, &pointer, 4);
    uint32_t borrowed = 1;
    std::memcpy(storage + 0x6C, &borrowed, 4);
    std::memcpy(expected, storage, sizeof(storage));
    g_tex_free_calls = 0;
    texture->~Texture();
    expect(g_tex_free_calls == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // Our pixels: freed and cleared; descriptors and flag untouched.
    seed_storage(storage, expected, sizeof(storage));
    std::memcpy(storage + 0x00, &pointer, 4);
    std::memset(storage + 0x6C, 0, 4);
    std::memcpy(expected, storage, sizeof(storage));
    std::memset(expected + 0x00, 0, 4);
    g_tex_free_calls = 0;
    g_tex_freed = nullptr;
    texture->~Texture();
    expect(g_tex_free_calls == 1);
    expect(g_tex_freed == &pixels);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // Redirect entry.
    seed_storage(storage, expected, sizeof(storage));
    std::memcpy(storage + 0x00, &pointer, 4);
    std::memset(storage + 0x6C, 0, 4);
    g_tex_free_calls = 0;
    texture_dtor_redirect(texture, nullptr);
    expect(g_tex_free_calls == 1);

    TextureFree = saved;
}

namespace {

// Target: PopsOriginalFull. Field order mirrors func_pops_full: caption,
// label, value, text, title, sprite, flag_a, flag_b, callback. Note both
// caption and label are plain char* here (PAD in every mangled name), unlike
// the const char* label used by the XPopsOriginalFull family.
struct PopsCall {
    char *caption; char *label; int value; char *text; int title;
    Sprite *sprite; int flag_a; int flag_b;
    int (__cdecl *callback)();
    int calls;
} g_pops = {};

int __cdecl observe_pops(char *caption, char *label, int value, char *text,
                         int title, Sprite *sprite, int flag_a, int flag_b,
                         int (__cdecl *callback)()) {
    g_pops = {caption, label, value, text, title, sprite, flag_a, flag_b,
             callback, g_pops.calls + 1};
    return 0x7A11;
}

int __cdecl pops_test_callback() { return 0; }

}  // namespace

void test_pop_pops_forwarders() {
    // PopsOriginalFull(caption, label, value, text, title, sprite, flag_a,
    // flag_b, callback) is the nine-argument popup builder at 0x006276A0
    // that every pop_*/pops_* forwarder below reduces to. The pop_* family
    // (8 functions) always passes a null sprite; the pops_* family (15
    // functions) always forwards its own sprite argument. Every literal
    // (-1, nullptr, 0, 1) and every passthrough position is asserted, with
    // distinct sentinels per parameter, so a swapped argument cannot pass
    // unnoticed.
    auto *const saved_full = PopsOriginalFull;
    char *const saved_caption = PopupStartCaption;
    char buffer[8] = {};
    PopsOriginalFull = &observe_pops;
    PopupStartCaption = buffer;

    char caption[] = "caption";
    char label[] = "label";
    char text[] = "text";
    Sprite sprite_value;

    const int kValue = 0x2002;
    const int kTitle = 0x1001;
    const int kFlagA = 0x4004;
    const int kFlagB = 0x5005;

    // pop_label_cb (0x00627130): shared (rebound) buffer, label/callback
    // only; value -1, no override text, title 0, null sprite, both flags
    // fixed at 1.
    g_pops.calls = 0;
    expect(pop_label_cb(label, &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == buffer);  // proves the rebound seam is read
    expect(g_pops.label == label);
    expect(g_pops.value == -1);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == 0);
    expect(g_pops.sprite == nullptr);
    expect(g_pops.flag_a == 1);
    expect(g_pops.flag_b == 1);
    expect(g_pops.callback == &pops_test_callback);

    // pop_caption (0x00627160): caller's own caption/label/callback; value
    // -1, no override text, title 0, null sprite, both flags fixed at 1.
    g_pops.calls = 0;
    expect(pop_caption(caption, label, &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == caption);
    expect(g_pops.label == label);
    expect(g_pops.value == -1);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == 0);
    expect(g_pops.sprite == nullptr);
    expect(g_pops.flag_a == 1);
    expect(g_pops.flag_b == 1);
    expect(g_pops.callback == &pops_test_callback);

    // pop_value_title (0x006271D0): shared (rebound) buffer, caller's
    // value/title; no override text, null sprite, both flags fixed at 1.
    g_pops.calls = 0;
    expect(pop_value_title(label, kValue, kTitle,
                           &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == buffer);
    expect(g_pops.label == label);
    expect(g_pops.value == kValue);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == kTitle);
    expect(g_pops.sprite == nullptr);
    expect(g_pops.flag_a == 1);
    expect(g_pops.flag_b == 1);
    expect(g_pops.callback == &pops_test_callback);

    // pop_caption_value_title (0x00627200): caller's caption/label/value/
    // title; no override text, null sprite, both flags fixed at 1.
    g_pops.calls = 0;
    expect(pop_caption_value_title(caption, label, kValue, kTitle,
                                   &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == caption);
    expect(g_pops.label == label);
    expect(g_pops.value == kValue);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == kTitle);
    expect(g_pops.sprite == nullptr);
    expect(g_pops.flag_a == 1);
    expect(g_pops.flag_b == 1);
    expect(g_pops.callback == &pops_test_callback);

    // pop_title (0x00627230): shared (rebound) buffer, value -1, caller's
    // title only; no override text, null sprite, both flags fixed at 1.
    g_pops.calls = 0;
    expect(pop_title(label, kTitle, &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == buffer);
    expect(g_pops.label == label);
    expect(g_pops.value == -1);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == kTitle);
    expect(g_pops.sprite == nullptr);
    expect(g_pops.flag_a == 1);
    expect(g_pops.flag_b == 1);
    expect(g_pops.callback == &pops_test_callback);

    // pop_caption_title (0x00627260): caller's caption/label/title; value
    // -1, no override text, null sprite, both flags fixed at 1.
    g_pops.calls = 0;
    expect(pop_caption_title(caption, label, kTitle,
                             &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == caption);
    expect(g_pops.label == label);
    expect(g_pops.value == -1);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == kTitle);
    expect(g_pops.sprite == nullptr);
    expect(g_pops.flag_a == 1);
    expect(g_pops.flag_b == 1);
    expect(g_pops.callback == &pops_test_callback);

    // pop_value_text_title (0x00627290): shared (rebound) buffer, caller's
    // value/override text/title; null sprite, both flags fixed at 1.
    g_pops.calls = 0;
    expect(pop_value_text_title(label, kValue, text, kTitle,
                                &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == buffer);
    expect(g_pops.label == label);
    expect(g_pops.value == kValue);
    expect(g_pops.text == text);
    expect(g_pops.title == kTitle);
    expect(g_pops.sprite == nullptr);
    expect(g_pops.flag_a == 1);
    expect(g_pops.flag_b == 1);
    expect(g_pops.callback == &pops_test_callback);

    // pop_full (0x006272C0): everything but the sprite and flags caller-
    // supplied - caption, value, override text, title; null sprite, both
    // flags fixed at 1.
    g_pops.calls = 0;
    expect(pop_full(caption, label, kValue, text, kTitle,
                    &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == caption);
    expect(g_pops.label == label);
    expect(g_pops.value == kValue);
    expect(g_pops.text == text);
    expect(g_pops.title == kTitle);
    expect(g_pops.sprite == nullptr);
    expect(g_pops.flag_a == 1);
    expect(g_pops.flag_b == 1);
    expect(g_pops.callback == &pops_test_callback);

    // pops_minimal (0x00627310): shared (rebound) buffer, caller's sprite
    // only; value -1, no override text, title 0, both flags fixed at 1.
    g_pops.calls = 0;
    expect(pops_minimal(label, &sprite_value,
                        &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == buffer);
    expect(g_pops.label == label);
    expect(g_pops.value == -1);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == 0);
    expect(g_pops.sprite == &sprite_value);
    expect(g_pops.flag_a == 1);
    expect(g_pops.flag_b == 1);
    expect(g_pops.callback == &pops_test_callback);

    // pops_flags (0x00627340): shared (rebound) buffer, caller's sprite and
    // both flags; value -1, no override text, title 0.
    g_pops.calls = 0;
    expect(pops_flags(label, &sprite_value, kFlagA, kFlagB,
                      &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == buffer);
    expect(g_pops.label == label);
    expect(g_pops.value == -1);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == 0);
    expect(g_pops.sprite == &sprite_value);
    expect(g_pops.flag_a == kFlagA);
    expect(g_pops.flag_b == kFlagB);
    expect(g_pops.callback == &pops_test_callback);

    // pops_caption (0x00627370): caller's caption/label/sprite; value -1,
    // no override text, title 0, both flags fixed at 1.
    g_pops.calls = 0;
    expect(pops_caption(caption, label, &sprite_value,
                        &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == caption);
    expect(g_pops.label == label);
    expect(g_pops.value == -1);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == 0);
    expect(g_pops.sprite == &sprite_value);
    expect(g_pops.flag_a == 1);
    expect(g_pops.flag_b == 1);
    expect(g_pops.callback == &pops_test_callback);

    // pops_caption_flags (0x006273A0): caller's caption/label/sprite/both
    // flags; value -1, no override text, title 0.
    g_pops.calls = 0;
    expect(pops_caption_flags(caption, label, &sprite_value, kFlagA, kFlagB,
                              &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == caption);
    expect(g_pops.label == label);
    expect(g_pops.value == -1);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == 0);
    expect(g_pops.sprite == &sprite_value);
    expect(g_pops.flag_a == kFlagA);
    expect(g_pops.flag_b == kFlagB);
    expect(g_pops.callback == &pops_test_callback);

    // pops_value_title (0x006273D0): shared (rebound) buffer, caller's
    // value/title/sprite; no override text, both flags fixed at 1.
    g_pops.calls = 0;
    expect(pops_value_title(label, kValue, kTitle, &sprite_value,
                            &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == buffer);
    expect(g_pops.label == label);
    expect(g_pops.value == kValue);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == kTitle);
    expect(g_pops.sprite == &sprite_value);
    expect(g_pops.flag_a == 1);
    expect(g_pops.flag_b == 1);
    expect(g_pops.callback == &pops_test_callback);

    // pops_value_title_flags (0x00627400): shared (rebound) buffer,
    // caller's value/title/sprite/both flags; no override text.
    g_pops.calls = 0;
    expect(pops_value_title_flags(label, kValue, kTitle, &sprite_value,
                                  kFlagA, kFlagB,
                                  &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == buffer);
    expect(g_pops.label == label);
    expect(g_pops.value == kValue);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == kTitle);
    expect(g_pops.sprite == &sprite_value);
    expect(g_pops.flag_a == kFlagA);
    expect(g_pops.flag_b == kFlagB);
    expect(g_pops.callback == &pops_test_callback);

    // pops_caption_value_title (0x006274D0): caller's caption/label/value/
    // title/sprite; no override text, both flags fixed at 1.
    g_pops.calls = 0;
    expect(pops_caption_value_title(caption, label, kValue, kTitle,
                                    &sprite_value,
                                    &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == caption);
    expect(g_pops.label == label);
    expect(g_pops.value == kValue);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == kTitle);
    expect(g_pops.sprite == &sprite_value);
    expect(g_pops.flag_a == 1);
    expect(g_pops.flag_b == 1);
    expect(g_pops.callback == &pops_test_callback);

    // pops_no_text (0x00627500): everything but the override text caller-
    // supplied - caption, value, title, sprite and both flags.
    g_pops.calls = 0;
    expect(pops_no_text(caption, label, kValue, kTitle, &sprite_value,
                        kFlagA, kFlagB, &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == caption);
    expect(g_pops.label == label);
    expect(g_pops.value == kValue);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == kTitle);
    expect(g_pops.sprite == &sprite_value);
    expect(g_pops.flag_a == kFlagA);
    expect(g_pops.flag_b == kFlagB);
    expect(g_pops.callback == &pops_test_callback);

    // pops_title (0x00627540): shared (rebound) buffer, value -1, caller's
    // title/sprite; no override text, both flags fixed at 1.
    g_pops.calls = 0;
    expect(pops_title(label, kTitle, &sprite_value,
                      &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == buffer);
    expect(g_pops.label == label);
    expect(g_pops.value == -1);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == kTitle);
    expect(g_pops.sprite == &sprite_value);
    expect(g_pops.flag_a == 1);
    expect(g_pops.flag_b == 1);
    expect(g_pops.callback == &pops_test_callback);

    // pops_title_flags (0x00627570): shared (rebound) buffer, value -1,
    // caller's title/sprite/both flags; no override text.
    g_pops.calls = 0;
    expect(pops_title_flags(label, kTitle, &sprite_value, kFlagA, kFlagB,
                            &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == buffer);
    expect(g_pops.label == label);
    expect(g_pops.value == -1);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == kTitle);
    expect(g_pops.sprite == &sprite_value);
    expect(g_pops.flag_a == kFlagA);
    expect(g_pops.flag_b == kFlagB);
    expect(g_pops.callback == &pops_test_callback);

    // pops_caption_title (0x006275A0): caller's caption/label/title/
    // sprite; value -1, no override text, both flags fixed at 1.
    g_pops.calls = 0;
    expect(pops_caption_title(caption, label, kTitle, &sprite_value,
                              &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == caption);
    expect(g_pops.label == label);
    expect(g_pops.value == -1);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == kTitle);
    expect(g_pops.sprite == &sprite_value);
    expect(g_pops.flag_a == 1);
    expect(g_pops.flag_b == 1);
    expect(g_pops.callback == &pops_test_callback);

    // pops_caption_title_flags (0x006275D0): caller's caption/label/title/
    // sprite/both flags; value -1, no override text.
    g_pops.calls = 0;
    expect(pops_caption_title_flags(caption, label, kTitle, &sprite_value,
                                    kFlagA, kFlagB,
                                    &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == caption);
    expect(g_pops.label == label);
    expect(g_pops.value == -1);
    expect(g_pops.text == nullptr);
    expect(g_pops.title == kTitle);
    expect(g_pops.sprite == &sprite_value);
    expect(g_pops.flag_a == kFlagA);
    expect(g_pops.flag_b == kFlagB);
    expect(g_pops.callback == &pops_test_callback);

    // pops_value_text_title (0x00627600): shared (rebound) buffer, caller's
    // value/override text/title/sprite; both flags fixed at 1.
    g_pops.calls = 0;
    expect(pops_value_text_title(label, kValue, text, kTitle, &sprite_value,
                                 &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == buffer);
    expect(g_pops.label == label);
    expect(g_pops.value == kValue);
    expect(g_pops.text == text);
    expect(g_pops.title == kTitle);
    expect(g_pops.sprite == &sprite_value);
    expect(g_pops.flag_a == 1);
    expect(g_pops.flag_b == 1);
    expect(g_pops.callback == &pops_test_callback);

    // pops_default_caption (0x00627630): shared (rebound) buffer; every
    // other argument - value, override text, title, sprite, both flags -
    // caller-supplied.
    g_pops.calls = 0;
    expect(pops_default_caption(label, kValue, text, kTitle, &sprite_value,
                                kFlagA, kFlagB,
                                &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == buffer);
    expect(g_pops.label == label);
    expect(g_pops.value == kValue);
    expect(g_pops.text == text);
    expect(g_pops.title == kTitle);
    expect(g_pops.sprite == &sprite_value);
    expect(g_pops.flag_a == kFlagA);
    expect(g_pops.flag_b == kFlagB);
    expect(g_pops.callback == &pops_test_callback);

    // pops_no_flags (0x00627670): everything but the flags caller-supplied
    // - caption, value, override text, title, sprite; both flags fixed at
    // 1.
    g_pops.calls = 0;
    expect(pops_no_flags(caption, label, kValue, text, kTitle, &sprite_value,
                         &pops_test_callback) == 0x7A11);
    expect(g_pops.calls == 1);
    expect(g_pops.caption == caption);
    expect(g_pops.label == label);
    expect(g_pops.value == kValue);
    expect(g_pops.text == text);
    expect(g_pops.title == kTitle);
    expect(g_pops.sprite == &sprite_value);
    expect(g_pops.flag_a == 1);
    expect(g_pops.flag_b == 1);
    expect(g_pops.callback == &pops_test_callback);

    PopsOriginalFull = saved_full;
    PopupStartCaption = saved_caption;
}

namespace {

struct AtexitThunkCase {
    void(__cdecl *thunk)();
    void *slot;   // the global seam to rebind
};

const AtexitThunkCase g_atexit_sprite_cases[] = {
    {&destroy_unused_sprite_var02, &g_UNUSED_SPRITE_VAR02},
    {&destroy_unused_sprite_var11, &g_UNUSED_SPRITE_VAR11},
    {&destroy_unused_sprite_var06, &g_UNUSED_SPRITE_VAR06},
    {&destroy_unused_sprite_var09, &g_UNUSED_SPRITE_VAR09},
    {&destroy_unused_sprite_var21, &g_UNUSED_SPRITE_VAR21},
    {&destroy_unused_sprite_var05, &g_UNUSED_SPRITE_VAR05},
    {&destroy_unused_sprite_var08, &g_UNUSED_SPRITE_VAR08},
    {&destroy_unused_sprite_var04, &g_UNUSED_SPRITE_VAR04},
    {&destroy_unused_sprite_var01, &g_UNUSED_SPRITE_VAR01},
    {&destroy_unused_sprite_var18, &g_UNUSED_SPRITE_VAR18},
    {&destroy_unused_sprite_var03, &g_UNUSED_SPRITE_VAR03},
    {&destroy_unused_sprite_var20, &g_UNUSED_SPRITE_VAR20},
    {&destroy_unused_sprite_var16, &g_UNUSED_SPRITE_VAR16},
    {&destroy_unused_sprite_var14, &g_UNUSED_SPRITE_VAR14},
    {&destroy_unused_sprite_var22, &g_UNUSED_SPRITE_VAR22},
    {&destroy_unused_sprite_var10, &g_UNUSED_SPRITE_VAR10},
    {&destroy_unused_sprite_var15, &g_UNUSED_SPRITE_VAR15},
    {&destroy_unused_sprite_var13, &g_UNUSED_SPRITE_VAR13},
    {&destroy_unused_sprite_var17, &g_UNUSED_SPRITE_VAR17},
    {&destroy_unused_sprite_var19, &g_UNUSED_SPRITE_VAR19},
    {&destroy_unused_sprite_var12, &g_UNUSED_SPRITE_VAR12},
    {&destroy_unused_sprite_var07, &g_UNUSED_SPRITE_VAR07},
    {&destroy_iface_std_popups_top_left_sprite, &g_IFACE_STD_POPUPS_TOP_LEFT_SPRITE},
    {&destroy_iface_std_popups_top_right_sprite, &g_IFACE_STD_POPUPS_TOP_RIGHT_SPRITE},
    {&destroy_iface_std_popups_bot_left_sprite, &g_IFACE_STD_POPUPS_BOT_LEFT_SPRITE},
    {&destroy_iface_std_popups_bot_right_sprite, &g_IFACE_STD_POPUPS_BOT_RIGHT_SPRITE},
    {&destroy_iface_std_popups_top_mid_sprite, &g_IFACE_STD_POPUPS_TOP_MID_SPRITE},
    {&destroy_iface_std_popups_bot_mid_sprite, &g_IFACE_STD_POPUPS_BOT_MID_SPRITE},
    {&destroy_iface_std_popups_mid_left_sprite, &g_IFACE_STD_POPUPS_MID_LEFT_SPRITE},
    {&destroy_iface_std_popups_mid_right_sprite, &g_IFACE_STD_POPUPS_MID_RIGHT_SPRITE},
    {&destroy_ter1_mine_sprite, &g_TER1_MINE_SPRITE},
    {&destroy_ter1_solar_collector_sprite, &g_TER1_SOLAR_COLLECTOR_SPRITE},
    {&destroy_ter1_tidal_harness_sprite, &g_TER1_TIDAL_HARNESS_SPRITE},
    {&destroy_ter1_mining_platform_sprite, &g_TER1_MINING_PLATFORM_SPRITE},
    {&destroy_ter1_tut_blank_sprite, &g_TER1_TUT_BLANK_SPRITE},
    {&destroy_ter1_kelp_farm_sprite, &g_TER1_KELP_FARM_SPRITE},
    {&destroy_ter1_condenser_sprite, &g_TER1_CONDENSER_SPRITE},
    {&destroy_ter1_echelon_mirror_sprite, &g_TER1_ECHELON_MIRROR_SPRITE},
    {&destroy_ter1_borehole_sprite, &g_TER1_BOREHOLE_SPRITE},
    {&destroy_ter1_borehole_cluster_sprite, &g_TER1_BOREHOLE_CLUSTER_SPRITE},
    {&destroy_ter1_monolith_sprite, &g_TER1_MONOLITH_SPRITE},
    {&destroy_ter1_bunker_sprite, &g_TER1_BUNKER_SPRITE},
    {&destroy_ter1_airbase_sprite, &g_TER1_AIRBASE_SPRITE},
    {&destroy_ter1_sensor_array_sprite, &g_TER1_SENSOR_ARRAY_SPRITE},
    {&destroy_red_alien_head_icon_sprite, &g_RED_ALIEN_HEAD_ICON_SPRITE},
    {&destroy_red_male_head_icon_sprite, &g_RED_MALE_HEAD_ICON_SPRITE},
    {&destroy_null_resource_icon_sprite, &g_NULL_RESOURCE_ICON_SPRITE},
    {&destroy_icon_tile_square_sprite, &g_ICON_TILE_SQUARE_SPRITE},
    {&destroy_battle_mind_worm_sprite, &g_BATTLE_MIND_WORM_SPRITE},
    {&destroy_battle_isle_deep_sprite, &g_BATTLE_ISLE_DEEP_SPRITE},
    {&destroy_battle_locusts_chiron_sprite, &g_BATTLE_LOCUSTS_CHIRON_SPRITE},
    {&destroy_battle_fungal_tower_sprite, &g_BATTLE_FUNGAL_TOWER_SPRITE},
    {&destroy_battle_spore_launcher_sprite, &g_BATTLE_SPORE_LAUNCHER_SPRITE},
    {&destroy_battle_sealurk_sprite, &g_BATTLE_SEALURK_SPRITE},
    {&destroy_scroll_bar_filler_icon_sprites, &g_SCROLL_BAR_FILLER_ICON_SPRITES},
    {&destroy_scroll_bar_small_filler_icon_sprite, &g_SCROLL_BAR_SMALL_FILLER_ICON_SPRITE},
    {&destroy_iface_general_windows_top_left_sprite, &g_IFACE_GENERAL_WINDOWS_TOP_LEFT_SPRITE},
    {&destroy_iface_general_windows_top_right_sprite, &g_IFACE_GENERAL_WINDOWS_TOP_RIGHT_SPRITE},
    {&destroy_iface_general_windows_bot_left_sprite, &g_IFACE_GENERAL_WINDOWS_BOT_LEFT_SPRITE},
    {&destroy_iface_general_windows_bot_right_sprite, &g_IFACE_GENERAL_WINDOWS_BOT_RIGHT_SPRITE},
    {&destroy_iface_general_windows_mid_left_sprite, &g_IFACE_GENERAL_WINDOWS_MID_LEFT_SPRITE},
    {&destroy_iface_general_windows_mid_right_sprite, &g_IFACE_GENERAL_WINDOWS_MID_RIGHT_SPRITE},
    {&destroy_iface_general_windows_top_mid_sprite, &g_IFACE_GENERAL_WINDOWS_TOP_MID_SPRITE},
    {&destroy_iface_general_windows_bot_mid_sprite, &g_IFACE_GENERAL_WINDOWS_BOT_MID_SPRITE},
    {&destroy_iface_general_windows_noncap_mid_sprite, &g_IFACE_GENERAL_WINDOWS_NONCAP_MID_SPRITE},
    {&destroy_iface_general_windows_noncap_left_sprite, &g_IFACE_GENERAL_WINDOWS_NONCAP_LEFT_SPRITE},
    {&destroy_iface_general_windows_noncap_right_sprite, &g_IFACE_GENERAL_WINDOWS_NONCAP_RIGHT_SPRITE},
    {&destroy_unused_sprite_var23, &g_UNUSED_SPRITE_VAR23},
    {&destroy_unused_sprite_var24, &g_UNUSED_SPRITE_VAR24},
    {&destroy_unused_sprite_var25, &g_UNUSED_SPRITE_VAR25},
    {&destroy_unused_sprite_var26, &g_UNUSED_SPRITE_VAR26},
    {&destroy_unused_sprite_var27, &g_UNUSED_SPRITE_VAR27},
    {&destroy_unused_sprite_var28, &g_UNUSED_SPRITE_VAR28},
    {&destroy_unused_sprite_var29, &g_UNUSED_SPRITE_VAR29},
    {&destroy_unused_sprite_var30, &g_UNUSED_SPRITE_VAR30},
    {&destroy_unused_sprite_var31, &g_UNUSED_SPRITE_VAR31},
    {&destroy_unused_sprite_var32, &g_UNUSED_SPRITE_VAR32},
    {&destroy_unused_sprite_var33, &g_UNUSED_SPRITE_VAR33},
    {&destroy_unused_sprite_var34, &g_UNUSED_SPRITE_VAR34},
    {&destroy_unused_sprite_var35, &g_UNUSED_SPRITE_VAR35},
    {&destroy_unused_sprite_var36, &g_UNUSED_SPRITE_VAR36},
    {&destroy_unused_sprite_var37, &g_UNUSED_SPRITE_VAR37},
    {&destroy_unused_sprite_var38, &g_UNUSED_SPRITE_VAR38},
    {&destroy_unused_sprite_var39, &g_UNUSED_SPRITE_VAR39},
    {&destroy_unused_sprite_var40, &g_UNUSED_SPRITE_VAR40},
    {&destroy_unused_sprite_var41, &g_UNUSED_SPRITE_VAR41},
    {&destroy_unused_sprite_var42, &g_UNUSED_SPRITE_VAR42},
    {&destroy_unused_sprite_var43, &g_UNUSED_SPRITE_VAR43},
    {&destroy_unused_sprite_var44, &g_UNUSED_SPRITE_VAR44},
    {&destroy_unused_sprite_var45, &g_UNUSED_SPRITE_VAR45},
    {&destroy_unused_sprite_var46, &g_UNUSED_SPRITE_VAR46},
    {&destroy_unused_sprite_var47, &g_UNUSED_SPRITE_VAR47},
    {&destroy_unused_sprite_var48, &g_UNUSED_SPRITE_VAR48},
    {&destroy_unused_sprite_var49, &g_UNUSED_SPRITE_VAR49},
    {&destroy_unused_sprite_var50, &g_UNUSED_SPRITE_VAR50},
    {&destroy_unused_sprite_var51, &g_UNUSED_SPRITE_VAR51},
    {&destroy_unused_sprite_var52, &g_UNUSED_SPRITE_VAR52},
    {&destroy_unused_sprite_var53, &g_UNUSED_SPRITE_VAR53},
    {&destroy_unused_sprite_var54, &g_UNUSED_SPRITE_VAR54},
    {&destroy_unused_sprite_var55, &g_UNUSED_SPRITE_VAR55},
    {&destroy_unused_sprite_var56, &g_UNUSED_SPRITE_VAR56},
    {&destroy_unused_sprite_var57, &g_UNUSED_SPRITE_VAR57},
    {&destroy_unused_sprite_var58, &g_UNUSED_SPRITE_VAR58},
    {&destroy_unused_sprite_var59, &g_UNUSED_SPRITE_VAR59},
    {&destroy_unused_sprite_var60, &g_UNUSED_SPRITE_VAR60},
    {&destroy_unused_sprite_var61, &g_UNUSED_SPRITE_VAR61},
    {&destroy_unused_sprite_var62, &g_UNUSED_SPRITE_VAR62},
    {&destroy_unused_sprite_var63, &g_UNUSED_SPRITE_VAR63},
    {&destroy_unused_sprite_var64, &g_UNUSED_SPRITE_VAR64},
    {&destroy_unused_sprite_var65, &g_UNUSED_SPRITE_VAR65},
    {&destroy_unused_sprite_var66, &g_UNUSED_SPRITE_VAR66},
    {&destroy_unused_sprite_var67, &g_UNUSED_SPRITE_VAR67},
    {&destroy_unused_sprite_var68, &g_UNUSED_SPRITE_VAR68},
    {&destroy_unused_sprite_var69, &g_UNUSED_SPRITE_VAR69},
    {&destroy_unused_sprite_var70, &g_UNUSED_SPRITE_VAR70},
    {&destroy_unused_sprite_var71, &g_UNUSED_SPRITE_VAR71},
    {&destroy_unused_sprite_var72, &g_UNUSED_SPRITE_VAR72},
    {&destroy_unused_sprite_var73, &g_UNUSED_SPRITE_VAR73},
    {&destroy_unused_sprite_var74, &g_UNUSED_SPRITE_VAR74},
    {&destroy_unused_sprite_var75, &g_UNUSED_SPRITE_VAR75},
    {&destroy_unused_sprite_var76, &g_UNUSED_SPRITE_VAR76},
    {&destroy_unused_sprite_var77, &g_UNUSED_SPRITE_VAR77},
    {&destroy_unused_sprite_var78, &g_UNUSED_SPRITE_VAR78},
    {&destroy_unused_sprite_var79, &g_UNUSED_SPRITE_VAR79},
    {&destroy_unused_sprite_var80, &g_UNUSED_SPRITE_VAR80},
    {&destroy_unused_sprite_var81, &g_UNUSED_SPRITE_VAR81},
    {&destroy_unused_sprite_var82, &g_UNUSED_SPRITE_VAR82},
    {&destroy_unused_sprite_var83, &g_UNUSED_SPRITE_VAR83},
};

const AtexitThunkCase g_atexit_caviar_cases[] = {
    {&destroy_unused_caviardata_var1, &g_UNUSED_CAVIARDATA_VAR1},
    {&destroy_ssf_caviardata, &g_SSF_CAVIARDATA},
    {&destroy_sdp_caviardata, &g_SDP_CAVIARDATA},
    {&destroy_sas_caviardata, &g_SAS_CAVIARDATA},
    {&destroy_scd_caviardata, &g_SCD_CAVIARDATA},
    {&destroy_scj_caviardata, &g_SCJ_CAVIARDATA},
    {&destroy_sags_caviardata, &g_SAGS_CAVIARDATA},
    {&destroy_sft_caviardata, &g_SFT_CAVIARDATA},
    {&destroy_vhr_caviardata1, &g_VHR_CAVIARDATA1},
    {&destroy_sht_caviardata, &g_SHT_CAVIARDATA},
    {&destroy_srb_caviardata, &g_SRB_CAVIARDATA},
    {&destroy_asas_caviardata, &g_ASAS_CAVIARDATA},
    {&destroy_reslaser_caviardata, &g_RESLASER_CAVIARDATA},
    {&destroy_resbolt_caviardata, &g_RESBOLT_CAVIARDATA},
    {&destroy_funload_caviardata, &g_FUNLOAD_CAVIARDATA},
    {&destroy_tecload_caviardata, &g_TECLOAD_CAVIARDATA},
    {&destroy_sp_disswave_caviardata, &g_SP_DISSWAVE_CAVIARDATA},
    {&destroy_sp_marined_caviardata, &g_SP_MARINED_CAVIARDATA},
    {&destroy_sp_nanoo_caviardata, &g_SP_NANOO_CAVIARDATA},
    {&destroy_sp_soporific_caviardata, &g_SP_SOPORIFIC_CAVIARDATA},
    {&destroy_aa01_caviardata, &g_AA01_CAVIARDATA},
    {&destroy_aa_rover_caviardata, &g_AA_ROVER_CAVIARDATA},
    {&destroy_ax_caviardata, &g_AX_CAVIARDATA},
    {&destroy_aa_caviardata, &g_AA_CAVIARDATA},
    {&destroy_acolpod_caviardata, &g_ACOLPOD_CAVIARDATA},
    {&destroy_at_caviardata, &g_AT_CAVIARDATA},
    {&destroy_vta_caviardata, &g_VTA_CAVIARDATA},
    {&destroy_atp_caviardata, &g_ATP_CAVIARDATA},
    {&destroy_ssfa_caviardata, &g_SSFA_CAVIARDATA},
    {&destroy_sfta_caviardata, &g_SFTA_CAVIARDATA},
    {&destroy_vw00_caviardata, &g_VW00_CAVIARDATA},
    {&destroy_viptawl_caviardata, &g_VIPTAWL_CAVIARDATA},
    {&destroy_viptasgn_caviardata, &g_VIPTASGN_CAVIARDATA},
    {&destroy_viptapsi_caviardata, &g_VIPTAPSI_CAVIARDATA},
    {&destroy_ptmod_caviardata, &g_PTMOD_CAVIARDATA},
    {&destroy_vb_caviardata, &g_VB_CAVIARDATA},
    {&destroy_vbp_caviardata, &g_VBP_CAVIARDATA},
    {&destroy_vgmc_caviardata, &g_VGMC_CAVIARDATA},
    {&destroy_vgmcp_caviardata, &g_VGMCP_CAVIARDATA},
    {&destroy_vlights_caviardata, &g_VLIGHTS_CAVIARDATA},
    {&destroy_vpt_caviardata, &g_VPT_CAVIARDATA},
    {&destroy_a_caviardata, &g_A_CAVIARDATA},
    {&destroy_apwall_caviardata, &g_APWALL_CAVIARDATA},
    {&destroy_asgen_caviardata, &g_ASGEN_CAVIARDATA},
    {&destroy_apsid_caviardata, &g_APSID_CAVIARDATA},
    {&destroy_va01_caviardata, &g_VA01_CAVIARDATA},
    {&destroy_vhr_caviardata2, &g_VHR_CAVIARDATA2},
    {&destroy_vi_caviardata, &g_VI_CAVIARDATA},
    {&destroy_vgmt_caviardata, &g_VGMT_CAVIARDATA},
    {&destroy_vgmtp_caviardata, &g_VGMTP_CAVIARDATA},
    {&destroy_unused_caviardata_var2, &g_UNUSED_CAVIARDATA_VAR2},
    {&destroy_vwntu_caviardata, &g_VWNTU_CAVIARDATA},
    {&destroy_vt_caviardata, &g_VT_CAVIARDATA},
    {&destroy_drop_caviardata, &g_DROP_CAVIARDATA},
    {&destroy_droplet_caviardata, &g_DROPLET_CAVIARDATA},
    {&destroy_vcl_caviardata, &g_VCL_CAVIARDATA},
    {&destroy_vclt00_caviardata, &g_VCLT00_CAVIARDATA},
    {&destroy_vht_vbp_caviardata, &g_VHT_VBP_CAVIARDATA},
    {&destroy_vhtp_caviardata, &g_VHTP_CAVIARDATA},
    {&destroy_vhttp_caviardata, &g_VHTTP_CAVIARDATA},
    {&destroy_vsp_caviardata, &g_VSP_CAVIARDATA},
    {&destroy_vsptf_caviardata, &g_VSPTF_CAVIARDATA},
    {&destroy_vsptb_caviardata, &g_VSPTB_CAVIARDATA},
    {&destroy_vfl_caviardata, &g_VFL_CAVIARDATA},
    {&destroy_vgs_caviardata, &g_VGS_CAVIARDATA},
    {&destroy_vgsp_caviardata, &g_VGSP_CAVIARDATA},
    {&destroy_vjtp_caviardata, &g_VJTP_CAVIARDATA},
    {&destroy_vcu_caviardata, &g_VCU_CAVIARDATA},
    {&destroy_vcup_caviardata, &g_VCUP_CAVIARDATA},
    {&destroy_vcuw_caviardata, &g_VCUW_CAVIARDATA},
    {&destroy_vct_caviardata, &g_VCT_CAVIARDATA},
    {&destroy_vctp_caviardata, &g_VCTP_CAVIARDATA},
    {&destroy_vctb_caviardata, &g_VCTB_CAVIARDATA},
    {&destroy_vwntt_caviardata, &g_VWNTT_CAVIARDATA},
    {&destroy_vwnst_caviardata, &g_VWNST_CAVIARDATA},
    {&destroy_vwnaa_caviardata, &g_VWNAA_CAVIARDATA},
    {&destroy_vm_caviardata, &g_VM_CAVIARDATA},
    {&destroy_vm13_caviardata, &g_VM13_CAVIARDATA},
    {&destroy_nw_caviardata, &g_NW_CAVIARDATA},
    {&destroy_ni_caviardata, &g_NI_CAVIARDATA},
    {&destroy_nlc_caviardata, &g_NLC_CAVIARDATA},
};

const AtexitThunkCase g_atexit_texture_cases[] = {
    {&destroy_radius1_texture, &g_RADIUS1_TEXTURE},
    {&destroy_radius2_texture, &g_RADIUS2_TEXTURE},
    {&destroy_flat_arid_land_texture, &g_FLAT_ARID_LAND_TEXTURE},
    {&destroy_dune_land_texture, &g_DUNE_LAND_TEXTURE},
    {&destroy_rainfall_single_tile_texture, &g_RAINFALL_SINGLE_TILE_TEXTURE},
};

const AtexitThunkCase g_atexit_wave_cases[] = {
    {&destroy_alphamenu_wave, &g_ALPHAMENU_WAVE},
    {&destroy_basewin_wave, &g_BASEWIN_WAVE},
    {&destroy_credits_wave, &g_CREDITS_WAVE},
    {&destroy_designwin_wave, &g_DESIGNWIN_WAVE},
    {&destroy_menu_up_wave, &g_MENU_UP_WAVE},
    {&destroy_menu_down_wave, &g_MENU_DOWN_WAVE},
    {&destroy_scoot_wave, &g_SCOOT_WAVE},
    {&destroy_ok_wave, &g_OK_WAVE},
    {&destroy_passover_wave, &g_PASSOVER_WAVE},
    {&destroy_maininterface_wave, &g_MAININTERFACE_WAVE},
    {&destroy_multiwin_wave, &g_MULTIWIN_WAVE},
    {&destroy_top_menu_wave, &g_TOP_MENU_WAVE},
    {&destroy_crash_landing_wave, &g_CRASH_LANDING_WAVE},
    {&destroy_wave_general, &g_WAVE_GENERAL},
};

const AtexitThunkCase g_atexit_buffer_cases[] = {
    {&destroy_pcx_parse_temp_buffer1, &g_PCX_PARSE_TEMP_BUFFER1},
    {&destroy_iface_std_popups_middle_buffer, &g_IFACE_STD_POPUPS_MIDDLE_BUFFER},
    {&destroy_vehdraw_buffer, &g_VEHDRAW_BUFFER},
    {&destroy_buffer, &g_BUFFER},
};

const AtexitThunkCase g_atexit_group_cases[] = {
    {&destroy_prefwin_buttongroup, &g_PREFWIN_BUTTONGROUP},
};

const AtexitThunkCase g_atexit_battlewin_cases[] = {
    {&destroy_battlewin, &g_BattleWin},
};

const AtexitThunkCase g_atexit_fx_cases[] = {
    {&destroy_fx, &g_FX},
};

const AtexitThunkCase g_atexit_fontqueue_cases[] = {
    {&destroy_fontqueue_val2, &g_FONTQUEUE_VAL2},
    {&destroy_fontqueue_val1, &g_FONTQUEUE_VAL1},
};

const AtexitThunkCase g_atexit_font_cases[] = {
    {&destroy_jackal_font, &g_JACKAL_FONT},
};

struct AtexitArrayCase {
    void(__cdecl *thunk)();
    void *slot;
    uint32_t element_size;
    int count;
    func_thiscall_teardown **teardown_slot;
};

const AtexitArrayCase g_atexit_array_cases[] = {
    {&destroy_cpu_waves, &g_CPU_WAVES, 0x6C, 45, &WaveElementTeardown},
    {&destroy_factionart, &FactionArtGlobal, 0x65C, 8, &FactionArtElementTeardown},
    {&destroy_iface_close_x_sprites, &g_IFACE_CLOSE_X_SPRITES, 0x2C, 3, &SpriteElementTeardown},
    {&destroy_iface_box_sprites1, &g_IFACE_BOX_SPRITES1, 0x2C, 51, &SpriteElementTeardown},
    {&destroy_iface_box_sprites2, &g_IFACE_BOX_SPRITES2, 0x2C, 51, &SpriteElementTeardown},
    {&destroy_iface_box_sprites3, &g_IFACE_BOX_SPRITES3, 0x2C, 51, &SpriteElementTeardown},
    {&destroy_iface_box_sprites4, &g_IFACE_BOX_SPRITES4, 0x2C, 51, &SpriteElementTeardown},
    {&destroy_iface_box_sprites5, &g_IFACE_BOX_SPRITES5, 0x2C, 51, &SpriteElementTeardown},
    {&destroy_iface_box_sprites6, &g_IFACE_BOX_SPRITES6, 0x2C, 51, &SpriteElementTeardown},
    {&destroy_iface_box_sprites7, &g_IFACE_BOX_SPRITES7, 0x2C, 51, &SpriteElementTeardown},
    {&destroy_iface_box_sprites8, &g_IFACE_BOX_SPRITES8, 0x2C, 51, &SpriteElementTeardown},
    {&destroy_iface_box_sprite_buffers, &g_IFACE_BOX_SPRITE_BUFFERS, 0x588, 51, &BufferElementTeardown},
    {&destroy_aa_wing_caviardata, &g_AA_WING_CAVIARDATA, 0xC, 2, &CaviarDataElementTeardown},
    {&destroy_acp_caviardata, &g_ACP_CAVIARDATA, 0xC, 4, &CaviarDataElementTeardown},
    {&destroy_viptr_caviardata, &g_VIPTR_CAVIARDATA, 0xC, 4, &CaviarDataElementTeardown},
    {&destroy_vipta_caviardata, &g_VIPTA_CAVIARDATA, 0xC, 2, &CaviarDataElementTeardown},
    {&destroy_vr_caviardata, &g_VR_CAVIARDATA, 0xC, 4, &CaviarDataElementTeardown},
    {&destroy_vrc_caviardata, &g_VRC_CAVIARDATA, 0xC, 4, &CaviarDataElementTeardown},
    {&destroy_unused_caviardata_var3, &g_UNUSED_CAVIARDATA_VAR3, 0xC, 2, &CaviarDataElementTeardown},
    {&destroy_vhta0_caviardata, &g_VHTA0_CAVIARDATA, 0xC, 2, &CaviarDataElementTeardown},
    {&destroy_vhttpa0_caviardata, &g_VHTTPA0_CAVIARDATA, 0xC, 3, &CaviarDataElementTeardown},
    {&destroy_vspa0_caviardata, &g_VSPA0_CAVIARDATA, 0xC, 2, &CaviarDataElementTeardown},
    {&destroy_vjt0_caviardata, &g_VJT0_CAVIARDATA, 0xC, 2, &CaviarDataElementTeardown},
    {&destroy_vcua0_caviardata, &g_VCUA0_CAVIARDATA, 0xC, 2, &CaviarDataElementTeardown},
    {&destroy_vct0_caviardata, &g_VCT0_CAVIARDATA, 0xC, 2, &CaviarDataElementTeardown},
    {&destroy_vw_caviardata, &g_VW_CAVIARDATA, 0xC, 16, &CaviarDataElementTeardown},
    {&destroy_vpbr0_caviardata, &g_VPBR0_CAVIARDATA, 0xC, 4, &CaviarDataElementTeardown},
    {&destroy_unused_caviardata_var4, &g_UNUSED_CAVIARDATA_VAR4, 0xC, 2, &CaviarDataElementTeardown},
    {&destroy_unused_caviardata_var5, &g_UNUSED_CAVIARDATA_VAR5, 0xC, 5, &CaviarDataElementTeardown},
    {&destroy_rocky_textures, &g_ROCKY_TEXTURES, 0x70, 4, &TextureElementTeardown},
    {&destroy_ocean_textures, &g_OCEAN_TEXTURES, 0x70, 2, &TextureElementTeardown},
    {&destroy_moist_land_textures, &g_MOIST_LAND_TEXTURES, 0x70, 16, &TextureElementTeardown},
    {&destroy_rainy_land_textures, &g_RAINY_LAND_TEXTURES, 0x70, 16, &TextureElementTeardown},
    {&destroy_jungle_land_textures, &g_JUNGLE_LAND_TEXTURES, 0x70, 15, &TextureElementTeardown},
    {&destroy_sunny_mesa_textures, &g_SUNNY_MESA_TEXTURES, 0x70, 8, &TextureElementTeardown},
    {&destroy_rainfall_single_tile_textures, &g_RAINFALL_SINGLE_TILE_TEXTURES, 0x70, 2, &TextureElementTeardown},
    {&destroy_road_textures, &g_ROAD_TEXTURES, 0x70, 9, &TextureElementTeardown},
    {&destroy_magtube_textures, &g_MAGTUBE_TEXTURES, 0x70, 9, &TextureElementTeardown},
    {&destroy_river_textures, &g_RIVER_TEXTURES, 0x70, 16, &TextureElementTeardown},
    {&destroy_mount_planet_textures, &g_MOUNT_PLANET_TEXTURES, 0x70, 3, &TextureElementTeardown},
    {&destroy_garland_crater_textures, &g_GARLAND_CRATER_TEXTURES, 0x70, 3, &TextureElementTeardown},
    {&destroy_fungus_textures, &g_FUNGUS_TEXTURES, 0x70, 30, &TextureElementTeardown},
    {&destroy_farm_textures, &g_FARM_TEXTURES, 0x70, 9, &TextureElementTeardown},
    {&destroy_forest_textures, &g_FOREST_TEXTURES, 0x70, 16, &TextureElementTeardown},
    {&destroy_ter1_white_org_yel_tile_sprites, &g_TER1_WHITE_ORG_YEL_TILE_SPRITES, 0x2C, 6, &SpriteElementTeardown},
    {&destroy_ter1_bottom_left_tile_sprites, &g_TER1_BOTTOM_LEFT_TILE_SPRITES, 0x2C, 9, &SpriteElementTeardown},
    {&destroy_ter1_unused_sprites2, &g_TER1_UNUSED_SPRITES2, 0x2C, 2, &SpriteElementTeardown},
    {&destroy_ter1_manifold_nexus_sprites, &g_TER1_MANIFOLD_NEXUS_SPRITES, 0x2C, 6, &SpriteElementTeardown},
    {&destroy_ter1wreck_unity_wreckage_sprites, &g_TER1WRECK_UNITY_WRECKAGE_SPRITES, 0x2C, 15, &SpriteElementTeardown},
    {&destroy_ter1wreck_unity_wreckage_alt_sprites, &g_TER1WRECK_UNITY_WRECKAGE_ALT_SPRITES, 0x2C, 4, &SpriteElementTeardown},
    {&destroy_fossil_field_ridge_sprites, &g_FOSSIL_FIELD_RIDGE_SPRITES, 0x2C, 6, &SpriteElementTeardown},
    {&destroy_ter1_unused_sprites1, &g_TER1_UNUSED_SPRITES1, 0x2C, 5, &SpriteElementTeardown},
    {&destroy_ter1_farm_sprites, &g_TER1_FARM_SPRITES, 0x2C, 5, &SpriteElementTeardown},
    {&destroy_ter1_soil_enricher_sprites, &g_TER1_SOIL_ENRICHER_SPRITES, 0x2C, 5, &SpriteElementTeardown},
    {&destroy_ter1_sea_land_resource_sprites, &g_TER1_SEA_LAND_RESOURCE_SPRITES, 0x2C, 12, &SpriteElementTeardown},
    {&destroy_ter1_landmark_resource_sprites, &g_TER1_LANDMARK_RESOURCE_SPRITES, 0x2C, 6, &SpriteElementTeardown},
    {&destroy_glow_sprites, &g_GLOW_SPRITES, 0x2C, 2, &SpriteElementTeardown},
    {&destroy_ter1_unity_pod_sprites, &g_TER1_UNITY_POD_SPRITES, 0x2C, 6, &SpriteElementTeardown},
    {&destroy_rainfall_double_tile_sprites, &g_RAINFALL_DOUBLE_TILE_SPRITES, 0x2C, 2, &SpriteElementTeardown},
    {&destroy_veh_sprites, &g_VEH_SPRITES, 0x2C, 152, &SpriteElementTeardown},
    {&destroy_flags_veh_sprites, &g_FLAGS_VEH_SPRITES, 0x2C, 112, &SpriteElementTeardown},
    {&destroy_icons_general_sprites, &g_ICONS_GENERAL_SPRITES, 0x2C, 16, &SpriteElementTeardown},
    {&destroy_resource_icon_sprites, &g_RESOURCE_ICON_SPRITES, 0x2C, 32, &SpriteElementTeardown},
    {&destroy_citizen_lg_cursor_sprites, &g_CITIZEN_LG_CURSOR_SPRITES, 0x2C, 8, &SpriteElementTeardown},
    {&destroy_specialist_lg_cursor_sprites, &g_SPECIALIST_LG_CURSOR_SPRITES, 0x2C, 7, &SpriteElementTeardown},
    {&destroy_citizen_sm_cursor_sprites, &g_CITIZEN_SM_CURSOR_SPRITES, 0x2C, 8, &SpriteElementTeardown},
    {&destroy_specialist_sm_cursor_sprites, &g_SPECIALIST_SM_CURSOR_SPRITES, 0x2C, 7, &SpriteElementTeardown},
    {&destroy_al_citizen_lg_cursor_sprites, &g_AL_CITIZEN_LG_CURSOR_SPRITES, 0x2C, 4, &SpriteElementTeardown},
    {&destroy_al_specialist_lg_cursor_sprites, &g_AL_SPECIALIST_LG_CURSOR_SPRITES, 0x2C, 7, &SpriteElementTeardown},
    {&destroy_al_citizen_sm_cursor_sprites, &g_AL_CITIZEN_SM_CURSOR_SPRITES, 0x2C, 4, &SpriteElementTeardown},
    {&destroy_al_specialist_sm_cursor_sprites, &g_AL_SPECIALIST_SM_CURSOR_SPRITES, 0x2C, 7, &SpriteElementTeardown},
    {&destroy_silver_menu_icon_sprites, &g_SILVER_MENU_ICON_SPRITES, 0x2C, 4, &SpriteElementTeardown},
    {&destroy_silver_checkbox_icon_sprites, &g_SILVER_CHECKBOX_ICON_SPRITES, 0x2C, 2, &SpriteElementTeardown},
    {&destroy_peace_sign_sprites, &g_PEACE_SIGN_SPRITES, 0x2C, 2, &SpriteElementTeardown},
    {&destroy_xi_boom_veh_sprites, &g_XI_BOOM_VEH_SPRITES, 0x2C, 144, &SpriteElementTeardown},
    {&destroy_xf_boom_veh_sprites, &g_XF_BOOM_VEH_SPRITES, 0x2C, 64, &SpriteElementTeardown},
    {&destroy_tech_icon_sprites, &g_TECH_ICON_SPRITES, 0x2C, 89, &SpriteElementTeardown},
    {&destroy_facility_icon_sprites, &g_FACILITY_ICON_SPRITES, 0x2C, 70, &SpriteElementTeardown},
    {&destroy_secret_project_icon_sprites, &g_SECRET_PROJECT_ICON_SPRITES, 0x2C, 64, &SpriteElementTeardown},
    {&destroy_iface_mp_combo_arrow_sprites, &g_IFACE_MP_COMBO_ARROW_SPRITES, 0x2C, 3, &SpriteElementTeardown},
    {&destroy_scroll_bar_arrow_icon_sprites, &g_SCROLL_BAR_ARROW_ICON_SPRITES, 0x2C, 12, &SpriteElementTeardown},
    {&destroy_scroll_bar_small_arrow_icon_sprites, &g_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES, 0x2C, 12, &SpriteElementTeardown},
    {&destroy_iface_lock_sprites, &g_IFACE_LOCK_SPRITES, 0x2C, 2, &SpriteElementTeardown},
    {&destroy_unused_sprites_var01, &g_UNUSED_SPRITES_VAR01, 0x2C, 8, &SpriteElementTeardown},
    {&destroy_iface_tech_tree_arrow_sprites, &g_IFACE_TECH_TREE_ARROW_SPRITES, 0x2C, 3, &SpriteElementTeardown},
    {&destroy_unused_sprites_var02, &g_UNUSED_SPRITES_VAR02, 0x2C, 3, &SpriteElementTeardown},
    {&destroy_unused_sprites_var03, &g_UNUSED_SPRITES_VAR03, 0x2C, 3, &SpriteElementTeardown},
    {&destroy_unused_sprites_var04, &g_UNUSED_SPRITES_VAR04, 0x2C, 3, &SpriteElementTeardown},
    {&destroy_unused_sprites_var05, &g_UNUSED_SPRITES_VAR05, 0x2C, 3, &SpriteElementTeardown},
    {&destroy_unused_sprites_var06, &g_UNUSED_SPRITES_VAR06, 0x2C, 3, &SpriteElementTeardown},
    {&destroy_unused_sprites_var07, &g_UNUSED_SPRITES_VAR07, 0x2C, 3, &SpriteElementTeardown},
    {&destroy_unused_sprites_var08, &g_UNUSED_SPRITES_VAR08, 0x2C, 3, &SpriteElementTeardown},
    {&destroy_unused_sprites_var09, &g_UNUSED_SPRITES_VAR09, 0x2C, 3, &SpriteElementTeardown},
    {&destroy_unused_sprites_var10, &g_UNUSED_SPRITES_VAR10, 0x2C, 3, &SpriteElementTeardown},
    {&destroy_basewin_sprites, &g_BASEWIN_SPRITES, 0x2C, 27, &SpriteElementTeardown},
    {&destroy_iface_green_right_arrow_sprite, &g_IFACE_GREEN_RIGHT_ARROW_SPRITE, 0x2C, 1, &SpriteElementTeardown},
    {&destroy_cursor_sprites, &g_CURSOR_SPRITES, 0x2C, 12, &SpriteElementTeardown},
    {&destroy_fonts, &g_FONTS, 0x28, 48, &FontElementTeardown},
    {&destroy_txtindex, &TxtIndexGlobal, 0x118, 4, &TextIndexElementTeardown},
};

// The opaque-storage thunks carry no typed global to
// rebind, so the storage address itself is the
// expectation: rebind the teardown seam and check the
// pointer it is handed. Two thunks sharing a seam
// (NetMsg, Palette) each still assert their own address.
struct AtexitOpaqueCase {
    void(__cdecl *thunk)();
    func_thiscall_teardown **slot;
    void *storage;
};

const AtexitOpaqueCase g_atexit_opaque_cases[] = {
    {&destroy_basewin, &BaseWinDtorTarget,
     reinterpret_cast<void *>(0x006A7628)},
    {&destroy_councwin, &CouncWinDtorTarget,
     reinterpret_cast<void *>(0x006FEC80)},
    {&destroy_datalink, &DatalinkDtorTarget,
     reinterpret_cast<void *>(0x00703EA0)},
    {&destroy_designwin, &DesignWinDtorTarget,
     reinterpret_cast<void *>(0x0071F2B0)},
    {&destroy_diplopop, &DiploPopDtorTarget,
     reinterpret_cast<void *>(0x00733990)},
    {&destroy_diplowin, &DiploWinDtorTarget,
     reinterpret_cast<void *>(0x0073ACD8)},
    {&destroy_famewin, &FameWinDtorTarget,
     reinterpret_cast<void *>(0x0074DAF8)},
    {&destroy_infowin, &InfoWinDtorTarget,
     reinterpret_cast<void *>(0x007AD2A0)},
    {&destroy_maininterface, &MainInterfaceDtorTarget,
     reinterpret_cast<void *>(0x007AE820)},
    {&destroy_messagewin, &MessageWinDtorTarget,
     reinterpret_cast<void *>(0x007F67F8)},
    {&destroy_monuwin, &MonuWinDtorTarget,
     reinterpret_cast<void *>(0x007F9F58)},
    {&destroy_multiwin, &MultiWinDtorTarget,
     reinterpret_cast<void *>(0x007FD648)},
    {&destroy_netmsg1, &NetMsgDtorTarget,
     reinterpret_cast<void *>(0x00805338)},
    {&destroy_netmsg2, &NetMsgDtorTarget,
     reinterpret_cast<void *>(0x007FFF80)},
    {&destroy_netwin, &NetWinDtorTarget,
     reinterpret_cast<void *>(0x0080A6F8)},
    {&destroy_newtechwin, &NewTechWinDtorTarget,
     reinterpret_cast<void *>(0x00811E40)},
    {&destroy_pickwin, &PickWinDtorTarget,
     reinterpret_cast<void *>(0x00822718)},
    {&destroy_prefwin, &PrefWinDtorTarget,
     reinterpret_cast<void *>(0x008578D8)},
    {&destroy_quaylewin, &QuayleWinDtorTarget,
     reinterpret_cast<void *>(0x00872CB0)},
    {&destroy_reportif, &ReportIfDtorTarget,
     reinterpret_cast<void *>(0x00885F38)},
    {&destroy_reportwin, &ReportWinDtorTarget,
     reinterpret_cast<void *>(0x00876478)},
    {&destroy_socialwinparent, &SocialWinDtorTarget,
     reinterpret_cast<void *>(0x008A6270)},
    {&destroy_statuswin, &StatusWinDtorTarget,
     reinterpret_cast<void *>(0x008C5568)},
    {&destroy_tutwin, &TutWinDtorTarget,
     reinterpret_cast<void *>(0x008C6E68)},
    {&destroy_vehdraw_caviar, &CaviarCloseTarget,
     reinterpret_cast<void *>(0x008CC828)},
    {&destroy_worldwin, &WorldWinDtorTarget,
     reinterpret_cast<void *>(0x008E9F60)},
    {&destroy_wave_device, &Wave_DeviceDtorTarget,
     reinterpret_cast<void *>(0x0090D978)},
    {&destroy_midi_device, &Midi_DeviceDtorTarget,
     reinterpret_cast<void *>(0x0090D950)},
    {&destroy_wave_in_device, &Wave_In_DeviceDtorTarget,
     reinterpret_cast<void *>(0x0090DB50)},
    {&destroy_console_timer, &TimeDtorTarget,
     reinterpret_cast<void *>(0x00939E88)},
    {&destroy_netdaemon, &NetDaemonDtorTarget,
     reinterpret_cast<void *>(0x0093CD90)},
    {&destroy_palette1, &PaletteDtorTarget,
     reinterpret_cast<void *>(0x0094C590)},
    {&destroy_palette2, &PaletteDtorTarget,
     reinterpret_cast<void *>(0x009523A0)},
    {&destroy_multidebug, &MultiDebugDtorTarget,
     reinterpret_cast<void *>(0x009B22F0)},
    {&destroy_stringtable, &StringsDtorTarget,
     reinterpret_cast<void *>(0x009B90D8)},
};

Wave *g_atexit_wave_seen;

int g_atexit_wave_calls;

void __thiscall observe_wave_destructor(Wave *wave) {
    g_atexit_wave_seen = wave;
    ++g_atexit_wave_calls;
}

void *g_vector_array_seen;

uint32_t g_vector_size_seen;

int g_vector_count_seen;

func_thiscall_teardown *g_vector_teardown_seen;

int g_vector_calls;

void __stdcall observe_vector_dtor(void *array, unsigned int element_size,
                                   int count,
                                   func_thiscall_teardown *teardown) {
    g_vector_array_seen = array;
    g_vector_size_seen = element_size;
    g_vector_count_seen = count;
    g_vector_teardown_seen = teardown;
    ++g_vector_calls;
}

int g_vector_sentinel;

func_thiscall_teardown *g_vector_ctor_seen;

void __stdcall observe_vector_ctor_iterator(void *array,
                                            unsigned int element_size,
                                            int count,
                                            func_thiscall_teardown *ctor,
                                            func_thiscall_teardown *dtor) {
    g_vector_array_seen = array;
    g_vector_size_seen = element_size;
    g_vector_count_seen = count;
    g_vector_ctor_seen = ctor;
    g_vector_teardown_seen = dtor;
    ++g_vector_calls;
}

void *g_atexit_opaque_seen;

int g_atexit_opaque_calls;

void __thiscall observe_opaque_teardown(void *object) {
    g_atexit_opaque_seen = object;
    ++g_atexit_opaque_calls;
}

}  // namespace

void test_atexit_teardown_thunks() {
    // Every thunk is "tear down the object at this fixed address". Rebinding
    // the per-global seam to a local object and watching the teardown's own
    // observable - the free seam it calls, or the destructor dependency -
    // proves each thunk reaches its own global and the right teardown.
    auto *const saved_sprite_free = SpriteFree;
    int *const saved_sprite_memory = SpriteMemoryUsed;
    auto *const saved_caviar_free = CaviarDataFreeRecord;
    auto *const saved_texture_free = TextureFree;
    auto *const saved_wave_dtor = WaveOriginalDestructor;
    SpriteFree = &observe_texture_free;   // same shape; shared observer
    TextureFree = &observe_texture_free;
    CaviarDataFreeRecord = &observe_caviar_free_record;
    WaveOriginalDestructor = &observe_wave_destructor;
    int memory_used = 0;
    SpriteMemoryUsed = &memory_used;

    auto *const saved_iterator = VectorDtorIterator;
    for (const AtexitThunkCase &entry : g_atexit_sprite_cases) {
        alignas(4) uint8_t fake[sizeof(Sprite)] = {};
        int sentinel = 0;
        void *pointer = &sentinel;
        std::memcpy(fake + 0x00, &pointer, 4);    // second free branch only
        auto **slot = static_cast<Sprite **>(entry.slot);
        Sprite *const saved = *slot;
        *slot = reinterpret_cast<Sprite *>(fake);
        g_tex_free_calls = 0;
        g_tex_freed = nullptr;
        entry.thunk();
        expect(g_tex_free_calls == 1);
        expect(g_tex_freed == &sentinel);         // its own global, freed once
        *slot = saved;
    }

    for (const AtexitThunkCase &entry : g_atexit_caviar_cases) {
        alignas(4) uint8_t fake[sizeof(CaviarData)] = {};
        int sentinel = 0;
        void *pointer = &sentinel;
        std::memcpy(fake + 0x8, &pointer, 4);
        auto **slot = static_cast<CaviarData **>(entry.slot);
        CaviarData *const saved = *slot;
        *slot = reinterpret_cast<CaviarData *>(fake);
        g_caviar_free_calls = 0;
        g_caviar_freed = nullptr;
        entry.thunk();
        expect(g_caviar_free_calls == 1);
        expect(g_caviar_freed == &sentinel);
        *slot = saved;
    }

    for (const AtexitThunkCase &entry : g_atexit_texture_cases) {
        alignas(4) uint8_t fake[sizeof(Texture)] = {};
        int sentinel = 0;
        void *pointer = &sentinel;
        std::memcpy(fake + 0x00, &pointer, 4);    // ours: borrowed_ stays 0
        auto **slot = static_cast<Texture **>(entry.slot);
        Texture *const saved = *slot;
        *slot = reinterpret_cast<Texture *>(fake);
        g_tex_free_calls = 0;
        g_tex_freed = nullptr;
        entry.thunk();
        expect(g_tex_free_calls == 1);
        expect(g_tex_freed == &sentinel);
        *slot = saved;
    }

    for (const AtexitThunkCase &entry : g_atexit_wave_cases) {
        alignas(4) uint8_t fake[sizeof(Wave)] = {};
        auto **slot = static_cast<Wave **>(entry.slot);
        Wave *const saved = *slot;
        *slot = reinterpret_cast<Wave *>(fake);
        g_atexit_wave_calls = 0;
        g_atexit_wave_seen = nullptr;
        entry.thunk();
        expect(g_atexit_wave_calls == 1);
        expect(g_atexit_wave_seen == reinterpret_cast<Wave *>(fake));
        *slot = saved;
    }

    // Array thunks hand the whole walk to the iterator seam: the observed
    // call must carry the rebound array, the exact element size and count,
    // and the rebound per-element teardown - proving the body reads all four
    // seams rather than any baked literal.
    VectorDtorIterator = &observe_vector_dtor;
    for (const AtexitArrayCase &entry : g_atexit_array_cases) {
        alignas(4) uint8_t fake[4] = {};
        auto **slot = static_cast<void **>(entry.slot);
        void *const saved_slot = *slot;
        func_thiscall_teardown *const saved_teardown = *entry.teardown_slot;
        *slot = fake;
        *entry.teardown_slot =
            reinterpret_cast<func_thiscall_teardown *>(&g_vector_sentinel);
        g_vector_calls = 0;
        entry.thunk();
        expect(g_vector_calls == 1);
        expect(g_vector_array_seen == fake);
        expect(g_vector_size_seen == entry.element_size);
        expect(g_vector_count_seen == entry.count);
        expect(g_vector_teardown_seen ==
               reinterpret_cast<func_thiscall_teardown *>(&g_vector_sentinel));
        *slot = saved_slot;
        *entry.teardown_slot = saved_teardown;
    }
    VectorDtorIterator = saved_iterator;

    // Buffer and ButtonGroup teardowns are already source-owned, so each
    // thunk is checked against ground truth directly: run the real teardown
    // on an identical twin and require the bytes to agree.
    //
    // Buffer::close reads *BufferResetValue520, whose default target is an
    // address in the original image. Nothing maps that address in the
    // standalone leaf process, so it has to be repointed at a local slot
    // first. This suite got away without it for a long time only because the
    // address happened to fall inside the test binary's own image; growing
    // the binary moved it outside and turned it into a page fault in the
    // release build while debug still passed.
    uint32_t *const saved_reset_520 = BufferResetValue520;
    uint32_t reset_520_slot = 0;
    BufferResetValue520 = &reset_520_slot;
    for (const AtexitThunkCase &entry : g_atexit_buffer_cases) {
        alignas(4) uint8_t fake[sizeof(Buffer)] = {};
        alignas(4) uint8_t twin[sizeof(Buffer)] = {};
        auto **slot = static_cast<Buffer **>(entry.slot);
        Buffer *const saved = *slot;
        *slot = reinterpret_cast<Buffer *>(fake);
        entry.thunk();
        reinterpret_cast<Buffer *>(twin)->destroy();
        expect_storage_bytes(fake, twin, sizeof(fake));
        *slot = saved;
    }
    BufferResetValue520 = saved_reset_520;
    for (const AtexitThunkCase &entry : g_atexit_group_cases) {
        alignas(4) uint8_t fake[sizeof(ButtonGroup)];
        alignas(4) uint8_t twin[sizeof(ButtonGroup)];
        seed_storage(fake, twin, sizeof(fake));
        auto **slot = static_cast<ButtonGroup **>(entry.slot);
        ButtonGroup *const saved = *slot;
        *slot = reinterpret_cast<ButtonGroup *>(fake);
        entry.thunk();
        reinterpret_cast<ButtonGroup *>(twin)->close();
        expect_storage_bytes(fake, twin, sizeof(fake));
        *slot = saved;
    }

    // FX and FontQueue teardowns hand their member-array walk to the vector
    // iterator; the observed array base must be the rebound global. Sizes and
    // counts are pinned by the destructors' own suites.
    VectorDtorIterator = &observe_vector_dtor;
    for (const AtexitThunkCase &entry : g_atexit_fx_cases) {
        alignas(4) uint8_t fake[sizeof(FX)];
        auto **slot = static_cast<FX **>(entry.slot);
        FX *const saved = *slot;
        *slot = reinterpret_cast<FX *>(fake);
        g_vector_calls = 0;
        entry.thunk();
        expect(g_vector_calls == 1);
        expect(g_vector_array_seen == fake);
        *slot = saved;
    }
    for (const AtexitThunkCase &entry : g_atexit_fontqueue_cases) {
        alignas(4) uint8_t fake[sizeof(FontQueue)];
        auto **slot = static_cast<FontQueue **>(entry.slot);
        FontQueue *const saved = *slot;
        *slot = reinterpret_cast<FontQueue *>(fake);
        g_vector_calls = 0;
        entry.thunk();
        expect(g_vector_calls == 1);
        expect(g_vector_array_seen == fake);
        *slot = saved;
    }
    VectorDtorIterator = saved_iterator;

    // The Font teardown runs the suite's Font::close double, whose writes to
    // the rebound object are themselves the observation.
    for (const AtexitThunkCase &entry : g_atexit_font_cases) {
        alignas(4) uint8_t fake[sizeof(Font)];
        uint8_t untouched[sizeof(Font)];
        seed_storage(fake, untouched, sizeof(fake));
        auto **slot = static_cast<Font **>(entry.slot);
        Font *const saved = *slot;
        *slot = reinterpret_cast<Font *>(fake);
        entry.thunk();
        int32_t first = 0;
        std::memcpy(&first, fake, 4);
        expect(first == -1);               // the double's signature write
        *slot = saved;
    }

    // The BattleWin teardown is the Time member at +8, observed through the
    // suite's Time::close double recording its receiver.
    for (const AtexitThunkCase &entry : g_atexit_battlewin_cases) {
        alignas(4) uint8_t fake[sizeof(BattleWin)] = {};
        auto **slot = static_cast<BattleWin **>(entry.slot);
        BattleWin *const saved = *slot;
        *slot = reinterpret_cast<BattleWin *>(fake);
        time_close_calls = 0;
        entry.thunk();
        expect(time_close_calls == 1);
        expect(time_close_targets[0] == reinterpret_cast<Time *>(fake + 8));
        *slot = saved;
    }

    // Opaque-storage thunks: the whole body is to hand this
    // address to that teardown, so the rebound seam receiving exactly
    // the recorded address, exactly once, is the whole contract. The
    // address is a literal in the body rather than a rebindable global,
    // which is what lets one seam serve two thunks over different storage.
    for (const AtexitOpaqueCase &entry : g_atexit_opaque_cases) {
        func_thiscall_teardown *const saved = *entry.slot;
        *entry.slot = &observe_opaque_teardown;
        g_atexit_opaque_calls = 0;
        g_atexit_opaque_seen = nullptr;
        entry.thunk();
        expect(g_atexit_opaque_calls == 1);
        expect(g_atexit_opaque_seen == entry.storage);
        *entry.slot = saved;
    }

    SpriteFree = saved_sprite_free;
    SpriteMemoryUsed = saved_sprite_memory;
    CaviarDataFreeRecord = saved_caviar_free;
    TextureFree = saved_texture_free;
    WaveOriginalDestructor = saved_wave_dtor;
}

void test_battle_win_dtor() {
    // The whole teardown is the Time member at +8: its close runs with that
    // member as `this`, and nothing outside that member is written. The
    // suite's Time::close double zeroes the member and stamps its resolution,
    // so the expectation models exactly that span and nothing else.
    alignas(4) uint8_t storage[sizeof(BattleWin) + 16];
    uint8_t expected[sizeof(storage)];
    auto *win = reinterpret_cast<BattleWin *>(storage);
    seed_storage(storage, expected, sizeof(storage));
    std::memcpy(expected, storage, sizeof(storage));
    std::memset(expected + 8, 0, sizeof(Time));
    const uint32_t resolution = 5;
    std::memcpy(expected + 8 + 0x20, &resolution, sizeof(resolution));
    time_close_calls = 0;      // the recording window is the first 8 calls
    win->~BattleWin();
    expect(time_close_calls == 1);
    expect(time_close_targets[0] == reinterpret_cast<Time *>(storage + 8));
    expect_storage_bytes(storage, expected, sizeof(storage));
    battle_win_dtor_redirect(win, nullptr);
    expect(time_close_calls == 2);
}

void test_fx_and_font_queue_dtors() {
    // Both destructors hand a member-array walk to the vector iterator with
    // the object itself as the array base: FX walks 0x61 Wave-shaped effects
    // of 0x6C bytes, FontQueue walks its three 0x28-byte Font slots. The
    // observer pins all four arguments, and the teardown argument must be
    // whatever the element seam currently holds - rebound here - rather than
    // any baked address.
    auto *const saved_iterator = VectorDtorIterator;
    VectorDtorIterator = &observe_vector_dtor;

    alignas(4) uint8_t fx_storage[sizeof(FX)];
    auto *fx = reinterpret_cast<FX *>(fx_storage);
    func_thiscall_teardown *const saved_effect = EffectElementTeardown;

    // The construction side first, through the same bank storage. Both
    // element seams are rebound to DISTINCT sentinels: the original pushes
    // the destructor before the constructor, so the one way to get this
    // function silently wrong is to hand the iterator the pair transposed,
    // and identical sentinels would not see it. Same for the stride/count
    // pair, which the original also pushes in the reverse of written order.
    auto *const saved_ctor_iterator = VectorCtorIterator;
    VectorCtorIterator = &observe_vector_ctor_iterator;
    func_thiscall_teardown *const saved_effect_ctor = EffectElementCtor;
    EffectElementCtor =
        reinterpret_cast<func_thiscall_teardown *>(&g_vector_sentinel);
    EffectElementTeardown =
        reinterpret_cast<func_thiscall_teardown *>(&g_vector_sentinel) + 1;
    g_vector_calls = 0;
    g_vector_ctor_seen = nullptr;
    g_vector_teardown_seen = nullptr;
    expect(fx_ctor_redirect(fx, nullptr) == fx);
    expect(g_vector_calls == 1);
    expect(g_vector_array_seen == fx_storage);
    expect(g_vector_size_seen == 0x6C);
    expect(g_vector_count_seen == 0x61);
    expect(g_vector_ctor_seen ==
           reinterpret_cast<func_thiscall_teardown *>(&g_vector_sentinel));
    expect(g_vector_teardown_seen ==
           reinterpret_cast<func_thiscall_teardown *>(&g_vector_sentinel) + 1);
    EffectElementCtor = saved_effect_ctor;
    VectorCtorIterator = saved_ctor_iterator;

    EffectElementTeardown =
        reinterpret_cast<func_thiscall_teardown *>(&g_vector_sentinel);
    g_vector_calls = 0;
    fx->~FX();
    expect(g_vector_calls == 1);
    expect(g_vector_array_seen == fx_storage);
    expect(g_vector_size_seen == 0x6C);
    expect(g_vector_count_seen == 0x61);
    expect(g_vector_teardown_seen ==
           reinterpret_cast<func_thiscall_teardown *>(&g_vector_sentinel));
    fx_dtor_redirect(fx, nullptr);
    expect(g_vector_calls == 2);
    EffectElementTeardown = saved_effect;

    alignas(4) uint8_t queue_storage[sizeof(FontQueue)];
    auto *queue = reinterpret_cast<FontQueue *>(queue_storage);
    func_thiscall_teardown *const saved_font = FontQueueElementTeardown;
    FontQueueElementTeardown =
        reinterpret_cast<func_thiscall_teardown *>(&g_vector_sentinel);
    g_vector_calls = 0;
    queue->~FontQueue();
    expect(g_vector_calls == 1);
    expect(g_vector_array_seen == queue_storage);
    expect(g_vector_size_seen == 0x28);
    expect(g_vector_count_seen == 3);
    expect(g_vector_teardown_seen ==
           reinterpret_cast<func_thiscall_teardown *>(&g_vector_sentinel));
    font_queue_dtor_redirect(queue, nullptr);
    expect(g_vector_calls == 2);
    FontQueueElementTeardown = saved_font;

    VectorDtorIterator = saved_iterator;
}

// Console::update_data (0x00514880) is a straight-line three-call dispatch
// that touches no field of any object: ecx is dead on entry and is overwritten
// by `mov ecx, 0x7ad2a0` before anything reads it, and the body contains no
// mov with a memory destination. The oracle is compositional: the reference
// image and event log are produced by running the documented component chain
//   InfoWin::change -> StatusWin::redraw seam -> MapWin::main_caption
// over byte-identical twin objects, and update_data must reproduce both.
//
// Two effects are deliberately NOT asserted, because the original discards
// them and so would an in-process differential against it: the a1 handed to
// InfoWin::change (0x00458900 is a bare ret 4), and the MapWin pointer loaded
// from the 0x007D3C3C slot (main_caption overwrites ecx before using it).
// Both are recorded as Verification notes on the recovery.

namespace {

struct ConsoleUpdateEvent {
    int kind;             // 1 = StatusWin::redraw, 2 = MainInterface::set_date
    const void *target;   // the `this` the dependency was handed
    const char *text;     // set_date's caption argument, nullptr otherwise
    int probe;            // which of the two distinct probe bodies ran
};

ConsoleUpdateEvent console_update_events[8];

int console_update_event_count = 0;

void console_update_reset_log() {
    console_update_event_count = 0;
    std::memset(console_update_events, 0, sizeof(console_update_events));
}

void console_update_record(int kind, const void *target, const char *text,
                           int probe) {
    const int index = console_update_event_count++;
    if (index >= static_cast<int>(ARRAYSIZE(console_update_events))) {
        return;
    }
    ConsoleUpdateEvent &event = console_update_events[index];
    event.kind = kind;
    event.target = target;
    event.text = text;
    event.probe = probe;
}

// Two distinct bodies per seam. Shape 1 installs the second pair, so a seam
// that were bound once instead of read per call would keep reporting probe 0.
void __thiscall console_update_redraw_probe_a(void *self) {
    console_update_record(1, self, nullptr, 0);
}

void __thiscall console_update_redraw_probe_b(void *self) {
    console_update_record(1, self, nullptr, 1);
}

void __thiscall console_update_set_date_probe_a(void *self, char *text) {
    console_update_record(2, self, text, 0);
}

void __thiscall console_update_set_date_probe_b(void *self, char *text) {
    console_update_record(2, self, text, 1);
}

// Console and MapWin are 0x247A8 and 0x22480 bytes, so the per-byte
// expect_storage_bytes() loop would issue ~1.2M assertions per suite run and
// slow every mutation-harness iteration. Compare with memcmp and fall back to
// the shared reporter, which names the fixture and the first differing
// offset, only when the images actually differ.
void console_update_expect_image(const char *fixture, const uint8_t *actual,
                                 const uint8_t *reference, size_t size) {
    const bool equal = std::memcmp(actual, reference, size) == 0;
    if (!equal) {
        report_storage_mismatch(fixture, actual, reference, size);
    }
    expect(equal);
}

// Console::focus fixture. Every callee is a recorder, and the two that can run
// mid-walk are able to republish the map-window table so the body's re-reads
// are observable rather than merely asserted in a comment.
struct FocusRecord {
    int cursor_calls;
    void *cursor_self;
    int cursor_x;
    int cursor_y;
    int focus_calls;
    void *focus_targets[16];
    int focus_x;
    int focus_y;
    int draw_calls;
    void *draw_target;
    int draw_type;
    int flush_calls;
    int flush_at_focus_calls;
    uint32_t sequence;
};

FocusRecord g_focus_rec;

int g_focus_result;

// When set, the cursor recorder swaps this in as slot 0, so a body that reused
// the pointer it loaded at the top of the iteration is caught.
void *g_focus_cursor_installs;

void **g_focus_table_slot0;


void __thiscall focus_stub_cursor_next(void *console, int x, int y) {
    ++g_focus_rec.cursor_calls;
    g_focus_rec.cursor_self = console;
    g_focus_rec.cursor_x = x;
    g_focus_rec.cursor_y = y;
    g_focus_rec.sequence = g_focus_rec.sequence * 16 + 1;
    if (g_focus_cursor_installs) {
        *g_focus_table_slot0 = g_focus_cursor_installs;
    }
}

int __thiscall focus_stub_map_focus(void *map_win, int x, int y) {
    if (g_focus_rec.focus_calls <
        static_cast<int>(sizeof(g_focus_rec.focus_targets) / sizeof(void *))) {
        g_focus_rec.focus_targets[g_focus_rec.focus_calls] = map_win;
    }
    ++g_focus_rec.focus_calls;
    g_focus_rec.focus_x = x;
    g_focus_rec.focus_y = y;
    g_focus_rec.sequence = g_focus_rec.sequence * 16 + 2;
    return g_focus_result;
}

void __thiscall focus_stub_draw_map(void *map_win, int draw_type) {
    ++g_focus_rec.draw_calls;
    g_focus_rec.draw_target = map_win;
    g_focus_rec.draw_type = draw_type;
    g_focus_rec.sequence = g_focus_rec.sequence * 16 + 3;
}

void __cdecl focus_stub_flush_input(void) {
    ++g_focus_rec.flush_calls;
    g_focus_rec.flush_at_focus_calls = g_focus_rec.focus_calls;
    g_focus_rec.sequence = g_focus_rec.sequence * 16 + 4;
}


// A MapWin stand-in reaching only as far as the two dwords focus reads.
constexpr size_t FocusWindowBytes = 0x1DD80;

constexpr size_t FocusFlagsOffset = 0x1DD70;

}  // namespace

void test_console_focus() {
    static_assert(MapWinTableSlots == 8,
                  "Console::focus walks exactly the eight catalogued slots");

    auto *const saved_cursor = ConsoleOriginalCursorNext;
    auto *const saved_focus = ConsoleOriginalMapWinFocus;
    auto *const saved_draw = ConsoleOriginalMapWinDrawMap;
    auto *const saved_flush = ConsoleOriginalFlushInput;
    void *const saved_global = ConsoleGlobal;
    int32_t *const saved_control = ConsoleControlTurnActive;
    MapWin **const saved_table = MapWinTable;
    int *const saved_local = LocalFaction;

    ConsoleOriginalCursorNext = &focus_stub_cursor_next;
    ConsoleOriginalMapWinFocus = &focus_stub_map_focus;
    ConsoleOriginalMapWinDrawMap = &focus_stub_draw_map;
    ConsoleOriginalFlushInput = &focus_stub_flush_input;
    int console_marker = 0;
    ConsoleGlobal = &console_marker;
    int32_t control_turn = 0;
    ConsoleControlTurnActive = &control_turn;
    int local_faction = 4;
    LocalFaction = &local_faction;

    std::vector<uint8_t> console_storage(sizeof(Console));
    auto *const console = reinterpret_cast<Console *>(console_storage.data());
    volatile uint32_t *const overlay = reinterpret_cast<volatile uint32_t *>(
        console_storage.data() + 0x23C00);

    // Four stand-in windows is enough: slot 0, an active tagged one, an active
    // untagged one, and an inactive one.
    std::vector<uint8_t> w0(FocusWindowBytes), w1(FocusWindowBytes),
        w2(FocusWindowBytes), w3(FocusWindowBytes),
        sentinel(FocusWindowBytes);
    auto as_win = [](std::vector<uint8_t> &raw) {
        return reinterpret_cast<MapWin *>(raw.data());
    };
    auto set_win = [](std::vector<uint8_t> &raw, uint32_t flags, uint32_t active) {
        std::memcpy(raw.data() + FocusFlagsOffset, &flags, sizeof(flags));
        std::memcpy(raw.data() + MapWinActiveOffset, &active, sizeof(active));
    };
    // One slot MORE than the body may walk. It holds a window that would be
    // accepted on every count, so a loop bound that runs one past the eight
    // catalogued slots focuses it and is caught.
    MapWin *table[MapWinTableSlots + 1];
    MapWinTable = table;
    g_focus_table_slot0 = reinterpret_cast<void **>(&table[0]);

    auto arrange = [&](uint32_t slot0_flags) {
        std::memset(console_storage.data(), 0, console_storage.size());
        for (size_t slot = 0; slot < MapWinTableSlots; ++slot) {
            table[slot] = nullptr;
        }
        set_win(sentinel, 0x60000000U, 1);
        table[MapWinTableSlots] = as_win(sentinel);
        // Slot 0 carries no activity dword on purpose: the body must never
        // consult one for it, and a zero there would drop the slot if it did.
        set_win(w0, slot0_flags, 0);
        table[0] = as_win(w0);
        g_focus_rec = FocusRecord{};
        g_focus_result = 0;
        g_focus_cursor_installs = nullptr;
        control_turn = 0;
    };

    // --- the mask, read through which windows a secondary slot accepts ---
    // MapWin::init publishes 0x20000000 on a net game's primary window,
    // 0x60000000 on a non-net primary, and 0x40000000 on a secondary. The test
    // is `test ecx, edx`, a bitwise AND, so a 0x60000000 window matches every
    // mask while the two single-bit windows are selective.
    struct MaskCase { int faction; uint32_t window; bool accepted; };
    const MaskCase mask_cases[] = {
        {-1, 0x20000000U, true},  {-1, 0x40000000U, true},
        {-1, 0x60000000U, true},  {-1, 0x00000000U, false},
        {4, 0x20000000U, true},   {4, 0x40000000U, false},
        {4, 0x60000000U, true},
        {5, 0x20000000U, false},  {5, 0x40000000U, true},
        {5, 0x60000000U, true},
        {INT_MIN, 0x40000000U, true},
        // Faction 0 is the one value that separates `< 0` from `<= 0`: it is
        // not negative, so it must take the comparison arm and land on
        // 0x40000000 against a local faction of 4, rejecting this window.
        {0, 0x20000000U, false},  {0, 0x40000000U, true},
    };
    for (const MaskCase &test : mask_cases) {
        arrange(0);
        // Slot 0 untagged with no control turn drops out, leaving slot 3 the
        // only window in play.
        set_win(w1, test.window, 1);
        table[3] = as_win(w1);
        g_focus_result = 1;
        expect(console->focus(11, 22, test.faction) == 0);
        expect(g_focus_rec.focus_calls == (test.accepted ? 1 : 0));
        if (test.accepted) {
            expect(g_focus_rec.focus_targets[0] == w1.data());
            expect(g_focus_rec.focus_x == 11 && g_focus_rec.focus_y == 22);
        }
        // Specialisation 4: a secondary window's success never raises the flag,
        // so flush_input never runs and the answer stays 0.
        expect(g_focus_rec.flush_calls == 0);
    }

    // --- the activity gate applies to slots 1..7 and NOT to slot 0 ---
    arrange(0x20000000U);
    set_win(w1, 0x20000000U, 0);   // tagged but inactive
    table[3] = as_win(w1);
    g_focus_result = 0;
    expect(console->focus(11, 22, 4) == 0);
    // Slot 0 ran despite its own activity dword being zero; slot 3 did not.
    expect(g_focus_rec.cursor_calls == 1);
    expect(g_focus_rec.focus_calls == 1);
    expect(g_focus_rec.focus_targets[0] == w0.data());

    // --- an empty slot is skipped before any exemption, slot 0 included ---
    arrange(0x60000000U);
    table[0] = nullptr;
    expect(console->focus(11, 22, -1) == 0);
    expect(g_focus_rec.cursor_calls == 0);
    expect(g_focus_rec.focus_calls == 0);

    // --- specialisation 2: an untagged slot 0 and the control-turn global ---
    for (int active = 0; active < 2; ++active) {
        arrange(0);
        control_turn = active ? 1 : 0;
        g_focus_result = 0;
        expect(console->focus(11, 22, 4) == 0);
        // The global is consulted only because the tag missed, and only it can
        // pull an untagged primary onto the cursor path.
        expect(g_focus_rec.cursor_calls == (active ? 1 : 0));
    }
    // A TAGGED slot 0 takes the cursor path whatever the global says.
    arrange(0x20000000U);
    control_turn = 0;
    expect(console->focus(11, 22, 4) == 0);
    expect(g_focus_rec.cursor_calls == 1);

    // --- cursor_next runs on the process-wide Console, not on `this` ---
    expect(g_focus_rec.cursor_self == &console_marker);
    expect(g_focus_rec.cursor_self != console_storage.data());
    expect(g_focus_rec.cursor_x == 11 && g_focus_rec.cursor_y == 22);

    // --- the survey-overlay latch ---
    // Set: cleared first, then slot 0 is focused. Success raises the flag and
    // flushes; failure repaints instead, with draw type 1.
    for (int moved = 0; moved < 2; ++moved) {
        arrange(0x20000000U);
        *overlay = 0x99;
        g_focus_result = moved;
        const int result = console->focus(11, 22, 4);
        expect(result == moved);
        expect(*overlay == 0);
        expect(g_focus_rec.focus_calls == 1);
        expect(g_focus_rec.focus_targets[0] == w0.data());
        expect(g_focus_rec.draw_calls == (moved ? 0 : 1));
        if (!moved) {
            expect(g_focus_rec.draw_target == w0.data());
            expect(g_focus_rec.draw_type == 1);
        }
        expect(g_focus_rec.flush_calls == (moved ? 1 : 0));
        // cursor, focus, then either flush or draw - and the flush is last,
        // after the whole walk, never inside it.
        expect(g_focus_rec.sequence == (moved ? 0x124U : 0x123U));
    }
    // Clear: the latch path is skipped entirely and slot 0 goes through the
    // same plain focus call a tagged secondary would.
    arrange(0x20000000U);
    *overlay = 0;
    g_focus_result = 1;
    expect(console->focus(11, 22, 4) == 1);
    expect(g_focus_rec.cursor_calls == 1);
    expect(g_focus_rec.draw_calls == 0);
    expect(g_focus_rec.focus_calls == 1);
    expect(g_focus_rec.flush_calls == 1);
    // The same clear latch with a primary that REFUSES. This is the one
    // arrangement in which the two slot-0 paths diverge observably: the latch
    // path repaints on refusal, the plain path does not. Without it a body
    // that took the latch arm unconditionally would pass every other case.
    arrange(0x20000000U);
    *overlay = 0;
    g_focus_result = 0;
    expect(console->focus(11, 22, 4) == 0);
    expect(g_focus_rec.focus_calls == 1);
    expect(g_focus_rec.draw_calls == 0);
    expect(g_focus_rec.flush_calls == 0);

    // --- the table is RE-READ after cursor_next ---
    // The cursor recorder republishes slot 0. A body that reused the pointer it
    // loaded at the top of the iteration would focus the OLD window.
    for (int latched = 0; latched < 2; ++latched) {
        arrange(0x20000000U);
        set_win(w2, 0x20000000U, 1);
        g_focus_cursor_installs = w2.data();
        *overlay = latched ? 0x99 : 0;
        g_focus_result = 0;
        expect(console->focus(11, 22, 4) == 0);
        expect(g_focus_rec.focus_calls == 1);
        expect(g_focus_rec.focus_targets[0] == w2.data());
        if (latched) {
            // The repaint reads slot 0 a SECOND time, after MapWin::focus
            // returned, so it sees the republished window too.
            expect(g_focus_rec.draw_target == w2.data());
        }
    }

    // --- flush_input runs once, after the whole eight-slot walk ---
    arrange(0x20000000U);
    *overlay = 0x99;
    set_win(w1, 0x40000000U, 1);
    set_win(w3, 0x40000000U, 1);
    table[3] = as_win(w1);
    table[7] = as_win(w3);
    g_focus_result = 1;
    expect(console->focus(11, 22, -1) == 1);
    expect(g_focus_rec.focus_calls == 3);
    expect(g_focus_rec.flush_calls == 1);
    // The ninth slot is never reached, whatever it holds.
    for (int visit = 0; visit < g_focus_rec.focus_calls; ++visit) {
        expect(g_focus_rec.focus_targets[visit] != sentinel.data());
    }
    // Every window had been visited before the flush, so it is not inside the
    // loop even though the flag was raised on the first iteration.
    expect(g_focus_rec.flush_at_focus_calls == 3);

    // --- through the redirect adapter ---
    arrange(0x20000000U);
    *overlay = 0x99;
    g_focus_result = 1;
    expect(console_focus_redirect(console, nullptr, 33, 44, 4) == 1);
    expect(g_focus_rec.focus_x == 33 && g_focus_rec.focus_y == 44);
    expect(g_focus_rec.flush_calls == 1);

    ConsoleOriginalCursorNext = saved_cursor;
    ConsoleOriginalMapWinFocus = saved_focus;
    ConsoleOriginalMapWinDrawMap = saved_draw;
    ConsoleOriginalFlushInput = saved_flush;
    ConsoleGlobal = saved_global;
    ConsoleControlTurnActive = saved_control;
    MapWinTable = saved_table;
    LocalFaction = saved_local;
}

void test_console_update_data() {
    static_assert(sizeof(Console) == 0x247A8,
                  "Console update_data fixture requires the legacy layout");
    static_assert(sizeof(MapWin) == 0x22480,
                  "Console update_data fixture requires the legacy MapWin layout");

    // The four defaults are the only constants this body has; every one of
    // them is rebound below, so assert them before rebinding or a wrong
    // address would be invisible to this suite.
    expect(ConsoleOriginalStatusWinRedraw
           == reinterpret_cast<func_status_win_redraw *>(0x004B9EA0));
    expect(ConsoleInfoWin == reinterpret_cast<void *>(0x007AD2A0));
    expect(ConsoleStatusWin == reinterpret_cast<void *>(0x008C5568));
    expect(ConsoleMapWinSlot == reinterpret_cast<void **>(0x007D3C3C));

    func_status_win_redraw *const saved_redraw = ConsoleOriginalStatusWinRedraw;
    void *const saved_info_win = ConsoleInfoWin;
    void *const saved_status_win = ConsoleStatusWin;
    void **const saved_map_slot = ConsoleMapWinSlot;
    func_set_date *const saved_set_date = MainInterfaceOriginalSetDate;
    void *const saved_interface = MainInterfaceGlobal;
    char *const saved_caption = MapWinMainCaption;

    char caption_a[] = "2101.01";
    char caption_b[] = "2189.12";
    int interface_a = 0;
    int interface_b = 0;

    for (int shape = 0; shape < 2; ++shape) {
        // Two genuinely different shapes: different argument, different probe
        // bodies behind both seams, different fixed-window objects, a
        // different MapWin slot cell, a different MainInterface and caption.
        const int argument = (shape == 0) ? 0 : -0x7FFFFFFF - 1;  // 0, INT_MIN
        ConsoleOriginalStatusWinRedraw = (shape == 0)
            ? &console_update_redraw_probe_a
            : &console_update_redraw_probe_b;
        MainInterfaceOriginalSetDate = (shape == 0)
            ? &console_update_set_date_probe_a
            : &console_update_set_date_probe_b;
        MainInterfaceGlobal = (shape == 0) ? static_cast<void *>(&interface_a)
                                           : static_cast<void *>(&interface_b);
        MapWinMainCaption = (shape == 0) ? caption_a : caption_b;

        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            // 16 bytes of leading and trailing canary around each of the four
            // objects the dispatch can reach.
            std::vector<uint8_t> console_storage(sizeof(Console) + 32);
            std::vector<uint8_t> info_storage(sizeof(InfoWin) + 32);
            std::vector<uint8_t> status_storage(sizeof(StatusWin) + 32);
            std::vector<uint8_t> map_storage(sizeof(MapWin) + 32);
            std::vector<uint8_t> console_seed(console_storage.size());
            std::vector<uint8_t> info_seed(info_storage.size());
            std::vector<uint8_t> status_seed(status_storage.size());
            std::vector<uint8_t> map_seed(map_storage.size());
            seed_storage(console_storage.data(), console_seed.data(),
                         console_storage.size());
            seed_storage(info_storage.data(), info_seed.data(),
                         info_storage.size());
            seed_storage(status_storage.data(), status_seed.data(),
                         status_storage.size());
            seed_storage(map_storage.data(), map_seed.data(),
                         map_storage.size());

            // Byte-identical twins. seed_storage() is a pure function of the
            // index, so each twin starts equal to its primary byte for byte.
            std::vector<uint8_t> console_twin(console_storage.size());
            std::vector<uint8_t> info_twin(info_storage.size());
            std::vector<uint8_t> status_twin(status_storage.size());
            std::vector<uint8_t> map_twin(map_storage.size());
            // Sized off the largest buffer: seed_storage() writes size bytes
            // into BOTH arguments, so a short scratch would overrun.
            std::vector<uint8_t> scratch(console_storage.size());
            seed_storage(console_twin.data(), scratch.data(),
                         console_twin.size());
            seed_storage(info_twin.data(), scratch.data(), info_twin.size());
            seed_storage(status_twin.data(), scratch.data(),
                         status_twin.size());
            seed_storage(map_twin.data(), scratch.data(), map_twin.size());
            expect(std::memcmp(console_twin.data(), console_seed.data(),
                               console_twin.size()) == 0);
            expect(std::memcmp(map_twin.data(), map_seed.data(),
                               map_twin.size()) == 0);

            // ---- Reference pass: the component chain on the twins.
            void *twin_slot_value = map_twin.data() + 16;
            ConsoleInfoWin = info_twin.data() + 16;
            ConsoleStatusWin = status_twin.data() + 16;
            ConsoleMapWinSlot = &twin_slot_value;

            console_update_reset_log();
            reinterpret_cast<InfoWin *>(ConsoleInfoWin)->change(argument);
            ConsoleOriginalStatusWinRedraw(ConsoleStatusWin);
            reinterpret_cast<MapWin *>(*ConsoleMapWinSlot)->main_caption();

            ConsoleUpdateEvent
                reference_events[ARRAYSIZE(console_update_events)];
            std::memcpy(reference_events, console_update_events,
                        sizeof(reference_events));
            const int reference_count = console_update_event_count;

            // The chain is derived, not assumed, to be a byte-level no-op:
            // the twins still match their own seeds, which is what makes
            // comparing the primaries against the twins meaningful.
            console_update_expect_image("update_data reference InfoWin",
                                        info_twin.data(), info_seed.data(),
                                        info_twin.size());
            console_update_expect_image("update_data reference StatusWin",
                                        status_twin.data(), status_seed.data(),
                                        status_twin.size());
            console_update_expect_image("update_data reference MapWin",
                                        map_twin.data(), map_seed.data(),
                                        map_twin.size());
            expect(twin_slot_value == map_twin.data() + 16);

            // ---- Function under test, on the primaries.
            void *slot_value = map_storage.data() + 16;
            ConsoleInfoWin = info_storage.data() + 16;
            ConsoleStatusWin = status_storage.data() + 16;
            ConsoleMapWinSlot = &slot_value;

            auto *const console =
                reinterpret_cast<Console *>(console_storage.data() + 16);
            console_update_reset_log();
            if (use_adapter) {
                console_update_data_redirect(console, nullptr, argument);
            } else {
                console->update_data(argument);
            }

            // Exactly the two dependency calls the chain made, in order,
            // through the probe bodies this shape installed.
            expect(console_update_event_count == reference_count);
            expect(console_update_event_count == 2);
            for (int index = 0; index < 2; ++index) {
                expect(console_update_events[index].kind
                       == reference_events[index].kind);
                expect(console_update_events[index].probe
                       == reference_events[index].probe);
                expect(console_update_events[index].probe == shape);
            }
            // StatusWin::redraw first, on the fixed StatusWin - not on the
            // Console it is called through, the InfoWin, or the MapWin.
            expect(console_update_events[0].kind == 1);
            expect(console_update_events[0].target == ConsoleStatusWin);
            expect(console_update_events[0].target
                   != static_cast<const void *>(console_storage.data() + 16));
            expect(console_update_events[0].target != ConsoleInfoWin);
            expect(console_update_events[0].target
                   != static_cast<const void *>(map_storage.data() + 16));
            expect(console_update_events[0].text == nullptr);
            // Then MapWin::main_caption, whose recovered body reaches the
            // fixed MainInterface with the fixed caption.
            expect(console_update_events[1].kind == 2);
            expect(console_update_events[1].target == MainInterfaceGlobal);
            expect(console_update_events[1].target
                   != static_cast<const void *>(map_storage.data() + 16));
            expect(console_update_events[1].text == MapWinMainCaption);
            expect(console_update_events[1].text
                   == ((shape == 0) ? caption_a : caption_b));
            // Distinct targets, so a body that passed the same object to both
            // dependencies cannot pass.
            expect(console_update_events[0].target
                   != console_update_events[1].target);

            // Byte-exact compositional images: every primary must equal the
            // twin the component chain ran on, canaries included.
            console_update_expect_image("update_data Console",
                                        console_storage.data(),
                                        console_twin.data(),
                                        console_storage.size());
            console_update_expect_image("update_data InfoWin",
                                        info_storage.data(), info_twin.data(),
                                        info_storage.size());
            console_update_expect_image("update_data StatusWin",
                                        status_storage.data(),
                                        status_twin.data(),
                                        status_storage.size());
            console_update_expect_image("update_data MapWin",
                                        map_storage.data(), map_twin.data(),
                                        map_storage.size());
            // And, independently of the twins, still equal to their own
            // seeds - the dispatch stores nothing anywhere.
            console_update_expect_image("update_data Console seed",
                                        console_storage.data(),
                                        console_seed.data(),
                                        console_storage.size());
            console_update_expect_image("update_data MapWin seed",
                                        map_storage.data(), map_seed.data(),
                                        map_storage.size());

            // The MapWin slot is read, never written.
            expect(slot_value == map_storage.data() + 16);
            // The bindings themselves are untouched by the call.
            expect(ConsoleInfoWin
                   == static_cast<void *>(info_storage.data() + 16));
            expect(ConsoleStatusWin
                   == static_cast<void *>(status_storage.data() + 16));
            expect(ConsoleMapWinSlot == &slot_value);
        }
    }

    ConsoleOriginalStatusWinRedraw = saved_redraw;
    ConsoleInfoWin = saved_info_win;
    ConsoleStatusWin = saved_status_win;
    ConsoleMapWinSlot = saved_map_slot;
    MainInterfaceOriginalSetDate = saved_set_date;
    MainInterfaceGlobal = saved_interface;
    MapWinMainCaption = saved_caption;
    console_update_reset_log();

    // Restored exactly, so no later test inherits this fixture's bindings.
    expect(ConsoleOriginalStatusWinRedraw
           == reinterpret_cast<func_status_win_redraw *>(0x004B9EA0));
    expect(ConsoleInfoWin == reinterpret_cast<void *>(0x007AD2A0));
    expect(ConsoleStatusWin == reinterpret_cast<void *>(0x008C5568));
    expect(ConsoleMapWinSlot == reinterpret_cast<void **>(0x007D3C3C));
}

// Self-registration.  Each case carries its baseline position in the original
// main() call list, so the run order does not depend on link or static-init
// order.
namespace {
LEAF_CASE(188, test_datalink_help_forwarders);
LEAF_CASE(189, test_net_daemon_synch_forwarders);
LEAF_CASE(190, test_net_daemon_unlock_veh);
LEAF_CASE(191, test_x_pop_forwarders);
LEAF_CASE(192, test_x_pops_forwarders);
LEAF_CASE(193, test_caviar_data_close);
LEAF_CASE(194, test_texture_dtor);
LEAF_CASE(195, test_pop_pops_forwarders);
LEAF_CASE(196, test_atexit_teardown_thunks);
LEAF_CASE(197, test_battle_win_dtor);
LEAF_CASE(198, test_fx_and_font_queue_dtors);
LEAF_CASE(220, test_console_focus);
LEAF_CASE(219, test_console_update_data);
}  // namespace
