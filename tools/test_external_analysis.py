#!/usr/bin/env python3

from collections import defaultdict
import csv
import hashlib
import json
from pathlib import Path
import tempfile
import unittest

from correlate_external_analysis import (
    correlate_leads,
    expand_addresses,
    output_stem,
    parse_disassembly,
    parse_function_notes,
    summarize_correlations,
)
from build_export_recovery_queue import (
    load_external_counts,
    map_exports,
    queue_rows,
    symbol_stem,
)
from fetch_external_analysis import fetch_artifact, validated_relative_path


class ExternalAnalysisTests(unittest.TestCase):
    def test_expands_abbreviated_address_groups(self):
        self.assertEqual(expand_addresses("645460/70"), [0x645460, 0x645470])
        self.assertEqual(
            expand_addresses("625E30/50/C0"), [0x625E30, 0x625E50, 0x625EC0])
        self.assertEqual(
            output_stem("Information/Yitzi/functionlisting.txt"),
            "information-yitzi-functionlisting-txt")

    def test_parses_function_notes_without_prose(self):
        text = (
            "422F00: primary note\n"
            "  4E9550: indented note\n"
            "text mentioning 501DA0 without a prefix\n")
        self.assertEqual(
            parse_function_notes(text), [(1, 0x422F00), (2, 0x4E9550)])

    def test_parses_disassembly_instruction_addresses(self):
        text = (
            "004079F7   mov eax, dword ptr ds:[edx+A88]  label\n"
            "not an instruction\n")
        self.assertEqual(parse_disassembly(text), [(1, 0x4079F7)])

    def test_correlates_and_summarizes_canonical_functions(self):
        first = {
            "address": "0x00401000", "name": "first", "recovery_state": "unrecovered"}
        second = {
            "address": "0x00402000", "name": "second", "recovery_state": "source_complete"}
        entries = {0x401000: first, 0x402000: second}
        ranges = [(0x401000, 0x401010, first), (0x402000, 0x402020, second)]
        rows = correlate_leads(
            [(1, 0x401000), (2, 0x402010), (3, 0x402011), (4, 0x403000)],
            entries, ranges)
        self.assertEqual(rows[0][2:], ("entry", first))
        self.assertEqual(rows[1][2:], ("interior", second))
        self.assertEqual(rows[3][2:], ("", None))
        summary = summarize_correlations(rows)
        self.assertEqual(summary[0]["canonical_address"], "0x00402000")
        self.assertEqual(summary[0]["lead_count"], 2)

        shared = {"address": "0x00403000", "name": "shared"}
        ambiguous = correlate_leads(
            [(5, 0x402010)], entries,
            ranges + [(0x402010, 0x402018, shared)])
        self.assertEqual(ambiguous[0][2:], ("ambiguous", None))

    def test_maps_and_ranks_exported_functions(self):
        exact = {
            "address": "0x00401000", "name": "?exact@@YAHH@Z",
            "recovery_state": "redirect_name_match", "binary_kind": "game",
            "size": "20", "caller_count": "3", "call_target_count": "0"}
        stem = {
            "address": "0x00402000", "name": "?signed@@YAHH@Z",
            "recovery_state": "original_dependency", "binary_kind": "game",
            "size": "30", "caller_count": "1", "call_target_count": "1"}
        exports = [("?exact@@YAHH@Z", "exact_impl"),
                   ("?signed@@YAHI@Z", "signed_impl")]
        mapped = map_exports(exports, [exact, stem])
        self.assertEqual(symbol_stem(exports[1][0]), "?signed")
        self.assertEqual(mapped[0][2], "exact")
        self.assertEqual(mapped[1][2], "unique_stem")
        rows = queue_rows(mapped, {"0x00401000": 2}, defaultdict(set))
        self.assertEqual(rows[0]["canonical_address"], "0x00402000")
        self.assertEqual(rows[0]["tier"], "P0")

        unrecovered = dict(exact, address="0x00403000", name="?new@@YAXXZ",
                           recovery_state="unrecovered")
        rows = queue_rows(
            map_exports([("?new@@YAXXZ", "new_impl")], [unrecovered]), {}, {})
        self.assertEqual(rows[0]["tier"], "P3")

    def test_validates_paths_and_external_summary_provenance(self):
        self.assertEqual(
            validated_relative_path("Information/Yitzi/variables.txt"),
            Path("Information/Yitzi/variables.txt"))
        for unsafe in ("../escape", "/absolute", "C:\\escape", "a\\..\\escape"):
            with self.assertRaises(RuntimeError):
                validated_relative_path(unsafe)

        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            catalog = root / "catalog.json"
            catalog.write_text(json.dumps({"artifacts": [{
                "path": "source.txt", "sha256": "abc",
                "kind": "human_function_notes"}]}), encoding="utf-8")
            inventory = root / "functions.csv"
            inventory.write_text("address,name\n", encoding="utf-8")
            inventory_sha256 = hashlib.sha256(inventory.read_bytes()).hexdigest()
            summary = root / "source-txt-summary.csv"
            with summary.open("w", newline="", encoding="utf-8") as file:
                writer = csv.DictWriter(file, fieldnames=[
                    "source_path", "source_sha256", "inventory_sha256", "canonical_address",
                    "lead_count"])
                writer.writeheader()
                writer.writerow({
                    "source_path": "source.txt", "source_sha256": "abc",
                    "inventory_sha256": inventory_sha256,
                    "canonical_address": "0x00401000", "lead_count": 2})
            counts, _ = load_external_counts(root, catalog, inventory)
            self.assertEqual(counts["0x00401000"], 2)
            summary.write_text(
                "source_path,source_sha256,canonical_address,lead_count\n"
                "source.txt,bad,0x00401000,2\n", encoding="utf-8")
            with self.assertRaises(RuntimeError):
                load_external_counts(root, catalog, inventory)
            summary.write_text(
                "source_path,source_sha256,inventory_sha256,canonical_address,lead_count\n"
                f"source.txt,abc,{inventory_sha256},0x00401000,2\n", encoding="utf-8")
            summary.rename(root / "untrusted-summary.csv")
            counts, _ = load_external_counts(root, catalog, inventory)
            self.assertFalse(counts)

    def test_fetch_rejects_symlinked_destination_before_creating_children(self):
        with tempfile.TemporaryDirectory(
                dir=Path(__file__).resolve().parent.parent / "build") as local_directory:
            with tempfile.TemporaryDirectory() as external_directory:
                output = Path(local_directory)
                (output / "Information").symlink_to(
                    Path(external_directory), target_is_directory=True)
                artifact = {
                    "download_url": "https://invalid.example/source.txt",
                    "path": "Information/nested/source.txt",
                    "sha256": "unused",
                    "size": 0,
                }
                with self.assertRaisesRegex(RuntimeError, "must not contain symlinks"):
                    fetch_artifact(artifact, output)
                self.assertFalse((Path(external_directory) / "nested").exists())


if __name__ == "__main__":
    unittest.main()
