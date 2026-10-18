#include "video.h"
#include "general.h"

// Queue operations are defined separately from their Video callers.
// Video needs out-of-line queue calls while these operations expose
// their internal cleanup paths to the compiler. This source home does
// not identify an original filename or library boundary.

// ORIGINAL: 0x0063E520 sub_63e520 0x0063E520-0x0063E534 BYTE_EXACT
// symbol    ??0VideoFrameQueue@@QAE@XZ
VideoFrameQueue::VideoFrameQueue() : head_(0), tail_(0), count_(0), limit_(-1) {}
// ORIGINAL: 0x0063E540 sub_63e540 0x0063E540-0x0063E58F FILE
// symbol    ?set_limit@VideoFrameQueue@@QAEXH@Z
void VideoFrameQueue::set_limit(int limit) { clear(); limit_ = limit; }

// ORIGINAL: 0x0063E6E0 sub_63e6e0 0x0063E6E0-0x0063E7EE FILE
// symbol    ?allocate@VideoFrameQueue@@QAEHPAPAEHHI@Z
// TRIED: the given definition head (`__stdcall`, no `this`) contradicts the disassembly - `ecx` does real field work (`self[0..3]`) with no matching stack slot, so this is really a `__thiscall` member with 4 stack params, not a 4-arg stdcall free function. Rewritten as a member on a synthetic class - landed FILE-mode because BODY mode's naive brace-balance extraction (`extract_forward_text`) stops at the first top-level `{...}` it meets, which is the class's own declaration block, truncating the out-of-line method definition entirely (measured: NO_COMPILE, "found 0" external symbols). Reproduces the redundant `if (headOld != 0)` re-test before the second `free()` (the original keeps it even though it is statically known true) since dropping it did not change the divergence. Landed at mnemonic_similarity 0.971, byte count already matches (270/270); only the free-path register scheduling around instruction 79 still differs.
int VideoFrameQueue::allocate(unsigned char **payload_out, int byte_count,
                              int frame_id, unsigned int frame_flags) {
    if (payload_out == 0) return 3;
    *payload_out = 0;
    if (count_ == limit_) return 0x14;

    VideoFrameNode *node;
    if (head_ == 0) {
        node = static_cast<VideoFrameNode *>(mem_get(sizeof(VideoFrameNode)));
        head_ = node;
        if (node == 0) return 4;
    } else {
        VideoFrameNode *const appended = static_cast<VideoFrameNode *>(mem_get(sizeof(VideoFrameNode)));
        tail_->next = appended;
        node = tail_->next;
        if (node == 0) return 4;
    }
    tail_ = node;
    node->next = 0;
    node->frame_flags = frame_flags;
    node->frame_id = frame_id;
    // Native zero-byte branch leaves bytes/byte_count untouched.
    // Optional initialization is NOT authorized; malloc state may be invalid.
    if (byte_count != 0) {
        node->bytes = static_cast<unsigned char *>(mem_get(byte_count));
        if (node->bytes == 0) {
            discard_head();
            return 4;
        }
        node->byte_count = static_cast<unsigned int>(byte_count);
    }
    ++count_;
    *payload_out = node->bytes;
    return 0;
}

// ORIGINAL: 0x0063E5E0 sub_63e5e0 0x0063E5E0-0x0063E6DD FILE
// symbol    ?append@VideoFrameQueue@@QAEHPBEHHI@Z
// TRIED: real `__thiscall` member (C0063e5e0::fill(int,int,int,int)) - `this[2]==this[3]` full-guard, mem_get(0x14) in BOTH the empty-list and grow-list arms matching the ASM exactly, a linked-list node {id,val,buf,buflen,next} appended at `this[1]`, and the alloc-failure path unwinding the node it just linked (free buf, unlink, free node, count--). Reproduces the whole body; MISMATCH #3 is the same "no ebp frame" prologue wall seen across this batch (0x005D4890, 0x005DEA20) plus a `cmp [mem],reg` vs `mov reg,[mem]; cmp reg,reg` operand-form choice neither a named local nor inlining changed.
int VideoFrameQueue::append(const unsigned char *source, int byte_count,
                            int frame_id, unsigned int frame_flags) {
    if (count_ == limit_) return 0x14;
    VideoFrameNode *node;
    if (head_ == 0) {
        node = static_cast<VideoFrameNode *>(mem_get(sizeof(VideoFrameNode)));
        head_ = node;
        if (node == 0) return 4;
    } else {
        tail_->next = static_cast<VideoFrameNode *>(mem_get(sizeof(VideoFrameNode)));
        node = tail_->next;
        if (node == 0) return 4;
    }
    tail_ = node;
    node->next = 0;
    node->frame_flags = frame_flags;
    node->frame_id = frame_id;
    if (byte_count != 0) {
        node->bytes = static_cast<unsigned char *>(mem_get(byte_count));
        if (node->bytes == 0) {
            discard_head();
            return 4;
        }
        if (source != 0) memcpy(node->bytes, source, byte_count);
        node->byte_count = static_cast<unsigned int>(byte_count);
    }
    ++count_;
    return 0;
}

// ORIGINAL: 0x0063E7F0 sub_63e7f0 0x0063E7F0-0x0063E81C BYTE_EXACT
// symbol    ?peek@VideoFrameQueue@@QBEPAEPAHPAI@Z
// TRIED: plateaus at MNEMONIC_ONLY 11/18 - eax/edx swap on the `first` branch (image keeps `first` in eax, node-reload in edx; this tree does the opposite). Caching the loaded front value in a named local, and `!=nullptr` vs bare pointer truthiness, both measured with no change - register allocation, not source shape.
unsigned char *VideoFrameQueue::peek(int *next_frame_id,
                                     unsigned int *frame_flags) const {
    if (head_ == 0) return 0;
    if (frame_flags != 0) *frame_flags = head_->frame_flags;
    if (next_frame_id != 0) *next_frame_id = head_->frame_id;
    return head_->bytes;
}

// ORIGINAL: 0x0063E820 sub_63e820 0x0063E820-0x0063E85B FILE
// symbol    ?discard_head@VideoFrameQueue@@QAEXXZ
void VideoFrameQueue::discard_head() {
    VideoFrameNode *const node = head_;
    if (node != 0) {
        if (node->bytes != 0) {
            free(node->bytes);
            node->bytes = 0;
        }
        head_ = head_->next;
        if (node != 0) free(node);
        --count_;
    }
}

// ORIGINAL: 0x0063E590 sub_63e590 0x0063E590-0x0063E5DF FILE
// symbol    ?clear@VideoFrameQueue@@QAEXXZ
void VideoFrameQueue::clear() {
    while (head_ != 0) {
        VideoFrameNode *const node = head_;
        VideoFrameNode *const next = node->next;
        if (node->bytes != 0) {
            free(node->bytes);
            node->bytes = 0;
        }
        free(node);
        head_ = next;
    }
    tail_ = 0;
    count_ = 0;
}


