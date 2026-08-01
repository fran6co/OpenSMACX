#!/usr/bin/env python3
"""Tests for tools/measure_exclusions.py.

Every fixture here is synthetic: a few dozen hand-assembled bytes standing in
for the pinned executable, so the classifiers can be driven through the exact
shapes that made the published figures wrong. Three of these cases exist
because the naive measurement they encode was actually published:

- `push -1; push imm32` counted as a frame registration, which invented ~685
  `_except_handler3` frames out of ordinary argument pushes;
- a per-function `__ehhandler` thunk counted as its own handler, which would
  report 387 distinct mechanisms instead of one;
- `out dx, al` decoded out of a switch table and reported as a port-I/O body.

The document-agreement check is here too, because a `--check` that cannot fail
is worth nothing: `test_compare_reports_a_drifted_figure` damages one number
and requires the disagreement to be named.
"""

from __future__ import annotations

import struct
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import measure_exclusions as measurer  # noqa: E402


BASE = 0x00401000
HANDLER = 0x00402000


def image(*chunks: tuple[int, bytes]) -> measurer.Image:
    """A synthetic image, always with something readable at HANDLER.

    `seh_registrations` only accepts a pushed value as a handler when the image
    has bytes there, which is what rejects `push 0xa0` on an image this small.
    """
    return measurer.Image(list(chunks) + [(HANDLER, b"\xcc" * 0x10)], BASE)


def cxx_thunk(func_info: int, handler: int, at: int) -> bytes:
    """`mov eax, <FuncInfo>; jmp <handler>` — VC6's per-function EH thunk."""
    displacement = handler - (at + 10)
    return b"\xb8" + struct.pack("<I", func_info) + b"\xe9" + struct.pack(
        "<i", displacement)


class SehCensusTests(unittest.TestCase):
    def test_finds_the_handler_pushed_before_the_store(self):
        body = (b"\x6a\xff"                                  # push -1
                b"\x68\x00\x20\x40\x00"                      # push 0x402000
                b"\x64\xa1\x00\x00\x00\x00"                  # mov eax, fs:[0]
                b"\x50"                                      # push eax
                + measurer.FS_ZERO_STORE_ESP)
        found = measurer.seh_registrations(image((BASE, body)))
        self.assertEqual([(BASE + 14, HANDLER)], found)

    def test_finds_the_store_first_prologue_variant(self):
        """VC6 also emits the fs load BEFORE the pushes, on large frames."""
        body = (b"\x55\x8b\xec"                              # push ebp; mov
                b"\x64\xa1\x00\x00\x00\x00"                  # mov eax, fs:[0]
                b"\x6a\xff"                                  # push -1
                b"\x68\x00\x20\x40\x00"                      # push 0x402000
                b"\x50"                                      # push eax
                + measurer.FS_ZERO_STORE_ESP)
        found = measurer.seh_registrations(image((BASE, body)))
        self.assertEqual([(BASE + 17, HANDLER)], found)

    def test_an_argument_push_pair_is_not_a_frame(self):
        """The published miscount. `push -1; push 0xa0; call` is two arguments.

        Counting the byte sequence returns 1,072 over the real image; only 385
        of those sit in front of a frame store.
        """
        body = (b"\x6a\xff"                                  # push -1
                b"\x68\xa0\x00\x00\x00"                      # push 0xa0
                b"\xe8\x00\x00\x00\x00")                     # call
        self.assertEqual([], measurer.seh_registrations(image((BASE, body))))

    def test_a_handler_outside_the_image_is_not_resolved(self):
        body = (b"\x68\x00\x00\x00\x90"                      # push 0x90000000
                + measurer.FS_ZERO_STORE_ESP)
        found = measurer.seh_registrations(image((BASE, body)))
        self.assertEqual([(BASE + 5, None)], found)

    def test_two_stores_in_one_body_are_two_registrations(self):
        one = b"\x68\x00\x20\x40\x00" + measurer.FS_ZERO_STORE_ESP
        found = measurer.seh_registrations(image((BASE, one + one)))
        self.assertEqual(2, len(found))
        self.assertEqual({HANDLER}, {handler for _, handler in found})


class HandlerResolutionTests(unittest.TestCase):
    def test_follows_the_per_function_thunk_to_the_shared_handler(self):
        thunk_at = 0x00402000
        handler = 0x00403000
        blob = bytearray(b"\x90" * 0x3000)
        blob[thunk_at - BASE:thunk_at - BASE + 10] = cxx_thunk(
            0x00404000, handler, thunk_at)
        resolved = measurer.resolve_handler(image((BASE, bytes(blob))),
                                            thunk_at)
        self.assertEqual(handler, resolved)

    def test_a_direct_handler_resolves_to_itself(self):
        blob = b"\x55\x8b\xec\x83\xec\x08\x53\x56\x57\x55"   # a real prologue
        self.assertEqual(BASE,
                         measurer.resolve_handler(image((BASE, blob)), BASE))

    def test_none_stays_none(self):
        self.assertIsNone(measurer.resolve_handler(image((BASE, b"")), None))


class JumpTableTests(unittest.TestCase):
    def test_a_table_of_addresses_into_the_function_is_data(self):
        spans = [(BASE, BASE + 0x100)]
        table = b"".join(struct.pack("<I", BASE + 0x40 + 8 * n)
                         for n in range(8))
        blob = b"\x90" * 0x20 + table + b"\x90" * 0x20
        self.assertTrue(
            measurer.looks_like_jump_table(image((BASE, blob)),
                                           BASE + 0x24, spans))

    def test_ordinary_code_is_not_a_table(self):
        spans = [(BASE, BASE + 0x100)]
        blob = b"\xba\xc8\x03\x00\x00\xee\x8a\x03\xc0\xe8\x02\xee" * 8
        self.assertFalse(
            measurer.looks_like_jump_table(image((BASE, blob)), BASE + 5,
                                           spans))

    def test_addresses_into_a_different_function_are_not_a_table(self):
        """Only entries inside the OWNING function count.

        A run of unrelated pointers - a vtable, a string table - is not
        evidence that this instruction is data.
        """
        spans = [(BASE, BASE + 0x100)]
        elsewhere = b"".join(struct.pack("<I", 0x00500000 + 8 * n)
                             for n in range(8))
        self.assertFalse(
            measurer.looks_like_jump_table(image((BASE, elsewhere)),
                                           BASE + 4, spans))


class PortIoTests(unittest.TestCase):
    def setUp(self):
        try:
            import capstone
        except ImportError:  # pragma: no cover - capstone is a hard dep here
            self.skipTest("capstone is not installed")
        self.disassembler = capstone.Cs(capstone.CS_ARCH_X86,
                                        capstone.CS_MODE_32)

    def row(self, low: int, high: int, name: str) -> dict:
        return {"address": "0x%08X" % low, "end_address": "0x%08X" % high,
                "body_ranges": "0x%08X-0x%08X" % (low, high),
                "size": str(high - low), "name": name,
                "recovery_state": "unrecovered", "binary_kind": "game"}

    def test_a_dac_write_loop_is_a_real_port_io_body(self):
        body = (b"\xba\xc8\x03\x00\x00"                      # mov edx, 0x3c8
                b"\xee"                                      # out dx, al
                b"\xba\xc9\x03\x00\x00"                      # mov edx, 0x3c9
                b"\xee"                                      # out dx, al
                b"\xc3")
        rows = [self.row(BASE, BASE + len(body), "sub_dac")]
        real, impostors = measurer.port_io_bodies(
            image((BASE, body)), rows, self.disassembler)
        self.assertEqual(["sub_dac"], [item["row"]["name"] for item in real])
        self.assertEqual([], impostors)
        self.assertEqual(["out dx, al", "out dx, al"], real[0]["sites"])

    def test_a_switch_table_that_decodes_as_out_is_not(self):
        """The impostor, reproduced exactly.

        A table of addresses into a function at `0x0040E6xx` carries the byte
        `0xE6` in every entry, and a sweep that desynchronises by one decodes
        each as `out 0x40, al`. This is why `battle_report`, `top_menu`,
        `probe` and ten others read as port-I/O bodies on a first pass.
        """
        table_base = 0x0040E600
        body = b"".join(struct.pack("<I", table_base + 8 * n)
                        for n in range(16))
        rows = [self.row(table_base, table_base + len(body), "sub_switch")]
        real, impostors = measurer.port_io_bodies(
            image((table_base, body)), rows, self.disassembler)
        self.assertEqual([], real)
        self.assertEqual(["sub_switch"],
                         [item["row"]["name"] for item in impostors])
        self.assertIn("out 0x40, al", impostors[0]["sites"])


class OwnerLookupTests(unittest.TestCase):
    def rows(self):
        return [
            {"address": "0x00401000", "end_address": "0x00401010",
             "body_ranges": "0x00401000-0x00401010;0x00650000-0x00650020",
             "size": "48", "name": "outlined", "recovery_state": "unrecovered",
             "binary_kind": "game"},
            {"address": "0x00401010", "end_address": "0x00401020",
             "body_ranges": "0x00401010-0x00401020", "size": "16",
             "name": "next", "recovery_state": "unrecovered",
             "binary_kind": "game"},
        ]

    def test_a_cold_span_belongs_to_the_function_that_owns_it(self):
        spans = measurer.owner_index(self.rows())
        self.assertEqual("outlined",
                         measurer.owner_of(spans, 0x00650008)["name"])

    def test_a_gap_between_spans_has_no_owner(self):
        spans = measurer.owner_index(self.rows())
        self.assertIsNone(measurer.owner_of(spans, 0x00640000))

    def test_an_address_below_every_span_has_no_owner(self):
        spans = measurer.owner_index(self.rows())
        self.assertIsNone(measurer.owner_of(spans, 0x00400000))

    def test_body_ranges_beats_end_minus_address(self):
        row = self.rows()[0]
        self.assertEqual([(0x00401000, 0x00401010),
                          (0x00650000, 0x00650020)],
                         measurer.body_spans(row))


class ClosureTests(unittest.TestCase):
    def edges(self, *pairs):
        return [{"source": "0x%08X" % source, "target": "0x%08X" % target,
                 "call_sites": []} for source, target in pairs]

    def test_reaches_callers_of_callers(self):
        edges = self.edges((0x1000, 0x2000), (0x2000, 0x3000))
        self.assertEqual({0x1000, 0x2000, 0x3000},
                         measurer.transitive_callers(edges, {0x3000}))

    def test_a_cycle_terminates(self):
        edges = self.edges((0x1000, 0x2000), (0x2000, 0x1000),
                           (0x2000, 0x3000))
        self.assertEqual({0x1000, 0x2000, 0x3000},
                         measurer.transitive_callers(edges, {0x3000}))

    def test_callees_are_not_reached(self):
        """Reaching DirectDraw is about callers, not callees."""
        edges = self.edges((0x1000, 0x2000))
        self.assertEqual({0x1000}, measurer.transitive_callers(edges, {0x1000}))


class DocumentAgreementTests(unittest.TestCase):
    DOC = ("prose\n\n```measured\n"
           "seh.frames = 402\n"
           "# a comment, and a blank line follow\n\n"
           "directx.percent_of_catalogued_bytes = 48.4\n"
           "```\n\nmore prose\n")

    def test_parses_ints_floats_comments_and_blanks(self):
        self.assertEqual(
            {"seh.frames": 402, "directx.percent_of_catalogued_bytes": 48.4},
            measurer.parse_measured_block(self.DOC))

    def test_a_document_without_the_block_is_an_error(self):
        with self.assertRaises(ValueError):
            measurer.parse_measured_block("no fence here")

    def test_a_malformed_line_is_an_error(self):
        with self.assertRaises(ValueError):
            measurer.parse_measured_block("```measured\nseh.frames 402\n```")

    def test_agreement_reports_nothing(self):
        declared = measurer.parse_measured_block(self.DOC)
        measured = dict(declared)
        self.assertEqual([], measurer.compare(measured, declared))

    def test_compare_reports_a_drifted_figure(self):
        """The positive control for --check: damage one number, see it named."""
        declared = measurer.parse_measured_block(self.DOC)
        measured = dict(declared, **{"seh.frames": 403})
        problems = measurer.compare(measured, declared)
        self.assertEqual(1, len(problems))
        self.assertIn("seh.frames", problems[0])
        self.assertIn("402", problems[0])
        self.assertIn("403", problems[0])

    def test_a_figure_the_document_never_declared_is_reported(self):
        declared = measurer.parse_measured_block(self.DOC)
        measured = dict(declared, **{"port_io.functions": 1})
        problems = measurer.compare(measured, declared)
        self.assertEqual(1, len(problems))
        self.assertIn("absent from the document", problems[0])

    def test_a_retired_figure_is_reported(self):
        declared = measurer.parse_measured_block(self.DOC)
        measured = {"seh.frames": 402}
        problems = measurer.compare(measured, declared)
        self.assertEqual(1, len(problems))
        self.assertIn("no longer measured", problems[0])

    def test_the_committed_document_parses(self):
        if not measurer.EXCLUSIONS_DOC.exists():
            self.skipTest("docs/EXCLUSIONS.md is not present")
        declared = measurer.parse_measured_block(
            measurer.EXCLUSIONS_DOC.read_text())
        self.assertIn("seh.frames", declared)


class LiveImageTests(unittest.TestCase):
    """Runs only where the pinned executable is present, and skips honestly."""

    def setUp(self):
        if not measurer.DEFAULT_EXE.exists():
            self.skipTest("the pinned executable is not present")
        if not measurer.EXCLUSIONS_DOC.exists():
            self.skipTest("docs/EXCLUSIONS.md is not present")

    def test_the_document_agrees_with_the_image(self):
        measured = measurer.measure(measurer.DEFAULT_EXE,
                                    measurer.FUNCTIONS_CSV,
                                    measurer.CALLGRAPH_JSON)
        measured.pop("_detail")
        declared = measurer.parse_measured_block(
            measurer.EXCLUSIONS_DOC.read_text())
        self.assertEqual([], measurer.compare(measured, declared))


class MissingExecutableTests(unittest.TestCase):
    """"Nothing measured" is an answer to `measure`, never to `--check`.

    The exe-absent guard sat before the --check branch and returned 0, so
    `--check` could report success having compared the document against nothing.
    The pinned executable is a gitignored proprietary artifact, so that is the
    ordinary state of any checkout without the game. Latent - nothing wires
    --check yet - and covered here so it stays fixed until something does.
    """

    def run_main(self, *argv):
        import contextlib
        import io
        with contextlib.redirect_stdout(io.StringIO()), \
             contextlib.redirect_stderr(io.StringIO()) as errors:
            status = measurer.main(list(argv))
        return status, errors.getvalue()

    def test_check_refuses_when_the_executable_is_absent(self):
        status, errors = self.run_main("--check", "--exe", "/nonexistent/x.exe")
        self.assertEqual(2, status)
        self.assertIn("verified NOTHING", errors)

    def test_plain_measurement_still_reports_nothing_and_succeeds(self):
        status, _ = self.run_main("--exe", "/nonexistent/x.exe")
        self.assertEqual(0, status)


if __name__ == "__main__":
    unittest.main()
