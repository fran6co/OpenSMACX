// Actual node declarations, allocation-base destructor and remove_all are
// inserted by pytest. Allocator tracking is a double, not StringStruct::add.
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <new>
typedef unsigned int uint32_t;
typedef char *LPSTR; // SDK mutable string pointer used by the live declaration.
#define MEASURED
#define nullptr 0 // Same VC6 compatibility spelling as the product build.
struct Heap {};
Heap *StringAllocationHeap;
struct Block { void *memory; int frees; bool heap_owned; };
Block blocks[16];
int block_count, callbacks;
bool clearing;
bool checking_text_order;
int text_releases;

void release(void *memory) {
    assert(!StringAllocationHeap);
    for (int i = 0; i < block_count; ++i) {
        if (blocks[i].memory == memory) {
            assert(!blocks[i].heap_owned && blocks[i].frees == 0);
            if (clearing) assert(callbacks > 0);
            // Each original list iteration releases text before payload before
            // entry. Construction below records those blocks in reverse order.
            if (checking_text_order) {
                assert(i == (text_releases / 3) * 3 + 2 - text_releases % 3);
                ++text_releases;
            }
            ++blocks[i].frees;
            free(memory);
            return;
        }
    }
    // A virtual-base pointer rather than the complete allocation fails here.
    assert(false);
}

// Also observe accidental use of the global delete path: it must never free
// a Heap-owned block or a virtual-base interior pointer.
void operator delete(void *memory) { release(memory); }

#define free release
// LIVE_ALLOCATION_BASE
#undef free
// LIVE_OWNER_PUBLICATION
#define free release
// LIVE_PAYLOAD
struct SpriteStruct;
// LIVE_ENTRY
StringStructPayload *StringStructEntry::string_payload() {
    return static_cast<StringStructPayload *>(payload);
}
const StringStructPayload *StringStructEntry::string_payload() const {
    return static_cast<const StringStructPayload *>(payload);
}
#undef free
// LIVE_LIST_BASE
// LIVE_LIST
// LIVE_CONSTRUCTOR
// LIVE_CLEANUP_HELPER
int StringStruct::on_insert(void *) { return 0; }
void StringStruct::on_remove(void *) {}
int StringList::on_insert(void *) { return 0; }
#define free release
// LIVE_TEXT_RELEASE
// LIVE_REMOVE
#undef free
// LIVE_BASE_DESTRUCTOR
// LIVE_DERIVED_DESTRUCTOR
// LIVE_CLEAR

// Deduce T from an argument: VC6 mis-selects the explicit specialization in
// nested calls when the template type appears only in the return type.
template<class T> T *make_node(Heap *owner, T *) {
    assert(block_count < 16 && !StringAllocationHeap);
    void *memory = malloc(sizeof(T));
    assert(memory);
    blocks[block_count].memory = memory;
    blocks[block_count].frees = 0;
    blocks[block_count++].heap_owned = owner != 0;
    StringAllocationHeap = owner;
    T *node = ::new(memory) T;
    assert(!StringAllocationHeap && node->allocation_owner_ == owner);
    return node;
}

void reclaim() {
    for (int i = 0; i < block_count; ++i) {
        if (!blocks[i].frees) free(blocks[i].memory);
    }
    block_count = 0;
}

struct ObservedList : StringStruct {
    virtual void on_remove(void *raw_payload) {
        StringStructPayload *payload = static_cast<StringStructPayload *>(raw_payload);
        assert(!StringAllocationHeap);
        assert(head_ && head_->payload == payload);
        assert(current_ == head_->next);
        // Neither the current payload nor its entry may be freed yet.
        for (int i = 0; i < block_count; ++i) {
            if (blocks[i].memory == head_ || blocks[i].memory == payload)
                assert(blocks[i].frees == 0);
        }
        ++callbacks;
    }
};

void check_clear(Heap *entry_owner, Heap *payload_owner, bool null_payload) {
    ObservedList list;
    StringStructEntry *first = make_node(entry_owner, static_cast<StringStructEntry *>(0));
    StringStructEntry *second = make_node(entry_owner, static_cast<StringStructEntry *>(0));
    first->payload = null_payload ? 0 : make_node(payload_owner, static_cast<StringStructPayload *>(0));
    second->payload = make_node(payload_owner, static_cast<StringStructPayload *>(0));
    first->next = first->previous = second;
    second->next = second->previous = first;
    list.head_ = first;
    list.current_ = second;
    list.entry_count_ = 2;
    list.current_position_ = 1;
    callbacks = 0;
    clearing = true;
    list.remove_all();
    clearing = false;
    assert(callbacks == 2 && !list.head_ && !list.entry_count_ && !list.current_position_);
    assert(!StringAllocationHeap);
    for (int i = 0; i < block_count; ++i)
        assert(blocks[i].frees == (blocks[i].heap_owned ? 0 : 1));
    list.remove_all();
    assert(callbacks == 2);
    reclaim();
}

template<class T> void check_virtual_delete(T *node, Heap *owner, unsigned int displacement) {
    StringAllocationBase *base = node;
    assert(reinterpret_cast<char *>(base) - reinterpret_cast<char *>(node) == displacement);
    delete base;
    assert(blocks[0].frees == (owner ? 0 : 1) && !StringAllocationHeap);
    reclaim();
}

void check_text_and_nodes(Heap *owner, bool automatic_cleanup, Heap *list_owner, int count) {
    StringAllocationHeap = list_owner;
    text_releases = 0;
    checking_text_order = true;
    {
        StringList list;
        StringStructEntry *entries[3];
        assert(count >= 0 && count <= 3);
        for (int i = 0; i < count; ++i) {
            StringStructEntry *entry = make_node(owner, static_cast<StringStructEntry *>(0));
            StringStructPayload *payload = make_node(owner, static_cast<StringStructPayload *>(0));
            payload->text = static_cast<char *>(malloc(1));
            assert(payload->text);
            *payload->text = 0;
            blocks[block_count].memory = payload->text;
            blocks[block_count].frees = 0;
            blocks[block_count++].heap_owned = owner != 0;
            entry->payload = payload;
            entries[i] = entry;
        }
        for (int j = 0; j < count; ++j) {
            entries[j]->next = entries[(j + 1) % count];
            entries[j]->previous = entries[(j + count - 1) % count];
        }
        list.allocator_ = owner;
        list.head_ = list.current_ = count ? entries[0] : 0;
        list.entry_count_ = count;
        if (!automatic_cleanup) {
            list.remove_all();
            assert(!list.head_ && !list.entry_count_ && !StringAllocationHeap);
        }
    } // Automatic destruction must run the derived text hook before the base.
    checking_text_order = false;
    assert(text_releases == (owner ? 0 : 3 * count));
    assert(StringAllocationHeap == list_owner);
    StringAllocationHeap = 0;
    for (int i = 0; i < block_count; ++i)
        assert(blocks[i].frees == (owner ? 0 : 1));
    reclaim();
}

void check_populated_virtual_delete(Heap *list_owner, Heap *node_owner) {
    StringList *list = make_node(list_owner, static_cast<StringList *>(0));
    StringStructEntry *entry = make_node(node_owner, static_cast<StringStructEntry *>(0));
    StringStructPayload *payload = make_node(node_owner, static_cast<StringStructPayload *>(0));
    payload->text = static_cast<char *>(malloc(1));
    assert(payload->text);
    blocks[block_count].memory = payload->text;
    blocks[block_count].frees = 0;
    blocks[block_count++].heap_owned = node_owner != 0;
    entry->payload = payload;
    entry->next = entry->previous = entry;
    list->head_ = list->current_ = entry;
    list->entry_count_ = 1;
    list->allocator_ = node_owner;
    StringAllocationBase *base = list;
    delete base;
    assert(!StringAllocationHeap);
    for (int i = 0; i < block_count; ++i)
        assert(blocks[i].frees == (blocks[i].heap_owned ? 0 : 1));
    reclaim();
}

int main() {
    assert(sizeof(StringStructPayload) == 0x18 && sizeof(StringStructEntry) == 0x1C);
    Heap heap;
    for (int allocation_mode = 0; allocation_mode != 4; ++allocation_mode)
        check_populated_virtual_delete(allocation_mode & 1 ? &heap : 0,
                                       allocation_mode & 2 ? &heap : 0);
    for (int list_mode = 0; list_mode != 2; ++list_mode) {
        Heap *owner = list_mode ? &heap : 0;
        check_virtual_delete(make_node(owner, static_cast<StringAllocationBase *>(0)), owner, 0);
        check_virtual_delete(make_node(owner, static_cast<StringStruct *>(0)), owner, 0x1C);
        check_virtual_delete(make_node(owner, static_cast<StringList *>(0)), owner, 0x28);
    }
    // Complete, non-deleting destruction publishes the virtual-base owner.
    // No hand-written destroy wrapper or manual virtual-base tail is needed.
    for (int owned = 0; owned != 2; ++owned) {
        Heap *owner = owned ? &heap : 0;
        StringList *list = make_node(owner, static_cast<StringList *>(0));
        list->~StringList();
        assert(StringAllocationHeap == owner && blocks[0].frees == 0);
        StringAllocationHeap = 0;
        reclaim(); // Storage only: explicit teardown already ended lifetime.
    }
    for (int mode = 0; mode < 4; ++mode)
        check_clear(mode & 1 ? &heap : 0, mode & 2 ? &heap : 0, false);
    // The generic list still delivers a null payload to its hook before the
    // language's null-delete guard. StringList's hook requires non-null input.
    check_clear(0, 0, true);
    for (int text_count = 0; text_count <= 3; ++text_count) {
        for (int text_mode = 0; text_mode < 8; ++text_mode)
            check_text_and_nodes(text_mode & 1 ? &heap : 0, (text_mode & 2) != 0,
                                 text_mode & 4 ? &heap : 0, text_count);
    }
    check_virtual_delete(make_node(0, static_cast<StringStructPayload *>(0)), 0, 0x10);
    check_virtual_delete(make_node(&heap, static_cast<StringStructPayload *>(0)), &heap, 0x10);
    check_virtual_delete(make_node(0, static_cast<StringStructEntry *>(0)), 0, 0x14);
    check_virtual_delete(make_node(&heap, static_cast<StringStructEntry *>(0)), &heap, 0x14);

    // Destruction without deallocation publishes the owner and leaves storage.
    StringStructPayload *payload = make_node(&heap, static_cast<StringStructPayload *>(0));
    payload->~StringStructPayload();
    assert(StringAllocationHeap == &heap && blocks[0].frees == 0);
    StringAllocationHeap = 0;
    reclaim();
    StringStructEntry *entry = make_node(&heap, static_cast<StringStructEntry *>(0));
    entry->~StringStructEntry();
    assert(StringAllocationHeap == &heap && blocks[0].frees == 0);
    StringAllocationHeap = 0;
    reclaim();

    StringAllocationHeap = &heap;
    delete static_cast<StringAllocationBase *>(0);
    delete static_cast<StringStruct *>(0);
    delete static_cast<StringList *>(0);
    delete static_cast<StringStructPayload *>(0);
    delete static_cast<StringStructEntry *>(0);
    assert(StringAllocationHeap == &heap);
    StringAllocationHeap = 0;
    StringStruct empty;
    empty.current_position_ = 7;
    empty.remove_all();
    assert(empty.current_position_ == 7); // Empty-head early return is retained.
    for (int count = -1; count <= 0; ++count) {
        StringStructEntry *node = make_node(0, static_cast<StringStructEntry *>(0));
        empty.head_ = empty.current_ = node;
        empty.entry_count_ = count;
        empty.current_position_ = 7;
        empty.remove_all();
        assert(!empty.head_ && !empty.entry_count_ && !empty.current_position_);
        assert(empty.current_ == node && blocks[0].frees == 0);
        delete node;
        reclaim();
    }
    return 0;
}
