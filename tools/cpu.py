"""How wide to run. One answer, so there is no formula left to get wrong.

THREE SPELLINGS DISAGREED, AND ONE OF THEM MEANT SERIAL. Before this module
the tree decided pool width in four places and got a different number in each:

  * `min(16, cpu_count() - 2)`      unit building in decomp_status.py
  * `min(8, cpu_count())`           verify_recovery_abi.py, capped "because
                                    two gate lanes run concurrently, each
                                    already at ctest --parallel 8" - a reason
                                    that retired with the ctest suite
  * `cpu_count()`                   the compile pool in byte_match.py, whose
                                    comment already said "EVERY CORE, not
                                    eight of them"
  * `0`                             the default in two argument parsers

The last is the one that cost real time. `0` reads as "one per core" in the
help text, and every pool guard is spelled `if jobs <= 1: <serial>`, so unless
a caller resolved it first the answer was ONE. `decomp_status.build_units`
resolved it; `byte_match_census.build_units` did not, and its own docstring
recorded the symptom without naming the cause: 665 s of straight-line Python
on a 16-core machine.

A slow default is a bug you can see. A default that silently means SERIAL is
one you cannot, and it recurred four times here, so it is answered once and
imported rather than restated.

This module deliberately imports nothing but `os`: the ABI verifier needs the
answer and has no business loading pefile and capstone to get it.
"""
import os


def worker_count(requested: int = 0) -> int:
    """`requested` if it is positive, otherwise every hardware core.

    A positive number is always obeyed - that is what a `--jobs` flag is for,
    and 1 remains the way to ask for serial. Only 0 or a negative asks this
    question, and the answer is the machine.
    """
    if requested and requested > 0:
        return requested
    return max(1, os.cpu_count() or 1)
