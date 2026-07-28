#!/usr/bin/env python3
"""The oracle tells a phase-4 work item from a wall the lift will never climb
by MATCHING THE TRAP'S REASON STRING, and those strings are written somewhere
else.

`oracle_trap_is_blocked_construct` in tools/lifted_oracle_fold.h decides whether
a trap becomes SKIP-reached-blocked (the run really did walk into an import, a
callee with no lifted body, or the SEH block - so the static plan's flag
described a live path) or SKIP-trap (a mnemonic nobody has lowered yet). On the
470 extcall-only functions that split was 79 to 0, so getting it wrong is not a
cosmetic matter: a classifier that stopped recognising the dispatcher's reason
would report 79 phase-4 items that do not exist.

The strings it matches are emitted by four other files. Editing one of them -
a reword, a typo fix, a rename during some unrelated change - leaves everything
compiling, every C++ test green, and the verdict silently degraded. Nothing
would notice, because the only place the two halves meet is at runtime under
Wine against the user's own executable.

So this test is the join. It reads the patterns out of the C++ header and the
reason strings out of the emitters, and requires that they still describe each
other IN BOTH DIRECTIONS:

  * every pattern in the header is a string some emitter really produces, so a
    pattern cannot rot into one nothing can match;
  * every reason an emitter produces for a construct the lift cannot model is
    matched by some pattern, so a new one cannot be added and quietly land in
    the SKIP-trap bucket.

It deliberately does NOT re-implement the classifier. The match semantics -
exact for strcmp, prefix for strncmp - are read out of the header's own calls,
so this file holds no second copy of the rules to drift from the first.
"""

from __future__ import annotations

import re
import unittest
from pathlib import Path

TOOLS = Path(__file__).resolve().parent
FOLD = TOOLS / "lifted_oracle_fold.h"

# Which emitter each pattern is claimed to come from. The claim is in a comment
# in the header; this is the machine-checkable half of it. A pattern with no
# entry here fails the test rather than being skipped, so adding one to the
# header without saying where it comes from is caught.
SOURCE_OF = {
    "call to an address with no lifted body": "lift_whole_image.py",
    "fs: displacement outside the modelled TIB": "lifted_tls.h",
    "synthetic import address with no import": "generate_imports.py",
    "unimplemented import ": "generate_imports.py",
    "import ": "lifted_imports.h",
}

# The reason strings that must be classified as blocked constructs, taken from
# the emitters by hand ONCE, here, and checked against the emitters below - so
# a rewording fails at the emitter check rather than passing unnoticed.
EMITTED_BLOCKED = {
    "lift_whole_image.py": [
        "call to an address with no lifted body",
    ],
    "lifted_tls.h": [
        "fs: displacement outside the modelled TIB",
    ],
    "generate_imports.py": [
        "synthetic import address with no import",
        # The shim body is a format string: `"unimplemented import {label}"`.
        # The label is a DLL!function pair, so the fixed part is the prefix.
        "unimplemented import KERNEL32.dll!ExitProcess",
    ],
    "lifted_imports.h": [
        "import argument is not an object of that size inside the image",
        "import array argument is larger than the whole image",
        "import string argument has no terminator before the end of the image",
        "import returned host memory the guest cannot address; the image needs "
        "its canonical base (see opensmacx_guest)",
    ],
}

# Reasons that must NOT be classified as blocked. Everything the shards emit is
# a bare mnemonic, plus the three whole-body reasons lift_whole_image.py writes.
# If one of these were classified as a wall, the 92 un-lowered instructions in
# the image would stop appearing in the phase-4 queue.
EMITTED_LOWERING_REFUSALS = [
    "bt", "loop", "pushal", "popal", "pushfd", "popfd", "pushf", "popf",
    "int3", "cpuid", "out", "cli", "sti",
    "self-modifying section", "fell off the end of the body", "not lowered",
]


def read(name: str) -> str:
    return (TOOLS / name).read_text(encoding="utf-8")


def folded(name: str) -> str:
    """An emitter's text with adjacent C string literals glued and whitespace
    collapsed, so a reason split across source lines is still findable.

    `"\\s*"` cannot glue two ARGUMENTS together, because a comma sits between
    those quotes and a comma is not whitespace.
    """
    return " ".join(re.sub(r'"\s*"', "", read(name)).split())


def needle(reason: str) -> str:
    """The fixed part of a reason. The import shim's label is a format
    substitution, so only the prefix before it can be looked for."""
    if reason.startswith("unimplemented import"):
        return "unimplemented import"
    return " ".join(reason.split())


def c_string(literal: str) -> str:
    """Join the pieces of a possibly-concatenated C string literal."""
    return "".join(re.findall(r'"((?:[^"\\]|\\.)*)"', literal))


def header_patterns() -> list[tuple[str, bool]]:
    """(text, exact) for every pattern oracle_trap_is_blocked_construct uses.

    Read out of the function body rather than restated, so this file cannot
    hold a second copy of the list that agrees with nothing.
    """
    text = read("lifted_oracle_fold.h")
    start = text.index("inline bool oracle_trap_is_blocked_construct")
    end = text.index("\n}", start)
    body = text[start:end]
    patterns: list[tuple[str, bool]] = []
    for match in re.finditer(
            r'std::(strcmp|strncmp)\(reason,\s*((?:"(?:[^"\\]|\\.)*"\s*)+)',
            body):
        patterns.append((c_string(match.group(2)), match.group(1) == "strcmp"))
    return patterns


def matches(patterns: list[tuple[str, bool]], reason: str) -> bool:
    for text, exact in patterns:
        if reason == text if exact else reason.startswith(text):
            return True
    return False


class TrapReasonCoupling(unittest.TestCase):
    def setUp(self) -> None:
        self.patterns = header_patterns()

    def test_the_header_was_parsed_at_all(self) -> None:
        # A parse that silently found nothing would make every other test in
        # this file vacuously pass, which is the classic way a coupling test
        # stops coupling anything.
        self.assertGreaterEqual(len(self.patterns), 5, self.patterns)
        self.assertTrue(any(exact for _, exact in self.patterns))
        self.assertTrue(any(not exact for _, exact in self.patterns))

    def test_every_pattern_names_a_source(self) -> None:
        for text, _ in self.patterns:
            self.assertIn(text, SOURCE_OF,
                          f"pattern {text!r} is matched by the classifier but "
                          f"this test does not say which file emits it")

    def test_every_pattern_still_matches_a_reason_its_emitter_emits(self) -> None:
        """A pattern has to match a REASON, not merely occur in the file.

        This assertion used to be `assertIn(text, read(source))`, and for the
        two prefix patterns that could not fail. `"import "` occurs in prose,
        in comments and in `#include` lines all over `lifted_imports.h`;
        stripping the `import ` prefix off all four of its refusal strings -
        which is exactly the reword this file exists to catch - left it green.
        Measured: only `test_the_emitters_really_emit_those_reasons` fell over.

        So the pattern is now checked against the reasons the emitter is
        PROVEN to emit (proven by that sibling test, which greps the file for
        each one). A pattern that matches none of them is a pattern nothing
        can produce, whatever else the file happens to contain.
        """
        for text, exact in self.patterns:
            source = SOURCE_OF[text]
            covered = [reason for reason in EMITTED_BLOCKED[source]
                       if matches([(text, exact)], reason)]
            self.assertTrue(
                covered,
                f"the classifier matches {text!r} but no reason "
                f"{source} is known to emit "
                f"{'equals' if exact else 'starts with'} it - the verdict has "
                f"silently degraded to SKIP-trap")
            for reason in covered:
                self.assertIn(
                    needle(reason), folded(source),
                    f"{source} no longer emits {reason!r}, the only reason "
                    f"pattern {text!r} was matching")

    def test_every_emitted_blocked_reason_is_classified(self) -> None:
        for reasons in EMITTED_BLOCKED.values():
            for reason in reasons:
                self.assertTrue(
                    matches(self.patterns, reason),
                    f"{reason!r} is a construct the lift cannot model and the "
                    f"classifier does not recognise it")

    def test_the_emitters_really_emit_those_reasons(self) -> None:
        # The list above is hand-written, so it has to be checked against the
        # files or it is just an opinion - and the test above now leans on it,
        # so this is where a reworded reason is actually caught.
        for name, reasons in EMITTED_BLOCKED.items():
            body = folded(name)
            for reason in reasons:
                self.assertIn(needle(reason), body,
                              f"{name} does not emit {needle(reason)!r}")

    def test_lowering_refusals_are_not_classified_as_blocked(self) -> None:
        for reason in EMITTED_LOWERING_REFUSALS:
            self.assertFalse(
                matches(self.patterns, reason),
                f"{reason!r} is an instruction waiting to be lowered, not a "
                f"wall - classifying it as one hides it from the phase-4 queue")

    def test_the_refused_mnemonics_are_the_real_ones(self) -> None:
        # The mnemonics above are only meaningful if the lift really refuses
        # them. Read the manifest the lift writes; skip only if it is absent,
        # because build/ is not in the repository.
        manifest = TOOLS.parent / "build" / "lifted" / "manifest.json"
        if not manifest.exists():
            self.skipTest("build/lifted/manifest.json not generated here")
        import json
        refused = json.loads(manifest.read_text(encoding="utf-8"))[
            "unsupported_mnemonics"]
        self.assertTrue(refused, "the lift refuses nothing, so this list is "
                                 "describing a program that does not exist")
        for mnemonic in refused:
            self.assertFalse(
                matches(self.patterns, mnemonic),
                f"the lift refuses {mnemonic!r} and the classifier calls it a "
                f"blocked construct")

    def test_the_verdict_names_agree_across_the_language_boundary(self) -> None:
        # The same shape of drift one level up. The C++ writes the verdict name
        # into the report; three Python constants in lifted_oracle_summary.py
        # then match on that text to decide which bytes count as evidence.
        # Rename the verdict in oracle_verdict_name and the summary keeps
        # running, prints 0 for the line that exists to make the weaker
        # evidence visible, and silently adds those bytes to `never compared`.
        names = read("lifted_oracle.cpp")
        summary = read("lifted_oracle_summary.py")
        for verdict, enumerator in (("PASS-paths-taken", "OraclePassPathsTaken"),
                                    ("SKIP-reached-blocked",
                                     "OracleSkipReachedBlocked")):
            self.assertIn(f'case {enumerator}: return "{verdict}";', names,
                          f"lifted_oracle.cpp no longer spells {enumerator} "
                          f"{verdict!r}")
            self.assertIn(f'"{verdict}"', summary,
                          f"lifted_oracle_summary.py does not mention "
                          f"{verdict!r}, so its bytes go unreported")
        # ...and the constant the "agreed only on the paths seeds took" line
        # tallies on has to be that exact string. Checking only that the text
        # appears SOMEWHERE in the file is not enough and the gap was measured:
        # setting PATHS_TAKEN_VERDICT to "PASS" left the earlier assertion
        # green, because the name is still spelled out in EXECUTED_VERDICTS a
        # few lines above - while the published line silently switched to
        # counting clean PASSes as weakened evidence.
        self.assertIn('PATHS_TAKEN_VERDICT = "PASS-paths-taken"', summary)
        for tuple_name in ("EXECUTED_VERDICTS", "COMPARED_VERDICTS"):
            start = summary.index(f"{tuple_name} = (")
            end = summary.index(")", start)
            self.assertIn('"PASS-paths-taken"', summary[start:end],
                          f"{tuple_name} omits the qualified pass, so those "
                          f"bytes count as never compared")

    def test_the_old_wording_is_gone(self) -> None:
        # The dispatcher's reason used to be "indirect call to an address with
        # no function", which is also inaccurate: a DIRECT call to an
        # uncatalogued address arrives at the same trap. If both wordings were
        # live at once the classifier would recognise one and miss the other.
        self.assertNotIn("indirect call to an address with no function",
                         read("lift_whole_image.py"))
        self.assertFalse(matches(self.patterns,
                                 "indirect call to an address with no function"))


if __name__ == "__main__":
    unittest.main()
