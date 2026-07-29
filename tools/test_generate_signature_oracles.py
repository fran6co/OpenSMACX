#!/usr/bin/env python3
"""Checks for the generated hybrid-runtime oracles.

The dangerous failure here is generating an oracle that CANNOT prove anything
and having its marker counted as a proof anyway. `unproven_recovered` reads the
`PROVEN-AGAINST-ORIGINAL:` markers this file emits, so a marker for a function
the harness cannot actually execute both forms of would lower a published
figure while proving nothing.

That is not hypothetical: the first run generated 63 oracles for functions the
hybrid never redirects, all 63 failed with "cannot suspend redirect", and
because phase-one oracles gate DLL initialisation the game refused to start.
Most of these tests are about what the generator REFUSES to emit.
"""

from __future__ import annotations

import csv
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import generate_signature_oracles as generator

FIELDS = ("address", "name", "size", "redirect_exports", "binary_kind",
          "recovery_state", "source_locations")


def write_functions(directory: Path, rows) -> Path:
    path = directory / "functions.csv"
    with path.open("w", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=FIELDS)
        writer.writeheader()
        for row in rows:
            writer.writerow(row)
    return path


def row(address="0x00400000", name="?thing@@YAHXZ", redirect="thing_export",
        state="source_complete", kind="game", size="40"):
    return {"address": address, "name": name, "size": size,
            "redirect_exports": redirect, "binary_kind": kind,
            "recovery_state": state, "source_locations": "src/x.cpp:1"}


class SelectionTests(unittest.TestCase):
    REDIRECTED = {0x00400000, 0x00400100}

    def candidates(self, rows, proven_rows=()):
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            functions = write_functions(base, rows)
            proven = base / "proven.csv"
            with proven.open("w", newline="") as handle:
                handle.write("address,name,size,mechanism,evidence\n")
                for address in proven_rows:
                    handle.write(f"{address},n,1,m,e\n")
            return generator.candidates(functions, proven, self.REDIRECTED)

    def test_a_redirected_zero_argument_function_is_selected(self):
        found = self.candidates([row()])
        self.assertEqual(1, len(found))
        self.assertEqual("thing", found[0]["symbol"])
        self.assertEqual("int", found[0]["return"])
        self.assertEqual("__cdecl", found[0]["convention"])

    def test_a_function_the_hybrid_DOES_NOT_REDIRECT_is_refused(self):
        # The proof calls one address twice, redirect suspended then restored.
        # With no redirect installed there is no second body to reach and
        # suspend_redirect_at refuses the address. Selecting on the inventory's
        # redirect_exports column instead of the real table produced 39 oracles
        # that ALL failed exactly that way.
        self.assertEqual([], self.candidates([row(address="0x00500000")]))

    def test_integer_arguments_are_accepted_and_driven(self):
        # A non-empty parameter list ends "@Z"; only the (void) form is "XZ".
        # Missing that made every one-argument function invisible.
        found = self.candidates([row(name="?energy_limit@@YAHH@Z")])
        self.assertEqual(1, len(found))
        self.assertEqual(["int"], found[0]["args"])

    def test_a_POINTER_argument_is_refused(self):
        # PAH is `int *`. Integers can be seeded; a pointer needs a staged
        # object, and inventing one proves nothing while looking exactly like a
        # proof. `?draw@@YAXPAH@Z` takes int* and must not be selected.
        self.assertEqual([], self.candidates([row(name="?draw@@YAXPAH@Z")]))

    def test_a_MEMBER_function_is_refused(self):
        # @@Q is a __thiscall member: it needs a `this` this cannot supply.
        # 815 of the redirected unproven functions are these, which is where
        # the next increment of coverage has to come from.
        self.assertEqual([], self.candidates([row(name="?close@Thing@@QAEXXZ")]))

    def test_an_UNRECOVERED_function_is_refused(self):
        # An oracle over one compares the original against itself.
        self.assertEqual([], self.candidates([row(state="unrecovered")]))

    def test_an_ALREADY_PROVEN_function_is_refused(self):
        # Re-proving inflates the count without adding evidence.
        self.assertEqual([], self.candidates([row()], ["0x00400000"]))

    def test_a_sub_function_with_no_mangled_name_is_refused(self):
        self.assertEqual([], self.candidates([row(name="sub_400000")]))

    def test_a_POINTER_return_type_is_refused(self):
        # PAH is `int *`. Comparing a returned pointer across two runs compares
        # addresses, which differ for reasons that have nothing to do with the
        # recovery being right.
        self.assertEqual([], self.candidates([row(name="?thing@@YAPAHXZ")]))

    def test_a_library_function_is_refused(self):
        self.assertEqual([], self.candidates([row(kind="library")]))


class EmissionTests(unittest.TestCase):
    def test_a_marker_is_emitted_for_every_selected_function(self):
        rows = generator.candidates(*self._paths(
            [row(), row(address="0x00400100", name="?other@@YAXXZ")]),
            SelectionTests.REDIRECTED)
        text = generator.emit(rows, claim_proofs=True)
        self.assertIn("// PROVEN-AGAINST-ORIGINAL: 0x00400000", text)
        self.assertIn("// PROVEN-AGAINST-ORIGINAL: 0x00400100", text)

    def test_a_proof_is_NOT_claimed_by_default(self):
        # The oracles cannot yet run to completion: restoring .data/.bss is
        # wrong for a function that allocates, and three runs of three ended in
        # a division by zero inside unrecovered original code. A marker would
        # lower unproven_recovered for a proof that never ran.
        rows = generator.candidates(*self._paths([row()]),
                                    SelectionTests.REDIRECTED)
        self.assertNotIn("PROVEN-AGAINST-ORIGINAL", generator.emit(rows))

    def test_nothing_selected_emits_no_marker(self):
        text = generator.emit([], claim_proofs=True)
        self.assertNotIn("PROVEN-AGAINST-ORIGINAL", text)

    def test_the_original_is_reached_with_the_redirect_SUSPENDED(self):
        text = generator.emit(generator.candidates(*self._paths([row()]), SelectionTests.REDIRECTED))
        self.assertIn("suspend_redirect_at(0x00400000U)", text)
        self.assertIn("resume_redirect_at(0x00400000U)", text)
        # Suspend must come first, or both calls run the recovered body and
        # the oracle compares the recovery against itself - which passes.
        self.assertLess(text.index("suspend_redirect_at(0x00400000U)"),
                        text.index("resume_redirect_at(0x00400000U)"))

    def test_globals_are_compared_not_just_the_return_value(self):
        # Most candidates return void and do their work in .data; a return-only
        # comparison would pass them unconditionally.
        text = generator.emit(generator.candidates(
            *self._paths([row(name="?quiet@@YAXXZ")]), SelectionTests.REDIRECTED))
        self.assertIn("same_globals", text)
        self.assertIn("globals differ", text)

    def test_the_process_is_left_as_it_was_found(self):
        text = generator.emit(generator.candidates(*self._paths([row()]), SelectionTests.REDIRECTED))
        # Two restores: one between the calls so both sides start equal, one
        # after so the oracle does not perturb the game that follows it.
        self.assertEqual(2, text.count("restore(before)"))

    def _paths(self, rows):
        directory = tempfile.mkdtemp()
        base = Path(directory)
        functions = write_functions(base, rows)
        proven = base / "proven.csv"
        proven.write_text("address,name,size,mechanism,evidence\n")
        return functions, proven


class PublishedFileTests(unittest.TestCase):
    def test_the_committed_oracle_is_current(self):
        """`src/generated_signature_oracle.cpp` must match its inputs.

        A stale copy claims proofs for a selection that no longer holds.
        """
        self.assertEqual(0, generator.main(["--check"]))


if __name__ == "__main__":
    unittest.main()
