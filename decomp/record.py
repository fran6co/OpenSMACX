"""Turning a measurement into what the annotation says.

ONE DIRECTION ONLY: a measurement may add a `BYTE_EXACT` claim and may take
one away, and it may move a lesson between the two vocabularies the grammar
allows - but it never invents prose. What a body was matched BY is something
a person writes; what a measurement knows is whether it still matches.

    from decomp.record import stamped, record_match

    stamped(record, verdict)      # the record the measurement implies
    record_match(record, ...)     # measure, then write that record back

WHY `stamped` IS SEPARATE from writing it. A caller that measures a batch
wants to decide what to do with the whole set before touching `src/` - the
coordinator in this project's recovery loop re-measures precisely because an
agent's report is a claim about a run nobody observed. So the pure function
is the one that says what the annotation should become, and putting it on
disk is a second, explicit step.
"""

from __future__ import annotations

from dataclasses import replace
from pathlib import Path

from .asm import AsmComparison, compare_record
from .model import DecompilationState, Tier
from .writer import write_file


def stamped(record: DecompilationState,
            verdict: AsmComparison) -> DecompilationState:
    """The record this measurement implies, without touching the disk.

    THE CLAIM FOLLOWS THE MEASUREMENT, both ways. `BYTE_EXACT` is a ratchet
    claim - "this was proved to reproduce the shipped bytes; fail if it
    stops" - so it is set when a measurement proves it and cleared when one
    does not. A claim that outlived its body is worse than no claim: every
    later run trusts it.

    THE LESSONS FOLLOW TOO, because the grammar admits each on exactly one
    kind of body. A `LEVER:` records the spelling that MADE a match, so it
    is a statement about a body that matches; a `RULED-OUT:` records a
    spelling that did not, so it is a statement about one that does not. A
    body that stops matching turns its levers into ruled-out spellings -
    they are still true, they simply stopped being the thing that worked -
    and a body that starts matching drops its ruled-out list, which the
    match has refuted wholesale.
    """
    matched = verdict.verdict is Tier.BYTE_EXACT
    if matched:
        return replace(record, byte_exact=True, ruled_out=())
    return replace(record, byte_exact=False, levers=(),
                   ruled_out=record.ruled_out
                   + tuple(prose for _key, prose in record.levers))


def record_match(record: DecompilationState, exe: Path | str,
                 compile_commands: Path | str, flags: tuple | str,
                 shared: frozenset = frozenset()) -> AsmComparison:
    """Measure `record`, write what the measurement says, return the verdict.

    The annotation is rewritten only when the measurement changes it, so a
    run over a tree that has not moved touches no file.
    """
    verdict = compare_record(record, exe, compile_commands, flags, shared)
    after = stamped(record, verdict)
    if after != record:
        write_file([after])
    return verdict
