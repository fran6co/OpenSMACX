#!/usr/bin/env python3
"""Checks for the generated hybrid-runtime oracles.

The dangerous failure here is generating an oracle that CANNOT prove anything
and having its marker counted as a proof anyway. `unproven_recovered` reads the
`PROVEN-AGAINST-ORIGINAL:` markers this file emits, so a marker for a function
the harness cannot actually execute both forms of would lower a published
figure while proving nothing.

That is not hypothetical, twice over. The first run generated 63 oracles for
functions the hybrid never redirects, all 63 failed with "cannot suspend
redirect", and because phase-one oracles gate DLL initialisation the game
refused to start. A later revision emitted 37 markers for functions that had
never been run at all, and promoted docs/recovery/ to match.

So the tests fall into three groups: what the generator REFUSES to emit, that a
marker cannot be minted by generating, and that the two guards which make a
member oracle mean anything are actually in the emitted code. That last group is
written so it FAILS if the guard is removed - a test that cannot fail is worth
nothing.
"""

from __future__ import annotations

import csv
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import generate_signature_oracles as generator

FIELDS = ("address", "name", "size", "prototype", "redirect_exports",
          "binary_kind", "recovery_state", "source_locations")

# A class size the tests can stage against, standing in for a static_assert.
SIZES = {"Thing": 0x40, "Unpinned": 0x40}


def write_functions(directory: Path, rows) -> Path:
    path = directory / "functions.csv"
    with path.open("w", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=FIELDS)
        writer.writeheader()
        for row in rows:
            writer.writerow(row)
    return path


def row(address="0x00400000", name="?thing@@YAHXZ", redirect="thing_export",
        state="source_complete", kind="game", size="40", prototype=""):
    return {"address": address, "name": name, "size": size,
            "prototype": prototype, "redirect_exports": redirect,
            "binary_kind": kind, "recovery_state": state,
            "source_locations": "src/x.cpp:1"}


def member(name="?poke@Thing@@QAEXH@Z", prototype=None, size="40",
           address="0x00400000"):
    if prototype is None:
        prototype = f"void (__thiscall {name})(Thing* this, int)"
    return row(address=address, name=name, size=size, prototype=prototype)


class SelectionTests(unittest.TestCase):
    REDIRECTED = {0x00400000, 0x00400100}

    def candidates(self, rows, proven_rows=(), sizes=None):
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            functions = write_functions(base, rows)
            proven = base / "proven.csv"
            with proven.open("w", newline="") as handle:
                handle.write("address,name,size,mechanism,evidence\n")
                for address in proven_rows:
                    handle.write(f"{address},n,1,m,e\n")
            return generator.candidates(
                functions, proven, self.REDIRECTED,
                SIZES if sizes is None else sizes)

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

    def test_a_one_argument_FREE_function_is_recognised_then_refused(self):
        # "@Z" terminates a non-empty parameter list; only (void) is bare "XZ".
        # Missing that made every one-argument function invisible and measured
        # zero where the answer was 36 - so the pattern is tested even though
        # the domain problem then refuses them.
        self.assertIsNotNone(
            generator.FREE_WITH_ARGS.match("?energy_limit@@YAHH@Z"))
        self.assertEqual([], self.candidates([row(name="?energy_limit@@YAHH@Z")]))

    def test_a_POINTER_argument_to_a_free_function_is_refused(self):
        self.assertEqual([], self.candidates([row(name="?draw@@YAXPAH@Z")]))

    # ---- members: what is now ADMITTED, and every reason one is still not ----

    def test_a_MEMBER_with_integer_arguments_IS_selected(self):
        # The inversion. @@QAE is a __thiscall member; the receiver is staged
        # and the argument types come from the prototype column, not the name.
        found = self.candidates([member()])
        self.assertEqual(1, len(found))
        self.assertEqual("Thing_poke", found[0]["symbol"])
        self.assertEqual("__thiscall", found[0]["convention"])
        self.assertEqual(["int"], found[0]["args"])
        self.assertEqual(0x40, found[0]["object_size"])

    def test_a_MEMBER_taking_no_arguments_IS_selected(self):
        # The safest shape there is: a staged `this` and no argument domain to
        # get wrong. 588 of the unproven members are this shape.
        # Deliberately NOT `close`: that is a lifecycle method and is refused a
        # few tests below, for a reason a real suite run measured.
        found = self.candidates([member(
            name="?refresh@Thing@@QAEXXZ",
            prototype="void (__thiscall ?refresh@Thing@@QAEXXZ)(Thing* this)")])
        self.assertEqual(1, len(found))
        self.assertEqual([], found[0]["args"])
        self.assertEqual("Thing", found[0]["class"])

    def test_a_member_of_a_class_with_NO_PINNED_SIZE_is_refused(self):
        # Without a static_assert against the original there is no true size,
        # and a fixture of the wrong length makes the comparison read past the
        # object. Deriving the size from the C++ type would be circular: the
        # recovered layout is exactly what is under test.
        self.assertEqual([], self.candidates([member()], sizes={}))

    def test_a_member_with_an_EMPTY_prototype_column_is_refused(self):
        # Empty for QAA/__cdecl and QAG/__stdcall members. Nothing to read means
        # nothing to stage.
        self.assertEqual([], self.candidates([member(prototype="")]))

    def test_a_member_that_is_NOT_THISCALL_is_refused(self):
        self.assertEqual([], self.candidates([member(
            name="?timer@Thing@@QAAXH@Z",
            prototype="void (__cdecl ?timer@Thing@@QAAXH@Z)(Thing* this, int)")]))

    def test_a_member_with_a_POINTER_argument_is_refused(self):
        # A pointer argument needs a second staged object, and inventing one
        # proves nothing while looking exactly like a proof.
        self.assertEqual([], self.candidates([member(
            name="?blit@Thing@@QAEXPAURECT@@@Z",
            prototype="void (__thiscall ?blit@Thing@@QAEXPAURECT@@@Z)"
                      "(Thing* this, RECT*)")]))

    def test_a_member_returning_a_POINTER_is_refused(self):
        # Comparing a returned pointer compares addresses, which differ for
        # reasons that have nothing to do with the recovery being right.
        self.assertEqual([], self.candidates([member(
            name="?get@Thing@@QAEPAHH@Z",
            prototype="int* (__thiscall ?get@Thing@@QAEPAHH@Z)"
                      "(Thing* this, int)")]))

    def test_a_member_BELOW_THE_SIZE_FLOOR_is_refused(self):
        # `xor eax,eax / ret` agrees with any transcription of itself. The
        # existing route already claims a 1 B and a 3 B function; that is a
        # wart, not a precedent to extend.
        self.assertEqual([], self.candidates([member(size="8")]))
        self.assertEqual(
            1, len(self.candidates([member(size=str(generator.MinimumBodyBytes))])))

    def test_a_member_of_a_HANDLE_OWNING_class_is_refused(self):
        # Restoring .data does not un-release a wave device, so the second call
        # operates on a handle the first one freed.
        for class_name in ("AutoSound", "Midi", "Wave"):
            with self.subTest(class_name=class_name):
                sizes = dict(SIZES)
                sizes[class_name] = 0x40
                self.assertEqual([], self.candidates([member(
                    name=f"?close@{class_name}@@QAEXXZ",
                    prototype=f"void (__thiscall ?close@{class_name}@@QAEXXZ)"
                              f"({class_name}* this)")], sizes=sizes))

    def test_a_LIFECYCLE_method_is_refused(self):
        # MEASURED, not anticipated. The first suite run died on its first
        # function: ?close@StringStruct@@QAEXXZ (0x00401060) took an unhandled
        # page fault reading 0x00000004 at 0x00401074. A zero-filled receiver is
        # safe only for a body that GUARDS its pointer fields; close() walks a
        # chain the constructor guarantees non-null, so [this+X] read 0 and
        # [0+4] faulted. Teardown frees what it finds and construction
        # allocates; either way the effect escapes the .data snapshot.
        for method in ("close", "init", "free", "destroy", "release"):
            with self.subTest(method=method):
                name = f"?{method}@Thing@@QAEXXZ"
                self.assertEqual([], self.candidates([member(
                    name=name,
                    prototype=f"void (__thiscall {name})(Thing* this)")]))

    def test_a_NON_lifecycle_method_of_the_same_class_is_still_selected(self):
        # The control for the gate above: it must refuse by METHOD, not sweep up
        # the whole class.
        name = "?id_to_pos@Thing@@QAEHH@Z"
        self.assertEqual(1, len(self.candidates([member(
            name=name,
            prototype=f"int (__thiscall {name})(Thing* this, int)")])))

    def test_a_member_of_TIME_is_refused_because_another_thread_writes_it(self):
        # MultimediaProc writes Time objects inside the snapshot window, so a
        # callback landing between two snapshots is a FALSE FAIL.
        sizes = dict(SIZES, Time=0x28)
        self.assertEqual([], self.candidates([member(
            name="?stop@Time@@QAEXXZ",
            prototype="void (__thiscall ?stop@Time@@QAEXXZ)(Time* this)")],
            sizes=sizes))

    # ---- state and kind ----

    def test_an_UNRECOVERED_function_is_refused(self):
        # An oracle over one compares the original against itself.
        self.assertEqual([], self.candidates([row(state="unrecovered")]))

    def test_an_already_proven_function_is_STILL_selected(self):
        # Deliberately not filtered, and the reason is circularity: publishing
        # this generator's own markers makes its functions proven, so filtering
        # on `proven` would select nothing on the next run and leave the
        # committed file permanently stale. Double-counting is not a risk -
        # export_proven_functions.py unions by address and records both
        # mechanisms on one row.
        self.assertEqual(1, len(self.candidates([row()], ["0x00400000"])))

    def test_a_sub_function_with_no_mangled_name_is_refused(self):
        self.assertEqual([], self.candidates([row(name="sub_400000")]))

    def test_a_library_function_is_refused(self):
        self.assertEqual([], self.candidates([row(kind="library")]))

    def test_an_UNSAFE_AT_ORACLE_TIME_address_is_refused(self):
        # help_tech dies on argument 0 and on 1: it needs game state that does
        # not exist when deferred oracles run. Not a domain problem.
        address = sorted(generator.UNSAFE_AT_ORACLE_TIME)[0]
        self.assertEqual([], self.candidates(
            [row(address=f"0x{address:08X}", name="?help_tech@@YAXXZ")],
            ), )


class MarkerTests(unittest.TestCase):
    """Generating must not be able to mint a proof. 37 markers once were."""

    def _rows(self):
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            functions = write_functions(base, [row(), member(
                address="0x00400100", name="?poke@Thing@@QAEXH@Z")])
            proven = base / "proven.csv"
            proven.write_text("address,name,size,mechanism,evidence\n")
            return generator.candidates(functions, proven,
                                        SelectionTests.REDIRECTED, SIZES)

    def test_an_UNEARNED_function_gets_NO_marker(self):
        text = generator.emit(self._rows(), earned=set())
        self.assertNotIn("PROVEN-AGAINST-ORIGINAL", text)
        self.assertIn("carry NO marker", text)

    def test_a_function_recorded_in_proven_csv_KEEPS_its_marker(self):
        # The five free functions in the tree were earned by three runs of
        # three; a regeneration must not silently drop them and shrink
        # proven.csv.
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            proven = base / "proven.csv"
            proven.write_text(
                "address,name,size,mechanism,evidence\n"
                "0x00400000,?thing@@YAHXZ,40,hybrid_runtime,"
                "src/generated_signature_oracle.cpp\n")
            output = base / "generated_signature_oracle.cpp"
            earned = generator.earned_markers(proven, output, None)
        self.assertEqual({0x00400000}, earned)

    def test_a_proven_row_for_a_DIFFERENT_file_does_not_earn_a_marker(self):
        # An island proof or a hand-written suite proves the function, but it is
        # not evidence about THIS file's oracle.
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            proven = base / "proven.csv"
            proven.write_text(
                "address,name,size,mechanism,evidence\n"
                "0x00400000,?thing@@YAHXZ,40,legacy_leaf_island,"
                "build/leaf-census/legacy-leaves.S\n")
            earned = generator.earned_markers(
                proven, base / "generated_signature_oracle.cpp", None)
        self.assertEqual(set(), earned)

    def test_a_PASS_verdict_line_earns_a_marker(self):
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            proven = base / "proven.csv"
            proven.write_text("address,name,size,mechanism,evidence\n")
            verdicts = base / "run.txt"
            verdicts.write_text(
                "GENERATED-ORACLE-VERDICT: 0x00400100 PASS\n")
            earned = generator.earned_markers(
                proven, base / "generated_signature_oracle.cpp", verdicts)
        self.assertEqual({0x00400100}, earned)

    def test_an_INCONCLUSIVE_verdict_line_earns_NOTHING(self):
        # THE LOAD-BEARING TEST. An INCONCLUSIVE result means every seed bailed
        # on a guard and the two bodies agreed without either running. If this
        # ever earns a marker, the route publishes agreement as proof at scale.
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            proven = base / "proven.csv"
            proven.write_text("address,name,size,mechanism,evidence\n")
            verdicts = base / "run.txt"
            verdicts.write_text(
                "GENERATED-ORACLE-VERDICT: 0x00400100 INCONCLUSIVE-no-effect\n"
                "GENERATED-ORACLE-VERDICT: 0x00400200 FAIL\n")
            earned = generator.earned_markers(
                proven, base / "generated_signature_oracle.cpp", verdicts)
        self.assertEqual(set(), earned)


class EmissionTests(unittest.TestCase):
    def _member_text(self):
        return generator.emit(MarkerTests()._rows())

    def test_the_original_is_reached_with_the_redirect_SUSPENDED(self):
        text = generator.emit(generator.candidates(
            *self._paths([row()]), SelectionTests.REDIRECTED, SIZES))
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
            *self._paths([row(name="?quiet@@YAXXZ")]),
            SelectionTests.REDIRECTED, SIZES))
        self.assertIn("same_globals", text)
        self.assertIn("globals differ", text)

    def test_the_process_is_left_as_it_was_found(self):
        text = generator.emit(generator.candidates(
            *self._paths([row()]), SelectionTests.REDIRECTED, SIZES))
        # Two restores: one between the calls so both sides start equal, one
        # after so the oracle does not perturb the game that follows it.
        self.assertEqual(2, text.count("restore(before)"))

    # ---- the two guards, each tested so that removing it FAILS ----

    def test_the_STAGED_OBJECT_is_restored_between_the_two_calls(self):
        # FALSE-PASS 3, and it is silent. The globals snapshot spans
        # 0x00682000..0x009C21F8; a staged object lives outside it. Restore only
        # the globals and the recovered call starts from whatever the original
        # left, so the comparison is A(s0) against R(A(s0)) - and every
        # idempotent setter agrees for free.
        text = self._member_text()
        body = text[text.index("verify_Thing_poke_"):]
        first = body.index("suspend_redirect_at")
        second = body.index("resume_redirect_at")
        between = body[first:second]
        self.assertIn("std::memcpy(staged, staged_seed, ObjectSize)", between,
                      "the fixture is not reseeded between the two calls")
        self.assertIn("std::memcmp(staged_original, staged, ObjectSize)", body,
                      "the object the two sides wrote is never compared")

    def test_a_function_that_produced_NO_EFFECT_reports_INCONCLUSIVE(self):
        # FALSE-PASS 1. Scroll::set_pos opens `if (!parent) return 0;`, so a
        # zero-filled receiver makes both sides bail at instruction three and
        # agree having executed four bytes of a hundred and six. The verdict
        # line is what stops that agreement becoming a marker.
        text = self._member_text()
        self.assertIn("observed_effect", text)
        self.assertIn("INCONCLUSIVE-no-effect", text)
        body = text[text.index("verify_Thing_poke_"):]
        self.assertIn("if (!observed_effect)", body)
        # The effect test must look at what the ORIGINAL did, comparing the
        # pre-call state against the post-call state - not the two sides
        # against each other, which agree precisely when nothing happened.
        self.assertIn("same_globals(before, after_original", body)
        self.assertIn("std::memcmp(staged_seed, staged_original", body)

    def test_the_receiver_is_seeded_with_ZEROS_not_a_pattern(self):
        # A patterned object makes every pointer field a wild address, and the
        # first body that dereferences one takes the game down - which is what
        # help_tech did on function three of thirty-six.
        text = self._member_text()
        self.assertIn("std::memset(staged_seed, 0, ObjectSize)", text)

    def test_the_staged_object_is_STATIC_not_on_the_stack(self):
        # Console is 0x247A8 and MapWin 0x22480, three buffers each, on the
        # game's own thread during startup. A sibling harness already took a
        # STATUS_STACK_OVERFLOW from a stack-reserve change.
        text = self._member_text()
        self.assertIn("static uint8_t staged[ObjectSize]", text)

    def test_the_receiver_is_passed_as_the_first_argument(self):
        text = self._member_text()
        self.assertIn("typedef void (__thiscall *Callable)(void *, int);", text)
        self.assertIn("target(staged, (int)argv[0])", text)

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
