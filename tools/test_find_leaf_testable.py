#!/usr/bin/env python3
"""Checks for the leaf-testable scanner.

The scanner decides what work is cheap, so its failure mode is not a crash but
a queue with the wrong things in it - and a wrong entry costs a whole recovery
attempt before anything says so. That is not hypothetical: the first version
accepted `BattleWin::stop_timer`, which cannot link into the target that would
test it, because it read the source list out of CMakeLists.txt with a regex
that over-matched.

So these tests are about the three conditions and about the one input that
must never be guessed at.
"""

from __future__ import annotations

import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import find_leaf_testable as scanner
from capstone import CS_ARCH_X86, CS_MODE_32, Cs

SPAN = (0x00400000, 0x00A0C000)


def decode(encoded: str, address: int = 0x00401000):
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    return list(decoder.disasm(bytes.fromhex(encoded), address))


class SourceListTests(unittest.TestCase):
    """Condition (3)'s input comes from the build or not at all."""

    def test_the_object_directory_is_the_source_list(self):
        with tempfile.TemporaryDirectory() as directory:
            build = Path(directory)
            objects = (build / "mingw-i686-release" / "CMakeFiles"
                       / "recovery-leaf-tests.dir" / "src")
            objects.mkdir(parents=True)
            for name in ("win", "basepop", "time_recovery"):
                (objects / f"{name}.cpp.obj").write_bytes(b"")
            self.assertEqual({"win", "basepop", "time_recovery"},
                             scanner.leaf_test_sources(build))

    def test_time_recovery_does_not_make_time_linkable(self):
        # The exact confusion that let BattleWin::stop_timer through: the class
        # is Time, the file that IS linked is time_recovery.cpp, and Time::stop
        # lives in time.cpp which is not.
        with tempfile.TemporaryDirectory() as directory:
            build = Path(directory)
            objects = (build / "mingw-i686-release" / "CMakeFiles"
                       / "recovery-leaf-tests.dir" / "src")
            objects.mkdir(parents=True)
            (objects / "time_recovery.cpp.obj").write_bytes(b"")
            sources = scanner.leaf_test_sources(build)
            self.assertIn("time_recovery", sources)
            self.assertNotIn("time", sources)

    def test_an_unbuilt_tree_is_refused_rather_than_guessed(self):
        # Returning an empty set here would make every candidate fail
        # condition (3) and report "0 remaining", which reads exactly like a
        # finished queue.
        with tempfile.TemporaryDirectory() as directory:
            with self.assertRaises(SystemExit):
                scanner.leaf_test_sources(Path(directory))


class ClassifyTests(unittest.TestCase):
    """Condition (1): what disqualifies a body on its own terms."""

    def classify(self, encoded: str, size: int | None = None):
        instructions = decode(encoded)
        return scanner.classify(instructions, 0x00401000,
                                size if size is not None else len(encoded) // 2,
                                SPAN)[:2]

    def test_a_plain_return_is_clean(self):
        callees, reasons = self.classify("31c0c3")          # xor eax,eax; ret
        self.assertEqual(set(), callees)
        self.assertEqual([], reasons)

    def test_a_field_read_through_a_register_is_not_a_global(self):
        callees, reasons = self.classify("8b8188a83000c3")  # mov eax,[ecx+0x30a88]
        self.assertEqual([], reasons)
        self.assertEqual(set(), callees)

    def test_an_absolute_memory_operand_is_a_global(self):
        # mov eax, [0x009156b0] - a fixed-address binding, hybrid-only.
        _, reasons = self.classify("a1b0569100")
        self.assertTrue(any("absolute global" in one for one in reasons),
                        reasons)

    def test_an_indirect_call_cannot_be_accounted_for(self):
        _, reasons = self.classify("ffd0")                  # call eax
        self.assertIn("indirect call", reasons)

    def test_an_indirect_jump_cannot_be_accounted_for(self):
        # jmp [eax+0xe8] - a virtual dispatch. The callee is not knowable, so
        # condition (2) cannot be decided and the body must be rejected rather
        # than assumed clean.
        _, reasons = self.classify("ffa0e8000000")
        self.assertIn("indirect jump", reasons)

    def test_a_direct_call_is_reported_as_a_callee(self):
        callees, reasons = self.classify("e8fb0f0000")      # call 0x00402000
        self.assertEqual({0x00402000}, callees)
        self.assertEqual([], reasons)

    def test_a_tail_jump_out_of_the_body_is_a_callee(self):
        callees, _ = self.classify("e9fb0f0000")            # jmp 0x00402000
        self.assertEqual({0x00402000}, callees)

    def test_a_jump_inside_the_body_is_not_a_callee(self):
        # Control flow, not a call: a loop head must not be mistaken for a
        # dependency, or every branching body would be rejected.
        callees, reasons = self.classify("ebfe", size=0x40)
        self.assertEqual(set(), callees)
        self.assertEqual([], reasons)


class BindingTests(unittest.TestCase):
    """An address used as a VALUE is testable but must be declared."""

    def bindings(self, encoded: str):
        instructions = decode(encoded)
        return scanner.classify(instructions, 0x00401000,
                                len(encoded) // 2, SPAN)[2]

    def test_an_absolute_immediate_is_flagged_not_rejected(self):
        # cmp ecx, 0x9156b0 - MapWin::UNK1 compares `this` against a global
        # object's address. Nothing is dereferenced, so it can be leaf-tested;
        # writing it hardcodes an image address, so it is a fixed data binding.
        instructions = decode("81f9b0569100")
        callees, reasons, bindings = scanner.classify(
            instructions, 0x00401000, 6, SPAN)
        self.assertEqual([], reasons, "must not be rejected")
        self.assertEqual([0x009156B0], bindings)

    def test_an_ordinary_constant_is_not_a_binding(self):
        self.assertEqual([], self.bindings("83f841"))   # cmp eax, 0x41

    def test_a_conditional_branch_target_is_not_a_binding(self):
        # je 0x00401010 from 0x00401000 - a label inside the body. Excluding
        # only "call" and "jmp" by name left these in, and four queue entries
        # were flagged as carrying fixed-address bindings that were branch
        # targets. Exclusion is by capstone GROUP so every branch form counts.
        self.assertEqual([], self.bindings("0f840a000000"))

    def test_a_short_conditional_branch_is_not_a_binding(self):
        self.assertEqual([], self.bindings("740a"))

    def test_a_call_target_is_not_counted_as_a_binding(self):
        # It is a callee, handled by conditions (2) and (3); counting it here
        # would flag every function that calls anything.
        self.assertEqual([], self.bindings("e8fb0f0000"))


if __name__ == "__main__":
    unittest.main()
