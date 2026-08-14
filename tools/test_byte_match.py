#!/usr/bin/env python3
"""Pin the byte-match oracle, in both directions.

CI has no Visual C++ 6.0 and no Wine, so the compiling half of this is skipped
there and the parts that could quietly turn a mismatch into a match are pinned
everywhere: the COFF walk that decides which bytes are "the function", the span
classifier that decides which bytes are even in scope, the jump-table clip, and
the tier ladder itself.

WHY THE TIER TESTS ARE HERE RATHER THAN LEFT TO THE LIVE CONTROLS. The live
controls in the tool can only run when something matched, and a tier that
never fires is indistinguishable from a tier that cannot. The SHAPE_EXACT
ladder in particular was WRONG on its first draft - it kept non-relocated
immediates, which made it nearly redundant with BYTE_EXACT, so it could almost
never fire. A control written to exercise it is what found that. These pin the
repaired behaviour so it cannot silently regress to the redundant version.

Every case that damages something asserts the damage LANDED before asserting
the verdict. The first version of the constant-perturbation control edited a
doc comment rather than the expression - `extract_body` deliberately opens
mid-comment - and reported a clean pass while changing no emitted byte. A
control that cannot fail is worth nothing, and one that silently stopped
testing its subject is worse, because it reads as evidence.
"""

from __future__ import annotations

import re
import struct
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import byte_match as tool  # noqa: E402


def build_coff(sections_in, symbols_in, relocations=None):
    """A minimal i386 COFF: [(name, code)], [(symbol, section, value, storage)].

    `relocations` is {section index (1-based): [(offset, symbol index, type)]}.
    Written by hand rather than produced by a compiler so the reader is tested
    against bytes whose layout is known here, not against whatever CL happened
    to emit.
    """
    relocations = relocations or {}
    header_size = 20 + 40 * len(sections_in)
    raw = b""
    placed = []
    for name, code in sections_in:
        placed.append((name, len(code), header_size + len(raw)))
        raw += code

    reloc_blobs, reloc_at = b"", {}
    reloc_base = header_size + len(raw)
    for index, entries in relocations.items():
        reloc_at[index] = (reloc_base + len(reloc_blobs), len(entries))
        for offset, symbol, kind in entries:
            reloc_blobs += struct.pack("<IIH", offset, symbol, kind)

    symbols = b""
    for name, section, value, storage in symbols_in:
        symbols += (name.encode().ljust(8, b"\0")[:8]
                    + struct.pack("<IhHBB", value, section, 0x20, storage, 0))

    table_at = reloc_base + len(reloc_blobs)
    head = struct.pack("<HHIIIHH", 0x14C, len(sections_in), 0, table_at,
                       len(symbols_in), 0, 0)
    table = b""
    for index, (name, size, pointer) in enumerate(placed, start=1):
        pointer_to_relocs, count = reloc_at.get(index, (0, 0))
        table += (name.encode().ljust(8, b"\0")[:8]
                  + struct.pack("<IIII", 0, 0, size, pointer)
                  + struct.pack("<IIHHI", pointer_to_relocs, 0, count, 0,
                                0x60501020))
    return head + table + raw + reloc_blobs + symbols + struct.pack("<I", 4)


RET = b"\xc3"


class CoffWalkTests(unittest.TestCase):
    def test_it_returns_the_single_external_text_symbol(self):
        data = build_coff([(".text", b"\x90\x90" + RET)],
                          [("?f@@YAXXZ", 1, 0, tool.IMAGE_SYM_CLASS_EXTERNAL)])
        code, mask = tool.object_code(data)
        self.assertEqual(code, b"\x90\x90" + RET)
        self.assertEqual(mask, set())

    def test_it_refuses_two_definitions_rather_than_guessing(self):
        """A helper defined alongside the subject gets inlined; the original
        did not inline it. Refusing is the only safe answer."""
        data = build_coff(
            [(".text", RET), (".text", RET)],
            [("?f@@YAXXZ", 1, 0, tool.IMAGE_SYM_CLASS_EXTERNAL),
             ("?g@@YAXXZ", 2, 0, tool.IMAGE_SYM_CLASS_EXTERNAL)])
        with self.assertRaises(ValueError) as caught:
            tool.object_code(data)
        self.assertIn("expected one external .text symbol", str(caught.exception))

    def test_auxiliary_records_are_skipped_not_decoded_as_symbols(self):
        """An aux record's bytes can read as a name and a storage class."""
        symbols = struct.pack("<8sIhHBB", b".text\0\0\0", 0, 1, 0, 3, 1)
        symbols += b"\x10\x00\x00\x00\x02\x00\x00\x00" + b"\0" * 10  # aux
        symbols += struct.pack("<8sIhHBB", b"?f@@YAXXZ"[:8], 0, 1, 0x20, 2, 0)
        head = struct.pack("<HHIIIHH", 0x14C, 1, 0, 20 + 40 + 1, 3, 0, 0)
        table = (b".text\0\0\0" + struct.pack("<IIII", 0, 0, 1, 20 + 40)
                 + struct.pack("<IIHHI", 0, 0, 0, 0, 0x60501020))
        data = head + table + RET + symbols + struct.pack("<I", 4)
        sections, parsed = tool.parse_coff(data)
        self.assertEqual(len(parsed), 3)
        self.assertEqual(parsed[1].storage, -1, "the aux record must be a hole")

    def test_the_eh_comdat_is_found_by_name_not_by_symbol(self):
        """`/GX` puts the unwind funclets in `.text$x`, which carries no
        external symbol - so a symbol lookup would never find it."""
        data = build_coff([(".text", RET), (".text$x", b"\xb8\x00\x00\x00\x00")],
                          [("?f@@YAXXZ", 1, 0, tool.IMAGE_SYM_CLASS_EXTERNAL)])
        code, _ = tool.object_code(data, want_eh=True)
        self.assertEqual(code, b"\xb8\x00\x00\x00\x00")

    def test_the_primary_walk_never_returns_the_eh_comdat(self):
        data = build_coff([(".text$x", b"\x90"), (".text", RET)],
                          [("?f@@YAXXZ", 2, 0, tool.IMAGE_SYM_CLASS_EXTERNAL)])
        code, _ = tool.object_code(data)
        self.assertEqual(code, RET)


class JumpTableClipTests(unittest.TestCase):
    """The original side excludes switch tables; `/Gy` puts them in the SAME
    COMDAT on the object side. Keeping them compares code against data."""

    def _object_with_table(self, entries):
        # Symbol index 1 is the LABEL. Getting this wrong points every
        # relocation at a symbol that does not exist, no label is ever matched,
        # nothing is clipped, and the test passes while proving nothing - which
        # is what the first draft of it did.
        code = RET
        table = b"\0\0\0\0" * entries
        relocs = {1: [(len(code) + 4 * i, 1, tool.IMAGE_REL_I386_DIR32)
                      for i in range(entries)]}
        return build_coff(
            [(".text", code + table)],
            [("?f@@YAXXZ", 1, 0, tool.IMAGE_SYM_CLASS_EXTERNAL),
             ("$L1", 1, len(code), tool.IMAGE_SYM_CLASS_LABEL)],
            relocs)

    def test_the_fixture_really_builds_a_table(self):
        """Guard the guard: if the relocations do not resolve to a LABEL there
        is nothing to clip and every other case here is vacuous."""
        sections, symbols = tool.parse_coff(self._object_with_table(7))
        labels = [s for s in symbols if s.storage == tool.IMAGE_SYM_CLASS_LABEL]
        self.assertEqual(len(labels), 1)
        targets = {r[1] for r in sections[0].relocations}
        self.assertTrue(
            any(symbols[t].storage == tool.IMAGE_SYM_CLASS_LABEL
                for t in targets),
            "no relocation targets a label; the clip has nothing to find")

    def test_a_trailing_relocation_run_is_clipped(self):
        code, _ = tool.object_code(self._object_with_table(7))
        self.assertEqual(code, RET, "28 table bytes would decode as 14 "
                                    "phantom instructions")

    def test_it_clips_nothing_when_there_is_no_table(self):
        data = build_coff([(".text", b"\x90\x90" + RET)],
                          [("?f@@YAXXZ", 1, 0, tool.IMAGE_SYM_CLASS_EXTERNAL)])
        code, _ = tool.object_code(data)
        self.assertEqual(code, b"\x90\x90" + RET)

    def _object_with_compressed_table(self, entries, indices):
        """The two-table shape VC6 emits for a SPARSE switch: a dword table
        carrying the relocations, then a dense byte table of indices into it
        carrying none, owning the COMDAT's last byte."""
        code = RET
        table = b"\0\0\0\0" * entries
        relocs = {1: [(len(code) + 4 * i, 1, tool.IMAGE_REL_I386_DIR32)
                      for i in range(entries)]}
        return build_coff(
            [(".text", code + table + bytes(indices))],
            [("?f@@YAXXZ", 1, 0, tool.IMAGE_SYM_CLASS_EXTERNAL),
             ("$L1", 1, len(code), tool.IMAGE_SYM_CLASS_LABEL)],
            relocs)

    def test_a_byte_index_table_after_the_dword_table_is_clipped(self):
        """Both tables go, not just the relocated one.

        The relocation run no longer reaches the COMDAT's end when a byte
        table follows it, so the trailing-table test rejected the whole thing
        and left 24 bytes of DATA in the compared span. That reported as a
        length mismatch - 48 against 72 - on a function whose code was
        byte-identical.
        """
        data = self._object_with_compressed_table(2, [0, 1, 1, 0, 1, 0])
        code, _ = tool.object_code(data)
        self.assertEqual(code, RET)

    def test_a_tail_that_cannot_be_indices_is_left_alone(self):
        """The test that stops this inventing a boundary: every byte of an
        index table indexes the dword table, so a byte >= the entry count is
        proof the tail is something else."""
        data = self._object_with_compressed_table(2, [0, 1, 9])
        code, _ = tool.object_code(data)
        self.assertEqual(len(code), len(RET) + 8 + 3,
                         "9 cannot index a 2-entry table, so nothing is a "
                         "table here and the whole span stays")

    def test_a_relocated_tail_is_not_an_index_table(self):
        """An index table carries no relocations; anything that does is code."""
        code = RET
        table = b"\0\0\0\0" * 2
        relocs = {1: [(len(code), 1, tool.IMAGE_REL_I386_DIR32),
                      (len(code) + 4, 1, tool.IMAGE_REL_I386_DIR32),
                      (len(code) + 8, 2, tool.IMAGE_REL_I386_DIR32)]}
        data = build_coff(
            [(".text", code + table + b"\0\0\0\0")],
            [("?f@@YAXXZ", 1, 0, tool.IMAGE_SYM_CLASS_EXTERNAL),
             ("$L1", 1, len(code), tool.IMAGE_SYM_CLASS_LABEL),
             ("?g@@3HA", 0, 0, tool.IMAGE_SYM_CLASS_EXTERNAL)], relocs)
        body, _ = tool.object_code(data)
        self.assertEqual(len(body), len(RET) + 12)

    def test_padding_after_the_index_table_does_not_defeat_it(self):
        data = self._object_with_compressed_table(2, [0, 1, 1, 0x90, 0x90])
        code, _ = tool.object_code(data)
        self.assertEqual(code, RET)

    def test_a_relocation_to_a_non_label_is_not_a_table(self):
        """A DIR32 to an external - an ordinary global reference - must not be
        mistaken for a case arm."""
        relocs = {1: [(1, 1, tool.IMAGE_REL_I386_DIR32)]}
        data = build_coff(
            [(".text", b"\xa1\x00\x00\x00\x00" + RET)],
            [("?f@@YAXXZ", 1, 0, tool.IMAGE_SYM_CLASS_EXTERNAL),
             ("?g@@3HA", 0, 0, tool.IMAGE_SYM_CLASS_EXTERNAL)], relocs)
        code, mask = tool.object_code(data)
        self.assertEqual(len(code), 6)
        self.assertEqual(mask, {1, 2, 3, 4}, "the operand is unknowable, "
                                             "the opcode is not")


class TierLadderTests(unittest.TestCase):
    """The ladder must be able to produce every rung it declares."""

    def test_identical_input_is_byte_exact(self):
        code = b"\x8b\x44\x24\x04" + RET
        self.assertEqual(tool.compare(code, set(), 0, code, set())["tier"],
                         "BYTE_EXACT")

    def test_a_relocation_on_EITHER_side_forgives_the_difference(self):
        """Which side carries the relocation is not symmetric, and requiring
        BOTH made BYTE_EXACT unreachable for 83% of the corpus.

        An absolute address is relocated in the IMAGE and not in an object
        built from a literal; a relative branch is relocated in the OBJECT and
        never in the image, because rebasing does not move it. An earlier
        version of this test asserted the opposite and passed, because every
        body it was ever run against was call-free."""
        left = b"\xa1\x00\x00\x40\x00" + RET
        right = b"\xa1\x00\x00\x00\x00" + RET
        mask = {1, 2, 3, 4}
        for left_mask, right_mask, why in (
                (mask, mask, "both sides"),
                (mask, set(), "image side only - an absolute address"),
                (set(), mask, "object side only - a relative branch")):
            self.assertEqual(
                tool.compare(left, left_mask, 0, right, right_mask)["tier"],
                "BYTE_EXACT", why)

    def test_an_opcode_byte_is_never_masked(self):
        """The relocation covers the DISPLACEMENT, so a call at the wrong
        offset, or a different instruction entirely, is still caught. This is
        what stops the relaxed rule from forgiving too much."""
        left = b"\xe8\x00\x00\x00\x00" + RET      # call rel32
        right = b"\xb8\x00\x00\x00\x00" + RET     # mov eax, imm32
        mask = {1, 2, 3, 4}
        self.assertNotEqual(
            tool.compare(left, mask, 0, right, mask)["tier"], "BYTE_EXACT",
            "the opcode differs and opcode bytes carry no relocation")

    def test_a_verdict_reports_how_much_it_actually_compared(self):
        """A body that is mostly relocation is mostly uncompared, and must not
        read as a strongly evidenced match."""
        code = b"\xe8\x00\x00\x00\x00" + RET
        result = tool.compare(code, set(), 0, code, {1, 2, 3, 4})
        self.assertEqual(result["tier"], "BYTE_EXACT")
        self.assertEqual(result["masked_bytes"], 4)
        self.assertEqual(result["compared_bytes"], 2)

    def test_a_wrong_displacement_is_shape_exact_and_localised(self):
        """The most actionable failure: right codegen, wrong field offset.
        Mnemonic-only comparison is blind to this."""
        left = b"\x8b\x41\x28" + RET        # mov eax, [ecx+0x28]
        right = b"\x8b\x41\x2c" + RET       # mov eax, [ecx+0x2c]
        result = tool.compare(left, set(), 0, right, set())
        self.assertEqual(result["tier"], "SHAPE_EXACT")
        self.assertEqual(result["differing_constants"],
                         [(0, "mov", "0x28", "0x2c")])

    def test_a_wrong_vtable_slot_is_caught(self):
        """`call [eax+0x64]` vs `call [eax+0x68]`. With 5,159 unresolved
        indirect call sites this is the case that most needs catching, and it
        is exactly what discarding operands hides."""
        left = b"\xff\x50\x64" + RET
        right = b"\xff\x50\x68" + RET
        result = tool.compare(left, set(), 0, right, set())
        self.assertEqual(result["tier"], "SHAPE_EXACT")
        self.assertTrue(result["differing_constants"])

    def test_shape_exact_is_not_redundant_with_byte_exact(self):
        """The tier's whole purpose. If this ever passes as BYTE_EXACT the
        ladder has regressed to the draft that kept immediates."""
        left, right = b"\x8b\x41\x28" + RET, b"\x8b\x41\x2c" + RET
        self.assertNotEqual(tool.compare(left, set(), 0, right, set())["tier"],
                            "BYTE_EXACT")

    def test_a_different_encoding_length_is_not_shape_exact(self):
        """`add eax, 1` (short form) against `add eax, 0x1000` (long form) is a
        different instruction SELECTION, not a different constant.

        MNEMONIC_ONLY is the right verdict and the reason is worth stating: the
        mnemonic sequence really is identical, so the tier that compares only
        mnemonics must say so. What matters is that the upper two tiers REFUSE
        it - a shorter encoding is a choice the compiler made differently, and
        calling that "same shape, different constant" would let a genuinely
        different instruction selection pass as a near miss."""
        left = b"\x83\xc0\x01" + RET
        right = b"\x05\x00\x10\x00\x00" + RET
        tier = tool.compare(left, set(), 0, right, set())["tier"]
        self.assertNotIn(tier, ("BYTE_EXACT", "SHAPE_EXACT"))
        self.assertEqual(tier, "MNEMONIC_ONLY")

    def test_a_changed_mnemonic_is_a_mismatch_with_an_index(self):
        left = b"\x01\xd8" + RET           # add eax, ebx
        right = b"\x29\xd8" + RET          # sub eax, ebx
        result = tool.compare(left, set(), 0, right, set())
        self.assertEqual(result["tier"], "MISMATCH")
        self.assertEqual(result["first_divergence"], 0)

    def test_the_ret_pop_count_reaches_the_verdict(self):
        """Capstone spells `ret 8` and `ret` with the same mnemonic, so the
        callee-pop count - the only ground truth for arity - used to be
        discarded. A wrong convention must not pass as a match."""
        self.assertEqual(tool.mnemonics(b"\xc2\x08\x00"), ["ret 8"])
        self.assertEqual(tool.mnemonics(RET), ["ret"])
        self.assertNotEqual(
            tool.compare(b"\xc2\x08\x00", set(), 0, RET, set())["tier"],
            "BYTE_EXACT")


class PaddingSymmetryTests(unittest.TestCase):
    """Trailing alignment padding must be stripped from BOTH sides or from
    neither. Stripping only the rebuilt side makes the lengths differ whenever
    the catalogued span includes padding, and then NO source can match - a
    false MISMATCH that reads as the author's fault."""

    def test_the_object_side_strips_trailing_padding(self):
        data = build_coff([(".text", RET + b"\x90\x90\x90")],
                          [("?f@@YAXXZ", 1, 0, tool.IMAGE_SYM_CLASS_EXTERNAL)])
        self.assertEqual(tool.object_code(data)[0], RET)

    def test_int3_padding_is_stripped_too(self):
        data = build_coff([(".text", RET + b"\xcc\xcc")],
                          [("?f@@YAXXZ", 1, 0, tool.IMAGE_SYM_CLASS_EXTERNAL)])
        self.assertEqual(tool.object_code(data)[0], RET)

    def test_an_interior_nop_is_never_stripped(self):
        """Only the END is trimmed, so a body that really executes a `nop`
        keeps it."""
        data = build_coff([(".text", b"\x90" + RET)],
                          [("?f@@YAXXZ", 1, 0, tool.IMAGE_SYM_CLASS_EXTERNAL)])
        self.assertEqual(tool.object_code(data)[0], b"\x90" + RET)


class SimilarityTests(unittest.TestCase):
    """First-divergence alone under-reports large near-misses. A 1,187 B body
    whose mnemonics agreed 95.6% reported `MISMATCH` at index 2 because one
    prologue `push` was missing and everything shifted by one - identical to
    what a wholly wrong body reports."""

    def test_a_near_miss_reports_high_similarity(self):
        left = b"\x55\x8b\xec" + b"\x40" * 20 + RET       # push ebp; mov; inc*20
        right = b"\x8b\xec" + b"\x40" * 20 + RET          # prologue push missing
        result = tool.compare(left, set(), 0, right, set())
        self.assertEqual(result["tier"], "MISMATCH")
        self.assertGreater(result["mnemonic_similarity"], 0.9,
                           "a one-instruction prologue difference is not a "
                           "wholly different body")

    def test_a_wholly_different_body_reports_low_similarity(self):
        left = b"\x40" * 20 + RET
        right = b"\x48" * 20 + RET                        # dec instead of inc
        result = tool.compare(left, set(), 0, right, set())
        self.assertLess(result["mnemonic_similarity"], 0.2)

    def test_similarity_never_softens_the_tier(self):
        """Reporting how close a body is must not turn a MISMATCH into a
        match - that is the 'honest SKIP into a flattering PASS' move."""
        left = b"\x55\x8b\xec" + b"\x40" * 20 + RET
        right = b"\x8b\xec" + b"\x40" * 20 + RET
        self.assertEqual(tool.compare(left, set(), 0, right, set())["tier"],
                         "MISMATCH")

    def test_the_edit_list_localises_the_differences(self):
        left = b"\x40" * 5 + b"\x48" + b"\x40" * 5 + RET
        right = b"\x40" * 11 + RET
        result = tool.compare(left, set(), 0, right, set())
        self.assertGreaterEqual(result["edit_count"], 1)
        self.assertTrue(result["edits"])


class FlagSetTests(unittest.TestCase):
    """Half this image keeps its frame pointer and half omits it, so no single
    `/Oy` setting can match both. These pin that both are tried and that the
    better verdict wins."""

    def test_the_cross_of_optimisation_level_and_frame_pointer_is_offered(self):
        """Both axes vary across the image and neither can be pinned once.

        Frame pointer: 1,518 remaining targets open `push ebp; mov ebp, esp`
        and 1,544 do not. Optimisation level: `_malloc` reproduces only under
        `/O1`, and `sub_515430` clears a stack argument with `pop` where `/O2`
        emits `add esp, 4`."""
        self.assertEqual(len(tool.FLAG_SETS), 4)
        for level in ("/O1", "/O2"):
            at_level = [f for f in tool.FLAG_SETS if level in f]
            self.assertEqual(len(at_level), 2, f"{level} needs both frames")
            self.assertEqual(len([f for f in at_level if "/Oy-" in f]), 1)

    def test_every_flag_set_keeps_the_settings_that_are_not_optional(self):
        """`/GX` reproduces the EH funclets 387 bodies carry; `/Gy` is
        confirmed by the image's own COMDAT folding; `/GR-` and `/c` are the
        original experiment's. Only `/Oy` and the `/O` level may vary."""
        for flags in tool.FLAG_SETS:
            for required in ("/c", "/Gy", "/GR-", "/GX"):
                self.assertIn(required, flags)

    def test_a_tie_on_tier_is_broken_by_how_close_the_result_is(self):
        """Ranking on tier alone let the FIRST flag set win whenever every set
        scored MISMATCH - so a frameless function was reported against the
        framed build even though the frameless attempt was visibly closer."""
        closer = {"tier": "MISMATCH", "mnemonic_similarity": 0.95}
        further = {"tier": "MISMATCH", "mnemonic_similarity": 0.40}
        self.assertTrue(tool._better(closer, further))
        self.assertFalse(tool._better(further, closer))

    def test_a_better_tier_always_wins_regardless_of_similarity(self):
        exact = {"tier": "BYTE_EXACT", "mnemonic_similarity": 0.0}
        near = {"tier": "MISMATCH", "mnemonic_similarity": 0.99}
        self.assertTrue(tool._better(exact, near))
        self.assertFalse(tool._better(near, exact))

    def test_the_tier_order_ranks_best_first(self):
        self.assertEqual(tool.TIER_ORDER[0], "BYTE_EXACT")
        self.assertLess(tool.TIER_ORDER.index("SHAPE_EXACT"),
                        tool.TIER_ORDER.index("MNEMONIC_ONLY"))
        self.assertLess(tool.TIER_ORDER.index("MNEMONIC_ONLY"),
                        tool.TIER_ORDER.index("MISMATCH"))
        for tier in ("BYTE_EXACT", "SHAPE_EXACT", "MNEMONIC_ONLY", "MISMATCH",
                     "NO_COMPILE", "SHARED_TAIL", "REFUSED"):
            self.assertIn(tier, tool.TIER_ORDER,
                          "a tier the comparator can emit but the ranker does "
                          "not know would raise ValueError mid-sweep")


class NormalisationTests(unittest.TestCase):
    """Nothing may be merged that the compiler chose."""

    def test_it_refuses_to_merge_instructions_that_differ(self):
        for left, right in ((b"\x8d\x04\x18", b"\x89\xd8"),   # lea vs mov
                            (b"\x01\xd8", b"\x29\xd8"),       # add vs sub
                            (b"\x74\x02", b"\x75\x02"),       # je vs jne
                            (b"\x40", b"\x83\xc0\x02")):      # inc vs add
            self.assertNotEqual(tool.mnemonics(left), tool.mnemonics(right))


class SpanClassifierTests(unittest.TestCase):
    """Which bytes are even in scope. Measured: all 448 extra spans in this
    image decode as code, none is a jump table, and 387 end in the
    `__CxxFrameHandler` thunk."""

    def test_a_selfmod_span_refuses_the_whole_row(self):
        layout = tool.BodyLayout(primary=[(0x401000, 0x401010)],
                                 selfmod=[(0x664100, 0x664110)])
        self.assertIn("_SELFMOD", layout.refusal)

    def test_a_row_with_no_primary_span_is_refused(self):
        self.assertTrue(tool.BodyLayout().refusal)

    def test_a_clean_row_is_not_refused(self):
        self.assertEqual(
            tool.BodyLayout(primary=[(0x401000, 0x401010)]).refusal, "")


class RatchetPolicyTests(unittest.TestCase):
    def test_only_byte_exact_advances_recovery_progress(self):
        wrong_offset = tool.compare(
            b"\x8b\x41\x28" + RET, set(), 0,
            b"\x8b\x41\x2c" + RET, set())
        self.assertEqual(wrong_offset["tier"], "SHAPE_EXACT")
        self.assertNotIn(wrong_offset["tier"], ("BYTE_EXACT",))
        self.assertIn("BYTE_EXACT", ("BYTE_EXACT",))

    def test_cli_success_also_requires_byte_exact(self):
        self.assertEqual(tool.recovery_exit_code("BYTE_EXACT"), 0)
        self.assertEqual(tool.recovery_exit_code("SHAPE_EXACT"), 1)


class ProvenanceTests(unittest.TestCase):
    def test_the_skip_path_names_what_is_missing(self):
        reason = tool.available()
        if reason:
            self.assertTrue(reason.strip(), "a skip must say what is absent")

    def test_no_opcode_byte_is_ever_printed(self):
        """The tool reports mnemonic NAMES, counts and indices. If a future
        change starts printing bytes, this is what should stop it."""
        result = tool.compare(b"\x01\xd8" + RET, set(), 0, b"\x29\xd8" + RET,
                              set())
        self.assertNotIn("\\x", str(result))
        for value in result.values():
            self.assertNotIsInstance(value, (bytes, bytearray))


class SubjectIsPassedTests(unittest.TestCase):
    """Every caller must actually HAND `object_code` the subject's name.

    `choose_subject_symbol` was written to pick the symbol the marker names
    out of several, and then no call site passed a name - so for a whole day
    it could only ever fall back to filtering the compiler's own `??_G`/`??_H`
    helpers, and a unit that defined anything beside its subject still
    refused. A mechanism nobody invokes is the shape this repository keeps
    finding in itself; a test that reads the call sites is the only thing
    that can see it, because the fallback path passes every unit test.
    """

    CALLERS = ("byte_match.py", "byte_match_census.py")

    def call_sites(self, name):
        text = (Path(__file__).resolve().parent / name).read_text()
        # `object_code(` through to its closing paren, across line breaks.
        return re.findall(r"object_code\((?:[^()]|\([^()]*\))*\)", text)

    def test_every_call_passes_a_subject(self):
        for name in self.CALLERS:
            sites = [one for one in self.call_sites(name)
                     if not one.startswith("object_code(data: bytes")]
            self.assertTrue(sites, f"{name} names no call at all")
            for site in sites:
                self.assertIn("subject=", " ".join(site.split()),
                              f"{name}: {site}")


class SubjectSymbolTests(unittest.TestCase):
    """Which external `.text` symbol is the subject.

    The rule used to be "there is exactly one", which the COMPILER defeats:
    a virtual destructor makes VC6 emit `??_G` beside the subject, an array
    member makes it emit `??_H`, and VC6's `<string>` instantiates
    `?id@?$ctype@G@std@@$E`. Each was worked around on its own - the last one
    is why `byte_match_census.std_shim` includes that header conditionally -
    and the first blocks every virtual destructor in the image, of which the
    catalogue holds 117.
    """

    class Symbol:
        def __init__(self, name):
            self.name = name

    def pairs(self, *names):
        return [(self.Symbol(name), f"section for {name}") for name in names]

    def test_a_static_subject_is_reachable_when_named(self):
        """zlib declares `build_tree` as `local`, which is `static`, and the
        shipped image holds a standalone `_build_tree`. `/Gy` gives it its own
        COMDAT with a STATIC symbol of that name, so the bytes were always
        there and only the selector could not reach them."""
        chosen = tool.choose_subject_symbol(
            [], "_build_tree", self.pairs("_build_tree", "_gen_bitlen"))
        self.assertEqual("_build_tree", chosen[0].name)

    def test_an_external_of_that_name_still_wins(self):
        chosen = tool.choose_subject_symbol(
            self.pairs("_build_tree"), "_build_tree",
            [(self.Symbol("_build_tree"), "the static one")])
        self.assertEqual("section for _build_tree", chosen[1])

    def test_statics_are_ignored_without_a_name(self):
        """The count rule is about AMBIGUITY, and a unit's statics are exactly
        the helpers it was never meant to score. Consulting them unasked would
        make every helper a candidate subject."""
        with self.assertRaises(ValueError):
            tool.choose_subject_symbol([], None, self.pairs("_helper"))

    def test_a_static_that_is_not_the_subject_is_not_chosen(self):
        with self.assertRaises(ValueError):
            tool.choose_subject_symbol([], "_wanted", self.pairs("_other"))

    def test_a_lone_symbol_is_the_subject(self):
        found = self.pairs("?run@Win@@QAEHH@Z")
        self.assertEqual("?run@Win@@QAEHH@Z",
                         tool.choose_subject_symbol(found)[0].name)

    def test_a_deleting_destructor_beside_the_subject_is_not_it(self):
        found = self.pairs("??0Win@@QAE@XZ", "??_GWin@@UAEPAXI@Z")
        self.assertEqual("??0Win@@QAE@XZ",
                         tool.choose_subject_symbol(found)[0].name)

    def test_an_array_helper_beside_the_subject_is_not_it(self):
        found = self.pairs("??_HNewTechWin@@QAEPAXPAXIIP6APAX0@Z@Z",
                           "??0NewTechWin@@QAE@XZ")
        self.assertEqual("??0NewTechWin@@QAE@XZ",
                         tool.choose_subject_symbol(found)[0].name)

    def test_a_deleting_destructor_alone_is_still_the_subject(self):
        # 117 of them are recovery targets in their own right, so the filter
        # must not empty the list it is filtering.
        found = self.pairs("??_GWin@@UAEPAXI@Z")
        self.assertEqual("??_GWin@@UAEPAXI@Z",
                         tool.choose_subject_symbol(found)[0].name)

    def test_two_hand_written_definitions_still_refuse(self):
        # The rule this replaces was guarding something real: a helper defined
        # beside the subject gets inlined into it and the original did not
        # inline it. Nothing above may weaken that.
        found = self.pairs("?helper@@YAHH@Z", "?subject@@YAHH@Z")
        with self.assertRaises(ValueError) as caught:
            tool.choose_subject_symbol(found)
        self.assertIn("found 2", str(caught.exception))

    def test_a_named_subject_is_taken_from_a_crowd(self):
        found = self.pairs("?helper@@YAHH@Z", "?subject@@YAHH@Z")
        self.assertEqual("?subject@@YAHH@Z",
                         tool.choose_subject_symbol(
                             found, "?subject@@YAHH@Z")[0].name)

    def test_a_named_subject_matches_the_cdecl_decoration(self):
        found = self.pairs("_init_opening", "?other@@YAHH@Z")
        self.assertEqual("_init_opening",
                         tool.choose_subject_symbol(
                             found, "init_opening")[0].name)

    def test_a_name_that_matches_nothing_says_so(self):
        found = self.pairs("?a@@YAHH@Z", "?b@@YAHH@Z")
        with self.assertRaises(ValueError) as caught:
            tool.choose_subject_symbol(found, "?missing@@YAHH@Z")
        self.assertIn("?missing@@YAHH@Z", str(caught.exception))


if __name__ == "__main__":
    unittest.main()
