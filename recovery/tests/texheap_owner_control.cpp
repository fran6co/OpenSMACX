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
// Controlled external allocator-boundary effects exercise the pinned read
// phases. They do not assert a callback in the shipped malloc implementation.
static TexHeap *boundary_receiver;
static int boundary_call, boundary_count;
static size_t boundary_base_adjustment;
static uint32_t boundary_capacity_adjustment;
static unsigned char *boundary_original_base;

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
    if (boundary_receiver != 0 && calls == boundary_call) {
        boundary_original_base = boundary_receiver->base_;
        if (boundary_count >= 0) {
            boundary_receiver->count_ = static_cast<uint32_t>(boundary_count);
        }
        boundary_receiver->base_ += boundary_base_adjustment;
        boundary_receiver->capacity_ += boundary_capacity_adjustment;
    }
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

LPVOID Heap::get(int requested) {
    if (free_size_ < requested) return 0;
    void *result = current_;
    current_ = static_cast<unsigned char *>(current_) + requested;
    free_size_ -= requested;
    return result;
}

static void begin(int offset, int failure = -1) {
    assert(live.empty());
    calls = 0; sizes.clear(); fail_at = failure; arena_offset = offset;
    boundary_receiver = 0; boundary_call = -1; boundary_count = -1;
    boundary_base_adjustment = 0; boundary_capacity_adjustment = 0;
    boundary_original_base = 0;
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
    // The terminal slot uses the incoming count, even if that receiver field
    // changes at the final allocation boundary. Restore it before teardown;
    // this control does not normalize the original live cleanup loop bound.
    begin(32);
    TexHeap changed_count;
    boundary_receiver = &changed_count;
    boundary_call = 6;
    boundary_count = 0;
    assert(changed_count.configure(3) == 0);
    assert(changed_count.count_ == 0 && boundary_original_base != 0);
    check_descriptor(changed_count.blocks_[0], boundary_original_base, 65504);
    assert(live.count(changed_count.blocks_[3]) == 1);
    changed_count.count_ = 3;
    changed_count.shutdown();
    assert(live.empty());

    // The terminal extent uses the post-allocation arena base and capacity.
    // The fixture allocation has physical slack for these bounded changes.
    begin(32);
    TexHeap changed_extent;
    boundary_receiver = &changed_extent;
    boundary_call = 6;
    boundary_base_adjustment = 32;
    boundary_capacity_adjustment = 64;
    assert(changed_extent.configure(3) == 0);
    assert(boundary_original_base != 0);
    assert(changed_extent.base_ == boundary_original_base + 32);
    assert(changed_extent.capacity_ == 4u * 65536u + 64u);
    const uintptr_t terminal =
        (reinterpret_cast<uintptr_t>(boundary_original_base) & ~uintptr_t(0xffff))
        + 3u * 65536u;
    check_descriptor(changed_extent.blocks_[3],
                     reinterpret_cast<unsigned char *>(terminal),
                     reinterpret_cast<uintptr_t>(changed_extent.base_)
                         + changed_extent.capacity_ - terminal);
    changed_extent.base_ = boundary_original_base;
    changed_extent.capacity_ = 4u * 65536u;
    changed_extent.shutdown();
    assert(live.empty());

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
