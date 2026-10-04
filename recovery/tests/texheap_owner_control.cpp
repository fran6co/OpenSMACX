// Host model for native635050's ordinary count domain. Actual declarations
// and TexHeap bodies are injected by pytest. Heap::get below is a bounded
// allocator model, not a replacement/verification of its original full body.
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <stdint.h>
#include <new>
#include <map>
#include <vector>
typedef uint32_t uint32;
typedef void *LPVOID;
typedef int BOOL;
#define UNUSED(name)
#define private public
struct TexHeap;
// @@DECLARATIONS@@
#undef private

struct Allocation { void *raw; size_t size; };
static std::map<void *, Allocation> live;
static int calls, fail_at, arena_offset;
static std::vector<size_t> sizes;

static void *mem_get(size_t size) {
    sizes.push_back(size);
    if (++calls == fail_at) return 0;
    void *raw = std::malloc(size + 0x20000);
    assert(raw);
    void *result = raw;
    if (size >= 0x10000) {
        uintptr_t boundary = (reinterpret_cast<uintptr_t>(raw) + 0xffff) & ~uintptr_t(0xffff);
        result = reinterpret_cast<void *>(boundary + arena_offset);
    }
    std::memset(result, 0xa5, size);
    Allocation record = {raw, size};
    live[result] = record;
    return result;
}
static void release(void *pointer) {
    if (!pointer) return;
    assert(live.count(pointer) == 1);
    std::free(live[pointer].raw);
    live.erase(pointer);
}
#define free release
// @@BODIES@@
#undef free

LPVOID Heap::get(size_t requested) {
    if (free_size_ < requested) return 0;
    void *result = current_;
    current_ = static_cast<unsigned char *>(current_) + requested;
    free_size_ -= requested;
    return result;
}

static void begin(int offset, int failure = -1) {
    assert(live.empty());
    calls = 0; sizes.clear(); fail_at = failure; arena_offset = offset;
}
static void check_descriptor(Heap *block, unsigned char *base, size_t size) {
    assert(block->err_flags_ == 1);
    assert(block->base_ == base && block->current_ == base);
    assert(block->base_size_ == size && block->free_size_ == size);
}
int main() {
    for (int offset_index = 0; offset_index != 3; ++offset_index) {
        const int offset = offset_index == 0 ? 0 : (offset_index == 1 ? 32 : 65520);
        for (int count = 1; count <= 4; ++count) {
            begin(offset);
            TexHeap heap;
            assert(!heap.base_ && !heap.blocks_ && !heap.count_ && !heap.capacity_);
            assert(heap.initialize(count) == &heap);
            assert(heap.count_ == static_cast<uint32_t>(count));
            assert(heap.capacity_ == static_cast<uint32_t>(count + 1) * 65536u);
            assert(live.size() == static_cast<size_t>(count + 3));
            assert(sizes[0] == heap.capacity_);
            assert(sizes[1] == static_cast<size_t>(count + 1) * sizeof(Heap *));
            unsigned char *boundary = heap.base_ + 65536 - offset;
            check_descriptor(heap.blocks_[0], heap.base_, 65536 - offset);
            for (int index = 1; index < count; ++index) {
                check_descriptor(heap.blocks_[index], boundary, 65535);
                boundary += 65536;
            }
            check_descriptor(heap.blocks_[count], boundary,
                             static_cast<size_t>(heap.base_ + heap.capacity_ - boundary));
            assert(heap.get_mem(8) == heap.base_);
            Heap *aligned = heap.blocks_[1];
            void *expected = aligned->free_size_ >= 8 ? aligned->current_ : 0;
            assert(heap.get_tex_mem(8) == expected);
            assert(heap.configure(2) == 0);
            assert(live.size() == 5);
            heap.shutdown();
            assert(live.empty());
            assert(!heap.base_ && !heap.blocks_ && !heap.count_ && !heap.capacity_);
        }
    }
    // Native zero count overwrites the first descriptor, leaving one leak.
    begin(32);
    TexHeap zero;
    assert(zero.configure(0) == 0);
    assert(live.size() == 4);
    zero.shutdown();
    assert(live.size() == 1);
    release(live.begin()->first);

    // First two null branches have initialized storage and are independently
    // exercisable. Later native null branches read untouched malloc slots;
    // mem_get normally terminates instead, so do not claim they are safe.
    begin(0, 1);
    TexHeap no_arena;
    assert(no_arena.configure(2) == 4);
    assert(!no_arena.base_ && no_arena.count_ == 2 && no_arena.capacity_ == 196608);
    assert(live.empty());
    begin(0, 2);
    TexHeap no_table;
    assert(no_table.configure(2) == 4);
    assert(!no_table.base_ && !no_table.blocks_);
    assert(no_table.count_ == 2 && no_table.capacity_ == 196608);
    assert(live.empty());
}
