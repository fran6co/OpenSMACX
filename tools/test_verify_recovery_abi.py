#!/usr/bin/env python3
"""The prefetch must never be able to change a verdict, only the clock.

verify_recovery_abi.py went from 48.4 s to 7.7 s by running its objdump and nm
calls in a thread pool and serving the linear checks from a cache. That is a
speedup applied to a VERIFIER, so the interesting tests are not "is it faster"
but the two properties that make it safe:

  1. a command the plan missed is still executed, so under-planning is slow
     rather than wrong;
  2. a command that fails during speculative prefetch is not cached, so the real
     call re-runs it and raises where it always did.

Both are invisible in a passing gate - a broken property 1 shows up as a
correct-but-slower run, and a broken property 2 as a verdict decided by a
command the checks never asked for. Neither would be noticed without these.
"""
import argparse
import pathlib
import subprocess
import unittest
from unittest import mock

import verify_recovery_abi


def make_args(**overrides):
    values = {"objdump": "OBJDUMP", "nm": "NM", "object_dir": None}
    values.update(overrides)
    return argparse.Namespace(**values)


class PlanTest(unittest.TestCase):
    """The plan is scanned out of the tool's own source, so it cannot go stale."""

    def test_plans_the_variants_the_source_actually_names(self):
        args = make_args(autosound_object="/objects/autosound.cpp.obj")
        planned = verify_recovery_abi.plan_prefetch(args, [])
        self.assertIn(["OBJDUMP", "-f", "/objects/autosound.cpp.obj"], planned)
        self.assertIn(["NM", "--defined-only", "/objects/autosound.cpp.obj"],
                      planned)
        # The expensive one: a full disassembly, which is what the parallelism
        # exists for.
        self.assertIn(
            ["OBJDUMP", "-d", "-r", "-C", "/objects/autosound.cpp.obj"], planned)

    def test_plans_nothing_for_an_object_that_was_not_supplied(self):
        # Every check is guarded by `if args.X_object:`, so prefetching an
        # absent object would run commands the checks never would.
        planned = verify_recovery_abi.plan_prefetch(make_args(), [])
        self.assertEqual([], planned)

    def test_plans_the_sweep_questions_for_every_swept_object(self):
        planned = verify_recovery_abi.plan_prefetch(
            make_args(), ["/objects/a.obj", "/objects/b.obj"])
        for path in ("/objects/a.obj", "/objects/b.obj"):
            self.assertIn(["OBJDUMP", "-h", path], planned)
            self.assertIn(["NM", "-u", path], planned)

    def test_plans_each_command_once(self):
        # log_object is disassembled twice in the source, with and without -C.
        args = make_args(log_object="/objects/log.cpp.obj")
        planned = verify_recovery_abi.plan_prefetch(args, [])
        self.assertEqual(len(planned), len({tuple(c) for c in planned}))


class GuardTests(unittest.TestCase):
    """The two guards mutation showed nothing was exercising."""

    def test_the_object_floor_default_is_a_real_floor(self):
        # `--object-floor 0` would let an empty or wrong --object-dir sweep
        # nothing and report clean, which is the exact defect the sweep was
        # widened to fix: it once covered 35 of 137 objects. Mutating the
        # default from 100 to 0 survived every test.
        # NOT by building a parser here with default=100 and asserting it is
        # 100 - that tests argparse, and it is exactly what the first version of
        # this test did, written while writing tests to kill vacuous
        # assertions. The SHIPPED SOURCE has to say it, because that is what
        # the mutant changes.
        source = pathlib.Path(verify_recovery_abi.__file__).read_text(
            encoding="utf-8")
        self.assertIn('"--object-floor", type=int, default=100', source)

    def test_object_dir_is_never_prefetched_as_an_object(self):
        # The plan walks every argument whose name ends in "object" and must
        # exclude object_dir, which is a DIRECTORY. Inverting that test made the
        # plan prefetch the directory and nothing else, and no test noticed.
        args = make_args(object_dir="/objects",
                         autosound_object="/objects/autosound.cpp.obj")
        planned = verify_recovery_abi.plan_prefetch(args, [])
        for command in planned:
            self.assertNotIn("/objects", command[-1:],
                             f"the object directory was planned as a file: "
                             f"{command}")
        self.assertTrue(any(c[-1] == "/objects/autosound.cpp.obj"
                            for c in planned),
                        "the real object should still be planned")


class CacheTest(unittest.TestCase):
    def setUp(self):
        verify_recovery_abi._CACHE.clear()
        verify_recovery_abi._EXECUTED_ON_DEMAND = 0
        self.addCleanup(verify_recovery_abi._CACHE.clear)

    def test_a_prefetched_command_is_not_executed_again(self):
        with mock.patch.object(verify_recovery_abi, "_execute",
                               return_value="output") as execute:
            verify_recovery_abi.prefetch([["tool", "argument"]], workers=2)
            self.assertEqual(1, execute.call_count)
            self.assertEqual("output",
                             verify_recovery_abi.run(["tool", "argument"]))
            self.assertEqual(1, execute.call_count)
        self.assertEqual(0, verify_recovery_abi._EXECUTED_ON_DEMAND)

    def test_a_command_the_plan_missed_is_still_executed(self):
        """Property 1: under-planning costs time, never correctness."""
        with mock.patch.object(verify_recovery_abi, "_execute",
                               return_value="output") as execute:
            self.assertEqual("output",
                             verify_recovery_abi.run(["tool", "unplanned"]))
            self.assertEqual(1, execute.call_count)
        self.assertEqual(1, verify_recovery_abi._EXECUTED_ON_DEMAND)

    def test_a_failing_prefetch_does_not_decide_anything(self):
        """Property 2: the real call re-runs it and raises where it always did."""
        failure = subprocess.CalledProcessError(1, ["tool", "argument"])
        with mock.patch.object(verify_recovery_abi, "_execute",
                               side_effect=failure):
            verify_recovery_abi.prefetch([["tool", "argument"]], workers=2)
            self.assertEqual({}, verify_recovery_abi._CACHE)
            with self.assertRaises(subprocess.CalledProcessError):
                verify_recovery_abi.run(["tool", "argument"])

    def test_a_repeated_request_returns_identical_bytes(self):
        # The neutrality argument rests on this: the checks below the cache see
        # the same bytes they would have seen from a fresh subprocess.
        with mock.patch.object(verify_recovery_abi, "_execute",
                               side_effect=["first", "second"]):
            first = verify_recovery_abi.run(["tool", "argument"])
            second = verify_recovery_abi.run(["tool", "argument"])
        self.assertEqual(first, second)


if __name__ == "__main__":
    unittest.main()
