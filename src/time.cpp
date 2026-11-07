#include "stdafx.h"

#include "time.h"
#include "win.h"         // do_all_draws/do_non_input, clock_wait's pump

// These are behavior views over observed fields, not names or complete
// allocation claims for either original class. No connection between their
// construction/use/lifetime is inferred from the nearby addresses.
class RangePrefixInitializationView {
public:
    RangePrefixInitializationView();
    unsigned int unknown_dword_0_;
    unsigned int zeroed_dword_4_;
    unsigned int unknown_dword_8_;
    unsigned int unknown_dword_C_;
};

// RECOVERY_ROLE: SCAFFOLD Provisional four-word initialization view; original containing class, complete extent and constructor-to-instance lifetime flow remain unproved. Matching the endpoint views' offsets is not a receiver ownership link.
// ORIGINAL: 0x004C8110 sub_4c8110 0x004C8110-0x004C813B BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x004C8110-0x004C813B: complete thiscall constructor first zeroes four bytes [this+4] through memset, then stores dword [this], dword [this+8], and dword [this+0xC] in this order; [this+4] is zeroed by the actual four-byte memset call at 0x004C811B to 0x006465F0", "git:aee589a5dd0fca997773070668af4e1ff9026684:src/recovered/004c8110.cpp: historical-only seed hint // symbol ??0LegacyBlock@@QAE@XZ; this deleted seed-file spelling is not emitted as the current symbol and is not independently established as the original class", "pinned scan: no direct caller edge or absolute-address data reference to 4C8110 was found"], "reason": "Cover only the constructor's concrete four-dword initialization through a typed source view. Preserve its original span and claim; keep field roles, complete original class name, allocation extent and runtime lifetime unknown. The view is not merged with the separate time-range receiver."}
// symbol    ??0RangePrefixInitializationView@@QAE@XZ
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma function(memset)
#endif
RangePrefixInitializationView::RangePrefixInitializationView() {
    memset(&zeroed_dword_4_, 0, sizeof(zeroed_dword_4_));
    unknown_dword_0_ = 0;
    unknown_dword_8_ = 0;
    unknown_dword_C_ = 0;
}

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x004C8140 nullsub_223 0x004C8140-0x004C8141 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x004C8140-0x004C8141 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x004C8110 sub_4c8110 (src/time.cpp) and 0x004C8150 sub_4c8150 (src/stringstruct.cpp); band 0x004C8070..0x004C8140 holds the time.cpp/word_state.cpp leaf helpers; owner inferred from the immediately preceding function"]}
// symbol    ?nullsub_223@@YAXXZ
// kind      game
void __cdecl nullsub_223() {
}
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma intrinsic(memset)
#endif

class TimeRangeReceiverView {
public:
    // Endpoint operations and sample share only the observed fields;
    // original class and lifetime ownership are not established.
    void set_lower_endpoint(int value);
    void set_upper_endpoint(int value);
    int sample();
private:
    unsigned char unobserved_prefix_0_[4];
    unsigned char enabled_flags_4_;
    unsigned char unobserved_bytes_5_7_[3];
    int lower_bound_8_;
    int upper_bound_C_;
};


// The paired operations update the two bounds in their original order. Their
// unsigned comparisons are pinned behavior, not a claim about endpoint meaning.
/*
 * RECOVERY_HISTORY: v1 address=0x004C80C0 source_sha256=e7103f386f51cc4338c932611b975da2e3655a90005600cdac86173b07d8c0ee envelope_sha256=e0d6eb0a4d32e64334cc1f4cded43e7d701b510f32d7a1a14fb4d75f7ad5a11f
 * [ARCHIVE] /*
 * [ARCHIVE] Purpose: Clamp field 0xc against an argument, then store unconditionally.
 * [ARCHIVE]          Emitted by tools/generate_field_accessors.py from
 * [ARCHIVE]
 * [ARCHIVE]              push ebp / mov ebp, esp / mov eax, dword ptr [ebp + 8] / mov edx, dword ptr [ecx + 0xc] / cmp edx, eax / jae 0x4c80d0 / mov dword ptr [ecx + 0xc], eax / mov dword ptr [ecx + 8], eax / pop ebp / ret 4
 * [ARCHIVE]
 * [ARCHIVE]          The name is the ADDRESS, not an invention, and `self` is a void
 * [ARCHIVE]          pointer because this body needs `this` to be nothing more than a
 * [ARCHIVE]          pointer and an offset - the class it belongs to is not established.
 * [ARCHIVE]          The adapter declares 1 stack dword(s) so it
 * [ARCHIVE]          cleans 4 bytes, taken from the body's own `ret` -
 * [ARCHIVE]          the only statement of this function's arity that exists, since it has
 * [ARCHIVE]          no mangled name. Declaring fewer would leave them on the caller's
 * [ARCHIVE]          stack.
 * [ARCHIVE] // RECOVERY_ROLE: SCAFFOLD Untyped raw-field or constant adapter; real method ownership and named member relationships remain recovery obligations.
 * [ARCHIVE] // original marker: 0x004C80C0 sub_4c80c0 0x004C80C0-0x004C80D7 BYTE_EXACT
 * [ARCHIVE] // symbol    ?field_accessor_004c80c0_redirect@@YIXPAX0H@Z
 * [ARCHIVE] // size      23 bytes
 * [ARCHIVE] // prototype
 * [ARCHIVE] // callers   0   call targets   0
 * [ARCHIVE] // kind      game
 * [ARCHIVE] // flags     frame;hidden;sp_ready;purged_ok
 * [ARCHIVE] // calls     (none)
 * [ARCHIVE] Return Value: n/a
 * [ARCHIVE] Status: Complete
 * [ARCHIVE] * /
 */
// RECOVERY_ROLE: SCAFFOLD Provisional TimeRangeReceiverView member/layout: pinned endpoint operations write the same +8/+C fields read by mapped 0x004C8350, supporting one observed-state API. The original complete class identity, object extent, construction, caller set, and lifetime remain unproved.
// ORIGINAL: 0x004C80C0 sub_4c80c0 0x004C80C0-0x004C80D7 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x004C80C0-0x004C80D7: ECX receiver, one stack dword, unsigned endpoint compare and ordered stores at +8/+C", "source:src/time.cpp mapped 0x004C8350 reads the same +8/+C signed bounds and byte +4 flag through TimeRangeReceiverView", "pinned-analysis limitation: no caller, constructor, allocation extent, or runtime lifetime witness is available for these endpoints"], "reason": "Place the paired endpoint operation in the same provisional observed-field view as 0x004C8350 because the pinned endpoint stores and mapped sample reads address the same +8/+C fields, while the sample also reads the +4 flag. This supports shared field API closure, not original complete class identity, object extent, constructor, caller set, or lifetime. Preserve the pinned unsigned comparisons and store order."}
// symbol    ?set_lower_endpoint@TimeRangeReceiverView@@QAEXH@Z
// kind      game
void TimeRangeReceiverView::set_lower_endpoint(int value) {
    if (static_cast<DWORD>(upper_bound_C_) < static_cast<DWORD>(value)) {
        upper_bound_C_ = value;
    }
    lower_bound_8_ = value;
}

/*
 * RECOVERY_HISTORY: v1 address=0x004C80E0 source_sha256=e7103f386f51cc4338c932611b975da2e3655a90005600cdac86173b07d8c0ee envelope_sha256=8c15061494d8d9b690e02e41f5aa7054bafa4a713979ab8f6841a65deb438b95
 * [ARCHIVE] /*
 * [ARCHIVE] Purpose: Clamp field 0x8 against an argument, then store unconditionally.
 * [ARCHIVE]          Emitted by tools/generate_field_accessors.py from
 * [ARCHIVE]
 * [ARCHIVE]              push ebp / mov ebp, esp / mov eax, dword ptr [ebp + 8] / mov edx, dword ptr [ecx + 8] / cmp edx, eax / jbe 0x4c80f0 / mov dword ptr [ecx + 8], eax / mov dword ptr [ecx + 0xc], eax / pop ebp / ret 4
 * [ARCHIVE]
 * [ARCHIVE]          The name is the ADDRESS, not an invention, and `self` is a void
 * [ARCHIVE]          pointer because this body needs `this` to be nothing more than a
 * [ARCHIVE]          pointer and an offset - the class it belongs to is not established.
 * [ARCHIVE]          The adapter declares 1 stack dword(s) so it
 * [ARCHIVE]          cleans 4 bytes, taken from the body's own `ret` -
 * [ARCHIVE]          the only statement of this function's arity that exists, since it has
 * [ARCHIVE]          no mangled name. Declaring fewer would leave them on the caller's
 * [ARCHIVE]          stack.
 * [ARCHIVE] // RECOVERY_ROLE: SCAFFOLD Untyped raw-field or constant adapter; real method ownership and named member relationships remain recovery obligations.
 * [ARCHIVE] // original marker: 0x004C80E0 sub_4c80e0 0x004C80E0-0x004C80F7 BYTE_EXACT
 * [ARCHIVE] // symbol    ?field_accessor_004c80e0_redirect@@YIXPAX0H@Z
 * [ARCHIVE] // size      23 bytes
 * [ARCHIVE] // prototype
 * [ARCHIVE] // callers   0   call targets   0
 * [ARCHIVE] // kind      game
 * [ARCHIVE] // flags     frame;hidden;sp_ready;purged_ok
 * [ARCHIVE] // calls     (none)
 * [ARCHIVE] Return Value: n/a
 * [ARCHIVE] Status: Complete
 * [ARCHIVE] * /
 */
// RECOVERY_ROLE: SCAFFOLD Provisional TimeRangeReceiverView member/layout: pinned endpoint operations write the same +8/+C fields read by mapped 0x004C8350, supporting one observed-state API. The original complete class identity, object extent, construction, caller set, and lifetime remain unproved.
// ORIGINAL: 0x004C80E0 sub_4c80e0 0x004C80E0-0x004C80F7 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x004C80E0-0x004C80F7: ECX receiver, one stack dword, unsigned endpoint compare and ordered stores at +8/+C", "source:src/time.cpp mapped 0x004C8350 reads the same +8/+C signed bounds and byte +4 flag through TimeRangeReceiverView", "pinned-analysis limitation: no caller, constructor, allocation extent, or runtime lifetime witness is available for these endpoints"], "reason": "Place the paired endpoint operation in the same provisional observed-field view as 0x004C8350 because the pinned endpoint stores and mapped sample reads address the same +8/+C fields, while the sample also reads the +4 flag. This supports shared field API closure, not original complete class identity, object extent, constructor, caller set, or lifetime. Preserve the pinned unsigned comparisons and store order."}
// symbol    ?set_upper_endpoint@TimeRangeReceiverView@@QAEXH@Z
// kind      game
void TimeRangeReceiverView::set_upper_endpoint(int value) {
    if (static_cast<DWORD>(lower_bound_8_) > static_cast<DWORD>(value)) {
        lower_bound_8_ = value;
    }
    upper_bound_C_ = value;
}

// Historical TRIED: the seed candidate calculated range before the clock call;
// the compiler scheduled the call earlier, retaining both endpoints and using
// another callee-saved register. A volatile range instead spilled through a
// frame. Those were compiler-shape findings, not evidence for a source class.
// The original adds the unsigned remainder to the reloaded upper endpoint.
// memcpy retains x86 32-bit wrap without C++ signed-overflow undefined behavior.
static int range_view_wrapped_upper_add(int upper, DWORD remainder) {
    const DWORD bits = static_cast<DWORD>(upper) + remainder;
    int result;
    memcpy(&result, &bits, sizeof(result));
    return result;
}

// Historical seed called this an unknown Subject::roll and kept an IAT alias;
// that is not pinned class/API evidence. The actual source uses the SDK clock.
// RECOVERY_ROLE: SCAFFOLD Provisional TimeRangeReceiverView member/layout: pinned endpoint operations write the same +8/+C fields read by mapped 0x004C8350, supporting one observed-state API. The original complete class identity, object extent, construction, caller set, and lifetime remain unproved.
// ORIGINAL: 0x004C8350 sub_4c8350 0x004C8350-0x004C8396 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x004C8350-0x004C8396: thiscall reads byte[this+4]&1; signed-compares dwords+8/+C; forms a wrapped unsigned inclusive difference before timeGetTime; divides unsigned; reloads +C after the call and adds the remainder", "pinned calls at 0x004C836D and 0x004C8384 target PE import slot 0x00669368, which uniquely resolves to WINMM timeGetTime; the normal SDK API is used, not an executable-address alias", "promoted historical seed src/recovered/units/004c8350.cpp retained the complete70-byte span and MISMATCH state; prior range-before-clock source-shape observation is preserved here for later restoration", "pinned scan: no direct caller edge or absolute-address data reference to 4C8350 was found", "source:src/time.cpp provisional endpoint member operations at 0x004C80C0/0x004C80E0 update the same +8/+C fields read by sample"], "reason": "Implement the observed time-based range read through a provisional receiver view, paired with 0x004C80C0/0x004C80E0 because those endpoint operations write the same +8/+C fields this body reads. Keep the separate RangePrefixInitializationView at 0x004C8110 unmerged. Preserve signed branch comparison, unsigned 32-bit range wrap, zero-divisor failure, the post-clock upper-field reload, and wrapped return addition. This relation does not prove original complete class identity, object extent, constructor, callers, or lifetime."}
// symbol    ?sample@TimeRangeReceiverView@@QAEHXZ
int TimeRangeReceiverView::sample() {
    if ((enabled_flags_4_ & 1) == 0) {
        return 0;
    }

    const int lower = lower_bound_8_;
    const int upper = upper_bound_C_;
    DWORD range;
    if (lower > upper) {
        range = static_cast<DWORD>(lower) - static_cast<DWORD>(upper) + 1u;
    } else {
        range = static_cast<DWORD>(upper) - static_cast<DWORD>(lower) + 1u;
    }
    const DWORD tick = timeGetTime();
    const DWORD remainder = tick % range;
    return range_view_wrapped_upper_add(upper_bound_C_, remainder);
}

// Pump redraws before non-input work at least once. The original captures the
// WINMM timeGetTime import once and compares unsigned elapsed time, including
// wraparound and negative arguments interpreted as unsigned durations.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0050DD90 ?clock_wait@@YAXH@Z 0x0050DD90-0x0050DDBA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/0050dd90.cpp retained exact seed and original extent; preserved without copying historical claim", "Pinned PE import669368 resolves WINMM.dll timeGetTime; full disassembly50DD90..50DDBA captures import once, calls5FCBB0 then5FCA30 and uses unsigned jb elapsed compare"], "reason": "Home cooperative unsigned elapsed-time wait to timer integration; preserve import capture, redraw/non-input ordering, mandatory first iteration and modular DWORD subtraction, without fixed import address."}
void __cdecl clock_wait(int milliseconds) {
    DWORD (WINAPI *const read_clock)() = timeGetTime;
    const DWORD started = read_clock();
    do {
        do_all_draws();
        do_non_input();
    } while (read_clock() - started < static_cast<DWORD>(milliseconds));
}
