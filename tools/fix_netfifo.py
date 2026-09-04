#!/usr/bin/env -S uv run python
"""Scratch: fix the NetFifo field names in the header and the bodies."""

import pathlib

h = pathlib.Path('src/hypothesis_layouts.h')
t = h.read_text()
t = t.replace("    NetFifoNode *extra_;   // +0x14, the intrusive next pointer",
              "    NetFifoNode *next_;    // +0x14, the intrusive next pointer")
t = t.replace(
    "  NetFifoNode *head_;   // 0x0\n"
    "  uint32_t tail_;       // 0x4, the tail entry's address once the queue is long",
    "  NetFifoNode *head_;   // 0x0\n"
    "  NetFifoNode *tail_;   // 0x4, the last entry enqueued")
h.write_text(t)

p = pathlib.Path('src/net_class.cpp')
t = p.read_text()
t = t.replace(
    "    field_0_ = 0;\n"
    "    field_4_ = 0;\n"
    "    field_8_ = 0;\n"
    "    InitializeCriticalSection(&lock_);",
    "    head_ = 0;\n"
    "    tail_ = 0;\n"
    "    count_ = 0;\n"
    "    InitializeCriticalSection(&lock_);")
t = t.replace("node->extra_ = extra;",
              "node->next_ = reinterpret_cast<NetFifoNode *>(extra);")
t = t.replace("uint32_t tail_ = 0;", "NetFifoNode *tail = 0;")
p.write_text(t)
print('ok')
