// Pytest inserts the current source declarations and hook bodies.
// Allocation and outer destruction are doubles; this is not list-lifetime proof.
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
typedef unsigned int uint32_t;
#define MEASURED

int allocations, fail_at, frees, heap_calls;
void *blocks[16];
int block_count;
void *allocate(size_t size) {
    if (++allocations == fail_at) return 0;
    void *memory = malloc(size);
    assert(memory && block_count < 16);
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
struct Heap { void *get(size_t size) { ++heap_calls; return allocate(size); } };
Heap *StringAllocationHeap;
void *mem_get(size_t size) { return allocate(size); }
// LIVE_ALLOCATION_BASE
StringAllocationBase::~StringAllocationBase() { StringAllocationHeap = allocation_owner_; }
// LIVE_PAYLOAD
// LIVE_ENTRY
// LIVE_LIST_BASE
// LIVE_LIST
// Full outer destruction remains a separate recovery obligation.
StringStruct::~StringStruct() {}
StringList::~StringList() {}
int StringStruct::on_insert(StringStructPayload *) { return 0; }
void StringStruct::on_remove(StringStructPayload *) {}
#define free release
// LIVE_TEXT_RELEASE
// LIVE_INSERT
// LIVE_REMOVE
#undef free

int insert(StringStruct *owner, StringStructPayload *payload) { return owner->on_insert(payload); }
void remove(StringStruct *owner, StringStructPayload *payload) { owner->on_remove(payload); }

int main() {
    StringList list;
    StringStructPayload payload;
    Heap heap;
    char source[] = "copied text";
    char empty[] = "";
    char sentinel[] = "untouched";
    char *StringList::*text_member = &StringList::pending_text_;
    assert(sizeof(list) == 0x30 && sizeof(payload) == 0x18);
    list.field_20_ = 0xFFFFFFFF;
    list.field_24_ = 37;

    // Null source is not permission to initialize or read the payload text.
    list.pending_text_ = 0;
    payload.text = sentinel;
    assert(insert(&list, &payload) == 0);
    assert(payload.text == sentinel && allocations == 0);
    assert(payload.field_8_ == 0xFFFFFFFF && payload.field_C_ == 37);

    list.pending_text_ = source;
    assert(list.*text_member == source);
    assert(insert(&list, &payload) == 0);
    assert(payload.text != source && strcmp(payload.text, source) == 0);
    assert(heap_calls == 0 && allocations == 1);
    remove(&list, &payload);
    assert(payload.text == 0 && frees == 1);
    remove(&list, &payload);
    assert(frees == 1);

    list.pending_text_ = empty;
    assert(insert(&list, &payload) == 0 && payload.text != 0 && *payload.text == 0);
    remove(&list, &payload);
    assert(frees == 2);

    // Failure publishes null text but does not reach either metadata store.
    fail_at = allocations + 1;
    payload.text = sentinel;
    payload.field_8_ = 9;
    payload.field_C_ = 10;
    assert(insert(&list, &payload) == 4 && payload.text == 0);
    assert(payload.field_8_ == 9 && payload.field_C_ == 10);
    fail_at = 0;

    list.allocator_ = &heap;
    list.pending_text_ = source;
    assert(insert(&list, &payload) == 0 && heap_calls == 1);
    char *owned = payload.text;
    remove(&list, &payload);
    assert(payload.text == owned && frees == 2);
    release(owned); // The owning heap, not the hook, releases this allocation.
    payload.text = 0;
    remove(&list, &payload);
    assert(frees == 3);

    fail_at = allocations + 1;
    payload.text = sentinel;
    payload.field_8_ = 11;
    payload.field_C_ = 12;
    assert(insert(&list, &payload) == 4 && payload.text == 0 && heap_calls == 2);
    assert(payload.field_8_ == 11 && payload.field_C_ == 12);
    for (int i = 0; i < block_count; ++i) assert(blocks[i] == 0);
    return 0;
}
