#include "stdafx.h"
#include <cstring>
#include "net_fifo.h"
#include "general.h"  // mem_get

// ORIGINAL: 0x006339E0 ??1NetFifo@@QAE@XZ 0x006339E0-0x00633A42
// RECOVERY_BINDING: v1 {"evidence": ["pinned span 0x006339E0-0x00633A42", "NetFifo is a 0x24-byte embedded member at Net+0xE8/+0x10C/+0x130", "the body invokes close() then DeleteCriticalSection(&lock_)"], "reason": "The original full destructor body now belongs to the real NetFifo owner; it calls close and destroys the queue lock."}
NetFifo::~NetFifo() {
    close();
    DeleteCriticalSection(&lock_);
}

/*
Purpose: Zero the three head words and initialise the lock the fifo guards.
// ORIGINAL: 0x006339C0 ??0NetFifo@@QAE@XZ 0x006339C0-0x006339DB BYTE_EXACT
// LEVER: PROMOTED out of src/recovered/units/006339c0.cpp, which reached the CRITICAL_SECTION through `(*reinterpret_cast<InitFn *>(g_00669168))(this + 0xc)` - the import slot spelled as a raw address. A plain `InitializeCriticalSection` call compiles to the image's own `call dword ptr [0x669168]`.
// LEVER: layout-corrected the IDB gave NetFifo 0x10 bytes and four uint32_t members. It is 0x24: ??0Net builds three of them at 0xE8, 0x10C and 0x130, spaced 0x24 apart, and this body starts a CRITICAL_SECTION at 0xC, which is 24 bytes. The fourth uint32_t was the head of that lock.
// symbol    ??0NetFifo@@QAE@XZ
// size      27 bytes
// kind      game
Return Value: `this`, which the image leaves in eax
Status: Complete
*/
NetFifo::NetFifo() {
    head_ = 0;
    tail_ = 0;
    count_ = 0;
    InitializeCriticalSection(&lock_);
}

// ---------------------------------------------------------------------------
// THE FIFO QUEUE - the four NetFifo bodies. A queue entry is one 0x18-byte
// allocation: the caller's int at +0, the routing key at +4, the return value
// at +8, a mem_get buffer at +0xC, its length at +0x10 and the intrusive next
// pointer at +0x14. head_ is +0, tail_ +4, count_ +8 and the critical section
// +0xC - the layout net_fifo.h declares.
//
// 0x00669174 and 0x0066917C are the IAT slots for LeaveCriticalSection and
// EnterCriticalSection, so the bodies below call the real functions; the
// image's `call dword ptr [0x66917c]` is the same import-table indirection.
// ---------------------------------------------------------------------------

// ORIGINAL: 0x00633A50 ?close@NetFifo@@QAEXXZ 0x00633A50-0x00633AAB BYTE_EXACT
// TRIED: all four bodies in this block land MISMATCH at instruction 0-1 with the same shape, and it is register allocation, not control flow: the image saves ebx/ebp/esi and dedicates one to the critical-section address (`lea ebp, [esi+0xc]` in the image is `lea ebp, [esi+0xc]` here too, but the image got there AFTER three pushes where this tree needs two). The archive's own transcription of close was 200/229 bytes at best for the sibling get. Naming head_/tail_/count_ and calling the real Enter/LeaveCriticalSection is the faithful form; the missing callee-saved push is not reachable from source form without a dedicated local nobody needs. 2026-09-04.
// size      92 bytes
// prototype void (__thiscall ?close@NetFifo@@QAEXXZ)(NetFifo* this)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
void NetFifo::close() {
  EnterCriticalSection(&lock_);
  while (head_ != 0) {
    NetFifoNode *next = head_->next_;
    if (head_->data_ != 0) {
      free(head_->data_);
    }
    head_->data_ = 0;
    if (head_ != 0) {
      free(head_);
    }
    head_ = next;
  }
  tail_ = 0;
  count_ = 0;
  LeaveCriticalSection(&lock_);
}

// ORIGINAL: 0x00633AB0 ?add@NetFifo@@QAEHPAXKKHK@Z 0x00633AB0-0x00633B9A
// size      235 bytes
// prototype int (__thiscall ?add@NetFifo@@QAEHPAXKKHK@Z)(NetFifo* this, void*, unsigned long, unsigned long, int, unsigned long)
// callers   2   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0050BA30
int NetFifo::add(void *data, unsigned long size, unsigned long key, int type,
                 unsigned long extra) {
  EnterCriticalSection(&lock_);

  // The empty/nonempty choice precedes allocation. Subsequent stores use
  // the live tail again, including after payload allocation/copy.
  NetFifoNode *node;
  if (head_ == 0) {
    node = static_cast<NetFifoNode *>(mem_get(0x18));
    head_ = node;
    if (node == 0) {
      LeaveCriticalSection(&lock_);
      return 4;
    }
  } else {
    node = static_cast<NetFifoNode *>(mem_get(0x18));
    tail_->next_ = node;
    node = tail_->next_;
    if (node == 0) {
      LeaveCriticalSection(&lock_);
      return 4;
    }
  }
  tail_ = node;
  tail_->next_ = 0;

  tail_->key_ = key;
  tail_->caller_value_ = type;
  // Fifth argument is the result DWORD at +8; +14 remains the link.
  tail_->result_ = static_cast<int>(extra);

  if (size != 0) {
    void *const payload = mem_get(size);
    tail_->data_ = payload;
    if (tail_->data_ == 0) {
      LeaveCriticalSection(&lock_);
      return 4;
    }
    if (data != 0) {
      memcpy(tail_->data_, data, size);
    }
    tail_->data_size_ = size;
  }

  ++count_;
  LeaveCriticalSection(&lock_);
  return 0;
}

// ORIGINAL: 0x00633D90 ?get@NetFifo@@QAEHPAXPAIPAH1@Z 0x00633D90-0x00633E75
// symbol    ?get@NetFifo@@QAEHPAXHPAH1@Z
// size      229 bytes
// prototype int (__thiscall ?get@NetFifo@@QAEHPAXPAIPAH1@Z)(NetFifo* this, void*, unsigned int*, int*, int*)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// The queue is walked from the head for a matching key, so this is the
// remove-by-key form; the 0x633F70 overload below pops the head instead.
// 633DBC/633DC0 compares the raw second argument DWORD, not its pointee.
int NetFifo::get(void *data, int key, int *type, int *len) {
  EnterCriticalSection(&lock_);

  NetFifoNode *cur = head_;
  if (cur == 0) {
    LeaveCriticalSection(&lock_);
    return 0;
  }

  NetFifoNode *prev = 0;
  while (cur->key_ != static_cast<uint32_t>(key)) {
    NetFifoNode *next = cur->next_;
    prev = cur;
    if (next == 0) {
      LeaveCriticalSection(&lock_);
      return 0;
    }
    cur = next;
  }

  if (len != 0) {
    *len = static_cast<int>(cur->data_size_);
  }
  if (cur->data_ != 0) {
    if (data != 0) {
      memcpy(data, cur->data_, cur->data_size_);
    }
    if (cur->data_ != 0) {
      free(cur->data_);
    }
    cur->data_ = 0;
  }

  if (prev == 0) {
    head_ = head_->next_;
  } else {
    prev->next_ = cur->next_;
  }

  if (type != 0) {
    *type = cur->caller_value_;
  }
  int result = cur->result_;
  free(cur);
  --count_;
  LeaveCriticalSection(&lock_);
  return result;
}

// ORIGINAL: 0x00633F70 ?get@NetFifo@@QAEHPAXPAIPAHPAI@Z 0x00633F70-0x00634067
// symbol    ?get@NetFifo@@QAEHPAXPAIPAH1@Z
// size      248 bytes
// prototype int (__thiscall ?get@NetFifo@@QAEHPAXPAIPAHPAI@Z)(NetFifo* this, void*, unsigned int*, int*, unsigned int*)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// The unsigned-int length pointer repeats the key type (backref 1); the
// keyed-search overload repeats the int* type (backref 2). Keep the original
// catalogue names as hints, distinct from these emitted-symbol bindings.
// The pop-the-head form: whatever is at the front of the queue goes back, and
// the key comes out with it.
int NetFifo::get(void *data, unsigned int *key, int *type,
                 unsigned int *len) {
  EnterCriticalSection(&lock_);

  NetFifoNode *node = head_;
  if (node == 0) {
    LeaveCriticalSection(&lock_);
    return 0;
  }

  if (len != 0) {
    *len = node->data_size_;
  }
  if (node->data_ != 0 && data != 0) {
    memcpy(data, node->data_, node->data_size_);
    if (node->data_ != 0) {
      free(node->data_);
    }
    node->data_ = 0;
  }
  // 633FD5 reloads the receiver's head after the output copy/free; the
  // selected node still supplies the outputs/result and is the node freed.
  head_ = head_->next_;
  if (key != 0) {
    *key = node->key_;
  }
  if (type != 0) {
    *type = node->caller_value_;
  }
  int result = node->result_;
  free(node);
  --count_;
  LeaveCriticalSection(&lock_);
  return result;
}

// Additional original operations on the same typed FIFO state.
/*
Purpose: Enqueue one entry on this NetFifo: allocate the 0x18-byte node,
         link it at the head, fill the key/caller-value/result words and -
         when a size is given - the copied payload block.

// ORIGINAL: 0x00633BA0 sub_633ba0 0x00633BA0-0x00633C81
// symbol    ?add_head@NetFifo@@QAEHPBXIIHH@Z
// TRIED: extern "C" __stdcall(5 int) verbatim head (ecx used with no stack slot -> reads as __thiscall); rewritten onto the queue.
// Historical seed emitted-symbol fact: @sub_633ba0@28
// size      225 bytes
// prototype
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00645930
// indirect  0x00633BA9 0x00633BC6 0x00633BF0 0x00633C42 0x00633C73
Return Value: 0 on success, 4 when an allocation fails
Status: Complete
*/
int NetFifo::add_head(const void *data, unsigned int size, unsigned int key, int caller_value, int result) {
    EnterCriticalSection(&this->lock_);

    if (this->head_ == 0) {
        NetFifo::NetFifoNode *const node =
            static_cast<NetFifo::NetFifoNode *>(mem_get(sizeof(NetFifo::NetFifoNode)));
        this->head_ = node;
        if (node == 0) {
            LeaveCriticalSection(&this->lock_);
            return 4;
        }
        this->tail_ = node;
        node->next_ = 0;
    } else {
        NetFifo::NetFifoNode *const node =
            static_cast<NetFifo::NetFifoNode *>(mem_get(sizeof(NetFifo::NetFifoNode)));
        if (node == 0) {
            LeaveCriticalSection(&this->lock_);
            return 4;
        }
        node->next_ = this->head_;
        this->head_ = node;
    }

    this->head_->key_ = key;
    this->head_->caller_value_ = caller_value;
    this->head_->result_ = result;

    if (size != 0) {
        void *const buffer = mem_get(size);
        this->head_->data_ = buffer;
        if (this->head_->data_ == 0) {
            LeaveCriticalSection(&this->lock_);
            return 4;
        }
        if (data != 0) {
            memcpy(buffer, data, size);
        }
        this->head_->data_size_ = size;
    }

    this->count_ += 1;
    LeaveCriticalSection(&this->lock_);
    return 0;
}


/*
Purpose: Remove one entry from this NetFifo by its key/caller-value pair:
         copy the payload out when a buffer is supplied, free the node's
         payload block, unlink and free the node.

// ORIGINAL: 0x00633C90 sub_633c90 0x00633C90-0x00633D81
// symbol    ?remove_key_caller@NetFifo@@QAEHPAXIHPAI@Z
// TRIED: nothing yet - MISMATCH #1 push/mov; the PROPOSAL makes the receiver a __thiscall member over the queue.
// Historical seed emitted-symbol fact: @sub_633c90@24
// size      241 bytes
// prototype
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2 0x00645930
// indirect  0x00633C9F 0x00633CAD 0x00633CDE 0x00633CF3 0x00633D71
Return Value: the entry's result word, or 0 when no entry matches
Status: Complete
*/
int NetFifo::remove_key_caller(void *payload_buffer, unsigned int key, int caller_value, unsigned int *size_out) {
    EnterCriticalSection(&this->lock_);
    NetFifo::NetFifoNode *node = this->head_;
    if (node == 0) {
        LeaveCriticalSection(&this->lock_);
        return 0;
    }
    NetFifo::NetFifoNode *previous = 0;
    while (node->key_ != key || node->caller_value_ != caller_value) {
        previous = node;
        node = node->next_;
        if (node == 0) {
            LeaveCriticalSection(&this->lock_);
            return 0;
        }
    }
    if (size_out != 0) {
        *size_out = node->data_size_;
    }
    if (node->data_ != 0) {
        if (payload_buffer != 0) {
            memcpy(payload_buffer, node->data_, node->data_size_);
        }
        if (node->data_ != 0) {
            free(node->data_);
        }
        node->data_ = 0;
    }
    const int result = node->result_;
    if (previous == 0) {
        this->head_ = this->head_->next_;
    } else {
        previous->next_ = node->next_;
    }
    free(node);
    this->count_ -= 1;
    LeaveCriticalSection(&this->lock_);
    return result;
}


/*
Purpose: Remove one entry from this NetFifo by its caller value: hand the
         key back through the second argument when supplied, copy and free
         the payload, unlink and free the node.

// ORIGINAL: 0x00633E80 sub_633e80 0x00633E80-0x00633F65
// symbol    ?remove_caller@NetFifo@@QAEHPAXPAIH1@Z
// TRIED: same head question as sub_633ba0 - the receiver is the queue.
// Historical seed emitted-symbol fact: @sub_633e80@24
// size      229 bytes
// prototype
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00644EF2 0x00645930
// indirect  0x00633E8A 0x00633E97 0x00633EBE 0x00633ED2 0x00633F56
Return Value: the entry's result word, or 0 when no entry matches
Status: Complete
*/
int NetFifo::remove_caller(void *payload_buffer, unsigned int *key_out, int caller_value, unsigned int *size_out) {
    EnterCriticalSection(&this->lock_);

    NetFifo::NetFifoNode *node = this->head_;
    if (node == 0) {
        LeaveCriticalSection(&this->lock_);
        return 0;
    }
    NetFifo::NetFifoNode *previous = 0;
    while (node->caller_value_ != caller_value) {
        previous = node;
        node = node->next_;
        if (node == 0) {
            LeaveCriticalSection(&this->lock_);
            return 0;
        }
    }

    if (size_out != 0) {
        *size_out = node->data_size_;
    }
    if (node->data_ != 0 && payload_buffer != 0) {
        memcpy(payload_buffer, node->data_, node->data_size_);
        if (node->data_ != 0) {
            free(node->data_);
        }
        node->data_ = 0;
    }

    if (previous == 0) {
        this->head_ = this->head_->next_;
    } else {
        previous->next_ = node->next_;
    }

    if (key_out != 0) {
        *key_out = node->key_;
    }
    const int result = node->result_;
    free(node);
    this->count_ -= 1;
    LeaveCriticalSection(&this->lock_);
    return result;
}


/*
Purpose: Peek one entry of this NetFifo by its key/caller-value pair: copy
         the payload out when a buffer is supplied, without unlinking or
         freeing anything.

// ORIGINAL: 0x00634020 sub_634020 0x00634020-0x006340B5
// symbol    ?peek_key_caller@NetFifo@@QAEHPAXIHPAI@Z
// Historical seed emitted-symbol fact: @sub_634020@24
// size      149 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00645930
// indirect  0x00634028 0x00634035 0x0063405B 0x0063406D 0x006340A8
Return Value: the entry's result word, or 0 when no entry matches
Status: Complete
*/
int NetFifo::peek_key_caller(void *payload_buffer, unsigned int key, int caller_value, unsigned int *size_out) {
    EnterCriticalSection(&this->lock_);

    NetFifo::NetFifoNode *node = this->head_;
    if (node == 0) {
        LeaveCriticalSection(&this->lock_);
        return 0;
    }
    while (node->key_ != key || node->caller_value_ != caller_value) {
        node = node->next_;
        if (node == 0) {
            LeaveCriticalSection(&this->lock_);
            return 0;
        }
    }
    if (size_out != 0) {
        *size_out = node->data_size_;
    }
    if (node->data_ != 0 && payload_buffer != 0) {
        memcpy(payload_buffer, node->data_, node->data_size_);
    }
    const int result = node->result_;
    LeaveCriticalSection(&this->lock_);
    return result;
}


/*
Purpose: Peek one entry of this NetFifo by its caller value alone, handing
         the key back through the second argument when supplied.

// ORIGINAL: 0x00634160 sub_634160 0x00634160-0x006341F9
// symbol    ?peek_caller@NetFifo@@QAEHPAXPAIH1@Z
// TRIED: MISMATCH #0 push/mov, prologue differs; same queue receiver.
// Historical seed emitted-symbol fact: @sub_634160@24
// size      153 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00645930
// indirect  0x00634168 0x00634175 0x00634192 0x006341A4 0x006341EC
Return Value: the entry's result word, or 0 when no entry matches
Status: Complete
*/
int NetFifo::peek_caller(void *payload_buffer, unsigned int *key_out, int caller_value, unsigned int *size_out) {
    EnterCriticalSection(&this->lock_);

    NetFifo::NetFifoNode *node = this->head_;
    if (node == 0) {
        LeaveCriticalSection(&this->lock_);
        return 0;
    }
    while (node->caller_value_ != caller_value) {
        node = node->next_;
        if (node == 0) {
            LeaveCriticalSection(&this->lock_);
            return 0;
        }
    }
    if (node == 0) {
        LeaveCriticalSection(&this->lock_);
        return 0;
    }
    if (size_out != 0) {
        *size_out = node->data_size_;
    }
    if (node->data_ != 0 && payload_buffer != 0) {
        memcpy(payload_buffer, node->data_, node->data_size_);
    }
    if (key_out != 0) {
        *key_out = node->key_;
    }
    const int result = node->result_;
    LeaveCriticalSection(&this->lock_);
    return result;
}


/*
Purpose: The keyed queue reader: under the queue lock, find the entry whose
         key matches, copy its payload out, and hand back its caller value,
         result word and size without unlinking the entry.

// ORIGINAL: 0x006340C0 sub_6340c0 0x006340C0-0x00634159
// symbol    ?peek_key@NetFifo@@QAEHPAXIPAH1@Z
// TRIED: the second reader on the queue at Net+0x10C. The two lock slots are the process import table's EnterCriticalSection (0x66917c) and LeaveCriticalSection (0x669174), the same pair the homed NetFifo::get calls by name.
// size      153 bytes
// prototype int (__thiscall sub_6340c0)(NetFifo* this, void*, int, int, int)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// indirect  0x006340C8 0x006340F3 0x00634134 0x0063414C
Status: Complete C++ owner body; original span and prior source lessons retained.
*/
int NetFifo::peek_key(void *data, unsigned int key, int *caller_value_out, int *size_out) {
    EnterCriticalSection(&lock_);
    NetFifoNode *node = head_;
    if (node == 0) {
        LeaveCriticalSection(&lock_);
        return 0;
    }
    while (node->key_ != static_cast<unsigned int>(key)) {
        node = node->next_;
        if (node == 0) {
            LeaveCriticalSection(&lock_);
            return 0;
        }
    }
    if (size_out != 0) {
        *size_out = static_cast<int>(node->data_size_);
    }
    if (node->data_ != 0 && data != 0) {
        memcpy(data, node->data_, node->data_size_);
    }
    if (caller_value_out != 0) {
        *caller_value_out = node->caller_value_;
    }
    const int result = node->result_;
    LeaveCriticalSection(&lock_);
    return result;
}


/*
Purpose: The head-of-queue reader: under the queue lock, copy the front
         entry's payload out and hand back its key, caller value, result word
         and size, again without unlinking it.

// ORIGINAL: 0x00634200 sub_634200 0x00634200-0x0063427E
// symbol    ?peek_head@NetFifo@@QAEHPAXPAIPAH2@Z
// TRIED: a ?sub_634200@@YGEHPAXHHH@Z-shaped thunk in the seed's reading. Its receiver is the queue at Net+0x10C, which is why the member has to be real for the caller's `add ecx, 0x10c` to exist at all. The two lock slots are the process import table's EnterCriticalSection/LeaveCriticalSection pair.
// size      126 bytes
// prototype int (__thiscall sub_634200)(NetFifo* this, void*, int, int, int)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// indirect  0x00634208 0x00634215 0x00634246 0x00634271
Status: Complete C++ owner body; original span and prior source lessons retained.
*/
int NetFifo::peek_head(void *data, unsigned int *key_out, int *caller_value_out, int *size_out) {
    EnterCriticalSection(&lock_);
    NetFifoNode *node = head_;
    if (node == 0) {
        LeaveCriticalSection(&lock_);
        return 0;
    }
    if (size_out != 0) {
        *size_out = static_cast<int>(node->data_size_);
    }
    // Unlike the searched peeks, 63422F/634256/634265/63426B reload this
    // receiver's head after each preceding output/copy, not the entry node.
    if (head_->data_ != 0 && data != 0) {
        memcpy(data, head_->data_, head_->data_size_);
    }
    if (key_out != 0) {
        *key_out = head_->key_;
    }
    if (caller_value_out != 0) {
        *caller_value_out = head_->caller_value_;
    }
    const int result = head_->result_;
    LeaveCriticalSection(&lock_);
    return result;
}
