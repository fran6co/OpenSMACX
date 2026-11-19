#pragma once

// Renderer rank and vendor are shared by CPU detection, preferences and
// renderer selection. They are bytes, not pointers or executable aliases.
extern unsigned char CaviarVoxelAlgoFlag;
extern unsigned char vox_cpu_vendor;

#if defined(__clang__) && defined(_M_IX86) && !defined(OPENSMACX_CPU_TEST_HARDWARE)
__attribute__((no_caller_saved_registers))
#endif
void vox_detect_cpu();
