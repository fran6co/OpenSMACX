#include "sprite.h"
#include "heap.h"
#include "general.h"
#include <new>
#include <stdlib.h>

// Keep the allocator implementation separate so Sprite and Texture retain
// their original out-of-line allocation calls. Historical object boundaries
// and the original source filename remain unproved.

// Heap metadata borrows the arena. Freeing its storage ends that lifetime;
// invoking Heap::~Heap here would incorrectly free each interior region.
inline void TexHeap::release_allocations() {
    if (base_ != 0) {
        free(base_);
        base_ = 0;
    }

    if (blocks_ != 0) {
        // The image re-reads count_ for the loop test at every iteration, so
        // the bound stays an expression over the field rather than a local.
        for (int index = 0; index < static_cast<int>(count_ + 1u); ++index) {
            Heap *block = blocks_[index];
            if (block != 0) {
                free(block);
                blocks_[index] = 0;
            }
        }
        if (blocks_ != 0) {
            free(blocks_);
        }
        blocks_ = 0;
    }

}

// ORIGINAL: 0x00634FB0 ??0TexHeap@@QAE@XZ 0x00634FB0-0x00634FC0 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:texheap-owner", "Native634FB0fourwordinitialization;634FC0ignores635050statusandreturnsthis;635050arena/table/20byteHeapallocations;635370/6353C0/634FE0sharedmemberconsumers.", "historical source:src/field_accessors.cpp"], "reason": "Recover source-owned typed arena and Heap-pointer table, genuine constructor and borrowed-region Heap lifetimes; initializer returns the native receiver. Preserve allocation order and native omissions, without invented global ownership."}
// symbol    ??0TexHeap@@QAE@XZ
TexHeap::TexHeap() {
    base_ = 0;
    blocks_ = 0;
    count_ = 0;
    capacity_ = 0;
}

// ORIGINAL: 0x00634FC0 ?UNK1@TexHeap@@QAEHH@Z 0x00634FC0-0x00634FD3 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:texheap-owner", "Native634FB0fourwordinitialization;634FC0ignores635050statusandreturnsthis;635050arena/table/20byteHeapallocations;635370/6353C0/634FE0sharedmemberconsumers.", "historical source:src/recovered/00634fc0.cpp"], "reason": "Recover source-owned typed arena and Heap-pointer table, genuine constructor and borrowed-region Heap lifetimes; initializer returns the native receiver. Preserve allocation order and native omissions, without invented global ownership."}
// symbol    ?initialize@TexHeap@@QAEPAU1@H@Z
TexHeap *TexHeap::initialize(int count) {
    configure(count);
    return this;
}

// ORIGINAL: 0x00634FE0 ?shutdown@TexHeap@@QAEXXZ 0x00634FE0-0x00635046 BYTE_EXACT
// size      102 bytes
// prototype void (__thiscall ?shutdown@TexHeap@@QAEXXZ)(TexHeap* this)
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00644EF2
// 0x00634FE0  ?shutdown@TexHeap@@QAEXXZ  ->  ?shutdown@TexHeap@@QAEXXZ
//
// Purpose: cut the whole sheet heap down - the base block, then the block
// TABLE (count_ + 1 Heap entries, each released before the table itself),
// then the three counters.


void TexHeap::shutdown() {
    release_allocations();
    count_ = 0;
    capacity_ = 0;
}

// ORIGINAL: 0x00635050 ?UNK2@TexHeap@@QAEHH@Z 0x00635050-0x00635370
// RECOVERY_BINDING: v1 {"evidence": ["task:texheap-owner", "Native634FB0fourwordinitialization;634FC0ignores635050statusandreturnsthis;635050arena/table/20byteHeapallocations;635370/6353C0/634FE0sharedmemberconsumers.", "historical source:src/unrecovered/00635050.cpp"], "reason": "Recover source-owned typed arena and Heap-pointer table, genuine constructor and borrowed-region Heap lifetimes; initializer returns the native receiver. Preserve allocation order and native omissions, without invented global ownership."}
// symbol    ?configure@TexHeap@@QAEHH@Z
// TRIED: MISMATCH #3 mov/push - the four-register prologue (ebx/ebp/esi/edi) the original uses for this heavily-inlined free/loop/alloc body is not reproduced by writing the four inlined cleanup sites as a shared macro; register pressure differs from the start.
int TexHeap::configure(int count) {
    release_allocations();
    count_ = static_cast<uint32_t>(count);
    const uint32_t entries = count_ + 1u;
    capacity_ = entries << 16;
    base_ = static_cast<unsigned char *>(mem_get(capacity_));
    if (base_ == 0) {
        return 4;
    }
    blocks_ = static_cast<Heap **>(mem_get(entries * sizeof(*blocks_)));
    if (blocks_ == 0) {
        if (base_ != 0) {
            free(base_);
        }
        base_ = 0;
        return 4;
    }

    void *storage = mem_get(sizeof(Heap));
    blocks_[0] = static_cast<Heap *>(storage);
    if (storage == 0) {
        release_allocations();
        count_ = 0;
        capacity_ = 0;
        return 4;
    }
    // Actual address alignment, not an executable address or self offset.
    const uintptr_t arena = reinterpret_cast<uintptr_t>(base_);
    uintptr_t cursor = (arena & ~static_cast<uintptr_t>(0xffff)) + 0x10000;
    new (storage) Heap(base_, static_cast<size_t>(cursor - arena));

    int index = 1;
    if (count > 1) {
        do {
            storage = mem_get(sizeof(Heap));
            blocks_[index] = static_cast<Heap *>(storage);
            if (storage == 0) {
                release_allocations();
                count_ = 0;
                capacity_ = 0;
                return 4;
            }
            new (storage) Heap(
                reinterpret_cast<unsigned char *>(cursor), 0xffff);
            cursor += 0x10000;
            ++index;
        } while (index < count);
    }

    storage = mem_get(sizeof(Heap));
    blocks_[count] = static_cast<Heap *>(storage);
    if (storage == 0) {
        release_allocations();
        count_ = 0;
        capacity_ = 0;
        return 4;
    }
    // With count==0 the original overwrites (and leaks) descriptor zero.
    // Preserve that omission; the normal caller domain is not yet proved.
    new (storage) Heap(
        reinterpret_cast<unsigned char *>(cursor),
        static_cast<size_t>(reinterpret_cast<uintptr_t>(base_) + capacity_ - cursor));
    return 0;
}

/*
Purpose: Hand out the first sheet-heap block whose free tail fits `size` and
         whose `current_` cursor is 64-KiB aligned (low 16 bits zero), returning
         the payload from Heap::get. The walk is get_mem's, with
         one more condition in the filter, and `Texture::extract` is its only
         caller - it takes the returned block as a borrowed texture rather
         than allocating fresh pixels.
// ORIGINAL: 0x00635370 ?get_tex_mem@TexHeap@@QAEHH@Z 0x00635370-0x006353BD
// symbol    ?get_tex_mem@TexHeap@@QAEPAXH@Z
// size      77 bytes
// prototype int (__thiscall ?get_tex_mem@TexHeap@@QAEHH@Z)(TexHeap* this, int)
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D4680
// TRIED (as src/recovered/units/00635370.cpp, raw pointer walks): the
//   artifact reached MISMATCH at instruction 3 with a push-ebp frame and two
//   callee-saved pushes where the image has three and no frame, and its note
//   ruled out a source-form cause. Homed here on the member spelling get_mem
//   already measures 26/33 with, the low-16 test written through the Heap
//   members rather than raw offsets; the frame shape is unchanged by that.
Status: Promoted from src/recovered/units/00635370.cpp.
*/
void *TexHeap::get_tex_mem(int size) {
    if (base_ != 0) {
        int index = 0;
        int count = static_cast<int>(count_ + 1u);
        if (count > 0) {
            Heap **blocks = blocks_;
            for (; index < count; ++index) {
                Heap *block = blocks[index];
                if ((reinterpret_cast<uintptr_t>(block->current_) & 0xffff) == 0 &&
                    static_cast<int>(block->free_size_) >= size) {
                    return blocks[index]->get(size);
                }
            }
        }
    }
    return 0;
}

/*
Purpose: Take `size` bytes of sheet memory: walk the heap's block table and
         allocate from the first block whose free tail is large enough.
// ORIGINAL: 0x006353C0 ?get_mem@TexHeap@@QAEHH@Z 0x006353C0-0x00635402 FILE SEMANTIC
// symbol    ?get_mem@TexHeap@@QAEPAXH@Z
// TRIED 2026-09-03, two batches of four: count declared before index, both
//   for-loop forms (initialiser inside the `for`, index declared outside it),
//   a named `Heap *block` local instead of the double `blocks[index]` access,
//   `count_ + 1` without the static_cast, `1 + static_cast<int>(count_)`, an
//   empty-initialiser `for` over the same pair, and `blocks` hoisted above
//   the count guard. All eight: 26 of 33 or worse. `blocks` above the guard
//   is 18 of 33 and the four restructurings are 5 of 33 with 35 instructions
//   - two MORE than the image, because moving index's initialiser inside the
//   guard makes VC6 emit a second `xor`.
// LEVER (not enough): the committed spelling is the only one at the image's
//   33 instructions, and it is 26 of 33 with MNEMONIC_ONLY 1.000 - the four
//   divergences are ALL the same register swap, the loop index in eax and
//   `count_ + 1` in edx where this tree has them the other way round
//   (`blocks`, `size`, the walking pointer and the block load all agree).
//   Also swept --all-flags: /O2 /Ob0 /GX and /O2 /Gy /GR- all tie at 26 of
//   33; /G6 is 2 of 33. Plateau: a pure allocator coin flip on a 66-byte
//   body, no source relationship left to name.
// size      66 bytes
// prototype int (__thiscall ?get_mem@TexHeap@@QAEHH@Z)(#120* this, int)
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D4680
Status: Promoted 2026-08-29 from src/recovered/units/006353c0.cpp, which
measured MNEMONIC_ONLY against the opaque scaffold and carried no claim. The
`+0x10` read is the block Heap's `free_size_`; the allocation itself is
`Heap::get` (0x005D4680). `Sprite::extract` below is the one caller, and it
only reaches here when a caller passed a heap at all.
*/
void *TexHeap::get_mem(int size) {
    if (base_ != 0) {
        int index = 0;
        int count = static_cast<int>(count_ + 1u);
        if (count > 0) {
            Heap **blocks = blocks_;
            while (index < count) {
                if (static_cast<int>(blocks[index]->free_size_) >= size) {
                    return blocks[index]->get(size);
                }
                ++index;
            }
        }
    }
    return 0;
}

