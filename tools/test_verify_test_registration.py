#!/usr/bin/env python3
"""The check that makes the self-registering gameplay suite safe, itself untested.

Self-registration removed the one merge conflict where a careless resolution is
SILENT - drop a line from the central call list and the test stops running while
everything remaining still passes. It moved the failure rather than removing it:
now the way to lose a test is to write it and forget the GAMEPLAY_CASE line, and
nothing can see that except this check. The compiler is happy, every assertion
passes, and the evaluated-assertion counter merely reads lower.

So a bug here silently un-guards the thing self-registration was done for.
"""
import contextlib
import io
import sys
import tempfile
import unittest
from pathlib import Path

import verify_test_registration as registration

PREAMBLE = "#include <cstdio>\n\nnamespace {\n\n"


def case(name, registered=True):
    body = f"void {name}() {{\n    expect(true);\n}}\n"
    return body + (f"GAMEPLAY_CASE({name});\n\n" if registered else "\n")


def suite(cases, main_body="    return failures();\n"):
    text = PREAMBLE + "".join(cases) + "}  // namespace\n\nint main()\n{\n"
    return text + main_body + "}\n"


class AuditTests(unittest.TestCase):
    def test_a_registered_case_is_seen_as_both_defined_and_registered(self):
        defined, registered, direct = registration.audit_text(
            suite([case("test_alpha")]))
        self.assertEqual(["test_alpha"], defined)
        self.assertEqual({"test_alpha"}, registered)
        self.assertEqual([], direct)

    def test_a_defined_but_unregistered_case_is_detected(self):
        defined, registered, _ = registration.audit_text(
            suite([case("test_alpha", registered=False)]))
        self.assertEqual(["test_alpha"], defined)
        self.assertEqual(set(), registered)

    def test_a_direct_call_from_main_is_detected(self):
        # The shape self-registration removed. If it comes back, the registry
        # has been bypassed and the silent-merge-loss returns with it.
        _, _, direct = registration.audit_text(
            suite([case("test_alpha")], "    test_alpha();\n"))
        self.assertEqual(["test_alpha"], direct)

    def test_a_call_before_main_is_not_a_direct_call(self):
        # Calls inside other test bodies are ordinary code, not the call list.
        text = (PREAMBLE + "void test_beta() {\n    test_alpha();\n}\n"
                + "GAMEPLAY_CASE(test_beta);\n}  // namespace\n\n"
                + "int main()\n{\n    return failures();\n}\n")
        _, _, direct = registration.audit_text(text)
        self.assertEqual([], direct)


class MainTests(unittest.TestCase):
    def run_main(self, text, floor=1):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "suite.cpp"
            path.write_text(text, encoding="utf-8")
            argv = sys.argv
            sys.argv = ["verify_test_registration.py", "--suite", str(path),
                        "--floor", str(floor)]
            try:
                with contextlib.redirect_stdout(io.StringIO()) as out, \
                     contextlib.redirect_stderr(io.StringIO()) as err:
                    status = registration.main()
                return status, out.getvalue() + err.getvalue()
            finally:
                sys.argv = argv

    def test_a_fully_registered_suite_passes(self):
        self.assertEqual(0, self.run_main(suite([case("test_a")]))[0])

    def test_an_unregistered_case_fails_and_names_it(self):
        status, output = self.run_main(
            suite([case("test_a"), case("test_b", registered=False)]))
        self.assertEqual(1, status)
        self.assertIn("test_b", output)
        self.assertIn("DO NOT RUN", output)

    def test_a_reinstated_direct_call_fails(self):
        status, output = self.run_main(
            suite([case("test_a")], "    test_a();\n"))
        self.assertEqual(1, status)
        self.assertIn("called directly", output)

    def test_too_few_cases_refuses_rather_than_reporting_clean(self):
        # A suite this tool failed to parse looks identical to a tiny one, and
        # "0 cases, all registered" is the vacuous pass the floor exists for.
        status, output = self.run_main(suite([case("test_a")]), floor=50)
        self.assertEqual(1, status)
        self.assertIn("below the floor", output)

    def test_an_absent_suite_verifies_nothing_and_says_so(self):
        argv = sys.argv
        sys.argv = ["verify_test_registration.py", "--suite", "/nonexistent.cpp"]
        try:
            with contextlib.redirect_stderr(io.StringIO()) as err:
                status = registration.main()
        finally:
            sys.argv = argv
        self.assertEqual(1, status)
        self.assertIn("verified NOTHING", err.getvalue())


class CommittedSuiteTests(unittest.TestCase):
    def test_the_real_suite_is_fully_registered(self):
        if not registration.DEFAULT_SUITE.is_file():
            self.skipTest("the gameplay suite is absent")
        defined, registered, direct = registration.audit(
            registration.DEFAULT_SUITE)
        self.assertGreaterEqual(len(defined), 50)
        self.assertEqual([], [n for n in defined if n not in registered])
        self.assertEqual([], direct)


if __name__ == "__main__":
    unittest.main()
