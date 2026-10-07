#include "stdafx.h"
#include "word_state.h"

/*
Purpose: Set +0 to 0x24, clear +4, and set bit zero at +8 while returning the
         original receiver pointer. The sequence is retained in source order.

         Original instruction record: mov eax,ecx / mov ecx,[eax+8] /
         mov dword [eax+4],0 / or ecx,1 / mov dword [eax],0x24 /
         mov [eax+8],ecx / ret
// RECOVERY_ROLE: SCAFFOLD Provisional bounded receiver-state method; original class, complete extent, callers, producer and lifetime remain unknown.
// ORIGINAL: 0x004C8070 sub_4c8070 0x004C8070-0x004C8089 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Express the pinned receiver-shaped word operation as a member of a distinct bounded source-state type without claiming its original class or lifetime.","evidence":["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x004C8070-0x004C8089 reads +8, clears +4, writes +0=0x24, stores the modified +8 word, and returns the receiver","idb:exact pinned IDB function boundary 0x004C8070-0x004C8089; no type/owner record","proposal:neutral member name denotes only the observed bounded word operations; no original class/extent/caller/lifetime assertion"]}
// symbol    ?initialize@PrefixFlagState@@QAEPAU1@XZ
// size      25 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: receiver pointer
Status: Complete
*/
PrefixFlagState *PrefixFlagState::initialize() {
    uint32_t saved_word_08 = word_08_;
    word_04_ = 0;
    saved_word_08 |= 1U;
    word_00_ = 0x24U;
    word_08_ = saved_word_08;
    return this;
}

/*
Purpose: Zero the observed words at +0 through +1C and set the word at +20
         to all ones, then return the original receiver pointer.

         Original instruction record: mov eax,ecx / xor ecx,ecx /
         store zero at +4,+8,+0C,+10,+0,+14,+18,+1C /
         store 0xFFFFFFFF at +20 / ret

         Historical seed provenance: this identity was previously rendered
         as an address-named void-pointer redirect because its class was not
         established. The current member type remains a bounded source model,
         not a claim about that original class.
// RECOVERY_ROLE: SCAFFOLD Provisional bounded receiver-state method; original class, complete extent, callers, producer and lifetime remain unknown.
// ORIGINAL: 0x004C8090 sub_4c8090 0x004C8090-0x004C80B3 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Express the pinned nine-word reset as a member of a distinct bounded source-state type without relating it to either nearby leaf type.","evidence":["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x004C8090-0x004C80B3 writes eight zero dwords and then 0xFFFFFFFF at +0x20, returning the receiver","idb:exact pinned image has no xref-tag entries or class/type/lifetime record at this address","proposal:bounded type describes only touched words; no original class/extent/caller/lifetime assertion"]}
// symbol    ?reset@NineWordSentinelState@@QAEPAU1@XZ
// size      35 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: receiver pointer
Status: Complete
*/
NineWordSentinelState *NineWordSentinelState::reset() {
    word_04_ = 0;
    word_08_ = 0;
    word_0C_ = 0;
    word_10_ = 0;
    word_00_ = 0;
    word_14_ = 0;
    word_18_ = 0;
    word_1C_ = 0;
    word_20_ = 0xFFFFFFFFU;
    return this;
}

/*
Purpose: Clear the observed words at +4 and +8, preserve +0, and return the
         original receiver pointer.

         Original instruction record: mov eax,ecx / xor ecx,ecx /
         store zero at +4 / store zero at +8 / ret

         Historical seed provenance: this identity was previously rendered
         as an address-named void-pointer redirect because its class was not
         established. The current member type remains a bounded source model,
         not a claim about that original class.
// RECOVERY_ROLE: SCAFFOLD Provisional bounded receiver-state method; original class, complete extent, callers, producer and lifetime remain unknown.
// ORIGINAL: 0x004C8100 sub_4c8100 0x004C8100-0x004C810B BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Express the pinned two-word clear as a member of a third, distinct bounded source-state type without joining it to either nearby type.","evidence":["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x004C8100-0x004C810B clears +4 and +8, leaves +0 alone, and returns the receiver","idb:exact pinned IDB has no type/owner evidence for this address","proposal:bounded type names only the three-word test prefix; no original class/extent/caller/lifetime assertion"]}
// symbol    ?reset_tail@PairWordState@@QAEPAU1@XZ
// size      11 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: receiver pointer
Status: Complete
*/
PairWordState *PairWordState::reset_tail() {
    word_04_ = 0;
    word_08_ = 0;
    return this;
}
