#!/usr/bin/env python3
"""The harness that proves checks can fail must not be fakeable itself.

Its own first run demonstrated the hazard: a damage case whose substitution
matched nothing produced input identical to the real file, the check passed, and
that was reported as the check being unable to fail. Both directions of that
mistake are covered here - damage that does not damage, and a check that exits 0.

The second demonstration is why the requirement is DERIVED and why most of this
file is about the derivation. The coverage requirement was policed against a
hand-written set while CMakeLists.txt registered nine more; and once it was
derived, fourteen registered checks were excused from it by a second
hand-written set, which nobody had ever tried to empty. So what is tested below
is the parser and the exemption contract, because those are the parts a future
edit can quietly empty while every case still prints `ok`.
"""
import sys
import tempfile
import unittest
from pathlib import Path

import verify_checks_can_fail as harness

REAL_CMAKE = (harness.REPO_ROOT / "CMakeLists.txt").read_text(encoding="utf-8")


class SubstituteTests(unittest.TestCase):
    def test_a_pattern_that_is_absent_is_refused(self):
        # The exact bug: `RESOURCE_LOCK wineprefix` against a file that says
        # `RESOURCE_LOCK "wineprefix"`.
        with self.assertRaises(harness.Skip):
            harness.substitute('RESOURCE_LOCK "wineprefix"',
                               "RESOURCE_LOCK wineprefix", "")

    def test_a_present_pattern_is_replaced(self):
        self.assertEqual("b", harness.substitute("a", "a", "b"))

    def test_the_refusal_says_it_would_prove_nothing(self):
        with self.assertRaises(harness.Skip) as raised:
            harness.substitute("hello", "absent", "x")
        self.assertIn("prove nothing", str(raised.exception))


class GateCheckDerivationTests(unittest.TestCase):
    """What must be covered is read out of CMakeLists.txt, not remembered."""

    def test_the_deeply_nested_blocks_are_found(self):
        # A previous attempt used `add_test\(NAME\s+(\w+)(.*?)\n\s*\)`, which
        # looks right, parsed a plausible total, and silently lost every block
        # whose body contains a line ending in `)`. The miss was undetectable
        # from the count: the two names it dropped were both in the hand-written
        # set it was replacing, so they could never appear as absent. Hence
        # assertions on NAMES.
        derived = harness.parse_gate_checks(REAL_CMAKE)
        self.assertEqual("verify_recovery_abi.py", derived.get("recovery-abi"))
        self.assertEqual("audit_export_signedness.py",
                         derived.get("export-signedness-audit"))

    def test_the_checks_the_hand_written_set_omitted_are_derived(self):
        # These sat outside the coverage requirement while this file's docstring
        # said nothing could. If the derivation regresses to a shape that misses
        # them, that is the regression to catch.
        derived = harness.parse_gate_checks(REAL_CMAKE)
        for name, tool in (("def-append-only", "verify_def_append_only.py"),
                           ("build-freshness", "verify_build_freshness.py"),
                           ("tool-reachability",
                            "verify_tool_reachability.py")):
            self.assertEqual(tool, derived.get(name),
                             f"{name} is not derived from CMakeLists.txt")

    def test_the_artifact_currency_checks_are_derived(self):
        # `<generator>.py --check` is a gate check like any other: it can report
        # a derived artifact current while comparing nothing.
        derived = harness.parse_gate_checks(REAL_CMAKE)
        for name in ("access-lower-bounds-current", "base-edges-current",
                     "agreed-class-sizes-current", "idb-members-current",
                     "hypothesis-layouts-current", "derived-prototypes-current",
                     "vtable-shim-header-current", "class-size-bounds-current",
                     "yitzi-sizes-current", "member-offsets-current"):
            self.assertIn(name, derived)

    def test_unit_test_wrappers_are_not_gate_checks(self):
        # `test_*.py` is a unit suite; whether CMake runs it is
        # verify_tool_test_registration's job. Counting one here would demand a
        # damage case for a positive control, forever. The predecessor asked
        # only whether the block mentioned `tools/`, so it did exactly that.
        derived = harness.parse_gate_checks(REAL_CMAKE)
        self.assertEqual([], [name for name, tool in derived.items()
                              if tool.startswith("test_")])

    def test_a_block_whose_only_tools_script_is_a_test_is_not_a_check(self):
        # The live file happens to register its unit suites through a helper.
        # This is the shape that would break it, stated directly, so the
        # property does not depend on CMakeLists.txt keeping that habit.
        self.assertEqual({}, harness.parse_gate_checks(
            'add_test(NAME alpha-tests\n'
            '    COMMAND "${PY}" "${DIR}/tools/test_verify_alpha.py")\n'))

    def test_a_test_with_no_tools_script_is_not_a_gate_check(self):
        self.assertEqual({}, harness.parse_gate_checks(
            "add_test(NAME recovery-leaf-tests COMMAND recovery-leaf-tests)\n"))

    def test_the_command_is_read_out_of_the_matching_add_test(self):
        # The scan balances parentheses to find where each block ends. Getting
        # that wrong attributes a tool to the previous test, which would satisfy
        # coverage for a check that has no case.
        text = ('add_test(NAME alpha\n'
                '    COMMAND "${PY}" "${DIR}/tools/verify_alpha.py")\n'
                'add_test(NAME beta\n'
                '    COMMAND "${PY}" "${DIR}/tools/verify_beta.py")\n')
        self.assertEqual({"alpha": "verify_alpha.py", "beta": "verify_beta.py"},
                         harness.parse_gate_checks(text))

    def test_an_unbalanced_paren_in_a_comment_does_not_run_the_scan_on(self):
        # CMakeLists.txt is full of English prose, and prose contains "(" alone.
        # Without comment stripping the alpha block never closes and beta's tool
        # is attributed to alpha - so beta looks uncovered and alpha looks
        # covered by a case that defends something else.
        text = ('add_test(NAME alpha\n'
                '    COMMAND "${PY}" "${DIR}/tools/verify_alpha.py")\n'
                '# a five-byte jump (E9 rel32 is what gets written\n'
                'add_test(NAME beta\n'
                '    COMMAND "${PY}" "${DIR}/tools/verify_beta.py")\n')
        self.assertEqual({"alpha": "verify_alpha.py", "beta": "verify_beta.py"},
                         harness.parse_gate_checks(text))

    def test_a_hash_inside_a_quoted_string_is_not_a_comment(self):
        # Stripping too much is the mirror defect, and the regex this replaced
        # had it: it would drop the COMMAND line and make a real gate check
        # invisible.
        text = ('add_test(NAME alpha\n'
                '    COMMAND "${PY}" "${DIR}/tools/verify_alpha.py"\n'
                '        --label "count # of rows")\n')
        self.assertEqual({"alpha": "verify_alpha.py"},
                         harness.parse_gate_checks(text))

    def test_the_helper_definition_itself_is_not_a_gate_check(self):
        # opensmacx_add_python_tool_test's body contains `add_test(NAME ${name}`.
        self.assertEqual({}, harness.parse_gate_checks(
            'function(f)\n  add_test(NAME ${name}\n'
            '      COMMAND "${PY}" "${DIR}/tools/verify_x.py")\nendfunction()\n'))

    # THE PAREN BALANCER, in both directions. Mutating the weights survived
    # every test above, because a block that runs too long still reports its
    # own first tool - the damage only shows when a block has no tool of its
    # own, or when a paren appears before the COMMAND.
    def test_a_block_with_no_tool_does_not_absorb_the_next_one(self):
        # If the scan overruns the closing paren, a test that launches a BINARY
        # is credited with the next test's script - so a real gate check looks
        # covered by a case defending something else entirely.
        text = ('add_test(NAME leaf-tests COMMAND recovery-leaf-tests (extra)\n'
                ')\n'
                'add_test(NAME alpha\n'
                '    COMMAND "${PY}" "${DIR}/tools/verify_alpha.py")\n')
        self.assertEqual({"alpha": "verify_alpha.py"},
                         harness.parse_gate_checks(text))

    def test_a_paren_before_the_command_does_not_end_the_block_early(self):
        # The mirror: a scan that stops at the first `)` never reaches COMMAND,
        # so a registered gate check vanishes from the requirement.
        text = ('add_test(NAME alpha\n'
                '    CONFIGURATIONS $<IF:(Debug),a,b>\n'
                '    COMMAND "${PY}" "${DIR}/tools/verify_alpha.py")\n')
        self.assertEqual({"alpha": "verify_alpha.py"},
                         harness.parse_gate_checks(text))

    def test_an_unterminated_block_does_not_crash_the_scan(self):
        # The scan walks to the end of the file looking for the closing paren.
        # One step further and it indexes past the end, so a half-written
        # CMakeLists.txt takes down the tool with an IndexError instead of
        # simply finding no tool - and a crash in this tool is a gate that
        # cannot report on any check at all.
        self.assertEqual({}, harness.parse_gate_checks(
            'add_test(NAME alpha\n    COMMAND "${PY}" "recovery-leaf-tests"\n'))

    def test_an_escaped_quote_does_not_leave_the_scanner_inside_a_string(self):
        # `line[index - 1]` decides whether a backslash-escaped quote toggles
        # the string state. Get it wrong and the scanner believes it is still
        # inside a string for the rest of the file, so every `#` stops being a
        # comment and commented-out registrations come back as real ones.
        text = ('add_test(NAME alpha\n'
                '    COMMAND "${PY}" "${DIR}/tools/verify_alpha.py" "a \\" b"\n'
                ')\n'
                '# add_test(NAME ghost COMMAND "${DIR}/tools/verify_ghost.py")\n')
        self.assertEqual({"alpha": "verify_alpha.py"},
                         harness.parse_gate_checks(text))


class LiveContractTests(unittest.TestCase):
    """The contract as it stands against the real repository."""

    def test_the_scan_refuses_rather_than_reporting_an_empty_sweep(self):
        # A parser that matches nothing must not read as "every check covered".
        # The floor sits in gate_checks rather than in main() because
        # verify_check_tests_observe.py takes its population from here too.
        empty = Path(tempfile.mkdtemp()) / "CMakeLists.txt"
        empty.write_text("project(x)\n")
        with self.assertRaises(SystemExit):
            harness.gate_checks(empty)

    def test_the_floor_is_not_slack(self):
        # The assertion above is satisfied by a floor of ZERO, which is the
        # whole defect it was written to prevent. A floor only binds if it sits
        # just under the real count; far below, the parser could lose most of
        # the gate and still clear it.
        derived = len(harness.parse_gate_checks(REAL_CMAKE))
        self.assertLessEqual(harness.GATE_CHECK_FLOOR, derived,
                             "the floor is above the real count, so this tool "
                             "cannot pass at all")
        self.assertGreaterEqual(harness.GATE_CHECK_FLOOR, derived - 2,
                                f"a floor of {harness.GATE_CHECK_FLOOR} against "
                                f"{derived} real checks is slack, not a floor")

    def test_every_case_names_a_check_cmake_registers(self):
        # A case defending a name nothing registers scores `ok` and covers no
        # gate check - coverage theatre that survives every other assertion.
        derived = harness.gate_checks()
        for check, description, _, _ in harness.CASES:
            self.assertIn(check, derived,
                          f"case {description!r} names unregistered {check}")

    def test_every_registered_check_has_a_case_or_a_recorded_exemption(self):
        derived = harness.gate_checks()
        declared = {check for check, _, _, _ in harness.CASES}
        self.assertEqual(set(), set(derived) - declared - set(harness.EXEMPT))

    def test_no_exemption_outlives_the_check_it_excuses(self):
        # An exemption for a check that no longer exists excuses nothing, and
        # it makes the backlog look larger than it is.
        self.assertEqual(set(), set(harness.EXEMPT) - set(harness.gate_checks()))

    def test_the_exemption_ratchet_is_tight(self):
        # Not `<=`. Slack in the ratchet is a free exemption that nobody has to
        # justify, and it makes "add a reason to EXEMPT" cheaper than writing
        # the damage case - which is the outcome the ratchet exists to prevent.
        self.assertEqual(len(harness.EXEMPT), harness.MAX_EXEMPT,
                         "the ratchet must equal the number of exemptions, so "
                         "adding one is a deliberate, visible act")

    def test_the_exemption_list_only_shrinks(self):
        # THE RATCHET, stated as history. It held 14 names; lower this only by
        # deleting one, never by adding.
        self.assertLessEqual(len(harness.EXEMPT), 1)

    def test_every_exemption_records_a_reason(self):
        for name, reason in harness.EXEMPT.items():
            self.assertGreater(len(reason.strip()), 40,
                               f"{name} is exempt without a stated reason")

    def test_every_case_declares_expected_refusal_text(self):
        # Without it a usage error scores as a refusal; see RefusalTests.
        for check, description, _, expected in harness.CASES:
            self.assertTrue(expected and expected.strip(),
                            f"{check}: {description} has no expected text")


class StaleArtifactFactoryTests(unittest.TestCase):
    """The undamaged control inside stale_derived_artifact().

    It is what turned a false `ok` into an honest SKIP on the factory's first
    run: a generator that derives part of its answer from `output_path.name`
    regenerates something else under any other basename, and is then stale for
    a reason that has nothing to do with the damage.
    """

    def test_an_absent_artifact_skips_rather_than_passing(self):
        case = harness.stale_derived_artifact("x.py", "--out", "docs/absent.csv")
        with self.assertRaises(harness.Skip):
            case(Path("/nonexistent-workspace"))

    def test_the_control_copy_keeps_the_basename(self):
        import inspect
        body = inspect.getsource(harness.stale_derived_artifact)
        self.assertIn('workspace / "control" / source.name', body)

    def test_a_control_that_already_fails_skips_rather_than_scoring_ok(self):
        # The property the control exists for: if the UNDAMAGED copy is already
        # refused, a refusal on the damaged one proves nothing about the damage.
        case = harness.stale_derived_artifact(
            "verify_checks_can_fail.py", "--no-such-flag", "CMakeLists.txt")
        with tempfile.TemporaryDirectory() as directory:
            with self.assertRaises(harness.Skip) as raised:
                case(Path(directory))
        self.assertIn("UNMODIFIED", str(raised.exception))


class ShadowRepoTests(unittest.TestCase):
    """The last-resort mechanism for a check with no redirectable input."""

    def test_the_shadow_root_is_not_the_real_repository(self):
        # `Path(__file__).resolve()` follows symlinks. A symlink farm would
        # resolve straight back to the real tree, and the case would then
        # measure - and could damage - the repository it was shielding.
        with tempfile.TemporaryDirectory() as directory:
            root = harness.shadow_repo(Path(directory), ("tools",))
            tool = root / "tools" / "verify_checks_can_fail.py"
            self.assertTrue(tool.is_file())
            self.assertEqual(root, tool.resolve().parent.parent)
            self.assertNotEqual(harness.REPO_ROOT, tool.resolve().parent.parent)

    def test_an_absent_tree_skips_rather_than_building_an_empty_shadow(self):
        with tempfile.TemporaryDirectory() as directory:
            with self.assertRaises(harness.Skip):
                harness.shadow_repo(Path(directory), ("no-such-tree",))


class RefusalTests(unittest.TestCase):
    """A non-zero exit is not evidence that a check refused anything.

    This was the audit's headline finding against this very file: argparse exits
    2 on an unrecognised flag, and most of the checks here are invoked with at
    least one flag no add_test passes. Renaming any of them would leave ctest
    green while the damage case became `ok (exit 2)` on a usage banner.
    """

    def test_exit_zero_is_not_a_refusal(self):
        real, why = harness.refusal_is_real(0, "all clean", "clean")
        self.assertFalse(real)
        self.assertIn("exited 0", why)

    def test_a_usage_banner_is_not_a_refusal(self):
        real, why = harness.refusal_is_real(
            2, "usage: verify_x.py [-h] [--src SRC]\nerror: unrecognized", "x")
        self.assertFalse(real)
        self.assertIn("failed to run", why)

    def test_a_traceback_is_not_a_refusal(self):
        real, _ = harness.refusal_is_real(
            1, "Traceback (most recent call last):\n  File ...", "x")
        self.assertFalse(real)

    def test_the_wrong_refusal_is_not_accepted(self):
        # Exiting non-zero for some unrelated reason is not proof that the
        # property under test was the one detected.
        real, why = harness.refusal_is_real(1, "some other complaint", "x")
        self.assertFalse(real)
        self.assertIn("does not contain", why)

    def test_a_real_refusal_is_accepted(self):
        real, why = harness.refusal_is_real(
            1, "load-time-addresses: a fixed game address is dereferenced at "
               "load time", "dereferenced at load time")
        self.assertTrue(real)
        self.assertEqual("", why)

    def test_skip_is_not_a_pass(self):
        # This asserted on main().__doc__ - which is None, so the assertion was
        # vacuous and stayed green under four of five mutations of the code it
        # named. The behaviour is covered for real in
        # MainLoopTests.test_a_skipped_case_does_not_satisfy_coverage; what is
        # left here is the type contract that case depends on.
        self.assertTrue(issubclass(harness.Skip, Exception))


class MainLoopTests(unittest.TestCase):
    """Drive main() itself. Nothing here did, and it showed.

    Mutation-testing this tool against its own suite killed 0 of 30 mutants:
    every test exercised a pure helper - substitute(), refusal_is_real(), the
    CASES shape - and none ran the loop that decides anything. So every
    perturbation of the scoring, the coverage contract and the exit status
    survived, in the tool that certifies every other check can fail.
    """

    def drive(self, cases, registered, exempt=None, max_exempt=8):
        import contextlib
        import io
        saved = (harness.CASES, harness.gate_checks, harness.EXEMPT,
                 harness.MAX_EXEMPT)
        harness.CASES = cases
        # The requirement is derived from CMake, so a synthetic case set needs a
        # synthetic registry too - otherwise every fake check reads as one CMake
        # does not register.
        harness.gate_checks = lambda *a, **k: dict(registered)
        harness.EXEMPT = exempt or {}
        harness.MAX_EXEMPT = max_exempt
        argv = sys.argv
        sys.argv = ["verify_checks_can_fail.py"]
        try:
            with contextlib.redirect_stdout(io.StringIO()) as out, \
                 contextlib.redirect_stderr(io.StringIO()) as err:
                status = harness.main()
            return status, out.getvalue() + err.getvalue()
        finally:
            (harness.CASES, harness.gate_checks, harness.EXEMPT,
             harness.MAX_EXEMPT) = saved
            sys.argv = argv

    @staticmethod
    def refusing(workspace):
        return [sys.executable, "-c",
                "import sys; sys.stderr.write('it refused\\n'); sys.exit(1)"]

    @staticmethod
    def passing(workspace):
        return [sys.executable, "-c", "pass"]

    @staticmethod
    def crashing(workspace):
        return [sys.executable, "-c",
                "import sys; sys.stderr.write('usage: x\\n'); sys.exit(2)"]

    def test_a_refusing_check_is_reported_and_exits_zero(self):
        status, output = self.drive(
            (("demo", "a real refusal", self.refusing, "it refused"),),
            {"demo": "verify_demo.py"})
        self.assertEqual(0, status)
        self.assertIn("1 damage case(s) refused", output)

    def test_a_check_that_passes_on_damage_fails_the_run(self):
        status, output = self.drive(
            (("demo", "no refusal", self.passing, "anything"),),
            {"demo": "verify_demo.py"})
        self.assertEqual(1, status)
        self.assertIn("exited 0", output)

    def test_a_crashing_check_does_not_count_as_a_refusal(self):
        # The audit's headline finding against this file: exit 2 from argparse
        # scored identically to the check doing its job.
        status, output = self.drive(
            (("demo", "a usage error", self.crashing, "it refused"),),
            {"demo": "verify_demo.py"})
        self.assertEqual(1, status)
        self.assertIn("failed to run", output)

    def test_a_registered_check_with_no_case_fails(self):
        # The coverage contract: a check ships with a proof it can fail.
        status, output = self.drive((), {"demo": "verify_demo.py"})
        self.assertEqual(1, status)
        self.assertIn("unproven", output)

    def test_a_skipped_case_does_not_satisfy_coverage(self):
        def skipping(workspace):
            raise harness.Skip("the artifact is absent")
        status, output = self.drive(
            (("demo", "skipped", skipping, "x"),), {"demo": "verify_demo.py"})
        self.assertEqual(1, status)
        self.assertIn("unproven", output)

    def test_an_exempt_check_needs_no_case(self):
        status, _ = self.drive((), {"demo": "verify_demo.py"},
                               exempt={"demo": "a recorded reason"})
        self.assertEqual(0, status)

    def test_an_exemption_for_an_unregistered_check_refuses(self):
        # An exemption that outlives its subject is a hole nobody sees, because
        # the name it excuses no longer appears anywhere.
        status, output = self.drive(
            (("demo", "a real refusal", self.refusing, "it refused"),),
            {"demo": "verify_demo.py"},
            exempt={"deleted-check": "a reason from a previous era"})
        self.assertEqual(1, status)
        self.assertIn("must not outlive its subject", output)

    def test_exceeding_the_exemption_ratchet_refuses(self):
        # Otherwise "add a reason to EXEMPT" is an easier way out than writing
        # the damage case, and the list grows instead of shrinking.
        status, output = self.drive(
            (), {"a": "verify_a.py", "b": "verify_b.py"},
            exempt={"a": "reason one", "b": "reason two"}, max_exempt=1)
        self.assertEqual(1, status)
        self.assertIn("may only shrink", output)

    def test_exemptions_exactly_at_the_ratchet_are_accepted(self):
        # THE BOUNDARY. The comparison survived mutation to its non-strict form
        # because every test above sat well clear of the edge.
        status, _ = self.drive((), {"a": "verify_a.py"},
                               exempt={"a": "a recorded reason"}, max_exempt=1)
        self.assertEqual(0, status)

    def test_the_reason_shown_for_an_unproven_check_is_its_own(self):
        # The lookup that pairs a skipped case with the check it was for. Under
        # `!=` it hands each check somebody else's excuse, which is how a reader
        # concludes the wrong artifact is missing.
        def skipping_alpha(workspace):
            raise harness.Skip("alpha's artifact is absent")

        def skipping_beta(workspace):
            raise harness.Skip("beta's artifact is absent")

        status, output = self.drive(
            (("alpha", "s", skipping_alpha, "x"),
             ("beta", "s", skipping_beta, "x")),
            {"alpha": "verify_alpha.py", "beta": "verify_beta.py"})
        self.assertEqual(1, status)
        self.assertIn("alpha  (verify_alpha.py)  - alpha's artifact", output)
        self.assertIn("beta  (verify_beta.py)  - beta's artifact", output)

    def test_a_case_naming_an_unregistered_check_refuses(self):
        # Coverage theatre: the case runs, scores `ok`, and defends nothing that
        # ctest executes.
        status, output = self.drive(
            (("ghost", "a real refusal", self.refusing, "it refused"),),
            {"demo": "verify_demo.py"}, exempt={"demo": "a recorded reason"})
        self.assertEqual(1, status)
        self.assertIn("does not register", output)


if __name__ == "__main__":
    unittest.main()
