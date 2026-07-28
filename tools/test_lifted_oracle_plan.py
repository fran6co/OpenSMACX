#!/usr/bin/env python3
"""Checks for the oracle's plan, and above all for its undefined-flag mask.

The plan decides what the differential oracle compares. Everything in it can
only ever LOSE coverage, and losing coverage is silent: a function that stops
being compared does not fail, it disappears from the failure list, and the
headline number goes UP. Nothing downstream can tell "no divergence" from "not
looked at". So the properties worth asserting here are the ones whose wrong
answer still produces a well-formed plan the oracle runs happily.

`undef=<hex>` is the newest and sharpest instance. It names the flags the
oracle is allowed to ignore for one function, because IDIV/DIV/MUL/IMUL leave
them architecturally undefined and the host's x86 emulation writes something
the lift cannot be expected to match. A mask that is one bit too wide stops
comparing a flag that is perfectly well defined, forever, for that function.

* THE DIRECTION IS THE WHOLE RISK. A flag is masked when it is undefined on a
  path reaching a RET - but a later instruction that REDEFINES it takes it back
  out. Getting that backwards yields a mask that is a superset of the right
  answer, which is exactly the invisible failure: `idiv` then `cmp` is common,
  and masking six flags for every function containing a divide anywhere would
  quietly stop comparing flags across most of the image. The tests therefore
  pin both directions, per flag, including `inc` - which redefines five flags
  and NOT CF, so the mask after `idiv; inc eax; ret` must be exactly CF.

* CF AND OF AFTER IMUL ARE DEFINED. They are the flags multiply exists to
  report, and they are the ones lowering bugs show up in. Capstone additionally
  claims IMUL defines SF, which the SDM contradicts and this host disproves;
  the module overrides it. Both halves are asserted: SF must be masked, CF and
  OF must not.

* A UNION OVER EXITS, NOT ONE EXIT. The oracle cannot know which RET a case
  took, so the mask covers all of them. A body with a `daa` exit (OF undefined)
  and a `test` exit (AF undefined) must yield BOTH bits; either exit alone
  gives a different, plausible, smaller answer.

* MASK 0 MUST LEAVE NO TRACE. The token is absent for an unmasked function,
  which is how a plan written before the token existed keeps meaning "compare
  everything" to the C++ reader. A writer that emitted `undef=0` would be read
  by lifted_oracle_main.cpp as ... 0, and would still be a lie the moment
  anything started matching on the token's presence.

* EVERY SPAN IS WALKED. 402 functions carry a second, outlined span in the cold
  region; this repo has already shipped a bug that read only the first. A body
  that divides in span one and redefines in span two must come out unmasked,
  and a walk that stops at the first span produces the full mask instead.

The scan tests cover the rest of the row for the same reason: `fs` is decided
by naming the segment REGISTER, because testing the mnemonic instead flagged
four functions for an fs: they do not contain (`movsd` starts with "mov"), and
each of those was skipped rather than compared.
"""

from __future__ import annotations

import contextlib
import io
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from capstone import CS_ARCH_X86, CS_MODE_32, Cs  # noqa: E402
from capstone import x86 as capstone_x86  # noqa: E402

import lifted_oracle_plan as plan  # noqa: E402
import lift_whole_image as lift  # noqa: E402

BASE = 0x00401000          # somewhere in .text; nothing depends on the value
COLD = 0x00402000          # the outlined-span region, one section away

DECODER = Cs(CS_ARCH_X86, CS_MODE_32)
DECODER.detail = True

CF = plan.ORACLE_FLAG_BITS["CF"]
PF = plan.ORACLE_FLAG_BITS["PF"]
AF = plan.ORACLE_FLAG_BITS["AF"]
ZF = plan.ORACLE_FLAG_BITS["ZF"]
SF = plan.ORACLE_FLAG_BITS["SF"]
OF = plan.ORACLE_FLAG_BITS["OF"]
DF = plan.ORACLE_FLAG_BITS["DF"]
ARITHMETIC = CF | PF | AF | ZF | SF | OF   # everything a divide leaves undefined


def names(mask: int) -> set:
    """The flag names in a mask, so a failure reads as SF and not as 0x10."""
    return {name for name, bit in plan.ORACLE_FLAG_BITS.items() if mask & bit}


def decode(encoded: str, address: int = BASE) -> list:
    """Every instruction in `encoded`, a hex string, decoded from `address`."""
    instructions = list(DECODER.disasm(bytes.fromhex(encoded), address))
    if not instructions:
        raise AssertionError(f"{encoded} did not decode")
    consumed = sum(one.size for one in instructions)
    if consumed != len(encoded) // 2:
        raise AssertionError(f"{encoded} decoded only {consumed} of "
                             f"{len(encoded) // 2} bytes")
    return instructions


def one_of(encoded: str):
    instructions = decode(encoded)
    if len(instructions) != 1:
        raise AssertionError(f"{encoded} is {len(instructions)} instructions")
    return instructions[0]


def mask_of(*bodies: tuple) -> int:
    """The undefined-exit mask of a body given as (address, hex) spans."""
    instructions = {}
    spans = []
    for address, encoded in bodies:
        for one in decode(encoded, address):
            instructions[one.address] = one
        spans.append((address, address + len(encoded) // 2))
    return plan.undefined_exit_flags(instructions, spans)


# ---------------------------------------------------------------------------
# A PE the tests own outright. `scan` reads bytes through lift.read_bytes and
# nothing else, so the section table plus a raw buffer is the whole contract.
# ---------------------------------------------------------------------------
class FakeSection:
    def __init__(self, name: str, rva: int, size: int, pointer: int):
        self.Name = name.encode("ascii").ljust(8, b"\0")
        self.VirtualAddress = rva
        self.Misc_VirtualSize = size
        self.SizeOfRawData = size
        self.PointerToRawData = pointer


class FakeHeader:
    def __init__(self, base: int):
        self.ImageBase = base


class FakeImport:
    def __init__(self, address: int):
        self.address = address


class FakeImportEntry:
    def __init__(self, addresses):
        self.imports = [FakeImport(a) for a in addresses]


class FakePE:
    """One .text section at 0x00401000, 0x3000 bytes, initially zero.

    The import directory models pefile's `fast_load=True` contract exactly:
    `DIRECTORY_ENTRY_IMPORT` and `DIRECTORY_ENTRY_DELAY_IMPORT` DO NOT EXIST
    until `parse_data_directories()` is called. An earlier version of this
    stub made `parse_data_directories` a no-op and declared no imports, so
    deleting the call from `iat_slots` was invisible - and that deletion takes
    the live IAT slot count from 221 to 0, drops the `iat` flag from all 2,983
    functions that carry it, and hands every one of them to the oracle to
    actually call into DDRAW/USER32/KERNEL32.
    """

    RVA = 0x1000
    SIZE = 0x3000
    # ...and a second, self-modifying section, so `section_of` has something
    # other than ".text" to return and the unliftable-section guard can be
    # driven end to end rather than only through a hand-built dict.
    SELFMOD_RVA = 0x4000
    SELFMOD_SIZE = 0x1000

    def __init__(self, imports=(), delay_imports=()):
        self.OPTIONAL_HEADER = FakeHeader(0x00400000)
        self.sections = [
            FakeSection(".text", self.RVA, self.SIZE, self.RVA),
            FakeSection("_selfmod", self.SELFMOD_RVA, self.SELFMOD_SIZE,
                        self.SELFMOD_RVA),
        ]
        self.__data__ = bytearray(self.SELFMOD_RVA + self.SELFMOD_SIZE)
        self._imports = tuple(imports)
        self._delay_imports = tuple(delay_imports)
        self.parsed = False

    def place(self, address: int, encoded: str) -> None:
        offset = address - self.OPTIONAL_HEADER.ImageBase   # raw pointer == rva
        blob = bytes.fromhex(encoded)
        self.__data__[offset:offset + len(blob)] = blob

    def parse_data_directories(self):
        # Until this runs, a fast-loaded PE has no import directory at all.
        self.parsed = True
        if self._imports:
            self.DIRECTORY_ENTRY_IMPORT = [FakeImportEntry(self._imports)]
        if self._delay_imports:
            self.DIRECTORY_ENTRY_DELAY_IMPORT = [
                FakeImportEntry(self._delay_imports)]


def scanned(*bodies: tuple, slots=frozenset(), entries=(),
            section=".text") -> dict:
    """Run the real `scan` over one function whose spans hold `bodies`."""
    pe = FakePE()
    spans = []
    for address, encoded in bodies:
        pe.place(address, encoded)
        spans.append((address, address + len(encoded) // 2))
    subject = {"address": bodies[0][0], "name": "subject",
               "spans": spans, "section": section}
    functions = [subject]
    for address in entries:
        functions.append({"address": address, "name": f"other_{address:x}",
                          "spans": [(address, address + 1)],
                          "section": ".text"})
        pe.place(address, "c3")
    plan.scan(pe, functions, set(slots))
    return subject


def plan_rows(csv_text: str, bodies: dict, imports=()) -> tuple:
    """Run the real `main()` end to end; return ({name: flags}, stdout).

    Goes through `pefile.PE` -> `iat_slots` -> `scan` -> `close_over_calls` ->
    the writer, so it covers the wiring between them as well as each stage.
    Both the written rows and the printed histogram come back, because some of
    what this tool publishes appears only on stdout.
    """
    pe = FakePE(imports=imports)
    for address, encoded in bodies.items():
        pe.place(address, encoded)
    with tempfile.TemporaryDirectory() as directory:
        root = Path(directory)
        (root / "functions.csv").write_text(csv_text)
        argv = ["lifted_oracle_plan.py", "--exe", str(root / "unused.exe"),
                "--functions", str(root / "functions.csv"),
                "--out", str(root / "plan.tsv")]
        original_pe, original_argv = plan.pefile.PE, sys.argv
        plan.pefile.PE = lambda *args, **kwargs: pe
        sys.argv = argv
        captured = io.StringIO()
        try:
            with contextlib.redirect_stdout(captured):
                if plan.main() != 0:
                    raise AssertionError("plan.main() did not return 0")
            text = (root / "plan.tsv").read_text()
        finally:
            plan.pefile.PE, sys.argv = original_pe, original_argv
    rows = {}
    for line in text.splitlines():
        if line.startswith("#"):
            continue
        address, flags, name = line.split("\t")
        rows[name] = flags
    return rows, captured.getvalue()


class InstructionFlagEffectTests(unittest.TestCase):
    """One instruction at a time: what does it define, what does it wreck.

    This is the input to everything else. A flag wrongly reported undefined
    here becomes a masked flag in every function containing the instruction.
    """

    def test_divide_leaves_the_six_arithmetic_flags_undefined(self):
        # The SDM says CF OF SF ZF AF PF are all undefined after DIV and IDIV,
        # and it defines nothing at all - which is why the measured
        # ?fixed_div@@YAHJJ@Z comes back with its ENTRY flags intact.
        for encoded, text in (("f7f9", "idiv ecx"), ("f7f1", "div ecx"),
                              ("f6f1", "div cl")):
            with self.subTest(text):
                defined, undefined = plan.instruction_flag_effect(one_of(encoded))
                self.assertEqual(set(), names(defined))
                self.assertEqual(names(ARITHMETIC), names(undefined))

    def test_multiply_undefines_four_and_defines_carry_and_overflow(self):
        # CF and OF are the whole point of a multiply's flags: they say the
        # result was truncated. Masking them would delete the only flag
        # evidence a multiply lowering produces.
        #
        # NOTE ON CREDIT, because an earlier report claimed this test proves
        # the module's override: it does not, for three of the four flags.
        # Capstone already reports AF, PF and ZF undefined for both multiplies,
        # and reports all four for MUL. Only IMUL's SF is contributed by
        # MULTIPLY_UNDEFINED - see the test below, which measures exactly that.
        for encoded, text in (("0fafc6", "imul eax, esi"), ("f7e9", "imul ecx"),
                              ("6bc005", "imul eax, eax, 5"),
                              ("f7e1", "mul ecx")):
            with self.subTest(text):
                defined, undefined = plan.instruction_flag_effect(one_of(encoded))
                self.assertEqual({"SF", "ZF", "AF", "PF"}, names(undefined))
                self.assertEqual({"CF", "OF"}, names(defined))
                self.assertFalse(undefined & (CF | OF),
                                 "CF/OF after a multiply are DEFINED")

    def test_the_imul_sign_flag_override_is_still_in_force(self):
        # Capstone reports MODIFY_SF for IMUL. The SDM says undefined, and the
        # host agrees: -5 * 7 comes back with SF CLEAR. Believing capstone
        # reported sub_559210 as a lowering defect in SF. If capstone is ever
        # fixed this test still passes; if the OVERRIDE is dropped it fails.
        instruction = one_of("0fafc6")
        self.assertTrue(instruction.eflags & capstone_x86.X86_EFLAGS_MODIFY_SF,
                        "capstone no longer claims IMUL defines SF - the "
                        "override is now redundant, not wrong")
        _, undefined = plan.instruction_flag_effect(instruction)
        self.assertTrue(undefined & SF)

    def test_exactly_which_bits_the_override_contributes(self):
        """SF for IMUL, and nothing else. Measured, not assumed.

        MULTIPLY_UNDEFINED names four flags, but three of them are bits
        capstone already sets, so zeroing ZF/AF/PF inside it is an EQUIVALENT
        mutation - the tests stay green and the real image's plan is
        byte-identical. That is not a defect (the module says outright that MUL
        is listed "so the two multiplies cannot drift apart"), but it means
        only one bit of that constant is load-bearing today, and it should be
        recorded rather than discovered later. If capstone stops reporting one
        of the other three, this test starts failing and the override becomes
        load-bearing for it too - which is the notification worth having.
        """
        def capstone_undefined(one) -> int:
            mask = 0
            for value, bit in plan._CS_UNDEFINES.items():
                if one.eflags & value:
                    mask |= bit
            return mask

        imul = one_of("0fafc6")
        self.assertEqual({"AF", "PF", "ZF"}, names(capstone_undefined(imul)))
        self.assertEqual({"SF"},
                         names(plan.MULTIPLY_UNDEFINED
                               & ~capstone_undefined(imul)))
        mul = one_of("f7e1")
        self.assertEqual({"AF", "PF", "SF", "ZF"},
                         names(capstone_undefined(mul)))
        self.assertEqual(set(),
                         names(plan.MULTIPLY_UNDEFINED
                               & ~capstone_undefined(mul)))

    def test_a_flag_is_never_both_defined_and_undefined(self):
        # The two sets feed `(incoming & ~defined) | undefined`, where an
        # overlap would mask a flag the same instruction just defined.
        for encoded in ("0fafc6", "f7e1", "f7f9", "85c0", "d3e0", "27",
                        "39c0", "83c001", "40", "fc", "9d"):
            with self.subTest(encoded):
                defined, undefined = plan.instruction_flag_effect(one_of(encoded))
                self.assertEqual(0, defined & undefined)

    def test_ordinary_arithmetic_undefines_nothing(self):
        # If these undefined anything, nearly every function in the image
        # would carry a mask.
        for encoded, text in (("39c0", "cmp eax, eax"), ("83c001", "add eax, 1"),
                              ("89c8", "mov eax, ecx"), ("c3", "ret"),
                              ("90", "nop"), ("fc", "cld")):
            with self.subTest(text):
                _, undefined = plan.instruction_flag_effect(one_of(encoded))
                self.assertEqual(0, undefined)

    def test_the_logic_and_shift_group_undefines_only_the_adjust_flag(self):
        # AF is the one the manuals leave undefined here; CF/OF after a shift
        # are defined and were measured to agree in 651 of 651 cases.
        for encoded, text in (("85c0", "test eax, eax"), ("21c8", "and eax, ecx"),
                              ("31c0", "xor eax, eax"), ("d1e0", "shl eax, 1"),
                              ("d3e0", "shl eax, cl"), ("c1f802", "sar eax, 2")):
            with self.subTest(text):
                defined, undefined = plan.instruction_flag_effect(one_of(encoded))
                self.assertEqual({"AF"}, names(undefined))
                self.assertEqual({"CF", "PF", "ZF", "SF", "OF"}, names(defined))

    def test_the_direction_flag_is_never_undefined_by_anything(self):
        # DF is in the comparison because leaving it out was a false-PASS hole.
        # No instruction in the image's vocabulary leaves it undefined, so a
        # mask that ever contains DF is a bug in this module, not in the ISA.
        for encoded in ("f7f9", "f7e1", "0fafc6", "85c0", "27", "fc", "fd",
                        "f3a5", "9d"):
            with self.subTest(encoded):
                _, undefined = plan.instruction_flag_effect(one_of(encoded))
                self.assertEqual(0, undefined & DF)


class UndefinedExitFlagTests(unittest.TestCase):
    """The dataflow: which of those undefined flags survive to a RET."""

    def test_a_divide_at_the_exit_masks_all_six(self):
        self.assertEqual(names(ARITHMETIC), names(mask_of((BASE, "f7f9" "c3"))))

    def test_a_multiply_at_the_exit_masks_four_and_spares_carry_and_overflow(self):
        mask = mask_of((BASE, "0fafc6" "c3"))
        self.assertEqual({"SF", "ZF", "AF", "PF"}, names(mask))
        self.assertFalse(mask & (CF | OF))

    def test_a_body_with_no_undefining_instruction_masks_nothing(self):
        # 3,410 of the 5,673 functions are this case, and for every one of them
        # the plan must carry no token at all.
        self.assertEqual(0, mask_of((BASE, "89c8" "83c001" "39c0" "c3")))

    def test_an_empty_body_masks_nothing(self):
        self.assertEqual(0, plan.undefined_exit_flags({}, [(BASE, BASE)]))

    def test_a_later_redefinition_takes_the_flag_back_out_of_the_mask(self):
        # THE test. `idiv` then `cmp` is an ordinary shape, and masking six
        # flags for it would silently stop comparing flags in most of the
        # image while every function still PASSed.
        self.assertEqual(0, mask_of((BASE, "f7f9" "39c0" "c3")))

    def test_redefinition_is_per_flag_and_not_all_or_nothing(self):
        # `inc` writes PF AF ZF SF OF and deliberately NOT CF. So exactly one
        # bit survives the divide. A model that treats any flag-writing
        # instruction as a full reset returns 0 here; one that ignores
        # redefinition returns all six.
        self.assertEqual({"CF"}, names(mask_of((BASE, "f7f9" "40" "c3"))))

    def test_a_redefinition_that_leaves_its_own_hole_keeps_that_hole(self):
        # `test` redefines five flags and undefines AF, so the mask after a
        # multiply is AF alone - not the multiply's four, not nothing.
        self.assertEqual({"AF"}, names(mask_of((BASE, "0fafc6" "85c0" "c3"))))

    def test_a_flag_redefined_on_only_one_path_stays_masked(self):
        #   idiv ecx          undefines six
        #   je   +2           -> the RET, with all six still undefined
        #   cmp  eax, eax     redefines them on the fallthrough
        #   ret
        # The oracle cannot know which edge a case took, so the union is the
        # only sound answer.
        self.assertEqual(names(ARITHMETIC),
                         names(mask_of((BASE, "f7f9" "7402" "39c0" "c3"))))

    def test_every_exit_contributes_and_no_single_one_explains_the_answer(self):
        #   idiv ecx
        #   je   +2           -> the second exit
        #   daa               undefines OF only
        #   ret               exit A: OF
        #   test eax, eax     undefines AF only
        #   ret               exit B: AF
        # Exit A alone is 0x20, exit B alone is 0x04; only the union is 0x24,
        # so "first exit wins" and "last exit wins" both fail here.
        self.assertEqual({"OF", "AF"},
                         names(mask_of((BASE, "f7f9" "7402" "27" "c3"
                                              "85c0" "c3"))))

    def test_a_loop_reaches_a_fixpoint_and_still_answers_per_flag(self):
        #   idiv ecx
        #   dec  ecx          redefines five, never CF
        #   jne  -3           back edge
        #   ret
        self.assertEqual({"CF"},
                         names(mask_of((BASE, "f7f9" "49" "75fd" "c3"))))

    def test_a_call_resets_the_state_because_the_callee_owns_the_flags(self):
        # Whatever the callee leaves in EFLAGS is the callee's plan row. This
        # is what stops one divide anywhere in the image from masking flags in
        # every one of its callers.
        self.assertEqual(0, mask_of((BASE, "f7f9" "e800000000" "c3")))

    def test_a_tail_call_out_of_the_function_is_an_exit(self):
        # `jmp` to an address outside every span: the flags at that point are
        # observable, so they count. Masking more than the truth is the safe
        # direction only here, where the alternative is comparing a flag the
        # architecture never defined.
        self.assertEqual(names(ARITHMETIC),
                         names(mask_of((BASE, "f7f9" "e9f90f0000"))))

    def test_a_conditional_branch_out_of_the_function_is_an_exit_too(self):
        #   idiv ecx
        #   je   0x401044     outside the span -> exit carrying all six
        #   cmp  eax, eax     the fallthrough redefines everything
        #   ret
        # Ignoring the out-of-span edge answers 0.
        self.assertEqual(names(ARITHMETIC),
                         names(mask_of((BASE, "f7f9" "7440" "39c0" "c3"))))

    def test_running_off_the_end_of_a_span_is_an_exit(self):
        # A span that ends without a RET - capstone stopped early, or the tail
        # is data. The flags at that point are still observable, and dropping
        # the edge would answer 0 for a body that is nothing but a divide.
        self.assertEqual(names(ARITHMETIC), names(mask_of((BASE, "f7f9"))))

    def test_a_loop_instruction_branches_like_a_jcc(self):
        #   idiv ecx
        #   loop +2           -> the RET, six flags still undefined
        #   cmp  eax, eax     the fallthrough redefines them
        #   ret
        # `loop` is not a `j*` mnemonic, so it needs its own arm; without one
        # its taken edge disappears and the answer is 0.
        self.assertEqual(names(ARITHMETIC),
                         names(mask_of((BASE, "f7f9" "e202" "39c0" "c3"))))

    def test_the_second_span_is_walked_and_can_clear_the_mask(self):
        #   0x00401000  idiv ecx / jmp 0x00402000     the hot span
        #   0x00402000  cmp eax, eax / ret            the outlined span
        # A walk that only knows the first span sees the jmp leave the
        # function, calls it a tail call, and reports all six.
        self.assertEqual(0, mask_of((BASE, "f7f9" "e9f90f0000"),
                                    (COLD, "39c0" "c3")))

    def test_the_second_span_can_also_be_the_only_source_of_the_mask(self):
        #   0x00401000  jmp 0x00402000
        #   0x00402000  idiv ecx / ret
        # The mirror image: a walk that stops at the first span reports 0 and
        # the oracle then compares six flags the architecture never defined,
        # which is a false FAIL rather than a silent pass. Both directions of
        # the same bug are one edit apart.
        self.assertEqual(names(ARITHMETIC),
                         names(mask_of((BASE, "e9fb0f0000"),
                                       (COLD, "f7f9" "c3"))))

    def test_an_interrupt_return_exits_with_the_flags_it_arrived_with(self):
        """The `iret` arm takes `incoming`, not `outgoing` - and that is
        observable, contrary to an earlier claim in this repo.

        A previous round recorded this arm as an "honest gap" that "cannot be
        asserted", on the reasoning that hlt/iret/iretd have no flag effect so
        `incoming == outgoing` and no input distinguishes the two. That is
        false for iret: capstone reports eflags 0x7ff for it, i.e. it DEFINES
        all seven flags, because it pops EFLAGS off the stack. So `outgoing`
        is 0 where `incoming` is the divide's six, and the two arms give
        different answers on one three-byte body.

        `incoming` is the right one. IRET restores flags from a frame the
        oracle never modelled, so what it "defines" is not something the lift
        can be expected to reproduce; the flags that were undefined on the way
        in are the honest answer. Only `hlt` remains genuinely
        indistinguishable, and it is not an exit anything can observe.
        """
        # idiv ecx / iret - HEAD answers with the divide's six flags.
        self.assertEqual(names(ARITHMETIC), names(mask_of((BASE, "f7f9" "cf"))))
        # ...and the same body ending in a plain ret answers the same, so the
        # assertion above is not just re-testing the generic return path.
        self.assertEqual(names(ARITHMETIC), names(mask_of((BASE, "f7f9" "c3"))))

    def test_a_halt_ends_the_walk_without_a_successor(self):
        # `hlt` genuinely has no flag effect, so incoming == outgoing and this
        # cannot distinguish the two arms. It is asserted only to pin that hlt
        # terminates a path at all rather than falling through into whatever
        # bytes follow.
        self.assertEqual(names(ARITHMETIC), names(mask_of((BASE, "f7f9" "f4"))))


class ScanTests(unittest.TestCase):
    """The per-function row, decoded from a PE the test owns.

    `scan` is where the spans are read, so the mask's multi-span property has
    to be asserted through it as well as through the dataflow alone.
    """

    def test_the_mask_reaches_the_function_row(self):
        self.assertEqual(names(ARITHMETIC),
                         names(scanned((BASE, "f7f9" "c3"))["undef"]))

    def test_scan_decodes_every_span_not_just_the_first(self):
        # Same shape as the dataflow test, but the bytes now come out of the
        # image the way they do in production: a `scan` that reads only
        # spans[0] never decodes the redefinition and reports all six.
        subject = scanned((BASE, "f7f9" "e9f90f0000"), (COLD, "39c0" "c3"))
        self.assertEqual(0, subject["undef"])

    def test_a_clean_body_carries_no_mask_and_no_local_flags(self):
        subject = scanned((BASE, "89c8" "83c001" "c3"))
        self.assertEqual(0, subject["undef"])
        self.assertEqual(set(), subject["local"])

    def test_fs_is_decided_by_the_segment_register(self):
        # mov eax, fs:[0] - a real SEH-chain read.
        self.assertIn("fs", scanned((BASE, "64a100000000" "c3"))["local"])

    def test_a_string_move_is_not_an_fs_reference(self):
        # `rep movsd` carries an implicit es:, and its mnemonic starts with
        # "mov". Testing the mnemonic flagged 0x00592940 and three others for
        # an fs: they do not contain; 0x00592940 PASSES when it is run.
        self.assertNotIn("fs", scanned((BASE, "f3a5" "c3"))["local"])

    def test_an_iat_call_is_recognised_by_its_slot(self):
        # call dword ptr [0x00600000], with that address in the import table.
        self.assertIn("iat", scanned((BASE, "ff1500006000" "c3"),
                                     slots={0x00600000})["local"])
        # The same instruction against an address that is NOT a slot is an
        # ordinary absolute indirect call, and must not skip the function.
        self.assertNotIn("iat", scanned((BASE, "ff1500006000" "c3"),
                                        slots={0x00600004})["local"])

    def test_an_iat_match_requires_an_absolute_address(self):
        """A slot number appearing as a DISPLACEMENT is not an IAT reference.

        The guard demands no base and no index register, and the negative case
        above varies only the displacement - so dropping the two register
        tests was invisible. `[eax + 0x600000]` is an array walk whose stride
        happens to start at a slot address; treating it as an import would
        skip a function that never leaves the image.
        """
        # call dword ptr [eax + 0x00600000]
        based = scanned((BASE, "ff9000006000" "c3"), slots={0x00600000})
        self.assertNotIn("iat", based["local"])
        self.assertIn("indirect", based["local"])
        # call dword ptr [eax*4 + 0x00600000] - indexed, no base.
        indexed = scanned((BASE, "ff148500006000" "c3"), slots={0x00600000})
        self.assertNotIn("iat", indexed["local"])
        # ...and the plain absolute form, same slot, still matches. Without
        # this the two assertions above would pass on a scan that never
        # detects an IAT reference at all.
        self.assertIn("iat", scanned((BASE, "ff1500006000" "c3"),
                                     slots={0x00600000})["local"])

    def test_a_direct_call_to_a_catalogued_function_is_an_edge_not_a_blocker(self):
        subject = scanned((BASE, "e8fb0f0000" "c3"), entries=(COLD,))
        self.assertEqual({COLD}, subject["edges"])
        self.assertNotIn("extcall", subject["local"])

    def test_a_direct_call_to_an_uncatalogued_address_blocks(self):
        subject = scanned((BASE, "e8fb0f0000" "c3"))
        self.assertEqual(set(), subject["edges"])
        self.assertIn("extcall", subject["local"])

    def test_an_indirect_call_and_an_x87_body_are_flagged(self):
        self.assertIn("indirect", scanned((BASE, "ffd0" "c3"))["local"])
        self.assertIn("x87", scanned((BASE, "d900" "c3"))["local"])


def parse_undef_token(flags: str) -> int:
    """What lifted_oracle_main.cpp reads back out of the flags field.

    A deliberate restatement of the C++ `undefined_exit_flags`: split the field
    on commas, find the element that starts with `undef=` and has at least one
    character after it, and read that as hex. Absent means zero. Keeping the
    reader here in the test is what makes "an old plan with no token behaves
    exactly as it did" an assertion rather than a comment.
    """
    for element in flags.split(","):
        if element.startswith("undef=") and len(element) > len("undef="):
            return int(element[len("undef="):], 16)
    return 0


class PlanRowTests(unittest.TestCase):
    """The written row, because the mask has to survive being a string."""

    CSV = ("address,size,name,recovery_state,body_ranges\n"
           "0x00401000,3,divides,unrecovered,0x00401000-0x00401003\n"
           "0x00401010,6,clean,unrecovered,0x00401010-0x00401016\n"
           "0x00401020,5,divides_with_x87,unrecovered,0x00401020-0x00401025\n")

    def rows(self, *extra: str) -> dict:
        pe = FakePE()
        pe.place(0x00401000, "f7f9" "c3")            # idiv ecx / ret
        pe.place(0x00401010, "89c8" "83c001" "c3")   # mov / add / ret
        pe.place(0x00401020, "d900" "f7f9" "c3")     # fld / idiv ecx / ret
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "functions.csv").write_text(self.CSV)
            argv = ["lifted_oracle_plan.py", "--exe", str(root / "unused.exe"),
                    "--functions", str(root / "functions.csv"),
                    "--out", str(root / "plan.tsv"), *extra]
            original_pe, original_argv = plan.pefile.PE, sys.argv
            plan.pefile.PE = lambda *args, **kwargs: pe
            sys.argv = argv
            try:
                with contextlib.redirect_stdout(io.StringIO()):
                    self.assertEqual(0, plan.main())
                text = (root / "plan.tsv").read_text()
            finally:
                plan.pefile.PE, sys.argv = original_pe, original_argv
        rows = {}
        for line in text.splitlines():
            if line.startswith("#"):
                continue
            address, flags, name = line.split("\t")
            rows[name] = flags
        return rows

    def test_a_masked_function_carries_the_token_the_oracle_can_read_back(self):
        flags = self.rows()["divides"]
        self.assertIn("undef=3f", flags)
        self.assertEqual(ARITHMETIC, parse_undef_token(flags))

    def test_an_unmasked_function_carries_no_token_at_all(self):
        # Absence is the encoding of zero. A row that said `undef=0` would
        # parse to the same number today and would be a different plan format
        # tomorrow - and, right now, would appear in every one of the 3,410
        # rows that are supposed to say only "ok".
        flags = self.rows()["clean"]
        self.assertEqual("ok", flags)
        self.assertNotIn("undef", flags)
        self.assertEqual(0, parse_undef_token(flags))

    def test_the_token_shares_the_field_with_the_other_flags(self):
        # The flags are ONE tab-separated field, comma-joined, and the C++ side
        # scans the same field for "x87" and for the token. A row that put the
        # mask anywhere else would take the whole row's flag set with it.
        flags = self.rows()["divides_with_x87"]
        self.assertEqual(["undef=3f", "x87"], flags.split(","))
        self.assertEqual(ARITHMETIC, parse_undef_token(flags))


class SuppressedMaskTests(PlanRowTests):
    """`--no-undef`: the native-x86 experiment, on the same three bodies.

    The mask exists because a TRANSLATED host answered inconsistently for
    architecturally-undefined flags. Suppressing the token makes the oracle
    compare those bits instead of ignoring them, which is strictly stricter -
    it can only turn a PASS into a FAIL. Inheriting PlanRowTests is deliberate:
    every assertion above re-runs unchanged here, so a flag that quietly
    changed anything ELSE about the row would fail those inherited tests
    rather than these.
    """

    def rows(self, *extra: str) -> dict:
        return super().rows("--no-undef", *extra)

    def test_a_masked_function_carries_the_token_the_oracle_can_read_back(self):
        # Inverted: with the mask suppressed the divider looks like any other
        # clean function, which is the entire point of the flag.
        flags = self.rows()["divides"]
        self.assertEqual("ok", flags)
        self.assertNotIn("undef", flags)
        self.assertEqual(0, parse_undef_token(flags))

    def test_the_token_shares_the_field_with_the_other_flags(self):
        # The OTHER flags must survive. Suppressing the mask that shared this
        # field must not take x87 - a scheduling flag the C++ side reads from
        # the same string - with it.
        flags = self.rows()["divides_with_x87"]
        self.assertEqual(["x87"], flags.split(","))
        self.assertEqual(0, parse_undef_token(flags))

    def test_the_mask_is_still_computed_only_not_printed(self):
        # Suppression is a REPORTING choice. If it had been implemented by
        # skipping undefined_exit_flags, the histogram's other undef- lines
        # would be the only evidence and this tool would have two analyses.
        pe = FakePE()
        pe.place(0x00401000, "f7f9" "c3")
        functions = [{"address": 0x00401000, "name": "divides",
                      "spans": [(0x00401000, 0x00401003)], "section": ".text"}]
        plan.scan(pe, functions, set())
        self.assertEqual(ARITHMETIC, functions[0]["undef"])


class SuppressedMaskHistogramTests(unittest.TestCase):
    """The printed histogram has to agree with the file, or the run lies."""

    CSV = PlanRowTests.CSV

    def stdout(self, *extra: str) -> str:
        pe = FakePE()
        pe.place(0x00401000, "f7f9" "c3")
        pe.place(0x00401010, "89c8" "83c001" "c3")
        pe.place(0x00401020, "d900" "f7f9" "c3")
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "functions.csv").write_text(self.CSV)
            argv = ["lifted_oracle_plan.py", "--exe", str(root / "unused.exe"),
                    "--functions", str(root / "functions.csv"),
                    "--out", str(root / "plan.tsv"), *extra]
            original_pe, original_argv = plan.pefile.PE, sys.argv
            plan.pefile.PE = lambda *args, **kwargs: pe
            sys.argv = argv
            captured = io.StringIO()
            try:
                with contextlib.redirect_stdout(captured):
                    self.assertEqual(0, plan.main())
            finally:
                plan.pefile.PE, sys.argv = original_pe, original_argv
        return captured.getvalue()

    def test_the_default_run_reports_the_masked_functions(self):
        self.assertIn("undef", self.stdout())

    def test_a_suppressed_run_reports_no_undef_line_at_all(self):
        # Including the per-flag `undef-cf` style lines: a histogram still
        # counting masks while the file carries none would be the dashboard
        # error this project already made once.
        self.assertNotIn("undef", self.stdout("--no-undef"))


class CallGraphTests(unittest.TestCase):
    """What travels along a call edge - and what must not.

    `close_over_calls` spreads the disqualifying facts backwards because
    calling a neighbour that calls DirectDraw is calling DirectDraw. The mask
    is the opposite: the flags at a callee's RET are overwritten or ignored by
    the caller's own path to its own RET, so inheriting it would mask flags in
    every caller of every function that divides.
    """

    @staticmethod
    def graph():
        return [{"address": 0x401000, "local": {"x87"}, "edges": {0x402000},
                 "undef": 0},
                {"address": 0x402000, "local": set(), "edges": {0x403000},
                 "undef": ARITHMETIC},
                {"address": 0x403000, "local": {"iat"}, "edges": set(),
                 "undef": 0}]

    def test_a_blocker_two_calls_away_still_blocks(self):
        functions = self.graph()
        plan.close_over_calls(functions)
        self.assertIn("iat", functions[0]["flags"])
        self.assertIn("iat", functions[1]["flags"])

    def test_the_undefined_mask_does_not_travel_to_the_caller(self):
        functions = self.graph()
        plan.close_over_calls(functions)
        self.assertEqual(0, functions[0]["undef"])
        self.assertEqual(ARITHMETIC, functions[1]["undef"])

    def test_every_spreading_flag_spreads(self):
        """Transitivity is asserted for all six flags, not just for `iat`.

        The graph above plants only `iat` as the far blocker, so five of the
        six members of `spreading` were undefended: `spreading = {"iat"}`
        alone took the live "testable (no blocker)" count from 1,941 to 2,482
        - a 27.9% inflation of the number this tool exists to produce - with
        the whole suite green. Each flag is checked on its own so a failure
        names the one that stopped travelling.
        """
        for flag in ("iat", "fs", "extcall", "selfmod", "x87", "indirect"):
            with self.subTest(flag=flag):
                functions = [
                    {"address": 0x401000, "local": set(), "edges": {0x402000},
                     "undef": 0},
                    {"address": 0x402000, "local": set(), "edges": {0x403000},
                     "undef": 0},
                    {"address": 0x403000, "local": {flag}, "edges": set(),
                     "undef": 0},
                ]
                plan.close_over_calls(functions)
                self.assertIn(flag, functions[1]["flags"],
                              f"{flag} did not reach its direct caller")
                self.assertIn(flag, functions[0]["flags"],
                              f"{flag} did not reach two calls back")

    def test_nothing_travels_forwards_along_a_call_edge(self):
        """A callee does not inherit its caller's blockers.

        The closure direction is the whole claim: reaching an import makes the
        CALLER untestable, not the callee. Reversing it would disqualify the
        leaves - which are exactly the functions the oracle can test.
        """
        functions = [
            {"address": 0x401000, "local": {"iat"}, "edges": {0x402000},
             "undef": 0},
            {"address": 0x402000, "local": set(), "edges": set(), "undef": 0},
        ]
        plan.close_over_calls(functions)
        self.assertEqual(set(), functions[1]["flags"])

    def test_a_call_edge_to_a_function_that_is_not_in_the_plan_is_ignored(self):
        functions = [{"address": 0x401000, "local": set(),
                      "edges": {0x999000}, "undef": 0}]
        plan.close_over_calls(functions)
        self.assertEqual(set(), functions[0]["flags"])

    def test_a_call_cycle_terminates_and_still_spreads(self):
        functions = [
            {"address": 0x401000, "local": set(), "edges": {0x402000},
             "undef": 0},
            {"address": 0x402000, "local": {"fs"}, "edges": {0x401000},
             "undef": 0},
        ]
        rounds = plan.close_over_calls(functions)
        self.assertIn("fs", functions[0]["flags"])
        self.assertGreaterEqual(rounds, 2)


class ImportDirectoryTests(unittest.TestCase):
    """`iat_slots` has to PARSE the directory before it can read it.

    `main` opens the image with `fast_load=True`, which is what makes a
    6,000-function scan finish, and a fast-loaded PE has no import directory
    until `parse_data_directories()` runs. Delete that one call and `iat_slots`
    returns an empty set in silence - `getattr(pe, "DIRECTORY_ENTRY_IMPORT",
    [])` swallows the absence - and the import wall disappears: measured on the
    real image, IAT slots 221 -> 0 and the `iat` flag vanishes from all 2,983
    functions carrying it, every one of which is then handed to the oracle to
    call for real. That is the hazard this module's docstring opens with.
    """

    def test_the_import_directory_is_parsed_before_it_is_read(self):
        pe = FakePE(imports=(0x00600000, 0x00600004))
        self.assertFalse(hasattr(pe, "DIRECTORY_ENTRY_IMPORT"))
        slots = plan.iat_slots(pe)
        self.assertTrue(pe.parsed, "parse_data_directories was never called")
        self.assertEqual({0x00600000, 0x00600004}, slots)

    def test_delay_imports_are_slots_too(self):
        # A delay-loaded import is still a call that leaves the image; the
        # thunk resolves on first use and lands in the same DLL.
        pe = FakePE(imports=(0x00600000,), delay_imports=(0x00601000,))
        self.assertEqual({0x00600000, 0x00601000}, plan.iat_slots(pe))

    def test_an_image_with_no_imports_yields_no_slots(self):
        pe = FakePE()
        self.assertEqual(set(), plan.iat_slots(pe))
        self.assertTrue(pe.parsed)

    def test_a_slot_with_no_address_is_not_a_slot(self):
        # pefile leaves `address` None for an import it could not place.
        pe = FakePE(imports=(0x00600000, None))
        self.assertEqual({0x00600000}, plan.iat_slots(pe))

    def test_the_parsed_slots_reach_the_function_rows(self):
        """End to end: an import in the directory becomes an `iat` flag.

        Asserting `iat_slots` alone would not catch `main` failing to pass its
        result to `scan`, which is the wiring that puts the flag on the row.
        """
        rows, _ = plan_rows(
            "address,size,name,recovery_state,body_ranges\n"
            "0x00401000,7,calls_import,unrecovered,0x00401000-0x00401007\n",
            {0x00401000: "ff1500006000" "c3"},
            imports=(0x00600000,))
        # `call [0x00600000]` is an absolute indirect call as well as an
        # import, so the row legitimately carries both flags.
        self.assertEqual(["iat", "indirect"], rows["calls_import"].split(","))


class SelfModifyingSectionTests(unittest.TestCase):
    """A function in an unliftable section is `selfmod`, decoded or not.

    Nothing exercised the section guard, so `if False:` left the flag off all
    22 self-modifying functions and moved "testable" from 1,941 to 1,963 -
    handing the oracle bodies the lift deliberately fills with traps.
    """

    def test_a_function_in_an_unliftable_section_is_flagged(self):
        self.assertIn("_selfmod", lift.UNLIFTABLE_SECTIONS,
                      "the section name this test relies on has moved")
        subject = scanned((BASE, "89c8" "c3"), section="_selfmod")
        self.assertIn("selfmod", subject["local"])

    def test_an_ordinary_section_is_not_flagged(self):
        subject = scanned((BASE, "89c8" "c3"), section=".text")
        self.assertNotIn("selfmod", subject["local"])


class IntraFunctionTransferTests(unittest.TestCase):
    """A branch to your own body is control flow, not a call out.

    No fixture ever gave the subject a target inside its own span, so
    `if target not in inside:` -> `if True:` was undefended: every internal
    `jmp` became an `extcall`, taking "testable" from 1,941 to 1,545 (-20.4%)
    and `extcall` from 3,472 to 4,030 across 558 rows.
    """

    def test_a_jump_inside_the_body_is_neither_an_edge_nor_a_blocker(self):
        # jmp +0 (to the ret that follows), then ret: entirely internal.
        subject = scanned((BASE, "e900000000" "c3"))
        self.assertEqual(set(), subject["edges"])
        self.assertNotIn("extcall", subject["local"])

    def test_a_call_inside_the_body_is_neither_an_edge_nor_a_blocker(self):
        # call +0, i.e. the instruction after itself, then ret.
        subject = scanned((BASE, "e800000000" "c3"))
        self.assertEqual(set(), subject["edges"])
        self.assertNotIn("extcall", subject["local"])

    def test_a_jump_into_the_second_span_is_still_internal(self):
        # jmp 0x00402000, which the second span owns.
        subject = scanned((BASE, "e9fb0f0000" "c3"), (COLD, "c3"))
        self.assertEqual(set(), subject["edges"])
        self.assertNotIn("extcall", subject["local"])

    def test_a_jump_just_past_the_body_is_external(self):
        """The boundary, so "inside" cannot quietly become "anywhere".

        Byte-for-byte the same body as the test above; the ONLY difference is
        that the second span is gone, so 0x00402000 is no longer this
        function's. It must now block.
        """
        subject = scanned((BASE, "e9fb0f0000" "c3"))
        self.assertIn("extcall", subject["local"])


class IndirectTransferTests(unittest.TestCase):
    """`indirect` means an indirect CALL, not any indirect transfer.

    An indirect `jmp` is overwhelmingly a switch dispatch, which
    `jump_tables.py` resolves; an indirect `call` is a function pointer the
    lift cannot follow. Broadening the arm to `else:` flagged 114 more
    functions as `indirect`. Pinned, because the two are informational counts
    the project reads to decide what to work on next.
    """

    def test_an_indirect_call_is_flagged(self):
        self.assertIn("indirect", scanned((BASE, "ffd0" "c3"))["local"])

    def test_an_indirect_jump_is_not_an_indirect_call(self):
        # jmp eax - a switch dispatch, resolved elsewhere.
        self.assertNotIn("indirect", scanned((BASE, "ffe0" "c3"))["local"])

    def test_a_direct_call_is_not_indirect(self):
        self.assertNotIn("indirect", scanned((BASE, "e8fb0f0000" "c3"),
                                             entries=(COLD,))["local"])


class BlockingSetTests(unittest.TestCase):
    """Which flags cost a function its place in the testable count.

    Four block - iat, fs, extcall, selfmod - and two do not: x87 and indirect
    are informational, and the functions carrying them ARE tested. Nothing
    asserted the membership, so adding x87 to `blocking` moved "testable" from
    1,941 to 1,831 and removing selfmod moved it to 1,963, both with the suite
    green and neither touching a single plan row - the damage is confined to
    the headline the tool prints.
    """

    BLOCKING = ("iat", "fs", "extcall", "selfmod")
    INFORMATIONAL = ("x87", "indirect")

    def count_testable(self, encoded: str) -> int:
        rows, out = plan_rows(
            "address,size,name,recovery_state,body_ranges\n"
            f"0x00401000,{len(encoded) // 2},subject,unrecovered,"
            f"0x00401000-0x{0x00401000 + len(encoded) // 2:08X}\n",
            {0x00401000: encoded}, imports=(0x00600000,))
        for line in out.splitlines():
            if line.startswith("testable (no blocker)"):
                return int(line.split()[-1])
        raise AssertionError(f"no testable line in:\n{out}")

    def test_a_clean_function_is_testable(self):
        self.assertEqual(1, self.count_testable("89c8" "c3"))

    def test_each_blocking_flag_costs_the_function_its_place(self):
        bodies = {
            "iat": "ff1500006000" "c3",          # call [0x00600000]
            "fs": "64a100000000" "c3",           # mov eax, fs:[0]
            "extcall": "e8fb0f0000" "c3",        # call an uncatalogued address
        }
        for flag, encoded in bodies.items():
            with self.subTest(flag=flag):
                self.assertEqual(0, self.count_testable(encoded))

    def test_selfmod_costs_the_function_its_place_too(self):
        """`selfmod` is the fourth blocker and comes from the SECTION.

        It has no encoding, so it cannot ride along in the loop above - which
        is exactly why dropping it from `blocking` survived: the count moved
        from 1,941 to 1,963 on the real image with everything green. The body
        here is clean; only its address makes it unliftable.
        """
        address = 0x00404000                     # inside FakePE's _selfmod
        rows, out = plan_rows(
            "address,size,name,recovery_state,body_ranges\n"
            f"0x{address:08X},2,subject,unrecovered,"
            f"0x{address:08X}-0x{address + 2:08X}\n",
            {address: "89c8" "c3"})
        self.assertEqual("selfmod", rows["subject"])
        self.assertIn("testable (no blocker)  0", out)

    def test_the_informational_flags_do_not_block(self):
        bodies = {"x87": "d900" "c3", "indirect": "ffd0" "c3"}
        for flag, encoded in bodies.items():
            with self.subTest(flag=flag):
                rows, _ = plan_rows(
                    "address,size,name,recovery_state,body_ranges\n"
                    f"0x00401000,{len(encoded) // 2},subject,unrecovered,"
                    f"0x00401000-0x{0x00401000 + len(encoded) // 2:08X}\n",
                    {0x00401000: encoded})
                self.assertIn(flag, rows["subject"])
                self.assertEqual(1, self.count_testable(encoded),
                                 f"{flag} must not block")


class HistogramTests(unittest.TestCase):
    """The printed counts, because they are what a human reads off the run.

    The per-flag `undef-<NAME>` breakdown is printed by a three-line loop that
    nothing asserted: deleting it removed every `undef-AF/PF/ZF/SF/CF/OF` line
    from the output with all 38 tests green. Those six numbers are how the
    project checks the mask is doing what its comment claims ("names a flag
    other than AF in exactly 14 functions").
    """

    CSV = ("address,size,name,recovery_state,body_ranges\n"
           "0x00401000,3,divides,unrecovered,0x00401000-0x00401003\n"
           "0x00401010,6,clean,unrecovered,0x00401010-0x00401016\n")
    BODIES = {0x00401000: "f7f9" "c3", 0x00401010: "89c8" "83c001" "c3"}

    def output(self) -> str:
        return plan_rows(self.CSV, self.BODIES)[1]

    def test_each_undefined_flag_gets_its_own_line(self):
        out = self.output()
        # One idiv: all six arithmetic flags, each counted once.
        for name in ("CF", "PF", "AF", "ZF", "SF", "OF"):
            with self.subTest(name=name):
                self.assertIn(f"undef-{name} 1", " ".join(out.split()))

    def test_the_direction_flag_never_appears_in_the_histogram(self):
        self.assertNotIn("undef-DF", self.output())

    def test_the_undef_total_counts_functions_not_flags(self):
        out = " ".join(self.output().split())
        # One masked function, six masked flags. `undef 6` would mean the
        # per-function counter is counting bits.
        self.assertIn("undef 1", out)
        self.assertNotIn("undef 6", out)

    def test_the_headline_counts_are_printed(self):
        out = self.output()
        self.assertIn("functions              2", out)
        self.assertIn("testable (no blocker)  2", out)


if __name__ == "__main__":
    unittest.main()
