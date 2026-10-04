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
    group.head = first; group.tail = third; group.count = 3;
}
int main() {
    WaveControlGroup group;
    std::memset(&group, 0xa5, sizeof(group));
    wave_control_group_ctor_redirect(&group, 0);
    assert(group.enabled == 1 && group.volume == 127);
    assert(!group.head && !group.tail && !group.cursor && group.count == 0);
    assert(group.pad[0] == 0xa5 && group.pad[1] == 0xa5 && group.pad[2] == 0xa5);
    wave_control_group_dtor_redirect(&group, 0);
    assert(deleted_count == 0);
    Wave wave = {1};
    WaveGroupNode sentinel = {0, 0, &wave};
    group.cursor = &sentinel;
    link(group, node(&wave), node(&wave), node(&wave));
    wave_control_group_dtor_redirect(&group, 0);
    assert(deleted_count == 3);
    assert(!group.head && !group.tail && group.count == 0);
    assert(group.cursor == &sentinel);

    WaveGroupNode *first = node(0), *second = node(&wave), *third = node(&wave);
    link(group, first, second, third);
    wave_control_group_dtor_redirect(&group, 0);
    assert(deleted_count == 4); // Null wave stops after releasing that node.
    assert(group.head == second && group.tail == third && group.count == 2);
    assert(second->prev == 0 && second->next == third && third->prev == second);
    assert(group.cursor == &sentinel);
    wave_control_group_dtor_redirect(&group, 0);
    assert(deleted_count == 6 && group.count == 0 && !group.head && !group.tail);
    assert(group.cursor == &sentinel);
    for (unsigned int index = 0; index < allocated_count; ++index) assert(!allocated[index]);
}
