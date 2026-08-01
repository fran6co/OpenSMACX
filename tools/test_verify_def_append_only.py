#!/usr/bin/env python3

from pathlib import Path
import subprocess
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
        import sys
        argv = ["verify_def_append_only.py", "--def-file", str(self.path),
                "--repo-root", str(self.root), "--base", base]
        with mock.patch.object(sys, "argv", argv):
            return guard.main()

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


if __name__ == "__main__":
    unittest.main()
