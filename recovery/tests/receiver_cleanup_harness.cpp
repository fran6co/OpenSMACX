// Live receiver declarations, cleanup and allocation-owner publication are
// inserted by pytest. Sound is a collaborator double, not a recovered factory.
#include <assert.h>
#include <stdlib.h>
#include <new>

struct Heap {};
Heap *StringAllocationHeap;
void *nodes[3], *payloads[3];
int node_frees[3], payload_frees[3], destroyed[3], hooks[3];
int hook_count, destruction_count;

void operator delete(void *memory) {
    if (!memory) return;
    for (int i = 0; i < 3; ++i) {
        if (memory == nodes[i]) ++node_frees[i];
        if (memory == payloads[i]) ++payload_frees[i];
    }
    free(memory);
}

class Sound {
 public:
    virtual void set_fade() {}
    virtual void unused_slot() {}
    virtual ~Sound() {}
};

// LIVE_ALLOCATION_BASE
// LIVE_OWNER_PUBLICATION
#define private public
// LIVE_RECEIVER_DECLARATIONS
#undef private
// LIVE_CLEAR_BODY

int JackalVoiceRx::on_insert(void *) { return 0; }
void JackalVoiceRx::on_remove(void *) {}

struct Payload : Sound {
    int index;
    Payload(int value) : index(value) {}
    ~Payload() {
        assert(hook_count == destruction_count + 1);
        assert(hooks[destruction_count] == index);
        ++destroyed[index];
        ++destruction_count;
    }
};

struct Receiver : JackalVoiceRx {
    virtual void on_remove(void *payload) {
        // The previous node must clear even a non-null Heap handoff before
        // invoking another payload hook; checking only after the loop misses it.
        assert(!StringAllocationHeap);
        assert(head_ && head_->payload_ == payload);
        assert(cursor_ == head_->next_);
        assert(hook_count < 3);
        int index = -1;
        for (int i = 0; i < 3; ++i)
            if (payload && payloads[i] == payload) index = i;
        assert(index == -1 || !destroyed[index]);
        hooks[hook_count++] = index;
    }
};

int main() {
    Receiver receiver;
    assert(sizeof(VoiceReceiverEntry) == 0x1C);
    assert(sizeof(JackalVoiceRx) == 0x24);

    // Null head is a no-op, even with stale counters/cursor.
    receiver.count_ = 7;
    receiver.cursor_index_ = 9;
    VoiceReceiverEntry untouched;
    receiver.cursor_ = &untouched;
    receiver.clear();
    assert(receiver.count_ == 7 && receiver.cursor_index_ == 9);
    assert(receiver.cursor_ == &untouched && !hook_count);

    // Non-null head with a non-positive count clears state without deletion.
    for (int count = -1; count <= 0; ++count) {
        receiver.head_ = &untouched;
        receiver.count_ = count;
        receiver.clear();
        assert(!receiver.head_ && !receiver.count_ && !receiver.cursor_index_);
        assert(receiver.cursor_ == &untouched && !hook_count);
    }

    Heap pool;
    VoiceReceiverEntry *entry[3];
    for (int i = 0; i < 3; ++i) {
        // The middle node belongs to a Heap. Its backing allocation is
        // deliberately retained until after clear; only the owner may free it.
        StringAllocationHeap = i == 1 ? &pool : 0;
        nodes[i] = malloc(sizeof(VoiceReceiverEntry));
        assert(nodes[i]);
        entry[i] = ::new (nodes[i]) VoiceReceiverEntry;
        assert(!StringAllocationHeap);
        assert(entry[i]->allocation_owner_ == (i == 1 ? &pool : 0));
        entry[i]->id_ = i;
        payloads[i] = i < 2 ? new Payload(i) : 0;
        entry[i]->payload_ = static_cast<Sound *>(static_cast<Payload *>(payloads[i]));
    }
    for (int j = 0; j < 3; ++j) {
        entry[j]->next_ = entry[(j + 1) % 3];
        entry[j]->previous_ = entry[(j + 2) % 3];
    }
    receiver.head_ = entry[0];
    receiver.count_ = 3;
    receiver.cursor_index_ = 2;
    receiver.clear();
    assert(hook_count == 3 && hooks[0] == 0 && hooks[1] == 1 && hooks[2] == -1);
    assert(destruction_count == 2 && destroyed[0] == 1 && destroyed[1] == 1);
    assert(payload_frees[0] == 1 && payload_frees[1] == 1);
    assert(node_frees[0] == 1 && node_frees[1] == 0 && node_frees[2] == 1);
    assert(!StringAllocationHeap);
    assert(!receiver.head_ && !receiver.count_ && !receiver.cursor_index_);
    // Original cleanup leaves the cursor at the old circular head. A second
    // clear must not follow that stale pointer or repeat any destruction.
    receiver.clear();
    assert(hook_count == 3 && destruction_count == 2);
    free(nodes[1]);
    return 0;
}
