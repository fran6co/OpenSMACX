#pragma once

#include <windows.h>
#include <stdint.h>

/* 0x24 bytes, not the IDB's 0x10. TWO INDEPENDENT PROOFS FROM THE IMAGE:
   ??0Net builds three NetFifos at 0xE8, 0x10C and 0x130 (see src/net_class.h),
   spaced exactly 0x24 apart; and ??0NetFifo at 0x006339C0 ends
   `lea eax, [esi + 0xc]; push eax; call dword ptr [0x669168]`, which the
   import table resolves to KERNEL32!InitializeCriticalSection - so a
   CRITICAL_SECTION starts at 0xC, and 0xC + 24 == 0x24. */
class NetFifo {
 public:
  NetFifo();
  ~NetFifo();
  void close();
  int add(void *data, unsigned long size, unsigned long key, int type,
          unsigned long extra);
  int add_head(const void *data, unsigned int size, unsigned int key,
               int caller_value, int result);
  int remove_key_caller(void *payload_buffer, unsigned int key,
                        int caller_value, unsigned int *size_out);
  int remove_caller(void *payload_buffer, unsigned int *key_out,
                    int caller_value, unsigned int *size_out);
  int peek_key_caller(void *payload_buffer, unsigned int key,
                      int caller_value, unsigned int *size_out);
  int peek_caller(void *payload_buffer, unsigned int *key_out,
                  int caller_value, unsigned int *size_out);
  int peek_key(void *data, unsigned int key, int *caller_value_out,
               int *size_out);
  int peek_head(void *data, unsigned int *key_out, int *caller_value_out,
                int *size_out);
  int get(void *data, unsigned int *key, int *type, int *len);
  int get(void *data, unsigned int *key, int *type, unsigned int *len);

  // Each original add allocates one 0x18-byte intrusive node; the gets free
  // nodes, while peek operations retain them. Fields are based on actual
  // accesses in the complete queue family, not an IDB size guess.
  struct NetFifoNode {
    int caller_value_;       // +0x0
    uint32_t key_;           // +0x4
    int result_;             // +0x8
    void *data_;             // +0xC
    uint32_t data_size_;     // +0x10
    NetFifoNode *next_;       // +0x14
  };

  NetFifoNode *head_;        // +0x0
  NetFifoNode *tail_;        // +0x4
  uint32_t count_;           // +0x8
  CRITICAL_SECTION lock_;    // +0xC, extent +0x18
};
