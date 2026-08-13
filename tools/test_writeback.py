#!/usr/bin/env python3
"""Pin the splice, because a wrong one is invisible in a diff review.

The writeback edits `src/` from a line number in a catalogue. Every way it can
go wrong produces a file that still looks plausible: a doc comment swallowed
into the body, a `}` left orphaned below the replacement, a `Status: Complete`
annotation deleted so `recovery_state` silently regresses on the next
inventory. None of those fail a compile of the CENSUS unit, which is
scaffolding rather than the real file, so the BYTE_EXACT re-check in the tool
would pass and the tree would be broken anyway.

The line-shift tests matter for the same reason and are worse to debug: a
catalogued location that is off by three still extracts A body, just the wrong
one, and the census would then report a mismatch on a function whose source
never changed.

No Wine and no VC6 here. The verification step is patched out; what is pinned
is the editing, the shifting and the restore-on-refusal, which is the half that
runs everywhere and the half that mutates the tree.
"""

from __future__ import annotations

import csv
import json
import re
import sys
import textwrap
import unittest
from pathlib import Path
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))

import byte_match_census as census  # noqa: E402
import emit_translation_unit as emit  # noqa: E402
import writeback as tool  # noqa: E402

SOURCE = textwrap.dedent("""\
    #include "stdafx.h"

    /*
    Purpose: Set both coordinates.
    Original Offset: 00601B80
    Status: Complete
    */
    void BasePop::set_loc(int x, int y) {
        loc_a_ = x;
    }

    /*
    Purpose: A later function whose line number must move.
    Status: Complete
    */
    int BasePop::later() {
        return 1;
    }
    """)

# `set_loc`'s doc comment opens on line 3; `later`'s on line 12.
FIRST = 0x00601B80
SECOND = 0x00601C00

# The real tree, captured at import: the fixture patches `tool.MATCHED_DIR` at
# a temporary directory, and the store guard below is about the repository.
ROOT = Path(__file__).resolve().parent.parent
CMAKE = ROOT / "CMakeLists.txt"
# Spelled the way the build would have to spell it, derived rather than typed
# so moving the store cannot leave a guard watching an address nothing uses.
STORE = Path(tool.MATCHED_DIR)
STORE_PATH = STORE.relative_to(ROOT).as_posix()


def cmake_code(text=None):
    """CMakeLists.txt - or `text` - with its comments removed.

    The prose is not the build. The store guard used to substring-search the
    whole file, and it went red on 2026-08-12 when two comments - one on the
    test that was deleting `src/recovered/00401000.cpp`, one on the frontier
    join - mentioned the directory by name while the source list was
    untouched. A guard that fires on the documentation of the thing it
    protects can only be made green by not writing the explanation, so it has
    to read the code.

    A `#` inside a quoted string is not a comment; nothing here spans a quote
    across lines (the six odd-quote lines are all prose, and CMake's bracket
    comment `#[[` appears nowhere), so tracking quotes within the line is
    enough. `text` exists so the callers below can put damage through the
    same strip that reads the tree, rather than a paraphrase of it.
    """
    stripped = []
    for line in (CMAKE.read_text() if text is None else text).splitlines():
        cut, quoted = len(line), False
        for index, char in enumerate(line):
            if char == '"':
                quoted = not quoted
            elif char == "#" and not quoted:
                cut = index
                break
        stripped.append(line[:cut])
    return "\n".join(stripped)


def compiled_sources(code):
    """Every C/C++ path the build names, from any target or list."""
    return re.findall(r"[\w./${}-]+\.c(?:c|pp|xx)?\b", code)


class WritebackFixture(unittest.TestCase):
    """A tree with two catalogued bodies and the two catalogues that point at
    them. Shared by the splice tests and the store tests; it carries no test
    methods of its own so neither set runs twice.

    There were three. `docs/recovery/functions.csv` was deleted (185dd977) and
    the function catalogue is now derived from the `src/` annotations, so the
    splice has nothing to shift for it - `src/` IS the pointer. What remains
    are the two stores that keep line numbers as text of their own.
    """

    def setUp(self):
        self.root = Path(self.enterContext(
            __import__("tempfile").TemporaryDirectory()))
        (self.root / "src").mkdir()
        self.source = self.root / "src" / "basepop.cpp"
        self.source.write_text(SOURCE)

        recovery = self.root / "docs" / "recovery"
        recovery.mkdir(parents=True)
        self.ledger_csv = recovery / "byte-match.csv"
        # `src/other.cpp` is the negative control: a row in a DIFFERENT file at
        # the same line must not move. It used to sit in the functions CSV; it
        # moved here when that one went away, because losing it would leave the
        # shift able to renumber the whole catalogue with the tests green.
        self.write_csv(self.ledger_csv, "source_location",
                       [("0x00601B80", "src/basepop.cpp:3"),
                        ("0x00601C00", "src/basepop.cpp:12"),
                        ("0x00700000", "src/other.cpp:12")])
        (self.root / "mizuchi").mkdir()
        self.source_map = self.root / "mizuchi" / "source-map.json"
        # `indent=2` plus a trailing newline is what `emit_asm_dumps.py`
        # writes; matching it here is what makes the restore-on-refusal test
        # able to observe a byte-identical revert rather than a reformat.
        self.source_map.write_text(json.dumps([
            {"file": "src/basepop.cpp", "line": 3, "name": "set_loc"},
            {"file": "src/basepop.cpp", "line": 12, "name": "later"},
        ], indent=2) + "\n")

        self.functions = {
            FIRST: {"name": "?set_loc@BasePop@@QAEXHH@Z",
                    "source_locations": "src/basepop.cpp:3"},
            SECOND: {"name": "?later@BasePop@@QAEHXZ",
                     "source_locations": "src/basepop.cpp:12"},
        }
        self.enterContext(mock.patch.object(census, "REPO_ROOT", self.root))
        self.enterContext(mock.patch.object(tool, "LEDGER_CSV", self.ledger_csv))
        self.enterContext(mock.patch.object(tool, "SOURCE_MAP", self.source_map))
        self.enterContext(mock.patch.object(tool, "REPO_ROOT", self.root))
        self.enterContext(mock.patch.object(emit, "load_functions",
                                            lambda: self.functions))
        # EVERY subclass, not just the store's own: a splice that will not
        # verify now falls back to landing, so any test can reach the store.
        # Without this the fallback wrote into the real `src/recovered/`.
        self.matched = self.root / "src" / "recovered"
        self.enterContext(mock.patch.object(tool, "MATCHED_DIR", self.matched))

    @staticmethod
    def write_csv(path, field, rows):
        with path.open("w", newline="") as handle:
            writer = csv.DictWriter(handle, fieldnames=("address", field, "name"))
            writer.writeheader()
            for address, location in rows:
                writer.writerow({"address": address, field: location, "name": ""})

    @staticmethod
    def locations(path, field):
        with path.open(newline="") as handle:
            return {row["address"]: row[field] for row in csv.DictReader(handle)}

    def writeback(self, code, tier="BYTE_EXACT", target=hex(FIRST)):
        with mock.patch.object(tool, "verify", return_value={"tier": tier}):
            return tool.writeback(target, code)


class WritebackTest(WritebackFixture):
    def test_replaces_the_definition_and_keeps_the_doc_comment(self):
        result = self.writeback("void BasePop::set_loc(int x, int y) {\n"
                                "    loc_a_ = x;\n    loc_b_ = y;\n}\n")
        text = self.source.read_text()
        self.assertIn("Status: Complete", text)
        self.assertIn("Purpose: Set both coordinates.", text)
        self.assertIn("loc_b_ = y;", text)
        # The old body is gone rather than merely followed by the new one.
        self.assertEqual(text.count("void BasePop::set_loc"), 1)
        self.assertEqual(text.count("loc_a_ = x;"), 1)
        # And the untouched neighbour survives intact.
        self.assertIn("int BasePop::later() {", text)
        self.assertEqual(result["line_delta"], 1)
        self.assertTrue(result["verified"])

    def test_a_longer_body_shifts_only_later_locations_in_the_same_file(self):
        self.writeback("void BasePop::set_loc(int x, int y) {\n"
                       "    loc_a_ = x;\n    loc_b_ = y;\n}\n")
        ledger = self.locations(self.ledger_csv, "source_location")
        # The spliced row itself does not move; the one below it does; the one
        # in another file does not.
        self.assertEqual(ledger["0x00601B80"], "src/basepop.cpp:3")
        self.assertEqual(ledger["0x00601C00"], "src/basepop.cpp:13")
        self.assertEqual(ledger["0x00700000"], "src/other.cpp:12")
        entries = json.loads(self.source_map.read_text())
        self.assertEqual([e["line"] for e in entries], [3, 13])
        # The shifted line still opens the neighbour's doc comment.
        lines = self.source.read_text().splitlines()
        self.assertEqual(lines[12].strip(), "/*")

    def test_a_shorter_body_shifts_the_other_way(self):
        self.writeback("void BasePop::set_loc(int x, int y) { loc_a_ = x; }\n")
        ledger = self.locations(self.ledger_csv, "source_location")
        self.assertEqual(ledger["0x00601C00"], "src/basepop.cpp:10")
        self.assertEqual(json.loads(self.source_map.read_text())[1]["line"], 10)
        lines = self.source.read_text().splitlines()
        self.assertEqual(lines[9].strip(), "/*")

    def test_a_failed_verification_restores_every_file(self):
        before = (self.source.read_text(), self.ledger_csv.read_text(),
                  self.source_map.read_text())
        with self.assertRaises(tool.Refused) as caught:
            self.writeback("void BasePop::set_loc(int x, int y) {\n"
                           "    loc_a_ = x;\n    loc_b_ = y;\n}\n",
                           tier="MISMATCH")
        self.assertIn("MISMATCH", str(caught.exception))
        after = (self.source.read_text(), self.ledger_csv.read_text(),
                 self.source_map.read_text())
        self.assertEqual(before, after)

    def test_an_empty_submission_is_refused(self):
        with self.assertRaises(tool.Refused):
            self.writeback("   \n\n")

    def test_resolves_a_mangled_name_as_well_as_an_address(self):
        result = self.writeback("void BasePop::set_loc(int x, int y) {\n}\n",
                                target="?set_loc@BasePop@@QAEXHH@Z")
        self.assertEqual(result["address"], "0x00601B80")


class SpliceFallbackTest(WritebackFixture):
    """A body `src/` cannot hold is stored, not thrown away.

    The emitted unit declares the subject's class as an opaque shell, so an
    agent reaching several fields declares a shadow struct beside the
    function. `splice` carries only the definition into `src/`, that struct
    does not come with it, and the spliced file stops compiling. On the first
    real run that was 4 of 10 writebacks - every one a body already verified
    BYTE_EXACT on its own.
    """


    def splice_fails(self, code):
        # BYTE_EXACT on its own, NO_COMPILE once spliced - which is exactly
        # the shape of the failure being handled.
        verdicts = iter([{"tier": "NO_COMPILE", "note": "shadow struct is missing"},
                         {"tier": "BYTE_EXACT"}])
        with mock.patch.object(tool, "verify", side_effect=lambda *a, **k: next(verdicts)):
            return tool.writeback(hex(FIRST), code)

    def test_a_body_src_cannot_hold_lands_in_the_store(self):
        before = self.source.read_text()
        result = self.splice_fails("struct Shadow { int a; };\n"
                                   "void BasePop::set_loc(int x, int y) {\n"
                                   "    reinterpret_cast<Shadow *>(this)->a = x + y;\n}\n")

        self.assertEqual("src/recovered/00601b80.cpp", result["source_location"])
        self.assertTrue((self.matched / "00601b80.cpp").is_file())
        self.assertTrue(result["verified"])
        self.assertEqual(self.source.read_text(), before,
                         "the refused splice must have been reverted")

    def test_the_reason_the_splice_was_refused_is_carried(self):
        result = self.splice_fails("struct Shadow { int a; };\n"
                                   "void BasePop::set_loc(int x, int y) {}\n")
        self.assertFalse(result["spliced"])
        self.assertIn("NO_COMPILE", result["splice_refusal"])

    def test_a_splice_that_verifies_still_wins(self):
        result = self.writeback("void BasePop::set_loc(int x, int y) {\n"
                                "    loc_a_ = x;\n    loc_b_ = y;\n}\n")
        self.assertEqual("src/basepop.cpp:3", result["source_location"])
        self.assertFalse((self.matched / "00601b80.cpp").exists())


class ImproveTest(WritebackFixture):
    """`--improve` lands a body that is better but not yet exact.

    Refusing everything except BYTE_EXACT threw real work away. Two committed
    bodies were NO_COMPILE under this very recipe because `BOOL` and `LPSTR`
    are never forward-declared for a type used only on a local, and the
    one-word respelling that fixes them is not byte-exact - so the row kept
    reading as a mismatch for a reason that had nothing to do with its code.

    The ordering is `byte_match._better`, the same one the ledger ratchet and
    `verify_recovered_function.py --against-committed` already use.
    """

    BODY = "void BasePop::set_loc(int x, int y) {\n    loc_a_ = x;\n}\n"

    def run_improve(self, incumbent, candidate, improve=True):
        """(result or None). `splice` measures the incumbent first, then the
        spliced result; a refused splice then tries the store, which measures
        again and refuses too, so the last verdict repeats."""
        verdicts = iter([incumbent, candidate])
        def verdict(*_args, **_kwargs):
            return next(verdicts, candidate)
        with mock.patch.object(tool, "verify", side_effect=verdict):
            try:
                return tool.writeback(hex(FIRST), self.BODY, improve=improve)
            except tool.Refused:
                return None

    def test_a_strictly_better_body_lands(self):
        result = self.run_improve({"tier": "NO_COMPILE"}, {"tier": "MISMATCH"})
        self.assertEqual("src/basepop.cpp:3", result["source_location"])
        self.assertEqual("MISMATCH", result["tier"])
        self.assertEqual("NO_COMPILE", result["improved_from"])

    def test_it_is_not_reported_as_verified(self):
        # `verified` means BYTE_EXACT. An improvement is progress, not proof,
        # and the ledger must not read it as a match.
        result = self.run_improve({"tier": "NO_COMPILE"}, {"tier": "MISMATCH"})
        self.assertFalse(result["verified"])

    def test_a_tie_is_refused(self):
        before = self.source.read_text()
        self.assertIsNone(
            self.run_improve({"tier": "MISMATCH"}, {"tier": "MISMATCH"}))
        self.assertEqual(self.source.read_text(), before)

    def test_a_worse_body_is_refused(self):
        before = self.source.read_text()
        self.assertIsNone(
            self.run_improve({"tier": "MISMATCH"}, {"tier": "NO_COMPILE"}))
        self.assertEqual(self.source.read_text(), before)

    def test_without_the_flag_only_byte_exact_lands(self):
        before = self.source.read_text()
        self.assertIsNone(
            self.run_improve({"tier": "NO_COMPILE"}, {"tier": "MISMATCH"},
                             improve=False))
        self.assertEqual(self.source.read_text(), before)

    def test_byte_exact_still_lands_and_still_reads_as_verified(self):
        result = self.run_improve({"tier": "MISMATCH"}, {"tier": "BYTE_EXACT"})
        self.assertEqual("src/basepop.cpp:3", result["source_location"])
        self.assertTrue(result["verified"])


class MatchedStoreTest(WritebackFixture):
    """A match with no owner lands in the store instead of being lost.

    Every one of the 2,783 unrecovered rows - the entire population the agent
    loop works on - has no `source_locations`. Refusing them meant the ledger
    recorded BYTE_EXACT while the body stayed in gitignored `build/`, so a
    `git clean` undid the proving. These pin the other destination.
    """

    def setUp(self):
        super().setUp()
        self.functions[FIRST]["source_locations"] = ""
        self.functions[FIRST]["name"] = "sub_601b80"

    def test_a_body_with_no_owner_is_stored_and_the_owning_file_is_untouched(self):
        before = self.source.read_text()
        result = self.writeback("extern \"C\" int __cdecl sub_601b80()\n"
                                "{\n    return 0;\n}\n")

        stored = self.matched / "00601b80.cpp"
        self.assertTrue(stored.is_file())
        self.assertIn("return 0;", stored.read_text())
        self.assertEqual(result["source_location"],
                         "src/recovered/00601b80.cpp")
        self.assertTrue(result["verified"])
        self.assertEqual(self.source.read_text(), before,
                         "landing must not edit a file that owns something else")

    def test_the_stored_file_says_what_it_is(self):
        self.writeback("extern \"C\" int __cdecl sub_601b80() { return 0; }\n")
        stored = (self.matched / "00601b80.cpp").read_text()
        self.assertIn("0x00601B80", stored)
        self.assertIn("sub_601b80", stored)
        # It lives under src/ now, so "this is not compiled" has to be on the
        # file itself rather than inferable from the directory it sits in.
        self.assertIn("OPENSMACX_SOURCES", stored)

    def test_the_header_is_stripped_when_the_body_is_read_back(self):
        # `verify` is handed what is ON DISK, so the provenance comment must
        # not reach the compiler as part of the definition.
        self.writeback("extern \"C\" int __cdecl sub_601b80() { return 0; }\n")
        body = tool.read_matched_body(self.matched / "00601b80.cpp")
        self.assertTrue(body.startswith("extern"), body)
        self.assertNotIn("NOT product source", body)

    def test_a_body_opening_with_its_own_comment_survives_the_strip(self):
        self.writeback("// why this shape\nextern \"C\" int __cdecl "
                       "sub_601b80() { return 0; }\n")
        body = tool.read_matched_body(self.matched / "00601b80.cpp")
        self.assertIn("extern", body)

    def test_a_failed_verification_leaves_no_file_behind(self):
        with self.assertRaises(tool.Refused) as caught:
            self.writeback("extern \"C\" int __cdecl sub_601b80() "
                           "{ return 1; }\n", tier="MISMATCH")
        self.assertIn("MISMATCH", str(caught.exception))
        self.assertFalse((self.matched / "00601b80.cpp").exists(),
                         "a store holding a body that does not verify is "
                         "worse than an empty one")

    def test_a_failed_verification_restores_a_previous_body(self):
        self.writeback("extern \"C\" int __cdecl sub_601b80() { return 0; }\n")
        good = (self.matched / "00601b80.cpp").read_text()
        with self.assertRaises(tool.Refused):
            self.writeback("extern \"C\" int __cdecl sub_601b80() "
                           "{ return 1; }\n", tier="MISMATCH")
        self.assertEqual((self.matched / "00601b80.cpp").read_text(), good)

    def test_source_locations_is_not_pointed_at_the_store(self):
        # It means "placed in the tree as product source", and both
        # `export_recovery_inventory` and `audit_recovered_signatures` read it
        # that way. Pointing it here would make them describe an artefact.
        # The catalogue row is the `src/` annotation now, so the assertion is
        # that landing writes no annotation claiming a `src/` home: the row
        # stays empty and the ledger keeps pointing where it did.
        self.writeback("extern \"C\" int __cdecl sub_601b80() { return 0; }\n")
        self.assertEqual(self.functions[FIRST]["source_locations"], "")
        self.assertNotIn("ORIGINAL: 0x00601B80\n",
                         self.source.read_text())
        locations = self.locations(self.ledger_csv, "source_location")
        self.assertEqual(locations["0x00601B80"], "src/basepop.cpp:3")

    def test_an_empty_submission_is_still_refused(self):
        with self.assertRaises(tool.Refused):
            self.writeback("   \n\n")
        self.assertFalse(self.matched.exists())


class StoreIsNotCompiledTest(unittest.TestCase):
    """The guard behind every claim in the stored file's header.

    `build_unit` writes "NOT product source ... not in OPENSMACX_SOURCES and
    not compiled" into every stored body - 381 of them today. That is true only because
    CMakeLists names each compiled file by hand; the day a glob over `src/`
    is added, the store joins the build and every one of those headers starts
    lying. It reads the real tree rather than the fixture, so it is outside
    `WritebackFixture`, which patches `MATCHED_DIR` at a temp directory.
    """

    def test_no_target_names_a_file_in_the_store(self):
        sources = compiled_sources(cmake_code())
        # Non-vacuity: the extraction really found the hand-written list.
        self.assertIn("src/alpha.cpp", sources)
        self.assertGreater(len(sources), 100)
        self.assertEqual([s for s in sources if "recovered/" in s], [])

    def test_the_same_filter_would_catch_a_stored_body(self):
        # The positive control the substring version never had: with 383
        # bodies on disk and none of them compiled, "no offender found" and
        # "the filter cannot find one" look identical from the outside.
        stored = sorted(STORE.glob("*.cpp"))
        self.assertTrue(stored, "an empty store makes the guard vacuous")
        listed = "add_executable(OpenSMACX src/recovered/%s)" % stored[0].name
        self.assertEqual(
            [s for s in compiled_sources(listed) if "recovered/" in s],
            ["src/recovered/" + stored[0].name])

    def test_no_command_names_the_store_directory(self):
        # A path-shaped filter only sees commands that name a `.cpp`, and two
        # CMake commands take a whole directory without naming one:
        # `aux_source_directory(src/recovered SRCS)` and, through a generated
        # child list, `add_subdirectory(src/recovered)`. Fed to the test above
        # in a temp copy of CMakeLists, both passed it - the store would have
        # been compiled with the guard green. Once the prose is stripped the
        # old substring is usable again and covers them, because no command,
        # variable or generator expression can reach the store without
        # spelling its path. Only the path: bare "recovered" is not the
        # needle, since the registration of `test_verify_recovered_function`
        # spells that word in code rather than in prose.
        self.assertNotIn(STORE_PATH, cmake_code())
        # Both controls, since the assertion is only as good as the strip: it
        # has to survive the damage and ignore the documentation of it.
        self.assertIn(STORE_PATH,
                      cmake_code("aux_source_directory(%s S)" % STORE_PATH))
        self.assertNotIn(
            STORE_PATH,
            cmake_code("# %s/00401000.cpp is inert" % STORE_PATH))

    def test_nothing_globs_for_sources(self):
        # `GLOBAL` properties carry the tool-test registry and are not this;
        # match the command, which lets the check cover the whole file rather
        # than a window of characters after the source list.
        code = cmake_code()
        self.assertIsNone(re.search(r"file\s*\(\s*GLOB", code, re.IGNORECASE))
        # And the control, since the pattern is the whole assertion.
        self.assertIsNotNone(
            re.search(r"file\s*\(\s*GLOB", "file(GLOB_RECURSE s src/*.cpp)",
                      re.IGNORECASE))


class SplitDefinitionTest(unittest.TestCase):
    def test_definition_starts_after_the_doc_comment(self):
        span = ["/*", "Status: Complete", "*/", "void f() {", "}"]
        self.assertEqual(tool.split_definition(span), 3)

    def test_a_span_with_no_comment_starts_at_zero(self):
        self.assertEqual(tool.split_definition(["void f() {", "}"]), 0)

    def test_a_comment_close_inside_the_body_does_not_move_the_boundary(self):
        # A `*/` after the opening brace belongs to the body. Letting it count
        # would splice away real code and leave the file still compiling.
        span = ["void f() {", "    /* inner */", "    return;", "}"]
        self.assertEqual(tool.split_definition(span), 0)


class DocCommentMergeTest(unittest.TestCase):
    """One doc comment per function, not two.

    The brief shows an agent the committed body INCLUDING its header comment
    and asks for the complete definition back, so submissions carry a comment.
    The splice kept the existing comment as well, and 22 duplicated headers
    landed in product source in a single day before anyone looked.
    """

    OLD = ["/*", "Purpose: The old summary.", "Original Offset: 00401000",
           "Status: Complete", "*/"]

    def test_a_submitted_comment_replaces_the_existing_one(self):
        new = ["/*", "Purpose: A better summary.", "Original Offset: 00401000",
               "*/", "void f() {", "}"]
        comment, body = tool.merge_doc_comment(self.OLD, new)
        self.assertEqual(comment, new[:4])
        self.assertEqual(body, ["void f() {", "}"])

    def test_a_bare_submission_keeps_the_existing_comment(self):
        comment, body = tool.merge_doc_comment(self.OLD, ["void f() {", "}"])
        self.assertEqual(comment, self.OLD)
        self.assertEqual(body, ["void f() {", "}"])

    def test_a_purpose_line_is_carried_over_rather_than_dropped(self):
        # `Purpose:` is the one thing in the old comment not recoverable from
        # the code, so a submission that omits it does not get to delete it.
        new = ["/*", "Original Offset: 00401000", "*/", "void f() {", "}"]
        comment, _ = tool.merge_doc_comment(self.OLD, new)
        self.assertIn("Purpose: The old summary.", comment)
        self.assertEqual(comment[0], "/*")

    def test_a_submitted_purpose_wins_over_the_old_one(self):
        new = ["/*", "Purpose: A better summary.", "*/", "void f() {", "}"]
        comment, _ = tool.merge_doc_comment(self.OLD, new)
        self.assertNotIn("Purpose: The old summary.", comment)


class LeadingCommentTest(unittest.TestCase):
    def test_it_counts_a_leading_block(self):
        self.assertEqual(
            tool.leading_comment(["/*", "x", "*/", "void f() {", "}"]), 3)

    def test_code_first_is_not_a_leading_comment(self):
        self.assertEqual(tool.leading_comment(["void f() {", "}"]), 0)

    def test_a_comment_after_the_brace_does_not_count(self):
        # Otherwise a body whose first statement is commented would have that
        # comment mistaken for the header and spliced away.
        self.assertEqual(
            tool.leading_comment(["void f() {", "/* inner", "*/", "}"]), 0)

    def test_an_unterminated_block_counts_as_none(self):
        self.assertEqual(tool.leading_comment(["/*", "x"]), 0)


if __name__ == "__main__":
    unittest.main()
