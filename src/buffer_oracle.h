#pragma once

// Buffer suite of the in-process runtime oracle; registered in
// src/runtime_oracle.cpp and executed by run_runtime_oracles().
bool run_buffer_oracle_suite();

// Deferred suite: exercises Buffer::close's twenty-entry release loop against
// real allocations from the executable heap. Run from run_deferred_oracles().
bool run_buffer_release_suite();
