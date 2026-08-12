#!/usr/bin/env python3
"""Unit tests for the single-function verifier.

The compile needs VC6 and is exercised by running the tool. What is tested here
is the contract an agent loop depends on: the exit code, and that a function
with nothing recovered says so instead of reporting a verdict about nothing.
"""

import io
import shutil
import tempfile
import unittest
from contextlib import redirect_stdout, redirect_stderr
from pathlib import Path
from unittest import mock

import verify_recovered_function as verifier


class ExitCodeTest(unittest.TestCase):
    """Exit 0 means BYTE_EXACT and nothing else - the same contract
    `byte_match.py` uses, so this can be a loop condition."""

    def setUp(self):
        self.verify = verifier.writeback.verify
        self.body = verifier.committed_body
        verifier.committed_body = lambda address: ("body", "src/x.cpp:1")

    def tearDown(self):
        verifier.writeback.verify = self.verify
        verifier.committed_body = self.body

    def run_main(self, tier, extra=None):
        verifier.writeback.verify = lambda a, b: {"tier": tier, **(extra or {})}
        out = io.StringIO()
        with redirect_stdout(out):
            status = verifier.main(["0x00401000"])
        return status, out.getvalue()

    def test_byte_exact_exits_zero(self):
        self.assertEqual(self.run_main("BYTE_EXACT")[0], 0)

    def test_every_other_tier_exits_nonzero(self):
        for tier in ("MISMATCH", "MNEMONIC_ONLY", "NO_COMPILE", "REFUSED",
                     "SHAPE_EXACT", "SHARED_TAIL"):
            self.assertEqual(self.run_main(tier)[0], 1, tier)

    def test_the_divergence_is_printed(self):
        _, output = self.run_main(
            "MISMATCH", {"note": "#0: original 'xor' vs rebuilt 'ret 8'"})
        self.assertIn("original 'xor'", output)

    def test_a_refusal_reason_is_printed(self):
        _, output = self.run_main(
            "NO_COMPILE", {"refusal_reason": "error C2065: 'Win'"})
        self.assertIn("C2065", output)

    def test_empty_verdict_fields_are_not_printed(self):
        _, output = self.run_main("BYTE_EXACT", {"note": ""})
        self.assertNotIn("divergence", output)


class NothingRecoveredTest(unittest.TestCase):
    """A function with no committed body must not be scored.

    Reporting a verdict for one would be a statement about a body that does not
    exist; `byte_match_fanout.py --prepare` is the tool for those, and saying so
    is more use than a tier.
    """

    def setUp(self):
        self.body = verifier.committed_body

    def tearDown(self):
        verifier.committed_body = self.body

    def test_it_skips_rather_than_scoring(self):
        verifier.committed_body = lambda address: (
            None, "0x00401000 has no source_locations")
        out = io.StringIO()
        with redirect_stdout(out):
            status = verifier.main(["0x00401000"])
        self.assertEqual(status, 0)
        self.assertIn("SKIP", out.getvalue())


class AddressParsingTest(unittest.TestCase):
    def test_a_non_hex_address_is_a_usage_error(self):
        out = io.StringIO()
        with redirect_stdout(out):
            self.assertEqual(verifier.main(["not-an-address"]), 2)


class RankDirectoryTest(unittest.TestCase):
    """`--dir` arrived from `try_variants.py`, which had no tests at all.

    The contract that matters is the same one the single-body mode has: exit 0
    means BYTE_EXACT and nothing else, so the command works as a loop
    condition. A ranking that got CLOSER without arriving is still a failure,
    and that is the case a "did it improve" reading would get wrong.
    """

    def setUp(self):
        self.score_all = verifier.score_all
        self.work = tempfile.mkdtemp()

    def tearDown(self):
        verifier.score_all = self.score_all
        shutil.rmtree(self.work, ignore_errors=True)

    def write(self, *names):
        for name in names:
            Path(self.work, f"{name}.cpp").write_text("void f() {}\n")

    def run_dir(self, ranked):
        verifier.score_all = lambda address, bodies: ranked
        out = io.StringIO()
        with redirect_stdout(out):
            status = verifier.main(["0x00401000", "--dir", self.work])
        return status, out.getvalue()

    def test_a_byte_exact_winner_exits_zero(self):
        self.write("a")
        status, output = self.run_dir([("a", {"tier": "BYTE_EXACT"})])
        self.assertEqual(status, 0)
        self.assertIn("a is BYTE_EXACT", output)

    def test_a_near_miss_winner_still_fails(self):
        self.write("a", "b")
        status, output = self.run_dir([("a", {"tier": "MNEMONIC_ONLY"}),
                                       ("b", {"tier": "MISMATCH"})])
        self.assertEqual(status, 1)
        self.assertIn("none reached BYTE_EXACT", output)

    def test_every_candidate_is_reported_not_just_the_winner(self):
        # The ranking IS the output: an agent picks its next edit from the
        # losers, so printing only the winner would throw away the answer.
        self.write("a", "b")
        _, output = self.run_dir([("a", {"tier": "MISMATCH", "note": "#3 jl"}),
                                  ("b", {"tier": "NO_COMPILE"})])
        self.assertIn("a", output)
        self.assertIn("b", output)
        self.assertIn("#3 jl", output)

    def test_an_empty_directory_is_a_usage_error(self):
        # Not "nothing reached BYTE_EXACT": there was nothing to score, and a
        # run that measures nothing must not report a verdict.
        err = io.StringIO()
        with redirect_stderr(err):
            self.assertEqual(
                verifier.main(["0x00401000", "--dir", self.work]), 2)

    def test_dir_and_body_together_are_a_usage_error(self):
        self.write("a")
        err = io.StringIO()
        with redirect_stderr(err):
            status = verifier.main(
                ["0x00401000", "--dir", self.work, "--body", "-"])
        self.assertEqual(status, 2)


class ScoreAllOrderingTest(unittest.TestCase):
    """Ranking is by `byte_match.TIER_ORDER`, then how close, then name.

    Ordering on tier alone leaves ties in whatever order the filenames landed
    in, which is exactly the case an agent is using this to break. Since the
    ranking moved to one batched compile per flag set, this drives that path
    rather than the retired per-candidate one.
    """

    def setUp(self):
        self.saved = {
            "load_functions": verifier.emit.load_functions,
            "load_callees": verifier.emit.load_callees,
            "load_derived": verifier.emit.load_derived,
            "build_unit": verifier.writeback.build_unit,
            "load_rows": verifier.byte_match.load_rows,
            "shared": verifier.byte_match.shared_span_index,
            "span": verifier.byte_match.original_span_bytes,
            "mask": verifier.byte_match.original_relocation_mask,
            "batch": verifier.byte_match.compile_batch,
            "objcode": verifier.byte_match.object_code,
            "compare": verifier.byte_match.compare,
            "opsim": verifier._operand_similarity,
        }

    def tearDown(self):
        verifier.emit.load_functions = self.saved["load_functions"]
        verifier.emit.load_callees = self.saved["load_callees"]
        verifier.emit.load_derived = self.saved["load_derived"]
        verifier.writeback.build_unit = self.saved["build_unit"]
        verifier.byte_match.load_rows = self.saved["load_rows"]
        verifier.byte_match.shared_span_index = self.saved["shared"]
        verifier.byte_match.original_span_bytes = self.saved["span"]
        verifier.byte_match.original_relocation_mask = self.saved["mask"]
        verifier.byte_match.compile_batch = self.saved["batch"]
        verifier.byte_match.object_code = self.saved["objcode"]
        verifier.byte_match.compare = self.saved["compare"]
        verifier._operand_similarity = self.saved["opsim"]

    def test_best_tier_first_then_closest(self):
        verdicts = {
            "far":   {"tier": "MISMATCH", "mnemonic_similarity": 0.10},
            "near":  {"tier": "MISMATCH", "mnemonic_similarity": 0.90},
            "exact": {"tier": "SHAPE_EXACT"},
            "broke": None,                       # did not compile
        }
        verifier.emit.load_functions = lambda: {}
        verifier.emit.load_callees = lambda: {}
        verifier.emit.load_derived = lambda: {}
        verifier.writeback.build_unit = lambda *a, **k: "unit"
        verifier.byte_match.load_rows = lambda: {
            0x00401000: {"address": "0x00401000", "end_address": "0x00401010"}}
        verifier.byte_match.shared_span_index = lambda rows: {}
        verifier.byte_match.original_span_bytes = lambda pe, lo, hi: b"\x90" * 16
        verifier.byte_match.original_relocation_mask = lambda pe, lo, hi: b""
        verifier.byte_match.compile_batch = lambda units, work, flags: (
            {name: (None if verdicts[name] is None else b"obj")
             for name in units}, {"broke": "C2065"})
        verifier.byte_match.object_code = lambda data: (b"code", b"")
        verifier.byte_match.compare = lambda *a, **k: None   # replaced below
        verifier._operand_similarity = lambda *a, **k: 0.0

        # `compare` is keyed off which unit is being scored, which the batch
        # loop does not pass - so it is resolved by call order instead.
        order = iter([verdicts[n] for n in sorted(verdicts) if verdicts[n]])
        verifier.byte_match.compare = lambda *a, **k: dict(next(order))

        with mock.patch("pefile.PE", lambda *a, **k: object()), \
             mock.patch.object(verifier.byte_match, "FLAG_SETS", ("/c",)), \
             mock.patch.object(verifier, "form_report", lambda body: ([], [])), \
             mock.patch.object(verifier.byte_match, "match_function",
                               lambda *a, **k: {"tier": "SHAPE_EXACT"}):
            ranked = verifier.score_all(
                0x00401000, {name: "body" for name in verdicts})

        tiers = {name: verdict.get("tier") for name, verdict in ranked}
        self.assertEqual(tiers["broke"], "NO_COMPILE")
        self.assertEqual(ranked[-1][0], "broke")
        self.assertEqual(len(ranked), 4)


if __name__ == "__main__":
    unittest.main()
