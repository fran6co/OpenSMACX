// Recovery leaf tests: locks.
//
// Carved verbatim out of tests/recovery_leaf_tests.cpp.  Add a case by
// appending a `void test_*()` here plus `LEAF_CASE(LEAF_APPEND, test_*);` at
// the bottom.  This file is the only file that edit touches: the per-family
// case counts are derived from these `LEAF_CASE(` lines at build time, and
// LEAF_APPEND needs no offset because ties there break by name.
#include "leaf_shared.h"

#define LEAF_FAMILY "locks"

void test_lock_any_locks() {
    // any_locks: the 0xE0 dword forces a yes; otherwise records 1..7 are
    // scanned, but only those whose slot bit is set in both the low and second
    // bytes of the enable mask, and only entry flags with the low bit set
    // count. Record 0 is never scanned. Each of those conditions is exercised.
    uint32_t mask = 0;
    uint32_t *const saved_mask = LockEnableMask;
    LockEnableMask = &mask;

    std::vector<uint8_t> storage(sizeof(Lock) + 32);
    auto *lock = reinterpret_cast<Lock *>(storage.data() + 16);
    uint8_t *const obj = storage.data() + 16;
    auto set_field_e0 = [&](uint32_t v) { std::memcpy(obj + 0xE0, &v, 4); };
    auto set_flag = [&](int record, int entry, int32_t v) {
        std::memcpy(obj + record * 0x1C + 0xC + entry * 0xC, &v, sizeof(v));
    };

    // Shortcut: field at 0xE0 non-zero returns 1 regardless of the rest.
    seed_storage(storage.data(), nullptr, 0);
    std::memset(obj, 0, sizeof(Lock));
    set_field_e0(0x99);
    expect(lock->any_locks() == 1);

    // 0xE0 clear, mask 0: nothing scanned.
    std::memset(obj, 0, sizeof(Lock));
    mask = 0;
    // Even with an engaged entry, no bit set means no scan.
    set_flag(2, 0, 1);
    expect(lock->any_locks() == 0);

    // Bit 2 set in both mask bytes, record 2 entry engaged: yes.
    mask = 0x0404;
    expect(lock->any_locks() == 1);
    // The other entry of the same record also counts.
    set_flag(2, 0, 0);
    set_flag(2, 1, 1);
    expect(lock->any_locks() == 1);
    // Entry flag with the low bit clear does not count.
    set_flag(2, 1, 2);
    expect(lock->any_locks() == 0);

    // Bit set only in the low byte, not the second: record not scanned.
    std::memset(obj, 0, sizeof(Lock));
    mask = 0x0004;
    set_flag(2, 0, 1);
    expect(lock->any_locks() == 0);

    // Record 0 is never scanned, even with its bit set in both bytes.
    std::memset(obj, 0, sizeof(Lock));
    mask = 0x0101;
    set_flag(0, 0, 1);
    expect(lock->any_locks() == 0);
    expect(lock_any_locks_redirect(lock, nullptr) == 0);

    LockEnableMask = saved_mask;
}

namespace {

void *g_sq_unlock_entries[4];

int g_sq_unlock_slots[4];

int g_sq_unlock_calls;

void __thiscall observe_square_unlock(void *entry, int slot) {
    if (g_sq_unlock_calls < 4) {
        g_sq_unlock_entries[g_sq_unlock_calls] = entry;
        g_sq_unlock_slots[g_sq_unlock_calls] = slot;
    }
    ++g_sq_unlock_calls;
}

}  // namespace

void test_lock_unlock() {
    // unlock(slot) forwards both of record[slot]'s square entries to
    // SquareLock::unlock, clears the record's flag byte, and forgets the slot
    // as the active lock only when it was. The two entries are at record+4 and
    // record+0x10, which the test confirms the seam receives.
    auto *const saved = LockSquareUnlock;
    LockSquareUnlock = &observe_square_unlock;

    std::vector<uint8_t> storage(sizeof(Lock) + 32);
    auto *lock = reinterpret_cast<Lock *>(storage.data() + 16);
    uint8_t *const obj = storage.data() + 16;
    auto read32 = [&](size_t off) {
        int32_t v = 0; std::memcpy(&v, obj + off, sizeof(v)); return v;
    };

    // Active lock is slot 3: unlocking it clears the 0xE0/0xE4 fields.
    std::memset(obj, 0xAB, sizeof(Lock));
    int32_t three = 3;
    std::memcpy(obj + 0xE0, &three, 4);
    g_sq_unlock_calls = 0;
    lock->unlock(3);
    expect(read32(0xE0) == 0);
    expect(read32(0xE4) == 0);
    // Both entries of record 3 forwarded, at record+4 and record+0x10.
    expect(g_sq_unlock_calls == 2);
    expect(g_sq_unlock_entries[0] == obj + 3 * 0x1C + 4);
    expect(g_sq_unlock_entries[1] == obj + 3 * 0x1C + 0x10);
    expect(g_sq_unlock_slots[0] == 3 && g_sq_unlock_slots[1] == 3);
    // The record's flag byte is cleared.
    expect((obj[3 * 0x1C] & 0xFF) == 0);

    // Unlocking a different slot than the active one leaves 0xE0 alone but
    // still unlocks that slot's entries.
    std::memset(obj, 0, sizeof(Lock));
    int32_t five = 5;
    std::memcpy(obj + 0xE0, &five, 4);
    g_sq_unlock_calls = 0;
    lock->unlock(2);
    expect(read32(0xE0) == 5);          // still the active lock
    expect(g_sq_unlock_calls == 2);
    expect(g_sq_unlock_entries[0] == obj + 2 * 0x1C + 4);

    g_sq_unlock_calls = 0;
    lock_unlock_redirect(lock, nullptr, 1);
    expect(g_sq_unlock_calls == 2);
    expect(g_sq_unlock_entries[0] == obj + 1 * 0x1C + 4);

    LockSquareUnlock = saved;
}

void test_lock_global_lock() {
    // A try-lock on the 0xE0/0xE4 owner fields: free or already-mine succeeds
    // and records the owner, another owner fails without change. All three
    // cases are checked, including that a failed attempt leaves the fields
    // untouched.
    std::vector<uint8_t> storage(sizeof(Lock) + 32);
    std::vector<uint8_t> expected(storage.size());
    auto *lock = reinterpret_cast<Lock *>(storage.data() + 16);
    uint8_t *const obj = storage.data() + 16;
    auto read32 = [&](size_t off) {
        int32_t v = 0; std::memcpy(&v, obj + off, sizeof(v)); return v;
    };
    auto set_owner = [&](int32_t v) { std::memcpy(obj + 0xE0, &v, 4); };

    // Free (0): succeeds, records owner 3 and marks held.
    seed_storage(storage.data(), expected.data(), storage.size());
    set_owner(0);
    expect(lock->global_lock(3) == 0);
    expect(read32(0xE0) == 3);
    expect(read32(0xE4) == 1);

    // Already held by 3: taking it again as 3 still succeeds.
    expect(lock->global_lock(3) == 0);
    expect(read32(0xE0) == 3);
    expect(read32(0xE4) == 1);

    // Held by 5, another owner 3 fails and changes nothing.
    set_owner(5);
    int32_t held_e4 = 0x77;
    std::memcpy(obj + 0xE4, &held_e4, 4);
    std::memcpy(expected.data(), storage.data(), storage.size());
    expect(lock->global_lock(3) == 1);
    expect(read32(0xE0) == 5);          // unchanged
    expect(read32(0xE4) == 0x77);       // unchanged
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Redirect takes the free lock.
    set_owner(0);
    expect(lock_global_lock_redirect(lock, nullptr, 9) == 0);
    expect(read32(0xE0) == 9);
}

namespace {

int g_current_server_result = 1;

int g_current_server_calls = 0;

int __cdecl observe_current_server() {
    ++g_current_server_calls;
    return g_current_server_result;
}

}  // namespace

void test_lock_check_global_2() {
    // Reports whether the owner's global lock can be released: only on the
    // server, only when held by that owner, and only when no slot record still
    // has an active square. Every gate is exercised, and success is confirmed
    // to clear the held flag while the failing paths leave it set.
    auto *const saved = LockCurrentServer;
    LockCurrentServer = &observe_current_server;

    std::vector<uint8_t> storage(sizeof(Lock) + 32);
    auto *lock = reinterpret_cast<Lock *>(storage.data() + 16);
    uint8_t *const obj = storage.data() + 16;
    auto set32 = [&](size_t off, int32_t v) { std::memcpy(obj + off, &v, 4); };
    auto read32 = [&](size_t off) {
        int32_t v = 0; std::memcpy(&v, obj + off, sizeof(v)); return v;
    };
    auto set_flag = [&](int record, int entry, int32_t v) {
        std::memcpy(obj + record * 0x1C + 0xC + entry * 0xC, &v, sizeof(v));
    };

    // Not the server: no regardless of the rest.
    std::memset(obj, 0, sizeof(Lock));
    set32(0xE4, 1); set32(0xE0, 3);
    g_current_server_result = 0;
    expect(lock->check_global_2(3) == 0);
    expect(read32(0xE4) == 1);          // held flag untouched
    g_current_server_result = 1;

    // Not marked held: no.
    set32(0xE4, 0);
    expect(lock->check_global_2(3) == 0);

    // Held, but by a different owner: no.
    set32(0xE4, 1); set32(0xE0, 5);
    expect(lock->check_global_2(3) == 0);
    expect(read32(0xE4) == 1);

    // Held by us, but a slot record still active: no, held flag kept.
    set32(0xE0, 3);
    set_flag(4, 1, 1);
    expect(lock->check_global_2(3) == 0);
    expect(read32(0xE4) == 1);

    // Held by us, nothing active: yes, and the held flag is cleared.
    set_flag(4, 1, 0);
    expect(lock->check_global_2(3) == 1);
    expect(read32(0xE4) == 0);

    // Record 0 active is ignored (scan starts at 1), so still releasable.
    set32(0xE4, 1);
    set_flag(0, 0, 1);
    expect(lock->check_global_2(3) == 1);
    set32(0xE4, 1);
    expect(lock_check_global_2_redirect(lock, nullptr, 3) == 1);

    LockCurrentServer = saved;
}

namespace {

int g_msg_args[6];

int g_msg_calls;

void __cdecl observe_message_data(int a1, int a2, int a3, int a4, int a5, int a6) {
    g_msg_args[0]=a1; g_msg_args[1]=a2; g_msg_args[2]=a3;
    g_msg_args[3]=a4; g_msg_args[4]=a5; g_msg_args[5]=a6;
    ++g_msg_calls;
}

}  // namespace

void test_lock_check_global() {
    // The void sibling of check_global_2: on a clean release it broadcasts
    // message_data(0x1205, owner, 0, 0, 0, 0) and clears the held flag rather
    // than returning a value. The same four no-op gates apply, and the message
    // arguments are pinned.
    auto *const saved_server = LockCurrentServer;
    auto *const saved_msg = LockMessageData;
    LockCurrentServer = &observe_current_server;   // reused from check_global_2
    LockMessageData = &observe_message_data;

    std::vector<uint8_t> storage(sizeof(Lock) + 32);
    auto *lock = reinterpret_cast<Lock *>(storage.data() + 16);
    uint8_t *const obj = storage.data() + 16;
    auto set32 = [&](size_t off, int32_t v) { std::memcpy(obj + off, &v, 4); };
    auto read32 = [&](size_t off) {
        int32_t v = 0; std::memcpy(&v, obj + off, sizeof(v)); return v;
    };
    auto set_flag = [&](int record, int entry, int32_t v) {
        std::memcpy(obj + record * 0x1C + 0xC + entry * 0xC, &v, sizeof(v));
    };

    // Not the server: nothing.
    std::memset(obj, 0, sizeof(Lock));
    set32(0xE4, 1); set32(0xE0, 7);
    g_current_server_result = 0;
    g_msg_calls = 0;
    lock->check_global();
    expect(g_msg_calls == 0);
    expect(read32(0xE4) == 1);
    g_current_server_result = 1;

    // Not held: nothing.
    set32(0xE4, 0);
    g_msg_calls = 0;
    lock->check_global();
    expect(g_msg_calls == 0);

    // Held, but a record still active: nothing, flag kept.
    set32(0xE4, 1);
    set_flag(3, 0, 1);
    g_msg_calls = 0;
    lock->check_global();
    expect(g_msg_calls == 0);
    expect(read32(0xE4) == 1);

    // Held, nothing active: broadcasts and clears the flag.
    set_flag(3, 0, 0);
    g_msg_calls = 0;
    lock->check_global();
    expect(g_msg_calls == 1);
    expect(g_msg_args[0] == 0x1205);
    expect(g_msg_args[1] == 7);          // the owner from 0xE0
    expect(g_msg_args[2] == 0 && g_msg_args[5] == 0);
    expect(read32(0xE4) == 0);

    set32(0xE4, 1);
    g_msg_calls = 0;
    lock_check_global_redirect(lock, nullptr);
    expect(g_msg_calls == 1);

    LockCurrentServer = saved_server;
    LockMessageData = saved_msg;
}

namespace {

void *g_sq_lock_entry;

int g_sq_lock_args[4];

int g_sq_lock_calls;

int g_sq_lock_result;

int __thiscall observe_square_lock(void *entry, int a1, int a2, int a3, int a4) {
    g_sq_lock_entry = entry;
    g_sq_lock_args[0] = a1;
    g_sq_lock_args[1] = a2;
    g_sq_lock_args[2] = a3;
    g_sq_lock_args[3] = a4;
    ++g_sq_lock_calls;
    return g_sq_lock_result;
}

}  // namespace

void test_lock_add_lock() {
    // add_lock(slot, flags, a3, a4) forwards to SquareLock::lock on the slot
    // record's second square entry - at record+0x10, i.e. records_[slot]+0x1C*slot
    // +0x10 - with mask bit 0x10 forced into the flags argument, and returns
    // whatever the seam returns. The seam receives (entry, slot, flags|0x10,
    // a3, a4); the two things a wrong recovery would get wrong are the entry
    // index (record+0x10, not record+4) and the 0x10 bit.
    auto *const saved = LockSquareLock;
    LockSquareLock = &observe_square_lock;

    std::vector<uint8_t> storage(sizeof(Lock) + 32);
    auto *lock = reinterpret_cast<Lock *>(storage.data() + 16);
    uint8_t *const obj = storage.data() + 16;
    std::memset(obj, 0xCD, sizeof(Lock));

    g_sq_lock_calls = 0;
    g_sq_lock_result = 0x1234;
    int rv = lock->add_lock(3, 0x21, 7, 9);
    expect(g_sq_lock_calls == 1);
    expect(g_sq_lock_entry == obj + 3 * 0x1C + 0x10);   // entries[1]
    expect(g_sq_lock_args[0] == 3);                     // slot
    expect(g_sq_lock_args[1] == (0x21 | 0x10));         // flags with 0x10 set
    expect(g_sq_lock_args[2] == 7);
    expect(g_sq_lock_args[3] == 9);
    expect(rv == 0x1234);                               // result passthrough

    // A slot-0 lock still targets record 0's second entry, and the 0x10 bit is
    // set even when the caller's flags already lack it.
    g_sq_lock_calls = 0;
    g_sq_lock_result = -1;
    rv = lock_add_lock_redirect(lock, nullptr, 0, 0, 1, 2);
    expect(g_sq_lock_calls == 1);
    expect(g_sq_lock_entry == obj + 0x10);
    expect(g_sq_lock_args[1] == 0x10);
    expect(rv == -1);

    LockSquareLock = saved;
}

namespace {

// A two-call recorder for the double-lock in Lock::lock: captures each call's
// entry pointer and four args, and returns a per-call configured result.
void *g_sq_lock2_entries[2];

int g_sq_lock2_args[2][4];

int g_sq_lock2_results[2];

int g_sq_lock2_calls;

int __thiscall observe_square_lock2(void *entry, int a1, int a2, int a3, int a4) {
    int idx = g_sq_lock2_calls;
    if (idx < 2) {
        g_sq_lock2_entries[idx] = entry;
        g_sq_lock2_args[idx][0] = a1;
        g_sq_lock2_args[idx][1] = a2;
        g_sq_lock2_args[idx][2] = a3;
        g_sq_lock2_args[idx][3] = a4;
    }
    ++g_sq_lock2_calls;
    return idx < 2 ? g_sq_lock2_results[idx] : 0;
}

}  // namespace

void test_lock_lock() {
    // Lock::lock(slot, flags, a3, a4, a5, a6, a7) locks both of a slot's square
    // entries: entries[0] with (slot, flags, a3, a4), entries[1] with (slot, a5,
    // a6, a7). It refuses when another owner holds the global lock, takes the
    // global lock when flags&2 is set, unwinds (unlocking both) when either
    // square fails, and on a global success over the server runs the release
    // broadcast (skipping its own slot) that check_global does.
    auto *const saved_lock = LockSquareLock;
    auto *const saved_unlock = LockSquareUnlock;
    auto *const saved_server = LockCurrentServer;
    auto *const saved_msg = LockMessageData;
    LockSquareLock = &observe_square_lock2;
    LockSquareUnlock = &observe_square_unlock;
    LockCurrentServer = &observe_current_server;
    LockMessageData = &observe_message_data;

    std::vector<uint8_t> storage(sizeof(Lock) + 32);
    auto *lock = reinterpret_cast<Lock *>(storage.data() + 16);
    uint8_t *const obj = storage.data() + 16;
    auto set32 = [&](size_t off, int32_t v) { std::memcpy(obj + off, &v, 4); };
    auto read32 = [&](size_t off) {
        int32_t v = 0; std::memcpy(&v, obj + off, sizeof(v)); return v;
    };
    auto set_flag = [&](int record, int entry, int32_t v) {
        std::memcpy(obj + record * 0x1C + 0xC + entry * 0xC, &v, sizeof(v));
    };

    // Refusal: another owner (5) holds the global lock, a lock for slot 3 is
    // rejected outright - no square touched, owner unchanged.
    std::memset(obj, 0, sizeof(Lock));
    set32(0xE0, 5);
    g_sq_lock2_calls = 0;
    expect(lock->lock(3, 2, 0, 0, 0, 0, 0) == 1);
    expect(g_sq_lock2_calls == 0);
    expect(read32(0xE0) == 5);

    // Plain double-lock (flags&2 clear, no global): both squares locked with
    // the right entry pointers and arg groups, entries reset to sentinels
    // first, no broadcast.
    std::memset(obj, 0, sizeof(Lock));
    g_sq_lock2_calls = 0;
    g_sq_lock2_results[0] = 0; g_sq_lock2_results[1] = 0;
    g_msg_calls = 0;
    int rv = lock->lock(3, 0, 11, 12, 13, 14, 15);
    expect(rv == 0);
    expect(g_sq_lock2_calls == 2);
    expect(g_sq_lock2_entries[0] == obj + 3 * 0x1C + 4);    // entries[0]
    expect(g_sq_lock2_args[0][0] == 3 && g_sq_lock2_args[0][1] == 0 &&
           g_sq_lock2_args[0][2] == 11 && g_sq_lock2_args[0][3] == 12);
    expect(g_sq_lock2_entries[1] == obj + 3 * 0x1C + 0x10);  // entries[1]
    expect(g_sq_lock2_args[1][0] == 3 && g_sq_lock2_args[1][1] == 13 &&
           g_sq_lock2_args[1][2] == 14 && g_sq_lock2_args[1][3] == 15);
    // Both entries were reset to {-1, -1, 0} before the locks ran.
    expect(read32(3 * 0x1C + 4) == -1 && read32(3 * 0x1C + 8) == -1);
    expect(read32(3 * 0x1C + 0x10) == -1 && read32(3 * 0x1C + 0x14) == -1);
    expect(read32(0xE0) == 0 && g_msg_calls == 0);          // no global taken

    // Global success off the server: owner and held flag are set, no broadcast.
    std::memset(obj, 0, sizeof(Lock));
    g_current_server_result = 0;
    g_sq_lock2_calls = 0;
    g_sq_lock2_results[0] = 0; g_sq_lock2_results[1] = 0;
    g_msg_calls = 0;
    rv = lock->lock(4, 2, 0, 0, 0, 0, 0);
    expect(rv == 0);
    expect(read32(0xE0) == 4 && read32(0xE4) == 1);
    expect(g_msg_calls == 0);

    // Global success on the server, nothing else active: broadcasts the
    // release and drops the held flag.
    std::memset(obj, 0, sizeof(Lock));
    g_current_server_result = 1;
    g_sq_lock2_calls = 0;
    g_sq_lock2_results[0] = 0; g_sq_lock2_results[1] = 0;
    g_msg_calls = 0;
    rv = lock->lock(4, 2, 0, 0, 0, 0, 0);
    expect(rv == 0);
    expect(g_msg_calls == 1);
    expect(g_msg_args[0] == 0x1205 && g_msg_args[1] == 4);
    expect(read32(0xE4) == 0);

    // Global success on the server but another slot still has an active square:
    // no broadcast.
    std::memset(obj, 0, sizeof(Lock));
    set_flag(2, 0, 1);                     // slot 2, first entry active
    g_current_server_result = 1;
    g_sq_lock2_calls = 0;
    g_sq_lock2_results[0] = 0; g_sq_lock2_results[1] = 0;
    g_msg_calls = 0;
    rv = lock->lock(4, 2, 0, 0, 0, 0, 0);
    expect(rv == 0 && g_msg_calls == 0);

    // First square fails: only the first lock is attempted, both entries are
    // unlocked, and the global lock this slot held is dropped.
    std::memset(obj, 0, sizeof(Lock));
    g_sq_lock2_calls = 0;
    g_sq_lock2_results[0] = 99; g_sq_lock2_results[1] = 0;
    g_sq_unlock_calls = 0;
    rv = lock->lock(6, 2, 0, 0, 0, 0, 0);
    expect(rv == 1);
    expect(g_sq_lock2_calls == 1);         // second lock skipped
    expect(g_sq_unlock_calls == 2);
    expect(g_sq_unlock_entries[0] == obj + 6 * 0x1C + 4);
    expect(g_sq_unlock_entries[1] == obj + 6 * 0x1C + 0x10);
    expect(read32(0xE0) == 0 && read32(0xE4) == 0);   // owner dropped

    // Second square fails: both locks attempted, both entries unlocked.
    std::memset(obj, 0, sizeof(Lock));
    g_sq_lock2_calls = 0;
    g_sq_lock2_results[0] = 0; g_sq_lock2_results[1] = 7;
    g_sq_unlock_calls = 0;
    rv = lock_lock_redirect(lock, nullptr, 2, 0, 0, 0, 0, 0, 0);
    expect(rv == 1);
    expect(g_sq_lock2_calls == 2);
    expect(g_sq_unlock_calls == 2);
    expect(g_sq_unlock_entries[0] == obj + 2 * 0x1C + 4);
    expect(g_sq_unlock_entries[1] == obj + 2 * 0x1C + 0x10);

    LockSquareLock = saved_lock;
    LockSquareUnlock = saved_unlock;
    LockCurrentServer = saved_server;
    LockMessageData = saved_msg;
}

// map.cpp is not linked into the leaf-test target, so SquareLock::unlock's
// direct dependencies are provided here: the two map-bound globals it reads,
// a faithful copy of the tiny xrange it wraps x through, and an observer in
// place of unlock_map that records the footprint calls the test then checks.
struct SqUnlockMapCall { int x, y, faction; };

std::vector<SqUnlockMapCall> g_sq_unlock_map_calls;

void __cdecl unlock_map(uint32_t x, uint32_t y, uint32_t faction_id) {
    g_sq_unlock_map_calls.push_back(
        {static_cast<int>(x), static_cast<int>(y), static_cast<int>(faction_id)});
}

// lock_map's observer records its calls and can be told to report one tile as
// already held (a nonzero BOOL) so the abort path can be exercised.
struct SqLockMapCall { int x, y, faction; };

std::vector<SqLockMapCall> g_sq_lock_map_calls;

int g_sq_lock_map_fail_at = -1;

BOOL __cdecl lock_map(uint32_t x, uint32_t y, uint32_t faction_id) {
    int i = static_cast<int>(g_sq_lock_map_calls.size());
    g_sq_lock_map_calls.push_back(
        {static_cast<int>(x), static_cast<int>(y), static_cast<int>(faction_id)});
    return (g_sq_lock_map_fail_at >= 0 && i == g_sq_lock_map_fail_at) ? TRUE : FALSE;
}

int __cdecl xrange(int x) {
    if (!*MapIsFlat) {
        if (x >= 0) {
            if (x >= *MapLongitudeBounds) {
                x -= *MapLongitudeBounds;
            }
        } else {
            x += *MapLongitudeBounds;
        }
    }
    return x;
}

int *MapLongitudeBounds;

int *MapLatitudeBounds;

BOOL *MapIsFlat;

// game.cpp is not linked either, so Console::edit_lock's GameState global is
// provided here, alongside a stand-in for the GetKeyState import it consults.
uint32_t *GameState;

// general.cpp is not linked here either; Console::editor_undo's only call is
// observed rather than run.
int g_load_undo_arg;

int g_load_undo_calls;

void __cdecl load_undo(int type) { g_load_undo_arg = type; ++g_load_undo_calls; }

namespace {

int g_console_key_ret;

int g_console_key_vk;

}  // namespace

SHORT __stdcall fake_console_get_key_state(int virtual_key) {
    g_console_key_vk = virtual_key;
    return static_cast<SHORT>(g_console_key_ret);
}

func_get_key_state *g_console_key_fn = &fake_console_get_key_state;

void test_square_lock_unlock() {
    // SquareLock::unlock releases the square and its footprint. When the lock
    // bit (0x1) is clear it does nothing; when set it clears the record to its
    // unset sentinels and, for an on-map coordinate, calls unlock_map over a
    // footprint whose size the flags choose - one tile, or a radius of 25 (0x4
    // set, 0x10 clear) or 81 (also 0x8 set) tiles - walked through the shared
    // RadiusOffset tables and wrapped in x by xrange. unlock_map is observed
    // (map.cpp is not linked here); the expected call sequence is recomputed the
    // same way, in order, pinning the count, the offset tables, the x-wrap, the
    // bounds skip, and the factionID passthrough at once.
    int width = 40, height = 40;
    BOOL flat = TRUE;
    MapLongitudeBounds = &width;
    MapLatitudeBounds = &height;
    MapIsFlat = &flat;

    std::vector<uint8_t> storage(sizeof(SquareLock) + 32);
    auto *sq = reinterpret_cast<SquareLock *>(storage.data() + 16);
    uint8_t *const rec = storage.data() + 16;
    auto set_rec = [&](int32_t first, int32_t second, int32_t flag) {
        std::memcpy(rec + 0, &first, 4);
        std::memcpy(rec + 4, &second, 4);
        std::memcpy(rec + 8, &flag, 4);
    };
    auto read_rec = [&](size_t off) {
        int32_t v = 0; std::memcpy(&v, rec + off, 4); return v;
    };

    // Lock bit clear: nothing happens, the record is left untouched.
    g_sq_unlock_map_calls.clear();
    set_rec(5, 7, 0);
    sq->unlock(3);
    expect(read_rec(0) == 5 && read_rec(4) == 7 && read_rec(8) == 0);
    expect(g_sq_unlock_map_calls.empty());

    // Lock set but the coordinate is off the map: the record is cleared to its
    // sentinels with no footprint work. All four out-of-bounds directions.
    for (auto coord : {std::pair<int, int>{width + 5, 20}, {-1, 20},
                       {20, height}, {20, -1}}) {
        g_sq_unlock_map_calls.clear();
        set_rec(coord.first, coord.second, 1);
        sq->unlock(3);
        expect(read_rec(0) == -1 && read_rec(4) == -1 && read_rec(8) == 0);
        expect(g_sq_unlock_map_calls.empty());
    }

    // On-map: the observed unlock_map call sequence must match, in order, the
    // footprint the flags select (recomputed via xrange + RadiusOffset), and the
    // record ends cleared to sentinels.
    auto run_case = [&](int rec_x, int rec_y, int flag, int count) {
        g_sq_unlock_map_calls.clear();
        set_rec(rec_x, rec_y, flag);
        sq->unlock(3);
        expect(read_rec(0) == -1 && read_rec(4) == -1 && read_rec(8) == 0);
        std::vector<SqUnlockMapCall> want;
        for (int i = 0; i < count; ++i) {
            int nx = xrange(rec_x + RadiusOffsetX[i]);
            int ny = rec_y + RadiusOffsetY[i];
            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                want.push_back({nx, ny, 3});
            }
        }
        expect(g_sq_unlock_map_calls.size() == want.size());
        for (size_t i = 0; i < want.size() &&
                           i < g_sq_unlock_map_calls.size(); ++i) {
            expect(g_sq_unlock_map_calls[i].x == want[i].x &&
                   g_sq_unlock_map_calls[i].y == want[i].y &&
                   g_sq_unlock_map_calls[i].faction == want[i].faction);
        }
    };

    flat = TRUE;
    run_case(20, 20, 1, 1);            // plain lock: a single tile
    run_case(20, 20, 1 | 4, 25);       // 0x4 -> radius of 25
    run_case(20, 20, 1 | 4 | 8, 81);   // 0x4 and 0x8 -> radius of 81
    run_case(20, 20, 1 | 4 | 0x10, 1); // 0x10 overrides the radius back to 1
    run_case(20, 20, 1 | 8, 1);        // 0x8 without 0x4 stays a single tile

    // Round map: a lock at the x-edge whose footprint wraps around, so xrange
    // (not a raw add) is what the recovery must use.
    flat = FALSE;
    run_case(1, 20, 1 | 4, 25);
    flat = TRUE;

    // The redirect entry drives the same footprint, and passes its faction.
    g_sq_unlock_map_calls.clear();
    set_rec(20, 20, 1);
    square_lock_unlock_redirect(sq, nullptr, 6);
    expect(g_sq_unlock_map_calls.size() == 1);
    expect(g_sq_unlock_map_calls[0].x == 20 && g_sq_unlock_map_calls[0].y == 20 &&
           g_sq_unlock_map_calls[0].faction == 6);
}

void test_square_lock_lock() {
    // SquareLock::lock stores the coordinate and flags|1 in the record
    // unconditionally, then - for an on-map coordinate - calls lock_map over the
    // same flag-sized footprint SquareLock::unlock walks, abandoning the attempt
    // (return 1) at the first tile another faction already holds. lock_map is
    // observed here; the expected call sequence is recomputed the same way.
    int width = 40, height = 40;
    BOOL flat = TRUE;
    MapLongitudeBounds = &width;
    MapLatitudeBounds = &height;
    MapIsFlat = &flat;

    std::vector<uint8_t> storage(sizeof(SquareLock) + 32);
    auto *sq = reinterpret_cast<SquareLock *>(storage.data() + 16);
    uint8_t *const rec = storage.data() + 16;
    auto read_rec = [&](size_t off) {
        int32_t v = 0; std::memcpy(&v, rec + off, 4); return v;
    };

    // Off-map: the record is still stamped with the coordinate and flags|1, but
    // no footprint work happens and the result is 0.
    g_sq_lock_map_calls.clear();
    g_sq_lock_map_fail_at = -1;
    std::memset(rec, 0xEE, sizeof(SquareLock));
    expect(sq->lock(3, 4, 100, 20) == 0);   // x=100 off-map
    expect(read_rec(0) == 100 && read_rec(4) == 20 && read_rec(8) == (4 | 1));
    expect(g_sq_lock_map_calls.empty());

    // On-map: the observed lock_map sequence matches the footprint the flags
    // select, the record is stamped, and the result is 0 when nothing is held.
    auto run_case = [&](int rec_x, int rec_y, int flags, int count) {
        g_sq_lock_map_calls.clear();
        g_sq_lock_map_fail_at = -1;
        expect(sq->lock(3, flags, rec_x, rec_y) == 0);
        expect(read_rec(0) == rec_x && read_rec(4) == rec_y &&
               read_rec(8) == (flags | 1));
        std::vector<SqLockMapCall> want;
        for (int i = 0; i < count; ++i) {
            int nx = xrange(rec_x + RadiusOffsetX[i]);
            int ny = rec_y + RadiusOffsetY[i];
            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                want.push_back({nx, ny, 3});
            }
        }
        expect(g_sq_lock_map_calls.size() == want.size());
        for (size_t i = 0; i < want.size() &&
                           i < g_sq_lock_map_calls.size(); ++i) {
            expect(g_sq_lock_map_calls[i].x == want[i].x &&
                   g_sq_lock_map_calls[i].y == want[i].y &&
                   g_sq_lock_map_calls[i].faction == want[i].faction);
        }
    };

    flat = TRUE;
    run_case(20, 20, 1, 1);            // plain lock: a single tile
    run_case(20, 20, 1 | 4, 25);       // 0x4 -> radius of 25
    run_case(20, 20, 1 | 4 | 8, 81);   // 0x4 and 0x8 -> radius of 81
    run_case(20, 20, 1 | 4 | 0x10, 1); // 0x10 overrides the radius back to 1
    run_case(20, 20, 8, 1);            // 0x8 without 0x4 stays a single tile

    // Round map: a lock at the x-edge whose footprint wraps around.
    flat = FALSE;
    run_case(1, 20, 1 | 4, 25);
    flat = TRUE;

    // A contested tile: lock_map reports the 4th tile (call index 3) already
    // held, so the attempt aborts there with only four calls and a result of 1.
    g_sq_lock_map_calls.clear();
    g_sq_lock_map_fail_at = 3;
    expect(sq->lock(3, 1 | 4, 20, 20) == 1);
    expect(g_sq_lock_map_calls.size() == 4);

    // The redirect entry forwards all four args, including the faction.
    g_sq_lock_map_calls.clear();
    g_sq_lock_map_fail_at = -1;
    expect(square_lock_lock_redirect(sq, nullptr, 6, 1, 20, 20) == 0);
    expect(g_sq_lock_map_calls.size() == 1);
    expect(g_sq_lock_map_calls[0].faction == 6 &&
           g_sq_lock_map_calls[0].x == 20 && g_sq_lock_map_calls[0].y == 20);
}

void test_console_edit_lock() {
    // edit_lock is meaningful only in the scenario editor: outside it, always
    // unlocked. Inside it, editing locks when Scroll Lock is toggled (bit 0 of
    // GetKeyState) or the game is in editor-only mode. GetKeyState is stood in
    // for through the rebound import slot so the key state is controllable.
    uint32_t state = 0;
    auto *const saved_gs = GameState;
    auto *const saved_slot = ConsoleEditKeyStateSlot;
    GameState = &state;
    g_console_key_fn = &fake_console_get_key_state;
    ConsoleEditKeyStateSlot = &g_console_key_fn;

    std::vector<uint8_t> buf(64);   // this is never dereferenced by edit_lock
    auto *con = reinterpret_cast<Console *>(buf.data());

    // Not in the scenario editor: unlocked regardless of key or editor-only bit.
    state = 0;
    g_console_key_ret = 1;
    expect(con->edit_lock() == 0);
    state = STATE_EDITOR_ONLY_MODE;
    expect(con->edit_lock() == 0);

    // Scenario editor + Scroll Lock toggled (bit 0) locks editing, and the key
    // queried is Scroll Lock.
    state = STATE_SCENARIO_EDITOR;
    g_console_key_ret = 1;
    g_console_key_vk = 0;
    expect(con->edit_lock() == 1);
    expect(g_console_key_vk == VK_SCROLL);

    // Only bit 0 counts: the down bit (0x80) alone does not lock.
    g_console_key_ret = 0x80;
    expect(con->edit_lock() == 0);

    // Editor-only mode locks editing even with the key clear.
    g_console_key_ret = 0;
    state = STATE_SCENARIO_EDITOR | STATE_EDITOR_ONLY_MODE;
    expect(con->edit_lock() == 1);

    // Scenario editor, key clear, not editor-only: unlocked.
    state = STATE_SCENARIO_EDITOR;
    g_console_key_ret = 0;
    expect(con->edit_lock() == 0);

    // Redirect entry.
    g_console_key_ret = 1;
    expect(console_edit_lock_redirect(con, nullptr) == 1);

    GameState = saved_gs;
    ConsoleEditKeyStateSlot = saved_slot;
}

// Self-registration.  Each case carries its baseline position in the original
// main() call list, so the run order does not depend on link or static-init
// order.
namespace {
LEAF_CASE(156, test_lock_any_locks);
LEAF_CASE(157, test_lock_unlock);
LEAF_CASE(158, test_lock_global_lock);
LEAF_CASE(159, test_lock_check_global_2);
LEAF_CASE(160, test_lock_check_global);
LEAF_CASE(161, test_lock_add_lock);
LEAF_CASE(162, test_lock_lock);
LEAF_CASE(163, test_square_lock_unlock);
LEAF_CASE(164, test_square_lock_lock);
LEAF_CASE(165, test_console_edit_lock);
}  // namespace
