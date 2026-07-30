#!/usr/bin/env python3
"""Checks for the whole-image lifter's non-obvious decisions.

The generator's output is two million lines nobody reads, so the parts worth
testing are the small judgements that decide what that output MEANS: which
functions are excluded, how big the flat memory model has to be, and whether
the dispatch table is total. A silent mistake in any of those produces a
skeleton that compiles and links perfectly while being wrong about the image.
"""

from __future__ import annotations

import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import lift_whole_image as lifter


class FakeSection:
    def __init__(self, name: str, rva: int, virtual: int, raw: int):
        self.Name = name.encode("ascii").ljust(8, b"\0")
        self.VirtualAddress = rva
        self.Misc_VirtualSize = virtual
        self.SizeOfRawData = raw
        self.PointerToRawData = rva


class FakeHeader:
    def __init__(self, base: int):
        self.ImageBase = base


class FakePE:
    def __init__(self, sections):
        self.OPTIONAL_HEADER = FakeHeader(0x00400000)
        self.sections = sections


def build_pe():
    return FakePE([
        FakeSection(".text", 0x1000, 0x2000, 0x2000),
        # Virtual size far exceeds raw size, which is how .data carries
        # megabytes of zero-initialised globals in a small file. A span
        # computed from raw sizes would under-allocate the model and turn
        # every BSS access into an out-of-bounds write.
        FakeSection(".data", 0x4000, 0x300000, 0x1000),
        FakeSection("_SELFMOD", 0x310000, 0x1000, 0x1000),
    ])


class ImageSpanTests(unittest.TestCase):
    def test_span_covers_virtual_size_not_raw(self):
        base, size = lifter.image_span(build_pe())
        self.assertEqual(base, 0x00400000)
        # .data ends at rva 0x304000; the span must reach it, page-rounded.
        self.assertGreaterEqual(size, 0x310000 + 0x1000)
        self.assertEqual(size % 0x1000, 0)

    def test_span_is_page_rounded(self):
        pe = FakePE([FakeSection(".text", 0x1000, 0x1001, 0x1001)])
        _, size = lifter.image_span(pe)
        self.assertEqual(size, 0x3000)


class SectionTests(unittest.TestCase):
    def test_section_name_is_lowercased_and_trimmed(self):
        pe = build_pe()
        self.assertEqual(lifter.section_of(pe, 0x00400000 + 0x1000), ".text")
        self.assertEqual(
            lifter.section_of(pe, 0x00400000 + 0x310000), "_selfmod")

    def test_selfmod_is_declared_unliftable(self):
        # Self-modifying code cannot have a static translation by definition,
        # so this is the one exclusion the generator must never lose.
        self.assertIn("_selfmod", lifter.UNLIFTABLE_SECTIONS)

    def test_address_outside_every_section_is_not_attributed(self):
        self.assertEqual(lifter.section_of(build_pe(), 0x7F000000), "")


class LoadFunctionsTests(unittest.TestCase):
    def write_csv(self, directory: Path, rows: list[str]) -> Path:
        path = directory / "functions.csv"
        header = ("address,end_address,size,segment,name,prototype,"
                  "body_ranges,binary_kind,flags,source_locations,"
                  "source_statuses,redirect_exports,original_dependencies,"
                  "recovery_state,priority,notes,comments,call_target_count,"
                  "caller_count")
        path.write_text("\n".join([header] + rows) + "\n", encoding="utf-8")
        return path

    def test_external_library_rows_are_excluded(self):
        # The CRT and Windows come from the host toolchain. Lifting them would
        # translate code the build already supplies, and their addresses must
        # NOT enter the dispatch table or an indirect call into the CRT would
        # reach a lifted copy instead of the real one.
        with tempfile.TemporaryDirectory() as raw:
            directory = Path(raw)
            path = self.write_csv(directory, [
                "0x00401000,0x00401010,16,.text,game_fn,,,game,,,,,,unrecovered,,,,0,0",
                "0x00402000,0x00402010,16,.text,crt_fn,,,game,,,,,,external_library,,,,0,0",
            ])
            functions = lifter.load_functions(build_pe(), path)
        self.assertEqual([f["name"] for f in functions], ["game_fn"])

    def test_zero_size_rows_are_excluded(self):
        with tempfile.TemporaryDirectory() as raw:
            directory = Path(raw)
            path = self.write_csv(directory, [
                "0x00401000,0x00401000,0,.text,empty,,,game,,,,,,unrecovered,,,,0,0",
            ])
            self.assertEqual(lifter.load_functions(build_pe(), path), [])

    def test_functions_are_sorted_for_binary_search(self):
        # lifted_dispatch.cpp does a std::lower_bound over the emitted table,
        # so emission order IS the correctness condition, not a formatting
        # preference. Out of order, lookups silently miss.
        with tempfile.TemporaryDirectory() as raw:
            directory = Path(raw)
            path = self.write_csv(directory, [
                "0x00403000,0x00403010,16,.text,c,,,game,,,,,,unrecovered,,,,0,0",
                "0x00401000,0x00401010,16,.text,a,,,game,,,,,,unrecovered,,,,0,0",
                "0x00402000,0x00402010,16,.text,b,,,game,,,,,,unrecovered,,,,0,0",
            ])
            functions = lifter.load_functions(build_pe(), path)
        self.assertEqual([f["name"] for f in functions], ["a", "b", "c"])


class BodySpanTests(unittest.TestCase):
    """`size` is the SUM of body_ranges, never a contiguous extent.

    402 of the 5,673 game functions (7.1%) carry a second span in the cold
    0x0065xxxx region, and for every one of them `address + size` runs past the
    first span into the next function - up to 2,102 bytes past, on the largest
    functions in the game. Reading that many bytes from the entry point was the
    original bug here, and it is invisible in skeleton mode because a wrong
    byte count only inflates a placeholder body.
    """

    def test_multiple_ranges_are_parsed_not_summed(self):
        spans = lifter.body_spans({
            "address": "0x00612830", "size": "418",
            "body_ranges": "0x00612830-0x0061295F;0x00663060-0x006630D3",
        })
        self.assertEqual(spans, [(0x00612830, 0x0061295F),
                                 (0x00663060, 0x006630D3)])
        # The summed size is 418, but the first span is only 303 bytes. A
        # contiguous read of 418 would reach 0x006129D2, well past the body.
        self.assertEqual(sum(high - low for low, high in spans), 418)
        self.assertNotEqual(spans[0][1], 0x00612830 + 418)

    def test_single_range_still_works(self):
        self.assertEqual(
            lifter.body_spans({"address": "0x00401000", "size": "16",
                               "body_ranges": "0x00401000-0x00401010"}),
            [(0x00401000, 0x00401010)])

    def test_missing_ranges_fall_back_to_address_plus_size(self):
        self.assertEqual(
            lifter.body_spans({"address": "0x00401000", "size": "16",
                               "body_ranges": ""}),
            [(0x00401000, 0x00401010)])

    def test_spans_are_sorted(self):
        # The cold span can be listed first; emission and label resolution both
        # assume ascending order.
        spans = lifter.body_spans({
            "address": "0x00401000", "size": "32",
            "body_ranges": "0x00663060-0x00663070;0x00401000-0x00401010",
        })
        self.assertEqual(spans, [(0x00401000, 0x00401010),
                                 (0x00663060, 0x00663070)])

    def test_loaded_functions_carry_spans_and_span_bytes(self):
        header = ("address,end_address,size,segment,name,prototype,"
                  "body_ranges,binary_kind,flags,source_locations,"
                  "source_statuses,redirect_exports,original_dependencies,"
                  "recovery_state,priority,notes,comments,call_target_count,"
                  "caller_count")
        row = ("0x00401000,0x00401010,32,.text,outlined,,"
               "0x00401000-0x00401010;0x00663000-0x00663010,"
               "game,,,,,,unrecovered,,,,0,0")
        with tempfile.TemporaryDirectory() as raw:
            path = Path(raw) / "functions.csv"
            path.write_text(header + "\n" + row + "\n", encoding="utf-8")
            functions = lifter.load_functions(build_pe(), path)
        self.assertEqual(len(functions[0]["spans"]), 2)
        self.assertEqual(functions[0]["span_bytes"], 32)


class SymbolTests(unittest.TestCase):
    def test_symbols_are_address_derived_and_unique(self):
        first = lifter.symbol_for({"address": 0x004BA870})
        second = lifter.symbol_for({"address": 0x004ba871})
        self.assertEqual(first, "lifted_004ba870")
        self.assertNotEqual(first, second)


class LoweringTests(unittest.TestCase):
    """One refused instruction must cost one trap, not a whole function.

    The tempting rule is to abandon a body at its first refusal, and it is
    wrong in a way that hides: 879 of the 5,655 liftable functions contain at
    least one instruction the lowerer refuses, so abandoning would throw away
    the other 99% of each of those bodies AND collapse a 6,491-entry histogram
    into "879 functions failed", which names nothing to build next.
    """

    @staticmethod
    def items(encoded: str, address: int = 0x00401000) -> list[tuple]:
        from capstone import CS_ARCH_X86, CS_MODE_32, Cs
        decoder = Cs(CS_ARCH_X86, CS_MODE_32)
        decoder.detail = True
        return [("code", one)
                for one in decoder.disasm(bytes.fromhex(encoded), address)]

    def test_a_refusal_traps_one_instruction_and_lowering_continues(self):
        # nop; bswap eax; inc eax - the middle instruction has no lowering.
        # Both neighbours must still be reached, and the trap must name the
        # refused address rather than the function's. (`div` was the example
        # here until the multiply/divide helpers landed and it started to
        # lower.)
        lines, refused = lifter.lower_body(self.items("900fc840"))
        self.assertEqual({"bswap": 1}, dict(refused))
        text = "\n".join(lines)
        self.assertIn("opensmacx_trap(0x00401001U, \"bswap\");", text)
        # The inc after the bswap is what proves lowering did not stop.
        self.assertIn("opensmacx_inc32", text)

    def test_the_histogram_counts_every_occurrence_not_every_function(self):
        # Two bswaps in one body are two entries in the work queue: the phase 4
        # ordering is by how often an instruction appears, so deduplicating
        # per function would rank a rare instruction in many functions above a
        # common one concentrated in few.
        _, refused = lifter.lower_body(self.items("0fc80fc8"))
        self.assertEqual({"bswap": 2}, dict(refused))

    def test_a_branch_inside_the_function_gets_a_label(self):
        # jmp +0 - the target is the next instruction, which is inside this
        # body, so it must be a goto and the label must exist.
        lines = lifter.lower_body(self.items("eb0090"))[0]
        text = "\n".join(lines)
        self.assertIn("goto L_00401002;", text)
        self.assertIn("L_00401002: ;", text)

    def test_a_branch_outside_the_function_gets_no_label(self):
        # jmp +0x40 lands past the end of this two-instruction body, so it is
        # a tail call. Emitting a label for it would be a goto to nowhere and
        # would not compile; emitting the dispatch is the whole reason the
        # label set is computed per function.
        text = "\n".join(lifter.lower_body(self.items("eb4090"))[0])
        self.assertIn("opensmacx_dispatch(0x00401042U)(s);", text)
        self.assertNotIn("goto", text)

    def test_a_self_recursive_call_is_not_turned_into_a_goto(self):
        # call +0, whose target is the next instruction and therefore inside
        # the body. A call is still a call: lowering it as a jump would skip
        # the pushed return address and the matching RET would then pop a
        # word of the caller's frame.
        text = "\n".join(lifter.lower_body(self.items("e80000000090"))[0])
        self.assertIn("opensmacx_dispatch(0x00401005U)(s);", text)
        self.assertNotIn("goto", text)

    def test_every_label_is_followed_by_a_statement(self):
        # A C++17 label at the end of a compound statement is a syntax error,
        # and a NOP lowers to nothing at all - so `jmp +1; nop` puts a label
        # in front of an empty lowering at the end of the body.
        lines = lifter.lower_body(self.items("eb0090"))[0]
        # The NOP emits nothing, so the label really is the last line here.
        self.assertTrue(lines[-1].startswith("L_"), lines)
        for line in lines:
            if line.startswith("L_"):
                self.assertTrue(line.endswith(": ;"), line)

    def test_undecodable_bytes_become_a_trap_rather_than_a_gap(self):
        # 0xff 0xff does not decode. Dropping it would emit a body that runs
        # straight past a stretch of the original as if it were not there.
        lines, refused = lifter.lower_body(
            [("data", (0x00401004, 2))])
        self.assertEqual({"(undecodable)": 1}, dict(refused))
        self.assertIn("opensmacx_trap(0x00401004U, \"2 undecodable bytes\");",
                      lines[0])


class EmissionTests(unittest.TestCase):
    def test_unliftable_function_body_traps_instead_of_lowering(self):
        with tempfile.TemporaryDirectory() as raw:
            out = Path(raw)
            functions = [{
                "address": 0x00664000, "size": 16, "name": "sub_664000",
                "spans": [(0x00664000, 0x00664010)], "span_bytes": 16,
                "section": "_selfmod", "instructions": 4,
                "fully_decoded": True, "state": "unrecovered",
            }]
            shards = lifter.write_shards(out, functions, 1)
            text = shards[0].read_text(encoding="utf-8")
        self.assertIn("self-modifying section", text)
        # A trapping body must contain no placeholder work at all, or a later
        # reader would take the statements for a real partial lowering.
        self.assertNotIn("opensmacx_cmp32", text)

    def test_statement_budget_scales_with_instruction_count(self):
        small = lifter.body_statements(
            {"address": 0x00401000, "instructions": 10})
        large = lifter.body_statements(
            {"address": 0x00401000, "instructions": 100})
        self.assertEqual(len(small), 10 * lifter.STATEMENTS_PER_INSTRUCTION)
        self.assertEqual(len(large), 100 * lifter.STATEMENTS_PER_INSTRUCTION)

    def test_image_source_carries_no_original_bytes(self):
        # The generated tree must describe the image's shape and never its
        # contents; contents are loaded at runtime from the user's own file.
        with tempfile.TemporaryDirectory() as raw:
            out = Path(raw)
            lifter.write_image(out, build_pe(), 0x00400000, 0x1000)
            text = (out / "lifted_image.cpp").read_text(encoding="utf-8")
        # Sized from the ONE span constant, never by restating its parts. The
        # old spelling named the image and the stack, so when the guest heap
        # was added for --build-state it silently covered neither - the array
        # would have been 2 MiB short of the span the loader hands out, and
        # only the static_assert below it caught that. Deriving means the next
        # region cannot reintroduce the gap, so the assertion is on the
        # DERIVATION and not merely on today's text.
        self.assertIn("unsigned char opensmacx_image[OpensmacxSpanSize];", text)
        self.assertNotIn("OpensmacxImageSize + OpensmacxStackSpanSize", text)
        self.assertNotIn("0x90,", text)

    def test_image_source_ties_its_size_to_the_stack_geometry(self):
        # lifted_loader.h derives the stack's address range from
        # OpensmacxImageSize and hands out a megabyte above the image. Nothing
        # in the type system connects that to the array's real size, because
        # lifted_runtime.h declares opensmacx_image[] incomplete. The
        # definition is the only place the two can be checked against each
        # other, so the check has to be emitted here or it exists nowhere.
        with tempfile.TemporaryDirectory() as raw:
            out = Path(raw)
            lifter.write_image(out, build_pe(), 0x00400000, 0x1000)
            text = (out / "lifted_image.cpp").read_text(encoding="utf-8")
        self.assertIn("static_assert(sizeof(opensmacx_image) == "
                      "OpensmacxSpanSize,", text)

    def test_dispatch_table_holds_every_function(self):
        with tempfile.TemporaryDirectory() as raw:
            out = Path(raw)
            functions = [
                {"address": 0x00401000, "size": 8, "name": "a",
                 "section": ".text", "instructions": 2,
                 "fully_decoded": True, "state": "unrecovered"},
                {"address": 0x00402000, "size": 8, "name": "b",
                 "section": ".text", "instructions": 2,
                 "fully_decoded": True, "state": "unrecovered"},
            ]
            lifter.write_dispatch(out, functions)
            text = (out / "lifted_dispatch.cpp").read_text(encoding="utf-8")
        self.assertIn("0x00401000U, &lifted_00401000", text)
        self.assertIn("0x00402000U, &lifted_00402000", text)

    def test_selfmod_functions_still_appear_in_the_dispatch_table(self):
        # They are not lowered, but they must remain reachable: an indirect
        # call into _SELFMOD has to hit the trap and name the address, not
        # fall off the end of the table with a misleading message.
        with tempfile.TemporaryDirectory() as raw:
            out = Path(raw)
            functions = [{
                "address": 0x00664000, "size": 16, "name": "sub_664000",
                "section": "_selfmod", "instructions": 4,
                "fully_decoded": True, "state": "unrecovered",
            }]
            lifter.write_dispatch(out, functions)
            text = (out / "lifted_dispatch.cpp").read_text(encoding="utf-8")
        self.assertIn("0x00664000U, &lifted_00664000", text)



class CrtInitBoundsTests(unittest.TestCase):
    """The two initialiser arrays, read from __cinit rather than guessed.

    A name-based guess cannot do this job and the measurement says how badly:
    of 434 live .CRT$XC entries 42 are not named `??__E`, and NONE of the four
    .CRT$XI entries are - including ___onexitinit, which builds the atexit table
    every C++ initialiser then registers with. A walk driven by the `??__E` name
    filter therefore runs the wrong subset and stops in __onexit.
    """

    class FakePE:
        def __init__(self, code, base=0x00400000, at=0x00644DD2):
            self.OPTIONAL_HEADER = type("H", (), {"ImageBase": base})()
            offset = at - base
            self.sections = [type("S", (), {
                "VirtualAddress": 0x1000,
                "Misc_VirtualSize": offset + len(code),
                "SizeOfRawData": offset + len(code),
                "PointerToRawData": 0x200,
            })()]
            self.__data__ = (b"\x00" * (0x200 + offset - 0x1000) + code)

    def _cinit(self, first=(0x006826D0, 0x006826E4),
               second=(0x00682000, 0x006826CC)):
        # push end / push begin / call __initterm, twice. rel32 call from the
        # instruction after it to 0x00644ED8.
        def push(value):
            return b"\x68" + value.to_bytes(4, "little")

        code = b""
        at = 0x00644DD2
        for begin, end in (first, second):
            code += push(end) + push(begin)
            at_call = at + len(code)
            rel = (0x00644ED8 - (at_call + 5)) & 0xFFFFFFFF
            code += b"\xe8" + rel.to_bytes(4, "little")
        return code + b"\xc3"

    def test_both_ranges_are_read_in_the_order_cinit_walks_them(self):
        bounds = lifter.crt_init_bounds(self.FakePE(self._cinit()))
        self.assertEqual(
            [("XI", 0x006826D0, 0x006826E4), ("XC", 0x00682000, 0x006826CC)],
            bounds)

    def test_XI_comes_FIRST_because_it_builds_the_atexit_table(self):
        bounds = lifter.crt_init_bounds(self.FakePE(self._cinit()))
        self.assertEqual("XI", bounds[0][0])

    def test_every_range_is_ASCENDING(self):
        # cdecl pushes right to left, so the LAST push is __initterm's FIRST
        # argument. I read it the other way round first and got two negative
        # spans, which emit as an empty walk that looks like a table with no
        # entries rather than like a bug.
        #
        # Asserted as a property of the RESULT rather than by damaging the guard
        # and expecting SystemExit: that form of the test does not fail when the
        # guard is removed - verified, with __pycache__ cleared - because the
        # reversed input still yields two ranges and returns normally. A test
        # that cannot fail is worth nothing, so this checks the thing that would
        # actually be wrong.
        for name, begin, end in lifter.crt_init_bounds(
                self.FakePE(self._cinit())):
            self.assertLess(begin, end, f"{name} range is not ascending")

    def test_a_reversed_push_order_is_REFUSED(self):
        with self.assertRaises(SystemExit):
            lifter.crt_init_bounds(self.FakePE(
                self._cinit(first=(0x006826E4, 0x006826D0))))

    def test_a_body_without_two_initterm_calls_is_refused(self):
        with self.assertRaises(SystemExit):
            lifter.crt_init_bounds(self.FakePE(b"\xc3"))


if __name__ == "__main__":
    unittest.main()

