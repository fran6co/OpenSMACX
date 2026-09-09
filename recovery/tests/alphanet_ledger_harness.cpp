// Live ledger/payload declarations and virtual bodies are inserted by pytest.
// Allocation and nested-list collaborators are doubles, not full-game proof.
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int uint32_t;
struct GUID { uint32_t Data1; unsigned short Data2, Data3; unsigned char Data4[8]; };
int allocations, fail_at, frees;
void *blocks[32];
int block_count;
void *allocate(size_t size) {
    if (++allocations == fail_at) return 0;
    void *memory = malloc(size);
    assert(memory && block_count < 32);
    blocks[block_count++] = memory;
    return memory;
}
void release(void *memory) {
    for (int i = 0; i < block_count; ++i) {
        if (blocks[i] == memory) {
            blocks[i] = 0;
            ++frees;
            free(memory);
            return;
        }
    }
    assert(false);
}
struct Heap { void *get(size_t size) { return allocate(size); } };
Heap *StringAllocationHeap;
void *mem_get(size_t size) { return allocate(size); }
struct NetUnk72C { unsigned char opaque[0x2C]; };
// LIVE_ALLOCATION_BASE
StringAllocationBase::~StringAllocationBase() { StringAllocationHeap = allocation_owner_; }
// LIVE_PAYLOAD
// LIVE_LEDGER_BASE
// LIVE_DERIVED_DECLARATION
// LIVE_LEDGER_CONSTRUCTOR
int AlphaNetStringStruct::on_insert(NetAllocationNode *) { return 0; }
void AlphaNetStringStruct::on_remove(NetAllocationNode *) {}
// Full list destruction remains unfinished; these tests exercise the hooks.
AlphaNetStringStruct::~AlphaNetStringStruct() {}
AlphaNetLedger::~AlphaNetLedger() {}
#define free release
// LIVE_INSERT
// LIVE_REMOVE
#undef free

int main() {
    assert(sizeof(NetAllocationNode) == 0x60);
    assert(sizeof(AlphaNetStringStruct) == 0x24 && sizeof(AlphaNetLedger) == 0x40);
    GUID guid = {0x12345678, 0x2345, 0x3456, {1,2,3,4,5,6,7,8}};
    Heap heap;
    AlphaNetLedger ledger;
    AlphaNetStringStruct *base = &ledger;
    ledger.instance_guid_ = &guid;
    ledger.session_flags_ = 0x1357;
    ledger.max_players_ = 8;
    ledger.current_players_ = 3;
    ledger.field_34_ = 0x2468;
    for (int scenario = 0; scenario < 10; ++scenario) {
        allocations = frees = block_count = 0;
        fail_at = scenario == 4 || scenario == 8 ? 1 : scenario == 5 || scenario == 9 ? 2 : 0;
        ledger.allocator_ = scenario >= 6 ? &heap : 0;
        ledger.session_name_ = scenario == 0 || scenario == 2 || scenario == 6 ? 0 : "session";
        ledger.password_ = scenario == 0 || scenario == 1 || scenario == 6 ? 0 : "secret";
        NetAllocationNode payload;
        payload.session_name_ = payload.password_ = 0;
        payload.session_flags_ = 0xAAAAAAAA;
        int result = base->on_insert(&payload);  // Real virtual dispatch.
        assert(result == (fail_at ? 4 : 0));
        if (!fail_at) {
            assert(memcmp(&payload.instance_guid_, &guid, sizeof(guid)) == 0);
            assert(payload.session_flags_ == 0x1357 && payload.max_players_ == 8);
            assert(payload.current_players_ == 3 && payload.field_28_ == 0x2468);
            if (ledger.session_name_) {
                assert(payload.session_name_ != ledger.session_name_);
                assert(strcmp(payload.session_name_, ledger.session_name_) == 0);
            } else assert(!payload.session_name_);
            if (ledger.password_) {
                assert(payload.password_ != ledger.password_);
                assert(strcmp(payload.password_, ledger.password_) == 0);
            } else assert(!payload.password_);
        } else {
            assert(payload.session_flags_ == 0xAAAAAAAA);
            assert(!payload.password_);
            assert((payload.session_name_ != 0) == (fail_at == 2));
        }
        char *name = payload.session_name_, *password = payload.password_;
        base->on_remove(&payload);
        if (ledger.allocator_) {
            assert(frees == 0 && payload.session_name_ == name && payload.password_ == password);
        } else {
            assert(!payload.session_name_ && !payload.password_);
            assert(frees == block_count);
            base->on_remove(&payload);  // Clearing twice does not free twice.
            assert(frees == block_count);
        }
        for (int j = 0; j < block_count; ++j) if (blocks[j]) release(blocks[j]);
    }
    return 0;
}
