#pragma once

// Caviar's shared callback-backed allocation and exact byte-stream reader.
// Keep record parsing and engine message storage in their own owners.
extern "C" int __cdecl vox_read_exact(int fd, void *buffer, int count);
extern "C" int __cdecl vox_alloc_block(int size, void *out_storage);

