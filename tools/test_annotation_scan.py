#!/usr/bin/env python3
"""Offline tests for tools/annotation_scan.py.

The scanner is the reader for the single source-map grammar, and everything
downstream (the status tool, the migrator, the placeholder generator) trusts
its spans, so these pin the grammar's edges with synthetic fixtures. No VC6,
no Wine, no PE - and no reads of the real src/ tree, so a migration in
progress cannot move these expectations.
"""

from __future__ import annotations

import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import annotation_scan as scan  # noqa: E402


def write(root: Path, rel: str, text: str) -> Path:
    path = root / rel
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text)
    return path


class Grammar(unittest.TestCase):
    """The new marker, in every comment shape it promises to survive."""

    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.root = Path(self._tmp.name)

    def tearDown(self):
        self._tmp.cleanup()

    def scan(self, text: str, rel: str = "src/x.cpp"):
        path = write(self.root, rel, text)
        return scan.scan_file(path), path

    def test_body_marker_line_comment(self):
        annotations, _ = self.scan(
            "// ORIGINAL: 0x00401640\n"
            "int f() {\n    return 1;\n}\n")
        self.assertEqual(len(annotations), 1)
        ann = annotations[0]
        self.assertEqual(ann.address, 0x00401640)
        self.assertEqual(ann.mode, scan.MODE_BODY)
        self.assertEqual(ann.state, scan.STATE_IMPLEMENTED)
        self.assertFalse(ann.deprecated)
        self.assertIn("return 1;", ann.region)
        self.assertTrue(ann.region.rstrip().endswith("}"))

    def test_body_marker_inside_block_comment(self):
        annotations, _ = self.scan(
            "/*\nPurpose: demo.\nORIGINAL: 0x00401640\nStatus: Complete\n*/\n"
            "int f() {\n    return 1;\n}\n")
        self.assertEqual(len(annotations), 1)
        self.assertEqual(annotations[0].address, 0x00401640)
        self.assertIn("return 1;", annotations[0].region)

    def test_marker_on_comment_opening_line(self):
        annotations, _ = self.scan(
            "/* ORIGINAL: 0x00401640 */\n"
            "int f() {\n    return 1;\n}\n")
        self.assertEqual(len(annotations), 1)

    def test_file_marker_compiles_whole_file(self):
        text = ("// ORIGINAL: 0x00403B70 FILE\n"
                "// name ?foo@@YAH@Z\n"
                "int __cdecl foo(int x) { return x + 1; }\n")
        annotations, path = self.scan(text)
        self.assertEqual(len(annotations), 1)
        ann = annotations[0]
        self.assertEqual(ann.mode, scan.MODE_FILE)
        self.assertEqual(ann.recipe, "verbatim")
        self.assertEqual(ann.region, text)

    def test_excluded_marker_cites_ground(self):
        annotations, _ = self.scan(
            "// ORIGINAL: 0x0064A123 EXCLUDED S1 availability - CRT\n")
        self.assertEqual(len(annotations), 1)
        ann = annotations[0]
        self.assertEqual(ann.state, scan.STATE_EXCLUDED)
        self.assertEqual(ann.exclusion, "S1")

    def test_excluded_accepts_section_letter_forms(self):
        for spelling, want in (("S2a", "S2a"), ("2a", "S2a"), ("§3", "S3")):
            annotations, _ = self.scan(
                f"// ORIGINAL: 0x005D4240 EXCLUDED {spelling}\n")
            self.assertEqual(annotations[0].exclusion, want, spelling)

    def test_placeholder_is_measured_not_claimed(self):
        annotations, _ = self.scan(
            "// ORIGINAL: 0x00405C20\n"
            "void __cdecl bar(void) {\n    // BODY GOES HERE.\n}\n")
        self.assertEqual(annotations[0].state, scan.STATE_PLACEHOLDER)

    def test_placeholder_file_mode_sentinel_only(self):
        annotations, _ = self.scan(
            "// ORIGINAL: 0x00405C20 FILE\n// placeholder\n\n"
            "// BODY GOES HERE.\n")
        self.assertEqual(annotations[0].state, scan.STATE_PLACEHOLDER)

    def test_code_after_sentinel_is_implemented(self):
        # Work appended BELOW the sentinel counts as a body.
        annotations, _ = self.scan(
            "// ORIGINAL: 0x00405C20\n"
            "void __cdecl bar(void) {\n    // BODY GOES HERE.\n"
            "    return;\n}\n")
        self.assertEqual(annotations[0].state, scan.STATE_IMPLEMENTED)

    def test_pristine_scaffold_is_still_a_placeholder(self):
        # The emitter puts a PLACEHOLDER-tagged return after the sentinel so
        # non-void skeletons compile before a body exists; that residue is
        # untouched by definition.
        annotations, _ = self.scan(
            "// ORIGINAL: 0x0044AF90 FILE\n"
            "extern \"C\" int __cdecl sub_44af90() {\n"
            "    // BODY GOES HERE.\n"
            "    //\n    // advice comments\n\n"
            "    return (int)0;  // PLACEHOLDER - replace with the body\n"
            "}\n")
        self.assertEqual(annotations[0].state, scan.STATE_PLACEHOLDER)

    def test_marker_in_code_is_not_a_map_entry(self):
        annotations, _ = self.scan(
            'const char *s = "ORIGINAL: 0x00401640";\n')
        self.assertEqual(annotations, [])

    def test_proven_against_original_is_not_a_marker(self):
        # The oracle files carry `PROVEN-AGAINST-ORIGINAL: 0x...` provenance
        # comments; the lookbehind keeps them out of the map.
        annotations, _ = self.scan(
            "// PROVEN-AGAINST-ORIGINAL: 0x006070C0  ?close@BaseButton@@\n")
        self.assertEqual(annotations, [])

    def test_multiple_functions_one_file(self):
        annotations, _ = self.scan(
            "// ORIGINAL: 0x00401640\n"
            "int f() {\n    return 1;\n}\n"
            "// ORIGINAL: 0x00402530\n"
            "int g() {\n    return 2;\n}\n")
        self.assertEqual([a.address for a in annotations],
                         [0x00401640, 0x00402530])
        self.assertIn("return 1;", annotations[0].region)
        self.assertNotIn("return 2;", annotations[0].region)
        self.assertIn("return 2;", annotations[1].region)
        self.assertNotIn("return 1;", annotations[1].region)

    def test_regions_nest_braces(self):
        annotations, _ = self.scan(
            "// ORIGINAL: 0x00401640\n"
            "int f(int a) {\n    if (a) {\n        return 1;\n    }\n"
            "    return 0;\n}\n"
            "// ORIGINAL: 0x00402530\n"
            "int g() {\n    return 2;\n}\n")
        self.assertEqual(annotations[0].region.count("return"), 2)
        self.assertIn("return 2;", annotations[1].region)


class DeprecatedForms(unittest.TestCase):
    """Read-only recognition of the pre-migration spellings."""

    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.root = Path(self._tmp.name)

    def tearDown(self):
        self._tmp.cleanup()

    def scan(self, text: str, rel: str):
        path = write(self.root, rel, text)
        return scan.scan_file(path), path

    def test_legacy_block_comment(self):
        annotations, _ = self.scan(
            "/*\nPurpose: demo.\nOriginal Offset: 00401640\n"
            "Status: Complete\n*/\n"
            "int StringStruct::current_id() {\n    return 0;\n}\n",
            "src/x.cpp")
        self.assertEqual(len(annotations), 1)
        ann = annotations[0]
        self.assertTrue(ann.deprecated)
        self.assertEqual(ann.address, 0x00401640)
        self.assertEqual(ann.mode, scan.MODE_BODY)
        self.assertIn("return 0;", ann.region)

    def test_legacy_offset_without_hex_is_ignored(self):
        annotations, _ = self.scan(
            "/*\nOriginal Offset: n/a\n*/\n"
            "int helper() {\n    return 0;\n}\n", "src/x.cpp")
        self.assertEqual(annotations, [])

    def test_legacy_trailing_form(self):
        annotations, _ = self.scan(
            "void __cdecl random_rand() { reseed(0); } // 00625700\n",
            "src/random.cpp")
        self.assertEqual(len(annotations), 1)
        ann = annotations[0]
        self.assertTrue(ann.deprecated)
        self.assertEqual(ann.address, 0x00625700)
        self.assertIn("reseed(0);", ann.region)

    def test_legacy_trailing_multiline_body(self):
        annotations, _ = self.scan(
            "void __cdecl f(int a) {\n    if (a) {\n        g();\n    }\n"
            "} // 00625720\n", "src/random.cpp")
        self.assertEqual(len(annotations), 1)
        self.assertIn("g();", annotations[0].region)
        self.assertTrue(annotations[0].region.rstrip().endswith(
            "} // 00625720"))

    def test_legacy_opening_brace_form(self):
        # src/text.cpp's shape: the address rides the OPENING brace line.
        annotations, _ = self.scan(
            "int __cdecl text_get_number(int min, int max) { // 00585120\n"
            "    return source(min, max);\n}\n", "src/text.cpp")
        self.assertEqual(len(annotations), 1)
        ann = annotations[0]
        self.assertTrue(ann.deprecated)
        self.assertEqual(ann.address, 0x00585120)
        self.assertIn("source(min, max);", ann.region)
        self.assertTrue(ann.region.rstrip().endswith("}"))

    def test_marker_wins_over_recovered_adapter(self):
        # A migrated file under src/recovered/ is read by the new grammar even
        # while the adapter exists: explicit marker suppresses the adapter.
        annotations, _ = self.scan(
            "// ORIGINAL: 0x00401000\n"
            "StringStruct::StringStruct(int a1) {\n    p[0] = 0;\n}\n",
            "src/recovered/00401000.cpp")
        self.assertEqual(len(annotations), 1)
        self.assertFalse(annotations[0].deprecated)

    def test_proved_store_adapter(self):
        annotations, _ = self.scan(
            "// 0x00401000  ??0StringStruct@@QAE@H@Z  ->  same\n"
            "//\n// provenance prose\n\n"
            "StringStruct::StringStruct(int a1) {\n    p[0] = 0;\n}\n",
            "src/recovered/00401000.cpp")
        self.assertEqual(len(annotations), 1)
        ann = annotations[0]
        self.assertTrue(ann.deprecated)
        self.assertEqual(ann.mode, scan.MODE_BODY)
        self.assertEqual(ann.recipe, "writeback")
        self.assertIn("p[0] = 0;", ann.region)
        self.assertNotIn("provenance prose", ann.region)

    def test_preserved_unit_adapter(self):
        text = ("// PRESERVED UNIT - measured BYTE_EXACT.\n"
                "// address        0x00403B70\n"
                "// GENERATED SKELETON - tools/emit_translation_unit.py\n"
                "int f() {\n    return 1;\n}\n")
        annotations, _ = self.scan(text, "src/recovered/units/00403b70.cpp")
        self.assertEqual(len(annotations), 1)
        ann = annotations[0]
        self.assertTrue(ann.deprecated)
        self.assertEqual(ann.mode, scan.MODE_FILE)
        self.assertEqual(ann.recipe, "verbatim")
        self.assertEqual(ann.region, text)


class CrossRef(unittest.TestCase):

    def annotate(self, address, path="src/x.cpp", line=1):
        return scan.Annotation(address=address, mode=scan.MODE_BODY,
                               state=scan.STATE_IMPLEMENTED, path=path,
                               line=line, region="int f() {}\n")

    def test_matched_and_catalog_only(self):
        catalog = {0x00401000: {}, 0x00401060: {}, 0x00401560: {}}
        annotations = [self.annotate(0x00401000), self.annotate(0x00401560)]
        cross = scan.cross_reference(annotations, catalog)
        self.assertEqual(set(cross.matched), {0x00401000, 0x00401560})
        self.assertEqual(cross.catalog_only, [0x00401060])
        self.assertEqual(cross.uncatalogued, [])
        self.assertEqual(cross.duplicates, {})

    def test_uncatalogued_annotation(self):
        cross = scan.cross_reference([self.annotate(0x00999999)], {0x00401000: {}})
        self.assertEqual([a.address for a in cross.uncatalogued], [0x00999999])
        self.assertEqual(cross.matched, {})

    def test_duplicate_is_reported_and_unmatched(self):
        catalog = {0x00401000: {}}
        first = self.annotate(0x00401000, "src/a.cpp")
        second = self.annotate(0x00401000, "src/b.cpp")
        cross = scan.cross_reference([first, second], catalog)
        self.assertEqual(cross.matched, {})
        self.assertEqual(cross.duplicates[0x00401000], [first, second])


class ExtractorEquivalence(unittest.TestCase):
    """The text extractor is a copy of the census brace counter, so it is
    pinned to the canonical one over the whole real tree: a drift between
    them would mean the scanner and the ratchet measure different spans,
    which is the exact failure a second extractor is not allowed to add."""

    def test_forward_text_matches_census_on_every_real_annotation(self):
        import byte_match_census
        checked = 0
        for annotation in scan.scan_tree(scan.REPO_ROOT / "src"):
            if annotation.mode != scan.MODE_BODY or not annotation.line \
                    or annotation.extract_error \
                    or annotation.state != scan.STATE_IMPLEMENTED \
                    or annotation.recipe == "writeback":
                # writeback-recipe bodies are the definition after the
                # header run, never a brace count from the marker line, so
                # the census span does not apply to them.
                continue
            line_text = (scan.REPO_ROOT / annotation.path).read_text() \
                .splitlines()[annotation.line - 1]
            if scan.LEGACY_TRAILING.search(line_text) \
                    or scan.LEGACY_OPENING.search(line_text):
                # Inline-form markers: the census forward count from the
                # marker line never applied to them (it lands in the NEXT
                # function for the trailing form), so there is no census
                # span to agree with. The backward/forward extractors they
                # use are pinned by their own tests instead.
                continue
            source = f"{annotation.path}:{annotation.line}"
            try:
                _, lines, start, end = byte_match_census.body_span(source)
            except (ValueError, OSError):
                self.fail(f"census refuses {source} but the scan accepted it")
            want = "\n".join(lines[start:end + 1]) + "\n"
            if "*/" in want.split("{", 1)[0]:
                want = "/*\n" + want
            self.assertEqual(annotation.region, want, source)
            checked += 1
        self.assertGreater(checked, 2000)


class TreeScanning(unittest.TestCase):

    def test_scan_tree_is_deterministic_and_recursive(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            write(root, "src/deep/nested.cpp",
                  "// ORIGINAL: 0x00401640\nint f() {\n    return 1;\n}\n")
            write(root, "src/a.cpp",
                  "// ORIGINAL: 0x00402530\nint g() {\n    return 2;\n}\n")
            write(root, "src/skip.txt", "ORIGINAL: 0x00999999\n")
            found = scan.scan_tree(root)
            # Fixture trees live outside the repo, so paths are absolute;
            # order and addresses are the contract under test.
            self.assertEqual([a.address for a in found],
                             [0x00402530, 0x00401640])
            self.assertTrue(found[0].path.endswith("src/a.cpp"))
            self.assertTrue(found[1].path.endswith(
                str(Path("src/deep/nested.cpp"))))




class LessonGrammarTest(unittest.TestCase):
    """`LEVER:` and `RULED-OUT:`, and where they are NOT read from."""

    def scan(self, text):
        return scan.scan_text(text, "src/x.cpp")[0]

    def test_a_lever_carries_its_fingerprint_and_prose(self):
        a = self.scan("// ORIGINAL: 0x00401000 BYTE_EXACT\n"
                      "// LEVER: jl/jge  flipped the condition and swapped arms\n"
                      "void f() {}\n")
        self.assertEqual(a.levers,
                         (("jl/jge", "flipped the condition and swapped arms"),))

    def test_ruled_out_is_unkeyed(self):
        # Deliberately: it sits on a body that has NOT matched, so the
        # divergence is live and the key is measured. A key never written can
        # never be stale.
        a = self.scan("// ORIGINAL: 0x00401000\n"
                      "// RULED-OUT: ternary; do/while; counting down\n"
                      "void f() {}\n")
        self.assertEqual(a.ruled_out, ("ternary; do/while; counting down",))

    def test_an_indented_continuation_joins_the_line_above(self):
        a = self.scan("// ORIGINAL: 0x00401000\n"
                      "// RULED-OUT: plain immediate; named local;\n"
                      "//            register storage; a 1-bit bitfield\n"
                      "void f() {}\n")
        self.assertEqual(len(a.ruled_out), 1)
        self.assertIn("bitfield", a.ruled_out[0])

    def test_the_run_ends_at_the_first_non_comment(self):
        # Otherwise a `RULED-OUT:` mentioned in prose halfway down a 1,000-line
        # file would attach itself to the marker at the top.
        a = self.scan("// ORIGINAL: 0x00401000\n"
                      "void f() {}\n"
                      "// RULED-OUT: this is prose, not a claim\n")
        self.assertEqual(a.ruled_out, ())

    def test_the_existing_extern_symbol_lever_convention_is_not_read(self):
        # `// EXTERN-SYMBOL LEVER:` predates this grammar and appears 17 times
        # in src/recovered/units/. It is a different token and must stay one.
        a = self.scan("// ORIGINAL: 0x00401000 BYTE_EXACT\n"
                      "// EXTERN-SYMBOL LEVER: the table is walked by pointer\n"
                      "void f() {}\n")
        self.assertEqual(a.levers, ())

    def test_a_body_with_no_lessons_carries_empty_tuples(self):
        a = self.scan("// ORIGINAL: 0x00401000\nvoid f() {}\n")
        self.assertEqual((a.levers, a.ruled_out), ((), ()))


class RegionEndTests(unittest.TestCase):
    """Where a marker's region stops. Three agents found this independently.

    The counter closed the region at the FIRST return to brace depth zero, so
    anything a body needs AHEAD of its definition - a helper class expressing
    a `__thiscall` receiver, an edited VCall shim whose generated form is
    nullary, an inline `operator new` - was where the region ended, and the
    function the marker names was dropped in silence. The piece then scored
    "expected one external .text symbol, found 0" while the same text compiled
    standalone. Four addresses in one batch were converted to FILE mode to
    route around it.
    """

    def end(self, *lines):
        return scan.region_end(list(lines), 0)

    def test_a_helper_class_does_not_end_the_region(self):
        self.assertEqual(5, self.end(
            "// ORIGINAL: 0x00401000 X",
            "class Helper { public: void f(); };",
            "",
            "void g() {",
            "  int x;",
            "}"))

    def test_an_unmarked_neighbour_is_not_absorbed(self):
        """A type declaration closes with `};`, a function body with `}`.

        That is the only signal for telling "a helper this piece needs" from
        "the next definition, which carries no marker of its own", and both
        are in this tree - `test_writeback` caught an over-eager region
        swallowing a neighbouring function.
        """
        self.assertEqual(2, self.end(
            "// ORIGINAL: 0x00401000 X",
            "void g() {",
            "}",
            "",
            "void later() {",
            "}"))

    def test_a_brace_in_a_string_is_not_structure(self):
        self.assertEqual(3, self.end(
            "// ORIGINAL: 0x00401000 X",
            "void g() {",
            '  char *s = "}";',
            "}"))

    def test_a_brace_in_a_comment_is_not_structure(self):
        # An agent wrote `class Shim { ... };` into a RULED-OUT note and the
        # note truncated its own body - the bug reproduced by describing it.
        self.assertEqual(3, self.end(
            "// ORIGINAL: 0x00401000 X",
            "// note: class Shim { v(); };",
            "void g() {",
            "}"))

    def test_a_single_line_definition_closes_the_region(self):
        # Its delta is zero, so testing the depth change alone never saw it
        # open at all.
        self.assertEqual(1, self.end(
            "// ORIGINAL: 0x00401000 X", "void g() { return; }"))

    def test_the_next_marker_bounds_the_region(self):
        self.assertEqual(1, self.end(
            "// ORIGINAL: 0x00401000 X",
            "void g() { return; }",
            "",
            "// ORIGINAL: 0x00402000 Y",
            "void h() { }"))


if __name__ == "__main__":
    unittest.main()
