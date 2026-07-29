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

    def test_an_indexed_global_array_is_a_global(self):
        # mov [eax*4 + 0x6970ac], ecx - a global ARRAY. Requiring index == 0
        # let this through, and Dialog::set_def_dialog_text_color writes four
        # of them; a leaf fixture would have stored to an unmapped address.
        _, reasons = self.classify("890c85ac706900")
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


class InexpressibleTests(unittest.TestCase):
    """Some bodies cannot be WRITTEN, which the other conditions never ask."""

    def classify(self, encoded: str):
        instructions = decode(encoded)
        return scanner.classify(instructions, 0x00401000,
                                len(encoded) // 2, SPAN)[1]

    def test_port_output_has_no_cxx_expression(self):
        # out dx, al - the VGA palette upload at 0x005d4240. Privileged, and
        # it would fault in user mode even written as inline assembly.
        reasons = self.classify("ee")
        self.assertTrue(any("no C++ expression" in one for one in reasons),
                        reasons)

    def test_disabling_interrupts_has_no_cxx_expression(self):
        self.assertTrue(any("cli" in one for one in self.classify("fa")))

    def test_port_input_has_no_cxx_expression(self):
        self.assertTrue(any("no C++ expression" in one
                            for one in self.classify("ec")))

    def test_an_ordinary_body_is_unaffected(self):
        # The guard must not catch anything a normal function does; `in` and
        # `out` are short mnemonics and a sloppy substring test would match
        # plenty of instructions that are perfectly recoverable.
        self.assertEqual([], self.classify("8b4148c3"))
        self.assertEqual([], self.classify("0faf4104c3"))   # imul eax,[ecx+4]


class ImageAddressInARegisterTests(unittest.TestCase):
    """An absolute global does not have to be a displacement."""

    def classify(self, encoded: str):
        instructions = decode(encoded)
        return scanner.classify(instructions, 0x00401000,
                                len(encoded) // 2, SPAN)[1]

    def test_writing_through_a_register_holding_an_image_address(self):
        # mov edi,0x90db24 / rep stosd - eleven dwords into a global. The
        # displacement check never sees it, because the operand has a base.
        reasons = self.classify("bf24db9000f3ab")
        self.assertTrue(any("through a register" in one for one in reasons),
                        reasons)

    def test_the_taint_survives_a_lea(self):
        # ?clear_monuments@@YAXXZ: mov edx,0x94cea0 / lea eax,[edx-0x488] /
        # mov dword [eax],0 - the store is two steps from the immediate.
        reasons = self.classify("ba a0ce9400 8d8278fbffff c70000000000".replace(" ", ""))
        self.assertTrue(any("through a register" in one for one in reasons),
                        reasons)

    def test_an_image_address_that_is_only_PUSHED_is_still_just_a_binding(self):
        # sub_59d230 pushes 0x6900c4 as an argument and never dereferences it.
        # That is a fixed data binding, which is a different and lesser thing.
        reasons = self.classify("68c400690068feffffff")
        self.assertFalse(any("through a register" in one for one in reasons),
                         reasons)

    def test_storing_an_image_address_as_a_VALUE_is_not_a_dereference(self):
        # ??0ImageButton publishes two vtable addresses into its own object:
        # mov dword [ecx],0x670a94. The address is written, not read through.
        reasons = self.classify("c701940a6700")
        self.assertFalse(any("through a register" in one for one in reasons),
                         reasons)

    def test_an_ordinary_this_dereference_is_untouched(self):
        self.assertEqual([], self.classify("8b4148c3"))

    def test_an_address_outside_the_image_does_not_taint(self):
        # mov edi,0x1000 / mov [edi],eax - not an image address, so this rule
        # has nothing to say about it.
        reasons = self.classify("bf00100000 8907".replace(" ", ""))
        self.assertFalse(any("through a register" in one for one in reasons),
                         reasons)

    def test_overwriting_a_tainted_register_clears_it(self):
        # mov edi,0x90db24 / mov edi,ecx / mov [edi],eax - EDI no longer holds
        # the image address, and a tracker that never forgot would reject a
        # recoverable body.
        reasons = self.classify("bf24db9000 89cf 8907".replace(" ", ""))
        self.assertFalse(any("through a register" in one for one in reasons),
                         reasons)


class UnwindFuncletTests(unittest.TestCase):
    """A body that reads EBP without setting it up is not callable."""

    def test_reading_ebp_without_a_prologue_is_a_funclet(self):
        # mov ecx,[ebp-0x10] - the shape MSVC emits into 0x0065xxxx for EH
        # unwind. EBP belongs to the frame being unwound.
        self.assertTrue(scanner.inherits_a_frame(decode("8b4df0")))

    def test_establishing_a_frame_first_is_an_ordinary_function(self):
        # push ebp / mov ebp,esp / mov eax,[ebp+8]
        self.assertFalse(scanner.inherits_a_frame(decode("5589e58b4508")))

    def test_a_body_that_never_touches_ebp_is_fine(self):
        self.assertFalse(scanner.inherits_a_frame(decode("8b4104c3")))


class ImplementationHomeTests(unittest.TestCase):
    """Where a recovered body LIVES, which is condition (3)'s real input."""

    def homes(self, files: dict):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory)
            for name, text in files.items():
                (source / name).write_text(text)
            return scanner.implementation_files(source)

    def test_an_original_offset_comment_names_the_file(self):
        found = self.homes({"win.cpp": "/*\nOriginal Offset: 005EE050\n*/\n"})
        self.assertEqual({0x005EE050: "win"}, found)

    def test_a_class_whose_file_is_not_named_after_it_is_found(self):
        # The case the old name-guessing got wrong: Time's recovered half lives
        # in time_recovery.cpp, and BattleWin::stop_timer's callee Time::stop
        # lives in time.cpp. Guessing "time" from the class found neither
        # reliably.
        found = self.homes({"time_recovery.cpp": "Original Offset: 00616200\n",
                            "time.cpp": "Original Offset: 00616730\n"})
        self.assertEqual("time_recovery", found[0x00616200])
        self.assertEqual("time", found[0x00616730])

    def test_a_free_function_is_found_like_any_other(self):
        # Free functions have no class to guess a filename from, which excluded
        # 349 of 657 rejected candidates under 64 bytes - the largest single
        # reason, and not a property of the code.
        found = self.homes({"alpha.cpp": "Original Offset: 0042A020\n"})
        self.assertEqual({0x0042A020: "alpha"}, found)

    def test_lower_case_offsets_are_read_too(self):
        found = self.homes({"a.cpp": "Original Offset: 0042a020\n"})
        self.assertIn(0x0042A020, found)

    def test_a_file_with_no_offsets_contributes_nothing(self):
        self.assertEqual({}, self.homes({"b.cpp": "int f() { return 0; }\n"}))


class ArityTests(unittest.TestCase):
    """The declaration must clean what the body cleans."""

    def test_no_parameters_and_a_ret_8_disagree(self):
        # ?on_redraw@Win@@QAEHXZ: no parameters declared, eight bytes cleaned.
        # A recovered body popping the declared zero corrupts its caller.
        convention, total = scanner.declared_arity("?on_redraw@Win@@QAEHXZ")
        self.assertEqual(0, total)

    def test_two_ints_declare_eight_bytes(self):
        _, total = scanner.declared_arity("?on_mouse_leave@BaseWin@@QAEXHH@Z")
        self.assertEqual(8, total)

    def test_one_int_declares_four_bytes(self):
        _, total = scanner.declared_arity("?UNK3@BasePop@@QAEXH@Z")
        self.assertEqual(4, total)


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
