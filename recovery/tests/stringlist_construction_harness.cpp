// Pytest injects actual node/list declarations, construction, add and clearing.
// Allocators and fatal reporting are doubles; this is not a full-game oracle.
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <new>
typedef unsigned int uint32_t;
#define MEASURED
#define nullptr 0
#define MB_OK 0
struct Heap;
Heap *StringAllocationHeap;
struct Block { void *memory; Heap *owner; int frees; };
Block blocks[32];
int block_count, allocations, fail_at, notices;
Heap *requested_owner[32];
size_t requested_size[32];

void *allocate(size_t size, Heap *owner) {
    assert(allocations < 32);
    requested_owner[allocations] = owner;
    requested_size[allocations++] = size;
    if (allocations == fail_at) return 0;
    void *memory = malloc(size);
    assert(memory && block_count < 32);
    memset(memory, 0xA5, size);
    blocks[block_count].memory = memory;
    blocks[block_count].owner = owner;
    blocks[block_count++].frees = 0;
    return memory;
}
struct Heap { void *get(size_t size) { return allocate(size, this); } };
void *mem_get(size_t size) { return allocate(size, 0); }
void release(void *memory) {
    assert(!StringAllocationHeap);
    for (int i = 0; i < block_count; ++i) {
        if (blocks[i].memory == memory) {
            assert(!blocks[i].owner && !blocks[i].frees);
            ++blocks[i].frees;
            free(memory);
            return;
        }
    }
    assert(false); // Includes rejecting a virtual-base pointer for deallocation.
}
int MessageBoxA(void *window, const char *text, const char *caption, unsigned flags) {
    assert(!window && !flags);
    assert(strcmp(text, "out of heap space in new function") == 0);
    assert(strcmp(caption, "Aborting") == 0);
    ++notices;
    return 0;
}
void test_exit(int code) { assert(code == 3 && notices == 1); throw code; }

// LIVE_ALLOCATION_BASE
// LIVE_OWNER_PUBLICATION
#define free release
// LIVE_PAYLOAD
// LIVE_ENTRY
#undef free
// LIVE_LIST_BASE
// LIVE_CLEANUP_HELPER
int StringStruct::on_insert(StringStructPayload *) { return 0; }
void StringStruct::on_remove(StringStructPayload *) {}
// LIVE_BASE_DESTRUCTOR
// LIVE_CLEAR
#define exit test_exit
// LIVE_NODE_ALLOCATION
// LIVE_ADD
#undef exit

struct HookList : StringStruct {
    int inserts, removes, mutate_count;
    HookList() : inserts(0), removes(0), mutate_count(0) {}
    virtual int on_insert(StringStructPayload *payload) {
        assert(!StringAllocationHeap);
        if (entry_count_ == 0) {
            assert(head_->payload == payload && !current_);
            assert(head_->next == head_ && head_->previous == head_);
        } else {
            assert(current_ == head_->previous && current_->payload == payload);
            assert(current_->next == head_ && current_->previous->next == current_);
        }
        payload->text = 0;
        ++inserts;
        if (mutate_count) entry_count_ = mutate_count;
        return 4; // add ignores the hook's result, including this error value.
    }
    virtual void on_remove(StringStructPayload *) { ++removes; }
};

void park(StringStruct &list) {
    list.head_ = list.current_ = 0;
    list.entry_count_ = 0;
}
void reset() {
    for (int i = 0; i < block_count; ++i)
        if (!blocks[i].frees) free(blocks[i].memory);
    block_count = allocations = fail_at = notices = 0;
    StringAllocationHeap = 0;
}

void check_success(Heap *explicit_owner, Heap *inherited_owner) {
    StringAllocationHeap = inherited_owner;
    {
        HookList list;
        list.allocator_ = explicit_owner;
        Heap *owner = explicit_owner ? explicit_owner : inherited_owner;
        assert(list.allocation_owner_ == inherited_owner && !StringAllocationHeap);
        assert(list.add(7) == 0 && list.entry_count_ == 1 && list.current_position_ == 0);
        StringStructEntry *first = list.head_;
        assert(first == list.current_ && first->id == 7);
        assert(list.add(8) == 0 && list.entry_count_ == 2 && list.current_position_ == 1);
        StringStructEntry *second = list.current_;
        assert(list.head_ == first && second != first && second->id == 8);
        assert(first->next == second && first->previous == second);
        assert(second->next == first && second->previous == first);
        assert(allocations == 4 && list.inserts == 2);
        for (int i = 0; i < allocations; ++i) {
            assert(requested_owner[i] == owner);
            assert(requested_size[i] == (i % 2 ? 0x18 : 0x1C));
        }
        assert(first->allocation_owner_ == owner && second->allocation_owner_ == owner);
        assert(first->payload->allocation_owner_ == owner && second->payload->allocation_owner_ == owner);
        list.remove_all();
        assert(list.removes == 2 && !list.head_ && !list.entry_count_ && !StringAllocationHeap);
        for (int j = 0; j < block_count; ++j) assert(blocks[j].frees == (owner ? 0 : 1));
    }
    reset();
}

void check_crt_failure(bool existing, int failed_allocation) {
    {
        HookList list;
        if (existing) assert(list.add(1) == 0);
        StringStructEntry *old_head = list.head_;
        StringStructEntry *old_current = list.current_;
        list.current_position_ = 77;
        fail_at = allocations + failed_allocation;
        assert(list.add(2) == 4);
        assert(list.entry_count_ == (existing ? 1 : 0) && list.current_position_ == 77);
        assert(list.inserts == (existing ? 1 : 0) && !notices);
        if (!existing) {
            assert(!list.current_);
            if (failed_allocation == 1) assert(!list.head_);
            else {
                assert(list.head_->id == 2 && !list.head_->payload);
                assert(list.head_->next == list.head_ && list.head_->previous == list.head_);
            }
        } else if (failed_allocation == 1) {
            // The original writes the failed allocation into old_tail->next.
            assert(list.head_ == old_head && list.current_ == old_current);
            assert(!old_head->next && old_head->previous == old_head);
        } else {
            assert(list.head_ == old_head && list.current_ != old_current);
            assert(old_head->next == list.current_ && old_head->previous == list.current_);
            assert(list.current_->next == old_head && list.current_->previous == old_head);
            assert(list.current_->id == 2 && !list.current_->payload);
        }
        park(list); // Partial failure is not a valid completed circular list.
    }
    reset();
}

void check_heap_failure(bool inherited, bool existing, int failed_allocation) {
    Heap heap;
    StringAllocationHeap = inherited ? &heap : 0;
    {
        HookList list;
        if (!inherited) list.allocator_ = &heap;
        if (existing) assert(list.add(1) == 0);
        fail_at = allocations + failed_allocation;
        bool caught = false;
        try { list.add(2); } catch (int code) { assert(code == 3); caught = true; }
        assert(caught && notices == 1 && allocations == fail_at);
        assert(requested_owner[allocations - 1] == &heap);
        assert(list.entry_count_ == (existing ? 1 : 0) && list.inserts == (existing ? 1 : 0));
        park(list);
    }
    reset();
}

int main() {
    Heap explicit_owner, inherited_owner;
    check_success(0, 0);
    check_success(&explicit_owner, 0);
    check_success(0, &inherited_owner);
    check_success(&explicit_owner, &inherited_owner);
    for (int existing = 0; existing < 2; ++existing) {
        for (int failure = 1; failure <= 2; ++failure) {
            check_crt_failure(existing != 0, failure);
            check_heap_failure(false, existing != 0, failure);
            check_heap_failure(true, existing != 0, failure);
        }
    }
    {
        HookList list;
        list.mutate_count = 7;
        assert(list.add(3) == 0 && list.entry_count_ == 8 && list.current_position_ == 7);
        park(list); // Count changed by the hook; do not walk it as eight nodes.
    }
    reset();
    return 0;
}
