#!/usr/bin/env python3

from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest import mock

import verify_def_append_only as guard


DEF = ('LIBRARY OpenSMACX\nEXPORTS\n'
       '    "?alpha@@YAXH@Z" = _Z5alphai\n'
       '    "?beta@@YAXH@Z" = _Z4betai\n')


class ParseTests(unittest.TestCase):
    def test_export_names_are_extracted(self):
        self.assertEqual(guard.exported_names(DEF),
                         {"?alpha@@YAXH@Z", "?beta@@YAXH@Z"})

    def test_a_file_with_no_exports_parses_empty(self):
        self.assertEqual(guard.exported_names("LIBRARY X\nEXPORTS\n"), set())


class RepositoryTests(unittest.TestCase):
    """A real git repo, because the check is a comparison against history."""

    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        (self.root / "src").mkdir()
        self.path = self.root / "src" / "OpenSMACX.def"
        self.git("init", "-q", ".")
        self.git("config", "user.email", "t@t")
        self.git("config", "user.name", "t")
        self.path.write_text(DEF, encoding="utf-8")
        self.git("add", "-A")
        self.git("commit", "-qm", "base")

    def git(self, *argv):
        subprocess.run(["git", "-C", str(self.root), *argv], check=True,
                       capture_output=True)

    def run_check(self, base="HEAD"):
        argv = ["verify_def_append_only.py", "--def-file", str(self.path),
                "--repo-root", str(self.root), "--base", base]
        with mock.patch.object(sys, "argv", argv):
            return guard.main()

    def run_over_history(self, floor=1):
        """No --base: the mode the gate actually runs."""
        argv = ["verify_def_append_only.py", "--def-file", str(self.path),
                "--repo-root", str(self.root), "--revision-floor", str(floor)]
        with mock.patch.object(sys, "argv", argv):
            return guard.main()

    def run_as_the_gate_does(self):
        """Not one flag the gate does not pass: no --base, no --revision-floor.

        `add_test(NAME def-append-only)` in CMakeLists passes NO arguments at
        all, so --def-file and --repo-root are the only concession this makes to
        running in a temporary repository. Every other test here pins a
        behaviour; this one pins the DEPLOYMENT, which is where the defect was.
        """
        argv = ["verify_def_append_only.py", "--def-file", str(self.path),
                "--repo-root", str(self.root)]
        with mock.patch.object(sys, "argv", argv):
            return guard.main()

    def commit(self, text, message="change"):
        self.path.write_text(text, encoding="utf-8")
        self.git("add", "-A")
        self.git("commit", "-qm", message)

    def porcelain(self):
        done = subprocess.run(["git", "-C", str(self.root), "status",
                               "--porcelain"], capture_output=True, text=True)
        return done.stdout.strip()

    def test_unchanged_passes(self):
        self.assertEqual(self.run_check(), 0)

    def test_adding_an_export_passes(self):
        self.path.write_text(DEF + '    "?gamma@@YAXH@Z" = _Z5gammai\n',
                             encoding="utf-8")
        self.assertEqual(self.run_check(), 0)

    def test_removing_an_export_fails(self):
        self.path.write_text('LIBRARY OpenSMACX\nEXPORTS\n'
                             '    "?alpha@@YAXH@Z" = _Z5alphai\n',
                             encoding="utf-8")
        self.assertEqual(self.run_check(), 1)

    def test_a_rename_that_keeps_the_count_still_fails(self):
        # The real incident: 312b5cf removed one alias and added another, so
        # the export COUNT was unchanged at 467 and only a set difference sees
        # it. A count check would have passed that commit.
        self.path.write_text('LIBRARY OpenSMACX\nEXPORTS\n'
                             '    "?alpha@@YAXH@Z" = _Z5alphai\n'
                             '    "?renamed@@YAXH@Z" = _Z4betai\n',
                             encoding="utf-8")
        self.assertEqual(self.run_check(), 1)

    def test_aliasing_both_spellings_passes(self):
        # The correct way to fix a decoration: additive.
        self.path.write_text(DEF + '    "?beta_fixed@@YAXH@Z" = _Z4betai\n',
                             encoding="utf-8")
        self.assertEqual(self.run_check(), 0)

    def test_an_unparseable_file_fails_rather_than_comparing_nothing(self):
        self.path.write_text("LIBRARY OpenSMACX\nEXPORTS\n", encoding="utf-8")
        self.assertEqual(self.run_check(), 1)

    def test_an_unreadable_base_fails_rather_than_reporting_clean(self):
        self.assertEqual(self.run_check(base="no-such-revision"), 1)

    # ---------------------------------------------------------------- history
    #
    # THE MODE THE GATE ACTUALLY RUNS. `add_test(NAME def-append-only)` passes
    # no --base, and every test above passes one, so for this check's whole life
    # the tested behaviour and the deployed behaviour were different behaviours.

    def test_a_committed_removal_is_caught(self):
        # The decisive case, and the one --base HEAD structurally cannot see:
        # the tree is CLEAN, so HEAD and the working copy agree exactly.
        self.commit('LIBRARY OpenSMACX\nEXPORTS\n'
                    '    "?alpha@@YAXH@Z" = _Z5alphai\n', "drop beta")
        self.assertEqual("", self.porcelain(),
                         "the control is only meaningful on a clean tree")
        self.assertEqual(1, self.run_over_history())

    def test_the_same_committed_removal_passes_against_head(self):
        # The contrast that makes the fix worth having, asserted rather than
        # described. If this ever starts failing, the two modes have converged
        # and the history walk is no longer buying anything.
        self.commit('LIBRARY OpenSMACX\nEXPORTS\n'
                    '    "?alpha@@YAXH@Z" = _Z5alphai\n', "drop beta")
        self.assertEqual(0, self.run_check(base="HEAD"))

    def test_the_gate_invocation_catches_a_committed_removal(self):
        # The same removal reached through the gate's own argv - defaults for
        # --base AND --revision-floor - because a check can be right in the mode
        # its tests use and vacuous in the mode CMake runs. The floor is 10, so
        # the history has to be deep enough to clear it.
        for revision in range(2, 14):
            self.commit(DEF + f"; revision {revision}\n", f"revision {revision}")
        self.commit('LIBRARY OpenSMACX\nEXPORTS\n'
                    '    "?alpha@@YAXH@Z" = _Z5alphai\n', "drop beta")
        self.assertEqual("", self.porcelain())
        self.assertEqual(1, self.run_as_the_gate_does())

    def test_the_gate_invocation_passes_on_an_append_only_history(self):
        # The positive control for the test above: same depth, same flags, and
        # the only difference is that nothing was dropped. Without this, a tool
        # that refused unconditionally would look correct.
        for revision in range(2, 14):
            self.commit(DEF + f"; revision {revision}\n", f"revision {revision}")
        self.commit(DEF + '    "?gamma@@YAXH@Z" = _Z5gammai\n', "add gamma")
        self.assertEqual(0, self.run_as_the_gate_does())

    def test_an_export_added_and_later_dropped_is_caught(self):
        # A union over history, not "the set at the oldest revision". An alias
        # that appears in the middle and disappears later is invisible to any
        # single-revision comparison.
        self.commit(DEF + '    "?middle@@YAXH@Z" = _Z6middlei\n', "add middle")
        self.commit(DEF, "drop middle")
        self.assertEqual(1, self.run_over_history())

    def test_an_append_only_history_passes(self):
        self.commit(DEF + '    "?gamma@@YAXH@Z" = _Z5gammai\n', "add gamma")
        self.assertEqual(0, self.run_over_history())

    def test_too_little_history_refuses_rather_than_reporting_clean(self):
        # A path git does not know finds no revisions, so every export looks
        # new and nothing can have been removed - clean, and vacuous.
        self.assertEqual(1, self.run_over_history(floor=50))

    def test_the_revision_walk_finds_the_commits(self):
        self.commit(DEF + '    "?gamma@@YAXH@Z" = _Z5gammai\n', "add gamma")
        self.assertEqual(
            2, len(guard.revisions_touching(self.path, self.root)))

    def test_the_union_reports_revisions_it_could_not_read(self):
        # An incomplete union is a gap a removal can hide in, so it must be
        # refused rather than silently narrowed.
        names, unreadable = guard.ever_exported(
            self.path, self.root, ["no-such-revision"])
        self.assertEqual(set(), names)
        self.assertEqual(["no-such-revision"], unreadable)

    def test_a_revision_from_before_the_file_existed_is_unreadable(self):
        # `<spec> missing` is also what git says when the revision resolves but
        # the path is absent from it - the same gap, a different cause.
        self.git("rm", "-q", str(self.path))
        self.git("commit", "-qm", "remove the file entirely")
        empty = subprocess.run(["git", "-C", str(self.root), "rev-parse",
                                "HEAD"], capture_output=True, text=True)
        self.path.parent.mkdir(exist_ok=True)   # git rm took the empty dir too
        self.path.write_text(DEF, encoding="utf-8")
        names, unreadable = guard.ever_exported(
            self.path, self.root, [empty.stdout.strip()])
        self.assertEqual(set(), names)
        self.assertEqual(1, len(unreadable))

    def test_a_gap_in_the_history_refuses_rather_than_being_skipped(self):
        # A revision the walk cannot read is a hole the union does not cover,
        # and main() must refuse on it - not narrow the union and report clean.
        # Deleting and restoring the file produces exactly such a revision.
        self.git("rm", "-q", str(self.path))
        self.git("commit", "-qm", "delete the def entirely")
        self.path.parent.mkdir(exist_ok=True)  # git rm took the empty dir too
        self.commit(DEF, "restore the def")
        self.assertEqual(1, self.run_over_history())

    # ------------------------------------------------------------------ speed
    #
    # The walk reads the history in two `git cat-file` batches instead of one
    # `git show` per revision. A speedup has to agree with what it replaced, or
    # it is just a different answer arriving sooner.

    def test_the_batched_union_agrees_with_reading_revisions_one_by_one(self):
        self.commit(DEF + '    "?middle@@YAXH@Z" = _Z6middlei\n', "add middle")
        self.commit(DEF + '    "?gamma@@YAXH@Z" = _Z5gammai\n', "swap middle")
        self.commit(DEF, "drop both")
        revisions = guard.revisions_touching(self.path, self.root)
        self.assertEqual(4, len(revisions))

        one_by_one = set()
        for revision in revisions:
            one_by_one |= guard.names_at(revision, self.path, self.root)

        batched, unreadable = guard.ever_exported(self.path, self.root,
                                                  revisions)
        self.assertEqual([], unreadable)
        self.assertEqual(one_by_one, batched)
        self.assertIn("?middle@@YAXH@Z", batched)

    def test_the_whole_history_costs_two_git_processes(self):
        # Flat in the length of the history, and identical blobs read once -
        # both claims the docstring makes, observed rather than asserted in
        # prose. A gate nobody waits for is a gate somebody deletes.
        self.commit(DEF + '    "?gamma@@YAXH@Z" = _Z5gammai\n', "add gamma")
        self.commit(DEF, "revert to base")   # the base blob again, verbatim
        self.commit(DEF + "; a comment changes nothing\n", "comment")
        revisions = guard.revisions_touching(self.path, self.root)
        self.assertEqual(4, len(revisions))

        calls, real_run = [], subprocess.run

        def spy(command, **kwargs):
            calls.append((command, kwargs.get("input")))
            return real_run(command, **kwargs)

        with mock.patch.object(guard.subprocess, "run", spy):
            names, unreadable = guard.ever_exported(self.path, self.root,
                                                    revisions)
        self.assertEqual([], unreadable)
        self.assertEqual({"?alpha@@YAXH@Z", "?beta@@YAXH@Z",
                          "?gamma@@YAXH@Z"}, names)
        self.assertEqual(2, len(calls),
                         "one git process per revision is what this replaced")
        self.assertEqual(3, len(calls[1][1].decode().split()),
                         "two revisions share a blob; it is read once")

    # ------------------------------------------------------- outside the repo
    #
    # A def file that is not inside --repo-root has no history to compare
    # against. That must be a refusal, not a ValueError out of relative_to: a
    # traceback is what verify_checks_can_fail.py counts as NOT a refusal.

    def outside_the_repo(self):
        """A def file in its own temporary directory, never a bare /tmp name.

        Two lanes run this suite at once here, and a fixed path under /tmp is a
        collision between them - one lane's cleanup deleting the other's file.
        """
        elsewhere = tempfile.TemporaryDirectory()
        self.addCleanup(elsewhere.cleanup)
        path = Path(elsewhere.name) / "OpenSMACX.def"
        path.write_text(DEF, encoding="utf-8")
        return path

    def test_a_def_outside_the_repo_refuses_over_history(self):
        argv = ["verify_def_append_only.py", "--def-file",
                str(self.outside_the_repo()), "--repo-root", str(self.root),
                "--revision-floor", "1"]
        with mock.patch.object(sys, "argv", argv):
            self.assertEqual(1, guard.main())

    def test_a_def_outside_the_repo_refuses_against_a_base(self):
        argv = ["verify_def_append_only.py", "--def-file",
                str(self.outside_the_repo()), "--repo-root", str(self.root),
                "--base", "HEAD"]
        with mock.patch.object(sys, "argv", argv):
            self.assertEqual(1, guard.main())


if __name__ == "__main__":
    unittest.main()
