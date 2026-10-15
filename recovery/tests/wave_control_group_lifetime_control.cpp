// Host semantic controls, not native ABI/array-construction proof.
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <stdint.h>
#include <new>
#define __fastcall
#define nullptr 0
struct Wave { int value; };
// @@DECLARATIONS@@
struct ObservedWaveControlGroup {
    unsigned char enabled;
    unsigned char pad[3];
    unsigned int volume;
    WaveGroupList list_;
};
union GroupStorage {
    long double alignment;
    char bytes[sizeof(WaveControlGroup)];
};
static WaveControlGroup *construct(GroupStorage &storage) {
    return new (storage.bytes) WaveControlGroup();
}
static ObservedWaveControlGroup observe(const GroupStorage &storage) {
    ObservedWaveControlGroup result;
    std::memcpy(&result, storage.bytes, sizeof(result));
    return result;
}
static void clear(WaveControlGroup *group) {
    group->~WaveControlGroup();
}
static void *allocated[64];
static unsigned int allocated_count;
static unsigned int deleted_count;
void *operator new(size_t size) throw(std::bad_alloc) {
    void *memory = std::malloc(size);
    if (!memory) throw std::bad_alloc();
    return memory;
}
void operator delete(void *memory) throw() {
    bool found = false;
    for (unsigned int i = 0; i < allocated_count; ++i) {
        if (allocated[i] == memory) {
            found = true; allocated[i] = 0; break;
        }
    }
    assert(found); // Never release the nonowned Wave or an already freed node.
    ++deleted_count;
    std::free(memory);
}
// @@BODIES@@
static WaveGroupNode *node(Wave *wave) {
    WaveGroupNode *result = new WaveGroupNode;
    allocated[allocated_count++] = result;
    result->prev = 0; result->next = 0; result->wave = wave;
    return result;
}
static void link(WaveControlGroup &group, WaveGroupNode *first, WaveGroupNode *second,
                 WaveGroupNode *third) {
    first->next = second; second->prev = first;
    second->next = third; third->prev = second;
    group.list_.head = first; group.list_.tail = third; group.list_.count = 3;
}
int main() {
    GroupStorage storage;
    std::memset(storage.bytes, 0xa5, sizeof(storage.bytes));
    WaveControlGroup *group = construct(storage);
    assert(group->enabled == 1 && group->volume == 127);
    assert(!group->list_.head && !group->list_.tail && !group->list_.cursor && group->list_.count == 0);
    assert(group->pad[0] == 0xa5 && group->pad[1] == 0xa5 && group->pad[2] == 0xa5);
    clear(group);
    assert(deleted_count == 0);
    Wave wave = {1};
    WaveGroupNode sentinel = {0, 0, &wave};
    group = construct(storage);
    group->list_.cursor = &sentinel;
    link(*group, node(&wave), node(&wave), node(&wave));
    clear(group);
    ObservedWaveControlGroup state = observe(storage);
    assert(deleted_count == 3);
    assert(!state.list_.head && !state.list_.tail && state.list_.count == 0);
    assert(state.list_.cursor == &sentinel);

    WaveGroupNode *first = node(0), *second = node(&wave), *third = node(&wave);
    group = construct(storage);
    link(*group, first, second, third);
    group->list_.cursor = &sentinel;
    clear(group);
    state = observe(storage);
    assert(deleted_count == 4); // Null wave stops after releasing that node.
    assert(state.list_.head == second && state.list_.tail == third && state.list_.count == 2);
    assert(second->prev == 0 && second->next == third && third->prev == second);
    assert(state.list_.cursor == &sentinel);
    group = construct(storage);
    group->list_.head = state.list_.head;
    group->list_.tail = state.list_.tail;
    group->list_.cursor = state.list_.cursor;
    group->list_.count = state.list_.count;
    clear(group);
    state = observe(storage);
    assert(deleted_count == 6 && state.list_.count == 0 && !state.list_.head && !state.list_.tail);
    assert(state.list_.cursor == &sentinel);
    for (unsigned int index = 0; index < allocated_count; ++index) assert(!allocated[index]);
}
