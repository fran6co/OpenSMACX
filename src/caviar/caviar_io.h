#pragma once

// Caviar's shared callback-backed allocation and exact byte-stream reader.
// Keep record parsing and engine message storage in their own owners.
extern "C" int __cdecl vox_read_exact(int fd, void *buffer, int count);
extern "C" int __cdecl vox_alloc_block(int size, void *out_storage);

// A typed slot converts to the storage API's void* without a T**/void** alias
// cast. The shared allocator publishes it before staging any error message.
template<class T>
inline int vox_alloc_typed(int size, T **out) {
    return vox_alloc_block(size, out);
}
