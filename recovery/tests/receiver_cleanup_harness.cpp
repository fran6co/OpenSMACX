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

void observe_removal(JackalVoiceRx *receiver, void *payload) {
    assert(!StringAllocationHeap);
    assert(receiver->head_ && receiver->head_->payload_ == payload);
    assert(receiver->cursor_ == receiver->head_->next_);
    assert(hook_count < 3);
    int index = -1;
    for (int i = 0; i < 3; ++i)
        if (payload && payloads[i] == payload) index = i;
    assert(index == -1 || !destroyed[index]);
    hooks[hook_count++] = index;
}
int JackalVoiceRx::on_insert(void *) { return 0; }
void JackalVoiceRx::on_remove(void *payload) { observe_removal(this, payload); }

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
        observe_removal(this, payload);
    }
};

// Critical-section collaborators count the live queue's lifecycle. These are
// not a replacement implementation of Windows locking or a threading proof.
typedef unsigned int uint32_t;
struct CRITICAL_SECTION { int live, held, reserved[4]; };
int lock_initializations, lock_deletions, lock_entries, lock_leaves;
void InitializeCriticalSection(CRITICAL_SECTION *lock) {
    lock->live = 1; lock->held = 0; ++lock_initializations;
}
void EnterCriticalSection(CRITICAL_SECTION *lock) {
    assert(lock->live && !lock->held); lock->held = 1; ++lock_entries;
}
void LeaveCriticalSection(CRITICAL_SECTION *lock) {
    assert(lock->live && lock->held); lock->held = 0; ++lock_leaves;
}
void DeleteCriticalSection(CRITICAL_SECTION *lock) {
    assert(lock->live && !lock->held); lock->live = 0; ++lock_deletions;
}
// LIVE_QUEUE_DECLARATIONS
NetFifo *active_queue;
void *queue_nodes[3], *queue_payloads[3];
int queue_node_frees[3], queue_payload_frees[3];
void queue_free(void *memory) {
    assert(active_queue && active_queue->lock_.held);
    bool found = false;
    for (int i = 0; i < 3; ++i) {
        if (memory && memory == queue_nodes[i]) {
            assert(active_queue->head_ == memory);
            assert(!active_queue->head_->data_);
            assert(++queue_node_frees[i] == 1);
            found = true;
        }
        if (memory && memory == queue_payloads[i]) {
            assert(++queue_payload_frees[i] == 1);
            found = true;
        }
    }
    assert(found);
    free(memory);
}
#define free queue_free
// LIVE_QUEUE_CONSTRUCTOR
// LIVE_QUEUE_CLOSE
// LIVE_QUEUE_DESTRUCTOR
#undef free

void check_queues() {
    for (int scenario = 0; scenario < 3; ++scenario) {
        for (int j = 0; j < 3; ++j) {
            queue_nodes[j] = queue_payloads[j] = 0;
            queue_node_frees[j] = queue_payload_frees[j] = 0;
        }
        {
            NetFifo queue;
            active_queue = &queue;
            assert(sizeof(NetFifo) == 0x24);
            assert(!queue.head_ && !queue.tail_ && !queue.count_);
            if (scenario) {
                for (int i = 0; i < 3; ++i) {
                    NetFifo::NetFifoNode *node = static_cast<NetFifo::NetFifoNode *>(malloc(sizeof(*node)));
                    assert(node);
                    queue_nodes[i] = node;
                    queue_payloads[i] = i == 1 ? 0 : malloc(4);
                    node->data_ = queue_payloads[i];
                    node->next_ = 0;
                    if (queue.tail_) queue.tail_->next_ = node;
                    else queue.head_ = node;
                    queue.tail_ = node;
                    ++queue.count_;
                }
            }
            if (scenario < 2) {
                queue.close();
                assert(!queue.head_ && !queue.tail_ && !queue.count_);
                assert(queue.lock_.live && !queue.lock_.held);
                queue.close();
            }
            // Scenario 2 leaves populated cleanup to the actual destructor.
        }
        active_queue = 0;
        assert(lock_initializations == scenario + 1 && lock_deletions == scenario + 1);
        assert(lock_entries == lock_leaves);
        for (int k = 0; k < 3; ++k) {
            assert(queue_node_frees[k] == (scenario ? 1 : 0));
            assert(queue_payload_frees[k] == (scenario && k != 1 ? 1 : 0));
        }
    }
}

int main() {
    check_queues();
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

    // Implicit member destruction must drain once before the virtual base
    // republishes the receiver's owner. This invokes the live destructor.
    hook_count = destruction_count = 0;
    for (int k = 0; k < 3; ++k) {
        nodes[k] = payloads[k] = 0;
        node_frees[k] = payload_frees[k] = destroyed[k] = 0;
    }
    StringAllocationHeap = &pool;
    {
        JackalVoiceRx automatic;
        assert(!StringAllocationHeap);
        nodes[0] = malloc(sizeof(VoiceReceiverEntry));
        VoiceReceiverEntry *last = ::new (nodes[0]) VoiceReceiverEntry;
        payloads[0] = new Payload(0);
        last->payload_ = static_cast<Payload *>(payloads[0]);
        last->next_ = last->previous_ = last;
        automatic.head_ = last;
        automatic.count_ = 1;
        automatic.cursor_index_ = 7;
    }
    assert(hook_count == 1 && destruction_count == 1);
    assert(node_frees[0] == 1 && payload_frees[0] == 1);
    assert(StringAllocationHeap == &pool);
    StringAllocationHeap = 0;
    return 0;
}
