#!/usr/bin/env python3
"""Unit tests for the import-shim generator.

The derivation under test is small and its failure mode is not: a wrong
argument size becomes a wrong stdcall cleanup, which corrupts the CALLER's
stack and fails far from the cause. So the cases here are about the ways the
size can be got wrong or lost - a symbol that exists only as `__imp_`, an
undecorated cdecl export, an ordinal import with no name at all - rather than
about the happy path.

Nothing here reads the pinned executable. `nm` is replaced by a script that
prints a fixture, so what is exercised is the parsing and the policy, not the
toolchain.
"""

from __future__ import annotations

import sys
import tempfile
import textwrap
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import generate_imports as gen  # noqa: E402


def fake_nm(directory: Path, output: str) -> Path:
    """An executable standing in for `nm`, printing a fixed listing."""
    script = directory / "fake-nm"
    script.write_text("#!/bin/sh\ncat <<'END'\n" + output + "\nEND\n",
                      encoding="utf-8")
    script.chmod(0o755)
    return script


class TestSanitize(unittest.TestCase):
    def test_uppercases_and_drops_the_extension(self):
        self.assertEqual(gen.sanitize("KERNEL32.dll"), "KERNEL32")
        self.assertEqual(gen.sanitize("comdlg32.dll"), "COMDLG32")

    def test_replaces_punctuation_so_the_result_is_an_identifier(self):
        self.assertEqual(gen.sanitize("api-ms-win-crt.dll"), "API_MS_WIN_CRT")

    def test_only_a_trailing_extension_is_dropped(self):
        # "dllhost.dll" must not lose the inner "dll".
        self.assertEqual(gen.sanitize("dllhost.dll"), "DLLHOST")


class TestArgumentBytes(unittest.TestCase):
    def run_nm(self, listing: str) -> dict:
        with tempfile.TemporaryDirectory() as raw:
            directory = Path(raw)
            library = directory / "libfake.a"
            library.write_bytes(b"!<arch>\n")
            return gen.argument_bytes(fake_nm(directory, listing), library)

    def test_reads_the_at_suffix_as_the_byte_count(self):
        found = self.run_nm("00000000 T _ReadFile@20\n"
                            "00000000 T _GetTickCount@0")
        self.assertEqual(found["ReadFile"], 20)
        self.assertEqual(found["GetTickCount"], 0)

    def test_reads_symbols_that_exist_only_as_imp_pointers(self):
        # The whole Interlocked family has no `T` thunk in mingw's
        # libkernel32.a - the compiler supplies them as intrinsics - and is
        # visible only through __imp_. Reading just the thunks silently loses
        # three imports that a lift cannot do without.
        found = self.run_nm("00000000 I __imp__InterlockedIncrement@4")
        self.assertEqual(found["InterlockedIncrement"], 4)

    def test_an_undecorated_export_is_cdecl_and_pops_nothing(self):
        found = self.run_nm("00000000 T _some_cdecl_entry")
        self.assertIsNone(found["some_cdecl_entry"])

    def test_ignores_symbols_that_are_neither_text_nor_import(self):
        found = self.run_nm("00000000 b .bss\n"
                            "         U _SomethingUndefined@4")
        self.assertEqual(found, {})

    def test_a_missing_library_yields_nothing_rather_than_raising(self):
        with tempfile.TemporaryDirectory() as raw:
            directory = Path(raw)
            self.assertEqual(
                gen.argument_bytes(fake_nm(directory, ""),
                                   directory / "absent.a"),
                {})


class TestImplementedImports(unittest.TestCase):
    def test_reads_the_declared_counts_out_of_the_header(self):
        with tempfile.TemporaryDirectory() as raw:
            header = Path(raw) / "lifted_imports.h"
            header.write_text(textwrap.dedent("""\
                #define OPENSMACX_ARGS_KERNEL32_ReadFile 5
                #define OPENSMACX_ARGS_KERNEL32_GetTickCount 0
                // #define OPENSMACX_ARGS_KERNEL32_NotReally 9
                """), encoding="utf-8")
            found = gen.implemented_imports(header)
        self.assertEqual(found,
                         {"KERNEL32_ReadFile": 5, "KERNEL32_GetTickCount": 0})

    def test_the_real_header_agrees_with_the_real_import_libraries(self):
        # Not a fixture: this is the same cross-check the generated
        # static_assert makes, run without a compiler. Every implemented
        # import's hand-written argument count must equal the @N the
        # toolchain's own import library reports. It is here as well as in
        # the generated code so that a disagreement is a test failure naming
        # the import rather than a wall of template errors.
        header = Path(gen.__file__).resolve().parent / "lifted_imports.h"
        declared = gen.implemented_imports(header)
        if not gen.DEFAULT_NM.exists() or not gen.DEFAULT_LIBS.exists():
            self.skipTest("the i686 toolchain is not installed")
        cache: dict[str, dict] = {}
        checked = 0
        for key, count in sorted(declared.items()):
            dll, name = key.split("_", 1)
            if dll not in cache:
                cache[dll] = gen.argument_bytes(
                    gen.DEFAULT_NM,
                    gen.library_for(gen.DEFAULT_LIBS, dll + ".dll"))
            self.assertIn(name, cache[dll],
                          f"{key} is implemented but not exported by lib{dll.lower()}.a")
            self.assertEqual(cache[dll][name], count * 4,
                             f"{key}: prototype says {count} arguments,"
                             f" the import library says @{cache[dll][name]}")
            checked += 1
        self.assertGreater(checked, 60)


class TestRender(unittest.TestCase):
    def entry(self, **overrides) -> dict:
        base = {"dll": "KERNEL32.dll", "name": "ReadFile",
                "key": "KERNEL32_ReadFile", "iat": 0x006691F4,
                "convention": "stdcall", "argument_bytes": 20,
                "unresolved_reason": "", "declared_arguments": 5,
                "implemented": True, "index": 0,
                "synthetic": gen.IMPORT_BASE}
        base.update(overrides)
        return base

    def test_an_implemented_shim_cleans_up_by_the_derived_count(self):
        text = gen.render([self.entry()], Path("x"))
        self.assertIn("s.esp += 4U + 20U;", text)
        self.assertIn("opensmacx_shim_KERNEL32_ReadFile(s)", text)

    def test_it_static_asserts_the_two_counts_against_each_other(self):
        text = gen.render([self.entry()], Path("x"))
        self.assertIn("static_assert(OPENSMACX_ARGS_KERNEL32_ReadFile * 4U",
                      text)
        self.assertIn("== 20U,", text)

    def test_an_unimplemented_import_traps_naming_itself(self):
        text = gen.render(
            [self.entry(implemented=False, declared_arguments=None)],
            Path("x"))
        self.assertIn('"unimplemented import KERNEL32.dll!ReadFile"', text)
        # And it must NOT move ESP: a trap that first pops a guessed number
        # of bytes would corrupt the caller on the way to reporting.
        self.assertNotIn("s.esp +=", text)

    def test_a_cdecl_import_pops_only_the_return_address(self):
        text = gen.render(
            [self.entry(convention="cdecl", argument_bytes=0)], Path("x"))
        self.assertIn("s.esp += 4U + 0U;", text)

    def test_the_slot_table_is_dense_and_in_synthetic_order(self):
        entries = [self.entry(index=0, synthetic=gen.IMPORT_BASE),
                   self.entry(index=1, name="WriteFile",
                              key="KERNEL32_WriteFile", iat=0x006691F0,
                              synthetic=gen.IMPORT_BASE + gen.IMPORT_STRIDE)]
        text = gen.render(entries, Path("x"))
        first = text.index("&opensmacx_import_000,")
        second = text.index("&opensmacx_import_001,")
        self.assertLess(first, second)

    def test_it_asserts_the_range_constants_match_the_header(self):
        text = gen.render([self.entry()], Path("x"))
        self.assertIn(f"OpensmacxImportBase == {gen.IMPORT_BASE:#010x}U",
                      text)
        self.assertIn(f"OpensmacxImportStride == {gen.IMPORT_STRIDE}U", text)


class TestPolicy(unittest.TestCase):
    def test_the_reserved_range_starts_above_every_image_section(self):
        # The image ends below 0x00A10000; a synthetic address must never
        # collide with a real one or the dispatcher's range test is wrong.
        self.assertGreater(gen.IMPORT_BASE, 0x00A10000)

    def test_the_stride_leaves_the_index_recoverable(self):
        self.assertEqual(gen.IMPORT_BASE % gen.IMPORT_STRIDE, 0)


class TestAgainstTheRealImage(unittest.TestCase):
    """The one test that reads the user's executable, and reads only shape."""

    def setUp(self):
        if not gen.DEFAULT_EXE.exists():
            self.skipTest("the pinned executable is not present")
        if not gen.DEFAULT_NM.exists():
            self.skipTest("the i686 toolchain is not installed")

    def test_every_import_gets_a_distinct_slot_and_synthetic_address(self):
        header = Path(gen.__file__).resolve().parent / "lifted_imports.h"
        entries = gen.collect(gen.DEFAULT_EXE, gen.DEFAULT_NM,
                              gen.DEFAULT_LIBS, header)
        self.assertEqual(len(entries), 221)
        self.assertEqual(len({one["iat"] for one in entries}), len(entries))
        self.assertEqual(len({one["synthetic"] for one in entries}),
                         len(entries))
        self.assertEqual(len({one["dll"] for one in entries}), 11)

    def test_only_ordinal_imports_lack_an_argument_size(self):
        header = Path(gen.__file__).resolve().parent / "lifted_imports.h"
        entries = gen.collect(gen.DEFAULT_EXE, gen.DEFAULT_NM,
                              gen.DEFAULT_LIBS, header)
        unsized = [one for one in entries if one["argument_bytes"] is None]
        self.assertTrue(all(one["name"].startswith("ORD_")
                            for one in unsized),
                        [one["name"] for one in unsized])
        self.assertEqual(len(unsized), 4)

    def test_nothing_is_implemented_without_a_derived_argument_size(self):
        header = Path(gen.__file__).resolve().parent / "lifted_imports.h"
        entries = gen.collect(gen.DEFAULT_EXE, gen.DEFAULT_NM,
                              gen.DEFAULT_LIBS, header)
        for one in entries:
            if one["implemented"]:
                self.assertIsNotNone(one["argument_bytes"], one["name"])

    def test_each_iat_address_is_derived_correctly_from_FirstThunk(self):
        # The IAT address is one of the two facts that cannot be invented, and
        # it is taken here straight from pefile's `entry.address`. That
        # attribute is one pefile version away from meaning the INT rather
        # than the IAT, and the C++ side cannot notice: every import would bind
        # to a slot four bytes along, every call in the image would reach the
        # neighbouring function, and every ABI check would still pass because
        # each shim is internally consistent.
        #
        # So the address is re-derived here from a different fact - the
        # descriptor's FirstThunk RVA plus four times the import's position in
        # its own DLL - and the two must agree for all 221.
        header = Path(gen.__file__).resolve().parent / "lifted_imports.h"
        entries = gen.collect(gen.DEFAULT_EXE, gen.DEFAULT_NM,
                              gen.DEFAULT_LIBS, header)
        derived = {}
        pe = gen.pefile.PE(str(gen.DEFAULT_EXE), fast_load=True)
        pe.parse_data_directories(directories=[
            gen.pefile.DIRECTORY_ENTRY["IMAGE_DIRECTORY_ENTRY_IMPORT"]])
        base = pe.OPTIONAL_HEADER.ImageBase
        for descriptor in pe.DIRECTORY_ENTRY_IMPORT:
            dll = descriptor.dll.decode("ascii")
            first_thunk = descriptor.struct.FirstThunk
            for position, one in enumerate(descriptor.imports):
                name = (one.name.decode("ascii") if one.name
                        else f"ORD_{one.ordinal}")
                derived[(dll, name)] = base + first_thunk + 4 * position
        self.assertEqual(len(derived), 221)
        for one in entries:
            self.assertEqual(
                one["iat"], derived[(one["dll"], one["name"])],
                f'{one["dll"]}!{one["name"]}: collect() says'
                f' {one["iat"]:#010x}, FirstThunk arithmetic says'
                f' {derived[(one["dll"], one["name"])]:#010x}')

    def test_the_generated_source_carries_no_image_bytes(self):
        # The policy check: output may name imports and addresses, never
        # content. An IAT slot address is structure; the word inside it at
        # rest would be original data, and never appears.
        header = Path(gen.__file__).resolve().parent / "lifted_imports.h"
        entries = gen.collect(gen.DEFAULT_EXE, gen.DEFAULT_NM,
                              gen.DEFAULT_LIBS, header)
        text = gen.render(entries, gen.DEFAULT_EXE)
        self.assertNotIn("\\x", text)
        self.assertNotIn("unsigned char", text)


if __name__ == "__main__":
    unittest.main()
