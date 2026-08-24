"""Turning a measurement into what the annotation says.

ONE DIRECTION ONLY: a measurement may add a `BYTE_EXACT` claim and may take
one away, and it may move a lesson between the two vocabularies the grammar
allows - but it never invents prose. What a body was matched BY is something
a person writes; what a measurement knows is whether it still matches.

    from decomp.record import stamped, record_match

    stamped(record, verdict)      # the record the measurement implies
    record_match(record, exe, command, flags)   # measure, then write it

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


def stamped(record: DecompilationState, verdict: AsmComparison,
            demote: bool = False) -> DecompilationState:
    """The record this measurement implies, without touching the disk.

    WHAT IT IMPLIES, NOT WHAT TO WRITE. Removing a claim is a statement
    about the body; whether to act on it is a decision, and `record_match`
    does not take it without being asked. See there.

    THE CLAIM FOLLOWS THE MEASUREMENT, both ways. `BYTE_EXACT` is a ratchet
    claim - "this was proved to reproduce the shipped bytes; fail if it
    stops" - so it is set when a measurement proves it and cleared when one
    does not. A claim that outlived its body is worse than no claim: every
    later run trusts it.

    THE LESSONS FOLLOW TOO, because the grammar admits each on exactly one
    kind of body. A `LEVER:` records the spelling that MADE a match, so it
    is a statement about a body that matches; a `TRIED:` records a
    spelling that did not, so it is a statement about one that does not. A
    body that stops matching turns its levers into tried spellings -
    they are still true, they simply stopped being the thing that worked -
    and a body that starts matching drops its tried list, which the
    match has refuted wholesale.
    """
    matched = verdict.verdict is Tier.BYTE_EXACT
    if matched:
        return replace(record, byte_exact=True, ruled_out=())
    # A CLAIM IS NOT LOWERED WITHOUT BEING ASKED. The floor is the number of
    # claims, so clearing one lowers the floor and the gate then passes
    # because there is nothing left to check - a ratchet quietly ceasing to
    # be one. `docs/DECOMP_MAP.md` is explicit that a claim which stops
    # reproducing is a failure by address and file: "either a tooling change
    # or a lost scaffolding and both need a human". `demote` is how a caller
    # says it has looked. The LESSONS move either way, because a `LEVER` the
    # measurement contradicts is wrong whether or not anyone has decided
    # what to do about the claim.
    if not record.byte_exact:
        # A BODY THAT NEVER MATCHED KEEPS ITS LEVERS. They say "this spelling
        # moved this body toward the image", which is true at any tier - 119
        # of the tree's 248 lever lines sit on bodies that do not reproduce,
        # so this is what the tree actually does, whatever the grammar above
        # says a lever is for. Converting them here turned every one into a
        # `TRIED` reading "this does not work" the first time anyone ran
        # `record` on the body: the exact reverse of what was measured. One of
        # them recorded 4/36 -> 22/36.
        return replace(record, byte_exact=False)
    return replace(record, byte_exact=record.byte_exact and not demote,
                   levers=(),
                   # `f"{key} {prose}"`, NOT `prose`. A lever is stored split
                   # into a leading key and the rest, and dropping the key
                   # ATE THE FIRST WORD of every migrated line: "same as
                   # has_tech" became "as has_tech", "`MEASURED inline`"
                   # became "inline`", "signedness - player_factor" became
                   # "- player_factor".
                   ruled_out=record.ruled_out
                   + tuple(f"{key} {prose}" for key, prose in record.levers))


def record_match(record: DecompilationState, exe: Path | str,
                 command: list[str], flags: tuple | str,
                 shared: frozenset = frozenset(),
                 demote: bool = False) -> AsmComparison:
    """Measure `record`, write what the measurement says, return the verdict.

    ONE RECORD. A caller writing several in the same FILE must not use this
    in a loop: the writer re-reads the file each time, and records read
    before an earlier write can end up pointing at the wrong lines. Measure
    them all, `stamped` them all, and hand the whole set to `write_file`,
    which groups by path and writes each file once. (The sharpest version of
    that hazard is gone - a write no longer canonicalises a wrapped lesson
    onto one line and shifts everything below it - and `writer` now checks
    each record's lesson offsets against the text before trusting them. The
    rule stands anyway: a record is a position in one text.)

    THE DEMOTION IS A ONE-WORD EDIT NOW. `stamped` moves a LEVER into
    `ruled_out`, and the writer re-spells that single word on the author's
    own line, so the wrapping and the parenthetical measurement beside it
    survive the demotion instead of being regenerated.

    A claim is not lowered unless `demote` - see `stamped`.

    The annotation is rewritten only when the measurement changes it, so a
    run over a tree that has not moved touches no file.
    """
    verdict = compare_record(record, exe, command, flags, shared)
    after = stamped(record, verdict, demote)
    if after != record:
        write_file([after])
    return verdict
