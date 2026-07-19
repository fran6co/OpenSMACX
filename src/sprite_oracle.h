#pragma once

// Sprite suite of the in-process runtime oracle; registered in
// src/runtime_oracle.cpp and executed by run_runtime_oracles().
bool run_sprite_oracle_suite();

// Deferred suite: exercises Sprite::close's release path against real
// allocations from the executable's heap, which is only usable after its CRT
// has started. Run from run_deferred_oracles(), not from DllMain.
bool run_sprite_release_suite();
